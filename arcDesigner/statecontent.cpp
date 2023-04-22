#include "statecontent.h"
#include "ui_mainwindow.h"
#include "designersetfile.h"
#include "main.h"
#include "uistateframe/statestripbar.h"
#include <QPushButton>
#include <QSizePolicy>
#include <QCheckBox>
#include <QDebug>
#include <QStyle>
#define TR(qstring) QObject::tr(qPrintable(qstring))
using namespace DESIGNERSETFILE_PARA;
using namespace STATECONTENT_PARA;
using namespace GLOBAL_PARA;

StateContent * StateContent::instance = NULL;

// [1]
void StateContent::createUi(Ui::MainWindow *ui_)
{
    /* init GUI pattern */
    this->ui = ui_;
    connect(ui->lineEdit_comm, SIGNAL(textEdited(QString)), this, SIGNAL(editComment(QString)));
    QWidget * dock_w = ui->dockWidgetContents;
    QSizePolicy sp_retain = dock_w->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    dock_w->setSizePolicy(sp_retain);
    this->setEnabled(false);
//    ui->comboBox_inChan->view()->setFixedHeight(95);    //full display 6 items
//    ui->comboBox_outChan->view()->setFixedHeight(128);  //full display 8 items
    QSizePolicy sp_retain2 = ui->radioButton_isC->sizePolicy();
    sp_retain2.setRetainSizeWhenHidden(true);
    ui->radioButton_isC->setSizePolicy(sp_retain2);
    /* connect SIGNAL and SLOT */
    connect(ui->comboBox_timeMode, SIGNAL(currentIndexChanged(int)), ui->stack_Time, SLOT(setCurrentIndex(int)));
    connect(ui->comboBox_countMode, SIGNAL(currentIndexChanged(int)), ui->stack_Count, SLOT(setCurrentIndex(int)));
    connect(ui->comboBox_stateMode, SIGNAL(currentIndexChanged(int)), ui->stack_state, SLOT(setCurrentIndex(int)));
    connect(ui->comboBox_stateMode, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [=](int index){
                bool isVisible = index != ui->comboBox_stateMode->count()-1;
                ui->radioButton_isC->setVisible(isVisible);
                ui->radioButton_isS->setVisible(isVisible);
    });
    connect(ui->comboBox_outMode, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_outMode_currentIndexChanged(int)));
    connect(ui->comboBox_var,     SIGNAL(activated(QString)), this, SLOT(on_comboBox_var_activated(QString)));

    /* add TOOL_TIP_HINT */
    ui->comboBox_outMode->setToolTip(TR(OUTMODE_HINT));
    ui->lineEdit_comm->setToolTip(TR(COMMENT_HINT));
    ui->lineEdit_Duty->setToolTip(TR(DUTY_HINT));
    ui->lineEdit_Freq->setToolTip(TR(FREQ_HINT));
    ui->lineEdit_stateProb->setToolTip(TR(PROB_HINT));
    ui->lineEdit_timeProb->setToolTip(TR(PROB_HINT));
    ui->lineEdit_countProb->setToolTip(TR(PROB_HINT));
    ui->lineEdit_countFrom->setToolTip(TR(COUNT_HINT));
    ui->lineEdit_countTo->setToolTip(TR(COUNT_HINT));
    ui->lineEdit_countGo->setToolTip(TR(COUNT_HINT));
    ui->lineEdit_countElse->setToolTip(TR(COUNT_HINT));
    ui->lineEdit_countFixed->setToolTip(TR(COUNT_HINT));
    ui->lineEdit_timeFrom->setToolTip(TR(TIME_HINT));
    ui->lineEdit_timeTo->setToolTip(TR(TIME_HINT));
    ui->lineEdit_timeGo->setToolTip(TR(TIME_HINT));
    ui->lineEdit_timeElse->setToolTip(TR(TIME_HINT));
    ui->lineEdit_timeFixed->setToolTip(TR(TIME_HINT));
    ui->lineEdit_stateFrom->setToolTip(TR(STATE_HINT));
    ui->lineEdit_stateTo->setToolTip(TR(STATE_HINT));
    ui->lineEdit_stateGo->setToolTip(TR(STATE_HINT));
    ui->lineEdit_stateElse->setToolTip(TR(STATE_HINT));
    ui->lineEdit_stateFixed->setToolTip(TR(STATE_HINT));
}

StateContent *StateContent::getInstance()
{
    if(StateContent::instance == NULL)
        StateContent::instance = new StateContent(0);
    return StateContent::instance;
}

