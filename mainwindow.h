#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QThreadPool>
#include <QGraphicsScene>
#include "FFmpegVideoDecoder.h"
#include "zerniketransform.h"
#include "LoggerService.h"
#include "TaskProcessor.h"
#include "./ui_FishEye.h"
#include <memory.h>

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
    void DrawGraph(QImage img);
    void PrintDecoderInfo(QString dec);
    void loadDecoders();
    void TakePicture();
    void resetDecoder();

signals:
    void readyForFrame();
    void stopDecodingThread();


private:
    // pass the ref to rstp_connection
    AVFormatContext* m_pFormatContext;
    QImage m_FrameImage;
    FFmpegVideoDecoder* decoder;
    QString rtsp_addr;
    QString namePic;
    QGraphicsScene *scene;
    int numPic;
    ZernikeTransform* ZerTrans;

    QMainWindow *mw;
    Ui::FishEyeWindow *w;
    Ui::MainWindow *ui;
    void addcamera();

    bool m_bDewarp{false};
    int m_radius, m_aperture;
    int m_cx, m_cy;
    int m_dx, m_dy;
    float m_theta, m_phi;
    std::shared_ptr<spdlog::logger> m_logger;


};
#endif // MAINWINDOW_H
