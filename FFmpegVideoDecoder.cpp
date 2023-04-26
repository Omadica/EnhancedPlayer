#include "FFmpegVideoDecoder.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <QDebug>
#include <QLabel>

static void ppm_save(char* filename, AVFrame* frame)
{
    FILE* file;
    int i;

    file = fopen64(filename, "wb");
    frame->data[0];
    fprintf(file, "P6\n%d %d\n%d\n", frame->width, frame->height, 255);
    for (i = 0; i < frame->height; i++)
        fwrite(frame->data[0] + i * frame->linesize[0], 1, frame->width * 3, file);
    fclose(file);
}


AVPixelFormat ConvertFormats(AVFrame* frame)
{
    switch (frame->format) {
    case AV_PIX_FMT_YUVJ420P:
        return AV_PIX_FMT_YUV420P;
        break;
    case AV_PIX_FMT_YUVJ422P:
        return AV_PIX_FMT_YUV422P;
        break;
    case AV_PIX_FMT_YUVJ444P:
        return AV_PIX_FMT_YUV444P;
        break;
    case AV_PIX_FMT_YUVJ440P:
        return AV_PIX_FMT_YUV440P;
    default:
        return static_cast<AVPixelFormat>(frame->format);
        break;
    }
}

FFmpegVideoDecoder::FFmpegVideoDecoder(QObject *parent, AVFormatContext* ic, AVStream* stream)
    : QObject{parent}, m_pIc(ic), m_pStream(stream),
    m_pCctx(nullptr),
    codec(nullptr),
    m_pImg_conversion(nullptr),
    m_pPkt(nullptr),
    m_pFrame(nullptr),
    m_pFrame_converted(nullptr)
{

}

void FFmpegVideoDecoder::decode()
{
    int ret = 0;
    ret = av_read_play(m_pIc);
    if(ret < 0)
        emit error(QString("FFmpegVideoDecoder: Error, cannot read and play rtsp stream"));

    codec = avcodec_find_decoder_by_name("hevc");//(m_pStream->codecpar->codec_id);
    if(!codec)
        emit error(QString("FFmpegVideoDecoder: Error, cannot find decoder by codec_ID"));

    m_pCctx = avcodec_alloc_context3(codec);
    if(!m_pCctx)
        emit error(QString("FFmpegVideoDecoder: Error, cannot allocate codec context"));

    ret = avcodec_open2(m_pCctx, codec, nullptr);
    if(ret < 0)
        emit error(QString("FFmpegVideoDecoder: Error, cannot open codec"));

    m_pFrame = av_frame_alloc();
    m_pFrame_converted = av_frame_alloc();
    m_pFrame_converted->format = AV_PIX_FMT_RGB24;
    m_pFrame_converted->color_range = AVCOL_RANGE_JPEG;
    m_pFrame_converted->width =  m_pStream->codecpar->width;
    m_pFrame_converted->height = m_pStream->codecpar->height;
    av_frame_get_buffer(m_pFrame_converted, 0);
    m_pPkt   = av_packet_alloc();

    std::ofstream myfile;

    char buf[1024];
    char err_buf[1024];
    while(av_read_frame(m_pIc, m_pPkt) >= 0)
    {
        if(m_pPkt->stream_index == AVMEDIA_TYPE_VIDEO)
        {
            int check = 0;
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
                if(ret == 0)
                    break;
                else if (ret < 0) {
                    emit error(QString("Error during decoding"));
                    char err_buf[1024];
                    av_strerror(ret , err_buf, 1024);
                    qDebug() << err_buf << "\n";
                }
            }

            m_pFrame->color_range = (AVColorRange)1;
            m_pImg_conversion = sws_getContext
                (m_pFrame->width,
                 m_pFrame->height,
                 AV_PIX_FMT_YUVJ420P,
                 m_pFrame->width,
                 m_pFrame->height,
                 AV_PIX_FMT_RGB24,
                 SWS_SPLINE,
                 NULL,
                 NULL,
                 NULL);

            sws_scale(m_pImg_conversion,
                      m_pFrame->data,
                      m_pFrame->linesize,
                      0,
                      m_pCctx->height,
                      m_pFrame_converted->data,
                      m_pFrame_converted->linesize
                    );
            m_pLastFrame = QImage(m_pFrame_converted->width, m_pFrame_converted->height, QImage::Format_RGB888);
            for(int y=0; y < m_pFrame_converted->height; y++)
                memcpy(
                    m_pLastFrame.scanLine(y),
                    m_pFrame_converted->data[0] + y * m_pFrame_converted->linesize[0],
                    m_pFrame_converted->width*3
                );

            emit ReturnFrame(m_pLastFrame);


        }
        av_packet_unref(m_pPkt);
        av_frame_unref(m_pFrame);
        av_frame_unref(m_pFrame_converted);
    }
}
