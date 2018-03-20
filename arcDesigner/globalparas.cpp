#include "globalparas.h"
QDomElement createTempElement(QString nodeName)
{
    static QDomDocument doc("doc_temp");
    QDomElement dom = doc.createElement(nodeName);
    doc.appendChild(dom);
    return dom;
}

void printDomElement(QDomElement dom)
{
    QDomDocument temp("show Dom");
    temp.appendChild(dom.cloneNode());
    qDebug(qPrintable(temp.toString()));
}

int indexOf(const QStringList strlist, const QString str){
    int i=-1;
    foreach(QString temp, strlist){
        ++i;
        if(temp == str)
            return i;
    }
    return -1;
}

void trDeclaration() //in order to support multi_language translation, by LINGUIST
{
    while(0){
        QObject::tr("doVar");
        QObject::tr("doPin");
        QObject::tr("whenVar");
        QObject::tr("whenCount");
        QObject::tr("whenTime");
        QObject::tr("whenPin");
        QObject::tr("Modify the [VARIABLE] as defined in MENU>EDIT>Var_Assignment.");
        QObject::tr("Active the [OUT-Pin] as defined in MENU>EDIT>Pin_Assignment.");
        QObject::tr("When meet the condition of [VARIABLE expression], goto designed STATE.");
        QObject::tr("When meet the condition of [entry counter], goto designed STATE.");
        QObject::tr("When meet the condition of [timer out], goto designed STATE.");
        QObject::tr("When meet the condition of [IN-Pin triggered], goto designed STATE.");
        QObject::tr("- etc: \"3\"  =>  \"C3S1\" or \"S3\"\n "\
                    "- WARN: \"0\"  =>  treat as SESION END");
        QObject::tr("- etc: \"3\"  => the counter(c) increase at this STATE-entry, \n" \
                                 "              when (c) equal to \"3\" , goto NextState & reset (c) \n"
                                 "- WARN: \"0\"  =>  treat as \"+∞\", will never be satisfied");
        QObject::tr("- etc: \"3\"  =>  \"3\" sec \n" \
                                "- WARN: \"0\" =>  treat as \"+∞\", will never be satisfied");
        QObject::tr("- WARN: NEVER try BLINK in ArControl-LEVEL3");
        QObject::tr("- etc: \"3\"  =>  3Hz");
        QObject::tr("- etc: \"3\"  =>  3% on, 97% off" );
        QObject::tr("- etc: \"3\"  =>  P(GO)=3%, P(ELSE)=97%");
        QObject::tr("- CHECKED: It's trigged when signal is HIGH. \n"\
                    "- UNCHECKED:            wehn signal is LOW!");
        QObject::tr("COMMENT itself won't affect task running at all, "\
                    "but it's a good way to write notes.");
    }
}
