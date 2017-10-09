// the same as "StateBox" class
#include "QHBoxLayout"
#include <QDebug>
#include <QTabWidget>
#include "session.h"
#include "component.h"
#include "../main.h"


Session * Session::instance =  NULL;

using namespace GLOBAL_PARA;
typedef Component ChildClass;
Session::Session(QFrame *parent)
    :DynamicBox(parent),
     tabWidget(new QTabWidget()),
     doc_session(new QDomDocument("session"))
{
    initUI();
    //use "session->domToGui(Session::createEmptyDom());" to create widget
}

Session * Session::getInstance()
{
    if(Session::instance == NULL){
        Session::instance = new Session();
    }
    return Session::instance;
}

void Session::domToGui(const QDomElement dom_this)
{
    /* check and assert */
    SCPP_ASSERT_THROW(dom_this.tagName() == DOM_SESSION);


    /* clear doc and clone <DOM_SESSION>*/
    QDomElement dom_this_clone = dom_this.cloneNode().toElement(); //a copy
    this->doc_session->clear();
    doc_session->appendChild(doc_session->createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\""));
    this->doc_session->appendChild(dom_this_clone);
    this->dom_data = this->doc_session->documentElement(); // dom_data = doc_session

    /* comment */
    qt_noop();

    /* loop for create children */
    QDomElement dom_child = dom_data.firstChildElement();
    this->childList.clear();
    while(!dom_child.isNull()){
        SCPP_ASSERT_THROW(dom_child.tagName() == DOM_COMPONENT);
        this->childList << new ChildClass(dom_child);
        dom_child = dom_child.nextSiblingElement();
    }

    /* add to ui */
    this->tabWidget->clear(); //clear components and its dom
    foreach(DynamicBox* s, childList)
        this->tabWidget->addTab((ChildClass *)s, "");
    this->childChanged();
    this->connectChildrenSlot();
}

QDomElement Session::createEmptyDom()
{
    QDomElement dom_temp = createTempElement(DOM_SESSION).cloneNode().toElement();
    QDomElement dom_child = ChildClass::createEmptyDom();
    dom_temp.appendChild(dom_child);
    return dom_temp;
}

void Session::guiToDom()
{
    /* ci children */
    foreach(DynamicBox* s, childList)
        s->guiToDom();

    /* ci this own */
    qt_noop();
}
void Session::childWantDuplicate()
{
    ChildClass * obj = qobject_cast<ChildClass *> (this->sender());
    obj->guiToDom();
    QDomElement dom_obj = obj->getDomElementClone();
    int index = this->indexChildSender();
    this->wantChildAddTo(dom_obj, index);
}

void Session::childWantDel()
{
    ChildClass * obj = qobject_cast<ChildClass *> (this->sender());
    int index = this->indexChildSender();
    this->tabWidget->removeTab(index);
    this->childList.removeAt(index);
    obj->deleteLater();
    this->childChanged();
    this->update();
}

void Session::childWantAddBrotherPre()
{
    QDomElement dom_obj = ChildClass::createEmptyDom();
    int index = this->indexChildSender();
    this->wantChildAddTo(dom_obj, index);
}
void Session::childWantAddBrotherPost()
{
    QDomElement dom_obj = ChildClass::createEmptyDom();
    int index = this->indexChildSender();
    this->wantChildAddTo(dom_obj, index+1);
}

void Session::wantChildAddTo(QDomElement childDom, int index)
{
    ChildClass * obj2 = new ChildClass(childDom);
    this->connectChildSlot(obj2);
    this->tabWidget->insertTab(index, obj2, "");
    this->childList.insert(index, obj2);
    this->domInsertChild(index, childDom);
    this->childChanged();
    this->update();
}

void Session::childChanged()
{
    for(int i=0; i<childList.count(); ++i){
        ChildClass * child = qobject_cast<ChildClass *>(childList[i]);
        child->setTitleNum(i+1);
        this->tabWidget->setTabText(i, QString("C%1").arg(i+1)); //tab label change
    }
}

void Session::initUI()
{
    QBoxLayout *ly = new QHBoxLayout();
    this->setLayout(ly);

    /* state strip functional indicator */
    QBoxLayout *lyss = new QVBoxLayout();
    lyss->addSpacerItem(new QSpacerItem(10, 97));
    for(int i=0; i < STR_L_STYPE.count(); ++i){
        QString strLabel = QObject::tr(qPrintable(STR_L_STYPE[i]));
        QString strLabel_hint = QObject::tr(qPrintable(STR_L_STYPE_HINT[i]));
        QLabel *labelHint = new QLabel(strLabel, this);
        QString strStyle = STYLE_LABEL.arg(COLOR_TYPE[i]);
        labelHint->setStyleSheet(strStyle);
        labelHint->setToolTip(strLabel_hint);
        labelHint->setAlignment(Qt::AlignCenter);
        lyss->addWidget(labelHint);
    }
    lyss->addSpacerItem(new QSpacerItem(10, 27));

    /* tabWidget */
    ly->addLayout(lyss);
    ly->addWidget(this->tabWidget);
    ly->setContentsMargins(0, 0, 0, 0);
}
