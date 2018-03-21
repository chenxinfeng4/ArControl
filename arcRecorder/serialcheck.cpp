#include "serialcheck.h"
#include <QTime>
#include <QMenu>
#include <QAction>
#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>
#include <QDebug>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QDateTime>
#include <QProgressDialog>
#include <QProgressBar>
#include <QString>

using namespace SERIALCHECK_PARA;
using namespace SERIALCHECK_PRIVATE;
const QList<int> BAUDRATE_LIST = {250000, 9600};

void SERIALCHECK_PRIVATE::autoBaundRate(QSerialPort *serial, int &baund_pre, bool &ok)
{
    /* List of baund rate */
    QList<int> baund_list(BAUDRATE_LIST);
    baund_list.removeOne(baund_pre);
    baund_list.insert(0, baund_pre);
    baund_list<<BAUDRATE; //as otherwise

    /* QProgressBar */
    QProgressDialog pdialog;
    pdialog.setWindowFlags(pdialog.windowFlags() & (~Qt::WindowContextHelpButtonHint)
                           & (~Qt::WindowCloseButtonHint));
    pdialog.setModal(true);
    QProgressBar pbar;
    pbar.setTextVisible(false);
    pdialog.setCancelButton(0);
    pdialog.setBar(&pbar);
    pdialog.setMaximum(baund_list.count()+1);
    pdialog.setValue(0);
    pdialog.setWindowTitle(QObject::tr("Connecting ..."));
    pdialog.repaint();

    /* Loop checking baund-list */
    int baund_now;
    char buff[40];
    ok = false;
    qDebug()<<"=======LOOP for serial baund!====";
    foreach (baund_now, baund_list) {
        pdialog.setValue(pdialog.value()+1);
        pdialog.setLabelText(QObject::tr("Try Baund-Rate: %1").arg(baund_now));
        pdialog.repaint();

        if(serial->isOpen())
            serial->close();
        serial->setBaudRate(baund_now);
        qDebug()<<"===Serial "<<baund_now<<"====";
        if(!serial->open(QIODevice::ReadWrite)){
            qDebug()<<"Cannot open";
        }
        serial->setDataTerminalReady(true);
        serial->clear();

        if(! serial->waitForReadyRead(BAUNDRATE_TIMEOUT * 1000)){
            qDebug()<<"NOTHING to read";
            continue;
        }
        else{
            while(serial->waitForReadyRead(50));
            serial->peek(buff, sizeof(buff));
            buff[strlen(ARC_FLAG)] = '\0';
            if(strcmp(buff, ARC_FLAG)==0){
                /* success read this baund */
                baund_pre = baund_now;
                ok = true;
                qDebug()<<">>>> Success in this!";
                return;
            }
            else{
                qDebug()<<"buff " <<QString(buff);
                continue;
            }
        }
    }
}


QString SERIALCHECK_PRIVATE::isReloadable(const QString portName, const QString descripe, bool &ok)
{
   QString com_reload("");
   ok = false;
   if(portName.isEmpty())
       return com_reload;
   foreach(QSerialPortInfo info, QSerialPortInfo::availablePorts()){
       QString info_portName = info.portName();
       QString info_descripe = info.description();
       if(info_portName==portName && info_descripe==descripe){
           /* portName && descripe */
           com_reload = info_portName;
           qDebug()<<"break info_portName"<<info_portName;
           ok = true;
           break;
       }
       else if(info_descripe==descripe && !info.isBusy()) {
           /* descripe */
           com_reload = info_portName;
           qDebug()<<"continue info_portName"<<info_portName;
           ok = true;
           continue;
       }
   }
   return com_reload;
}

