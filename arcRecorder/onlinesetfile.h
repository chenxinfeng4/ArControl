#ifndef ONLINESETFILE_H
#define ONLINESETKFILE_H

#include <QObject>
#include <QString>

#include "main.h"

class QFile;
class QDomDocument;

namespace ONLINESETKFILE_PARA
{
    const QString SUFFIX(".bconf"), DOM_ROOT("SETTINGS"), DOM_SERIAL("SERIAL"),
                  DOM_DIR("DIR"),  DOM_DOCKW("DOCKWIDGET"), DOM_TABLE("TABLE"),
                  DOM_TABLE_ITEM("ITEM"), DOM_INFO("INFO"), ATT_VERSION("version"),
                  ATT_DATE("date"), ATT_NAME("name"),       ATT_COMM("comment"),
                  ATT_PORT("port"), ATT_DISCRIBE("describe"), ATT_ISAUTOS("isAutoScroll"),
                  ATT_ISDOCK("isDock"), ATT_ISFREEZ("isFreeze"),ATT_ISVIS("isVisible"),
                  ATT_DIRUPPER("upper"), ATT_DIRLOWWER("lowwer"), ATT_ISCON("isConnect"),
                  VAL_VERSION("0.8"), VAL_SOFTNAME("ArControl Online"),ATT_ISCOL("isColor");
    const QString DEFUALT_LOAD("default.bconf");
}
namespace ONLINESETKFILE_PRIVATE
{
}
class OnlineSetFile : public QObject
{
    Q_OBJECT
public:
    explicit OnlineSetFile(QObject *parent = 0);

private:
    QFile * file;
    QDomDocument * doc;
    void take_info();
signals:
    void want_settings_take();
    void give_dir(const QString & upper, const QString & lowwer);
    void give_serial(bool isConnect, const QString & port, const QString & describe);
    void give_dockwidget(bool isVisible, bool isDock, bool isAutoS, bool isFreeze);
    void give_table(bool isVisible, const QString & name, const QString & comm, bool isColor);
    void clean_table();

public slots:
    void want_task_saveas();
    void want_task_save();
    void want_task_savedefault();
    void want_task_load();
    void want_task_load(const QString file);
    void want_task_loaddefault();
    void want_task_new();
    void take_dir(const QString & upper, const QString & lowwer);
    void take_serial(bool isConnect, const QString & port, const QString & describe);
    void take_dockwidget(bool isVisible, bool isDock, bool isAutoS, bool isFreeze);
    void take_table(bool isVisible, const QString & name, const QString & comm, bool isColor=true);
};

#endif // ONLINETASKFILE_H