void StateContent::on_stripEnable_clicked(bool isEnable)
{
    StateStripBar * master_now = dynamic_cast<StateStripBar *> (sender());
    this->setEnabled(isEnable && (master == master_now));
}

void StateContent::on_stripPopup_clicked()
{
    StateStripBar * master_now =  dynamic_cast<StateStripBar *> (sender());
    SCPP_ASSERT(master_now!=NULL);
    ui->lineEdit_comm->setFocus();
    /* 判断是否是同一个 */
    if(this->master == master_now)
        return;
    disconnect(this->listen_commEdit);
    master = master_now;

    /* 提交gui数据到dom */
    gui_to_dom();

    /* 跟改 dom & master */
    this->dom_state_strip = master->getDomElement();

    /* 准备重绘 */
    this->listen_commEdit = connect(this, SIGNAL(editComment(QString)),
                                    master, SLOT(setComment(QString)));
    StripType type = (StripType) indexOf(STR_L_STYPE, dom_state_strip.attribute(ATT_TYPE));
    SCPP_ASSERT_THROW(type == master->getStripType());
    myType = type;

    /* 放入数据 */
    dom_to_gui();
    this->setEnabled(true);
}

void StateContent::setEnabled(bool arg)
{
    this->ui->dockWidgetContents->setVisible(arg);
}
void StateContent::gui_to_dom()
{
    qDebug()<<"StateContent::gui_to_dom()";
    if( dom_state_strip.isNull())
    {    qDebug()<<"dom re is NULL"; return;}

    /* comment */
    dom_state_strip.setAttribute(ATT_COMM, ui->lineEdit_comm->text());

    /*special type*/
    bool has_count(false), has_time(false), has_state(false);
    auto setHas_count_time_state = [&](bool h_count, bool h_time, bool h_state){
        has_count = h_count; has_time = h_time; has_state = h_state;
    };
    auto setAtt = [=](QDomElement parent, QString me, QString att, QString val){
        parent.firstChildElement(me).setAttribute(att, val);
    };
    auto ui2att = [=](QDomElement p, QString m, QString a, QLineEdit * w){
        setAtt(p, m, a, w->text());
    };
    switch(myType){
    case StripType::doVar :{
        QString codes = ui->textEdit_Code->toPlainText();
        dom_state_strip.firstChildElement(DOM_CODE).firstChild().setNodeValue(codes);
        setHas_count_time_state(false, false, false);
        break;
    }
    case StripType::doPin :{
        auto dom = dom_state_strip.firstChildElement(DOM_DOPIN);
        int chan_num = ui->comboBox_outChan->currentIndex()+1;
        dom.setAttribute(ATT_NUMBER, chan_num);
        int modeIndex = ui->comboBox_outMode->currentIndex();
        bool isTurnon = ui->checkBox_outTurn->isChecked();
        dom.setAttribute(ATT_TYPE, STR_L_OUTMODE.at(modeIndex));
        setAtt(dom, DOM_TURN,  ATT_ISENB, QString(isTurnon? "true":"false"));
        ui2att(dom, DOM_BLINK, ATT_FREQ, ui->lineEdit_Freq);
        ui2att(dom, DOM_BLINK, ATT_DUTY,  ui->lineEdit_Duty);
        setHas_count_time_state(false, true, false);
        break;
    }
    case StripType::whenVar:{
        QString codes = ui->textEdit_Code->toPlainText();
        dom_state_strip.firstChildElement(DOM_CODE).firstChild().setNodeValue(codes);
        setHas_count_time_state(false, false, true);
        break;
    }
    case StripType::whenCount:{
        setHas_count_time_state(true, false, true);
        break;
    }
    case whenTime:{
        setHas_count_time_state(false, true, true);
        break;
    }
    case whenPin:{
        int chan_num = ui->comboBox_inChan->currentIndex()+1;
        int modeIndex = ui->comboBox_inMode->currentIndex();
        setAtt(dom_state_strip, DOM_WHENPIN, ATT_NUMBER, QString::number(chan_num));
        setAtt(dom_state_strip, DOM_WHENPIN, ATT_ISHIGH, STR_L_INMODE.at(modeIndex));
        setHas_count_time_state(false, true, true);
        break;
    }
    default:
        SCPP_ASSERT_THROW(0, "no such type");
    }
    /* time & count & nextstate*/
    if(has_time){
        QString nodeName= DOM_TIME;
        auto dom = dom_state_strip.firstChildElement(nodeName);
        int modeIndex  = ui->comboBox_timeMode->currentIndex();
        dom.setAttribute(ATT_TYPE,  STR_L_RANDMODE.at(modeIndex));
        ui2att(dom, DOM_FIXED,  ATT_FIXED, ui->lineEdit_timeFixed);
        ui2att(dom, DOM_RANGE,  ATT_FROM,  ui->lineEdit_timeFrom);
        ui2att(dom, DOM_RANGE,  ATT_TO,    ui->lineEdit_timeTo);
        ui2att(dom, DOM_GOELSE, ATT_PROB,  ui->lineEdit_timeProb);
        ui2att(dom, DOM_GOELSE, ATT_GO,    ui->lineEdit_timeGo);
        ui2att(dom, DOM_GOELSE, ATT_ELSE,  ui->lineEdit_timeElse);
    }
    if(has_count){
        QString nodeName= DOM_COUNT;
        auto dom = dom_state_strip.firstChildElement(nodeName);
        int modeIndex  = ui->comboBox_countMode->currentIndex();
        dom.setAttribute(ATT_TYPE,  STR_L_RANDMODE.at(modeIndex));
        ui2att(dom, DOM_FIXED,  ATT_FIXED, ui->lineEdit_countFixed);
        ui2att(dom, DOM_RANGE,  ATT_FROM,  ui->lineEdit_countFrom);
        ui2att(dom, DOM_RANGE,  ATT_TO,    ui->lineEdit_countTo);
        ui2att(dom, DOM_GOELSE, ATT_PROB,  ui->lineEdit_countProb);
        ui2att(dom, DOM_GOELSE, ATT_GO,    ui->lineEdit_countGo);
        ui2att(dom, DOM_GOELSE, ATT_ELSE,  ui->lineEdit_countElse);
    }
    if(has_state){
        QString nodeName= DOM_NEXTSTATE;
        auto dom = dom_state_strip.firstChildElement(nodeName);
        int modeIndex  = ui->comboBox_stateMode->currentIndex();
        dom.setAttribute(ATT_TYPE,  STR_L_STATEMODE.at(modeIndex));
        ui2att(dom, DOM_FIXED,  ATT_FIXED, ui->lineEdit_stateFixed);
        ui2att(dom, DOM_RANGE,  ATT_FROM,  ui->lineEdit_stateFrom);
        ui2att(dom, DOM_RANGE,  ATT_TO,    ui->lineEdit_stateTo);
        ui2att(dom, DOM_GOELSE, ATT_PROB,  ui->lineEdit_stateProb);
        ui2att(dom, DOM_GOELSE, ATT_GO,    ui->lineEdit_stateGo);
        ui2att(dom, DOM_GOELSE, ATT_ELSE,  ui->lineEdit_stateElse);
        bool iscomponent = ui->radioButton_isC->isChecked();
        dom.setAttribute(ATT_ISCOMP, (iscomponent? "true":"false"));
    }
}
void StateContent::dom_to_gui()
{
    qDebug()<<"StateContent::dom_to_gui()";

    /* comment */
    ui->lineEdit_comm->setText(dom_state_strip.attribute(ATT_COMM));
    qDebug()<<"ATT_COMM"<<dom_state_strip.attribute(ATT_COMM);

    /* special type */
    //int how_timeshow=-1; //[-1 keep(has_time=1) | 0 invisible(has_time=0) | 1 visible(has_time=1)]
    bool has_count(false), has_time(false), has_state(false);
    auto setHas_main_count_time_state = [&](bool h_main, bool h_count, int how_timeshow, bool h_state){
        has_count=h_count; has_time=how_timeshow!=0; has_state=h_state;
        ui->stackedMain->setVisible(h_main);
        ui->group_count->setVisible(h_count);
        ui->group_state->setVisible(h_state);
        if(how_timeshow!=-1)
            ui->group_time->setVisible(how_timeshow);
    };
    auto setSpacer_stateHigh = [=](bool isHigh){
        Q_UNUSED(isHigh);
        qt_noop();
    };

    auto getAtt = [=](QDomElement parent, QString me, QString att, QString defV ="")-> QString{
        return parent.firstChildElement(me).attribute(att, defV);
    };
    auto att2ui = [=](QDomElement p, QString m, QString a, QLineEdit * w){
        w->setText(getAtt(p, m, a));
    };
    switch(myType){
    case StripType::doVar : {
        ui->comboBox_var->setCurrentIndex(0);
        ui->group_state->setVisible(false);
        ui->dockWidget->setWindowTitle(QObject::tr("doVar"));
        ui->stackedMain->setCurrentIndex(STATCK_IND_VAR);
        QString codes = dom_state_strip.firstChildElement(DOM_CODE).text(); //dom
        ui->textEdit_Code->setPlainText(codes);
        setHas_main_count_time_state(true, false, 0, false);
        setSpacer_stateHigh(false);
        break;
    }
    case StripType::doPin : {
        ui->stackedMain->setCurrentIndex(STACK_IND_DOPIN);
        ui->dockWidget->setWindowTitle(QObject::tr("doPin"));
        auto dom = dom_state_strip.firstChildElement(DOM_DOPIN);
        int chan_num = QString(dom.attribute(ATT_NUMBER)).toInt(); //1-8
        int modeIndex  = indexOf(STR_L_OUTMODE, dom.attribute(ATT_TYPE)); //0-2
        att2ui(dom, DOM_BLINK, ATT_FREQ, ui->lineEdit_Freq);
        att2ui(dom, DOM_BLINK, ATT_DUTY, ui->lineEdit_Duty);
        bool isTurnon = getAtt(dom,DOM_TURN, ATT_ISENB)=="true";
        ui->comboBox_outChan->setCurrentIndex(chan_num - 1);
        ui->comboBox_outMode->setCurrentIndex(modeIndex);
        ui->comboBox_outMode->currentIndexChanged(modeIndex);
        ui->checkBox_outTurn->setChecked(isTurnon);
        setHas_main_count_time_state(true, false, -1, false);
        setSpacer_stateHigh(true);
        break;
    }
    case StripType::whenVar: {
        ui->comboBox_var->setCurrentIndex(0);
        ui->stackedMain->setCurrentIndex(STATCK_IND_VAR);
        ui->dockWidget->setWindowTitle(QObject::tr("whenVar"));
        QString codes = dom_state_strip.firstChildElement(DOM_CODE).text();
        ui->textEdit_Code->setPlainText(codes);
        setHas_main_count_time_state(true, false, 0, true);
        setSpacer_stateHigh(false);
        break;
    }
    case StripType::whenCount: {
        ui->dockWidget->setWindowTitle(QObject::tr("whenCount"));
        setHas_main_count_time_state(false, true, 0, true);
        setSpacer_stateHigh(true);
        break;
    }
    case StripType::whenTime: {
        ui->dockWidget->setWindowTitle(QObject::tr("whenTime"));
        setHas_main_count_time_state(false, false, 1, true);
        break;
    }
    case StripType::whenPin: {
        ui->stackedMain->setCurrentIndex(STACK_IND_WHENPIN);
        ui->dockWidget->setWindowTitle(QObject::tr("whenPin"));
        int chan_num = QString(getAtt(dom_state_strip, DOM_WHENPIN, ATT_NUMBER)).toInt(); //PIN 1-6
        int modeIndex = indexOf(QStringList(STR_L_INMODE)<<"true"<<"false", //[HIGH,LOW,RISING,DOWN,HIGH,LOW]
                                getAtt(dom_state_strip, DOM_WHENPIN, ATT_ISHIGH, STR_L_INMODE[0]));
        if (modeIndex>=4) {
            modeIndex-=4;
        }
        ui->comboBox_inChan->setCurrentIndex(chan_num - 1);
        ui->comboBox_inMode->setCurrentIndex(modeIndex);
        setHas_main_count_time_state(true, false, 0, true);
        setSpacer_stateHigh(true);
        break;
    }
    default:
        SCPP_ASSERT_THROW(0, "no such type");
    }

    /* time or count */
    if(has_time){
        auto dom = dom_state_strip.firstChildElement(DOM_TIME);
        int modeIndex  = indexOf(STR_L_RANDMODE, dom.attribute(ATT_TYPE)); //0-2
        ui->comboBox_timeMode->setCurrentIndex(modeIndex);
        att2ui(dom, DOM_FIXED, ATT_FIXED, ui->lineEdit_timeFixed);
        att2ui(dom, DOM_RANGE, ATT_FROM, ui->lineEdit_timeFrom);
        att2ui(dom, DOM_RANGE, ATT_TO, ui->lineEdit_timeTo);
        att2ui(dom, DOM_GOELSE, ATT_PROB, ui->lineEdit_timeProb);
        att2ui(dom, DOM_GOELSE, ATT_GO, ui->lineEdit_timeGo);
        att2ui(dom, DOM_GOELSE, ATT_ELSE, ui->lineEdit_timeElse);
    }
    if(has_count){
        auto dom = dom_state_strip.firstChildElement(DOM_COUNT);
        int modeIndex  = indexOf(STR_L_RANDMODE, dom.attribute(ATT_TYPE)); //0-2
        ui->comboBox_countMode->setCurrentIndex(modeIndex);
        att2ui(dom, DOM_FIXED, ATT_FIXED, ui->lineEdit_countFixed);
        att2ui(dom, DOM_RANGE, ATT_FROM, ui->lineEdit_countFrom);
        att2ui(dom, DOM_RANGE, ATT_TO, ui->lineEdit_countTo);
        att2ui(dom, DOM_GOELSE, ATT_PROB, ui->lineEdit_countProb);
        att2ui(dom, DOM_GOELSE, ATT_GO, ui->lineEdit_countGo);
        att2ui(dom, DOM_GOELSE, ATT_ELSE, ui->lineEdit_countElse);
    }
    if(has_state){
        auto dom = dom_state_strip.firstChildElement(DOM_NEXTSTATE);
        int modeIndex  = indexOf(STR_L_STATEMODE, dom.attribute(ATT_TYPE)); //0-2
        ui->comboBox_stateMode->setCurrentIndex(modeIndex);
        att2ui(dom, DOM_FIXED, ATT_FIXED, ui->lineEdit_stateFixed);
        att2ui(dom, DOM_RANGE, ATT_FROM, ui->lineEdit_stateFrom);
        att2ui(dom, DOM_RANGE, ATT_TO, ui->lineEdit_stateTo);
        att2ui(dom, DOM_GOELSE, ATT_PROB, ui->lineEdit_stateProb);
        att2ui(dom, DOM_GOELSE, ATT_GO, ui->lineEdit_stateGo);
        att2ui(dom, DOM_GOELSE, ATT_ELSE, ui->lineEdit_stateElse);
        bool iscomponent = dom.attribute(ATT_ISCOMP) == "true";
        ui->radioButton_isC->setChecked(iscomponent);
        ui->radioButton_isS->setChecked(!iscomponent);
    }

}
void StateContent::on_comboBox_outMode_currentIndexChanged(int index)
{
    qDebug()<<"on_comboBox_outMode_currentIndexChanged("<< index <<")";
    const int turn(0), keep(1), freq(2);
    const int stack_turn(0), stack_freq(1);
    QStackedWidget * stack_o = ui->stacked_outMode;
    QGroupBox * group_t = ui->group_time;
    switch (index) {
    case turn:
        stack_o->setCurrentIndex(stack_turn);
        stack_o->setVisible(true);
        group_t->setVisible(false);
        break;
    case keep:
        stack_o->setVisible(false);
        group_t->setVisible(true);
        break;
    case freq:
        stack_o->setCurrentIndex(stack_freq);
        stack_o->setVisible(true);
        group_t->setVisible(true);
        break;
    default:
        SCPP_ASSERT_THROW(0, "No such type");
    }
}

