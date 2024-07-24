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


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{


    ui->setupUi(this);

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

    treeItem->setExpanded(true);
    treeItem2->setExpanded(true);

}


MainWindow::~MainWindow()
{
    qDebug() << "Disposing UI and threads";
    delete ui;
}

