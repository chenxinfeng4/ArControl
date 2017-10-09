#ifndef PIN_AI_H
#define PIN_AI_H

#include <QWidget>

class QLabel;
class QCombobox;
class QPushButton;
class QComboBox;

class PIN_AI : public QWidget
{
    Q_OBJECT
public:
    explicit PIN_AI(uint ai_pin, QWidget *parent = 0);
    uint getPin(){return _pin;}
private:
    uint _pin; // [0 1 2 3 4 5]
    QLabel * _label;
    QLabel * ui_aivalue;
    QLabel * ui_divalue;
    QComboBox * _combobox;
    QPushButton * ui_dobutton;

signals:
    void setDoValue(uint pin, bool value);
    void pinModeChange(uint pin, int mode);
public slots:
    void setValue(bool);
    void setValue(uint);
    void clear();
};




#endif // PIN_AI_H
