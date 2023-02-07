#include "myboxui.h"
#include <QInputDialog>
#include <QVBoxLayout>
#include <QAction>
#include <QMouseEvent>
#include <QFormLayout>
namespace MYBOXUI_PARA{
    const int FONT_SIZE = 9;
    const QString STYLE_COMM = "QLabel {background-color:rgb(85, 170, 255);" \
                              "font:%1 15pt \"Microsoft JhengHei UI\";};";
    const QString STYLE_CID = "QLabel {background-color:rgb(85, 170, 255);" \
                               "font: 87 %1 15pt \"Arial Black\";};";
}
using namespace MYBOXUI_PARA;
using namespace MYBOXUI_PRIVATE;

// [1] DynamicBox

void DynamicBox::connectChildSlot(DynamicBox *child)
{
    if(child == NULL)
        return;
    connect(child, SIGNAL(wantMeDel()), this, SLOT(childWantDel()));
    connect(child, SIGNAL(wantAddBrotherPre()), this, SLOT(childWantAddBrotherPre()));
    connect(child, SIGNAL(wantAddBrotherPost()), this, SLOT(childWantAddBrotherPost()));
    connect(child, SIGNAL(wantMeDuplicate()), this, SLOT(childWantDuplicate()));
}

void DynamicBox::connectChildrenSlot()
{
    foreach(DynamicBox * child, childList)
        this->connectChildSlot(child);
}

int DynamicBox::indexChildSender()
{
    QObject* child = this->sender();
    int i=-1;
    foreach(DynamicBox* temp, this->childList){
        ++i;
        if((void *)temp == (void *)child) return i;
    }
    return -1;
}

void DynamicBox::domInsertChild(int index, QDomElement dom_child)
{
    QList<QDomNode> dom_children;
    QDomNode dom_c = dom_data.firstChild();
    while(!dom_c.isNull()){
        dom_children << dom_c;
        dom_c = dom_c.nextSibling();
    }
    int count_c = dom_children.count();
    if(index<=0)
        dom_data.insertBefore(dom_child, dom_children.first()); // at the first
    else if(index>count_c-1)
        dom_data.insertAfter(dom_child, dom_children.last()); //at end
    else
        dom_data.insertBefore(dom_child, dom_children[index]); //at middle
}

// [1] end

// [2] MyBoxUI
MyBoxUI::MyBoxUI(QString pref, int fsize, QFrame *parent)
    : DynamicBox(parent), Cid_preix(pref),fontsize(fsize), Cid_num(0)
{
    this->initUI();
    /* style */
    labelCid->setText(Cid_preix+"1");
    labelComm->setText("comment");
    this->setFrameShape(QFrame::Box);
    this->setLineWidth(1);
    this->setMidLineWidth(2);

    /* connect sigal and slot */
    connect(this->labelCid, SIGNAL(mouseDoubleClicked()), this, SLOT(wantToComment()));
    connect(this->labelComm, SIGNAL(mouseDoubleClicked()), this, SLOT(wantToComment()));
}
void MyBoxUI::initUI()
{
    /* */
    QVBoxLayout * ly = new QVBoxLayout();
    this->setLayout(ly);
    this->labelCid = new MyLabel();
    this->labelComm = new MyLabel();
    labelCid->setStyleSheet(STYLE_CID.arg(this->fontsize));
    labelComm->setStyleSheet(STYLE_COMM.arg(this->fontsize));
    this->centrol = new QWidget();
    labelComm->setAlignment(Qt::AlignHCenter);
    QFormLayout * ly_title = new QFormLayout();
    ly_title->addRow(labelCid, labelComm);
    ly_title->setContentsMargins(0,0,0,0);
    ly_title->setSpacing(0);
    ly->addLayout(ly_title);
    ly->addWidget(this->centrol, 1);
    ly->setSpacing(0);
    ly->setMargin(1);
    ly->setContentsMargins(2,0,2,2);
    this->setTitleNum(this->Cid_num);
    /* */
    QAction * ac_comm = new QAction(tr("Comment"),this);
    QAction * ac_addpre = new QAction(tr("Add to pre."), this);
    QAction * ac_addpost = new QAction(tr("Add to post."), this);
    QAction * ac_dupl = new QAction(tr("Duplicate"), this);
    QAction * ac_del  = new QAction(tr("Delete"), this);
    connect(ac_comm, SIGNAL(triggered(bool)), this, SLOT(wantToComment()));
    connect(ac_del, SIGNAL(triggered(bool)), this, SIGNAL(wantMeDel()));
    connect(ac_dupl, SIGNAL(triggered(bool)), this, SIGNAL(wantMeDuplicate()));
    connect(ac_addpre, SIGNAL(triggered(bool)), this, SIGNAL(wantAddBrotherPre()));
    connect(ac_addpost, SIGNAL(triggered(bool)), this, SIGNAL(wantAddBrotherPost()));
    QList<QAction *> acs = QList<QAction *>()<<ac_comm<<ac_addpre<<ac_addpost<<ac_dupl<<ac_del;
    labelCid->addActions(acs);
    labelCid->setContextMenuPolicy(Qt::ActionsContextMenu);
    labelComm->addActions(acs);
    labelComm->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void MyBoxUI::setTitleNum(int cid)
{
    this->Cid_num = cid;
    this->labelCid->setText(this->Cid_preix + QString::number(cid) + QString(":"));
}

void MyBoxUI::wantToComment()
{
    QString text_old = this->comment();
    QString title = labelCid->text();
    bool ok;
    QString text_new = QInputDialog::getText(this, title,  tr("Set comments:"),
                                   QLineEdit::Normal, text_old, &ok);
    if(ok)
        this->setComment(text_new);
}
QString MyBoxUI::comment()
{
    return this->labelComm->text();
}

void MyBoxUI::setComment(const QString & comm)
{
    this->labelComm->setText(comm);
}
// [2] end

MYBOXUI_PRIVATE::MyLabel::MyLabel(QWidget *parent):QLabel(parent)
{

}

void MYBOXUI_PRIVATE::MyLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    if ( event->button() == Qt::LeftButton )
        emit mouseDoubleClicked();
}
