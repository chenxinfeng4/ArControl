#ifndef ONLINECHART_H
#define ONLINECHART_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QStringList>

#include "serialflowcontrol.h"
#include "main.h"

class QLayout;
class QTime;


QT_CHARTS_USE_NAMESPACE

namespace ONLINECHART_PARA
{
    const int EVERYTIMER=500; //500ms 刷新一次
    const int LINELOOPCOUNT=100;
    const QStringList KEYBUFFER = QStringList()
                    <<"OUT1"<<"OUT2"<<"OUT3"<<"OUT4"<<"OUT5"<<"OUT6"<<"OUT7"<<"OUT8"
                    <<"IN1" <<"IN2" <<"IN3" <<"IN4" <<"IN5" <<"IN6" <<"IN7" <<"IN8"
                    <<"C1S1"<<"C1S2"<<"C1S3"<<"C1S4"<<"C1S5"<<"C1S6"<<"C1S7"<<"C1S8"
                    <<"C2S1"<<"C2S2"<<"C2S3"<<"C2S4"<<"C2S5"<<"C2S6"<<"C2S7"<<"C2S8"
                    <<"C3S1"<<"C3S2"<<"C3S3"<<"C3S4"<<"C3S5"<<"C3S6"<<"C3S7"<<"C3S8"
                    <<"C4S1"<<"C4S2"<<"C4S3"<<"C4S4"<<"C4S5"<<"C4S6"<<"C4S7"<<"C4S8"
                    <<"C0S0";

}
namespace ONLINECHART_PRIVATE
{

}

class OnlineChart : public QObject
{
    Q_OBJECT
public:
    explicit OnlineChart(QObject *parent,
                         QWidget *container,
                         const QHash<QString, StreamItem> * mdatahash);

private:
    QWidget   * container;
    const QHash<QString, StreamItem> * datahash; //SerialFlowControl::datahash
    int         timerEventId;      //QObject->timer identifier,
    QLayout   * layout;
    QTime     * myTime;            //time flow afer "start" click
    bool        isStarted;
    QChart    * chart;
    QStringList strlist;           //trimed & not-empty
    QStringList strlist_init_vis;  //init visible lines of which in strlist
    QHash<QString, QLineSeries *> *lineseries; // lines in axes
    qreal       axisXmax;

    void recreateAxes();
signals:

public slots:
    void timerEvent(QTimerEvent * event);
    void when_start();
    void when_stop();
    void set_line_visible(const QString &name, bool visible);
    void add_line(const QString &name, bool visible = true);
    void rm_line(const QString &name);
    void rm_line_all();
};

#endif // ONLINECHART_H
