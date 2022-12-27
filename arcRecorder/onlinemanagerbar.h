#ifndef ONLINEMANAGERBAR_H
#define ONLINEMANAGERBAR_H
#include <QObject>
#include "main.h"

class QPushButton;
class QDir;
class QFile;
class QComboBox;
class QLineEdit;
class QTime;
static const char ADDNEW_HINT[] = "<add new>";
static const char startBG[] = "background-color: #B1F1C3";
static const char stopBG[]= "background-color: white";
QStringList getSubDirs(QDir dir0);
class OnlineManagerBar : public QObject
{
    Q_OBJECT
public:
    explicit OnlineManagerBar(QObject *parent,
                              QComboBox * cob_proj,
                              QComboBox * cob_subj,
                              QLineEdit * le_time,
                              QPushButton * btn_start);
    bool isStarted();
    bool isConnected();
    static QString datarootPath();
    void timerEvent(QTimerEvent *);
    void refreshdir();
    QPushButton * BTN_start;       //pushbutton-start

private:
    QComboBox * COB_proj;          //combobox-project
    QComboBox * COB_subj;          //combobox-subject
    QLineEdit * LE_time;           //lineedit-time flow
    bool        startstuts;        //is system started
    bool        connectstuts;      //is COM connected, "start" btn avaliable
    QTime     * myTime;            //time flow afer "start" click
    qint32      timestop;          //time to stop after "start" click
    int         timerEventId;      //QObject->timer identifier,
signals:
    void start();                  //menuly click to start
    void stop();                   //menuly
    void startstop(const bool &);  //menuly
    void raise_press_start();      //just notify, must receive reponse
    void raise_press_stop();       //no need to receive response
    void changeFdir(QString &arg1);//when changed dir for file
public slots:
    void startme();                //when arduino Hardware-trigger-start
    void stopme();                 //when arduino run end
    void on_COB_proj_activated(const QString &arg1);
    void on_COB_subj_activated(const QString &arg1);
    void on_BTN_start_clicked();
    void when_serial_isconnected(bool); //when Serial port is connected
    void when_spont_start();
    void when_spont_stop();
    void when_press_start();        //emit raise_press_start()
    void when_press_stop();         //emit when_press_stop()

private slots:
    void on_myTime_start();
    void on_myTime_stop();

signals:
    void takefrom_this(const QString & upper, const QString & lowwer);
public slots:
    void commit_settings();// emit takefrom_this()
    void giveto_this(const QString & upper, const QString & lowwer);
};

#endif // ONLINEMANAGERBAR_H
