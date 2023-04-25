#include "FFmpegVideoDecoder.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <QDebug>

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

    codec = avcodec_find_decoder_by_name("hevc");//m_pStream->codecpar->codec_id);
    if(!codec)
        emit error(QString("FFmpegVideoDecoder: Error, cannot find decoder by codec_ID"));

    m_pCctx = avcodec_alloc_context3(codec);
    if(!m_pCctx)
        emit error(QString("FFmpegVideoDecoder: Error, cannot allocate codec context"));

    ret = avcodec_open2(m_pCctx, codec, nullptr);
    if(ret < 0)
        emit error(QString("FFmpegVideoDecoder: Error, cannot open codec"));

    m_pFrame = av_frame_alloc();
    m_pPkt   = av_packet_alloc();

    std::ofstream myfile;
    int cnt = 0;



    while(av_read_frame(m_pIc, m_pPkt) >= 0 && cnt < 1000)
    {
        if(m_pPkt->stream_index == AVMEDIA_TYPE_VIDEO)
        {

            int check = 0;
            m_pPkt->stream_index = m_pStream->id;

            ret = avcodec_send_packet(m_pCctx, m_pPkt);
            char err_buf[1024];
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
            m_pImg_conversion = sws_getContext
                (m_pCctx->width,
                 m_pCctx->height,

                 (AVPixelFormat)m_pCctx->pix_fmt,

                 m_pCctx->width,
                 m_pCctx->height,
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

            std::stringstream name;
            name << "test" << cnt << ".ppm";
            // emit frameDecoded
            // Convert the frame to QImage
            m_pLastFrame = new QImage(m_pCctx->width, m_pCctx->height, QImage::Format_RGB888);

            for(int y=0; y < m_pCctx->height; y++)
                memcpy(m_pLastFrame->scanLine(y),m_pFrame_converted->data[0]+y*m_pFrame_converted->linesize[0],m_pCctx->width*3);

            // Set the time
            //DesiredFrameTime = ffmpeg::av_rescale_q(after,pFormatCtx->streams[videoStream]->time_base,millisecondbase);
            //LastFrameOk=true;


            //done = true;

        }
        av_packet_unref(m_pPkt);
        av_frame_unref(m_pFrame);
        av_frame_unref(m_pFrame_converted);
        cnt++;
        qDebug() << "Frame number: " << cnt << "\n";
    }
}
