#include <QDebug>
#include <QCloseEvent>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "main.h"
#include "varassigndialog.h"
#include "pinassigndialog.h"
#include "designersetfile.h"
#include "profiledialog.h"

#include "uistateframe/statebox.h"
#include "uistateframe/session.h"
#include "globalparas.h"
#include "statecontent.h"
#include "setfile2ino.h"
#include <QDomDocument>
#include <QVBoxLayout>
#include <QDebug>
using namespace GLOBAL_PARA;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow),
    pinAssignDialog(new PinAssignDialog(parent)),
    varAssignDialog(new VarAssignDialog(parent)),
    designerSetFile(DesignerSetFile::getInstance()),
    profileDialog(new ProfileDialog(parent)),
    session(Session::getInstance())
{
    ui->setupUi(this);
    ui->dockWidgetContents->updateSizeHint(QSize(286, 533));
    StateContent::getInstance()->createUi(ui);
    this->stateContent = StateContent::getInstance();

    connect(this, SIGNAL(closing()), qApp, SLOT(quit())); //this window close ==> app exit()

    /* session box */
    this->initSession();

    /* connect signal slot in GUI */
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->actionSave, SIGNAL(triggered(bool)), designerSetFile, SLOT(want_task_save()));
    connect(ui->actionSave_as, SIGNAL(triggered(bool)), designerSetFile, SLOT(want_task_saveas()));
    connect(ui->actionSave_as_defualt, SIGNAL(triggered(bool)), designerSetFile, SLOT(want_task_savedefault()));
    connect(ui->actionNew,  SIGNAL(triggered(bool)), designerSetFile, SLOT(want_task_new()));
    connect(ui->actionPin_Assignment, SIGNAL(triggered(bool)), pinAssignDialog, SLOT(show()));
    connect(ui->actionVar_Assignment, SIGNAL(triggered(bool)), varAssignDialog, SLOT(show()));
    connect(ui->actionProfile, SIGNAL(triggered(bool)), profileDialog, SLOT(show()));

    /* connect signal slot in CLASS */
    connect(designerSetFile, SIGNAL(want_settings_take()), stateContent, SLOT(gui_to_dom())); //this => file
    connect(designerSetFile, SIGNAL(give_taskList(QStringList)),this, SLOT(tastList_changed(QStringList)));//file => this
    connect(designerSetFile, SIGNAL(want_settings_take()), varAssignDialog, SLOT(commit_settings())); //this => file
    connect(designerSetFile, SIGNAL(want_settings_take()), pinAssignDialog, SLOT(commit_settings())); //this => file
    connect(designerSetFile, SIGNAL(give_pinassign(QList<bool>, QList<int>, QList<bool>, QList<QString>)),
            pinAssignDialog, SLOT(giveto_this(QList<bool>, QList<int>, QList<bool>, QList<QString>)));//file => this
    connect(pinAssignDialog, SIGNAL(takefrom_this(QList<bool>, QList<int>, QList<bool> , QList<QString>)),
            designerSetFile, SLOT(take_pinassign(QList<bool>, QList<int>, QList<bool>, QList<QString>)));//this => file
    connect(designerSetFile, SIGNAL(give_varassign(QList<QString>,QList<double>)),
            varAssignDialog, SLOT(giveto_this(QList<QString>,QList<double>))); //file => this
    connect(varAssignDialog, SIGNAL(takefrom_this(QList<QString>,QList<double>)),
            designerSetFile, SLOT(take_varassign(QList<QString>,QList<double>))); //this => file
    connect(designerSetFile, SIGNAL(want_settings_take()), session, SLOT(guiToDom())); //this => file
    connect(designerSetFile, &DesignerSetFile::give_session, [=](QDomElement dom_session){session->domToGui(dom_session);}); //file => this
    connect(designerSetFile, &DesignerSetFile::want_settings_take, [=](){designerSetFile->take_session(session->getDomElementClone());}); //this =>file
    connect(varAssignDialog, SIGNAL(report_this(QList<QString>,QList<double>)),
            stateContent, SLOT(on_varAssign_changed(QList<QString>,QList<double>)));
    connect(pinAssignDialog, SIGNAL(report_this(QList<bool>,QList<int>,QList<bool>,QList<QString>)),
            stateContent, SLOT(on_pinAssign_changed(QList<bool>,QList<int>,QList<bool>,QList<QString>)));
    /* init */
    designerSetFile->refresh_taskList();
    designerSetFile->want_task_loaddefault();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton button;
    button = QMessageBox::question(this, "ArControl",
                                   QString(tr("Are your sure to EXIT?")),
                                   QMessageBox::Yes | QMessageBox::No);
    if (button == QMessageBox::No) {
        event->ignore();
    }
    else if (button == QMessageBox::Yes) {
        qDebug()<<"closing";
        event->accept();
        emit closing();
    }
    else {
        Q_UNREACHABLE();
    }
}

void MainWindow::initSession()
{
    ui->widgetSession->setLayout(new QVBoxLayout());
    ui->widgetSession->layout()->addWidget(session);
}

void MainWindow::tastList_changed(const QStringList & taskList)
{
    foreach(QAction *ac, this->MN_taskList) {
        ac->deleteLater();
    }
    MN_taskList.clear();
    foreach(QString taskName, taskList) {
        QAction * ac = new QAction(taskName, this);
        connect(ac, &QAction::triggered, [taskName, this](){this->designerSetFile->want_task_load(taskName);});
        MN_taskList << ac;
    }
    ui->menuOpen_in->addActions(MN_taskList);
}
