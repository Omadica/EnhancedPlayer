#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <QCameraDevice>
#include <QListView>
#include <QPixmap>
#include <QIcon>
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

    if(!ui->graphicsView2->acceptDrops()){
        qDebug() << "Enabled AcceptDrops";
        ui->graphicsView1->setAcceptDrops(true);
    }

    if(!ui->graphicsView3->acceptDrops()){
        qDebug() << "Enabled AcceptDrops";
        ui->graphicsView1->setAcceptDrops(true);
    }

    if(!ui->graphicsView4->acceptDrops()){
        qDebug() << "Enabled AcceptDrops";
        ui->graphicsView1->setAcceptDrops(true);
    }

    scene = new QGraphicsScene(this);
    ui->graphicsView1->setScene(scene);



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

    const std::string camerasSubnet4[3] = {"10.31.4.99", "10.31.4.127", "10.31.4.144"};

    for( auto &it : camerasSubnet4){
        QTreeWidgetItem *cameraItem = new QTreeWidgetItem(treeItem2);
        cameraItem->setIcon(0, icon_cam);
        cameraItem->setText(0, it.c_str());
        treeItem2->addChild(cameraItem);
    }

    connect(ui->graphicsView1, &custom_view::callVideo,
            this, &MainWindow::playVideo);
    connect(ui->graphicsView2, &custom_view::callVideo,
            this, &MainWindow::playVideo);
    connect(ui->graphicsView3, &custom_view::callVideo,
            this, &MainWindow::playVideo);
    connect(ui->graphicsView4, &custom_view::callVideo,
            this, &MainWindow::playVideo);

    treeItem->setExpanded(true);
    treeItem2->setExpanded(true);

}

void MainWindow::playVideo(const QString url)
{

    QFuture<void> fut = QtConcurrent::run([=] {


        std::function<void(MediaWrapper::AV::VideoFrame*)> callback = [&](MediaWrapper::AV::VideoFrame* frame) {
            qDebug() << "Callback called";
            std::mutex mutex;
            std::unique_lock<std::mutex> lock(mutex);
            lastFrame = QImage(frame->width(), frame->height(), QImage::Format_RGB888);;
            for(int y=0; y < frame->height(); y++)
                memcpy(
                    lastFrame.scanLine(y),
                    frame->raw()->data[0] + y * frame->raw()->linesize[0],
                    frame->raw()->width*3
                );

            scene->clear();
            scene->addPixmap(QPixmap::fromImage(lastFrame));

        };

        qDebug() << "Reproducing video from " << url;
        const std::string URL = "rtsp://admin:admin@" + url.toStdString() + "/stream1";
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

