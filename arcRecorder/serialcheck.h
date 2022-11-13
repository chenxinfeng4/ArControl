#ifndef SERIALCHECK_H
#define SERIALCHECK_H

#include <QObject>

#include "main.h"

class QTime;
class QMenu;
class QAction;
class QSerialPort;

namespace SERIALCHECK_PARA
{
    const int EVERYTIMER = 2000; //1000ms
    const int BAUDRATE   = 250000; //9600
    const int BAUNDRATE_TIMEOUT = 2; //2sec
    const char ARC_FLAG[]  = "ArControl";
}
namespace SERIALCHECK_PRIVATE
{
    void autoBaundRate(QSerialPort *serial, int &baund_pre, bool &ok);
    QString isReloadable(const QString portName, const QString descripe, bool &ok);
}

class SerialCheck : public QObject
{
    Q_OBJECT
public:
    explicit SerialCheck(QObject *parent,
                         QMenu * mn_parent,
                         QAction * ac_info,
                         QAction * ac_eject,
                         QAction * ac_reload);
    void timerEvent(QTimerEvent *);
    QSerialPort   * COM_obj;       // COM object
private:
    QMenu         * MN_parent;     // parent for all NEXT MENU & ACTION
    QAction       * AC_info;       // infomation for coms
    QAction       * AC_eject;      // eject current
    QAction       * AC_reload;     // eject then load current
    QList<QAction *> MN_COM_cn;    // connectable COM
    int             timerEventId;  // save timer event id
    QString         COM_descripe;  // COM description, which use to RELOAD
    QString         reload_portName;//portName for reload;
    QString         COMs_info_pre; // save COM info, if nothing changed
    int             COM_baundRate; // last success baundrate
signals:
    void COM_obj_change(QSerialPort *); //(prt = NULL) == COM_obj_idle()
    void COM_obj_idle();           // NO COM work
public slots:
    void on_COM_cn();
    void on_AC_eject();
    void on_AC_reload();
    void on_AC_info();

signals:
    void takefrom_this(bool isConnect, const QString & port, const QString & describe); //this-> file
public slots:
    void commit_settings(); //emit takefrom_this()
    void giveto_this(bool isConnect, const QString & port, const QString & describe); //file-> this
};

#endif // SERIALCHECK_H
