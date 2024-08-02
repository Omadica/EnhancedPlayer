#ifndef CHARTVIEWJITTER_H
#define CHARTVIEWJITTER_H

#include <QObject>
#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QSplineSeries>

class ChartViewJitter : public QWidget
{
    Q_OBJECT
public:
    explicit ChartViewJitter(QChart *parent = nullptr){};


private:
    QChartView *view;
    QScatterSeries *measure;


signals:
};

#endif // CHARTVIEWJITTER_H
