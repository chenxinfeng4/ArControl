#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#define QT_NO_DEBUG
#include <QMainWindow>
#include "main.h"

class OnlineManagerBar;
class SerialCheck;
class QTime;
class SerialFlowControl;
class TasktoArduino;
class OnlineChart;
class OnlineTable;
class OnlineLED;
class OnlineSetFile;
class ProfileReader;
class ArcFirmata;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QTime * myTime;
    int timerEventId=0;
    OnlineManagerBar * onlineManagerBar;
    SerialCheck * serialCheck;
    SerialFlowControl * serialFlowControl;
    TasktoArduino  * tasktoArduino;
    OnlineChart * onlineChart;
    OnlineTable * onlineTable;
    OnlineLED   * onlineLED;
    OnlineSetFile * onlineSetFile;
    ProfileReader * profileReader;
    ArcFirmata * arcFirmata;
    void closeEvent(QCloseEvent *event);
private slots:

    void on_actionStream_Window_triggered(bool checked);

    void on_dockWidget_visibilityChanged(bool visible);

    void on_actionFirmata_Test_triggered();

    void on_action_aboutme_triggered();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
