#include "custom_view.h"
#include <QWidget>
#include <QModelIndex>
#include <QMenu>
#include <QMimeData>
#include <QFont>
#include <chrono>
#include <iostream>
#include "mainwindow.h"

std::shared_ptr<TaskManager::ThreadPool> threadpool;
std::shared_ptr<TaskManager::Scheduler> scheduler;


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

    this->acceptDrops();
    scene = new QGraphicsScene(this);
    this->setScene(scene);

    connect(this, &custom_view::callVideo, this, &custom_view::playVideo);
    connect(this, &custom_view::frameRGB, this, &custom_view::drawFrame);
    connect(this, &custom_view::stopped, this, &custom_view::notifyAll);

}

void custom_view::notifyAll()
{
    m_cv.notify_all();
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

void custom_view::dropEvent(QDropEvent *event)
{
    auto lock = std::unique_lock<std::mutex>(stopMutex);
    const QMimeData* mimeData =  event->mimeData();
    if(mimeData->hasText()){
        qDebug() << "Mime has text: " << mimeData->text() ;
        path = mimeData->text().toStdString();
    } else {
        qDebug() << "Mime has NOT text" ;
    }

    qDebug() << "Drop camera " << mimeData->text() << " on cell " << this->objectName();

    if(!bStreamingActive){
        scene->clear();
        scene->addText("Loading camera " + mimeData->text() + "...");
        emit callVideo(mimeData->text());
        bStreamingActive = true;
    } else {
        stopLive();
        m_cv.wait(lock, [this](){return !bStreamingActive;} );
        scene->clear();
        scene->addText("Loading camera " + mimeData->text() + "...");
        emit callVideo(mimeData->text());
        bStreamingActive = true;
    }
}

void custom_view::getUrlAndToken(std::string urlR, std::string tokenR)
{

    authMethod = "JWT";
    url = urlR;
    token = tokenR;
}

void custom_view::getAuthMethod(std::string auth, std::string urlR, std::string userR, std::string passwdR)
{
    authMethod = auth;
    url = urlR;
    user = userR;
    passwd = passwdR;
}

// void custom_view::playVideo(const QString path)
// {
//     auto fut = QtConcurrent::run([=](){
//         auto fut = scheduler->scheduleLambda("LiveJob MockUp", [&]() {
//             int idx = 0;
//             std::hash<std::thread::id> hasher;
//             while(true){
//                 std::this_thread::sleep_for(std::chrono::milliseconds(300));
//                 qDebug() << "Thread: " << hasher(std::this_thread::get_id()) << " Value: " << idx;
//                 idx++;
//             }

//         });
//     });

// }



void custom_view::playVideo(const QString path)
{
    qDebug() << "Play Video";
    fut = QtConcurrent::run([=] {


        callback = [&](MediaWrapper::AV::VideoFrame* frame) {

            qDebug() << "Callback called";

            QImage lastFramepp = QImage(frame->width(), frame->height(), QImage::Format_RGB888);

            for(int y=0; y < frame->height(); y++)
                memcpy(
                    lastFramepp.scanLine(y),
                    frame->raw()->data[0] + y * frame->raw()->linesize[0],
                    frame->raw()->width*3
                    );

            emit frameRGB(lastFramepp);
            // emit framePts(frame->pts().timestamp());
        };


        qDebug() << "Reproducing video from " << url;
        std::string URL;
        if(authMethod == "JWT"){
            URL = "rtsp://" + url + ":8554/" + path.toStdString() + "?jwt=" + token;
            qDebug() << "Reproducing video from " << URL ;
        } else if (authMethod == "Internal"){
            URL = "rtsp://" + user + ":" + passwd + "@" + url + ":8554/" + path.toStdString();
            qDebug() << "Reproducing video from " << URL ;
        }

        context = std::make_unique<TaskProcessor::ProcessorContext>(URL);

        auto fut = scheduler->scheduleLambda("LiveJob " + URL, [&]() {

           auto job = std::make_unique<TaskProcessor::LiveStream>(context->GetURL(), scheduler);
           bStreamingActive = true;
           context->set_processor(std::move(job));
           context->initializeProcessorContext();
           context->readAndDecode(callback);

           job.reset();

        });

        fut.wait();

        bStreamingActive = false;
        emit stopped();
        context.reset();
        m_cv.notify_all();
    });

}

void custom_view::drawFrame(QImage img)
{

    scene->clear();
    scene->setSceneRect(QRectF(0,0,img.width(), img.height()));
    scene->addPixmap(QPixmap::fromImage(img));

}

void custom_view::stopLive()
{
    auto futureStop = scheduler->scheduleLambda("StopJob ", [&]() {
        if(context){

            auto lock = std::unique_lock<std::mutex>(stopMutex);
            context->stopProcess();
            scene->clear();


            QImage image(QSize(4,4),QImage::Format_RGB32);
            QPainter painter(&image);
            painter.setBrush(QBrush(Qt::white));
            painter.fillRect(QRectF(0,0,4,4),Qt::white);
            painter.fillRect(QRectF(10,100,200,100),Qt::white);

            scene->addPixmap(QPixmap::fromImage(image));
            qDebug() << path << " stopped";

            scene->setSceneRect(QRectF(0,0,0,0));
            scene->update();

            //m_cv.wait(lock, [&](){return bStreamingActive==true;});
            fut.cancel();
            m_cv.notify_all();
            qDebug() << path << " notified";

        }
    });

}


void custom_view::dragEnterEvent(QDragEnterEvent *event) {
    event->acceptProposedAction();
    qDebug() << "Drag Enter Event";

}

void custom_view::dragMoveEvent(QDragMoveEvent *event) {
    // qDebug() << "Drag Move Envent" ;
    event->acceptProposedAction();
}

void custom_view::mousePressEvent(QMouseEvent* event)
{
    if(!isFocused)
    {
        event->accept();
        setLineWidth(2);
        setFrameShadow(Shadow::Plain);
        emit focusIn(this);
        emit getRecordings(path);
        isFocused = true;
    } else {
        setLineWidth(1);
        setFrameShadow(Shadow::Sunken);

        isFocused = false;
    }

    qDebug() << this->frameSize() ;
    qDebug() << scene->sceneRect();
    qDebug() << this->devicePixelRatioFScale();
    qDebug() << bStreamingActive;

}

custom_view::~custom_view()
{

    qDebug() << "Stopping context ..." ;
    if(context){
        qDebug() << "Waiting stop context" ;
        auto lock = std::unique_lock<std::mutex>(stopMutex);
        context->stopProcess();
        m_cv.wait(lock, [this](){return !bStreamingActive;} );
        qDebug() << "Context stopped, move to close view...";
    }

    qDebug() << "Destroing view ..." ;

}
