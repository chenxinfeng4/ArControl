#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include <QDebug>
#include <QDir>
#include <QClipboard>
#include <iostream>
#include "onlinemanagerbar.h"
#include "serialcheck.h"
#include "serialflowcontrol.h"
#include "tasktoarduino.h"
#include "onlinechart.h"
#include "onlinetable.h"
#include "onlineled.h"
#include "onlinesetfile.h"
#include "profilereader.h"
#include "arcfirmata/arcfirmata.h"
#include "globalparas.h"
#include "version.h"
#include <QtSerialPort/QSerialPort>
#include <QThread>
#include <QFileDialog>
#include <QDateTime>
#include <QCloseEvent>
#include <QMessageBox>
#include <QRegExp>
#include <QTimer>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->myTime = new QTime;
    ui->setupUi(this);



    onlineManagerBar = new OnlineManagerBar(this, ui->comboBox_2, ui->comboBox_3,
                                            ui->lineEdit_2, ui->pushButton_5);
    serialCheck  = new SerialCheck(this, ui->menuSeiral,  ui->actionInfo,
                                   ui->actionEject, ui->actionReload);
    serialFlowControl = new SerialFlowControl(this, ui->dockWidget, ui->lineE2b,  ui->plainTextEdit,
                          ui->checkBox_freeze, ui->checkBox_autoclear, ui->pushButton,ui->BTN_Clear,
                          ui->BTN_Copy, serialCheck->COM_obj);
    tasktoArduino    = new TasktoArduino(this, ui->comboBox_tasklist, serialCheck->COM_obj);
    onlineChart      = new OnlineChart(this, ui->widget_chart, serialFlowControl->get_datahash());
    onlineTable      = new OnlineTable(this, serialFlowControl->get_datahash(), ui->tableWidget,
                          ui->LE_CxSx, ui->BTN_additem, ui->BTN_rmitem, ui->BTN_upitem, ui->BTN_downitem);
    onlineLED        = new OnlineLED(this, serialFlowControl->get_datahash(), ui->gridLayout_led);
    onlineSetFile    = new OnlineSetFile(this);
    profileReader    = ProfileReader::getInstance();
    arcFirmata       = new ArcFirmata(this);

    connect(onlineManagerBar, SIGNAL(raise_press_start()), serialFlowControl, SLOT(when_press_start()));
    connect(onlineManagerBar, SIGNAL(raise_press_stop()), serialFlowControl, SLOT(when_press_stop()));
    connect(onlineManagerBar,  SIGNAL(changeFdir(QString&)),serialFlowControl,SLOT(when_dataDir_change(QString&)));
    connect(onlineManagerBar, SIGNAL(stop()),             serialCheck, SLOT(on_AC_reload()));
    connect(serialFlowControl, SIGNAL(raise_isconnect(bool)),onlineManagerBar,SLOT(when_serial_isconnected(bool)));
    connect(serialFlowControl, SIGNAL(raise_spont_start()), onlineManagerBar, SLOT(when_spont_start()));
    connect(serialFlowControl, SIGNAL(raise_spont_stop()),  onlineManagerBar, SLOT(when_spont_stop()));
    connect(serialFlowControl, SIGNAL(raise_everyTimeCycle()),onlineLED,SLOT(receive_everyTimeCycle()));
	connect(onlineLED, SIGNAL(swithlevel_outpin(int)),     serialFlowControl, SLOT(swithlevel_outpin(int)));
    connect(tasktoArduino   ,  SIGNAL(raise_eject()),      serialCheck, SLOT(on_AC_eject()));
    connect(tasktoArduino   ,  SIGNAL(raise_reload()),     serialCheck, SLOT(on_AC_reload()));
    connect(onlineManagerBar,  SIGNAL(start()),            onlineChart, SLOT(when_start()));
    connect(onlineManagerBar,  SIGNAL(stop()),             onlineChart, SLOT(when_stop()));
    connect(onlineManagerBar,  SIGNAL(start()),            onlineTable, SLOT(when_start()));
    connect(onlineManagerBar,  SIGNAL(stop()),             onlineTable, SLOT(when_stop()));
    connect(onlineManagerBar,  SIGNAL(stop()),             onlineLED,   SLOT(clean_led()));
    connect(onlineTable, SIGNAL(add_line(QString,bool)),   onlineChart, SLOT(add_line(QString,bool)));
    connect(onlineTable, SIGNAL(set_line_visible(QString,bool)),onlineChart, SLOT(set_line_visible(QString,bool)));
    connect(onlineTable, SIGNAL(rm_line(QString)),         onlineChart, SLOT(rm_line(QString)));
    connect(onlineTable, SIGNAL(rm_line_all()),            onlineChart, SLOT(rm_line_all()));
    connect(ui->actionOpen, SIGNAL(triggered(bool)),       onlineSetFile, SLOT(want_task_load()));
    connect(ui->actionSave, SIGNAL(triggered(bool)),       onlineSetFile, SLOT(want_task_save()));
    connect(ui->actionSave_as, SIGNAL(triggered(bool)),    onlineSetFile, SLOT(want_task_saveas()));
    connect(ui->actionNew,  SIGNAL(triggered(bool)),       onlineSetFile, SLOT(want_task_new()));
    connect(ui->actionSave_as_default, SIGNAL(triggered(bool)), onlineSetFile, SLOT(want_task_savedefault()));
    connect(ui->actionExit, SIGNAL(triggered(bool)),       this,          SLOT(close()));
    connect(ui->actionAbout_me, SIGNAL(triggered(bool)),   this, SLOT(on_action_aboutme_triggered()));

    connect(onlineSetFile, SIGNAL(give_table(bool,QString,QString,bool)), onlineTable, SLOT(giveto_this(bool,QString,QString,bool)));
    connect(onlineSetFile, SIGNAL(clean_table()),          onlineTable, SLOT(clean_table()));
    connect(onlineSetFile, SIGNAL(want_settings_take()),   onlineTable, SLOT(commit_settings()));
    connect(onlineTable, SIGNAL(takefrom_this(bool,QString,QString,bool)), onlineSetFile, SLOT(take_table(bool,QString,QString,bool)));
    connect(onlineSetFile, SIGNAL(give_dockwidget(bool,bool,bool,bool)), serialFlowControl, SLOT(giveto_this(bool,bool,bool,bool)));
    connect(onlineSetFile, SIGNAL(want_settings_take()),   serialFlowControl, SLOT(commit_settings()));
    connect(serialFlowControl, SIGNAL(takefrom_this(bool,bool,bool,bool)), onlineSetFile, SLOT(take_dockwidget(bool,bool,bool,bool)));
    connect(onlineSetFile, SIGNAL(give_serial(bool,QString,QString)), serialCheck, SLOT(giveto_this(bool,QString,QString)));
    connect(onlineSetFile, SIGNAL(want_settings_take()),   serialCheck, SLOT(commit_settings()));
    connect(serialCheck, SIGNAL(takefrom_this(bool,QString,QString)), onlineSetFile, SLOT(take_serial(bool,QString,QString)));
    connect(onlineSetFile, SIGNAL(give_dir(QString,QString)), onlineManagerBar, SLOT(giveto_this(QString,QString)));
    connect(onlineSetFile, SIGNAL(want_settings_take()),   onlineManagerBar, SLOT(commit_settings()));
    connect(onlineManagerBar, SIGNAL(takefrom_this(QString,QString)), onlineSetFile, SLOT(take_dir(QString,QString)));

