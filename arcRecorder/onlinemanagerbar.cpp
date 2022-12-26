#include "onlinemanagerbar.h"

#include <QPushButton>
#include <QDir>
#include <QComboBox>
#include <QLineEdit>
#include <QFileInfo>
#include <QDebug>
#include <QInputDialog>
#include <QTime>
#include <QThread>
static void checkDatarootPath(); //local scope in .cpp
QStringList getSubDirs(QDir  dir0);        //local scope in .cpp
OnlineManagerBar::OnlineManagerBar(QObject *parent,
                                   QComboBox * cob_proj,
                                   QComboBox * cob_subj,
                                   QLineEdit * le_time,
                                   QPushButton * btn_start)
    : QObject(parent),  COB_proj(cob_proj), COB_subj(cob_subj),
      LE_time(le_time), BTN_start(btn_start),
      startstuts(false), connectstuts(false),
      myTime(new QTime()), timestop(0)
{
    /* make up */
    checkDatarootPath();
    this->COB_proj->clear();
    this->COB_proj->addItems(getSubDirs(QDir(this->datarootPath())));
    this->COB_proj->addItem(ADDNEW_HINT); //MACRO in onlinemanagerbar.h, "<add new>"
    this->COB_subj->clear();
    if(this->COB_proj->count()>0)
        this->on_COB_proj_activated(this->COB_proj->currentText());
    this->LE_time->setStyleSheet(stopBG);
    this->BTN_start->setEnabled(false);
    /* connect SINGAL with SLOT */
    connect(COB_proj, SIGNAL(activated(QString)), this, SLOT(on_COB_proj_activated(QString)) );
    connect(COB_subj, SIGNAL(activated(QString)), this, SLOT(on_COB_subj_activated(QString)) );
    connect(BTN_start,SIGNAL(clicked()), this, SLOT(on_BTN_start_clicked()));
    connect(this,     SIGNAL(start()),   this, SLOT(on_myTime_start()));
    connect(this,     SIGNAL(stop()),    this, SLOT(on_myTime_stop()));

    qDebug()<<"Onlineguibar====QThread"<<this->thread();
}
void OnlineManagerBar::on_COB_proj_activated(const QString &arg1)
{
    this->COB_subj->clear();    /* subject clear */
    if(arg1.isEmpty())          /* nothing selected */
        return;
    if(arg1==ADDNEW_HINT){      /* <add new> selected */
        bool ok;
        QString text = QInputDialog::getText(0, tr("Project"), tr("Add a new item:"),
                                             QLineEdit::Normal,"proj_1", &ok);
        if (ok && !text.isEmpty()){ /*have input */
            QDir dir(OnlineManagerBar::datarootPath());
            if(dir.mkdir(text)) {  /*create project dir */
                int currentInd = this->COB_proj->count()-1;
                this->COB_proj->insertItem(currentInd, text);
                this->COB_proj->setCurrentIndex(currentInd);
            }
            else {                 /*create project dir -> fail */
                qDebug()<<"创建"<<text<<"失败!";
                return;
            }
        }
    }

    /* watch for subject */
    QDir dir2(OnlineManagerBar::datarootPath());
    if(dir2.cd(this->COB_proj->currentText())){ /*proj available*/
        this->COB_subj->addItems(getSubDirs(dir2));
        this->COB_subj->addItem(ADDNEW_HINT);
        if(this->COB_subj->count()>1){
            this->COB_subj->setCurrentIndex(0);
            this->on_COB_subj_activated(this->COB_subj->currentText());
        }
    }
    else{
        qDebug()<<"cannot entry dir";
    }

}

void OnlineManagerBar::on_COB_subj_activated(const QString &arg1)
{
    if(arg1.isEmpty())          /* nothing selected */
        return;
    if(arg1==ADDNEW_HINT){      /* <add new> selected */
        bool ok;
        QString text = QInputDialog::getText(0, tr("Subject"), tr("Add a new item:"),
                                             QLineEdit::Normal,"subj_1", &ok);
        if (ok && !text.isEmpty()){ /*have input */
            QDir dir(OnlineManagerBar::datarootPath());
            if(dir.cd(this->COB_proj->currentText()) && dir.mkdir(text)) {  /*create subject dir */
                int currentInd = this->COB_subj->count()-1;
                this->COB_subj->insertItem(currentInd, text);
                this->COB_subj->setCurrentIndex(currentInd);
            }
            else {                 /*create subject dir -> fail */
                qDebug()<<"创建"<<text<<"失败!";
                return;
            }
        }
    }
    this->refreshdir(); //emit changeFdir
}

void OnlineManagerBar::refreshdir()
{
    QString datadir = (OnlineManagerBar::datarootPath() + "/"
              + this->COB_proj->currentText() + "/"
              + this->COB_subj->currentText());
    emit changeFdir(datadir);
    return;
}

void OnlineManagerBar::on_BTN_start_clicked()
{
    bool startstus_next = !startstuts; //this->startstuts
    if(startstus_next){ 
        this->when_press_start();
    }
    else{
        this->when_press_stop();
    }
}
void OnlineManagerBar::when_serial_isconnected(bool isconnected)
{
    this->connectstuts = isconnected;
    if(isconnected){           /*connected to serial */
        this->BTN_start->setEnabled(true);
    }
    else if(!this->startstuts){ /*no serial & not during RECORDING */
        this->BTN_start->setEnabled(false);
    }
}

