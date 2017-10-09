#ifndef DESIGNERSETFILE_H
#define DESIGNERSETFILE_H

#include <QObject>
#include "globalparas.h"
#include "profiledialog.h"
#include "uistateframe/session.h"
class QFile;
class QDomDocument;
class QDomElement;
class Setfile2INO;
namespace DESIGNERSETFILE_PARA
{
    const QString SUFFIX(".aconf"), DOM_ROOT("SETTINGS");
    const QString TASK_ROOT_PATH("./task"),  DEFUALT_LOAD("default");
}
namespace DESIGNERSETFILE_PRIVATE
{
}

class DesignerSetFile : public QObject
{
    Q_OBJECT
public:
    static DesignerSetFile * getInstance();
    ~DesignerSetFile();
    void refresh_taskList();
    void showDom();
private:
    QFile * file;
    QDomDocument * doc;
    QDomElement dom_session_temp;
    Setfile2INO  * setfile2Ino;   //".aconf" => "*.ino"
    void take_info();

    static DesignerSetFile * instance;
    explicit DesignerSetFile(QObject *parent = 0);
    explicit DesignerSetFile(const DesignerSetFile&):QObject(0){}
    void operator=(const DesignerSetFile&){}
    QDomElement getDomSet();
    QDomElement getDomProfile();
    QDomElement getDomSession();
    void setDomProfile(const QDomElement);
    void setDomSession(const QDomElement);
signals:
    void give_taskList(const QStringList &);
// [2] 文件 保存和载入
signals:
    void want_settings_take();
    void give_varassign(QList<QString> varNames, QList<double> varValues);
    void give_pinassign(QList<bool> isIns, QList<int> Nums, QList<bool> isEnables, QList<QString> comms);
    void give_session(QDomElement dom_session);
public slots:
    void want_task_saveas();
    void want_task_save();
    void want_task_savedefault();
    void want_task_load();
    void want_task_load(const QString file);
    void want_task_loaddefault();
    void want_task_new();
    void take_varassign(QList<QString> varNames, QList<double> varValues);
    void take_pinassign(QList<bool> isIns, QList<int> Nums, QList<bool> isEnables, QList<QString> comms);
    void take_session(QDomElement dom_session){this->dom_session_temp = dom_session;}
// [2] end

};

#endif // DESIGNERSETFILE_H
