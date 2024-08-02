#include <QPixmap>
#include <QString>
#include <QIcon>
#include <QtConcurrent/QtConcurrent>
#include "mainwindow.h"
#include "myqttreewidget.h"
#include "./ui_mainwindow.h"
#include <QDateTime>
#include <ostream>



#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{


    ui->setupUi(this);

    ui->treeWidget_2->setColumnCount(1);
    QIcon icon_DVR = QIcon::fromTheme("oxygen", QIcon("D:/Source/Repos/EXERCISE/EnhancedPlayer/artifacts/server-database.png"));
    QIcon icon_cam = QIcon::fromTheme("oxygen", QIcon("D:/Source/Repos/EXERCISE/EnhancedPlayer/artifacts/digikam.png"));

    connect(ui->graphicsView1, &custom_view::framePts, this, &MainWindow::jitterPlot);


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

    chart = new QChart();
    series = new QLineSeries();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle("line chart");

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    ui->formLayout->addWidget(chartView);


    std::ostringstream response;

    try{
        curlpp::Easy myRequest;
        myRequest.setOpt<curlpp::options::Url>("http://10.31.7.78:9997/v3/paths/list");
        myRequest.setOpt<curlpp::options::UserPwd>("stefano:stefano");
        myRequest.setOpt<curlpp::options::WriteStream>(&response);


        myRequest.perform();
    } catch (curlpp::RuntimeError & e) {
        std::cout << e.what() << std::endl;
    } catch (curlpp::LogicError & e) {
        std::cout << e.what() << std::endl;
    }

    QByteArray ba(response.str().data(), response.str().size());
    QJsonDocument json = QJsonDocument::fromJson(ba);

    qDebug() << json;


}


MainWindow::~MainWindow()
{
    qDebug() << "Disposing UI and threads";
    delete ui;
}





void MainWindow::jitterPlot(int64_t pts)
{
    // qDebug() << "Add point";

    // series->append(QDateTime::currentDateTime().toSecsSinceEpoch(), pts);
    // chart->setR
    // ui->formLayout->update();

}
