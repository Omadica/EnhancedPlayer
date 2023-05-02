#include "FFmpegVideoDecoder.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <QDebug>
#include <QLabel>


extern "C"
{
#include <libavutil/imgutils.h>
}

FFmpegVideoDecoder::FFmpegVideoDecoder(QObject *parent, AVFormatContext* ic, AVStream* stream, bool hw_accel, QString HWdec_name)
    : QObject{parent}, m_pIc(ic), m_pStream(stream), bool_hw_accel(hw_accel), HWDec_name(HWdec_name),
    m_pCctx(nullptr),
    codec(nullptr),
    m_pImg_conversion(nullptr),
    m_pHWconversion(nullptr),
    m_pPkt(nullptr),
    m_pFrame(nullptr),
    m_pFrame_converted(nullptr),
    m_pSWFrame(nullptr),
    m_pOutFrame(nullptr)
{

}

static AVBufferRef *hw_device_ctx = NULL;
static enum AVPixelFormat hw_pix_fmt;

static enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
                                        const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }

    fprintf(stderr, "Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}


void FFmpegVideoDecoder::decode()
{
    int ret = 0;
    ret = av_read_play(m_pIc);
    if(ret < 0)
        emit error(QString("FFmpegVideoDecoder: Error, cannot read and play rtsp stream"));

    if(bool_hw_accel)
    {
        type = av_hwdevice_find_type_by_name(HWDec_name.toStdString().c_str());
        if (type == AV_HWDEVICE_TYPE_NONE) {
            fprintf(stderr, "Device type %s is not supported.\n", "dxva2");
            fprintf(stderr, "Available device types:");
            while((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
                fprintf(stderr, " %s", av_hwdevice_get_type_name(type));
            fprintf(stderr, "\n");
            return;
        }
        emit infoDec(QString(av_hwdevice_get_type_name(type)));
        ret = av_find_best_stream(m_pIc, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    }
    else
    {
        codec = avcodec_find_decoder(m_pIc->streams[0]->codecpar->codec_id);
        if(!codec)
            emit error(QString("FFmpegVideoDecoder: Error, cannot find decoder by codec_ID"));
        else
            emit infoDec(HWDec_name);
    }

    if(bool_hw_accel){
        for (int i = 0;; i++) {
            const AVCodecHWConfig *config = avcodec_get_hw_config(codec, i);
            if (!config) {
                fprintf(stderr, "Decoder %s does not support device type %s.\n",
                        codec->name, av_hwdevice_get_type_name(type));
                return;
            }
            if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
                config->device_type == type) {
                hw_pix_fmt = config->pix_fmt;
                break;
            }
        }
    }


    m_pCctx = avcodec_alloc_context3(codec);
    if(!m_pCctx)
        emit error(QString("FFmpegVideoDecoder: Error, cannot allocate codec context"));


    if(bool_hw_accel){
        m_pCctx->get_format = get_hw_format;
        if (av_hwdevice_ctx_create(&hw_device_ctx, type, NULL, NULL, 0) < 0)
        {
            fprintf(stderr, "Failed to create specified HW device.\n");
            return;
        }
        m_pCctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    }

    ret = avcodec_open2(m_pCctx, codec, nullptr);
    if(ret < 0)
        emit error(QString("FFmpegVideoDecoder: Error, cannot open codec-contex"));

    m_pFrame = av_frame_alloc();

    if(bool_hw_accel)
        m_pSWFrame = av_frame_alloc();

    m_pOutFrame = av_frame_alloc();

    m_pFrame_converted = av_frame_alloc();
    m_pFrame_converted->format = AV_PIX_FMT_RGB24;
    m_pFrame_converted->color_range = AVCOL_RANGE_JPEG;
    m_pFrame_converted->width =  m_pIc->streams[0]->codecpar->width;
    m_pFrame_converted->height = m_pIc->streams[0]->codecpar->height;
    av_frame_get_buffer(m_pFrame_converted, 0);
    m_pPkt   = av_packet_alloc();
    m_pLastFrame = QImage(m_pFrame_converted->width, m_pFrame_converted->height, QImage::Format_RGB888);

    m_pImg_conversion = sws_getContext
        (m_pFrame_converted->width,
         m_pFrame_converted->height,
         AV_PIX_FMT_YUV420P,
         m_pFrame_converted->width,
         m_pFrame_converted->height,
         AV_PIX_FMT_RGB24,
         SWS_SPLINE,
         NULL,
         NULL,
         NULL);

    if(bool_hw_accel){
        m_pHWconversion = sws_getContext
            (m_pFrame_converted->width,
             m_pFrame_converted->height,
             AV_PIX_FMT_NV12,
             m_pFrame_converted->width,
             m_pFrame_converted->height,
             AV_PIX_FMT_RGB24,
             SWS_SPLINE,
             NULL,
             NULL,
             NULL);
    }

    char err_buf[1024];
    while(av_read_frame(m_pIc, m_pPkt) >= 0)
    {
        if(m_pPkt->stream_index == AVMEDIA_TYPE_VIDEO)
        {
            m_pPkt->stream_index = m_pStream->id;

            ret = avcodec_send_packet(m_pCctx, m_pPkt);

            av_strerror(ret , err_buf, 1024);
            qDebug() << err_buf << "\n";
            if(ret < 0)
                emit error(QString("FFmpegVideoDecoder: Error, cannot send packet") + QString::fromStdString(err_buf));

            while (ret >= 0) {
                ret = avcodec_receive_frame(m_pCctx, m_pFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
                    return;
                }
                if(ret == 0){
                    if(bool_hw_accel || m_pFrame->format == hw_pix_fmt)
                    {
                        ret = av_hwframe_transfer_data(m_pSWFrame, m_pFrame, 0);
                        if(ret<0)
                            fprintf(stderr, "Error transferring the data to system memory\n");
                        m_pOutFrame = m_pSWFrame;
                    } else {
                        m_pOutFrame = m_pFrame;
                    }

                    if(bool_hw_accel)
                    {
                        sws_scale(m_pHWconversion,
                                  m_pOutFrame->data,
                                  m_pOutFrame->linesize,
                                  0,
                                  m_pOutFrame->height,
                                  m_pFrame_converted->data,
                                  m_pFrame_converted->linesize
                                  );
                    } else {
                        sws_scale(m_pImg_conversion,
                                  m_pOutFrame->data,
                                  m_pOutFrame->linesize,
                                  0,
                                  m_pOutFrame->height,
                                  m_pFrame_converted->data,
                                  m_pFrame_converted->linesize
                                  );
                    }


                    for(int y=0; y < m_pFrame_converted->height; y++)
                        memcpy(
                            m_pLastFrame.scanLine(y),
                            m_pFrame_converted->data[0] + y * m_pFrame_converted->linesize[0],
                            m_pFrame_converted->width*3
                            );

                    emit ReturnFrame(m_pLastFrame);
                    break;
                }
                else if (ret < 0) {
                    emit error(QString("Error during decoding"));
                    char err_buf[1024];
                    av_strerror(ret , err_buf, 1024);
                    qDebug() << err_buf << "\n";
                }
            }
        }
        av_packet_unref(m_pPkt);
        av_frame_unref(m_pFrame);
        if(m_pSWFrame)
            av_frame_unref(m_pSWFrame);
        av_frame_unref(m_pOutFrame);
    }
}