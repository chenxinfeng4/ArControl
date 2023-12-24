#include <QDomDocument>
#include <QFile>
#include <QRegExp>
#include <QFileInfo>
#include <QTextStream>
#include <QFileDialog>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QCoreApplication>
#include "designersetfile.h"
#include "main.h"
#include "setfile2ino.h"

using namespace GLOBAL_PARA;
using namespace DESIGNERSETFILE_PARA;
using namespace DESIGNERSETFILE_PRIVATE;
QStringList getSubDirs(QDir  dir0);        //local scope in .cpp

DesignerSetFile * DesignerSetFile::instance = NULL;

// [1] basesic
QDomElement getDomWhat(QDomDocument *doc, QString nodeName)
{
    SCPP_ASSERT_THROW(!doc->isNull());
    if(doc->documentElement().isNull())
        doc->appendChild(doc->createElement(DOM_ARC));
    QDomElement dom_arc = doc->documentElement();
    SCPP_ASSERT_THROW(dom_arc.tagName()==DOM_ARC);
    if(dom_arc.firstChildElement(nodeName).isNull())
        dom_arc.appendChild(doc->createElement(nodeName));
    return dom_arc.firstChildElement(nodeName);
}
void setDomWhat(QDomDocument *doc, QString nodeName, const QDomElement dom_other)
{
    SCPP_ASSERT_THROW(!doc->isNull());
    SCPP_ASSERT_THROW(dom_other.nodeName()==nodeName, QString("%1, %2").arg(dom_other.nodeName()).arg(nodeName));
    if(doc->documentElement().isNull())
        doc->appendChild(doc->createElement(DOM_ARC));
    QDomElement dom_arc = doc->documentElement();
    SCPP_ASSERT_THROW(dom_arc.tagName()==DOM_ARC);
    QDomElement dom_old = dom_arc.firstChildElement(nodeName);
    if(! dom_old.isNull()){
        dom_arc.removeChild(dom_old);
    }
    QDomElement dom_new = dom_other.cloneNode().toElement();
    dom_arc.appendChild(dom_new);
}

DesignerSetFile::DesignerSetFile(QObject *parent)
    : QObject(parent), file(new QFile), doc(new QDomDocument("doc")),
      setfile2Ino(new Setfile2INO(parent))
{
}

QDomElement DesignerSetFile::getDomSet()
{
    return getDomWhat(this->doc, DOM_SETTINGS);
}

QDomElement DesignerSetFile::getDomProfile()
{
    return getDomWhat(this->doc, DOM_PROFILE);
}

QDomElement DesignerSetFile::getDomSession()
{
    return getDomWhat(this->doc, DOM_SESSION);
}

void DesignerSetFile::setDomProfile(const QDomElement dom)
{
    setDomWhat(this->doc, DOM_PROFILE, dom);
}

void DesignerSetFile::setDomSession(const QDomElement dom)
{
    setDomWhat(this->doc, DOM_SESSION, dom);
}


DesignerSetFile *DesignerSetFile::getInstance()
{
    if(DesignerSetFile::instance == NULL)
        DesignerSetFile::instance = new DesignerSetFile(0);
    return DesignerSetFile::instance;
}

DesignerSetFile::~DesignerSetFile()
{
    this->file->deleteLater();
    delete(this->doc);
}
// [1] end

// [2] important functions
void DesignerSetFile::refresh_taskList()
{
    QString dataroot = TASK_ROOT_PATH;
    QStringList taskList;
    QFileInfo f(dataroot);
    if(f.exists() && f.isDir()){
        taskList = getSubDirs(QDir(dataroot));
    }
    emit give_taskList(taskList);
}

void DesignerSetFile::showDom()
{
    qDebug()<<qPrintable(this->doc->toString());
}

void DesignerSetFile::take_info()
{
    QDomElement dom_info = doc->createElement(DOM_INFO); // "/SETTINGs/INFO"
    dom_info.setAttribute(ATT_VERSION, VAL_VERSION);           //"version:__txt__"
    dom_info.setAttribute(ATT_NAME, VAL_SOFTNAME); //"name:__txt__"
    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd, HH:mm:ss");
    dom_info.setAttribute(ATT_DATE, date);                 //"date:__txt__"
    this->getDomSet().appendChild(dom_info);
}

