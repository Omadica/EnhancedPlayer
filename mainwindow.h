#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrent>
#include <QChart>
#include <QChartView>
#include <QScatterSeries>
#include <memory.h>
#include <chrono>
#include "custom_view.h"
#include "LoggerService.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Ui::MainWindow *ui;

signals:
    void sendUrlAndToken(std::string url, std::string token);
    void sendAuthInternal(std::string authMethod, std::string url, std::string intUser, std::string intPass);

protected slots:
    void jitterPlot(int64_t pts);

    void connetToRecorder();
    void refreshToken();
    void logOutRevokeToken();

    void getTopology(); // used also for refreshing
    void setAuthMethod();
    void showWorkingThreads();

    void setStreamline(const std::string& path);

//public slots:
//    void stopBtnConnection();

private:
    QChartView *chartView;
    QScatterSeries *series;
    QChart *chart;

    QString token;
    QString expireIn;
    QString refExpireIn;
    QString refToken;
    QString sesssionStat;

    std::chrono::system_clock timer;
    std::chrono::system_clock timer2;
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point stopTime;
    std::chrono::system_clock::time_point startAbsTime;

    std::string authMethod;
    std::string intUser;
    std::string intPass;

    long pts0 {0};

};
#endif // MAINWINDOW_H
