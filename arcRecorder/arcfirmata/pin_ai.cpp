#include "pin_ai.h"
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QSpinBox>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

PIN_AI::PIN_AI(uint ai_pin, QWidget *parent)
    : QWidget(parent),
      _pin(ai_pin)
{
    Q_ASSERT(_pin<=5);

    /* layout */
    QHBoxLayout * layout = new QHBoxLayout();
    this->setLayout(layout);
    layout->setMargin(0);
    QString str = QString("A%1 IN%2").arg(_pin).arg(_pin+1);
    _label = new QLabel(str, this);
    _label->setMinimumWidth(70);
    _combobox = new QComboBox(this);
    QStringList str_l_mode;
    str_l_mode << tr("analog input  ")<<tr("digital input ")
               << tr("digital output");
    _combobox->addItems(str_l_mode);
    _combobox->setMinimumWidth(120);
    layout->addWidget(_label);
    layout->addWidget(_combobox);

    /* stack layout */
    QStackedLayout * stackedlayout = new QStackedLayout();
    layout->addLayout(stackedlayout);
    ui_aivalue = new QLabel(this);
    stackedlayout->addWidget(ui_aivalue);

    ui_divalue = new QLabel(this);
    stackedlayout->addWidget(ui_divalue);

    ui_dobutton = new QPushButton("LOW", this);
    ui_dobutton->setCheckable(true);
    stackedlayout->addWidget(ui_dobutton);

    /* signal and slot */
    connect(ui_dobutton, &QPushButton::toggled, [=](bool isChecked){
        emit setDoValue(_pin, isChecked);
        ui_dobutton->setText(isChecked ? "HIGH" : "LOW");
    });
    connect(_combobox, SIGNAL(activated(int)),
            stackedlayout, SLOT(setCurrentIndex(int)));
    void (QComboBox::*eventFun)(int) = QComboBox::activated;
    connect(_combobox, eventFun,
            [=](int mode){emit pinModeChange(_pin, mode);});
}

void PIN_AI::setValue(bool voltage)
{
    ui_divalue->setText(voltage ? "    HIGH" : "LOW");
}

void PIN_AI::setValue(uint voltage)
{
    ui_aivalue->setText(QString::asprintf("%4d", voltage));
}

void PIN_AI::clear()
{
    _combobox->setCurrentIndex(1);
    _combobox->activated(1);
    ui_aivalue->clear();
    ui_divalue->clear();
    ui_dobutton->setChecked(false);
    ui_dobutton->setText("LOW");
}
