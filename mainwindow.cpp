#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <QCameraDevice>
#include <QListView>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "qaudiodevice.h"
#include <iostream>
#include <QString>
#include "FFmpegLog.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavcodec/bsf.h>
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{

    my_libav *av_log = new my_libav();


    ui->setupUi(this);
    QMediaDevices devs = new QMediaDevices();
    QList<QCameraDevice> cams = devs.videoInputs(); // video input has member "last"
    QList<QAudioDevice> audio = devs.audioInputs();

    auto col = 0;
    for (const auto &it : cams){
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, it.description());

        auto res_max = it.photoResolutions().first();
        auto res_min = it.photoResolutions().last();
        item->setText(1, QString::number(res_max.width()) + "X" + QString::number(res_max.height())) ;
    }

    av_log_set_level(AV_LOG_INFO);
    AVFormatContext* ic = nullptr;
    avformat_open_input(&ic, "rtsp://admin:Heisenberg@192.168.1.172", NULL, NULL);
    avformat_find_stream_info(ic, NULL);
    int iVideoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    av_read_play(ic);

    std::cout << iVideoStream << std::endl;

    AVCodecID eVideoCodec = ic->streams[iVideoStream]->codecpar->codec_id;
    int nWidth = ic->streams[iVideoStream]->codecpar->width;
    int nHeight = ic->streams[iVideoStream]->codecpar->height;
    enum AVPixelFormat eChromaFormat = (AVPixelFormat)ic->streams[iVideoStream]->codecpar->format;
    AVRational rTimeBase = ic->streams[iVideoStream]->time_base;
    double timeBase = av_q2d(rTimeBase);
    int nBitRate = ic->streams[iVideoStream]->codecpar->bit_rate;

    AVBSFContext *bsfc = NULL;
    const AVBitStreamFilter *bsf = av_bsf_get_by_name("hevc_mp4toannexb");
    av_bsf_alloc(bsf, &bsfc);
    avcodec_parameters_copy(bsfc->par_in, ic->streams[iVideoStream]->codecpar);
    av_bsf_init(bsfc);

    int e = 0;
    AVPacket pkt;
    AVPacket pktFiltered;
    uint8_t ** ppVideo = nullptr;
    int *pnVideoBytes = nullptr;
    int64_t *pst = nullptr;
    while ((e = av_read_frame(ic, &pkt)) >= 0 && pkt.stream_index != iVideoStream) {
        av_packet_unref(&pkt);
    }

    av_bsf_send_packet(bsfc, &pkt);
    av_bsf_receive_packet(bsfc, &pktFiltered);


    AVFrame *frame = nullptr;
    // pktFiltered (is the packet)
    const AVCodec *dec = avcodec_find_decoder(eVideoCodec);
    AVCodecContext *ctx = avcodec_alloc_context3(dec);

    avcodec_open2(ctx, dec, NULL);
    avcodec_send_packet(ctx, &pktFiltered);
    int ret = 0;
    while(av_read_frame(ic,&pkt)>=0 ){
        ret = avcodec_receive_frame(ctx, frame);
        if (ret == 0 || ret == AVERROR(EAGAIN))
            break;
    }



}

MainWindow::~MainWindow()
{
    delete ui;
}
