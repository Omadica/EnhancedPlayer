#include <memory.h>
#include <QPixmap>
#include <QString>
#include <QIcon>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>
#include <QLineEdit>
#include "mainwindow.h"
#include "myqttreewidget.h"
#include "./ui_mainwindow.h"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

extern std::shared_ptr<TaskManager::ThreadPool> threadpool;
extern std::shared_ptr<TaskManager::Scheduler> scheduler;


using namespace std::chrono_literals;


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{


    ui->setupUi(this);
    ui->lineEdit_3->setEchoMode(QLineEdit::Password);
    ui->treeWidget_2->setColumnCount(1);

    ui->comboBox->setCurrentIndex(-1);
    QIcon iconFF = QIcon::fromTheme("oxygen", QIcon("MediaControl/media-seek-forward.png"));
    QIcon iconBF = QIcon::fromTheme("oxygen", QIcon("MediaControl/media-seek-backward.png"));
    QIcon iconStop = QIcon::fromTheme("oxygen", QIcon("MediaControl/media-playback-stop.png"));
    QIcon iconPlay = QIcon::fromTheme("oxygen", QIcon("MediaControl/media-playback-start.png"));

    ui->stopBtn->setIcon(iconStop);
    ui->playBtn->setIcon(iconPlay);
    ui->fastForBtn->setIcon(iconFF);
    ui->backForBtn->setIcon(iconBF);

    connect(ui->pushButton, &QPushButton::released, this, &MainWindow::connetToRecorder);
    connect(ui->comboBox, &QComboBox::currentIndexChanged, this, &MainWindow::setAuthMethod);

    // Start the application with no hooks.
    connect(ui->stopBtn, &QPushButton::released, ui->graphicsView1, &custom_view::stopLive, Qt::DirectConnection);
    connect(ui->stopBtn, &QPushButton::released, ui->graphicsView2, &custom_view::stopLive, Qt::DirectConnection);
    connect(ui->stopBtn, &QPushButton::released, ui->graphicsView3, &custom_view::stopLive, Qt::DirectConnection);
    connect(ui->stopBtn, &QPushButton::released, ui->graphicsView4, &custom_view::stopLive, Qt::DirectConnection);
    connect(ui->graphicsView1, &custom_view::getRecordings, this, &MainWindow::setStreamline);

    chart = new QChart();
    series = new QScatterSeries();
    chart->legend()->hide();

    chart->addSeries(series);

    chart->createDefaultAxes();
    chart->setTitle("line chart");

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    ui->formLayout->addWidget(chartView);

    startTime = timer.now();
    startAbsTime = timer2.now();

    size_t nthreads = std::thread::hardware_concurrency();
    threadpool = std::make_shared<TaskManager::ThreadPool>(nthreads);
    scheduler = std::make_shared<TaskManager::Scheduler>(threadpool, nthreads);

}


void MainWindow::setAuthMethod()
{
    authMethod = ui->comboBox->currentText().toStdString();
    qDebug() << authMethod;
}

void MainWindow::connetToRecorder()
{
    std::ostringstream response;
    if(authMethod == "Json Web Token"){
        connect(this, &MainWindow::sendUrlAndToken, ui->graphicsView1, &custom_view::getUrlAndToken);
        connect(this, &MainWindow::sendUrlAndToken, ui->graphicsView2, &custom_view::getUrlAndToken);
        connect(this, &MainWindow::sendUrlAndToken, ui->graphicsView3, &custom_view::getUrlAndToken);
        connect(this, &MainWindow::sendUrlAndToken, ui->graphicsView4, &custom_view::getUrlAndToken);

        try {
            curlpp::Easy authRequest;

            QString Url = QString("http://") + ui->lineEdit->text() + QString(":8080/realms/mediamtx/protocol/openid-connect/token");
            QString Data = QString("client_id=mediamtx&client_secret=0VEOXcpiagJqr09xh7MLpoF6lVDtTq8K&username=") +
                           ui->lineEdit_2->text() + QString("&password=") +
                           ui->lineEdit_3->text() +
                           QString("&grant_type=password");

            authRequest.setOpt<curlpp::options::Url>(Url.toStdString());
            authRequest.setOpt<curlpp::options::PostFields>(Data.toStdString());
            authRequest.setOpt<curlpp::options::WriteStream>(&response);

            authRequest.perform();

        } catch (...) {
            QMessageBox msgBox;
            msgBox.setText("Authentication failed.");
            msgBox.exec();
            return;
        }


        QByteArray ba(response.str().data(), response.str().size());
        QJsonDocument json = QJsonDocument::fromJson(ba);
        QJsonObject jsonObj = json.object();
        token = jsonObj["access_token"].toString();
        expireIn = jsonObj["expires_in"].toString();
        refExpireIn = jsonObj["refresh_expires_in"].toString();
        refToken = jsonObj["refresh_token"].toString();
        sesssionStat = jsonObj["session_state"].toString();

        emit sendUrlAndToken(ui->lineEdit->text().toStdString(), token.toStdString());

    } else if (authMethod == "Internal") {
        connect(this, &MainWindow::sendAuthInternal, ui->graphicsView1, &custom_view::getAuthMethod);
        connect(this, &MainWindow::sendAuthInternal, ui->graphicsView2, &custom_view::getAuthMethod);
        connect(this, &MainWindow::sendAuthInternal, ui->graphicsView3, &custom_view::getAuthMethod);
        connect(this, &MainWindow::sendAuthInternal, ui->graphicsView4, &custom_view::getAuthMethod);

        intUser = ui->lineEdit_2->text().toStdString();
        intPass = ui->lineEdit_3->text().toStdString();

        emit sendAuthInternal(authMethod, ui->lineEdit->text().toStdString(), intUser, intPass);

    } else {

        QMessageBox msgBox;
        msgBox.setText("Invalid or not implemented yet.");
        msgBox.exec();
        return;
    }

    getTopology();

}

