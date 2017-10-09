#include "pinassigndialog.h"
#include "ui_pinassigndialog.h"
#include "globalparas.h"

#include <QStringList>

using namespace PINASSIGNDIALOG_PARA;
using namespace GLOBAL_PARA;
// [1] 基本的 GUI 逻辑
PinAssignDialog::PinAssignDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PinAssignDialog)
{
    ui->setupUi(this);
    this->setWindowFlags((this->windowFlags()
                         & (~Qt::WindowContextHelpButtonHint))
                         | Qt::WindowStaysOnTopHint);
    initUi();

    /* siganl and slot */
    connect(this, SIGNAL(finished(int)), this, SLOT(commit_settings()));
}

PinAssignDialog::~PinAssignDialog()
{
    delete ui;
}

void PinAssignDialog::initUi()
{
    for(int icase=0; icase<2; ++icase){
        /* prepare paras */
        QTableWidget * tableWidget = (icase? ui->tableInput : ui->tableOutput);
        const int colCount(3), rowCount = (icase? IN_CHAN_COUNT : OUT_CHAN_COUNT);
        QStringList str_h_headers, str_v_headers;
        str_h_headers << "Pin" << "Enable" << "Comment";
        QString label1 = (icase? "IN%1" : "OUT%1"); //"IN1 - IN6", "OUT1 - OUT8"
        QString label2 = (icase? "A%1"  : "D%1");   //"A0  - A5" , "D2   - D9"
        const char size_h_headers[colCount] = {50, 50, 100}; //colum width

        /* make header */
        for(int i=0; i<rowCount; ++i)
            str_v_headers << label1.arg(i+1);
        for(int i=0; i<colCount; ++i)
            tableWidget->horizontalHeader()->resizeSection(i, size_h_headers[i]);

        tableWidget->setRowCount(rowCount);
        tableWidget->setColumnCount(colCount);
        tableWidget->setHorizontalHeaderLabels(str_h_headers);
        tableWidget->setVerticalHeaderLabels(str_v_headers);

        /* make items  */
        auto defualtFlag = QTableWidgetItem().flags();
        for(int i=0; i<rowCount; ++i) {
            QTableWidgetItem * item_realpin, * item_enable, * item_comm;
            item_realpin = new QTableWidgetItem(label2.arg(icase? i:i+2));
            item_realpin->setTextAlignment(Qt::AlignCenter);
            item_realpin->setFlags(defualtFlag & (~Qt::ItemIsEnabled));
            tableWidget->setItem(i,IND_PIN,item_realpin);/* "A0" */

            item_enable  = new QTableWidgetItem();
            item_enable->setFlags(defualtFlag & (~Qt::ItemIsEditable));
            item_enable->setCheckState(true? Qt::Checked : Qt::Unchecked);
            tableWidget->setItem(i,IND_ENB,item_enable);/* "A0" */

            item_comm    = new QTableWidgetItem(QString("#%1").arg(i+1));
            tableWidget->setItem(i,IND_COMM,item_comm);
        }
    }
}
// [1] end


// [2] 文件 <==> this
void PinAssignDialog::commit_settings()
{
    QList<bool> isIns; QList<int> Nums; QList<bool> isEnables; QList<QString> comms;
    for(int icase=0; icase<2; ++icase){
        QTableWidget * tableWidget = (icase? ui->tableInput : ui->tableOutput);
        const int colCount(3), rowCount = (icase? TABLE_IN_COL_COUT : TABLE_OUT_COL_COUT);
        Q_ASSERT_X(tableWidget->columnCount() == colCount, "none match", "none match table");
        Q_ASSERT_X(tableWidget->rowCount() == rowCount, "none match", "none match table");

        /* scan items */
        for(int i=0; i<rowCount; ++i) {
            QString str_comm = tableWidget->item(i, IND_COMM)->text();
            bool is_enable   = tableWidget->item(i, IND_ENB)->checkState() == Qt::Checked;
            isIns<<(bool)icase; Nums<< i+1; isEnables<<is_enable; comms<<str_comm;
        }
    }
    emit takefrom_this(isIns, Nums, isEnables, comms);
    emit report_this(isIns, Nums, isEnables, comms);
}

void PinAssignDialog::giveto_this(QList<bool> isIns, QList<int> Nums, QList<bool> isEnables, QList<QString> comms)
{
    emit report_this(isIns, Nums, isEnables, comms); //emit original signal
    this->initUi(); //clear all
    int c = isIns.count();
    for(int i=0; i<c; ++i){
        int icase = isIns[i];
        QTableWidget * tableWidget = (icase? ui->tableInput : ui->tableOutput);
        const int colCount(3), rowCount = (icase? TABLE_IN_COL_COUT : TABLE_OUT_COL_COUT);
        Q_ASSERT_X(tableWidget->columnCount() == colCount, "none match", "none match table");
        Q_ASSERT_X(tableWidget->rowCount() == rowCount, "none match", "none match table");
        int Num = Nums[i];
        Q_ASSERT_X(Num <= rowCount && Num >=1, "none match", "Num exceed!");
        tableWidget->item(Num-1, IND_COMM)->setText(comms[i]);
        tableWidget->item(Num-1, IND_ENB)->setCheckState(isEnables[i] ? Qt::Checked : Qt::Unchecked);
    }
}

// [2] end


