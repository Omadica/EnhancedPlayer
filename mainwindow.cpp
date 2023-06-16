#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <QCameraDevice>
#include <QListView>
#include <QPixmap>
#include <QIcon>
#include <QTimer>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "qaudiodevice.h"
#include <iostream>
#include <QString>
#include <unordered_map>
#include <QGraphicsPixmapItem>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include "FFmpegLog.h"
#include <cuda.h>
#include <cuda_runtime.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavcodec/bsf.h>
#include <libavfilter/avfilter.h>
}

std::unordered_map<AVCodecID, QString> Supported_codec = {
    {AV_CODEC_ID_HEVC, QString("HEVC(H265)")},
    {AV_CODEC_ID_H264, QString("AV1(H264)")},
    {AV_CODEC_ID_MJPEG, QString("MJPEG")},
};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    /**
     * @brief Catch the FFmpeg log and put them on std::out
     */
    // my_libav *av_log = new my_libav();

    /**
     * @brief Set-up the user interface
     */
    ui->setupUi(this);

    /**
     * check Nvidia device(s) (The device search has to be improved)
     */
    int nGpu = 0;
    cuDeviceGetCount(&nGpu);

    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, 0);
    ui->deviceNames->addItem(QString(prop.name));



    /**
     * @brief Create QmediaDev and search for local cams
     */
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

    QIcon p = QIcon(QPixmap::fromImage(QImage(16, 16 , QImage::Format_RGB888)));
    this->setWindowIcon(p);

    /**/
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    numPic = 1;
    namePic = QString("Chessboard_") + QString::number(numPic) + ".png";
    ui->textSave->setText(namePic);

    /**
     * @brief Rtsp connection, check if the URI is available
     */
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(RtspConnection()));
    connect(ui->btnTakePicture, SIGNAL(clicked(bool)), SLOT(TakePicture()));
    connect(ui->btnPlayback, SIGNAL(clicked()), this,  SLOT(StartPlayback()));
    connect(ui->checkBox, SIGNAL(clicked()), this, SLOT(loadDecoders()));
    connect(ui->btnStop, SIGNAL(clicked()), this, SLOT(resetDecoder()));

//    ZerTrans = new ZernikeTransform();
//    ZerTrans->transformFrame();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadDecoders()
{
    const AVCodec * codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);

    for (int i = 0;; i++) {
        const AVCodecHWConfig *config_hevc = avcodec_get_hw_config(codec, i);
        if(!config_hevc)
            break;
        ui->comboBox->addItem(QString(av_hwdevice_get_type_name(config_hevc->device_type)));
    }
}

void MainWindow::resetDecoder()
{
    decoder->deleteLater();
    emit stopDecodingThread();

}

void MainWindow::TakePicture()
{
    m_FrameImage.save(namePic);
    numPic++;
    namePic = QString("Chessboard_") + QString::number(numPic) + ".png";
    ui->textSave->setText(namePic);
}

void MainWindow::DrawGraph(QImage img)
{
    
    cv::Size imgSize = cv::Size(img.width(), img.height());

    m_FrameImage = img;
    scene->clear();
    scene->addPixmap(QPixmap::fromImage(img));
}

void MainWindow::PrintDecoderInfo(QString dec)
{
    ui->label_17->setText(dec);
}

void MainWindow::RtspConnection()
{
    int ret = 0;
    m_pFormatContext = avformat_alloc_context();


    rtsp_addr = "rtsp://" + ui->textUser->text() + ":" + ui->textPasswd->text() + "@" + ui->textIP->text() + ui->textOptions->text();
    ret = avformat_open_input(&m_pFormatContext, rtsp_addr.toStdString().c_str(), NULL, NULL);


    if (ret < 0)
        qDebug() << "Error to create AvFormatContext";
    else
        ui->label_6->setText(QString("connected"));


    ret = avformat_find_stream_info(m_pFormatContext, NULL);
    if(ret < 0)
        qDebug() << "Error to find stream";

    int nVideoStream = 0;
    int nAudioStream = 0;

    for(unsigned int i=0; i < m_pFormatContext->nb_streams; i++)
    {
        if (m_pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            ui->label_10->setText(QString::number(m_pFormatContext->streams[i]->codecpar->width) + "X" + QString::number(m_pFormatContext->streams[i]->codecpar->height));
            auto codec = Supported_codec.find(m_pFormatContext->streams[i]->codecpar->codec_id);
            ui->label_11->setText(codec->second);
            ui->label_12->setText(QString::number(m_pFormatContext->streams[i]->codecpar->format));
            ui->label_14->setText(QString::number(m_pFormatContext->streams[i]->r_frame_rate.num) + "/" + QString::number(m_pFormatContext->streams[i]->r_frame_rate.den));

            nVideoStream = i;
        }
        if (m_pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            nAudioStream = i;

        if(nVideoStream == -1 || nAudioStream == -1)
            qDebug() << "Warning: Video = " << nVideoStream << "  " << "Audio = " << nAudioStream;
    }

}

void MainWindow::StartPlayback()
{
    QThread *thread = new QThread();;
    bool hw_dec = ui->checkBox->isChecked();
    bool nvidia_devices = ui->deviceNames->currentText() != "" ? true : false;

    QString HWdecoder_name = ui->comboBox->currentText();
    qDebug() << HWdecoder_name.toStdString().c_str();

    decoder = new FFmpegVideoDecoder(nullptr, rtsp_addr, hw_dec, nvidia_devices, HWdecoder_name);
    decoder->moveToThread(thread);
    connect(thread, &QThread::started, decoder, &FFmpegVideoDecoder::decode);
    connect(decoder, SIGNAL(ReturnFrame(QImage)), this, SLOT(DrawGraph(QImage)));
    connect(this, SIGNAL(stopDecodingThread()), thread, SLOT(quit()));

    thread->start();

}

