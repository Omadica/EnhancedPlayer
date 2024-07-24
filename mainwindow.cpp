#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <QCameraDevice>
#include <QListView>
#include <QPixmap>
#include <QIcon>
#include <QPainter>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include "mainwindow.h"
#include "myqttreewidget.h"
#include "./ui_mainwindow.h"
#include "qaudiodevice.h"
#include <iostream>
#include <QString>
#include <unordered_map>
#include <QGraphicsPixmapItem>
#include <TaskManager.h>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtConcurrent/QtConcurrent>
#include <memory>
static std::shared_ptr<TaskManager::ThreadPool> threadpool;
static std::shared_ptr<TaskManager::Scheduler> scheduler;

static QThreadPool qtPool;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    const spdlog::level::level_enum log_level = spdlog::level::debug;
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("LiveJob.log", true);

    console_sink->set_level(log_level);
    file_sink->set_level(log_level);
    std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink};
    auto m_logger = std::make_shared<spdlog::logger>("NativeLog", begin(sinks), end(sinks));
    m_logger->set_level(log_level);
    m_logger->enable_backtrace(32);
    // spdlog::register_logger(m_logger);

    MediaWrapper::AV::init();
    size_t nthreads = std::thread::hardware_concurrency();
    threadpool = std::make_shared<TaskManager::ThreadPool>(nthreads);
    scheduler = std::make_shared<TaskManager::Scheduler>(threadpool, nthreads*20);


    ui->setupUi(this);

    if(!ui->graphicsView1->acceptDrops()){
        qDebug() << "Enabled AcceptDrops";
        ui->graphicsView1->setAcceptDrops(true);
    }

    scene1 = new QGraphicsScene(this);
    ui->graphicsView1->setScene(scene1);



    if(!ui->graphicsView2->acceptDrops()){
        qDebug() << "Enabled AcceptDrops";
        ui->graphicsView2->setAcceptDrops(true);
    }

    scene2 = new QGraphicsScene(this);
    ui->graphicsView2->setScene(scene2);




    if(!ui->graphicsView3->acceptDrops()){
        qDebug() << "Enabled AcceptDrops";
        ui->graphicsView3->setAcceptDrops(true);
    }

    scene3 = new QGraphicsScene(this);
    ui->graphicsView3->setScene(scene3);



    if(!ui->graphicsView4->acceptDrops()){
        qDebug() << "Enabled AcceptDrops";
        ui->graphicsView4->setAcceptDrops(true);
    }

    scene4 = new QGraphicsScene(this);
    ui->graphicsView4->setScene(scene4);



    ui->treeWidget_2->setColumnCount(1);
    QIcon icon_DVR = QIcon::fromTheme("oxygen", QIcon("D:/Source/Repos/EXERCISE/EnhancedPlayer/artifacts/server-database.png"));
    QIcon icon_cam = QIcon::fromTheme("oxygen", QIcon("D:/Source/Repos/EXERCISE/EnhancedPlayer/artifacts/digikam.png"));


    QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->treeWidget_2);
    treeItem->setIcon(0, icon_DVR);
    treeItem->setText(0, "DVR2");

    const std::string camerasSubnet7[2] = {"10.31.7.74", "10.31.7.79"};

    for(auto &it : camerasSubnet7){
        QTreeWidgetItem *cameraItem = new QTreeWidgetItem(treeItem);
        cameraItem->setIcon(0, icon_cam);
        cameraItem->setText(0, it.c_str());

        treeItem->addChild(cameraItem);
    }

    QTreeWidgetItem *treeItem2 = new QTreeWidgetItem(ui->treeWidget_2);
    treeItem2->setIcon(0, icon_DVR);
    treeItem2->setText(0, "DVR1");

    const std::string camerasSubnet4[3] = {"10.31.4.99", "10.31.4.127", "10.31.4."};

    for( auto &it : camerasSubnet4){
        QTreeWidgetItem *cameraItem = new QTreeWidgetItem(treeItem2);
        cameraItem->setIcon(0, icon_cam);
        cameraItem->setText(0, it.c_str());
        treeItem2->addChild(cameraItem);
    }

    connect(ui->graphicsView1, &custom_view::callVideo, this, &MainWindow::playVideo1);
    connect(ui->graphicsView2, &custom_view::callVideo, this, &MainWindow::playVideo2);
    connect(ui->graphicsView3, &custom_view::callVideo, this, &MainWindow::playVideo3);
    connect(ui->graphicsView4, &custom_view::callVideo, this, &MainWindow::playVideo4);
    connect(this, &MainWindow::frameRGB1, this, &MainWindow::drawFrame1);
    connect(this, &MainWindow::frameRGB2, this, &MainWindow::drawFrame2);
    connect(this, &MainWindow::frameRGB3, this, &MainWindow::drawFrame3);
    connect(this, &MainWindow::frameRGB4, this, &MainWindow::drawFrame4);

    treeItem->setExpanded(true);
    treeItem2->setExpanded(true);

}

void MainWindow::drawFrame1(QImage img)
{

        scene1->clear();
        scene1->addPixmap(QPixmap::fromImage(img));




}

void MainWindow::drawFrame2(QImage img)
{

        scene2->clear();
        scene2->addPixmap(QPixmap::fromImage(img));


}


