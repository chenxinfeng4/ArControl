// the same as "StateBox" class
#include "component.h"
#include "../main.h"
#include "statebox.h"
#include <QHBoxLayout>
#include <QDebug>
typedef StateBox ChildClass;
using namespace GLOBAL_PARA;
using namespace COMPONENT_PARA;
Component::Component(QDomElement dom_this, QFrame *parent)
    : MyBoxUI(TITLE_PREFIX, TITLE_FONTSIZE, parent)
{
    this->dom_data = dom_this;
    this->initUI();
}
QDomElement Component::createEmptyDom()
{
    /*this root <DOM_COMPONENT>*/
    QDomElement dom_temp = createTempElement(DOM_COMPONENT).cloneNode().toElement();
    dom_temp.setAttribute(ATT_COMM, "");

    // <DOM_STATE>
    QDomElement dom_child = ChildClass::createEmptyDom();
    dom_temp.appendChild(dom_child);
    return dom_temp;
}

void Component::guiToDom()
{
    /* ci children */
    foreach(DynamicBox* s, childList)
        s->guiToDom();

    /* ci this own */
    this->dom_data.setAttribute(ATT_COMM, this->comment());
}
void Component::childWantDuplicate()
{
    ChildClass * obj = qobject_cast<ChildClass *> (this->sender());
    obj->guiToDom();
    QDomElement dom_obj = obj->getDomElementClone();
    int index = this->indexChildSender();
    this->wantChildAddTo(dom_obj, index);
}

void Component::childWantDel()
{
    ChildClass * obj = qobject_cast<ChildClass *> (this->sender());
    int index = this->indexChildSender();
    this->ly->removeWidget(obj);
    this->childList.removeAt(index);
    obj->deleteLater();
    this->childChanged();
    this->update();
}

void Component::childWantAddBrotherPre()
{
    QDomElement dom_obj = ChildClass::createEmptyDom();
    int index = this->indexChildSender();
    this->wantChildAddTo(dom_obj, index);
}
void Component::childWantAddBrotherPost()
{
    QDomElement dom_obj = ChildClass::createEmptyDom();
    int index = this->indexChildSender();
    this->wantChildAddTo(dom_obj, index+1);
}

void Component::wantChildAddTo(QDomElement childDom, int index)
{
    ChildClass * obj2 = new ChildClass(childDom);
    this->connectChildSlot(obj2);
    this->ly->insertWidget(index, obj2);
    this->childList.insert(index, obj2);
    this->domInsertChild(index, childDom);
    this->childChanged();
    this->update();
}

void Component::childChanged()
{
    for(int i=0; i<childList.count(); ++i){
        ChildClass * child = qobject_cast<ChildClass *>(childList[i]);
        child->setTitleNum(i+1);
    }
}

void Component::initUI()
{
    /* check and assert */
    SCPP_ASSERT_THROW(!this->dom_data.isNull());
    SCPP_ASSERT_THROW(dom_data.tagName() == DOM_COMPONENT);

    /* comm */
    QString comm = dom_data.attribute(ATT_COMM);
    this->setComment(comm); //ui changed;

    /* loop for create children */
    QDomElement dom_child = dom_data.firstChildElement();
    while(!dom_child.isNull()){
        SCPP_ASSERT_THROW(dom_child.tagName() == DOM_STATE);
        this->childList << new ChildClass(dom_child);
        dom_child = dom_child.nextSiblingElement();
    }

    /* add to gui */
    ly = new QHBoxLayout();
    this->centralWidget()->setLayout(ly);
    foreach(DynamicBox* s, childList)
        ly->addWidget((ChildClass *)s);

    this->childChanged();
    this->connectChildrenSlot();
}

