#include "main.h"
#include "globalparas.h"
#include "setfile2ino.h"
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QDateTime>
#include <QList>
#include <QRegExp>
#include <QDir>
using namespace SETFILE2INO_PARA;
using namespace GLOBAL_PARA;
class DataStorage
{
private:
    QString data;
    DataStorage():data(""){}
    static DataStorage * instance;
public:
    static DataStorage * getInstance();
    void clear(){this->data.clear();}
    QString toString(){return this->data;}
    void append(const QString str){this->data.append(str);}
};
DataStorage * DataStorage::instance=NULL;
DataStorage * DataStorage::getInstance()
{
    if(instance==NULL)
        instance = new DataStorage();
    return instance;
}

void pL(QString strline);
void pL(int ntab, QString strline);
inline void pL1(QString strline){pL(1, strline);}
inline void pL2(QString strline){pL(2, strline);}

struct var_assign{
    QString name, value;
};
struct pin_assign{
    bool isIn, isEnable;
    unsigned int num;
    QString comm;
};

bool get_echoTask(QDomElement dom_root);
int get_recordLevel(QDomElement dom_root);
QString get_arduinoBoard(QDomElement dom_root);
QString get_startLevel(QDomElement dom_root);
QList<var_assign> get_varAssign(QDomElement dom_root);
QList<pin_assign> get_pinAssign(QDomElement dom_root);
int get_countC(QDomElement dom_root); //[1-nc]
int get_countCx(QDomElement dom_root, int numC); //[1-ns]
QList<QString> get_commCx(QDomElement dom_root, int numC);
void print_Cx_at(QDomElement dom_root, int numC);
void print_Sx_at(QDomElement dom_Sx, int numC, int numS);

