#ifndef FFMPEGVIDEODECODER_H
#define FFMPEGVIDEODECODER_H

#include <QObject>
#include <QImage>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

class FFmpegVideoDecoder : public QObject
{
    Q_OBJECT
public:
    explicit FFmpegVideoDecoder(QObject *parent = nullptr, AVFormatContext* ic = nullptr, AVStream *stream = nullptr);
    virtual ~FFmpegVideoDecoder() = default;

public slots:
    void decode();

signals:
    void finished();
    void ReturnFrame(QImage img);
    void error(QString error);

private:
    AVCodecContext* m_pCctx;
    const AVCodec* codec;
    SwsContext *m_pImg_conversion;

    AVFormatContext* m_pIc;
    AVStream* m_pStream;

    AVPacket* m_pPkt;
    AVFrame* m_pFrame;
    AVFrame* m_pFrame_converted;
    QImage m_pLastFrame;
};

#endif // FFMPEGVIDEODECODER_H
