#ifndef ONLINETABLE_H
#define ONLINETABLE_H

#include <QObject>
#include <QHash>
#include <QColor>

#include "serialflowcontrol.h"
#include "main.h"

class QTableWidget;
class QLineEdit;
class QPushButton;
class QTime;
class QTableWidgetItem;

namespace ONLINETABEL_PARA
{
    const int EVERYTIMER = 500; //500ms
    const QColor COLORLIST[] = {
        QColor(  0,138,115), //深绿
        QColor( 21,254, 21), //亮绿色
        QColor(140,249,140), //浅绿
        QColor(190,249,190), //浅浅绿
        QColor(220,249,220), //浅浅浅绿
        QColor(255,255,255)  //白
    };
    const int COLORLIST_LEN = sizeof(COLORLIST)/sizeof(COLORLIST[0]);
    const int IND_VIS = 0;   // is visiable
    const int IND_NAME = 1;  // "name"
    const int IND_COMM = 2;  // "comment"
    const int IND_COUNT = 3; // "count"
    const int IND_ISCOL = 3; // is color for "count"
    const int IND_ALL = 4;
}
namespace ONLINETABEL_PRIVATE
{
}
class OnlineTable : public QObject
{
    Q_OBJECT
public:
    explicit OnlineTable(QObject *parent,
                         const QHash<QString, StreamItem> * mdatahash,
                         QTableWidget * mtableWidget,
                         QLineEdit * le_name,
                         QPushButton * btn_add,
                         QPushButton * btn_sub,
                         QPushButton * btn_up,
                         QPushButton * btn_down);
    void swapRow(int r1, int r2);
    void initPush2Chart();
private:
    const QHash<QString, StreamItem> *datahash; //SerialFlowControl::datahash
    QTableWidget *tableWidget;
    QLineEdit    *LE_name;
    QPushButton  *BTN_add;
    QPushButton  *BTN_rm;
    QPushButton  *BTN_up;
    QPushButton  *BTN_down;
    QStringList   namelist;
    int           timerEventId;
    QTime        *myTime;            //time flow afer "start" click
    bool          isStarted;
signals:  //与chart逐个对应，通讯
    void add_line(const QString &name, bool visible);
    void set_line_visible(const QString &name, bool visible);
    void rm_line(const QString &name);
    void rm_line_all();
public slots:
    void timerEvent(QTimerEvent * event);
    void when_start();
    void when_stop();
    void on_BTN_add_click(bool vis=true, QString name="", QString comm="", int count=0, int isCol = true);
    void on_BTN_rm_click();
    void on_BTN_up_click();
    void on_BTN_down_click();
    void on_tableWidget_click(QTableWidgetItem *);

signals: //准备写入到文件
    void takefrom_this(bool isVisible, const QString & name, const QString & comm, bool isColor); //this -> file
public slots: //准备读取自文件
    void clean_table();    //file -> this
    void commit_settings();    //this -> file
    void giveto_this(bool isVisible, const QString &name, const QString &comm, bool isColor = true); //file -> this
};

#endif // ONLINETABLE_H
