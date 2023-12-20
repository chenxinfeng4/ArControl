#include "pin_do.h"
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QPushButton>
#include <QSlider>
#include <QDebug>
#include <QLabel>
#include <QComboBox>
#include <QSlider>

PIN_DO::PIN_DO(uint do_pin, QWidget *parent)
    : QWidget(parent), _pin(do_pin)
{
    Q_ASSERT(_pin>=2 && _pin<=13);

    /* layout */
    QHBoxLayout * layout = new QHBoxLayout();
    this->setLayout(layout);
    layout->setMargin(0);

    QString str;
    if(_pin <=9)
        str = QString("D%1 OUT%2").arg(_pin).arg(_pin-1);
    else
        str = QString("D%1").arg(_pin);
    _label = new QLabel(str, this);
    _label->setMinimumWidth(70);
    _combobox = new QComboBox(this);
    QStringList str_l_mode;
    str_l_mode << tr("digital input ") << tr("digital output");
    if(hasPwm())
        str_l_mode << tr("digital pwm  ");
    _combobox->addItems(str_l_mode);
    _combobox->setMinimumWidth(120);
    layout->addWidget(_label);
    layout->addWidget(_combobox);

    /* stack layout */
    QStackedLayout * stackedlayout = new QStackedLayout();
    layout->addLayout(stackedlayout);
    ui_divalue = new QLabel(this); //digital input
    stackedlayout->addWidget(ui_divalue);

    ui_dobutton = new QPushButton("LOW", this); //digital output
    ui_dobutton->setCheckable(true);
    stackedlayout->addWidget(ui_dobutton);
    _pwmslider = new QSlider(Qt::Horizontal, this); //digital pwm
    _pwmslider->setMaximum(255);
    _pwmslider->setMinimum(0);
    stackedlayout->addWidget(_pwmslider);

    /* signal and slot */
    connect(ui_dobutton, &QPushButton::toggled, [=](bool isChecked){
        emit setDoValue(_pin, isChecked);
        ui_dobutton->setText(isChecked ? "HIGH" : "LOW");
    });
    connect(_combobox, SIGNAL(activated(int)),
            stackedlayout, SLOT(setCurrentIndex(int)));
    void (QComboBox::*eventFun)(int) = static_cast<void (QComboBox::*)(int)>(&QComboBox::activated);
    connect(_combobox, eventFun,
            [=](int mode){emit pinModeChange(_pin, mode);});
    connect(_pwmslider, &QSlider::valueChanged,
            [=](int value){emit setPWMValue(_pin, value);});
}
void PIN_DO::setValue(bool voltage)
{
    ui_divalue->setText(voltage ? "    HIGH" : "LOW");
}

void PIN_DO::clear()
{
    _combobox->setCurrentIndex(1);
    _combobox->activated(1);
    _pwmslider->setValue(0);
    ui_dobutton->setChecked(false);
    ui_dobutton->setText("LOW");
}

bool PIN_DO::hasPwm()
{
    return PIN_DO_PRIVATE::PIN_PWM.contains(this->_pin);
}
