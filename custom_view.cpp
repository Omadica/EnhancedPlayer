#include "custom_view.h"
#include <QWidget>
#include <QModelIndex>
#include <QMenu>

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


void custom_view::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();

    if(event->button() == Qt::RightButton)
    {
        QMenu *menu = new QMenu(this);

        QAction *dewarp = new QAction(this);
        dewarp->setIconVisibleInMenu(true);
        dewarp->setText(tr("Dewarp FishEye"));

        QAction *zoom1 = new QAction(this);
        zoom1->setIconVisibleInMenu(true);
        zoom1->setText(tr("Reset zoom 1:1"));

        QAction *undistortLens = new QAction(this);
        undistortLens->setIconVisibleInMenu(true);
        undistortLens->setText(tr("Lens correction"));

        menu->addAction(zoom1);
        menu->addAction(undistortLens);
        menu->addAction(dewarp);
        menu->popup(this->viewport()->mapToGlobal(pos));
    }
    else if(event->button() == Qt::LeftButton)
    {

        m_bIsMousePressed = true;
        topLeft = pos;
        bottomRight = topLeft;
        qDebug() << pos << " " << m_bIsMousePressed;
    }
}

void custom_view::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bIsMousePressed)
    {
        QPoint pos = event->pos();
        bottomRight = pos;
        qDebug() << pos << " " << m_bIsMousePressed;
    }
    update();
}

void custom_view::mouseReleaseEvent(QMouseEvent *event)
{
    m_bIsMousePressed = false;
    if(event->button() == Qt::LeftButton)
        rects.append(QRect(topLeft, bottomRight));
    qDebug() << event->pos() << " " << m_bIsMousePressed;
}

