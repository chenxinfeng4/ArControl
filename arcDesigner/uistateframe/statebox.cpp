#include "statebox.h"
#include "../main.h"
#include <QVBoxLayout>
#include <QDebug>
typedef StateStripBar ChildClass;
using namespace GLOBAL_PARA;
using namespace STATEBOX_PARA;
using namespace STATECONTENT_PARA;
StateBox::StateBox(QDomElement dom_this, QFrame *parent)
    : MyBoxUI(TITLE_PREFIX, TITLE_FONTSIZE, parent)
{
    this->dom_data = dom_this;
    this->initUI();
}

QDomElement StateBox::createEmptyDom()
{
    /*this root <DOM_STATE>*/
    QDomElement dom_temp = createTempElement(DOM_STATE).cloneNode().toElement();
    dom_temp.setAttribute(ATT_COMM, "");

    // <DOM_SSTRIP>
    auto stripList = QList<StripType>()<<StripType::doVar<<doPin
                    <<whenVar<<whenCount<<whenTime<<whenPin;
    foreach(StripType type, stripList) {
        QDomElement dom_state_strip = ChildClass::createEmptyDom(type);
        dom_temp.appendChild(dom_state_strip);
    }
    return dom_temp;
}

void StateBox::guiToDom()
{
    /* ci children */
    foreach(DynamicBox* s, childList)
        s->guiToDom();

    /* ci this own */
    this->dom_data.setAttribute(ATT_COMM, this->comment());
}

void StateBox::childWantDuplicate()
{
    ChildClass * obj = qobject_cast<ChildClass *> (this->sender());
    obj->guiToDom();
    QDomElement dom_obj = obj->getDomElementClone();
    int index = this->indexChildSender();
    this->wantChildAddTo(dom_obj, index);
}

void StateBox::childWantDel()
{
    ChildClass * obj = qobject_cast<ChildClass *> (this->sender());
    int index = this->indexChildSender();
    this->ly->removeWidget(obj);
    this->childList.removeAt(index);
    obj->deleteLater();
    this->update();
}

void StateBox::childWantAddBrotherPre()
{
    ChildClass * obj = qobject_cast<ChildClass *> (this->sender());
    QDomElement dom_obj = ChildClass::createEmptyDom(obj->getStripType());
    int index = this->indexChildSender();
    this->wantChildAddTo(dom_obj, index);
}
void StateBox::childWantAddBrotherPost()
{
    ChildClass * obj = qobject_cast<ChildClass *> (this->sender());
    QDomElement dom_obj = ChildClass::createEmptyDom(obj->getStripType());
    int index = this->indexChildSender();
    this->wantChildAddTo(dom_obj, index+1);
}

void StateBox::wantChildAddTo(QDomElement childDom, int index)
{
    ChildClass * obj2 = new ChildClass(childDom);
    this->connectChildSlot(obj2);
    this->ly->insertWidget(index, obj2);
    this->childList.insert(index, obj2);
    this->domInsertChild(index, childDom);
    this->childChanged();
    this->update();
}

void StateBox::childChanged()
{
}

void StateBox::initUI()
{
    /* check and assert */
    SCPP_ASSERT_THROW(!this->dom_data.isNull());
    SCPP_ASSERT_THROW(dom_data.tagName() == DOM_STATE);

    /* comm */
    QString comm = dom_data.attribute(ATT_COMM);
    this->setComment(comm); //ui changed;

    /* loop for create children */
    QDomElement dom_child = dom_data.firstChildElement();
    while(!dom_child.isNull()){
        SCPP_ASSERT_THROW(dom_child.tagName() == DOM_SSTRIP);
        SCPP_ASSERT_THROW(STR_L_STYPE.contains(dom_child.attribute(ATT_TYPE)));
        this->childList << new ChildClass(dom_child);
        dom_child = dom_child.nextSiblingElement();
    }

    /* add to gui */
    ly = new QVBoxLayout();
    this->centralWidget()->setLayout(ly);
    foreach(DynamicBox* s, childList)
        ly->addWidget((ChildClass *)s);

    this->childChanged();
    this->connectChildrenSlot();
}

