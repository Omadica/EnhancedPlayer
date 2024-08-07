#include "custom_view.h"
#include <QWidget>
#include <QModelIndex>
#include <QMenu>
#include <QtConcurrent/QtConcurrent>
#include <QMimeData>


custom_view::custom_view(QWidget *parent) : QGraphicsView(parent), m_bIsMousePressed(false)
{

    const spdlog::level::level_enum log_level = spdlog::level::critical;
    console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("LiveJob.log", true);

    console_sink->set_level(log_level);
    file_sink->set_level(log_level);
    std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink};
    m_logger = std::make_shared<spdlog::logger>("NativeLog", begin(sinks), end(sinks));
    m_logger->set_level(log_level);
    m_logger->enable_backtrace(32);


    MediaWrapper::AV::init();

    size_t nthreads = std::thread::hardware_concurrency();
    threadpool = std::make_shared<TaskManager::ThreadPool>(nthreads);
    scheduler = std::make_shared<TaskManager::Scheduler>(threadpool, nthreads*20);

    this->acceptDrops();
    scene = new QGraphicsScene(this);
    this->setScene(scene);

    connect(this, &custom_view::callVideo, this, &custom_view::playVideo);
    connect(this, &custom_view::frameRGB, this, &custom_view::drawFrame);

}

void custom_view::wheelEvent(QWheelEvent *event)
{

    setTransformationAnchor(AnchorUnderMouse);
    double scalefactor = 1.05;
    if( event->angleDelta().y() > 0 )
    {
        scale(scalefactor, scalefactor);
    }
    else
    {
        scale(1/scalefactor, 1/scalefactor);
    }
}

void custom_view::dropEvent(QDropEvent *event){
    const QMimeData* mimeData =  event->mimeData();
    if(mimeData->hasText())
        qDebug() << "Mime has text: " << mimeData->text() ;
    else
        qDebug() << "Mime has NOT text" ;

    qDebug() << "Drop camera " << mimeData->text() << " on cell " << this->objectName();
    emit callVideo(mimeData->text());
}

void custom_view::getUrlAndToken(std::string urlR, std::string tokenR)
{

    url = urlR;
    token = tokenR;
}


void custom_view::playVideo(const QString path)
{
    qDebug() << "Play Video";

    QFuture<void> fut = QtConcurrent::run([=] {


        callback = [&](MediaWrapper::AV::VideoFrame* frame) {

            qDebug() << "Callback called";

            QImage lastFramepp = QImage(frame->width(), frame->height(), QImage::Format_RGB888);;

            for(int y=0; y < frame->height(); y++)
                memcpy(
                    lastFramepp.scanLine(y),
                    frame->raw()->data[0] + y * frame->raw()->linesize[0],
                    frame->raw()->width*3
                    );

            emit frameRGB(lastFramepp);
            emit framePts(frame->pts().timestamp());
        };


        qDebug() << "Reproducing video from " << url;
        const std::string URL = "rtsp://" + url + ":8554/" + path.toStdString() + "?jwt=" + token;
        qDebug() << "Reproducing video from " << URL ;
        auto context = std::make_unique<TaskProcessor::ProcessorContext>(URL);

        auto fut = scheduler->scheduleLambda("LiveJob"+URL, [&]() {
            qDebug() << "Hello from Lambda";

            auto job = std::make_unique<TaskProcessor::LiveStream>(context->GetURL(), scheduler);
            context->set_processor(std::move(job));
            context->initializeProcessorContext();
            context->readAndDecode(callback);

        });
        fut.wait();
    });
}


void custom_view::drawFrame(QImage img)
{
    scene->clear();
    scene->addPixmap(QPixmap::fromImage(img));

}


void custom_view::dragEnterEvent(QDragEnterEvent *event) {
    event->acceptProposedAction();
    qDebug() << "Drag Enter Event";

}

void custom_view::dragMoveEvent(QDragMoveEvent *event) {
    // qDebug() << "Drag Move Envent" ;
    event->acceptProposedAction();
}
