#ifndef CUSTOM_VIEW_H
#define CUSTOM_VIEW_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>

class custom_view : public QGraphicsView
{
    Q_OBJECT
public:
    explicit custom_view(QWidget *parent = nullptr);


protected:
    virtual void wheelEvent(QWheelEvent *event) override;
    using QGraphicsView::QGraphicsView;


};

#endif // CUSTOM_VIEW_H
