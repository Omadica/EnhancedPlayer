#include "custom_view.h"
#include <QWidget>
#include <QModelIndex>
#include <QMenu>
#include <QMimeData>

custom_view::custom_view(QWidget *parent) : QGraphicsView(parent), m_bIsMousePressed(false)
{

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

    qDebug() << "Drop Event" ;
    emit callVideo();
}


void custom_view::dragEnterEvent(QDragEnterEvent *event) {
    event->acceptProposedAction();
    qDebug() << "Drag Enter Event";

}

void custom_view::dragMoveEvent(QDragMoveEvent *event) {
    qDebug() << "Drag Move Envent" ;
    event->acceptProposedAction();

}
//void custom_view::mousePressEvent(QMouseEvent *event)
//{
//    /*
//    QPoint pos = event->pos();
//    if(event->button() == Qt::RightButton)
//    {
//        QMenu *menu = new QMenu(this);

//        QAction *zoom1 = new QAction(this);
//        zoom1->setIconVisibleInMenu(true);
//        zoom1->setText(tr("Reset zoom 1:1"));

//        QAction *undistortLens = new QAction(this);
//        undistortLens->setIconVisibleInMenu(true);
//        undistortLens->setText(tr("Lens correction"));

//        menu->addAction(zoom1);
//        menu->addAction(undistortLens);
//        menu->addAction(dewarp);
//        menu->popup(this->viewport()->mapToGlobal(pos));
//    }
//    else if(event->button() == Qt::LeftButton)
//    {
//        QAction *dewarp = new QAction(this);
//        dewarp->setIconVisibleInMenu(true);
//        dewarp->setText(tr("Dewarp FishEye"));

//        QAction *zoom1 = new QAction(this);
//        zoom1->setIconVisibleInMenu(true);
//        zoom1->setText(tr("Reset zoom 1:1"));

//        QAction *undistortLens = new QAction(this);
//        undistortLens->setIconVisibleInMenu(true);
//        undistortLens->setText(tr("Lens correction"));


//        menu->addAction(zoom1);
//        menu->addAction(undistortLens);
//        menu->addAction(dewarp);
//        menu->popup(this->viewport()->mapToGlobal(pos));
//    }
//    else if(event->button() == Qt::LeftButton)
//    {


//void custom_view::mouseMoveEvent(QMouseEvent *event)
//{
//    /*
//    if (m_bIsMousePressed)
//    {
//        QPoint pos = event->pos();
//        bottomRight = pos;
//        qDebug() << pos << " " << m_bIsMousePressed;
//        //update();
//    }
//    */
//        m_bIsMousePressed = true;
//        topLeft = pos;
//        bottomRight = topLeft;
//        qDebug() << pos << " " << m_bIsMousePressed;
//    }
//    */
//}


//void custom_view::mouseMoveEvent(QMouseEvent *event)
//{
//    /*
//    if (m_bIsMousePressed)
//    {
//        QPoint pos = event->pos();
//        bottomRight = pos;
//        qDebug() << pos << " " << m_bIsMousePressed;
//        //update();
//    }
//    */

//}

//void custom_view::mouseReleaseEvent(QMouseEvent *event)
//{
//    /*
//    m_bIsMousePressed = false;
//    if(event->button() == Qt::LeftButton)
//        rects.append(QRect(topLeft, bottomRight));
//    qDebug() << event->pos() << " " << m_bIsMousePressed;
//    */
//}

//void custom_view::resizeEvent(QResizeEvent *) {
//    QList<QGraphicsItem *> i = items();
//    int window_w = width();
//    if (window_w == 0 || i.size() != 1) return;

//    auto *item = qgraphicsitem_cast<QGraphicsPixmapItem *>(i[0]);
//    qreal img_w = static_cast<double>(item->pixmap().width());
//    qreal factor = window_w / img_w;
//    item->setScale(factor);

//    QRectF rect = item->boundingRect();
//    rect.setHeight(height());
//    rect.setWidth(width());
//    rect.moveCenter(item->boundingRect().center());

//    QGraphicsScene *s = scene();
//    s->setSceneRect(rect);

//    item->setTransformOriginPoint(item->boundingRect().center());
//    centerOn(item);
//}
