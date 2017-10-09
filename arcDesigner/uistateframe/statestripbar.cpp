#include "statestripbar.h"
#include "session.h"
#include "../statecontent.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QSizePolicy>
#include <QAction>
#include <QDebug>
#include <QDomDocument>
#include <QInputDialog>

using namespace GLOBAL_PARA;
using namespace STATESTRIPBAR_PARA;

StateStripBar::StateStripBar(QDomElement dom_this, QFrame *parent)
    :DynamicBox(parent), stateContent(StateContent::getInstance())
{
    this->dom_data = dom_this;
    initUI();

    /* connect signal and slots */
    //connect(stateContent, SIGNAL(editComment(QString)), this, SLOT(setComment(QString)));//false
    connect(this, SIGNAL(stripPopup_clicked()), stateContent, SLOT(on_stripPopup_clicked()));
    connect(this, SIGNAL(stripEnable_clicked(bool)), stateContent, SLOT(on_stripEnable_clicked(bool)));
}

QDomElement StateStripBar::createEmptyDom(StripType type_)
{
    /*this root <DOM_SSTRIP>*/
    QDomElement dom_state_strip_temp = createTempElement(DOM_SSTRIP).cloneNode().toElement();
    dom_state_strip_temp.setAttribute(ATT_TYPE, STR_L_STYPE[type_]); //default type
    dom_state_strip_temp.setAttribute(ATT_ISENB, (false?"ture":"false"));
    dom_state_strip_temp.setAttribute(ATT_COMM, "");

    auto createDom = [&](QDomElement fatherNode, QString nodeName)->QDomElement{
        QDomElement dom = createTempElement(nodeName);
        fatherNode.appendChild(dom);
        return dom;
    };
    // <DOM_CODE>
    QDomElement dom_code = createDom(dom_state_strip_temp, DOM_CODE);
    dom_code.appendChild( QDomDocument("doc temp").createCDATASection("") );

    // <DOM_DOPIN>
    QDomElement dom_dopin = createDom(dom_state_strip_temp, DOM_DOPIN);
    QDomElement dom_keepon = createDom(dom_dopin, DOM_KEEPON);
    QDomElement dom_turn  = createDom(dom_dopin, DOM_TURN);
    QDomElement dom_blink = createDom(dom_dopin, DOM_BLINK);
    dom_dopin.setAttribute(ATT_TYPE, STR_L_OUTMODE[0]);
    dom_dopin.setAttribute(ATT_NUMBER, "1");
    dom_turn.setAttribute(ATT_ISENB, "false");
    dom_blink.setAttribute(ATT_FREQ, "1");
    dom_blink.setAttribute(ATT_DUTY, "50");
    // <DOM_WHENPIN>
    QDomElement dom_whenpin = createDom(dom_state_strip_temp, DOM_WHENPIN);
    dom_whenpin.setAttribute(ATT_NUMBER, "1");
    // <DOM_COUNT>
    QDomElement dom_count = createDom(dom_state_strip_temp, DOM_COUNT);
    QDomElement dom_fixed = createDom(dom_count, DOM_FIXED);
    QDomElement dom_range = createDom(dom_count, DOM_RANGE);
    QDomElement dom_goelse = createDom(dom_count, DOM_GOELSE);
    dom_count.setAttribute(ATT_TYPE, STR_L_RANDMODE[0]);
    dom_fixed.setAttribute(ATT_FIXED, "0");
    dom_range.setAttribute(ATT_FROM, "0");
    dom_range.setAttribute(ATT_TO, "0");
    dom_goelse.setAttribute(ATT_PROB, "100");
    dom_goelse.setAttribute(ATT_GO, "0");
    dom_goelse.setAttribute(ATT_ELSE, "0");
    // <DOM_TIME>
    QDomElement dom_time = dom_count.cloneNode().toElement();
    dom_time.setTagName(DOM_TIME);
    dom_state_strip_temp.appendChild(dom_time);
    // <DOM_NEXTSTATE>
    QDomElement dom_nextstate = dom_count.cloneNode().toElement();
    dom_nextstate.setTagName(DOM_NEXTSTATE);
    dom_state_strip_temp.appendChild(dom_nextstate);
    dom_nextstate.setAttribute(ATT_ISCOMP, "false");
    return dom_state_strip_temp;
}

void StateStripBar::guiToDom()
{
//    this->dom_data.setAttribute(ATT_COMM, this->comment()); //have done in "StateContent"
    this->dom_data.setAttribute(ATT_ISENB, this->CKB_enable->isChecked()?"true":"false");
}

