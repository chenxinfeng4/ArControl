#ifndef ONLINELED_H
#define ONLINELED_H

#include <QObject>
#include <QHash>
#include <QColor>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include "serialflowcontrol.h"
#include "main.h"


class QTime;

namespace ONLINELED_PARA
{
    static const char highValueBG[] = "background-color: #11FF00";
    static const char blinkValueBG[] = "background-color: #B1F1C3";
    static const char lowValueBG[]= "background-color: white";
}

namespace ONLINELED_PRIVATE
{
}

class QLineEditExt : public QLineEdit
{
    Q_OBJECT
public:
    QLineEditExt(QString s);
protected:
    void mouseDoubleClickEvent(QMouseEvent *);
signals:
    void doubleClicked();
};

class OnlineLED : public QObject
{
    Q_OBJECT
public:
    explicit OnlineLED(QObject *parent,
                         const QHash<QString, StreamItem> * mdatahash,
                         QGridLayout * gLayout);
private:
    const QHash<QString, StreamItem> * datahash; //SerialFlowControl::datahash
    QGridLayout * GLayout;
    QLineEdit *LE_inleds[6];
    QLineEditExt *LE_outleds[8];
    QLineEdit *LE_allleds[14];
    bool v_allleds[6+8];
    bool islastblink_allleds[6+8];
    qint32 counts_allleds[6+8];
signals:
    void swithlevel_outpin(int outpin);
public slots:
    void clean_led();
    void receive_everyTimeCycle();

};


#endif // ONLINELED_H
