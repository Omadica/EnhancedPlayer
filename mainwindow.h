#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void RtspConnection();


private:
    // pass the ref to rstp_connection
    AVFormatContext* m_pFormatContext;
    AVStream* m_pRtspStream;

    QThread *thread;



    Ui::MainWindow *ui;
    void addcamera();
};
#endif // MAINWINDOW_H
