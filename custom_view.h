#ifndef CUSTOM_VIEW_H
#define CUSTOM_VIEW_H

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

class custom_view : public QGraphicsView
{
    Q_OBJECT
public:
    explicit custom_view(QWidget *parent = nullptr);


protected:
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dragMoveEvent(QDragMoveEvent *event) override;

protected slots:
    void playVideo(const QString url);
    void drawFrame(QImage img);

signals:
    void callVideo(const QString);
    void frameRGB(QImage img);
    void framePts(int64_t pts);

private:
    std::shared_ptr<TaskManager::ThreadPool> threadpool;
    std::shared_ptr<TaskManager::Scheduler> scheduler;

    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink;
    std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink;
    std::shared_ptr<spdlog::logger> m_logger;

    std::function<void(MediaWrapper::AV::VideoFrame*)> callback;
    QGraphicsScene *scene;
    QList<QRect> rects;
    bool m_bIsMousePressed;
    QPoint topLeft;
    QPoint bottomRight;

};

#endif // CUSTOM_VIEW_H
