#ifndef MYQTTREEWIDGET_H
#define MYQTTREEWIDGET_H

#include <QObject>
#include <QTreeWidget>

class MyQtTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit MyQtTreeWidget(QWidget *parent = nullptr);
    virtual void mousePressEvent(QMouseEvent* event) override;

};

#endif // MYQTTREEWIDGET_H
