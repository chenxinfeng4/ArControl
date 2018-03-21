#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QStringList>
#include <QDomDocument>
namespace Ui {
class MainWindow;
}

class PinAssignDialog;
class VarAssignDialog;
class DesignerSetFile;
class QCloseEvent;
class ProfileDialog;
class Session;
class StateContent;
class Setfile2INO;
class SizeDockWidget : public QWidget
{
   Q_OBJECT
public:
   explicit SizeDockWidget(QWidget * parent = 0, Qt::WindowFlags f = 0)
      : QWidget(parent,f) {
      m_szHint = geometry().size();
   }

   void updateSizeHint(QSize szHint2Set) {
      m_szHint = szHint2Set;
      updateGeometry();
   }

   virtual QSize sizeHint() const {
      return m_szHint;
   }
protected:
   QSize m_szHint;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void closeEvent(QCloseEvent *event);
    void initSession();
signals:
    void closing();
private:
    Ui::MainWindow *ui;
    PinAssignDialog * pinAssignDialog;
    VarAssignDialog * varAssignDialog;
    DesignerSetFile * designerSetFile;
    ProfileDialog   * profileDialog;
    QList<QAction *>  MN_taskList;    // connectable COM
    Session         * session;
    StateContent    * stateContent;

private slots:
    void tastList_changed(const QStringList &);
    void on_action_aboutme_triggered();
};

#endif // MAINWINDOW_H
