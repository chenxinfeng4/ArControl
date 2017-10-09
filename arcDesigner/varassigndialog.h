#ifndef VARASSIGNDIALOG_H
#define VARASSIGNDIALOG_H

#include <QDialog>

namespace Ui {
class VarAssignDialog;
}

class VarAssignDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VarAssignDialog(QWidget *parent = 0);
    ~VarAssignDialog();
    void swapRow(int row1, int row2);
    void initUi();
private slots:
    void on_BTN_up_clicked();
    void on_BTN_down_clicked();
    void on_BTN_add_clicked(QString varName="", double varValue=0);
    void on_BTN_del_clicked();
    void on_listWidget_clicked();
    void on_BTN_ok_clicked();

private:
    Ui::VarAssignDialog *ui;

signals: //准备写入到文件
    void takefrom_this(QList<QString> varNames, QList<double> varValues); //this -> file
    void report_this(QList<QString> varNames, QList<double> varValues);
public slots://准备读取自文件
    void clean_this(); //file -> this
    void commit_settings();    //this -> file
    void giveto_this(QList<QString> varNames, QList<double> varValues); //file -> this
};

#endif // VARASSIGNDIALOG_H
