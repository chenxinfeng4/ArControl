#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QPushButton>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QClipboard>
#include <QApplication>
#include <QThread>
#include <QDockWidget>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QDir>

#include "onlinemanagerbar.h"
#include "serialflowcontrol.h"

namespace SERIALFLOWCONTROL_PRIVATE
{
    void Worker::doWork(QSerialPort *serial, const bool isConnect)
    {
        if(isConnect) {
            while(serial->canReadLine()) {
                QString str = serial->readLine();
                emit readLine(str);
            }
        }
        else {
            /*未成功连接上 (正常断开close，或异常拔出) */
        }
    }
}
using namespace SERIALFLOWCONTROL_PRIVATE;
using namespace SERIALFLOWCONTROL_PARA;

SerialFlowControl::SerialFlowControl(QObject     *parent,
                                     QDockWidget *dw_container,
                                     QLineEdit   *le_send,
                                     QPlainTextEdit *pte_data,
                                     QCheckBox   *ckb_freeze,
                                     QCheckBox   *ckb_autoc,
                                     QPushButton *btn_send,
                                     QPushButton *btn_clear,
                                     QPushButton *btn_copy,
                                     QSerialPort *mserial)
    : QObject(parent), DW_container(dw_container), LE_send(le_send), PTE_data(pte_data),
      CKB_freeze(ckb_freeze), CKB_autoc(ckb_autoc), BTN_send(btn_send),
      BTN_clear(btn_clear), BTN_copy(btn_copy), serial(mserial), isConnect(false),
      isConnect_pre(false),isStarted(false),datafile(new QFile),datadir("."),
      datahash(new QHash<QString, StreamItem>)
{

    /* 定时创建任务 */
    if(!this->startTimer(100)) { //100ms
        qDebug()<<"创建失败";
    }
    /* 连接信号与槽 worker */
    Worker * worker = new Worker();
    worker->moveToThread(QApplication::instance()->thread());
    connect(this, &SerialFlowControl::wantto_readLine, worker, &Worker::doWork);
    connect(worker, SIGNAL(readLine(QString)), this, SLOT(receive_readLine(QString)));
    /* 连接信号与槽 */
    connect(this->BTN_copy, SIGNAL(clicked()), this, SLOT(on_BTN_copy_clicked()));
    connect(this->BTN_send, SIGNAL(clicked()), this, SLOT(on_BTN_send_clicked()));
    connect(this->LE_send, SIGNAL(returnPressed()), this, SLOT(on_BTN_send_clicked()));
    connect(this->serial, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(on_serial_error(QSerialPort::SerialPortError)));
    connect(this->serial, SIGNAL(aboutToClose()), this, SLOT(on_serial_error()));
    connect(this->PTE_data,SIGNAL(blockCountChanged(int)), this, SLOT(on_PTE_data_count(int)));
    connect(this, SIGNAL(raise_isconnect(bool)), this, SLOT(on_isconnect(bool)));
    connect(this,SIGNAL(raise_spont_start()), this, SLOT(when_real_start()));
    connect(this,SIGNAL(raise_spont_stop()), this, SLOT(when_real_stop()));
    this->DW_container->setEnabled(this->isConnect);
    /* 用stream托管文件，以创建utf-8 */
    this->datastreamout.setDevice(this->datafile);
    this->datastreamout.setCodec("utf-8");

    /* create datetime */
    this->dateTime = QDateTime::currentDateTime();
}

void SerialFlowControl::calDataHot()
{
    /* 核实 开始task记录？  */
    if(!this->isStarted)
        return;
    const int redrawCount = 100;
    /* 逐个 item 计算 count_pre 和 delta */
    QHash<QString, StreamItem>::iterator i;
    for (i = (*datahash).begin(); i != (*datahash).end(); ++i) {
        StreamItem &iV = i.value(); //iV, 引用同一地址
        qint32 delta = qMax(iV.count - iV.count_pre, 0);
        iV.count_pre = iV.count;

        /* 开始判断热度 */
        //delta >=1 ===>>> iv.hot = 100 - 1;
        //delta ==0 ===>>> iv.hot = iv.hot - 1;
        //delta < 0 , 不存在
        //0<= iv.hot <=99
        iV.hot = qBound(0, redrawCount * delta + iV.hot - 1, redrawCount - 1);// --iv.hot & refresh ==>>> 0 ~ 99,
//        qDebug()<<"====热度===="<<i.key()<<i.value().hot;
    }

}

