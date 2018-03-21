#include "onlinetable.h"

#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QTime>
#include <QDebug>
#include <QCheckBox>
using namespace ONLINETABEL_PARA;
using namespace ONLINETABEL_PRIVATE;
const QColor * calColor(const qint32 &);
OnlineTable::OnlineTable(QObject *parent,
                         const QHash<QString, StreamItem> * mdatahash,
                         QTableWidget * mtableWidget,
                         QLineEdit * le_name,
                         QPushButton * btn_add,
                         QPushButton * btn_sub,
                         QPushButton * btn_up,
                         QPushButton * btn_down)
    : QObject(parent), datahash(mdatahash),tableWidget(mtableWidget),
      LE_name(le_name), BTN_add(btn_add), BTN_rm(btn_sub),
      BTN_up(btn_up), BTN_down(btn_down), myTime(new QTime), isStarted(false)
{


    /* table 样式 */
    tableWidget->setRowCount(0);
    tableWidget->setColumnCount(4);
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tableWidget->horizontalHeader()->resizeSection(0,30);
    tableWidget->horizontalHeader()->resizeSection(1,70);
    tableWidget->horizontalHeader()->resizeSection(2,90);
    tableWidget->horizontalHeader()->resizeSection(3,50);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->horizontalHeader()->setVisible(true);
    tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked);
    tableWidget->setEnabled(true);
    QStringList str_headers;
    str_headers << tr("Vis.") << tr("Key") << tr("Comment") << tr("Count");
    tableWidget->setHorizontalHeaderLabels(str_headers);

#ifndef GLOBALLY_INIT_ON
   /* 初始化一些数据 */
    this->namelist << "C1S1" << "C1S2" << "C2S1"<<"C3S1" << "C3S2" ;
    foreach(const QString str, this->namelist){
        this->on_BTN_add_click(true, str);
    }
#endif

    /* 连接信号与槽 */
    connect(this->LE_name, SIGNAL(returnPressed()),this, SLOT(on_BTN_add_click()));
    connect(this->BTN_add, SIGNAL(clicked(bool)), this, SLOT(on_BTN_add_click()));
    connect(this->BTN_rm, SIGNAL(clicked(bool)), this, SLOT(on_BTN_rm_click()));
    connect(this->BTN_down, SIGNAL(clicked(bool)), this, SLOT(on_BTN_down_click()));
    connect(this->BTN_up,   SIGNAL(clicked(bool)), this, SLOT(on_BTN_up_click()));
    connect(tableWidget,  SIGNAL(itemClicked(QTableWidgetItem*)),
            this, SLOT(on_tableWidget_click(QTableWidgetItem *)));
}

void OnlineTable::initPush2Chart()
{
    /* 将初始数据push 到 chart */
    emit rm_line_all();
    for(int i=0; i < tableWidget->rowCount(); ++i){
        const QString name = tableWidget->item(i,IND_NAME)->text();
        bool checked = tableWidget->item(i,IND_VIS)->checkState() == Qt::Checked;
        emit add_line(name, checked);
    }
}

void OnlineTable::timerEvent(QTimerEvent * event)
{
    Q_UNUSED(event);

    int c = tableWidget->rowCount();
    for (int i = 0; i < c; ++i){
        // i = row idx
        const QString  str = tableWidget->item(i,IND_NAME)->text();
        QTableWidgetItem * itemCount = tableWidget->item(i,IND_COUNT);
        qint32 count = (*datahash)[str].count;
        itemCount->setText(QString::number(count));
        if(tableWidget->item(i, IND_ISCOL)->checkState()== Qt::Checked) {
            qint32 hot = (*datahash)[str].hot;
            const QColor * color = calColor(hot);
            itemCount->setBackgroundColor(*color);
        }
        else {
            itemCount->setBackgroundColor(COLORLIST[COLORLIST_LEN-1]);
        }
    }
}
void OnlineTable::when_start()
{
    this->myTime->restart();
    /* 清除 */

    /* 建立标志位 */
    this->isStarted = true;
    this->timerEventId = this->startTimer(EVERYTIMER);
    this->tableWidget->selectColumn(1);
    Q_ASSERT(this->timerEventId != 0);
}
void OnlineTable::when_stop()
{
    this->killTimer(this->timerEventId);
    this->isStarted = false;
    int c = tableWidget->rowCount();

    /* 恢复颜色 */
    for (int i = 0; i < c; ++i){
        const QColor * color = calColor(0);
        tableWidget->item(i,IND_COUNT)->setBackgroundColor(* color);
    }
}

