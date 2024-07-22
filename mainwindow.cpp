#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <QCameraDevice>
#include <QListView>
#include <QPixmap>
#include <QIcon>
#include <QTimer>
#include "mainwindow.h"
#include "myqttreewidget.h"
#include "./ui_mainwindow.h"
#include "qaudiodevice.h"
#include <iostream>
#include <QString>
#include <unordered_map>
#include <QGraphicsPixmapItem>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtConcurrent/QtConcurrent>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{

    size_t nthreads = std::thread::hardware_concurrency();
    ui->setupUi(this);

    if(!ui->graphicsView->acceptDrops()){
        qDebug() << "Enabled AcceptDrops";
        ui->graphicsView->setAcceptDrops(true);
    }


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

    const std::string camerasSubnet4[3] = {"10.31.4.142", "10.31.4.146", "10.31.4.127"};

    for( auto &it : camerasSubnet4){
        QTreeWidgetItem *cameraItem = new QTreeWidgetItem(treeItem2);
        cameraItem->setIcon(0, icon_cam);
        cameraItem->setText(0, it.c_str());
        treeItem2->addChild(cameraItem);
    }


    connect(ui->graphicsView, &custom_view::callVideo, this, &MainWindow::playVideo);





    treeItem->setExpanded(true);
    treeItem2->setExpanded(true);

}


void MainWindow::playVideo()
{
    qDebug() << "Reproducing video";
}



MainWindow::~MainWindow()
{
    delete ui;
}