bool SerialFlowControl::getStarted() const
{
    return this->isStarted;
}

const QHash<QString, StreamItem> *SerialFlowControl::get_datahash()
{
    return this->datahash;
}
void SerialFlowControl::init_datahash()
{
    this->datahash->clear();
    for(int i=1; i<=20; i++){
        (*datahash)[QString("IN%1").arg(i)] = {0, 0, 0 ,0};
        (*datahash)[QString("OUT%1").arg(i)] = {0, 0, 0 ,0};
    }
}

void SerialFlowControl::on_PTE_data_count(int lines)
{
    if(this->CKB_autoc->isChecked() && lines>=MAX_LINES){
        this->PTE_data->clear();
    }
}

void SerialFlowControl::timerEvent(QTimerEvent * event)
{
    Q_UNUSED(event);
    emit wantto_readLine(this->serial, this->isConnect);
    if(this->isConnect && this->isStarted){emit raise_everyTimeCycle();}
    this->calDataHot();
}

void SerialFlowControl::receive_readLine(const QString & str)
{
    /* Show in Text-Flow-Window
     filter some format string, not shown to the screen */
    if(!str.startsWith(DATA_OUT_HIGH_CHAR) && !str.startsWith(DATA_IN_HIGH_CHAR)){
        this->PTE_data_buff.append(str);
        if(!this->CKB_freeze->isChecked()){ /* free to show flowing data */
            this->PTE_data->appendPlainText(this->PTE_data_buff.trimmed());//默认处理尾巴换行样式
            this->PTE_data_buff.clear();
        }
    }

    /* File: header parts */
    if (this->isStarted == false) {
        if (str.contains(BG_HEADER_STRING)) {
            file_header.clear();
            file_header += str;
        }
        else if(!str.contains(BG_FLOW_STRING)){
            file_header += str;
        }
    }

    /* 是否是开始or结束的信号 */
    if (this->isStarted) {
        /* 把数据流写入文件 */
        this->datastreamout << str;  //write "ArC-end" to file bg
        /* 加入datahash */
        int ind = str.indexOf(QChar(DATA_HINT_CHAR));
        if(ind>=1){
            /* 承认这是个合理的data行，则data/item +=1 */
            QString itemName = str.left(ind);
            ++(*datahash)[itemName].count;
            qDebug()<<itemName<<(*datahash)[itemName].count;
        }
        if(str.startsWith(DATA_IN_HIGH_CHAR)){
            /* IN 通道处于 HIGH */
            QString itemName = QString("IN%1").arg(str.right(2).trimmed()); //"IN1"
            (*datahash)[itemName].ishigh = true;
        }
        else if(str.startsWith(DATA_OUT_HIGH_CHAR)){
            /* OUT 通道处于 HIGH */
            QString itemName = QString("OUT%1").arg(str.right(2).trimmed());//"OUT1"
            (*datahash)[itemName].ishigh = true;
        }
        else if(ind>=1 && str.startsWith("IN")){
            /* IN 通道处于 LOW */
            QString itemName = QString("IN%1").arg(str.mid(2, ind-2)); //"IN1"
            (*datahash)[itemName].ishigh = false;
        }
        else if(ind>=1 && str.startsWith("OUT")){
            /* OUT 通道处于 LOW */
            QString itemName = QString("OUT%1").arg(str.mid(3, ind-3)); //"OUT1"
            (*datahash)[itemName].ishigh = false;
        }
        /* 已经开始，等待硬件自动结束的信号 "ArC-end"*/
        if(str.contains(END_FLOW_STRING)){
            emit raise_spont_stop();
        }
    }
    else if(str.contains(BG_FLOW_STRING)){
        /* 已经结束，等待开始的信号 "ArC-bg"*/
        emit raise_spont_start();

        this->datastreamout << this->file_header;
        this->datastreamout << str; //write "ArC-bg" to file bg
    }
}

void SerialFlowControl::giveto_this(bool isVisible, bool isDock, bool isAutoS, bool isFreeze)
{
    this->DW_container->setVisible(isVisible);
    this->DW_container->setFloating(!isDock);
    this->CKB_autoc->setChecked(isAutoS);
    this->CKB_freeze->setChecked(isFreeze);
}

void SerialFlowControl::commit_settings()
{
    bool isVisible = this->DW_container->isVisible();
    bool isDock = !(this->DW_container->isFloating());
    bool isAutoS = this->CKB_autoc->isChecked();
    bool isFreeze = this->CKB_freeze->isChecked();
    emit takefrom_this(isVisible, isDock, isAutoS, isFreeze);
}

