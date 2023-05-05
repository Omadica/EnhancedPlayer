#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QThreadPool>
#include "FFmpegVideoDecoder.h"

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
    void StartPlayback();
    void DrawGraph1(QImage img);
    void DrawGraph2(QImage img);
    void DrawGraph3(QImage img);
    void DrawGraph4(QImage img);
    void PrintDecoderInfo(QString dec);
    void loadDecoders();

signals:
    void readyForFrame();


private:
    // pass the ref to rstp_connection
    AVFormatContext* m_pFormatContext;
    QImage m_FrameImage;
    FFmpegVideoDecoder* decoder;
    QString rtsp_addr;




    Ui::MainWindow *ui;
    void addcamera();
};
#endif // MAINWINDOW_H
