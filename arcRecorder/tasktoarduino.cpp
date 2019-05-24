#include <QDir>
#include <QComboBox>
#include <QMessageBox>
#include <QProcess>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QDebug>
#include <QTime>
#include <QPalette>
#include <QLabel>
#include <QTimer>
#include <QThread>
#include <QtSerialPort/QSerialPort>

#include "tasktoarduino.h"
#include "onlinemanagerbar.h"
#include "profilereader.h"

using namespace TASKTOARDUINO_PRIVATE;
using namespace TASKTOARDUINO_PARA;

QString TASKTOARDUINO_PARA::ARDUINO_DEBUG = "";
QString TASKTOARDUINO_PARA::ARDUINO_BOARD = "";

// [1] extern functions
QStringList getSubDirs(QDir); //in "onlinemanagerbar.h"
static void checkTaskrootPath(); //in [this].cpp
// [1] end

// [2] Diaglog class
Dialog::Dialog(QWidget *parent): QDialog(parent),
    isNearEnd(false), isLastCrushStr(false)
{
    /* init View */
    this->plainTextEdit = new QPlainTextEdit(this);
    this->progressBar = new QProgressBar(this);
    this->progressBar->setTextVisible(false);
    this->labelTitle = new QLabel(this);
    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->addWidget(this->labelTitle);
    layout->addWidget(this->plainTextEdit);
    layout->addWidget(this->progressBar);
    QHBoxLayout * layout2 = new QHBoxLayout(this);
    pushButton = new QPushButton(tr("Abort"), this);
    layout2->addStretch();
    layout2->addWidget(pushButton);
    layout->addLayout(layout2);
    this->setLayout(layout);
    this->setWindowTitle(tr("Upload ..."));
    this->cleanThis();
    /* add Signal */
    connect(pushButton, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(this, SIGNAL(finished(int)), this, SLOT(cleanThis()));
    connect(this, SIGNAL(rejected()), this, SIGNAL(abort_click()));
}
void Dialog::cleanThis()
{
    this->plainTextEdit->clear();
    this->progressBar->setValue(5);
    this->progressBar->setStyleSheet("");
    this->isNearEnd = this->isLastCrushStr = false;
    this->labelTitle->setText(tr("Uploading task to Board"));
    this->pushButton->setText(tr("Abort"));
}
void Dialog::finishThis()
{
    this->progressBar->setValue(100);
    this->pushButton->setText(tr("OK"));
}

void Dialog::appendData(const QString &arg1)
{   //progress middle
    this->plainTextEdit->appendPlainText(arg1.trimmed() + "\n");
    this->isNearEnd = true;
    this->progressBar->setValue(90);
}
void Dialog::appendErr(const QString &arg1)
{
    if(this->isLastCrushStr ||
       arg1.contains("error:")|| arg1.contains(tr("error")) ||
       arg1.contains("can't") || arg1.contains(tr("can't")) ||
       arg1.contains("Problem") || arg1.contains(tr("Problem")) ||
       arg1.contains("exit status 1") || arg1.contains(tr("exit status 1")) ||
       arg1.contains(" not ") || arg1.contains(tr(" not ")) )  {
        // 抓取报错信息
        QString str = QString("<p><font color=\"#FF0000\">%1</p></td>").arg(arg1.trimmed()); //红
        this->plainTextEdit->appendHtml(str);
        this->progressBar->setValue(100);
        QString danger = "QProgressBar::chunk {background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0,stop: 0 #FF0350,stop: 0.4999 #FF0020,stop: 0.5 #FF0019,stop: 1 #FF0000 );}";
        this->progressBar->setStyleSheet(danger);
        this->isLastCrushStr = true;
    }
    else if (this->isNearEnd) {
        // progress end
        QString str = QString("<p><font color=\"#0000FF\">%1</p></td>").arg(arg1.trimmed()); //蓝
        this->plainTextEdit->appendHtml(str);
        this->progressBar->setValue(this->progressBar->value()+10);
    }
    else {
        // progress beginning
        this->plainTextEdit->appendPlainText(arg1.trimmed());
        this->progressBar->setValue(qMin(this->progressBar->value()+10, 80));
    }
}
void Dialog::setLabelTitle(const QString &arg1)
{
    this->labelTitle->setText(arg1);
}
// [2] end Dialog class


// [3] TasktoArduino class
TasktoArduino::TasktoArduino(QObject *parent,
                             QComboBox *cob_tasks,
                             const QSerialPort *mserial)
    : QObject(parent), COB_tasks(cob_tasks), serial(mserial),
      process(new QProcess), dialog(new Dialog), processTimeout(new QTimer)
{
    connect(this->COB_tasks, SIGNAL(activated(QString)), this, SLOT(on_COB_tasks_activate(QString)));
    checkTaskrootPath();
    this->refresh_tasklist();
    this->processTimeout->setInterval(TIMEOUT*1000);
    this->processTimeout->setSingleShot(true);

    /* 连接信号与槽 */
    connect(this->process, SIGNAL(readyReadStandardOutput()), this, SLOT(when_stdout_ready()));
    connect(this->process, SIGNAL(readyReadStandardError()), this, SLOT(when_stderr_ready()));
    connect(this->process, SIGNAL(finished(int)), this, SLOT(when_process_finished()));
    connect(this, SIGNAL(send_stdout(QString)), this->dialog, SLOT(appendData(QString)));
    connect(this, SIGNAL(send_stderr(QString)), this->dialog, SLOT(appendErr(QString)));
    connect(this->dialog, SIGNAL(abort_click()), this, SLOT(want_process_kill()));
    connect(this, SIGNAL(process_timeend()),    this->dialog, SLOT(finishThis()));
    connect(this->processTimeout, &QTimer::timeout, [=](){this->when_process_timeout();}); //不能直接用SLOT!!??
}
void TasktoArduino::when_stdout_ready()
{
    const QString str = QString::fromLocal8Bit(this->process->readAllStandardOutput());
    emit send_stdout(str);
}
void TasktoArduino::when_stderr_ready()
{
    const QString str = QString::fromLocal8Bit(this->process->readAllStandardError());
    emit send_stderr(str);
}
void TasktoArduino::want_process_kill()
{
    if (this->process->state()!=QProcess::NotRunning) {
        this->process->kill();
    }
}

void TasktoArduino::when_process_finished()
{
    emit raise_reload();
    this->processTimeout->stop();
    if(process->exitStatus()==QProcess::NormalExit  && process->exitCode()==0) {
        /* success to upload */
        emit process_timeend();
        QThread::msleep(200);
        dialog->close();
    }
    else if(process->exitStatus()==QProcess::NormalExit){
        /* error happend */
        emit send_stderr("error: fail to upload!");
    }
    else{
        /*process->kill */
        qDebug()<<"ExitStatus str"<<process->exitStatus();
    }
}

void TasktoArduino::when_process_timeout()
{
    /*Dialog will parse it as an error */
    emit send_stderr("An error: time out!");

    /* kill process */
    if (this->process->state()!=QProcess::NotRunning) {
        this->process->kill();
    }
}

QString TasktoArduino::taskrootPath()
{
    return QDir::currentPath()+"/task"; //taskrootPath = "C:/a/b/myexe/task"
}

void TasktoArduino::refresh_tasklist()
{
    /* make up ComboBox*/
    this->COB_tasks->clear();
    this->COB_tasks->addItem(QString(""));
    this->COB_tasks->addItems(getSubDirs(QDir(this->taskrootPath())));
    this->COB_tasks->addItem(tr(REFRESH_HINT));
    this->COB_tasks->setCurrentIndex(0);
}
void TasktoArduino::on_COB_tasks_activate(const QString &arg1)
{
    QString portName = this->serial->portName();
    if(arg1.isEmpty()){ /* nothing selected */
        return;
    }
    else if(arg1==REFRESH_HINT){ /* <Refresh> selected */
        this->refresh_tasklist();
    }
    else if(portName.isEmpty()) {
        QMessageBox::critical(0, tr("Error"), tr("None Serial-Port selected!"));
    }
    else { /* prepare to DOWNLOAD task -> Arduino */
        getFromProfile();  //update "ARDUINO_DEBUG" & "ARDUINO_BOARD"
        QString taskroot=TasktoArduino::taskrootPath();
        QFileInfo f(taskroot + "/" + arg1 + "/" + arg1 + ".ino");
        if(f.exists() && f.isFile() && f.isReadable()) {
            this->dialog->setLabelTitle(tr("Uploading %1 to %2@(%3)").arg(arg1).arg(ARDUINO_BOARD.toUpper()).arg(portName));
            this->upload_task(f.absoluteFilePath());
        }
        else {
            QMessageBox::critical(0, tr("Error"), tr("None invalid task-file exsits!"));
        }
    }
}
void TasktoArduino::upload_task(const QString &arg1)
{
    /* 从配置文件中读取 ARDUINO_DEBUG */
    QFileInfo fi(ARDUINO_DEBUG);
    try{
        SCPP_ASSERT_THROW(!ARDUINO_DEBUG.isEmpty(), "None arduino-debug exist");
        SCPP_ASSERT_THROW(fi.exists()&&fi.isFile(), tr("Invaild arduino-debuger %1").arg(fi.filePath()));
    }
    catch(...){
        QMessageBox::information(0, "Info", tr("No [arduino-debuger] finded, please pick it & retry"));
        return;
    }
    /* 开始烧录 */
    emit raise_eject();
    QStringList arglist;
    arglist <<"--board"<<"arduino:avr:"+ARDUINO_BOARD
            <<"--port"<<this->serial->portName()
            <<"--upload"<<arg1;
    qDebug()<<ARDUINO_DEBUG<<arglist;

    this->want_process_kill();
    QProcess * p = this->process;
    p->setProgram(ARDUINO_DEBUG);
    p->setArguments(arglist);

    /*  Set timeout trigger */
    this->processTimeout->start();

    p->start();
    dialog->setModal(true);
    dialog->show();
    qDebug()<<"Begin Uploading!";
}
// [3] end

static void checkTaskrootPath()
{
    QString taskroot=TasktoArduino::taskrootPath();
    QFileInfo f(taskroot);
    if(f.exists() && f.isDir()){
        return;
    }
    else {
        QDir d;
        if(d.mkpath(taskroot))
            qDebug()<<"cannot create path";
    }
}

// [4]
void TASKTOARDUINO_PRIVATE::getFromProfile()
{
    ARDUINO_DEBUG = ProfileReader::getInstance()->getArduino();
    ARDUINO_BOARD = ProfileReader::getInstance()->getBoard();
}
// [4] end