void SerialFlowControl::on_serial_error(QSerialPort::SerialPortError err)
{
    qDebug()<<"Serial Status: "<<err;
    if(err==QSerialPort::NoError) {
        this->isConnect=true;
    }
    else if (err==QSerialPort::NotOpenError){
        /* 再次打开 或 再次关闭 */
        //忽略
    }
    else if (err==QSerialPort::TimeoutError){
        /* 正在调试串口波特率时 */
        //忽略
    }
    else{
        this->isConnect=false;
    }
    bool isconnect = this->isConnect;
    if(isconnect!=this->isConnect_pre) {
        this->isConnect_pre = isconnect;
        emit raise_isconnect(isconnect);
    }

}
void SerialFlowControl::on_isconnect(const bool & isconnect)
{
    if(isconnect){
        /* 刚建立连接 */
        this->PTE_data->clear();
        this->PTE_data_buff.clear();
        this->DW_container->setEnabled(true);
        this->init_datahash();
    }
    else{
        this->DW_container->setEnabled(false);
    }
}
void SerialFlowControl::when_dataDir_change(QString &arg1)
{
    this->datadir = arg1;
    qDebug()<<"get data dir"<<arg1;
}

void SerialFlowControl::on_BTN_send_clicked()
{
    QString strsend = this->LE_send->text();
    this->serial->write(strsend.toLatin1());
    this->LE_send->clear();
}

void SerialFlowControl::on_BTN_copy_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(this->PTE_data->toPlainText());
}
void SerialFlowControl::on_BTN_clear_clicked()
{
    this->PTE_data->clear();
}
void SerialFlowControl::when_press_start()
{
    /* clean read & write buffer */
    /* 需要等到硬件反馈到 "ArC" 信号 */
    if(this->isConnect){
        serial->write(MY_STRBEGIN);
        this->dateTime = QDateTime::currentDateTime();
    }else{
        qDebug()<<"No Connection!";
    }
    if(this->isStarted){
         /*已经开始了 */
        qDebug()<<"Already Start!";
    }
}

void SerialFlowControl::when_press_stop()
{
    this->when_real_stop();
}
void SerialFlowControl::when_real_start()
{
    /* 建立文件 */
    if (this->datafile->isOpen()) {
        this->datafile->close();
    }

    QDateTime now = QDateTime::currentDateTime();
    if (this->dateTime.secsTo(now)>2){
        this->dateTime = now;
    }
    QString fname = this->dateTime.toString("yyyy-MMdd-HHmmss-zzz") + ".txt";
    QString pfname = this->datadir + "/" + fname;
    this->datafile->setFileName(pfname);
    if(!this->datafile->open(QIODevice::WriteOnly)) {
        /* 创建文件失败 尝试放在临时文件下 */
        pfname =  OnlineManagerBar::datarootPath() + "/" + fname;
        Q_ASSERT(this->datafile->open(QIODevice::WriteOnly));
        qDebug()<<"数据暂时存放在文件夹...下";
    }
    qDebug()<<"real start! >>" << pfname;

    /* 改变标志位 */
    this->isStarted = true;

    /* 清除datahash */
    this->init_datahash();
}
void SerialFlowControl::when_real_stop()
{
    /* 关闭文件 */
    if (this->datafile->isOpen()) {
        this->datafile->close();
    }


    /* txt 转化为 mat 文件*/
    QString CONVERTER = QDir::currentPath()+"/pytools/BF_arc2mat";
    QStringList arglist = QStringList() << this->datafile->fileName();
    QProcess *p = new QProcess();
    p->start(CONVERTER, arglist);

    /* 改变标志位 */
    qDebug()<<"real stop!";
    this->isStarted = false;

    /* 断开& 重连 串口 */
    // 交给 onlinemanagerbar(信号 whenstop) => serialcheck(槽 reload)
}
void SerialFlowControl::swithlevel_outpin(int outpin)
{
    if(this->isStarted && serial->isOpen()){
        QString buffer = QString("^%1\n").arg(QString::number(outpin));
        serial->write(buffer.toStdString().c_str(), buffer.size());
    }
}
void SerialFlowControl::swithlevel_inpin(int inpin)
{
    if(this->isStarted && serial->isOpen()){
        QString buffer = QString("&%1\n").arg(QString::number(inpin));
        serial->write(buffer.toStdString().c_str(), buffer.size());
    }
}
