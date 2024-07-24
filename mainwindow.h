#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QThreadPool>
#include <QGraphicsScene>
#include <QtConcurrent/QtConcurrent>
//#include "zerniketransform.h"
//#include "LoggerService.h"
#include "TaskProcessor.h"
#include <memory.h>

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
    void frameRGB1(QImage img);
    void frameRGB2(QImage img);
    void frameRGB3(QImage img);
    void frameRGB4(QImage img);


protected slots:
    void playVideo1(const QString url);
    void playVideo2(const QString url);
    void playVideo3(const QString url);
    void playVideo4(const QString url);

    void drawFrame1(QImage img);
    void drawFrame2(QImage img);
    void drawFrame3(QImage img);
    void drawFrame4(QImage img);

private:
    QGraphicsScene *scene1;
    QGraphicsScene *scene2;
    QGraphicsScene *scene3;
    QGraphicsScene *scene4;
    QImage lastFrame;
    std::function<void(MediaWrapper::AV::VideoFrame*)> callback;

};
#endif // MAINWINDOW_H
