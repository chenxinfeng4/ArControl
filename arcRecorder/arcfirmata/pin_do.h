#ifndef PIN_DO_H
#define PIN_DO_H

#include <QWidget>

class QLabel;
class QComboBox;
class QSlider;
class QPushButton;

namespace PIN_DO_PRIVATE {
    const QList<uint> PIN_PWM = QList<uint>()<<3<<5<<6<<9<<10<<11;
}
class PIN_DO : public QWidget
{
    Q_OBJECT
public:
    explicit PIN_DO(uint do_pin, QWidget *parent = 0);
    uint getPin(){return _pin;}
private:
    uint _pin; //[2 3 4 5 6 7 8 9 10 11 12 13]
    uint _mode;//[0=DI, 1=DO, 2=PWM]
    QLabel * _label;
    QLabel * ui_divalue;
    QLabel * ui_pwmValue;
    QPushButton * ui_dobutton;
    QComboBox * _combobox;
    QSlider * _pwmslider;
    bool hasPwm();
signals:
    void setDoValue(uint pin, bool value);
    void setPWMValue(uint pin, int value);
    void pinModeChange(uint pin, int mode);
public slots:
    void setValue(bool); //DO
    void clear();
};
#endif // PIN_DO_H
