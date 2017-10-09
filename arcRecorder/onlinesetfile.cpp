#include <QDomDocument>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QDateTime>
#include <QDebug>

#include "onlinesetfile.h"

using namespace ONLINESETKFILE_PARA;
using namespace ONLINESETKFILE_PRIVATE;

OnlineSetFile::OnlineSetFile(QObject *parent)
    : QObject(parent), file(new QFile), doc(new QDomDocument("doc"))
{
}

void OnlineSetFile::want_task_save()
{
    /* None have loaded */
    if(this->file->fileName().isEmpty()){
        this->want_task_saveas();
        if(this->file->fileName().isEmpty())
            return;
    }
    if(this->file->isOpen())
        this->file->close();

    /* init a empty doc with ROOT and ROOT/TABLE*/
    SCPP_ASSERT_THROW(file->open(QFile::WriteOnly | QFile::Text), "Cannot open file!");
    doc->clear();
    QDomProcessingInstruction  instruction =  doc->createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    doc->appendChild(instruction);
    QDomElement root = doc->createElement(DOM_ROOT); // "/SETTINGS"
    doc->appendChild(root);
    take_info();
    QDomElement dom_table = doc->createElement(DOM_TABLE);
    root.appendChild(dom_table);

    /* wait for doc change */
    emit want_settings_take();

    qDebug()<<"file"<<file->fileName();
    qDebug()<<qPrintable(doc->toString());
    /* write to file */
    QTextStream out(file);
    doc->save(out, QDomNode::EncodingFromDocument);
    qDebug()<<"Saving Done";
}

void OnlineSetFile::want_task_savedefault()
{
    this->file->setFileName(DEFUALT_LOAD);
    this->want_task_save();
    this->file->setFileName("");
}

void OnlineSetFile::want_task_saveas()
{
    QString fileName = QFileDialog::getSaveFileName(0, tr("Save Layout-Config File"),
                                                    "",
                                                    tr("Config File(*%1)").arg(SUFFIX));
    if(fileName.isNull()){
        return;
    }
    else{
        this->file->setFileName(fileName);
    }
    this->want_task_save();
}
void OnlineSetFile::want_task_load(const QString fileName)
{
    if(fileName.isEmpty()){
        return;
    }
    this->file->setFileName(fileName);
    SCPP_ASSERT_THROW(file->open(QFile::ReadOnly | QFile::Text));
    doc->setContent(file);
    file->close();

    /* parsing DOM file */
    // assert root & others
    QDomElement root = doc->documentElement();
    SCPP_ASSERT_THROW(!root.isNull(), "File-Element error!");
    SCPP_ASSERT_THROW(root.nodeName() == DOM_ROOT, "File-Element error!");

    // parsing INFO
    QDomElement dom_info = root.firstChildElement(DOM_INFO);
    SCPP_ASSERT_THROW(!dom_info.isNull(), "File-Element error!");
    QString info_name = dom_info.attribute(ATT_NAME);
    SCPP_ASSERT_THROW(info_name == VAL_SOFTNAME, "File-Element error!");
    QString info_version = dom_info.attribute(ATT_VERSION);
    Q_UNUSED(info_version);
    QString info_data = dom_info.attribute(ATT_DATE);
    Q_UNUSED(info_data);

    // parsing Dir
    QDomElement dom_dir = root.firstChildElement(DOM_DIR);
    SCPP_ASSERT_THROW(!dom_dir.isNull(), "File-Element error!");
    QString dir_upper = dom_dir.attribute(ATT_DIRUPPER);
    QString dir_lowwer = dom_dir.attribute(ATT_DIRLOWWER);

    // parsing Serial
    QDomElement dom_serial = root.firstChildElement(DOM_SERIAL);
    SCPP_ASSERT_THROW(!dom_serial.isNull(), "File-Element error!");
    QString serial_port = dom_serial.attribute(ATT_PORT);
    QString serial_descrbe = dom_serial.attribute(ATT_DISCRIBE);
    bool serial_isCon = dom_serial.attribute(ATT_ISCON, "true") == "true";

    // parsing DockWidget
    QDomElement dom_dockw = root.firstChildElement(DOM_DOCKW);
    SCPP_ASSERT_THROW(!dom_dockw.isNull(), "File-Element error!");
    bool dockw_isVis = dom_dockw.attribute(ATT_ISVIS, "true") == "true";
    bool dockw_isFre = dom_dockw.attribute(ATT_ISFREEZ, "false")== "true";
    bool dockw_isDoc = dom_dockw.attribute(ATT_ISDOCK, "true")== "true";
    bool dockw_isAut = dom_dockw.attribute(ATT_ISAUTOS, "true")== "true";

    // parsing TableWidget
    QDomElement dom_table = root.firstChildElement(DOM_TABLE);
    SCPP_ASSERT_THROW(!dom_table.isNull(), "File-Element error!");
    emit clean_table();
    QDomElement dom_table_item = dom_table.firstChildElement(DOM_TABLE_ITEM);
    while(!dom_table_item.isNull()){
        bool item_isVis = dom_table_item.attribute(ATT_ISVIS, "true") == "true";
        QString item_name  = dom_table_item.attribute(ATT_NAME);
        QString item_comm  = dom_table_item.attribute(ATT_COMM);
        bool item_isCol = dom_table_item.attribute(ATT_ISCOL, "true") == "true";
        emit give_table(item_isVis, item_name, item_comm, item_isCol);
        dom_table_item = dom_table_item.nextSiblingElement(DOM_TABLE_ITEM);
    }

    // emit Dir DockWidget
    emit give_dir(dir_upper, dir_lowwer);
    emit give_dockwidget(dockw_isVis, dockw_isDoc, dockw_isAut, dockw_isFre);
    emit give_serial(serial_isCon, serial_port, serial_descrbe);
}