void Setfile2INO::printIno(QString filePath)
{
    /* check file and load content */
    QFile fileXml(filePath);
    SCPP_ASSERT_THROW(fileXml.exists(), "file not exist!");
    SCPP_ASSERT_THROW(fileXml.open(QFile::ReadOnly | QFile::Text),"Cannot open file!");
    QDomDocument doc("doc");
    doc.setContent(&fileXml);
    fileXml.close();

    /* open or create an *.ino file */
    QFileInfo fi(filePath);
    QString outIno = QString("%1/%2%3").arg(fi.absolutePath()).arg(fi.baseName()).arg(INO_SUFFIX);
    QFile fileIno(outIno);
    SCPP_ASSERT_THROW(fileIno.open(QFile::WriteOnly | QFile::Text),"Cannot open file!");
    QTextStream out(&fileIno);
    out.setCodec("utf-8");

    /* get dom info */
    SCPP_ASSERT_THROW(!doc.isNull());
    QDomElement dom_root = doc.documentElement();
    QDomElement dom_profile = dom_root.firstChildElement(DOM_PROFILE);
    QDomElement dom_settings = dom_root.firstChildElement(DOM_SETTINGS);
    QDomElement dom_session = dom_root.firstChildElement(DOM_SESSION);
    SCPP_ASSERT_THROW(!dom_root.isNull() && !dom_profile.isNull()
                      && !dom_settings.isNull() && !dom_session.isNull());

    /* begin to write "pL"(print line)*/
    DataStorage::getInstance()->clear();

    /* title */
    //[1] if AI_REVERSE
    pL("///////////////////////////////////////////////////////////////////////////////////");
    pL("//////////////////////////////ArControl Style//////////////////////////////////////");
    //[2] if echo_task
    pL("// if echo task name");
    if(get_echoTask(dom_root)){
        pL("#define ECHO_TASKNAME   //echo this taskname(filename) in very begining");
        pL("const char taskName[] = __FILE__;");
    }
    //[3] how to start ArControl
    pL("// how to start ArControl");
    QString startLevel = get_startLevel(dom_root);
    pL(QString("#define %1").arg(startLevel));

    //[4] record level
    QString currentDir = QDir::currentPath();
    if(!currentDir.endsWith('/') && !currentDir.endsWith('\\'))
        currentDir += '/';
    pL("// recording level");
    int recordLevel = get_recordLevel(dom_root);
    if(recordLevel==1) {
        pL("#define NULL_RECORD ");
        pL(QString("#include \"%1ino/ArControl.h\"").arg(currentDir));
    }
    else if(recordLevel==2){
        pL(QString("#include \"%1ino/ArControl.h\"").arg(currentDir));
    }
    else if(recordLevel==3){
        QString arduinoBoard = get_arduinoBoard(dom_root);
        int arduinoBoardIndex = indexOf(STR_L_ARDUINOBOARD, arduinoBoard); //Uno=1; Mega=2;
        if(arduinoBoardIndex == 0){
            pL("#define UNO_SPEEDUP //only helpful to ArControl_AllinOne.h, improve AI-scaning");
        }
        else if(arduinoBoardIndex == 1){
            pL("#define MEGA_SPEEDUP //only helpful to ArControl_AllinOne.h, improve AI-scaning");
        }
        else{
            pL("#define NANO_SPEEDUP //only helpful to ArControl_AllinOne.h, improve AI-scaning");
        }
        pL("#define AI2IN 1		//AIx -> INy");
        pL("#define DO2OUT -1	//DOx -> OUTy");
        pL(QString("#include \"%1ino/ArControl_AllinOne.h\"").arg(currentDir));
    }
    pL("");

    /* task infomation (data file header) */
    pL("///////////////////////////////////////////////////////////////////////////////////");
    pL("///////////////////////////////task info///////////////////////////////////////////");
    pL("#define INFO \"\" \\");
    QList<pin_assign> L_pin_assign = get_pinAssign(dom_root);
    foreach(pin_assign data_item, L_pin_assign){
        if(!data_item.isEnable)
            continue;
        QString type = (data_item.isIn?"IN":"OUT");
        pL2(QString("\"@%1%2:%3\\n\" \\").arg(type).arg(data_item.num).arg(data_item.comm));
    }
    int countC = get_countC(dom_root);
    for(int iC=1; iC<=countC; ++iC){
        QList<QString> data_comm = get_commCx(dom_root, iC); //comments: Cx->CxS1->CxS2->...
        QString iC_comm = data_comm.at(0);
        pL2(QString("\"@C%1:%2\\n\" \\").arg(iC).arg(iC_comm));
        int countS = data_comm.count() - 1;
        for(int iS=1; iS<=countS; iS++){
           QString iS_comm = data_comm.at(iS);
           pL2(QString("\"@C%1S%2:%3\\n\" \\").arg(iC).arg(iS).arg(iS_comm));
        }
    }
    pL2("\"\"");
    pL("");

    /* global vars */
    pL("///////////////////////////////////////////////////////////////////////////////////");
    pL("///////////////////////////////global vars/////////////////////////////////////////");
    QList<var_assign> L_var_assign = get_varAssign(dom_root);
    foreach(var_assign data_item, L_var_assign){
        pL(QString("double %1 = %2;").arg(data_item.name).arg(data_item.value));
    }
    pL("");
    /* pin comment and default-assignment */
    pL("///////////////////////////////////////////////////////////////////////////////////");
    pL("//////////////////////////////Hardware Setup///////////////////////////////////////");
    foreach(pin_assign data_item, L_pin_assign){
        if(!data_item.isEnable)
            continue;
        QString type = (data_item.isIn?" IN":"OUT");
        int pinNum = data_item.num + (data_item.isIn? -1: 1);
        pL(QString("const int %1%2 = %3; \t//%4").arg(type).arg(data_item.num).arg(pinNum).arg(data_item.comm));
    }
    pL("void hardware_SETUP(){");
    pL1("Serial.print(F(INFO));");
    foreach(pin_assign data_item, L_pin_assign){
        if(data_item.isIn || !data_item.isEnable)
            continue;
        pL1(QString("pinMode(OUT%1, OUTPUT);").arg(data_item.num));
    }
    pL("}");
    pL("");
    /* session states */
    pL("///////////////////////////////////////////////////////////////////////////////////");
    pL("///////////////////////////////State Setup/////////////////////////////////////////");
    QString tempStr = "";
    for(int iC=1; iC<=countC; ++iC){
        pL(QString("#define numC%1 %2").arg(iC).arg(get_countCx(dom_root, iC)));
        if(iC==countC)
            tempStr += QString("numC%1").arg(iC);
        else
            tempStr += QString("numC%1 + ").arg(iC);
    }
    pL(QString("State stateList[%1];").arg(tempStr));
    for(int iC=1; iC<=countC; ++iC){
        pL(QString("State *C%1S[numC%1 + 1];").arg(iC));
    }
    pL(QString("State *C[%1];").arg(countC+1));
    pL("void State_SETUP()");
    pL("{");
    pL("///Session ");
    pL1("int i, j=0;");
    for(int iC=1; iC<=countC; ++iC){
        pL1(QString("for(i=1; i<=numC%1; ++i)").arg(iC));
        pL2(QString("C%1S[i]=stateList[j++].setCiSi(%1,i); //pick \"* State\" one by one").arg(iC));

    }
    QString tempStr_2, tempStr_3;
    for(int iC=1; iC<=countC; ++iC){
        tempStr_2 += QString("C%1S[0] = ").arg(iC);
        tempStr_3 += QString("C[%1] = C%1S[1]; ").arg(iC);
    }
    pL1(tempStr_2 + QString("C[0] = State::ENDSTATE;//End of Session"));
    pL1(tempStr_3 + QString("//Entry of Component"));
    pL1( "State::NEXTSTATE = C[1];//Entry of Session");

    /* inside Component and state */
    for(int iC=1; iC<=countC; ++iC){
        print_Cx_at(dom_root, iC);
        pL("");
    }
    pL("}");
    pL("//finished ArControl task");

    /* write to file */
    qDebug()<<"===ino===\n"<<qPrintable(DataStorage::getInstance()->toString())<<"===end===\n";
    out<< DataStorage::getInstance()->toString();
    fileIno.close();
}

