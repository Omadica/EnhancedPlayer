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

using namespace std::chrono_literals;


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{


    ui->setupUi(this);
    ui->lineEdit_3->setEchoMode(QLineEdit::Password);
    ui->treeWidget_2->setColumnCount(1);

    ui->comboBox->setCurrentIndex(-1);

    connect(ui->graphicsView1, &custom_view::framePts, this, &MainWindow::jitterPlot);
    connect(ui->pushButton, &QPushButton::released, this, &MainWindow::connetToRecorder);
    connect(ui->comboBox, &QComboBox::currentIndexChanged, this, &MainWindow::setAuthMethod);

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

}

void MainWindow::setAuthMethod()
{
    authMethod = ui->comboBox->currentText().toStdString();


    qDebug() << authMethod;
}


void MainWindow::connetToRecorder()
{
    if(authMethod == "Json Web Token"){
        connect(this, &MainWindow::sendUrlAndToken, ui->graphicsView1, &custom_view::getUrlAndToken);
        connect(this, &MainWindow::sendUrlAndToken, ui->graphicsView2, &custom_view::getUrlAndToken);
        connect(this, &MainWindow::sendUrlAndToken, ui->graphicsView3, &custom_view::getUrlAndToken);
        connect(this, &MainWindow::sendUrlAndToken, ui->graphicsView4, &custom_view::getUrlAndToken);

        std::ostringstream response;
        try {
            curlpp::Easy authRequest;

            QString Url = QString("http://") + ui->lineEdit->text() + QString(":8080/realms/mediamtx/protocol/openid-connect/token");
            QString Data = QString("client_id=mediamtx&client_secret=l5czaQ0e8XiWfP94ViLDQDK1i0sp8AHY&username=") +
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

        ui->tabWidget->setEnabled(true);
        ui->tab_1->setEnabled(false);
        ui->tabWidget_2->removeTab(0);
        ui->tab_3->setEnabled(true);
        ui->tab_4->setEnabled(true);
        ui->tab_5->setEnabled(true);
        ui->tab_6->setEnabled(true);
        ui->tab_7->setEnabled(true);

        emit sendUrlAndToken(ui->lineEdit->text().toStdString(), token.toStdString());

    } else if (authMethod == "Internal") {
        connect(this, &MainWindow::sendAuthInternal, ui->graphicsView1, &custom_view::getAuthMethod);
        connect(this, &MainWindow::sendAuthInternal, ui->graphicsView2, &custom_view::getAuthMethod);
        connect(this, &MainWindow::sendAuthInternal, ui->graphicsView3, &custom_view::getAuthMethod);
        connect(this, &MainWindow::sendAuthInternal, ui->graphicsView4, &custom_view::getAuthMethod);

        intUser = ui->lineEdit_2->text().toStdString();
        intPass = ui->lineEdit_3->text().toStdString();
        ui->tabWidget->setEnabled(true);
        ui->tab_1->setEnabled(false);
        ui->tabWidget_2->removeTab(0);
        ui->tab_3->setEnabled(true);
        ui->tab_4->setEnabled(true);
        ui->tab_5->setEnabled(true);
        ui->tab_6->setEnabled(true);
        ui->tab_7->setEnabled(true);

        emit sendAuthInternal(authMethod, ui->lineEdit->text().toStdString(), intUser, intPass);

    } else {

        QMessageBox msgBox;
        msgBox.setText("Invalid or not implemented yet.");
        msgBox.exec();
        return;
    }

    getTopology();


    QByteArray ba(response.str().data(), response.str().size());
    QJsonDocument json = QJsonDocument::fromJson(ba);
    QJsonObject jsonObj = json.object();
    token = jsonObj["access_token"].toString();
    expireIn = jsonObj["expires_in"].toString();
    refExpireIn = jsonObj["refresh_expires_in"].toString();
    refToken = jsonObj["refresh_token"].toString();
    sesssionStat = jsonObj["session_state"].toString();

    ui->tabWidget->setEnabled(true);
    ui->tab_1->setEnabled(false);
    ui->tabWidget_2->removeTab(0);
    ui->tab_3->setEnabled(true);
    ui->tab_4->setEnabled(true);
    ui->tab_5->setEnabled(true);
    ui->tab_6->setEnabled(true);
    ui->tab_7->setEnabled(true);

    emit sendUrlAndToken(ui->lineEdit->text().toStdString(), token.toStdString());

    getTopology();


void MainWindow::refreshToken()
{

}

void MainWindow::logOutRevokeToken()
{

}

void MainWindow::getTopology()
{
    QIcon icon_DVR = QIcon::fromTheme("oxygen", QIcon("D:/Source/Repos/EXERCISE/EnhancedPlayer/artifacts/server-database.png"));
    QIcon icon_cam = QIcon::fromTheme("oxygen", QIcon("D:/Source/Repos/EXERCISE/EnhancedPlayer/artifacts/digikam.png"));
    QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->treeWidget_2);
    treeItem->setIcon(0, icon_DVR);
    treeItem->setText(0, ui->lineEdit->text());

    std::vector<std::string> cameras = {};

    std::ostringstream recorderResp;
    curlpp::Easy recorderReq;

    try {

        recorderReq.setOpt<curlpp::options::Url>(QString(("http://")+ui->lineEdit->text()+QString(":9997/v3/paths/list?jwt=")+token).toStdString());
        recorderReq.setOpt<curlpp::options::WriteStream>(&recorderResp);

        recorderReq.perform();

    } catch (...) {
        QMessageBox msgBox;
        msgBox.setText("GetTopology failed.");
        msgBox.exec();
        return;
    }


    QByteArray ba(recorderResp.str().data(), recorderResp.str().size());
    QJsonDocument json = QJsonDocument::fromJson(ba);
    QJsonObject jsonObj = json.object();

    for (const QJsonValue& c : jsonObj["items"].toArray()){
        qDebug() << c["name"].toString();
        cameras.push_back(c["name"].toString().toStdString());
    }


    for(auto &it : cameras){
        QTreeWidgetItem *cameraItem = new QTreeWidgetItem(treeItem);
        cameraItem->setIcon(0, icon_cam);
        cameraItem->setText(0, it.c_str());
        treeItem->addChild(cameraItem);
    }

    treeItem->setExpanded(true);
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

    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime);
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