void StateContent::on_comboBox_var_activated(const QString &arg1)
{
    /* insert vars */
    if(arg1.isEmpty())
        return;
    ui->textEdit_Code->insertPlainText(arg1);
    ui->textEdit_Code->setFocus();
}

void StateContent::on_varAssign_changed(QList<QString> varNames, QList<double> varValues)
{
    Q_UNUSED(varValues);
    ui->comboBox_var->clear();
    ui->comboBox_var->addItem("");
    ui->comboBox_var->addItems(varNames);
}

void StateContent::on_pinAssign_changed(QList<bool> isIns, QList<int> Nums, QList<bool> isEnables, QList<QString> comms)
{
    int inChan_pre  = ui->comboBox_inChan->currentIndex();
    int outChan_pre = ui->comboBox_outChan->currentIndex();
    ui->comboBox_inChan->clear();
    ui->comboBox_outChan->clear();
    int c = isIns.count();
    for(int i=0; i<c; ++i){
        bool isIn = isIns[i];
        int Num  = Nums[i];
        bool isEnable = isEnables[i];
        QString comm = comms[i];
        QComboBox * comboBox = isIn ? ui->comboBox_inChan : ui->comboBox_outChan;
        QIcon itemIcon = QApplication::style()->standardIcon(isEnable?
                                QStyle::SP_DialogYesButton : QStyle::SP_DialogNoButton);
        QString pattern = isIn? "IN%1    %2" : "OUT%1    %2";
        QString itemText = pattern.arg(Num).arg(comm);
        comboBox->addItem(itemIcon, itemText);
    }
    ui->comboBox_inChan->setCurrentIndex(inChan_pre);
    ui->comboBox_outChan->setCurrentIndex(outChan_pre);
}
// [1] end

