#ifndef STATEBOX_H
#define STATEBOX_H

#include <QObject>
#include <QDomDocument>
#include "myboxui.h"
#include "statestripbar.h"
#include "../globalparas.h"

class QBoxLayout;
namespace STATEBOX_PARA{
    const QString TITLE_PREFIX("S");
    const int TITLE_FONTSIZE(12);
}
class StateBox : public MyBoxUI
{
    Q_OBJECT
public:
    explicit StateBox(QDomElement dom_this, QFrame *parent = 0);
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

#endif // STATEBOX_H
