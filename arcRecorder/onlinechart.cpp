#define QT_NO_DEBUG_OUTPUT

#include "onlinechart.h"
#include <QTime>
#include <QLayout>
#include <QVBoxLayout>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QAreaSeries>
#include <QDebug>

QT_CHARTS_USE_NAMESPACE

using namespace ONLINECHART_PRIVATE;
using namespace ONLINECHART_PARA;

static void removeQString(QStringList & slist, const QString & str);

OnlineChart::OnlineChart(QObject *parent,
                         QWidget *mcontainer,
                         const QHash<QString, StreamItem> * mdatahash)
                :QObject(parent), container(mcontainer), datahash(mdatahash),
                 myTime(new QTime), isStarted(false),
                 lineseries(new QHash<QString, QLineSeries *>)
{
    /* 建立 QChart */
    this->chart = new QChart;
    chart->createDefaultAxes();

    QChartView * chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);  //抗锯齿渲染
    QLegend * legend = chart->legend();
    legend->setAlignment(Qt::AlignRight);
    QFont font = legend->font();
    font.setBold(true);
    font.setPixelSize(16);
    legend->setFont(font);

    /* UI 元素 */
    this->layout = new QVBoxLayout;
    layout->addWidget(chartView);
    this->container->setLayout(layout);
    QLineSeries * demoSeries = new QLineSeries();
    demoSeries->setVisible(false);
    chart->addSeries(demoSeries);
    this->recreateAxes();

    /* 设置的默认 strlist */
    this->strlist = KEYBUFFER;
    this->strlist_init_vis.clear(); //none to be view

    /* 初始化 lineseriel */
    this->lineseries->clear();
    foreach (const QString str, this->strlist) {
        QLineSeries * series = new QLineSeries(this);
        series->setName(str);
        series->setVisible(false);
        this->chart->addSeries(series);
        (*lineseries)[str] = series;
    }
    qDebug()<<"strlist : "<< strlist;
    qDebug()<<"OnlineChart():Hash Count: "<<(*lineseries).size();
    chart->createDefaultAxes();
}

void OnlineChart::recreateAxes()
{
    chart->createDefaultAxes();
    chart->axisY()->setRange(0, LINELOOPCOUNT);
    chart->axisX()->setTitleText(tr("Time (min)"));
    chart->axisY()->setTitleText(tr("Count"));
}
void OnlineChart::timerEvent(QTimerEvent * event)
{
    Q_UNUSED(event);
    qreal tflow_min = this->myTime->elapsed() / qreal(60000);
    auto lineseries = this->lineseries;
    QHash<QString, QLineSeries *>::iterator i;
    for (i = (*lineseries).begin(); i != (*lineseries).end(); ++i) {
        qint32 count = (*datahash)[i.key()].count;
        if(!count) /*nothing in this KEY ==> reduce CPU time*/
            continue;
        count %= LINELOOPCOUNT;
        i.value()->append(tflow_min, count);
    }
    qreal maxX = 0;

    /* 矫正坐标轴范围 */
    if(tflow_min <= 1.0) {maxX = 1;}
    else if(tflow_min <= 2.0) {maxX = 2;}
    else if(tflow_min <= 5.0) {maxX = 5;}
    else if(tflow_min <= 10.0) {maxX = 10;}
    else if(tflow_min > this->axisXmax){maxX = this->axisXmax + 5;}
    if(maxX > this->axisXmax) {
        this->axisXmax = maxX;
        this->chart->axisX()->setMax(this->axisXmax);
        qDebug()<<"maxX"<<maxX<<"axisXmax"<<axisXmax;
    }
}
void OnlineChart::when_start()
{
    this->myTime->restart();
    /* 清除lineseries & 轴*/
    auto lineseries = this->lineseries;
    QHash<QString, QLineSeries *>::iterator i;
    for (i = (*lineseries).begin(); i != (*lineseries).end(); ++i) {
        i.value()->clear();
    }
    this->recreateAxes();
    this->axisXmax = 1;
    this->chart->axisX()->setMax(this->axisXmax);
    qDebug()<<"when_start(): lines count"<<(*lineseries).count();
    /* 建立标志位 */
    qDebug()<<"STRLIST VIS"<<this->strlist_init_vis;
    this->isStarted = true;
    this->timerEventId = this->startTimer(EVERYTIMER);
    Q_ASSERT(this->timerEventId != 0);
}

void OnlineChart::when_stop()
{
    qDebug()<<"SSSSSSSSSSStop";
    this->killTimer(this->timerEventId);
    this->isStarted = false;
}
void OnlineChart::set_line_visible(const QString &name, bool visible)
{
    const QString name_trim = name.trimmed();
    auto lineseries = this->lineseries;
    if((*lineseries).contains(name_trim)){
        (*lineseries)[name_trim]->setVisible(visible);
        if(visible)
            this->strlist_init_vis << name_trim;
        else
            removeQString(this->strlist_init_vis, name_trim);
    }
}
void OnlineChart::add_line(const QString &name, bool visible)
{
    const QString name_trim = name.trimmed();
    if(this->strlist.contains(name_trim)) {
        /* lineserie already existed */
        qDebug()<<"add_line(): contains"<<name<<visible;
        this->set_line_visible(name_trim,visible);
        (*(this->lineseries))[name_trim]->setVisible(visible);
    }
    else {
        /* new a lineserie */
        qDebug()<<"add_line(): no contains"<<name<<visible;
        this->strlist << name_trim;
        QLineSeries * series = new QLineSeries(this);
        series->setName(name_trim);
        series->setVisible(visible);
        chart->addSeries(series);
        chart->createDefaultAxes();
        (*(this->lineseries))[name_trim] = series;
    }
    if(visible)
        this->strlist_init_vis << name_trim;
    else
        removeQString(this->strlist_init_vis, name_trim);
}
void OnlineChart::rm_line(const QString &name)
{
    const QString name_trim = name.trimmed();
    if(KEYBUFFER.contains(name_trim)){
        /* in buffer : hide line*/
        (*(this->lineseries))[name_trim]->setVisible(false);
    }
    else {
        QMutableListIterator<QString> i(this->strlist);
        i.toBack();
        while(i.hasPrevious()){
            if(name_trim == i.previous()){
                /* in temp : delete line */
                (*(this->lineseries))[name_trim]->deleteLater();
                (*(this->lineseries)).remove(name_trim);
                i.remove();
                break; //have done removeDuplicates
            }
        }
    }
    removeQString(this->strlist_init_vis, name_trim);
}
void OnlineChart::rm_line_all()
{
    qDebug()<<"rm_line_all()_pre:Hash Count: "<<(*lineseries).size();
    foreach(const QString name, this->strlist)
        this->rm_line(name);

    qDebug()<<"rm_line_all()_post:Hash Count: "<<(*lineseries).size();
}

void removeQString(QStringList & slist, const QString & str)
{
    QMutableListIterator<QString> i(slist);
    i.toBack();
    while(i.hasPrevious()){
        if(str == i.previous())
            i.remove();
    }
}
