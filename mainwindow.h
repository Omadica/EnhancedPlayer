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

protected slots:
    void playVideo(const QString url);

private:
    QGraphicsScene *scene;
    QImage lastFrame;

};
#endif // MAINWINDOW_H
