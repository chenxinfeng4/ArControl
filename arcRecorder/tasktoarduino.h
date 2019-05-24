#ifndef TASKTOARDUINO_H
#define TASKTOARDUINO_H

#include <QObject>
#include <QDialog>
#include <QWidget>

#include "main.h"

class QComboBox;
class QSerialPort;
class QProcess;
class QPlainTextEdit;
class QProgressBar;
class QLabel;
class QPushButton;
class QTimer;

namespace TASKTOARDUINO_PRIVATE
{
    class Dialog : public QDialog
    {
        Q_OBJECT
    public:
        explicit Dialog(QWidget * parent = 0);
        QLabel        * labelTitle;
        QPlainTextEdit* plainTextEdit;
        QProgressBar  * progressBar;
        QPushButton   * pushButton;
        bool            isNearEnd;
        bool            isLastCrushStr;
    signals:
        void abort_click();
    public slots:
        void cleanThis();
        void finishThis();
        void setLabelTitle(const QString &arg1);
        void appendData(const QString &arg1);
        void appendErr(const QString &arg1);
    };
    void getFromProfile();
}
namespace TASKTOARDUINO_PARA
{
    const char REFRESH_HINT[] = "<Refresh>";
    extern QString ARDUINO_DEBUG;
    extern QString ARDUINO_BOARD;
    const int  TIMEOUT = 40;
}
class TasktoArduino : public QObject
{
    Q_OBJECT
public:
    explicit TasktoArduino(QObject *parent,
                           QComboBox *cob_tasks,
                           const QSerialPort *mserial);
    QComboBox         *COB_tasks; // list of task
    const QSerialPort *serial;    // serial obj

    static QString taskrootPath();
    QProcess * process;
    TASKTOARDUINO_PRIVATE::Dialog * dialog;
    QTimer   * processTimeout;
signals:
    void raise_eject();
    void raise_reload();
    void process_timeend();
    void send_stdout(const QString &);
    void send_stderr(const QString &);

public slots:
    void refresh_tasklist();
    void upload_task(const QString &);
    void on_COB_tasks_activate(const QString &);
    void when_stdout_ready();
    void when_stderr_ready();
    void when_process_finished();
    void want_process_kill();
    void when_process_timeout();
};

#endif // TASKTOARDUINO_H
