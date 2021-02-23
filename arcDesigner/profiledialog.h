#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
#include <QMainWindow>
#include <QMouseEvent>


class QLabel;
class QDomDocument;
class QFile;

namespace Ui {
class ProfileDialog;
}

namespace PROFILEDIALOG_PARA {
    const QString ARDUINO_REG = "HKEY_CLASSES_ROOT\\Arduino file\\shell\\open\\command";
    const QString DOM_ROOT("PROFILE");
    const QStringList STR_L_INPUT = QStringList() << "LOW" << "HIGH",
                      STR_L_LEVEL = QStringList() << "1" << "2" << "3";

}


//namespace PROFILEDIALOG_PRIVATE {

class FramelessWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FramelessWindow(QWidget *parent = 0);

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
private:
    int m_nMouseClick_X_Coordinate;
    int m_nMouseClick_Y_Coordinate;
    bool m_beMove;
};

//}
namespace PROFILEDIALOG_PARA {
    const QStringList PINMAP_IMAGE = QStringList()<<"Readme/Arduino_UNO_Pin.png"<<"Readme/Arduino_Mega_Pin.png"<<"Readme/Arduino_Nano_Pin.png";
}
class ProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProfileDialog(QWidget *parent = 0);
    ~ProfileDialog();

public slots:
    void pickArduinoIDE();

private slots:
    void on_BTN_viewpin_clicked();

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_BTN_chooseIDE_clicked();

private:
    Ui::ProfileDialog *ui;
    QMainWindow * imageWindow;
    QFile *file;
    QDomDocument * doc;
    void initUI();
    void file_to_this();
    void this_to_file();
    void write_to_file(bool useGUI); //write default or GUI to file
    void check_file(bool forceCreate=false); //check, if error nor not exist => create

private:
    // status data
    QString arduinoIDE;
    QString arduinoBoard;
    QString lang;
    bool echoTask;
    int  recordLevel; //[1 | 2 | 3]
    QString startModel;
    // status data <==> GUI
    void data_to_gui();
    void gui_to_data();
};

#endif // PROFILEDIALOG_H