void OnlineTable::on_BTN_add_click(bool vis, QString name,
                                   QString comm, int count, int isCol)
{
    /* 改变表单的格式 */
    if(name.isEmpty()){
        name = this->LE_name->text();
    }
    int sel = tableWidget->currentRow() + 1;
    qDebug()<<"SEL"<<sel;
    tableWidget->insertRow(sel);
    QTableWidgetItem * item;
    item = new QTableWidgetItem(name);
    item->setFlags(item->flags() & (~Qt::ItemIsEditable));
    tableWidget->setItem(sel,IND_NAME, item);
    item = new QTableWidgetItem();
    item->setFlags(item->flags() & (~Qt::ItemIsEditable) );
    item->setCheckState(vis ? Qt::Checked : Qt::Unchecked);
    tableWidget->setItem(sel,IND_VIS, item);
    item = new QTableWidgetItem(comm);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    tableWidget->setItem(sel,IND_COMM, item);
    item = new QTableWidgetItem(QString::number(count));
    item->setTextAlignment(Qt::AlignRight);
    item->setFlags(item->flags() & (~Qt::ItemIsEditable));
    item->setCheckState(isCol ? Qt::Checked : Qt::Unchecked);
    tableWidget->setItem(sel,IND_COUNT, item);
    tableWidget->selectRow(sel);

    /* push到chart */
    qDebug()<<"EMIT add"<<name;
    emit add_line(name, vis);
}

void OnlineTable::on_BTN_rm_click()
{
    /* 改变表单的格式 */
    int c = tableWidget->rowCount();
    if(c==0)
        return;
    int s = tableWidget->currentRow();
    const QString name = tableWidget->item(s, IND_NAME)->text();
    tableWidget->removeRow(s);
    tableWidget->selectRow(qMin(s , c-1));

    /* push到chart */
    qDebug()<<"EMIT delete"<<s;
    emit rm_line(name);
}

void OnlineTable::on_tableWidget_click(QTableWidgetItem * item)
{
    int col = tableWidget->column(item);
    if (col!=IND_VIS)
        return;

    /* check-item change => push到chart */
    int row = tableWidget->row(item);
    qDebug()<<"EMIT"<<row;
    const QString name = tableWidget->item(row, IND_NAME)->text();
    bool visible = item->checkState() == Qt::Checked;
    emit set_line_visible(name, visible);
}

void OnlineTable::clean_table()
{
    emit rm_line_all();
    int c= tableWidget->rowCount();
    for(int i=0; i<c; ++i)
        tableWidget->removeRow(0);
}

void OnlineTable::commit_settings()
{
    /* emit takefrom_table */
    int c= tableWidget->rowCount();
    for(int i=0; i<c; ++i){
        bool visible = tableWidget->item(i,IND_VIS)->checkState() == Qt::Checked;
        const QString name = tableWidget->item(i, IND_NAME)->text();
        const QString comm = tableWidget->item(i, IND_COMM)->text();
        bool isColor = tableWidget->item(i, IND_ISCOL)->checkState() == Qt::Checked;
        emit takefrom_this(visible, name, comm, isColor);
    }
}

void OnlineTable::giveto_this(bool isVisible, const QString &name, const QString &comm, bool isColor)
{
    this->on_BTN_add_click(isVisible, name, comm, 0, isColor);
}

void OnlineTable::on_BTN_up_click()
{
    int idx = tableWidget->currentRow();
    this->swapRow(idx-1, idx);
}

void OnlineTable::on_BTN_down_click()
{
    int idx = tableWidget->currentRow();
    this->swapRow(idx+1, idx);
}
void OnlineTable::swapRow(int row1, int row2)
{

    int c = tableWidget->rowCount();
    qDebug()<<"c="<<c;
    if(!(row1>=0 && row1<c && row2>=0 && row2<c)){
        qDebug()<<row1<<row2<<"return";
        return;
    }
    qDebug()<<row1<<row2<<"conitnue";
    int c2 = tableWidget->columnCount();
    for(int i=0; i<c2; ++i){
        QTableWidgetItem *a1, *a2;
        a1 = tableWidget->takeItem(row1, i);
        a2 = tableWidget->takeItem(row2, i);
        tableWidget->setItem(row1, i, a2);
        tableWidget->setItem(row2, i, a1);
    }
    tableWidget->selectRow(row1);
}

const QColor * calColor(const qint32 & hot)
{
    int ind;
    if (hot>=98)
        ind=0;
    else if (hot>=94)
        ind=1;
    else if (hot>84)
        ind=2;
    else if (hot>50)
        ind=3;
    else if (hot>20)
        ind=4;
    else
        ind=5;
    Q_ASSERT(ind<COLORLIST_LEN);
    return &(COLORLIST[ind]);
}
