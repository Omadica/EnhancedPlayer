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
#include <QtConcurrent/QtConcurrent>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{

    size_t nthreads = std::thread::hardware_concurrency();
    ui->setupUi(this);

    ui->treeWidget_2->setColumnCount(1);
    QIcon icon_DVR = QIcon::fromTheme("oxygen", QIcon("../artifacts/oxygen-icons/16x16/places/server-database.png"));
    QIcon icon_cam = QIcon::fromTheme("oxygen", QIcon("../artifacts/oxygen-icons/16x16/apps/digikam.png"));


    QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->treeWidget_2);
    treeItem->setIcon(0, icon_DVR);
    treeItem->setText(0, "DVR1");

    for(int i =0; i<3; i++){
        QTreeWidgetItem *cameraItem = new QTreeWidgetItem(treeItem);
        cameraItem->setIcon(0, icon_cam);
        cameraItem->setText(0, "Camera");
        treeItem->addChild(cameraItem);
    }

    QTreeWidgetItem *treeItem2 = new QTreeWidgetItem(ui->treeWidget_2);
    treeItem2->setIcon(0, icon_DVR);
    treeItem2->setText(0, "DVR2");

    for(int i =0; i<5; i++){
        QTreeWidgetItem *cameraItem = new QTreeWidgetItem(treeItem2);
        cameraItem->setIcon(0, icon_cam);
        cameraItem->setText(0, "Camera");
        treeItem2->addChild(cameraItem);
    }



    treeItem->setExpanded(true);
    treeItem2->setExpanded(true);

}

MainWindow::~MainWindow()
{
    delete ui;
}