void OnlineSetFile::want_task_load()
{
    /* Dialog to select file */
    QString fileName = QFileDialog::getOpenFileName(0, tr("Load Layout-Config File"),
                                                    "",
                                                    tr("Config File(*%1)").arg(SUFFIX));
    this->want_task_load(fileName);
}

void OnlineSetFile::want_task_loaddefault()
{
    QFileInfo fi(DEFUALT_LOAD);
    if(fi.exists() && fi.isFile()){
        /* have default.bconf */
        try{
            this->want_task_load(DEFUALT_LOAD);
        }
        catch(...){
            qDebug()<<"Invalid defualt bconf file!";
            QFile::remove(DEFUALT_LOAD);
            this->want_task_new();
        }
    }
    else{
        /* have no- default.bconf */
        qDebug()<<"None defualt bconf file!";
        this->want_task_new();
    }
    this->file->setFileName("");
}

void OnlineSetFile::want_task_new()
{
    this->file->setFileName("");
    emit clean_table();
    emit give_table(true, "OUT1", "", true);
    emit give_table(true, "OUT2", "", true);
    emit give_table(false, "OUT3", "", false);
    emit give_table(false, "OUT4", "", false);
    emit give_table(false, "OUT5", "", false);
    emit give_table(false, "OUT6", "", false);
    emit give_table(false, "OUT7", "", false);
    emit give_table(false, "OUT8", "", false);
    emit give_table(true, "IN1", "", true);
    emit give_table(true, "IN2", "", true);
    emit give_table(false, "IN3", "", false);
    emit give_table(false, "IN4", "", false);
    emit give_table(false, "IN5", "", false);
    emit give_table(false, "IN6", "", false);
    emit give_table(true, "C1S1", "Session begin", true);
    emit give_table(true, "C1S2", "", true);
    emit give_table(false, "C0S0", "Session end", false);
    emit give_dockwidget(true, true, true, false);
    emit give_dir("", "");
    emit give_serial(false, "", "");
}

void OnlineSetFile::take_dir(const QString &upper, const QString &lowwer)
{
    QDomElement root = doc->documentElement();
    QDomElement dom_dir = doc->createElement(DOM_DIR);
    root.appendChild(dom_dir);
    dom_dir.setAttribute(ATT_DIRUPPER, upper);
    dom_dir.setAttribute(ATT_DIRLOWWER, lowwer);
}

void OnlineSetFile::take_serial(bool isConnect, const QString &port, const QString &describe)
{
    QDomElement root = doc->documentElement();
    QDomElement dom_serial = doc->createElement(DOM_SERIAL);
    root.appendChild(dom_serial);
    dom_serial.setAttribute(ATT_ISCON, isConnect?"true":"false");
    dom_serial.setAttribute(ATT_PORT, port);
    dom_serial.setAttribute(ATT_DISCRIBE, describe);
}

void OnlineSetFile::take_dockwidget(bool isVisible, bool isDock, bool isAutoS, bool isFreeze)
{
    QDomElement root = doc->documentElement();
    QDomElement dom_dockw = doc->createElement(DOM_DOCKW);
    root.appendChild(dom_dockw);
    dom_dockw.setAttribute(ATT_ISVIS, isVisible?"true":"false");
    dom_dockw.setAttribute(ATT_ISDOCK, isDock?"true":"false");
    dom_dockw.setAttribute(ATT_ISAUTOS, isAutoS?"true":"false");
    dom_dockw.setAttribute(ATT_ISFREEZ, isFreeze?"true":"false");
}

void OnlineSetFile::take_table(bool isVisible, const QString &name, const QString &comm, bool isColor)
{
    QDomElement root = doc->documentElement();
    QDomElement dom_table = root.firstChildElement(DOM_TABLE);
    QDomElement dom_table_item = doc->createElement(DOM_TABLE_ITEM);
    dom_table.appendChild(dom_table_item);
    dom_table_item.setAttribute(ATT_ISVIS, isVisible?"true":"false");
    dom_table_item.setAttribute(ATT_NAME, name);
    dom_table_item.setAttribute(ATT_COMM, comm);
    dom_table_item.setAttribute(ATT_ISCOL, isColor);
}

void OnlineSetFile::take_info()
{
    QDomElement root = doc->documentElement();
    QDomElement dom_info = doc->createElement(DOM_INFO); // "/SETTINGs/INFO"
    root.appendChild(dom_info);
    dom_info.setAttribute(ATT_VERSION, VAL_VERSION);           //"version:__txt__"
    dom_info.setAttribute(ATT_NAME, VAL_SOFTNAME); //"name:__txt__"
    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd, HH:mm:ss");
    dom_info.setAttribute(ATT_DATE, date);                 //"date:__txt__"
}
