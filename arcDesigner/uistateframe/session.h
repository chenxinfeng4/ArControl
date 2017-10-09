// 单例模式
// the same as "StateBox" class
#ifndef SESSION_H
#define SESSION_H

#include <QObject>
#include <QDomDocument>
#include "myboxui.h"
#include "../globalparas.h"

class QTabWidget;
namespace SESSION_PARA{

}

class Session : public DynamicBox
{
private:
    explicit Session(QFrame *parent = 0);
    static Session * instance;
    explicit Session(const Session&):DynamicBox(0){Q_UNREACHABLE();}
    void operator=(const Session&){Q_UNREACHABLE();}
    QDomElement getDomElement(){Q_UNREACHABLE(); return QDomElement();} //overwite
public:
    static Session * getInstance();
public slots:
    void guiToDom(); // save to dom_this
    void domToGui(const QDomElement dom_this); //replace dom_this and gui
    static QDomElement createEmptyDom();
protected slots:
    void childWantDuplicate();
    void childWantDel();
    void childWantAddBrotherPre();
    void childWantAddBrotherPost();
protected :
    void wantChildAddTo(QDomElement childDom, int index);
    void childChanged();
private:
    void initUI();
    QTabWidget * tabWidget;
    QDomDocument * doc_session;
};

#endif // SESSION_H
