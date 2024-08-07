#include "myqttreewidget.h"
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>

MyQtTreeWidget::MyQtTreeWidget(QWidget *parent) : QTreeWidget(parent)
{
    qDebug() << "MyQtTreeWidget CTOR called";
}

void MyQtTreeWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QTreeWidgetItem *item = this->itemAt(event->pos());
        if (item) {
            QDrag *drag = new QDrag(this);
            QMimeData *mimeData = new QMimeData;
            mimeData->setText(item->text(0));
            drag->setMimeData(mimeData);
            drag->exec();
        }
    }
}