void DesignerSetFile::want_task_saveas()
{
    /* QInputDialog */
    bool ok;
    QString fileName = QFileInfo(this->file->fileName()).baseName(); //"./task/T1/T1.aconf" => "T1"
    QString file_d = (fileName.isEmpty() ? "Task_1" : fileName);
    fileName = QInputDialog::getText(0, tr("Save as"),
                                       tr("Task name: (with no space, no symbol)"),
                                       QLineEdit::Normal,
                                       file_d, &ok);
    fileName = fileName.trimmed();
    if (!ok || fileName.isEmpty()) {
        return;
    }
    else if(! QRegExp("\\w+").exactMatch(fileName)) {
        QMessageBox::critical(0, tr("Error"),
                              tr("Task names can only consist of ASCII characters"
                                 " and numbers (but cannot start with a number)"));
        return;
    }
    QString fullfile = QString("%1/%2/%2%3").arg(TASK_ROOT_PATH).arg(fileName).arg(SUFFIX);
    this->file->setFileName(fullfile);
    qDebug()<<"Save as"<<fullfile;

    /* call "save" */
    this->want_task_save();
}

void DesignerSetFile::want_task_save()
{
    /* It's a wild task */
    if(this->file->fileName().isEmpty()){
        this->want_task_saveas();
        if(this->file->fileName().isEmpty())
            return;
    }
    if(this->file->isOpen())
        this->file->close();

    /* init a empty doc */
    QString pathName = QFileInfo(this->file->fileName()).path();
    qDebug()<<"pathName"<<pathName;
    if (!QDir(pathName).exists())
        QDir().mkpath(pathName);
    SCPP_ASSERT_THROW(file->open(QFile::WriteOnly | QFile::Text),"Cannot open file!");
    doc->clear();
    QDomProcessingInstruction  instruction =  doc->createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    doc->appendChild(instruction);

    /* [1] clone <DOM_PROFILE> */
    QFile f_profile(PROFILE_FILE);
    QDomDocument doc_profile("doc_profile");
    SCPP_ASSERT_THROW(f_profile.open(QFile::ReadOnly | QFile::Text));
    doc_profile.setContent(&f_profile);
    f_profile.close();
    QDomElement dom_profile = doc_profile.documentElement();
    this->setDomProfile(dom_profile);

    /* [2] collect <DOM_SETTINGS> */
    this->take_info();
    emit want_settings_take(); //wait for SLOT "take_varassign()" & "take_pinassign" & "take_session"

    /* [3] clone <DOM_SESSION> */
    this->setDomSession(dom_session_temp);

    /* write and close */
    QTextStream out(file);
    doc->save(out, QDomNode::EncodingFromDocument);
    qDebug()<<"Saving Done";
    file->close();

    /* emit task list change */
    this->refresh_taskList();

    /* .config => *.ino */
    setfile2Ino->printIno(file->fileName());
}

void DesignerSetFile::want_task_export_pdf()
{
    /* It's a wild task */
    if(this->file->fileName().isEmpty()){
        this->want_task_saveas();
        if(this->file->fileName().isEmpty())
            return;
    }

    /* aconf 转化为 pdf 文件 */
    QString pathName = QFileInfo(this->file->fileName()).path();
//    QString CONVERTER = QDir::currentPath()+"/pytools/export_task_to_graphviz";
    QString CONVERTER = QCoreApplication::applicationDirPath()+"/pytools/export_task_to_graphviz";
    QProcess *p = new QProcess();QCoreApplication::applicationDirPath();
    QStringList arglist = QStringList() << pathName;
    p->start(CONVERTER, arglist);
}

void DesignerSetFile::want_task_savedefault()
{
    QString fullfile = QString("%1/%2/%2%3").arg(TASK_ROOT_PATH).arg(DEFUALT_LOAD).arg(SUFFIX);
    this->file->setFileName(fullfile);
    this->want_task_save();
    this->file->setFileName("");
}

