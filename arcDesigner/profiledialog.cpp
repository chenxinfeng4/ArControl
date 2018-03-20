#include "profiledialog.h"
#include "ui_profiledialog.h"
#include "main.h"
#include "globalparas.h"
#include <QImage>
#include <QImageReader>
#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QFileDialog>
#include <QSettings>
#include <QDomDocument>
#include <QTextStream>

using namespace PROFILEDIALOG_PARA;
using namespace GLOBAL_PARA;
QString searchArduinoIDEinWin();

//using namespace PROFILEDIALOG_PRIVATE;

// [1] basic
ProfileDialog::ProfileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProfileDialog),
    imageWindow(new FramelessWindow(parent)),
    file(new QFile(PROFILE_FILE)),
    doc(new QDomDocument("doc"))
{
    ui->setupUi(this);
    this->setWindowFlags((this->windowFlags()
                         & (~Qt::WindowContextHelpButtonHint))
                         | Qt::WindowStaysOnTopHint);

    this->initUI();
}

ProfileDialog::~ProfileDialog()
{
    delete ui;
}

void ProfileDialog::pickArduinoIDE()
{
    /* 判断平台 */
    QString debugFile;
#ifdef Q_OS_WIN
    debugFile = "arduino_debug.exe";
#endif
#ifdef Q_OS_LINUX
    debugFile = "arduino";
#endif
#ifdef Q_OS_MAC
    debugFile = "arduino.app";
#endif
     QString arduino_path = QFileDialog::getOpenFileName(0, tr("Choose Arduino IDE"),
                                                   "", debugFile); //may rais an-warning on debug-output, don't warry
     if(!arduino_path.isEmpty())
         this->arduinoIDE = arduino_path;
}

void ProfileDialog::on_BTN_viewpin_clicked()
{
    imageWindow->show();
}

void ProfileDialog::initUI()
{
    /* image */
    QImageReader reader(PINMAP_IMAGE);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()){
        qDebug("No such image!");
        return;
    }
    QLabel *imageLabel = new QLabel(imageWindow);
    imageWindow->setCentralWidget(imageLabel);

    imageLabel->setPixmap(QPixmap::fromImage(newImage));
    imageWindow->setWindowFlags(Qt::Popup);

    /* read profile.xml*/
    try{
        this->file_to_this();
    }
    catch(...){
        this->check_file(true);
        this->file_to_this();
    }
}

void ProfileDialog::on_buttonBox_accepted()
{
    /* commit setts */
    this->this_to_file();
    this->close();
}

void ProfileDialog::on_buttonBox_rejected()
{
    /* cannel setts */
    this->hide();
    this->file_to_this();
    this->close();
}

void ProfileDialog::on_BTN_chooseIDE_clicked()
{
    QString defualtIDE = searchArduinoIDEinWin();
    QString path = "";
    if(QFile(this->arduinoIDE).exists())
        path = QFileInfo(this->arduinoIDE).absolutePath();
    else if(QFile(defualtIDE).exists())
        path = QFileInfo(defualtIDE).absolutePath();
    QString debugFile;
#ifdef Q_OS_WIN
    debugFile = "arduino_debug.exe";
#endif
#ifdef Q_OS_LINUX
    debugFile = "arduino";
#endif
#ifdef Q_OS_MAC
    debugFile = "arduino.app";
#endif
    QString arduino_pathF = QFileDialog::getOpenFileName(0, tr("Choose Arduino IDE"),
                                                path, debugFile); //may rais an-warning on debug-output, don't warry
    if(!arduino_pathF.isEmpty()){
        this->arduinoIDE = arduino_pathF;
        this->data_to_gui();
        qDebug()<<"this->arduinoIDE"<<this->arduinoIDE;
    }
}

QString searchArduinoIDEinWin()
{
#ifndef Q_OS_WIN
    return QString("");
#endif
    QSettings reg(ARDUINO_REG, QSettings::NativeFormat);
    QString pathf_dirty = reg.value(".").toString(); // "\"D:\\L_Arduino_1.16.11\\arduino.exe\"
    QString pathf_clean = pathf_dirty.remove('\"').remove("%1").trimmed().replace('\\', '/')
                  .replace("arduino.exe", "arduino_debug.exe", Qt::CaseInsensitive);
    qDebug()<<"reg"<<pathf_clean;
    return pathf_clean;
}
// [1] end