void OnlineManagerBar::when_spont_start()
{
    this->startme();
}
void OnlineManagerBar::when_spont_stop()
{
    this->stopme();
}
void OnlineManagerBar::when_press_start()
{
    emit raise_press_start();
//    this->startme(); //wait for when_spont_start
}
void OnlineManagerBar::when_press_stop()
{
    emit raise_press_stop();
    this->stopme();
}

void OnlineManagerBar::startme() {
    bool cond = true; //预先条件
    if(!cond){
        return;
    }
    startstuts = !startstuts;
    emit(start());
    emit(startstop(startstuts));
    /* 各个控件样式改变 */
    this->BTN_start->setText(tr("STOP"));
    this->COB_proj->setEnabled(false);
    this->COB_subj->setEnabled(false);
    this->LE_time->setStyleSheet(startBG);
}
void OnlineManagerBar::stopme()
{
    bool cond = true;  //预先条件
    if(!cond){
        return;
    }
    startstuts = !startstuts;
    emit(stop());
    emit(startstop(startstuts));

    /* 各个控件样式改变 */
    this->BTN_start->setText(tr("START"));
    this->COB_proj->setEnabled(true);
    this->COB_subj->setEnabled(true);
    this->LE_time->setStyleSheet(stopBG);
}

bool OnlineManagerBar::isStarted()
{
    return this->startstuts;
}

bool OnlineManagerBar::isConnected()
{
    return this->connectstuts;
}

QString OnlineManagerBar::datarootPath()
{
    return QDir::currentPath()+"/data"; //dataroot =  "C:/a/b/myexe/data"
}
void OnlineManagerBar::on_myTime_start()
{
    /* time 计时与倒计时 */
    this->myTime->restart();
    QTime t = QTime::fromString(this->LE_time->text(), "h:m:s");
    if(t.isValid() && t.msecsSinceStartOfDay()!=0){
        t.setHMS(t.hour(),t.minute(),t.second());
        qDebug()<<"超时时间"<<t;
    }
    else{ //数据错误或者 00:00:00 -> 23:59:59
        t.setHMS(23, 59, 59);
        qDebug()<<"设定为永不超时";
    }
    this->timestop = t.msecsSinceStartOfDay();
    this->LE_time->setReadOnly(true);
    this->timerEventId = this->startTimer(200); /*内置函数 每0.2sec运算一次 */
    if(this->timerEventId == 0){
        qDebug()<<"Cannot create TimerEvent";
        return;
    }

}
void OnlineManagerBar::on_myTime_stop() /* 定时器触发 结束 */
{
    this->killTimer(this->timerEventId);
    this->LE_time->setText("00:00:00");
    this->LE_time->setReadOnly(false);
    this->LE_time->setStyleSheet(stopBG);
    qDebug()<<"stop myTimer";
}

void OnlineManagerBar::commit_settings()
{
    QString upper = this->COB_proj->currentText();
    QString lowwer = this->COB_subj->currentText();
    emit takefrom_this(upper, lowwer);
}

void OnlineManagerBar::giveto_this(const QString &upper, const QString &lowwer)
{
    /* find in upper ComboBox */
    int c_proj = this->COB_proj->count();
    bool find_proj = false;
    int i;
    for(i=0; i<c_proj; ++i)
        if(COB_proj->itemText(i) == upper){
            find_proj = true;
            break;
        }
    COB_proj->setCurrentIndex(find_proj?i:0);

    QString upper_now = COB_proj->currentText();
    if(upper_now==ADDNEW_HINT){
        COB_subj->clear();
        return;
    }
    this->on_COB_proj_activated(upper_now);
    /* find in lowwer ComboBox */
    int c_subj = this->COB_subj->count();
    bool find_subj = false;
    for(i=0; i<c_subj; ++i)
        if(COB_subj->itemText(i) == lowwer){
            find_subj = true;
            break;
        }
    COB_subj->setCurrentIndex(find_subj?i:0);
    QString lowwer_now = COB_subj->currentText();
    if(lowwer_now==ADDNEW_HINT)
        return;
    else
        this->on_COB_subj_activated(lowwer_now);
}

void OnlineManagerBar::timerEvent(QTimerEvent *event) /* 定时触发 */
{
    Q_UNUSED(event);
    int tflow = this->myTime->elapsed();
    QString str=QTime::fromMSecsSinceStartOfDay(tflow).toString("hh:mm:ss");
    this->LE_time->setText(str);

    /* 是否该达到倒计时终点 */
    if( tflow > this->timestop ) {
       when_press_stop();
    }
}

static void checkDatarootPath()
{
    QString dataroot=OnlineManagerBar::datarootPath();
    QFileInfo f(dataroot);
    if(f.exists() && f.isDir()){
        return;
    }
    else {
        QDir d;
        if(d.mkpath(dataroot))
            qDebug()<<"cannot create path";
        d.mkpath(dataroot+"/chen/any");
    }
}
QStringList getSubDirs(QDir dir0)
{
    QStringList strlist;
    foreach(QFileInfo dirnow, dir0.entryInfoList()){
        if(!dirnow.isDir() || dirnow.isHidden()) //文件和 ./  ../ 都排除了
            continue;
        QString dirFileName = dirnow.fileName();
        if(dirFileName=="." || dirFileName=="..")
            continue;
        strlist<<dirFileName;
    }
    return strlist;
}