SerialCheck::SerialCheck(QObject *parent,
                         QMenu * mn_parent,
                         QAction * ac_info,
                         QAction * ac_eject,
                         QAction * ac_reload)
    : QObject(parent), MN_parent(mn_parent), AC_info(ac_info),AC_eject(ac_eject),
      AC_reload(ac_reload), COM_baundRate(BAUDRATE)
{
    this->timerEventId = this->startTimer(EVERYTIMER);
    if(this->timerEventId == 0){
        qDebug()<<"Cannot create TimerEvent in SerialCheck";
        return;
    }
    this->COMs_info_pre = "";
    this->reload_portName = "";
    this->COM_obj = new QSerialPort(this->parent());
    this->COM_obj->setBaudRate(BAUDRATE); //250000, in serialcheck.h
    this->AC_reload->setEnabled(false);

    /* Menu Action connect*/
    connect(this->AC_reload,   SIGNAL(triggered()), this, SLOT(on_AC_reload()));
    connect(this->AC_info,     SIGNAL(triggered()), this, SLOT(on_AC_info()));
    connect(this->AC_eject,    SIGNAL(triggered()), this, SLOT(on_AC_eject()));
}
void SerialCheck::timerEvent(QTimerEvent * event)
{
    Q_UNUSED(event);
    const auto infos = QSerialPortInfo::availablePorts();
    QString s;
    for(const QSerialPortInfo &info : infos) {
        s = s + info.portName() + (info.isBusy() ? "1" : "0");
    }
    if(s == COMs_info_pre) {
        /******** 串口信息未改变 *******/
        return;
    }
    COMs_info_pre = s;
    qDebug()<<"COM info changed!";

    /******** 串口信息改变了 *******/
    /* 删除pre QAction */
    foreach(QAction *ac, MN_COM_cn) {
        ac->deleteLater();
    }
    MN_COM_cn.clear();
    /* 新建post QAction */
//    if(infos.isEmpty()){ /* 如果新的串口信息为空 */
//        return;
//    }
    foreach(QSerialPortInfo info, infos) {
        QString portName = info.portName();
        bool isBusy = info.isBusy();
        /* 连接 */
        QAction * com_cn = new QAction(portName, this->parent());
        com_cn->setEnabled(!isBusy);
        com_cn->setCheckable(true);
        com_cn->setChecked( portName==this->COM_obj->portName() );
        com_cn->setToolTip(info.description());
        connect(com_cn, SIGNAL(triggered()), this, SLOT(on_COM_cn()));
        MN_COM_cn << com_cn;
    }
    this->MN_parent->addActions(MN_COM_cn);

    /******** 加入 Reload提示 *******/
    if(!COM_obj->portName().isEmpty()) {
        /* 串口断开了 */
        bool ok;
        QString com_reload = isReloadable(COM_obj->portName(), COM_descripe, ok);
        qDebug()<<"com_reload"<<com_reload;
        if(!ok){
            /* 没有搜索到合适的串口 用来 reload */
            this->AC_reload->setEnabled(false);
            this->AC_reload->setText(tr("Reload"));
        }
        else{
            /* 搜索到合适的 reload串口 */
            this->AC_reload->setEnabled(true);
            this->AC_reload->setText(tr("Reload")+"("+com_reload+")");
            this->reload_portName = com_reload; /*刷新reload_portName*/
        }
    }

}
void SerialCheck::on_AC_eject()
{
    this->COM_obj->close();
    foreach(QSerialPortInfo info, QSerialPortInfo::availablePorts()){
        qDebug()<<info.portName()<<"is"<<(info.isBusy() ? "busy" : "idle");
    }
}
void SerialCheck::on_AC_reload()
{
    /* swith to reload_portName */
    bool ok_1;
    QString com_reload = isReloadable(reload_portName, COM_descripe, ok_1);
    if(!ok_1)
        return;
    this->reload_portName = com_reload;
    this->COM_obj->setPortName(this->reload_portName);

    /* open this port with auto detected BaundRate*/
    bool ok;
    autoBaundRate(this->COM_obj, this->COM_baundRate, ok);
}
void SerialCheck::on_AC_info()
{
    QDialog *a = new QDialog;
    QVBoxLayout layout;
    a->setWindowTitle(tr("Serial List"));
    foreach(QSerialPortInfo info, QSerialPortInfo::availablePorts()){
        QString s = QObject::tr("Port: ") + info.portName() + "\n"
                    + QObject::tr("Location: ") + info.systemLocation() + "\n"
                    + QObject::tr("Description: ") + info.description() + "\n"
                    + QObject::tr("Manufacturer: ") + info.manufacturer() + "\n"
                    + QObject::tr("Serial number: ") + info.serialNumber() + "\n"
                    + QObject::tr("Vendor Identifier: ") + (info.hasVendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : QString()) + "\n"
                    + QObject::tr("Product Identifier: ") + (info.hasProductIdentifier() ? QString::number(info.productIdentifier(), 16) : QString()) + "\n"
                    + QObject::tr("Busy: ") + (info.isBusy() ? QObject::tr("Yes") : QObject::tr("No")) + "\n";
        QLabel *label = new QLabel(s);
        if(info.portName() == this->COM_obj->portName()){
            label->setStyleSheet("background-color: #B1F1C3");
        }
        layout.addWidget(label);
    }
    a->setLayout(&layout);
    a->show();
}

void SerialCheck::commit_settings()
{
    bool isConnect = this->COM_obj->isOpen();
    QString port = this->reload_portName;
    QString describe = this->COM_descripe;
    emit takefrom_this(isConnect, port, describe);
}

void SerialCheck::giveto_this(bool isConnect, const QString &port, const QString &describe)
{
    this->reload_portName = port;
    this->COM_descripe = describe;
    if(isConnect){
        this->on_AC_reload();
    }
}

void SerialCheck::on_COM_cn()
{
    QAction * ac = (QAction * )sender(); /* sender(), SIGNAL obj, QObject */
    this->COM_obj->setPortName(ac->text());
    bool ok;
    autoBaundRate(this->COM_obj, this->COM_baundRate, ok);
    this->COM_descripe = QSerialPortInfo(*(this->COM_obj)).description();
    foreach(QAction *a, this->MN_COM_cn){
        a->setChecked(a==ac);
    }
}
