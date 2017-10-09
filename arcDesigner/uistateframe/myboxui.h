#ifndef MYBOXUI_H
#define MYBOXUI_H

#include <QFrame>
#include <QLabel>
#include <QDomDocument>
#include <QDebug>
namespace MYBOXUI_PRIVATE
{
class MyLabel: public QLabel
{
    Q_OBJECT
public:
    explicit MyLabel(QWidget *parent=0);
signals:
    void mouseDoubleClicked();
protected slots:
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
};
}
class DynamicBox: public QFrame //纯虚类
{
    Q_OBJECT
public:
    explicit DynamicBox(QFrame *parent = 0):QFrame(parent){}
    QDomElement getDomElement() const{return dom_data;}
    QDomElement getDomElementClone() const{return dom_data.cloneNode().toElement();}
public:
    ~DynamicBox(){
        if(this->dom_data.isNull())return;
        if(this->dom_data.parentNode().isNull()) return;
        qDebug()<<"Remove node: "<<dom_data.nodeName();
        this->dom_data.parentNode().removeChild(this->dom_data);
        this->dom_data.clear();
    }

signals:
    void wantMeDuplicate();
    void wantMeDel();
    void wantAddBrotherPre();
    void wantAddBrotherPost();
protected:
    QList<DynamicBox *> childList;
    int  indexChildSender();
    QDomElement  dom_data;
    void domInsertChild(int index, QDomElement dom_child);
public slots:
    virtual void guiToDom()=0;
protected slots:
    virtual void childWantDuplicate() = 0;
    virtual void childWantDel() = 0;
    virtual void childWantAddBrotherPre() = 0;
    virtual void childWantAddBrotherPost() = 0;
    void connectChildSlot(DynamicBox *child);
    void connectChildrenSlot();
};

class MyBoxUI : public DynamicBox
{
    Q_OBJECT
public:
    explicit MyBoxUI(QString pref, int fontsize, QFrame *parent = 0);
    QString comment();
    QWidget * centralWidget(){return centrol;}
    void setTitlePrefix(QString pref){Cid_preix = pref;setTitleNum(Cid_num);}
private:
    MYBOXUI_PRIVATE::MyLabel * labelCid;
    MYBOXUI_PRIVATE::MyLabel * labelComm;
    QString Cid_preix;
    qint32  fontsize;
    qint32  Cid_num;
    QWidget * centrol;
    void initUI();
protected:
    virtual void wantChildAddTo(QDomElement childDom, int index)=0;
    virtual void childChanged() = 0;
public slots:
    void setTitleNum(int scid);
    void setComment(const QString &); //write derectly
private slots:
    void wantToComment(); //raise message dialog

};

#endif // MYBOXUI_H