void MainWindow::drawFrame3(QImage img)
{

        scene3->clear();
        scene3->addPixmap(QPixmap::fromImage(img));


}


void MainWindow::drawFrame4(QImage img)
{

        scene4->clear();
        scene4->addPixmap(QPixmap::fromImage(img));


}


void MainWindow::playVideo1(const QString url)
{

    QFuture<void> fut = QtConcurrent::run([=] {

        callback = [&](MediaWrapper::AV::VideoFrame* frame) {
            qDebug() << "Callback called";
            QImage lastFramepp = QImage(frame->width(), frame->height(), QImage::Format_RGB888);;
            for(int y=0; y < frame->height(); y++)
                memcpy(
                    lastFramepp.scanLine(y),
                    frame->raw()->data[0] + y * frame->raw()->linesize[0],
                    frame->raw()->width*3
                    );


            emit frameRGB1(lastFramepp);
        };


        qDebug() << "Reproducing video from " << url;
        const std::string URL = "rtsp://admin:admin@" + url.toStdString() + "/stream2";
        qDebug() << "Reproducing video from " << URL ;
        auto context = std::make_unique<TaskProcessor::ProcessorContext>(URL);

        auto fut = scheduler->scheduleLambda("LiveJob"+URL, [&]() {

            auto job = std::make_unique<TaskProcessor::LiveStream>(context->GetURL(), scheduler);
            context->set_processor(std::move(job));
            context->initializeProcessorContext();
            context->readAndDecode(callback);

        });
        fut.wait();
    });

}

void MainWindow::playVideo2(const QString url)
{

    QFuture<void> fut = QtConcurrent::run([=] {

        callback = [&](MediaWrapper::AV::VideoFrame* frame) {
            qDebug() << "Callback called";
            QImage lastFramepp = QImage(frame->width(), frame->height(), QImage::Format_RGB888);;
            for(int y=0; y < frame->height(); y++)
                memcpy(
                    lastFramepp.scanLine(y),
                    frame->raw()->data[0] + y * frame->raw()->linesize[0],
                    frame->raw()->width*3
                    );


            emit frameRGB2(lastFramepp);
        };


        qDebug() << "Reproducing video from " << url;
        const std::string URL = "rtsp://admin:admin@" + url.toStdString() + "/stream2";
        qDebug() << "Reproducing video from " << URL ;
        auto context = std::make_unique<TaskProcessor::ProcessorContext>(URL);

        auto fut = scheduler->scheduleLambda("LiveJob"+URL, [&]() {

            auto job = std::make_unique<TaskProcessor::LiveStream>(context->GetURL(), scheduler);
            context->set_processor(std::move(job));
            context->initializeProcessorContext();
            context->readAndDecode(callback);

        });
        fut.wait();
    });

}

void MainWindow::playVideo3(const QString url)
{

    QFuture<void> fut = QtConcurrent::run([=] {

        callback = [&](MediaWrapper::AV::VideoFrame* frame) {
            qDebug() << "Callback called";
            QImage lastFramepp = QImage(frame->width(), frame->height(), QImage::Format_RGB888);;
            for(int y=0; y < frame->height(); y++)
                memcpy(
                    lastFramepp.scanLine(y),
                    frame->raw()->data[0] + y * frame->raw()->linesize[0],
                    frame->raw()->width*3
                    );


            emit frameRGB3(lastFramepp);
        };


        qDebug() << "Reproducing video from " << url;
        const std::string URL = "rtsp://admin:admin@" + url.toStdString() + "/stream2";
        qDebug() << "Reproducing video from " << URL ;
        auto context = std::make_unique<TaskProcessor::ProcessorContext>(URL);

        auto fut = scheduler->scheduleLambda("LiveJob"+URL, [&]() {

            auto job = std::make_unique<TaskProcessor::LiveStream>(context->GetURL(), scheduler);
            context->set_processor(std::move(job));
            context->initializeProcessorContext();
            context->readAndDecode(callback);

        });
        fut.wait();
    });


}


void MainWindow::playVideo4(const QString url)
{

    QFuture<void> fut = QtConcurrent::run([=] {

        callback = [&](MediaWrapper::AV::VideoFrame* frame) {
            qDebug() << "Callback called";
            QImage lastFramepp = QImage(frame->width(), frame->height(), QImage::Format_RGB888);;
            for(int y=0; y < frame->height(); y++)
                memcpy(
                    lastFramepp.scanLine(y),
                    frame->raw()->data[0] + y * frame->raw()->linesize[0],
                    frame->raw()->width*3
                    );


            emit frameRGB4(lastFramepp);
        };


        qDebug() << "Reproducing video from " << url;
        const std::string URL = "rtsp://admin:admin@" + url.toStdString() + "/stream2";
        qDebug() << "Reproducing video from " << URL ;
        auto context = std::make_unique<TaskProcessor::ProcessorContext>(URL);

        auto fut = scheduler->scheduleLambda("LiveJob"+URL, [&]() {

            auto job = std::make_unique<TaskProcessor::LiveStream>(context->GetURL(), scheduler);
            context->set_processor(std::move(job));
            context->initializeProcessorContext();
            context->readAndDecode(callback);

        });
        fut.wait();
    });
}


MainWindow::~MainWindow()
{
    qDebug() << "Disposing UI and threads";
    threadpool->dispose();
    threadpool.reset();
    delete ui;
}

