#include "Plot.h"
#include "ui_Plot.h"

Plot::Plot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Plot),
    _chartView(nullptr),
    _chart(new QChart)
{
    ui->setupUi(this);

    _chartView = new QChartView(_chart);
    _chartView->setRenderHint(QPainter::Antialiasing);
    _chartView->setUpdatesEnabled(true);

    ui->Plot_Layout->addWidget(_chartView);

    _axisX = new QValueAxis;
    _axisX->setTickCount(10);
    _axisX->setTitleText(_xTitle);
    _chart->addAxis(_axisX, Qt::AlignBottom);

    _axisY = new QValueAxis;
    _axisY->setTickCount(10);
    _axisY->setTitleText(_yTitle);
    _chart->addAxis(_axisY, Qt::AlignLeft);
}

Plot::~Plot()
{
    delete ui;
}

void Plot::appendData(int index, QPointF point, int buffer)
{
    _lines[index].serie->append(point);
    if (_lines[index].serie->count() > buffer)
    {
        _lines[index].serie->remove(0);
    }
    _chart->axisX(_lines[index].serie)->setMin(point.x() - 10);
    _chart->axisX(_lines[index].serie)->setMax(point.x());

}

void Plot::addSerie(const QString name)
{
    PlotLine plotLine;
    plotLine.name = name;
    plotLine.serie = new QLineSeries;
    _chart->addSeries(plotLine.serie);
    plotLine.serie->attachAxis(_axisX);
    plotLine.serie->attachAxis(_axisY);
    plotLine.serie->setName(plotLine.name);
    _lines.append(plotLine);
    _lines.last().serie->setVisible(true);
}

void Plot::setXText(const QString text)
{
    _chart->axisX()->setTitleText(text);
}

void Plot::setYText(const QString text)
{
    _chart->axisY()->setTitleText(text);
}

void Plot::setYAxisRange(const double min, const double max)
{
    _chart->axisY(_lines.last().serie)->setMin(min);
    _chart->axisY(_lines.last().serie)->setMax(max);
}

int Plot::count()
{
    return _lines.count();
}