void StateStripBar::checkDom()
{
    /* Assert */
    SCPP_ASSERT(!dom_data.isNull(), "dom should not empty");
    /* 删减去除不需要的dom */
    this->type =(StripType) indexOf(STR_L_STYPE, dom_data.attribute(ATT_TYPE));
    auto deleteDom = [&](QDomElement fatherNode, QString nodeName){
        SCPP_ASSERT_THROW(!fatherNode.isNull());
        QDomElement dom = fatherNode.firstChildElement(nodeName);
        if(!dom.isNull())
            dom.parentNode().removeChild(dom);
        SCPP_ASSERT_THROW(fatherNode.firstChildElement(nodeName).isNull());
    };
    QDomNode dom_code_text;
    switch(type){
    case StripType::doVar :
        deleteDom(dom_data, DOM_DOPIN);
        deleteDom(dom_data, DOM_WHENPIN);
        deleteDom(dom_data, DOM_TIME);
        deleteDom(dom_data, DOM_COUNT);
        deleteDom(dom_data, DOM_NEXTSTATE);
        dom_code_text = dom_data.firstChildElement(DOM_CODE).firstChild();
        if(dom_code_text.nodeValue().isEmpty()) {
            dom_code_text.setNodeValue(DOVAR_DEF_TEXT);
        }
        break;
    case StripType::doPin :
        deleteDom(dom_data, DOM_CODE);
        deleteDom(dom_data, DOM_WHENPIN);
        deleteDom(dom_data, DOM_COUNT);
        deleteDom(dom_data, DOM_NEXTSTATE);
        break;
    case StripType::whenVar :
        deleteDom(dom_data, DOM_WHENPIN);
        deleteDom(dom_data, DOM_DOPIN);
        deleteDom(dom_data, DOM_TIME);
        deleteDom(dom_data, DOM_COUNT);
        dom_code_text = dom_data.firstChildElement(DOM_CODE).firstChild();
        if(dom_code_text.nodeValue().isEmpty()) {
            dom_code_text.setNodeValue(DOVAR_DEF_TEXT);
        }
        break;
    case StripType::whenCount :
        deleteDom(dom_data, DOM_CODE);
        deleteDom(dom_data, DOM_WHENPIN);
        deleteDom(dom_data, DOM_DOPIN);
        deleteDom(dom_data, DOM_TIME);
        break;
    case StripType::whenTime :
        deleteDom(dom_data, DOM_CODE);
        deleteDom(dom_data, DOM_WHENPIN);
        deleteDom(dom_data, DOM_DOPIN);
        deleteDom(dom_data, DOM_COUNT);
        break;
    case StripType::whenPin :
        deleteDom(dom_data, DOM_CODE);
        deleteDom(dom_data, DOM_DOPIN);
        deleteDom(dom_data, DOM_TIME);
        deleteDom(dom_data, DOM_COUNT);
        break;
    default:
        SCPP_ASSERT_THROW("type error!");
    }
}

void StateStripBar::initUI()
{
    /* check dom */
    this->checkDom();

    /* add uicontrol to layout */
    QHBoxLayout * ly = new QHBoxLayout();
    this->setLayout(ly);
    this->labelHint = new QLabel("   ");
    QString strStyle = STYLE_LABEL.arg(COLOR_TYPE.at(this->type));
    labelHint->setStyleSheet(strStyle);
    this->CKB_enable = new QCheckBox();
    this->BTN_comm = new QPushButton();
    QSizePolicy sp_retain = BTN_comm->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    sp_retain.setHorizontalPolicy(QSizePolicy::Ignored);
    BTN_comm->setSizePolicy(sp_retain); 
    BTN_comm->setStyleSheet("QPushButton {text-align: left;}");
    ly->addWidget(labelHint);
    ly->addWidget(CKB_enable);
    ly->addWidget(BTN_comm, 1);
    ly->setContentsMargins(0,0,0,0);
    /* QAction */
    if(type==StripType::whenPin){
        QAction * ac_addpre = new QAction(tr("Add to pre."), this);
        QAction * ac_addpost= new QAction(tr("Add to post."), this);
        QAction * ac_dupl = new QAction(tr("Duplicate"), this);
        QAction * ac_del  = new QAction(tr("Delete"), this);
        connect(ac_del, SIGNAL(triggered(bool)), this, SIGNAL(wantMeDel()));
        connect(ac_dupl, SIGNAL(triggered(bool)), this, SIGNAL(wantMeDuplicate()));
        connect(ac_addpre, SIGNAL(triggered(bool)), this, SIGNAL(wantAddBrotherPre()));
        connect(ac_addpost, SIGNAL(triggered(bool)), this, SIGNAL(wantAddBrotherPost()));
        QList<QAction *> acs = QList<QAction *>()<<ac_addpre<<ac_addpost<<ac_dupl<<ac_del;
        this->labelHint->addActions(acs);
        this->labelHint->setText(" + ");
        this->labelHint->setContextMenuPolicy(Qt::ActionsContextMenu);
    }

    /* hint */
    QString strHint = STR_L_STYPE.at(this->type);
    this->labelHint->setToolTip(strHint);

    /* connect signal slot */
    connect(CKB_enable, &QCheckBox::clicked, [=](){BTN_comm->setVisible(CKB_enable->isChecked());});  
    connect(CKB_enable, SIGNAL(clicked(bool)), this, SIGNAL(stripEnable_clicked(bool)));
    connect(BTN_comm, SIGNAL(clicked(bool)), this, SIGNAL(stripPopup_clicked()));

    /* dom to gui */
    this->setComment(this->dom_data.attribute(ATT_COMM));
    CKB_enable->setChecked(this->dom_data.attribute(ATT_ISENB)=="true");
    CKB_enable->clicked(); //emit uicontrol signal
}