// [2] file read write
void ProfileDialog::file_to_this()
{
    /* read xml doc*/
    if(file->isOpen())
        file->close();
    SCPP_ASSERT_THROW(file->open(QFile::ReadOnly | QFile::Text));
    doc->setContent(file);
    file->close();

    /* parsing DOM file */
    // assert root
    QDomElement root = doc->documentElement();
    SCPP_ASSERT_THROW(!root.isNull(), "File-Element error!");
    SCPP_ASSERT_THROW(root.nodeName() == DOM_ROOT, "File-Element error!");

    auto readItem =  [=](QString itemName, const QString defValue = "", const QStringList listValue = QStringList())
            -> QString{
        QDomElement dom_item = root.firstChildElement(itemName);
        SCPP_ASSERT_THROW(!dom_item.isNull(), "File-Element error!");
        QString value = dom_item.text().isEmpty() ? defValue : dom_item.text();
        SCPP_ASSERT_THROW(listValue.isEmpty() || listValue.contains(value),
                          tr("File-Element %1 context %2 error!").arg(itemName).arg(value));
        return value;
    };

    // arduino_debug.exe
    this->arduinoIDE = readItem(DOM_ARDUINO);
    // language
    this->lang = readItem(DOM_LANG, STR_L_LANG.at(0), STR_L_LANG);
    //record level
    this->recordLevel = readItem(DOM_RECORDLEVEL, "3", STR_L_LEVEL).toInt();
    //task name echo
    this->echoTask = readItem(DOM_ECHOTASK, "true", STR_L_BOOL) == "true";
    //start model
    this->startModel = readItem(DOM_START, "START_SOFT", STR_L_START);

    /* data to gui */
    this->data_to_gui();
}

void ProfileDialog::this_to_file()
{
    this->write_to_file(true);
}
void ProfileDialog::write_to_file(bool useGUI)
{
    /* gui to data */
    this->gui_to_data();

    /* create xml doc*/
    doc->clear();
    QDomProcessingInstruction  instruction =  doc->createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    doc->appendChild(instruction);
    QDomElement root = doc->createElement(DOM_ROOT); // "/PROFILE"
    doc->appendChild(root);

    auto addItem = [&](QString itemName, QString v1, QString v0)->void{
        QDomElement dom_item = doc->createElement(itemName);
        dom_item.appendChild(doc->createTextNode(useGUI? v1 : v0));
        root.appendChild(dom_item);
    };

    //arduino_debug.exe
    addItem(DOM_ARDUINO, this->arduinoIDE, searchArduinoIDEinWin());
    //language
    addItem(DOM_LANG, this->lang, "English");
    //record level
    addItem(DOM_RECORDLEVEL, QString::number(this->recordLevel), "3");
    //task name echo
    addItem(DOM_ECHOTASK, (this->echoTask ? "true" :"false"), "true");
    //start model
    addItem(DOM_START, this->startModel, "START_SOFT");

    /* write to file */
    if(file->isOpen())
        file->close();
    SCPP_ASSERT_THROW(file->open(QFile::WriteOnly | QFile::Text), "Cannot open file!");
    QTextStream out(file);
    doc->save(out, QDomNode::EncodingFromDocument);
    file->close();
}

void ProfileDialog::check_file(bool forceCreate)
{
    /* check => { file no exist -> create || forceCreate -> create}  */
    if(!this->file->exists() || forceCreate)
        this->write_to_file(false); //use default paras
}

void ProfileDialog::data_to_gui()
{
    ui->lineEdit_pathIDE->setText(this->arduinoIDE);
    ui->comboBox_lang->setCurrentIndex(indexOf(STR_L_LANG, this->lang));
    ui->comboBox_level->setCurrentIndex(this->recordLevel - 1);
    ui->CKB_echotask->setChecked(this->echoTask);
    ui->comboBox_start->setCurrentIndex(indexOf(STR_L_START, this->startModel));
}

void ProfileDialog::gui_to_data()
{
    this->arduinoIDE = ui->lineEdit_pathIDE->text();
    qDebug()<<"comboBox_lang"<<ui->comboBox_lang->currentIndex();
    this->lang = STR_L_LANG.at(ui->comboBox_lang->currentIndex());
    this->recordLevel = ui->comboBox_level->currentIndex() + 1;
    this->echoTask = ui->CKB_echotask->isChecked();
    this->startModel = STR_L_START.at(ui->comboBox_start->currentIndex());
}

// [2] end


// [3] FramlessWindow
FramelessWindow::FramelessWindow(QWidget *parent):
    QMainWindow(parent)
{
    Qt::WindowFlags flags = Qt::Dialog;
    flags = flags | Qt::Popup| Qt::FramelessWindowHint;
    this->setWindowFlags (flags);
}

void FramelessWindow::mousePressEvent(QMouseEvent *event)
{
    bool isMouseLeft = event->button() == Qt::LeftButton;
    bool isMouseInside = true;
    if (isMouseLeft) {
        m_nMouseClick_X_Coordinate = event->x();
        m_nMouseClick_Y_Coordinate = event->y();

        QRect pos = this->geometry();
        isMouseInside =(m_nMouseClick_Y_Coordinate > 0
                       && m_nMouseClick_Y_Coordinate < pos.height()
                       && m_nMouseClick_X_Coordinate > 0
                       && m_nMouseClick_X_Coordinate < pos.width());
        qDebug()<<"mouse" << m_nMouseClick_X_Coordinate<<m_nMouseClick_Y_Coordinate;
        qDebug()<<"window" <<pos.bottom()<<pos.top()<<pos.left()<<pos.right();
        qDebug()<<"isMouseInside"<<isMouseInside;
        if(!isMouseInside)
            this->close();
    }
    m_beMove = isMouseLeft && isMouseInside;
}

void FramelessWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_beMove)
        return;
    this->move(event->globalX()-m_nMouseClick_X_Coordinate,
               event->globalY()-m_nMouseClick_Y_Coordinate);
}
// [3] end