// [2] sub funtions
void pL(QString strline)
{
    QString printData;
    printData += strline;
    printData += '\n';
    DataStorage::getInstance()->append(printData);
}
void pL(int ntab, QString strline)
{
    QString printData;
    for(int i=0; i<ntab;++i)
        printData += '\t';
    printData += strline;
    printData += '\n';
    DataStorage::getInstance()->append(printData);
}

bool get_echoTask(QDomElement dom_root) /* [true | false] */
{
    QString text = dom_root.firstChildElement(DOM_PROFILE).firstChildElement(DOM_ECHOTASK).text();
    SCPP_ASSERT_THROW( (QStringList()<<"true"<<"false").contains(text) );
    return text=="true";
}
int get_recordLevel(QDomElement dom_root) /* [1 | 2 | 3] */
{
    QString text = dom_root.firstChildElement(DOM_PROFILE).firstChildElement(DOM_RECORDLEVEL).text();
    SCPP_ASSERT_THROW( (QStringList()<<"1"<<"2"<<"3").contains(text) );
    return text.toInt();
}
QString get_arduinoBoard(QDomElement dom_root) /* [Uno | Mega | Nano] */
{
    QString text = dom_root.firstChildElement(DOM_PROFILE).firstChildElement(DOM_ARDUINOBOARD).text();
    SCPP_ASSERT_THROW( STR_L_ARDUINOBOARD.contains(text) );
    return text;
}
QString get_startLevel(QDomElement dom_root) /* [0 | 1 | 2 | 3] */
{
    QString text = dom_root.firstChildElement(DOM_PROFILE).firstChildElement(DOM_START).text();
    SCPP_ASSERT_THROW( STR_L_START.contains(text) );
    return text;
}
QList<var_assign> get_varAssign(QDomElement dom_root)
{
    QDomElement dom = dom_root.firstChildElement(DOM_SETTINGS).firstChildElement(DOM_VARALL);
    QDomElement dom_item = dom.firstChildElement(DOM_VARALL_VAR);
    QList<var_assign> data;
    while(!dom_item.isNull()) {
        var_assign data_item;
        data_item.name = dom_item.attribute(ATT_NAME);
        data_item.value = dom_item.attribute(ATT_VALUE);
        data<<data_item;
        dom_item = dom_item.nextSiblingElement(DOM_VARALL_VAR);
    }
    return data;
}
QList<pin_assign> get_pinAssign(QDomElement dom_root)
{
    QDomElement dom = dom_root.firstChildElement(DOM_SETTINGS).firstChildElement(DOM_PINALL);
    QDomElement dom_item = dom.firstChildElement(DOM_PINALL_PIN);
    QList<pin_assign> data;
    while(!dom_item.isNull()) {
        pin_assign data_item;
        data_item.isIn = dom_item.attribute(ATT_TYPE) == "IN"; //"IN" , "OUT"
        data_item.isEnable = dom_item.attribute(ATT_ISENB) == "true"; //"true" , "false"
        data_item.comm = dom_item.attribute(ATT_COMM);
        data_item.num = dom_item.attribute(ATT_NUMBER).toInt();
        data<<data_item;
        dom_item = dom_item.nextSiblingElement(DOM_PINALL_PIN);
    }
    qDebug()<<"get_pinAssign() data c="<< data.count();
    return data;
}
int get_countC(QDomElement dom_root)
{
    QDomElement dom = dom_root.firstChildElement(DOM_SESSION);
    QDomElement dom_c = dom.firstChildElement(DOM_COMPONENT);
    int counter=0;
    while(!dom_c.isNull()){
        ++counter;
        dom_c = dom_c.nextSiblingElement(DOM_COMPONENT);
    }
    return counter;
}
int get_countCx(QDomElement dom_root, int numC)
{
    SCPP_ASSERT_THROW( numC>0 && numC <= get_countC(dom_root));
    QDomElement dom = dom_root.firstChildElement(DOM_SESSION);
    QDomElement dom_c = dom.firstChildElement(DOM_COMPONENT);
    for(int i=1; i<numC; ++i)
        dom_c = dom_c.nextSiblingElement(DOM_COMPONENT);
    QDomElement dom_s = dom_c.firstChildElement(DOM_STATE);
    int counter = 0;
    while(!dom_s.isNull()){
        ++counter;
        dom_s = dom_s.nextSiblingElement(DOM_STATE);
    }
    return counter;
}

