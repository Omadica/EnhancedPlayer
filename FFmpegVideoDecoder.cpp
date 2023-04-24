#include "FFmpegVideoDecoder.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

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

    codec = avcodec_find_decoder(m_pStream->codecpar->codec_id);
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
    //while(av_read_frame(m_pIc, m_pPkt) >= 0 && cnt < 1000)
    while(true)
    {
        if(m_pPkt->stream_index == AVMEDIA_TYPE_VIDEO)
        {

            int check = 0;
            m_pPkt->stream_index = m_pStream->id;

            ret = avcodec_send_packet(m_pCctx, m_pPkt);
            if(ret < 0)
                emit error(QString("FFmpegVideoDecoder: Error, cannot send packet"));

            while (ret >= 0) {
                ret = avcodec_receive_frame(m_pCctx, m_pFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    return;
                else if (ret < 0) {
                    emit error(QString("Error during decoding"));
                }
            }

            sws_scale(m_pImg_conversion, m_pFrame->data, m_pFrame->linesize, 0, m_pCctx->height, m_pFrame_converted->data, m_pFrame_converted->linesize);
            std::stringstream name;
            name << "test" << cnt << ".ppm";
            myfile.open(name.str());
            myfile << "P3 " << m_pCctx->width << " " << m_pCctx->height << " 255\n";
            for(int y = 0; y < m_pCctx->height; y++)
            {
                for(int x = 0; x < m_pCctx->width * 3; x++)
                    myfile << (int)(m_pFrame_converted->data[0] + y * m_pFrame_converted->linesize[0])[x] << " ";
            }
            myfile.close();
        }
        av_packet_unref(m_pPkt);
        av_frame_unref(m_pFrame);
        av_frame_unref(m_pFrame_converted);
        cnt++;
    }
}