void DesignerSetFile::want_task_load()
{
    qt_noop(); /*cannot reach */
}

void DesignerSetFile::want_task_load(const QString taskName)
{
    QString fullfile = QString("%1/%2/%2%3").arg(TASK_ROOT_PATH).arg(taskName).arg(SUFFIX);
    QFile f(fullfile);
    SCPP_ASSERT_THROW(f.exists(), "task file not exsit!");
    SCPP_ASSERT_THROW(f.open(QFile::ReadOnly | QFile::Text));
    doc->setContent(&f);
    f.close();

    /* [1] parsing <DOM_PROFILE> */
    qt_noop(); //don't use <DOM_PROFILE>

    /* [2] parsing <DOM_SETTINGS> */
    // assert root & others
    QDomElement dom_set = this->getDomSet();
    SCPP_ASSERT_THROW(!dom_set.isNull(), "File-Element error!");
    SCPP_ASSERT_THROW(dom_set.nodeName() == DOM_SETTINGS, "File-Element error!");

    // [2] parsing INFO
    QDomElement dom_info = dom_set.firstChildElement(DOM_INFO);
    SCPP_ASSERT_THROW(!dom_info.isNull(), "File-Element error!");
    QString info_name = dom_info.attribute(ATT_NAME);
    SCPP_ASSERT_THROW(info_name == VAL_SOFTNAME, "File-Element error!");
    QString info_version = dom_info.attribute(ATT_VERSION);
    Q_UNUSED(info_version);
    QString info_data = dom_info.attribute(ATT_DATE);
    Q_UNUSED(info_data);

    // [2] parsing VarAssignDialog
    QDomElement dom_var = dom_set.firstChildElement(DOM_VARALL);
    SCPP_ASSERT_THROW(!dom_var.isNull(), "File-Element error!");
    QList<QString> varNames; QList<double> varValues;
    QDomElement dom_var_item = dom_var.firstChildElement(DOM_VARALL_VAR);
    while(!dom_var_item.isNull()){
        QString varName = dom_var_item.attribute(ATT_VARNAME, "");
        QString varValue_str = dom_var_item.attribute(ATT_VARVALUE, "0");
        double varValue = varValue_str.toDouble();
        varNames << varName;
        varValues << varValue;
        dom_var_item = dom_var_item.nextSiblingElement(DOM_VARALL_VAR);
    }
    emit give_varassign(varNames, varValues);

    // [2] parsing PinAssignDialog
    QDomElement dom_pin = dom_set.firstChildElement(DOM_PINALL);
    SCPP_ASSERT_THROW(!dom_pin.isNull(), "File-Element error!");
    QDomElement dom_pin_item = dom_pin.firstChildElement(DOM_PINALL_PIN);
    QList<bool> isIns; QList<int> Nums; QList<bool> isEnables; QList<QString> comms;
    while(!dom_pin_item.isNull()) {
        bool isEnable = dom_pin_item.attribute(ATT_ISENB, "true") == "true"; //"true" "false"
        QString comm  = dom_pin_item.attribute(ATT_COMM);
        bool isIn  = dom_pin_item.attribute(ATT_TYPE, "IN") == "IN"; //"IN" , "OUT"
        int  num   = dom_pin_item.attribute(ATT_NUMBER).toDouble();
        isIns<<isIn; Nums<<num; isEnables<<isEnable; comms<<comm;
        dom_pin_item = dom_pin_item.nextSiblingElement(DOM_PINALL_PIN);
    }
    emit give_pinassign(isIns, Nums, isEnables, comms);

    // [3] pasing Session
    QDomElement dom_session = this->getDomSession();
    SCPP_ASSERT_THROW(!dom_session.isNull(), "File-Element error!");
    emit give_session(dom_session);

    /* loaded finished */
    this->file->setFileName(f.fileName());
}