void MainWindow::refreshToken()
{

}

void MainWindow::logOutRevokeToken()
{

}

void MainWindow::showWorkingThreads()
{
    ui->wthreads->setText("Working Threads: ");
}

void MainWindow::getTopology()
{
    QIcon icon_DVR = QIcon::fromTheme("oxygen", QIcon("Devices/server-database.png"));
    QIcon icon_cam = QIcon::fromTheme("oxygen", QIcon("Devices/digikam.png"));
    QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->treeWidget_2);
    QTreeWidgetItem *treeItem2 = new QTreeWidgetItem(ui->treeWidget);
    treeItem->setIcon(0, icon_DVR);
    treeItem->setText(0, ui->lineEdit->text());

    treeItem2->setIcon(0, icon_DVR);
    treeItem2->setText(0, ui->lineEdit->text());

    std::vector<std::string> cameras = {};

    std::ostringstream recorderResp;
    curlpp::Easy recorderReq;

    try {
        if(authMethod == "Json Web Token")
            recorderReq.setOpt<curlpp::options::Url>(
                QString(QString("https://")+
                        ui->lineEdit->text()+
                        QString(":9997/v3/paths/list?jwt=")+token
                ).toStdString()
            );
        else if (authMethod == "Internal")
            recorderReq.setOpt<curlpp::options::Url>(
                QString(QString("https://")+
                        QString(intUser.c_str())+
                        QString(":")+
                        QString(intPass.c_str())+
                        QString("@")+
                        ui->lineEdit->text()+
                        QString(":9997/v3/paths/list?jwt=")+token
                ).toStdString()
            );


        recorderReq.setOpt<curlpp::options::SslVerifyPeer>(false);
        recorderReq.setOpt<curlpp::options::SslVerifyHost>(0);
        recorderReq.setOpt<curlpp::options::WriteStream>(&recorderResp);

        recorderReq.perform();

    } catch (std::exception &ex) {
        QMessageBox msgBox;
        msgBox.setText(ex.what());
        msgBox.exec();
        return;
    }

    ui->tab_1->setEnabled(false);
    ui->tabWidget_2->removeTab(0);
    ui->tab_3->setEnabled(true);
    ui->tab_4->setEnabled(true);
    ui->tab_5->setEnabled(true);
    ui->tab_6->setEnabled(true);
    ui->tab_7->setEnabled(true);


    QByteArray ba(recorderResp.str().data(), recorderResp.str().size());
    QJsonDocument json = QJsonDocument::fromJson(ba);
    QJsonObject jsonObj = json.object();

    for (const QJsonValue& c : jsonObj["items"].toArray()){
        qDebug() << c["name"].toString();
        cameras.push_back(c["name"].toString().toStdString());
        qDebug() << c["name"].toString();
    }


    for(auto &it : cameras){
        QTreeWidgetItem *cameraItem = new QTreeWidgetItem(treeItem);
        QTreeWidgetItem *cameraItem2 = new QTreeWidgetItem(treeItem2);
        cameraItem->setIcon(0, icon_cam);
        cameraItem->setText(0, it.c_str());
        cameraItem2->setIcon(0, icon_cam);
        cameraItem2->setText(0, it.c_str());
        treeItem->addChild(cameraItem);
        treeItem2->addChild(cameraItem2);
    }

    treeItem->setExpanded(true);
    treeItem2->setExpanded(true);
}

void MainWindow::setStreamline(const std::string& path)
{
    std::ostringstream streamLines;
    curlpp::Easy getRocordSegments;
    getRocordSegments.setOpt<curlpp::options::Url>(
        QString(QString("http://")+
                ui->lineEdit->text()+
                QString(":9996/list?path=") + QString(path.c_str()) + QString("&jwt=")+token
                ).toStdString()
        );

    getRocordSegments.setOpt<curlpp::options::WriteStream>(&streamLines);
    getRocordSegments.perform();
    QByteArray ba(streamLines.str().data(), streamLines.str().size());
    QJsonDocument json = QJsonDocument::fromJson(ba);
    QJsonArray jsonObj = json.array();
    for (const QJsonValue& c : jsonObj){
        auto date = QDateTime::fromString(c["start"].toString(), Qt::ISODate);
        qDebug() << c["duration"].toDouble();
        qDebug() << "Start" << c["start"].toString() << " Stop:" << date.addSecs(c["duration"].toDouble());
    }

    /*
     * Create a resizable timelince
     */
}

MainWindow::~MainWindow()
{

    qDebug() << "Disposing UI and threads";
    delete ui;
}


void MainWindow::jitterPlot(int64_t pts)
{
    qDebug() << "Add point";

    stopTime = timer.now();
    auto t2 = timer2.now();

    // auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime);
    auto t = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(t2-startAbsTime).count());
    pts = pts/100;
    auto jit = pts-pts0;

    series->append(static_cast<double>(t), (1000/jit));

    chart->axisY()->setRange(0, 60);
    chart->axisX()->setRange(series->points().begin()->x(), t);
    chartView->update();
    ui->formLayout->update();

    if(series->points().count() >= 500){
        series->clear();
    }

    pts0 = pts;
    startTime=stopTime;
}
