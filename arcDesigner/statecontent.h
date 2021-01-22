// 单例模式
#ifndef STATECONTENT_H
#define STATECONTENT_H

#include <QObject>
#include "mainwindow.h"
#include "designersetfile.h"
#include <QDomDocument>
#include <QStringList>
#include "globalparas.h"
namespace  STATECONTENT_PARA {
    const int STATCK_IND_VAR(0), STACK_IND_DOPIN(1), STACK_IND_WHENPIN(2);
}

class QPushButton;
class QCheckBox;
class StateStripBar;

class StateContent : public QObject
{
    Q_OBJECT
private:
    explicit StateContent(QObject * parent = 0):QObject(parent){}
    explicit StateContent(const DesignerSetFile&){}
    void operator=(const StateContent&){}
public:
    static StateContent * instance;
    void createUi(Ui::MainWindow * ui_);
    static StateContent * getInstance();   
signals:
    void editComment(const QString &);
public slots:
    void on_stripEnable_clicked(bool isEnable);
    void on_stripPopup_clicked();
    void gui_to_dom();
    void on_varAssign_changed(QList<QString> varNames, QList<double> varValues);
    void on_pinAssign_changed(QList<bool> isIns, QList<int> Nums, QList<bool> isEnables, QList<QString> comms);
private slots:
    void on_comboBox_outMode_currentIndexChanged(int index);
    void on_comboBox_var_activated(const QString &arg1);

private:
    Ui::MainWindow * ui;
    QDomElement  dom_state_strip;
    StateStripBar *   master;
    GLOBAL_PARA::StripType myType;
    QMetaObject::Connection listen_commEdit;
private:
    void setEnabled(bool isEnable);
    void dom_to_gui();   
};

#endif // STATECONTENT_H
