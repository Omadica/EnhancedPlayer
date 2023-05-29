#include "FFmpegVideoDecoder.h"
#include "AppDecUtils.h"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "qvideoframe.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <QDebug>
#include <QLabel>
#include <cuda.h>
#include <cuda_runtime.h>
#include <QByteArray>
#include <QVideoFrameFormat>
#include <QRandomGenerator>
#include <opencv2/core.hpp>

/***
 * A hint by Kef:
 * You connect your decoder to QVideoSink class. It is used to send QVideoFrame to QVideoWidget.
 * You need to check QVideoFrame class compatibility  with your frames and perhaps do additional transcoding.
 * Basically, ffmpeg -> video sink -> (transcode if needed) -> convert your frame to qvideoframe
 *
 * example: https://stackoverflow.com/questions/69432427/how-to-use-qvideosink-in-qml-in-qt6
 * complete example: https://github.com/eyllanesc/stackoverflow/tree/master/questions/69432427
 */


// Rotate cameraMatrix
// https://stackoverflow.com/questions/37117939/transform-a-frame-to-be-as-if-it-was-taken-from-above-using-opencv

//************************************
// Method:    avframeToCvmat
// Access:    public
// Returns:   cv::Mat
// Qualifier:
// Parameter: const AVFrame * frame
//************************************
cv::Mat avframeToCvmat(const AVFrame *frame) {
    int width = frame->width;
    int height = frame->height;
    cv::Mat image(height, width, CV_8UC3);
    int cvLinesizes[1];
    cvLinesizes[0] = image.step1();
    SwsContext *conversion = sws_getContext(
        width, height, (AVPixelFormat)frame->format, width, height,
        AVPixelFormat::AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    sws_scale(conversion, frame->data, frame->linesize, 0, height, &image.data,
              cvLinesizes);
    sws_freeContext(conversion);
    return image;
}

//************************************
// Method:    cvmatToAvframe
// Access:    public
// Returns:   AVFrame *
// Qualifier:
// Parameter: cv::Mat * image
// Parameter: AVFrame * frame
//************************************
AVFrame *cvmatToAvframe(cv::Mat *image, AVFrame *frame) {
    int width = image->cols;
    int height = image->rows;
    int cvLinesizes[1];
    cvLinesizes[0] = image->step1();
    if (frame == NULL) {
        frame = av_frame_alloc();
        av_image_alloc(frame->data, frame->linesize, width, height,
                       AVPixelFormat::AV_PIX_FMT_YUV420P, 1);
    }
    SwsContext *conversion = sws_getContext(
        width, height,
        AVPixelFormat::AV_PIX_FMT_BGR24,
        width, height,
        (AVPixelFormat)frame->format,
        SWS_FAST_BILINEAR,
        NULL, NULL, NULL
        );
    sws_scale(conversion,
              &image->data,
              cvLinesizes,
              0,
              height,
              frame->data,
              frame->linesize);
    sws_freeContext(conversion);
    return frame;
}



QVideoSink *FFmpegVideoDecoder::videoSink() const
{
    return m_videoSink.get();
}

void FFmpegVideoDecoder::setVideoSink(QVideoSink* newVideoSink)
{
    if (m_videoSink == newVideoSink)
        return;
    m_videoSink = newVideoSink;
    emit videoSinkChanged();
}




extern "C"
{
#include <libavutil/imgutils.h>
}

inline cudaVideoCodec FFmpeg2NvCodecId(AVCodecID id) {
    switch (id) {
    case AV_CODEC_ID_MPEG1VIDEO : return cudaVideoCodec_MPEG1;
    case AV_CODEC_ID_MPEG2VIDEO : return cudaVideoCodec_MPEG2;
    case AV_CODEC_ID_MPEG4      : return cudaVideoCodec_MPEG4;
    case AV_CODEC_ID_WMV3       :
    case AV_CODEC_ID_VC1        : return cudaVideoCodec_VC1;
    case AV_CODEC_ID_H264       : return cudaVideoCodec_H264;
    case AV_CODEC_ID_HEVC       : return cudaVideoCodec_HEVC;
    case AV_CODEC_ID_VP8        : return cudaVideoCodec_VP8;
    case AV_CODEC_ID_VP9        : return cudaVideoCodec_VP9;
    case AV_CODEC_ID_MJPEG      : return cudaVideoCodec_JPEG;
    case AV_CODEC_ID_AV1        : return cudaVideoCodec_AV1;
    default                     : return cudaVideoCodec_NumCodecs;
    }
}

void ConvertSemiplanarToPlanar(uint8_t *pHostFrame, int nWidth, int nHeight, int nBitDepth) {
    if (nBitDepth == 8) {
        // nv12->iyuv
        YuvConverter<uint8_t> converter8(nWidth, nHeight);
        converter8.UVInterleavedToPlanar(pHostFrame);
    } else {
        // p016->yuv420p16
        YuvConverter<uint16_t> converter16(nWidth, nHeight);
        converter16.UVInterleavedToPlanar((uint16_t *)pHostFrame);
    }
}

FFmpegVideoDecoder::FFmpegVideoDecoder(QObject *parent, QString rtsp, bool hw_accel, bool nvidia_dev, QString HWdec_name)
    : QObject{parent}, rtsp_addr(rtsp), bool_hw_accel(hw_accel), nv_hw_dev(nvidia_dev), HWDec_name(HWdec_name),
    m_pCctx(nullptr),
    codec(nullptr),
    m_pImg_conversion(nullptr),
    m_pHWconversion(nullptr),
    m_pPkt(nullptr),
    m_pFrame(nullptr),
    m_pFrame_converted(nullptr),
    m_pSWFrame(nullptr),
    m_pOutFrame(nullptr),
    m_pIc(nullptr),
    m_pStream(nullptr),
    nv_dec(nullptr),
    cuContext(nullptr),
    m_stopDecoding(false)
{
    qDebug() << HWdec_name.toStdString().c_str();
}

static AVBufferRef *hw_device_ctx = NULL;
static enum AVPixelFormat hw_pix_fmt;

static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
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
    cv::FileStorage file_read("cameraCalibration.ext", cv::FileStorage::READ);

    cv::Mat cameraMat;
    cv::Mat newCameraMat;
    cv::Mat dcoeff;
    cv::Mat RMat;
    cv::Mat TMat;

    file_read["cameraMat"] >> cameraMat;
    file_read["newCameraMat"] >> newCameraMat;
    file_read["distCoeffs"] >> dcoeff;
    file_read["Rvec"] >> RMat;
    file_read["Tvec"] >> TMat;
    cv::Mat map1;
    cv::Mat map2;
    cv::Mat new_cam;
    cv::Mat cameraRotated;

    file_read.release();

    nv_hw_dev = false;
    int nFrameReturned = 0, nFrame = 0;
    bool bDecodeOutSemiPlanar = false;
    uint8_t* pFrame;
    int ret = 0;
    ret = avformat_open_input(&m_pIc, rtsp_addr.toStdString().c_str(), NULL, NULL);


    if (ret < 0)
        qDebug() << "Error to create AvFormatContext";

    ret = avformat_find_stream_info(m_pIc, NULL);

    ret = av_read_play(m_pIc);
    cv::Size imgSize = cv::Size(m_pIc->streams[0]->codecpar->width, m_pIc->streams[0]->codecpar->height);
    // cv::Mat out1, out2, out3, out4, out5;
    // cv::undistort(raw_frame, new_frame, cameraMat, dcoeff, cameraMat);
    // cv::initUndistortRectifyMap(cameraMat, dcoeff, RMat, cameraMat,  cv::Size(m_pIc->streams[0]->codecpar->width, m_pIc->streams[0]->codecpar->height), CV_32FC1, map1, map2);
    // cv::stereoRectify(cameraMat, dcoeff, cv::getOptimalNewCameraMatrix(cameraMat, dcoeff, imgSize, 0.7, imgSize, 0), cv::Mat(), imgSize, RMat, TMat, out1, out2, out3, out4, out5);
    // cv::fisheye::initUndistortRectifyMap(cameraMat, dcoeff, cv::Mat(), newCameraMat, imgSize, CV_16SC2, map1, map2);
    // cv::initUndistortRectifyMap(cameraMat, dcoeff, cv::Mat(), cameraMat, imgSize, CV_16SC2, map1, map2);
    if(ret < 0)
        emit error(QString("FFmpegVideoDecoder: Error, cannot read and play rtsp stream"));

    if(bool_hw_accel)
    {
        type = av_hwdevice_find_type_by_name(HWDec_name.toStdString().c_str());
        qDebug() << HWDec_name.toStdString().c_str();
        if (type == AV_HWDEVICE_TYPE_NONE) {
            fprintf(stderr, "Device type %s is not supported.\n", "dxva2");
            fprintf(stderr, "Available device types:");
            while((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
                qDebug() << av_hwdevice_get_type_name(type);
            fprintf(stderr, "\n");
            return;
        }
        emit infoDec(QString(av_hwdevice_get_type_name(type)));
        ret = av_find_best_stream(m_pIc, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);

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
    else // SW decoding
    {
        codec = avcodec_find_decoder(m_pIc->streams[0]->codecpar->codec_id);
        if(!codec)
            emit error(QString("FFmpegVideoDecoder: Error, cannot find decoder by codec_ID"));
        else
            emit infoDec(HWDec_name);
    } // end if(bool_hw_accel)




    // alloc context3
    m_pCctx = avcodec_alloc_context3(codec);
    if(!m_pCctx)
        emit error(QString("FFmpegVideoDecoder: Error, cannot allocate codec context"));

    // if hwaccel == true, then create hwdevice
    if(bool_hw_accel){
        m_pCctx->get_format = get_hw_format;
        if (av_hwdevice_ctx_create(&hw_device_ctx, type, NULL, NULL, 0) < 0)
        {
            fprintf(stderr, "Failed to create specified HW device.\n");
            return;
        }
        m_pCctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    }

    // open the codecContext

    ret = avcodec_open2(m_pCctx, codec, nullptr);
    if(ret < 0)
        emit error(QString("FFmpegVideoDecoder: Error, cannot open codec-contex"));

    m_pFrame = av_frame_alloc();


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
         AV_PIX_FMT_BGR24,
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
    int frames = 0;
    while(av_read_frame(m_pIc, m_pPkt) >= 0)
    {
        if(m_pPkt->stream_index == AVMEDIA_TYPE_VIDEO)
        {
            m_pPkt->stream_index = m_pIc->streams[0]->id;
            ret = avcodec_send_packet(m_pCctx, m_pPkt);
            if(ret < 0)
                emit error(QString("FFmpegVideoDecoder: Error, cannot send packet") + QString::fromStdString(err_buf));

            while (ret >= 0) {
                ret = avcodec_receive_frame(m_pCctx, m_pFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
                    return;
                }
                if(ret == 0){
                    if(bool_hw_accel && m_pFrame->format == hw_pix_fmt)
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
                        cv::Mat new_frame;
                        cv::Mat raw_frame = avframeToCvmat(m_pOutFrame);;


                        // cv::remap(raw_frame, new_frame, map1, map2,  cv::INTER_NEAREST);
                        cv::undistort(raw_frame, new_frame, cameraMat, dcoeff, newCameraMat);
                        // cv::fisheye::undistortImage(raw_frame, new_frame, cameraMat, dcoeff, cameraMat);

                        int cvLinesizes[1];
                        cvLinesizes[0] = new_frame.step1();
                        sws_scale(m_pImg_conversion,
                                  &new_frame.data,
                                  cvLinesizes,
                                  0,
                                  m_pOutFrame->height,
                                  m_pFrame_converted->data,
                                  m_pFrame_converted->linesize
                                  );
                        new_frame.release();
                        raw_frame.release();
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
        frames++;

        av_packet_unref(m_pPkt);
        av_frame_unref(m_pFrame);
        if(m_pSWFrame)
            av_frame_unref(m_pSWFrame);
        av_frame_unref(m_pOutFrame);
    }
}


void FFmpegVideoDecoder::stopDecoding()
{
    m_stopDecoding = true;
    qDebug() << "settato m_stopDecoding = true";
}
