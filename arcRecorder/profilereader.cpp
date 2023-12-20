
#include "profilereader.h"
#include "globalparas.h"
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

using namespace PROFILEREADER_PARA;
using namespace PROFILEREADER_PRIVATE;
using namespace GLOBAL_PARA;

ProfileReader * ProfileReader::instance = NULL;
ProfileReader::ProfileReader(QObject *parent):QObject(parent)
{
}
ProfileReader::ProfileReader(const ProfileReader&):QObject(nullptr)
{
}
ProfileReader& ProfileReader::operator=(const ProfileReader&)
{
}

ProfileReader* ProfileReader::getInstance()
{
    if(ProfileReader::instance == NULL){
        ProfileReader::instance = new ProfileReader();
        ProfileReader::instance->checkProfile();
    }
    return ProfileReader::instance;
}

QString ProfileReader::getArduino()
{
    if(!this->hasChecked)
        this->checkProfile();
    this->hasChecked=true;
    return this->arduino_debug;
}

QString ProfileReader::getBoard()
{
    if(!this->hasChecked)
        this->checkProfile();
    this->hasChecked=true;
    return this->arduino_board;
}

void ProfileReader::checkProfile(bool newapath)
{
    QFileInfo finfo(PROFILE_FILE);
    QFile f(PROFILE_FILE);
    QDomDocument doc("doc");
    if(!finfo.exists() || !finfo.isFile()){
        /* create */
        qDebug()<<"No profile exsited, will create!";
        Q_ASSERT(f.open(QFile::ReadWrite | QFile::Text));
        doc.setContent(&f);
        QDomProcessingInstruction  instruction =  doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild(instruction);
        QDomElement root = doc.createElement(DOM_PROFILE);

        QDomElement dom_lang = doc.createElement(DOM_LANG);
        dom_lang.appendChild(doc.createTextNode(STR_L_LANG[0]));
        root.appendChild(dom_lang);

        QDomElement dom_arduinoboard = doc.createElement(DOM_ARDUINOBOARD);
        dom_arduinoboard.appendChild(doc.createTextNode(STR_L_ARDUINOBOARD[0]));
        root.appendChild(dom_arduinoboard);

        doc.appendChild(root);
        QTextStream out(&f);
        doc.save(out, QDomNode::EncodingFromDocument);
        f.close();
    }
    /* 矫正 文件 */
    qDebug()<<"Profile exsited";
    SCPP_ASSERT_THROW(f.open(QFile::ReadWrite | QFile::Text), "cannot open");
    doc.clear();
    doc.setContent(&f);
    QDomElement root = doc.documentElement();
    if(root.isNull()){
        root = doc.createElement(DOM_PROFILE);
        doc.appendChild(root);
    }
    QDomElement dom_arduino = root.firstChildElement(DOM_ARDUINO);
    if(dom_arduino.isNull()){
        dom_arduino = doc.createElement(DOM_ARDUINO);
        root.appendChild(dom_arduino);
    }
    if(dom_arduino.firstChildElement().isNull()){
        dom_arduino.appendChild(doc.createTextNode(""));
    }
    QString arduino_path = dom_arduino.text();

    /* 判断平台 */
    QString debugFile;
    bool reChoose = false;
    QFileInfo debuger(arduino_path);
#ifdef Q_OS_WIN
    debugFile = "arduino_debug.exe";
#endif
#ifdef Q_OS_LINUX
    debugFile = "arduino-cli";
#endif
#ifdef Q_OS_MAC
    debugFile = "arduino.app";
#endif

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    reChoose =(debugFile.isEmpty() || !debuger.exists() || !debuger.isFile() || !debuger.isExecutable());
    if(newapath || reChoose){
        QMessageBox::about(0, tr("About"),
                           tr("Choose Arduino IDE according to OS:\n"\
                              "-- WINDOWS: arduino_path/arduino_debug.exe\n"\
                              "-- LINUX  : arduino_path/arduino\n"\
                              "-- MAC    : arduino_path/arduino.app\n\n" \
                              "<Download>: https://www.arduino.cc/en/Main/Software\n" \
                              "<  Note  >: the Arduino IDE edition should be greater than 1.6.8!\n"\
                              "<  Note  >: You'd better set it within ARCDESIGNER>Profile!"));
         arduino_path = QFileDialog::getOpenFileName(0, tr("Choose Arduino IDE"),
                                                     "", debugFile); //may rais an-warning on debug-output, don't warry
         if(!arduino_path.isEmpty())
            dom_arduino.firstChild().setNodeValue(arduino_path);
    }
#elif defined(Q_OS_LINUX)
    dom_arduino.firstChild().setNodeValue("arduino-cli");
#endif

    f.resize(0);
    QTextStream out(&f);
    doc.save(out, QDomNode::EncodingFromDocument);
    f.close();
    this->arduino_debug = dom_arduino.text();

    /* See what is the Arduino Board */
    QDomElement dom_arduinoBoard = root.firstChildElement(DOM_ARDUINOBOARD);
    if(dom_arduinoBoard.isNull()){
        qDebug()<<">>>>dom_arduinoBoard.isNull()";
        dom_arduinoBoard = doc.createElement(DOM_ARDUINOBOARD);
        root.appendChild(dom_arduinoBoard);
    }
//    if(dom_arduinoBoard.firstChildElement().isNull()){
//        qDebug()<<">>>>dom_arduinoBoard.firstChildElement().isNull()";
//        dom_arduinoBoard.appendChild(doc.createTextNode(""));
//        dom_arduinoBoard.firstChild().setNodeValue(STR_L_ARDUINOBOARD.at(0));
//    }
    QString arduino_board_tmp = dom_arduinoBoard.text();
    qDebug()<<">>>>arduino_board_tmp"<<arduino_board_tmp;
    SCPP_ASSERT_THROW( STR_L_ARDUINOBOARD.contains(arduino_board_tmp) );
    if(indexOf(STR_L_ARDUINOBOARD, arduino_board_tmp)==0){
        this->arduino_board = "uno";
    }
    else if(indexOf(STR_L_ARDUINOBOARD, arduino_board_tmp)==1){
        this->arduino_board = "mega";
    }
    else {
        this->arduino_board = "nano";
    }

}

void ProfileReader::reArduinoPath()
{
    ProfileReader::checkProfile(true);
}

