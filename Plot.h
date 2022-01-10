#ifndef PLOT_H
#define PLOT_H

#include <QWidget>

#include <QVBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLegend>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include <QVector>
#include <QPointF>

struct PlotLine
{
    QLineSeries *serie;

    QString name;
};

namespace Ui {
class Plot;
}

class Plot : public QWidget
{
    Q_OBJECT

public:
    explicit Plot(QWidget *parent = nullptr);
    ~Plot();

    void appendData(int index, QPointF point, int buffer = 100);

    void addSerie(const QString name);

    void setXText(const QString text);

    void setYText(const QString text);

    void setYAxisRange(const double min, const double max);

    int count();

private:
    Ui::Plot *ui;
    QChartView *_chartView;
    QChart *_chart;
    QVector<PlotLine> _lines;

    QString _plotTitle;
    QString _xTitle;
    QString _yTitle;
    QValueAxis *_axisX;
    QValueAxis *_axisY;

};

#endif // PLOT_H
