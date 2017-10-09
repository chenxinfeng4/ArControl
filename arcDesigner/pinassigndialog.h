#ifndef PINASSIGNDIALOG_H
#define PINASSIGNDIALOG_H

#include <QDialog>

namespace Ui {
class PinAssignDialog;
}

namespace PINASSIGNDIALOG_PARA{
    const int TABLE_ROW_COUNT(3),
              TABLE_IN_COL_COUT(6),
              TABLE_OUT_COL_COUT(8),
              IND_PIN(0), IND_ENB(1), IND_COMM(2);
}

class PinAssignDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PinAssignDialog(QWidget *parent = 0);
    ~PinAssignDialog();
    void initUi();
private:
    Ui::PinAssignDialog *ui;

signals: //准备写入到文件
    void takefrom_this(QList<bool> isIns, QList<int> Nums, QList<bool> isEnables, QList<QString> comms); //this -> file, num=1:x
    void report_this(QList<bool> isIns, QList<int> Nums, QList<bool> isEnables, QList<QString> comms); //(this & file) -> ?
public slots://准备读取自文件
    void commit_settings();    //this -> file
    void giveto_this(QList<bool> isIns, QList<int> Nums, QList<bool> isEnables, QList<QString> comms); //file -> this, num=1:x
};

#endif // PINASSIGNDIALOG_H