void DesignerSetFile::want_task_loaddefault()
{
    QString fullfile = QString("%1/%2/%2%3").arg(TASK_ROOT_PATH).arg(DEFUALT_LOAD).arg(SUFFIX);
    QFileInfo fi(fullfile);
    if(fi.exists() && fi.isFile()) {
        /* have default.bconf */
        try{
            this->want_task_load(DEFUALT_LOAD);
        }
        catch(...){
            qDebug()<<"Invalid defualt aconf file!";
            QFile::remove(fullfile);
            this->want_task_new();
        }
    }
    else{
        /* have no- default.aconf */
        qDebug()<<"None defualt aconf file!";
        this->want_task_new();
    }
    this->file->setFileName("");
}

void DesignerSetFile::want_task_new()
{
    qDebug()<< "DesignerSetFile::want_task_new()";
    this->file->setFileName("");

    /* [1] parsing <DOM_PROFILE> */
    qt_noop(); //don't use <DOM_PROFILE>

    /* [2] parsing <DOM_SETTINGS> */
    // init VarAssignDialog
    QList<QString> varNames; QList<double> varValues;
    varNames<<"Var_1"<<"Var_2"<<"Var_3";
    varValues<< -2 << 0 << 1;
    emit give_varassign(varNames, varValues);

    // init PinAssignDialog
    static bool init = 1;
    if(init){
        init = false;
        QList<bool> isIns; QList<int> Nums;
        QList<bool> isEnables; QList<QString> comms;
        for(int isIn = 0; isIn<=1; ++isIn){
            int N_CHAN = ((bool)isIn) ? IN_CHAN_COUNT : OUT_CHAN_COUNT;
            for(int iChan=1; iChan<=N_CHAN; ++iChan){
                isIns<<isIn;
                Nums<<iChan;
                isEnables<<true;
                comms<<QString("#%1").arg(iChan);
            }
        }
        emit give_pinassign(isIns, Nums, isEnables, comms);
    }

    // init INFO
    qt_noop();

    /* [3] parsing <DOM_SESSION> */
    emit give_session(Session::createEmptyDom());
}

void DesignerSetFile::take_varassign(QList<QString> varNames, QList<double> varValues)
{
    QDomElement dom_set = this->getDomSet();
    QDomElement dom_varall = dom_set.firstChildElement(DOM_VARALL);
    if(dom_varall.isNull()){
        dom_varall = doc->createElement(DOM_VARALL);
        dom_set.appendChild(dom_varall);
    }
    while(!dom_varall.firstChild().isNull()){
        dom_varall.removeChild(dom_varall.firstChild());
    }
    int c = varNames.count();
    for(int i=0; i<c; ++i){
        QDomElement dom_varall_item = doc->createElement(DOM_VARALL_VAR);
        dom_varall.appendChild(dom_varall_item);
        dom_varall_item.setAttribute(ATT_VARNAME, varNames[i]);
        dom_varall_item.setAttribute(ATT_VARVALUE, varValues[i]);
    }
}

void DesignerSetFile::take_pinassign(QList<bool> isIns, QList<int> Nums, QList<bool> isEnables, QList<QString> comms)
{
    QDomElement dom_set = this->getDomSet();
    QDomElement dom_pinall = dom_set.firstChildElement(DOM_PINALL);
    if(dom_pinall.isNull()){
        dom_pinall = doc->createElement(DOM_PINALL);
        dom_set.appendChild(dom_pinall);
    }
    while(!dom_pinall.firstChild().isNull()){
        dom_pinall.removeChild(dom_pinall.firstChild());
    }
    int c = isIns.count();
    for(int i=0; i<c; ++i){
        QDomElement dom_pinall_item = doc->createElement(DOM_PINALL_PIN);
        dom_pinall.appendChild(dom_pinall_item);
        dom_pinall_item.setAttribute(ATT_TYPE, isIns[i]?"IN":"OUT");
        dom_pinall_item.setAttribute(ATT_NUMBER, Nums[i]);
        dom_pinall_item.setAttribute(ATT_ISENB, isEnables[i]?"true":"false");
        dom_pinall_item.setAttribute(ATT_COMM, comms[i]);
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
