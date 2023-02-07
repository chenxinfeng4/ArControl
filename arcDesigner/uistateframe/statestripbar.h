#ifndef STATESTRIPBAR_H
#define STATESTRIPBAR_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QDomDocument>
#include "../globalparas.h"
#include "../statecontent.h"  // StateStrip enum
#include "../designersetfile.h"
#include "../main.h"
#include "myboxui.h"

class StateContent;
namespace STATESTRIPBAR_PARA{
    const QString DOVAR_DEF_TEXT("//Write your Arduino C++ code below\n");
    const QString WHENVAR_DEF_TEXT("//Write your Arduino C++ code below\n" \
                                    "bool wantToGoNextState = false;\n" \
                                    "return wantToGoNextState;");
    class QLabelExt: public QLabel{
        Q_OBJECT
        using QLabel::QLabel;
    public:
        signals:
            void doubleClicked();
        protected:
            void mouseDoubleClickEvent(QMouseEvent *){
                emit doubleClicked();
            }
    };
}
class StateStripBar :public DynamicBox
{
    Q_OBJECT
public:
    explicit StateStripBar(QDomElement dom_this, QFrame *parent = 0);
    GLOBAL_PARA::StripType getStripType(){return type;}
    static QDomElement createEmptyDom(GLOBAL_PARA::StripType type_ = GLOBAL_PARA::StripType::whenPin);
    QString comment() const {return BTN_comm->text();}
signals:
    void stripEnable_clicked(bool isEnable);
    void stripPopup_clicked();
public slots:
    void setComment(const QString &arg1){BTN_comm->setText(arg1);}
    void guiToDom();
private:
    GLOBAL_PARA::StripType type;
    QLabel * labelHint;
    QPushButton * BTN_comm;
    QCheckBox * CKB_enable;
    StateContent * stateContent;
    void initUI();
    void checkDom();

protected slots:
    void childWantDuplicate(){}
    void childWantDel(){}
    void childWantAddBrotherPre(){}
    void childWantAddBrotherPost(){}
};


#endif // STATESTRIPBAR_H
