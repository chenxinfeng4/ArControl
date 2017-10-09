#ifndef ARCFIRMATA_H
#define ARCFIRMATA_H

#include <QMainWindow>

#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include "pin_ai.h"
#include "pin_do.h"
#include "./src/qfirmata.h"

class ArcFirmata : public QMainWindow
{
    Q_OBJECT

public:
    ArcFirmata(QWidget *parent = 0);
    ~ArcFirmata();
    void closeEvent(QCloseEvent *event);

private slots:
    void onDigitalModeChange(uint pin, int mode);
    void onAnalogModeChange(uint pin, int mode);
    void onDigitalPwmChange(uint pin, int value);
    void onDigitalWriteChange(uint pin, bool value);
    void onAnalogWriteDChange(uint pin, bool value);

    void onFirmwareVersionReceived(const int majorVersion, const int minorVersion);
    void onFirmwareNameReceived(QString firmwareName);
    void onInitialized(const int majorVersion, const int minorVersion, QString firmwareName);
    void onDigitalPinChanged(int);
    void onAnalogPinChanged(int);

    void onConnectButtonClicked();
    void onDisconnectButtonClicked();

    void onInitConnection();
    void onInitTask();
private:
    void initUI();
    void setPinsEnable(bool isEnabled);
    QFirmata *_firmata;
    QComboBox * _portComboBox;
    QPushButton *_connectButton;
    QPushButton *_disconnectButton;
    QVBoxLayout * _pinsLayout;
    QVector<PIN_AI*> _analogPinVec;
    QVector<PIN_DO*> _digitalPinVec;
};

#endif // ARCFIRMATA_TEST_H
