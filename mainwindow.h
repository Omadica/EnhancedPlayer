#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrent>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <memory.h>
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

protected slots:
    void jitterPlot(int64_t pts);

private:
    QChartView *chartView;
    QLineSeries *series;
    QChart *chart;

};
#endif // MAINWINDOW_H
