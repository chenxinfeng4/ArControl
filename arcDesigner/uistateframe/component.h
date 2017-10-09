#ifndef COMPONENT_H
#define COMPONENT_H

#include <QObject>
#include <QDomDocument>
#include "myboxui.h"
#include "../globalparas.h"

class QBoxLayout;
namespace COMPONENT_PARA{
    const QString TITLE_PREFIX("C");
    const int TITLE_FONTSIZE(15);
}
class Component : public MyBoxUI
{
    Q_OBJECT
public:
    explicit Component(QDomElement dom_this, QFrame *parent = 0);
    static QDomElement createEmptyDom();
public slots:
    void guiToDom();
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
    QBoxLayout * ly;
};
#endif // COMPONENT_H