#ifdef GLOBALLY_INIT_ON
    this->onlineSetFile->want_task_loaddefault();
#endif

    onlineManagerBar->refreshdir(); //board cast the initialized "dir"
    onlineTable->initPush2Chart();
    profileReader->getArduino();

    QRect pos =ui->dockWidget->geometry();
    qDebug()<<ui->dockWidget->geometry();
    pos.setWidth(250);
    ui->dockWidget->setGeometry(pos);
    qDebug()<<ui->dockWidget->geometry();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actionStream_Window_triggered(bool checked)
{
    if(ui->dockWidget->isVisible() != checked){
        ui->dockWidget->setVisible(checked);
    }
}

void MainWindow::on_dockWidget_visibilityChanged(bool visible)
{
    if(ui->actionStream_Window->isChecked() != visible){
        ui->actionStream_Window->setChecked(visible);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(this->serialFlowControl->getStarted()) {
        /* is running task */
        QMessageBox::StandardButton button;
        button = QMessageBox::question(this, tr("Exit Application"),
                                       QString(tr("A task is being RUNNING, sure to exit now?")),
                                       QMessageBox::Yes | QMessageBox::No);
        if (button == QMessageBox::No) {
            event->ignore();
        }
        else if (button == QMessageBox::Yes) {
            this->serialFlowControl->when_press_stop(); /* 保存txt数据 */
            event->accept();
        }
        else {
            Q_UNREACHABLE();
        }
    }
    this->serialCheck->on_AC_eject();
}

void MainWindow::on_actionFirmata_Test_triggered()
{
//    QString arduino_debug = ProfileReader::getInstance()->getArduino();
//    arduino_debug = arduino_debug.replace('\\', '/');
//    int ind = arduino_debug.lastIndexOf('/');
//    SCPP_ASSERT_THROW(ind>=0, "Arduino debuger path have not profiled yet!");
//    QString std_firmata = arduino_debug.left(ind)
//                       + "/libraries/Firmata/examples/StandardFirmata/StandardFirmata.ino";
//    qDebug()<<"std_firmata"<<std_firmata;

//    /* download "StandardFirmata.ino" to arduino */
//    tasktoArduino->upload_task(std_firmata);

    /* eject current COM */
    serialCheck->on_AC_eject();

    /* show window */
    arcFirmata->showNormal();
}

void MainWindow::on_action_aboutme_triggered()
{
    QMessageBox msgBox;
    msgBox.setWindowIcon(this->windowIcon());
    msgBox.setWindowTitle("About ArControl");
    msgBox.setText(QString("").append(VERSION_INFO::VERSION_NUM)
                   .append("    (").append(VERSION_INFO::VERSION_TIME).append(")").append("<br><br>")
                   .append("Further: <a href = https://github.com/chenxinfeng4/ArControl>tutorials</a>").append("<br>")
                   .append("Cite: <a href = https://www.frontiersin.org/articles/10.3389/fnbeh.2017.00244/full>this work</a> <br>")
                   .append("Authour: Xinfeng Chen (陈昕枫) <br>")
                   .append("Open source: <u>GNU LGPL v2.1</u>"));

    msgBox.exec();
}
