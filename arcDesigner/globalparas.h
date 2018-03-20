#ifndef GLOBALPARAS_H
#define GLOBALPARAS_H
#include <QObject>
#include <QDomDocument>
#include "main.h"
namespace  GLOBAL_PARA {
    const QString DOM_PINALL("PIN_ASSIGN"), DOM_ARDUINO("ARDUINO_DEBUG"),
                  DOM_ARC("ARC_DESIGNER"), DOM_SETTINGS("SETTINGS"), DOM_PROFILE("PROFILE"),
                  DOM_VARALL("VAR_ASSIGN"),  DOM_PINALL_PIN("PIN"), DOM_VARALL_VAR("VAR"),
                  DOM_INFO("INFO"), ATT_VERSION("version"), ATT_DATE("date"),
                  ATT_VARNAME("name"), ATT_VARVALUE("value"), ATT_NUMBER("number"),
                  ATT_COMM("comment"), ATT_ISENB("isEnable"), ATT_NAME("name"),
                  VAL_VERSION("0.8"), VAL_SOFTNAME("ArControl Designer"), ATT_TYPE("type");
    const QString DOM_SESSION("SESSION"), DOM_COMPONENT("COMPONENT"), DOM_STATE("STATE"),
                  DOM_SSTRIP("STATE_STRIP"), DOM_CODE("CODE"), DOM_DOPIN("doPin"),
                  DOM_KEEPON("keepon"), DOM_BLINK("blink"), DOM_TURN("turnon"),
                   ATT_FREQ("freq"), ATT_DUTY("duty"), DOM_WHENPIN("whenPin"),
                  DOM_TIME("time"), DOM_COUNT("count"), DOM_NEXTSTATE("nextstate"),
                  DOM_FIXED("fixed"), DOM_RANGE("range"), DOM_GOELSE("goelse"),
                  ATT_FIXED("fixed"), ATT_FROM("from"), ATT_TO("to"), ATT_PROB("probability"),
                  ATT_GO("go"), ATT_ELSE("else"), ATT_ISCOMP("iscomponent"),
                  ATT_ISHIGH("isHigh");
    const QString DOM_RECORDLEVEL("RECORD_LEVEL"), DOM_ECHOTASK("ECHO_TASK"),
                  DOM_START("START_MODEL"), DOM_LANG("LANG"), ATT_VALUE("value");
    const QStringList STR_L_STYPE = QStringList()
            <<"doVar"<<"doPin"<<"whenVar"<<"whenCount"<<"whenTime"<<"whenPin"; //state strip type
    const QStringList STR_L_STYPE_HINT = QStringList()
            <<"Modify the [VARIABLE] as defined in MENU>EDIT>Var_Assignment."
            <<"Active the [OUT-Pin] as defined in MENU>EDIT>Pin_Assignment."
            <<"When meet the condition of [VARIABLE expression], goto designed STATE."
            <<"When meet the condition of [entry counter], goto designed STATE."
            <<"When meet the condition of [timer out], goto designed STATE."
            <<"When meet the condition of [IN-Pin triggered], goto designed STATE.";
    const QStringList STR_L_OUTMODE = QStringList()
            << DOM_TURN << DOM_KEEPON << DOM_BLINK;
    const QStringList STR_L_RANDMODE = QStringList()
            << DOM_FIXED << DOM_RANGE << DOM_GOELSE;
    const QStringList STR_L_LANG = QStringList()
            << "English" << "简体中文";
    const QStringList STR_L_LANG_TRANSLATOR = QStringList()
            << "" << ":/cn.qm";
    const QStringList STR_L_BOOL = QStringList()
            << "false" << "true";
    const QStringList STR_L_START = QStringList()
            << "START_SOON" <<  "START_HARD" <<"START_SOFT" << "START_HARD_SOFT";
    enum StripType{doVar, doPin, whenVar, whenCount, whenTime, whenPin};
    const QString PROFILE_FILE("profile.xml");
    const int IN_CHAN_COUNT(6), OUT_CHAN_COUNT(8);
    const QStringList COLOR_TYPE = QStringList()
            << "rgb(251, 103, 109)" <<"rgb(255, 65, 85)"  //doVar doPin
            << "rgb(4, 116, 217)" << "rgb(83, 140, 49)"   //whenVar whenCount
            << "rgb(80, 169, 194)" << "rgb(38, 222, 168)"; //whenTime whenPin
    const QString STYLE_LABEL  = "QLabel {background-color:%1;};";
    const QString COMMENT_HINT("COMMENT itself won't affect task running at all, "\
                               "but it's a good way to write notes.");
    const QString STATE_HINT("- etc: \"3\"  =>  \"C3S1\" or \"S3\"\n "\
                             "- WARN: \"0\"  =>  treat as SESION END");
    const QString WHEN_ISHIGH_HINT("- CHECKED: It's trigged when signal is HIGH. \n"\
                                   "- UNCHECKED:            wehn signal is LOW!");
    const QString COUNT_HINT("- etc: \"3\"  => the counter(c) increase at this STATE-entry, \n" \
                             "              when (c) equal to \"3\" , goto NextState & reset (c) \n"
                             "- WARN: \"0\"  =>  treat as \"+∞\", will never be satisfied");
    const QString TIME_HINT("- etc: \"3\"  =>  \"3\" sec \n" \
                            "- WARN: \"0\" =>  treat as \"+∞\", will never be satisfied");
    const QString OUTMODE_HINT("- WARN: NEVER try BLINK in ArControl-LEVEL3");
    const QString FREQ_HINT("- etc: \"3\"  =>  3Hz");
    const QString DUTY_HINT("- etc: \"3\"  =>  3% on, 97% off" );
    const QString PROB_HINT("- etc: \"3\"  =>  P(GO)=3%, P(ELSE)=97%");
}

QDomElement createTempElement(QString nodeName);
void printDomElement(QDomElement dom);
int indexOf(const QStringList, const QString);
#endif // GLOBALPARAS_H
