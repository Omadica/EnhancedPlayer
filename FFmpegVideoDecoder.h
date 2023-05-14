#ifndef FFMPEGVIDEODECODER_H
#define FFMPEGVIDEODECODER_H

#include <QObject>
#include <QImage>
#include "NvCodecUtils.h"
#include "NvDecoder.h"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>

// for HW decoding
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
}

class FFmpegVideoDecoder : public QObject
{
    Q_OBJECT
public:
    explicit FFmpegVideoDecoder(QObject *parent = nullptr, QString rtsp_addr = "", bool hw_accel = false, bool nv_dev = false, QString HWdec_name = "dxva2");
    virtual ~FFmpegVideoDecoder() = default;

public slots:
    void decode();

signals:
    void finished();
    void ReturnFrame(QImage img);
    void error(QString error);
    void infoDec(QString dec);

private:
    /**
     * @brief m_pImg_conversion sws_context for color range conversion
     */
    SwsContext *m_pImg_conversion;

    /**
     * @brief Software decoding
     */
    QString rtsp_addr;
    AVCodecContext* m_pCctx;
    const AVCodec* codec;
    AVFormatContext* m_pIc;
    AVStream* m_pStream;
    AVPacket* m_pPkt;
    AVFrame* m_pFrame;
    AVFrame* m_pFrame_converted;

    /**
     * @brief Hardware decoding
     */
    enum AVHWDeviceType type;
    // format context (ic)
    // av stream
    // codecContext
    // codec (represents the decoder)
    // pkt and frame
    AVFrame* m_pSWFrame;
    AVFrame* m_pOutFrame;
    bool bool_hw_accel;
    bool nv_hw_dev;
    NvDecoder* nv_dec;
    CUcontext* cuContext;
    // Hardware color range conversion
    SwsContext *m_pHWconversion;
    QString HWDec_name;



    /**
     * @brief m_pLastFrame final QImage that will be send from the decoder to the main thread
     */
    QImage m_pLastFrame;
};

#endif // FFMPEGVIDEODECODER_H