QList<QString> get_commCx(QDomElement dom_root, int numC)
{
    /* 获取Cx 和 CxSx 的注释:
     * QList<QString> = Comments in Cx->CxS1->CxS2->CxS3->...
     */
    SCPP_ASSERT_THROW( numC>0 && numC <= get_countC(dom_root));
    QDomElement dom = dom_root.firstChildElement(DOM_SESSION);
    QDomElement dom_c = dom.firstChildElement(DOM_COMPONENT);
    for(int i=1; i<numC; ++i)
        dom_c = dom_c.nextSiblingElement(DOM_COMPONENT);

    QList<QString> data;
    QString Cx_comm = dom_c.attribute(ATT_COMM);
    data << Cx_comm;
    QDomElement dom_s = dom_c.firstChildElement(DOM_STATE);
    for(int numS=1; true; ++numS) {
        if(dom_s.isNull())
            break;
        QString CxSx_comm = dom_s.attribute(ATT_COMM);
        data << CxSx_comm;
        dom_s = dom_s.nextSiblingElement(DOM_STATE);
    }
    return data;
}

void print_Cx_at(QDomElement dom_root, int numC)
{
    SCPP_ASSERT_THROW( numC>0 && numC <= get_countC(dom_root));
    QDomElement dom = dom_root.firstChildElement(DOM_SESSION);
    QDomElement dom_c = dom.firstChildElement(DOM_COMPONENT);
    for(int i=1; i<numC; ++i)
        dom_c = dom_c.nextSiblingElement(DOM_COMPONENT);
    /* print Cx and Comment */
    QString comm = dom_c.attribute(ATT_COMM);
    pL(QString("///Component [%1]:%2").arg(numC).arg(comm));
    /* go inside state */
    QDomElement dom_s = dom_c.firstChildElement(DOM_STATE);
    for(int numS=1; true; ++numS) {
        if(dom_s.isNull())
            break;
        print_Sx_at(dom_s, numC, numS);
        dom_s = dom_s.nextSiblingElement();
    }
}
void print_Sx_at(QDomElement dom_s, int numC, int numS)
{
    /* print CxSx and Comment */
    QString comm = dom_s.attribute(ATT_COMM);
    pL1(QString("///C%1S[%2]: %3").arg(numC).arg(numS).arg(comm));
    QDomElement dom_s_s = dom_s.firstChildElement(DOM_SSTRIP);
    while(!dom_s_s.isNull()) {
        bool isEnable = dom_s_s.attribute(ATT_ISENB)=="true";
        QString s_type = dom_s_s.attribute(ATT_TYPE);
        SCPP_ASSERT_THROW(STR_L_STYPE.contains(s_type));
        StripType type = (StripType) indexOf(STR_L_STYPE, s_type);
        QString record_cond, name_switch;
        const QString CxSx = QString("C%1S[%2]").arg(numC).arg(numS);
        const QString Strip_comm = dom_s_s.attribute(ATT_COMM);
        bool has_state=false;
        if(isEnable){
            auto pick_t_c_s = [=](QDomElement dom_t_c_s)->QString{
                QString type = dom_t_c_s.attribute(ATT_TYPE);
                QString data;
                QDomElement dom_type = dom_t_c_s.firstChildElement(type);
                if(type == DOM_FIXED){
                    data = dom_type.attribute(ATT_FIXED);
                }
                else if(type == DOM_GOELSE){
                    data = QString("cpp_GoElse(%1,%2,%3)").arg(dom_type.attribute(ATT_PROB))
                            .arg(dom_type.attribute(ATT_GO)).arg(dom_type.attribute(ATT_ELSE));
                }
                else if(type == DOM_RANGE){
                    data = QString("cpp_Range_%1(%2,%3)").arg(dom_t_c_s.tagName()==DOM_TIME?"float":"int")
                            .arg(dom_type.attribute(ATT_FROM)).arg(dom_type.attribute(ATT_TO));
                }
                return data;
            };
            /* print do or condition */
            switch(type){
            case StripType::doVar :{
                QString format = "\t%1->dovar = []()-> void{%2\n\t};";
                QString inner = dom_s_s.text();
                inner.replace(QRegExp("^|\r|\r\n|\n"), "\n\t\t"); //add "\t\t" to linebegin
                record_cond = format.arg(CxSx).arg(inner);
                break;
            }
            case StripType::doPin :{
                QString format = "\t%1->dofun = []()-> void {%2}; //%3";
                QString inner;
                QDomElement dom_dopin = dom_s_s.firstChildElement(DOM_DOPIN);
                QString t = pick_t_c_s(dom_s_s.firstChildElement(DOM_TIME));
                QString numPin = dom_dopin.attribute(ATT_NUMBER);
                bool isTurnon  = dom_dopin.firstChildElement(DOM_TURN).attribute(ATT_ISENB) == "true";
                QString duty   = dom_dopin.firstChildElement(DOM_BLINK).attribute(ATT_DUTY);
                QString freq   = dom_dopin.firstChildElement(DOM_BLINK).attribute(ATT_FREQ);
                QString outmode= dom_dopin.attribute(ATT_TYPE);
                if(outmode==DOM_TURN){
                    inner = QString("digitalWrite(OUT%1, %2);").arg(numPin).arg(isTurnon?"HIGH":"LOW");
                }
                else if(outmode==DOM_KEEPON){
                    inner = QString("cpp_keepon(OUT%1, %2);").arg(numPin).arg(t);
                }
                else if(outmode==DOM_BLINK){
                    inner = QString("cpp_Hz_A(OUT%1, %2, %3, %4);").arg(numPin).arg(t).arg(freq).arg(duty);
                }
                record_cond = format.arg(CxSx).arg(inner).arg(Strip_comm);
                break;
            }
            case StripType::whenVar :{
                QString format = "\t%1->varListener = []()-> bool {%2;\n\t}; //%3";
                QString inner = dom_s_s.text();
                inner.replace(QRegExp("^|\r|\r\n|\n"), "\n\t\t"); //add "\t\t" to linebegin
                record_cond = format.arg(CxSx).arg(inner).arg(Strip_comm);
                name_switch = "varListenerSTATE";
                has_state = true;
                break;
            }
            case StripType::whenCount :{
                QString format = "\t%1->CountSet = []()-> int {%2}; //%3";
                name_switch = "CountSetSTATE";
                QString countn = pick_t_c_s(dom_s_s.firstChildElement(DOM_COUNT));
                QString inner = QString("return %1;").arg(countn);
                record_cond = format.arg(CxSx).arg(inner).arg(Strip_comm);
                has_state = true;
                break;
            }
            case StripType::whenTime :{
                QString format = "\t%1->TimerSet = []()-> float {%2}; //%3";
                name_switch = "TimerSetSTATE";
                QString t = pick_t_c_s(dom_s_s.firstChildElement(DOM_TIME));
                QString inner = QString("return %1;").arg(t);
                record_cond = format.arg(CxSx).arg(inner).arg(Strip_comm);
                has_state = true;
                break;
            }
            case StripType::whenPin :{
                QString format = "\t%1->evtListener = []()-> bool {%2}; //%3";
                name_switch = "evtListenerSTATE";
                QString numPin = dom_s_s.firstChildElement(DOM_WHENPIN).attribute(ATT_NUMBER);
                int modeIndex = indexOf(STR_L_INMODE, dom_s_s.firstChildElement(DOM_WHENPIN).attribute(ATT_ISHIGH, STR_L_INMODE[0]));
                QString inner;
                if (modeIndex==0){ //is High
                    inner = QString("return cpp_ListenAI(IN%1, HIGH);").arg(numPin);
                }else if (modeIndex==1){ //is Low
                    inner = QString("return cpp_ListenAI(IN%1, LOW);").arg(numPin);
                }else if (modeIndex==2){ //rising edge
                    inner = QString("return cpp_ListenAI_edge(IN%1, HIGH);").arg(numPin);
                }else { //downing edge
                    inner = QString("return cpp_ListenAI_edge(IN%1, LOW);").arg(numPin);
                }
                record_cond = format.arg(CxSx).arg(inner).arg(Strip_comm);
                has_state = true;
                break;
            }} // end switch
            /* print goto-state */
            pL(record_cond);
            if(has_state){
                QDomElement dom_nextstate = dom_s_s.firstChildElement(DOM_NEXTSTATE);
                if(dom_nextstate.attribute(ATT_TYPE) == DOM_ENDUP) {
                    QString format = "\t%1->%2 = []()-> State* {return C[0];};";
                    QString record_gostate = format.arg(CxSx).arg(name_switch);
                    pL(record_gostate);
                }
                else {
                    QString csNum = pick_t_c_s(dom_nextstate);
                    bool isComponent = dom_s_s.firstChildElement(DOM_NEXTSTATE).attribute(ATT_ISCOMP)=="true";
                    QString CxS = isComponent? "C" : QString("C%1S").arg(numC);
                    QString format = "\t%1->%2 = []()-> State* {int n=%3; return %4[n];};";
                    QString record_gostate = format.arg(CxSx).arg(name_switch).arg(csNum).arg(CxS);
                    pL(record_gostate);
                }
            }
            if(type == StripType::whenPin){
                pL(QString("\t%1->addlisten();").arg(CxSx));
            }
        }//end if
        dom_s_s = dom_s_s.nextSiblingElement(DOM_SSTRIP);
    }
}
// [2] end
