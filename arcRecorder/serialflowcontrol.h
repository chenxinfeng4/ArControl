#ifndef SERIALFLOWCONTROL_H
#define SERIALFLOWCONTROL_H

#include <QObject>
#include <QHash>
#include <QtGlobal>
#include <QtSerialPort/QSerialPort>
#include <QTextStream>

#include "main.h"

class QSerialPort;
class QPushButton;
class QLineEdit;
class QPlainTextEdit;
class QCheckBox;
class QDockWidget;
class QFile;

namespace SERIALFLOWCONTROL_PARA
{
    const char BG_HEADER_STRING[]  = "ArControl";
    const char BG_FLOW_STRING[] = "ArC-bg";
    const char END_FLOW_STRING[] = "ArC-end";
    const char MY_STRBEGIN[] = "b";
    const int  MAX_LINES = 100;
    const char DATA_HINT_CHAR = ':';
    const char DATA_IN_HIGH_CHAR = '$';
    const char DATA_OUT_HIGH_CHAR = '%';
}
namespace SERIALFLOWCONTROL_PRIVATE
{
    class Worker : public QObject
    {
        Q_OBJECT
    public slots:
        void doWork(QSerialPort * serial,const bool isConnect);
    signals:
        void readLine(const QString &result);
    };
}
struct StreamItem
{
    qint32 count;   //总共发生的数量
    qint32 count_pre;
    qint32 hot;
    qint8  ishigh;   //for INPUT and OUTPUT
};
class SerialFlowControl : public QObject
{
    Q_OBJECT
public:
    explicit SerialFlowControl(QObject     *parent,
                               QDockWidget *dw_container,
                               QLineEdit   *le_send,
                               QPlainTextEdit *pte_data,
                               QCheckBox   *ckb_freeze,
                               QCheckBox   *ckb_autoc,
                               QPushButton *btn_send,
                               QPushButton *btn_clear,
                               QPushButton *btn_copy,
                               QSerialPort *mserial
                               );
    void calDataHot();          //计算datahash中StreamItem各自的变化热度
    bool getStarted() const;
    const QHash<QString, StreamItem>  * get_datahash();

private:
    QDockWidget  * DW_container;
    QLineEdit    * LE_send;
    QPlainTextEdit * PTE_data;
    QCheckBox    *CKB_freeze;
    QCheckBox    *CKB_autoc;
    QPushButton  *BTN_send;
    QPushButton  *BTN_clear;
    QPushButton  *BTN_copy;
    QSerialPort  *serial;       //指针的值不变，地址不变，只是设置地址下的属性值
    volatile bool isConnect;    //是否可用
    volatile bool isConnect_pre;//保存上次的结果
    volatile bool isStarted;    //ArControl是否真实开始 在接受到"ArC"字符后确定
    QFile        *datafile;     //暂时存储的file，全路径('C:/a/b/proj_a/subj_b/171203123011.txt')
    QTextStream   datastreamout;//接管datafile的 out 流
    QString       datadir;      //datafile的存放目录, 全路径('C:/a/b/proj_a/subj_b')
    QString       PTE_data_buff; //暂时没有传到PTE中的数据
    QHash<QString, StreamItem>  *datahash; //存放运行时的所有item计数
    QString       file_header;   //文件的 header 从 "ArControl"到"-----TASKNAME-----";
    void init_datahash();
signals:
    void raise_spont_start();   //自发开始
    void raise_spont_stop();    //自发结束
    void wantto_readLine(QSerialPort*,const bool);
    void raise_everyTimeCycle();//每固定时间轮询，向外发出同步信号
    void raise_isconnect(const bool &); //自在连接和断开时发出，不重复发出
public slots:
    void timerEvent(QTimerEvent * event);
    void on_PTE_data_count(int);
    void on_BTN_send_clicked();
    void on_BTN_copy_clicked();
    void on_BTN_clear_clicked();
    void on_serial_error(QSerialPort::SerialPortError err = QSerialPort::UnknownError);
    void when_press_start();                    //信号由点击"开始"
    void when_press_stop();                     //信号由点击"结束"
    void when_real_start();
    void when_real_stop();
    void when_dataDir_change(QString &arq1);    //datafile的保存目录
    void on_isconnect(const bool &);
    void receive_readLine(const QString &);
    void swithlevel_outpin(int);
signals:
    void takefrom_this(bool isVisible, bool isDock, bool isAutoS, bool isFreeze);//this -> file
public slots:
    void giveto_this(bool isVisible, bool isDock, bool isAutoS, bool isFreeze); //file -> this
    void commit_settings(); //emit takefrom_this
};

#endif // SERIALFLOWCONTROL_H
