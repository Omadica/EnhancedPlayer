#ifndef CUSTOM_VIEW_H
#define CUSTOM_VIEW_H

#include <QtConcurrent/QtConcurrent>
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <TaskManager.h>
#include <TaskProcessor.h>
#include <MediaWrapper.h>
#include <QList>
#include <QRect>
#include <QString>
#include <future>

class custom_view : public QGraphicsView
{
    Q_OBJECT
public:
    explicit custom_view(QWidget *parent = nullptr);
    virtual ~custom_view();


protected:
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dragMoveEvent(QDragMoveEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;


protected slots:
    void playVideo(const QString url);
    void drawFrame(QImage img);
    void notifyAll();

public slots:
    void getUrlAndToken(std::string url, std::string token);
    void getAuthMethod(std::string auth, std::string urlR, std::string userR, std::string passwd);
    void stopLive();



signals:
    void callVideo(const QString);
    void frameRGB(QImage img);
    void framePts(int64_t pts);
    void focusIn(custom_view *cv);
    void focusOut();
    void stopped();



private:
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink;
    std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink;
    std::shared_ptr<spdlog::logger> m_logger;

    std::unique_ptr<TaskProcessor::ProcessorContext> context;
    std::function<void(MediaWrapper::AV::VideoFrame*)> callback;
    QGraphicsScene *scene;
    QList<QRect> rects;
    bool m_bIsMousePressed;
    QPoint topLeft;
    QPoint bottomRight;
    std::string url;
    std::string token;

    std::string authMethod;
    std::string user;
    std::string passwd;
    QFuture<void> fut;
    QPromise<void> promise;

    bool bStreamingActive = {false};

    bool isFocused {false};
    std::mutex stopMutex;
    std::condition_variable m_cv;

};

#endif // CUSTOM_VIEW_H
