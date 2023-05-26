#include "custom_view.h"
#include <QModelIndex>
#include <QMenu>

custom_view::custom_view(QWidget *parent) : QGraphicsView(parent)
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

    if(event->button() == Qt::RightButton){
        QMenu *menu = new QMenu(this);
        menu->addAction(new QAction("Reset zoom 1:1", this));
        menu->addAction(new QAction("Undistort lens", this));
        menu->addAction(new QAction("Dewarp fisheye", this));
        menu->popup(this->viewport()->mapToGlobal(pos));
    }
    else if(event->button() == Qt::LeftButton)
    {
        m_bIsMousePressed = true;
        topLeft = this->viewport()->mapFromGlobal(pos);
    }


}
