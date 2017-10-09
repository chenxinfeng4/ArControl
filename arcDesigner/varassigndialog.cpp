#include "varassigndialog.h"
#include "ui_varassigndialog.h"

#include <QRegExpValidator>
#include <QDebug>
#include <QVariant>
#include <QPair>

typedef QPair<QString, double> MyType;
Q_DECLARE_METATYPE(MyType)

const QString rx_pattem = "\\s*Var_\\w*\\s*";

// [1] 基本的GUI操作
VarAssignDialog::VarAssignDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VarAssignDialog)
{
    ui->setupUi(this);

    /* regexp for lineEdit ui->lineEdit_name */
    QRegExp rx(rx_pattem); // Var_name
    QValidator *validator = new QRegExpValidator(rx, this);
    ui->lineEdit_name->setValidator(validator);

    /* windows flag */
    this->setWindowFlags((this->windowFlags()
                          &(~Qt::WindowContextHelpButtonHint))
                         | Qt::WindowStaysOnTopHint);

    /* connect signal */
    connect(this, SIGNAL(finished(int)), this, SLOT(commit_settings()));

    /* init ui */
    this->initUi();
}

VarAssignDialog::~VarAssignDialog()
{
    delete ui;
}

void VarAssignDialog::swapRow(int row1, int row2)
{
    int c = ui->listWidget->count();
    if(!(row1>=0 && row1<c && row2>=0 && row2<c))
        return;
    QString str1 = ui->listWidget->item(row1)->text();
    QString str2 = ui->listWidget->item(row2)->text();
    QVariant d1  = ui->listWidget->item(row1)->data(Qt::UserRole);
    QVariant d2  = ui->listWidget->item(row2)->data(Qt::UserRole);
    ui->listWidget->item(row1)->setText(str2);
    ui->listWidget->item(row2)->setText(str1);
    ui->listWidget->item(row1)->setData(Qt::UserRole, d2);
    ui->listWidget->item(row2)->setData(Qt::UserRole, d1);
    ui->listWidget->setCurrentRow(row1);
}

void VarAssignDialog::initUi()
{
    ui->listWidget->clear();

    while(0){
        tr("doVar");
        tr("doPin");
        tr("whenVar");
        tr("whenPin");
        tr("whenTime");
        tr("whenCount");
    }
    qDebug()<<"+++2"<<tr(qPrintable(QString("whenTime")));
}

void VarAssignDialog::on_BTN_up_clicked()
{
    int idx = ui->listWidget->currentRow();
    this->swapRow(idx-1, idx);
}

void VarAssignDialog::on_BTN_down_clicked()
{
    int idx = ui->listWidget->currentRow();
    this->swapRow(idx+1, idx);
}

void VarAssignDialog::on_BTN_add_clicked(QString varName, double varValue)
{
    if(varName.isEmpty() && varValue==0){
        /* none input params ==> use gui data*/
        varName = ui->lineEdit_name->text();
        varValue = ui->spinBox_value->value();
    }
    QRegExp rx(rx_pattem);
    if(!rx.exactMatch(varName))
        return;
    varName = varName.trimmed();
    int varValue_int = varValue;
    QString argPattern = (varValue==0 ? "%1" : "%1 = %2");
    QString strItem;
    if(varValue == varValue_int){
        strItem = argPattern.arg(varName).arg(varValue_int);
    }
    else{
        char str[20];
        sprintf(str, "%.3f", varValue); //保留3位小数点
        strItem = argPattern.arg(varName).arg(str);
    }
    MyType itemData(varName, varValue);
    int row = ui->listWidget->currentRow() + 1;
    ui->listWidget->insertItem(row, strItem);
    ui->listWidget->setCurrentRow(row);
    ui->listWidget->item(row)->setData(Qt::UserRole, QVariant::fromValue(itemData));
}

void VarAssignDialog::on_BTN_del_clicked()
{
    int c = ui->listWidget->count();
    if(c==0)
        return;
    int row = ui->listWidget->currentRow();
    ui->listWidget->takeItem(row);
}

void VarAssignDialog::on_listWidget_clicked()
{
    QVariant itemData = ui->listWidget->currentItem()->data(Qt::UserRole);
    if(itemData.isNull())
        return;
    MyType itemData_p = itemData.value<MyType>();
    QString strName = itemData_p.first;
    double strValue = itemData_p.second;
    ui->lineEdit_name->setText(strName);
    ui->spinBox_value->setValue(strValue);
}
// [1] end


// [2] 文件 保存和载入
void VarAssignDialog::clean_this()
{
    ui->listWidget->clear();
}

void VarAssignDialog::commit_settings()
{
    int c = ui->listWidget->count();
    QList<QString> varNames;
    QList<double> varValues;
    for(int i=0; i<c; ++i){
        QVariant itemData = ui->listWidget->item(i)->data(Qt::UserRole);
        if(itemData.isNull())
            continue;
        MyType itemData_p = itemData.value<MyType>();
        varNames<<itemData_p.first; varValues<<itemData_p.second;
    }
    emit takefrom_this(varNames, varValues);
    emit report_this(varNames, varValues);
}

void VarAssignDialog::giveto_this(QList<QString> varNames, QList<double> varValues)
{
    emit report_this(varNames, varValues); //emit original signal
    ui->listWidget->clear(); //clear all
    int c = varNames.count();
    for(int i=0; i<c; ++i)
        this->on_BTN_add_clicked(varNames[i], varValues[i]);
}
// [2] end

void VarAssignDialog::on_BTN_ok_clicked()
{
    /* none input params ==> use gui data*/
    QString varName = ui->lineEdit_name->text();
    double varValue = ui->spinBox_value->value();

    QRegExp rx(rx_pattem);
    if(!rx.exactMatch(varName))
        return;

    /* add then delete */
    int row = ui->listWidget->currentRow();
    this->on_BTN_add_clicked(varName, varValue);
    if(ui->listWidget->count() == 1)
        return;
    ui->listWidget->takeItem(row);
}
