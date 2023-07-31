#ifndef CUSTOM_VIEW_H
#define CUSTOM_VIEW_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QList>
#include <QRect>

class custom_view : public QGraphicsView
{
    Q_OBJECT
public:
    explicit custom_view(QWidget *parent = nullptr);


protected:
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;

private slots:
    void clicked_on_dewarp();

signals:
    void click_on_dewarp();
//    virtual void mouseMoveEvent(QMouseEvent *event) override;
//    virtual void mouseReleaseEvent(QMouseEvent *event) override;
//    virtual void resizeEvent(QResizeEvent *) override;

private:
    QList<QRect> rects;
    bool m_bIsMousePressed;
    QPoint topLeft;
    QPoint bottomRight;

};

#endif // CUSTOM_VIEW_H
