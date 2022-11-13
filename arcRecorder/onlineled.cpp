#include <QSpacerItem>
#include <QDebug>
#include "onlineled.h"

using namespace ONLINELED_PARA;
using namespace ONLINELED_PRIVATE;

QLineEditExt::QLineEditExt(QString s): QLineEdit(s)
{

}

void QLineEditExt::mouseDoubleClickEvent(QMouseEvent * event){
    emit doubleClicked();
}

OnlineLED::OnlineLED(QObject *parent,
                       const QHash<QString, StreamItem> * mdatahash,
                       QGridLayout * gLayout)
    : QObject(parent), datahash(mdatahash), GLayout(gLayout)
{
    QLabel * LB_in = new QLabel("IN",GLayout->parentWidget());
    QLabel * LB_out = new QLabel("OUT",GLayout->parentWidget());
    GLayout->addWidget(LB_in,  0, 0);
    GLayout->addWidget(LB_out, 0, 1);
    GLayout->setContentsMargins(0,0,0,0);
    LB_in->setAlignment(Qt::AlignHCenter);
    LB_out->setAlignment(Qt::AlignHCenter);
    for(int i=0; i<6; i++){
        LE_inleds[i] = new QLineEditExt(QString("[ %1 ]").arg(i+1));
        GLayout->addWidget(LE_inleds[i], i+1, 0);
        LE_allleds[i] = LE_inleds[i];
        connect(LE_inleds[i], &QLineEditExt::doubleClicked, [=](){emit this->swithlevel_inpin(i+1);});
    }

    for(int i=0; i<8; i++){
        LE_outleds[i] = new QLineEditExt(QString("[ %1 ]").arg(i+1));
        GLayout->addWidget(LE_outleds[i], i+1, 1);
        LE_allleds[i+6] = LE_outleds[i];
        connect(LE_outleds[i], &QLineEditExt::doubleClicked, [=](){emit this->swithlevel_outpin(i+1);});
    }
    GLayout->setRowStretch(0, 0);

    GLayout->addItem(new QSpacerItem(1,30, QSizePolicy::Minimum, QSizePolicy::Expanding),9, 0);
    for(int i=0; i<14; i++){
        LE_allleds[i]->setReadOnly(true);
        LE_allleds[i]->setAlignment(Qt::AlignHCenter);
        LE_allleds[i]->setMaximumWidth(50);
    }
    clean_led();
    GLayout->setHorizontalSpacing(0);
    GLayout->setVerticalSpacing(10);
    /* 添加INPUT LED */

}
void OnlineLED::clean_led()
{
    for(int i=0;i<14; ++i){
        LE_allleds[i]->setStyleSheet(lowValueBG);
        v_allleds[i] = 0;
        counts_allleds[i] = 0;
        islastblink_allleds[i] = 0;
    }
}

void OnlineLED::receive_everyTimeCycle()
{
    QString strlist[6+8];
    for(int i=0; i<6; ++i){
        strlist[i] = QString("IN%1").arg(i+1);
    }
    for(int i=0; i<8; ++i){
        strlist[i+6] = QString("OUT%1").arg(i+1);
    }
    for(int i=0; i<14; ++i){
        bool v_now = (*datahash)[strlist[i]].ishigh;
        bool v_pre = v_allleds[i];
        qint32 count_now = (*datahash)[strlist[i]].count;
        qint32 count_pre = counts_allleds[i];
        qint32 d_count = count_now - count_pre;
        if((d_count==1 && v_pre==0) || d_count>1){
            /* 处于blink 时期，LED灯亮度减半 */
            LE_allleds[i]->setStyleSheet(blinkValueBG);
            islastblink_allleds[i] = true;
        }
        else if((v_now != v_pre)||islastblink_allleds[i]){
            /* 不处于 blink 时期，LED灯只能高亮或关闭 */
            LE_allleds[i]->setStyleSheet(v_now?highValueBG:lowValueBG);
            islastblink_allleds[i] = false;
        }
        v_allleds[i] = v_now;
        counts_allleds[i] = count_now;
    }
}
