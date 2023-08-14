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
//#include "FFmpegLog.h"

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
    connect(ui->graphicsView, SIGNAL(click_on_dewarp()), this, SLOT(openFishEyeWindow()) );

//    ZerTrans = new ZernikeTransform();
//    ZerTrans->transformFrame();

}


void MainWindow::openFishEyeWindow()
{
    m_fisheye = new fisheyeImgConv();

    m_radius = 704;
    ui->horizontalSlider->setValue(m_radius);

    m_cx = 704;
    ui->horizontalSlider_2->setValue(m_cx);

    m_cy = 704;
    ui->horizontalSlider_3->setValue(m_cy);

    m_dx = 0;
    ui->horizontalSlider_4->setValue(m_dx);

    m_dy = 0;
    ui->horizontalSlider_5->setValue(m_dy);

    m_aperture = 135;
    ui->horizontalSlider_6->setValue(m_aperture);

    m_theta = 150;
    ui->horizontalSlider_7->setValue(m_theta);

    m_phi = 135;
    ui->horizontalSlider_8->setValue(m_phi);

    m_bDewarp = true;


    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setRadius()));
    connect(ui->horizontalSlider_2, SIGNAL(valueChanged(int)), this, SLOT(setCx()));
    connect(ui->horizontalSlider_3, SIGNAL(valueChanged(int)), this, SLOT(setCy()));
    connect(ui->horizontalSlider_4, SIGNAL(valueChanged(int)), this, SLOT(setDx()));
    connect(ui->horizontalSlider_5, SIGNAL(valueChanged(int)), this, SLOT(setDy()));
    connect(ui->horizontalSlider_6, SIGNAL(valueChanged(int)), this, SLOT(setAperture()));
    connect(ui->horizontalSlider_7, SIGNAL(valueChanged(int)), this, SLOT(setTheta()));
    connect(ui->horizontalSlider_8, SIGNAL(valueChanged(int)), this, SLOT(setPhi()));


}

void MainWindow::setRadius()
{
    m_radius = ui->horizontalSlider->value();
    std::cout << "Raggio: " << m_radius << std::endl;
}

void MainWindow::setCx()
{
    m_cx = ui->horizontalSlider_2->value();
}

void MainWindow::setCy()
{
    m_cy = ui->horizontalSlider_3->value();
}

void MainWindow::setDx()
{
    m_dx = ui->horizontalSlider_4->value();
}

void MainWindow::setDy()
{
    m_dy = ui->horizontalSlider_5->value();
}

void MainWindow::setAperture()
{
    m_aperture = ui->horizontalSlider_6->value();
}


void MainWindow::setTheta()
{
    m_theta = ui->horizontalSlider_7->value();
}

void MainWindow::setPhi()
{
    m_phi = ui->horizontalSlider_8->value();
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
    m_FrameImage = img;
    if(m_bDewarp)
    {
        cv::Mat cv_img = cv::Mat(img.height(), img.width(), CV_8UC3, (cv::Scalar*)img.scanLine(0));
        m_fisheye->fisheye2equirect(cv_img, cv_img, cv::Size(1408, 1408), m_aperture, m_dx, m_dy, m_radius, false);
        //m_fisheye->equirect2persp(cv_img, cv_img, 120.0, m_theta, m_phi, 1408, 1408);

        QImage img_dewarped = QImage((uchar*) cv_img.data, cv_img.cols, cv_img.rows, cv_img.step, QImage::Format_RGB888);
        cv_img.release();

        scene->clear();
        scene->addPixmap(QPixmap::fromImage(img_dewarped));
    } else {
        scene->clear();
        scene->addPixmap(QPixmap::fromImage(img));
    }




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
    {
        qDebug() << "Error to create AvFormatContext";
        return;
    }
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

