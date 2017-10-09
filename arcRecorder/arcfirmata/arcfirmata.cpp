#include "arcfirmata.h"
#include "../profilereader.h"
#include "../vendor_modify/scpp_assert.h"
#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStatusBar>
#include <QProcess>
#include <QLabel>

#define QT_NO_DEBUG_OUTPUT
#define A0 14
ArcFirmata::ArcFirmata(QWidget *parent)
    :QMainWindow(parent),
    _firmata(new QFirmata(this))
{
    auto wtype = this->windowFlags() & ~(Qt::WindowMaximizeButtonHint);
    this->setWindowFlags(wtype);

    QWidget *widget = new QWidget(this);
    setCentralWidget(widget);
    setWindowTitle(tr("ArControl Firmata"));
    statusBar()->showMessage("");
    initUI();
}

ArcFirmata::~ArcFirmata()
{
    _firmata->close();
}

void ArcFirmata::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    qDebug()<<"ArcFirmata::closeEvent()";
    onDisconnectButtonClicked();
    statusBar()->clearMessage();
}

void ArcFirmata::onDigitalModeChange(uint pin, int mode)
{
    qDebug()<<"digital"<<pin<< "pinmode changed"<<mode;
    switch(mode){ //[0=DI, 1=DO, 2=PWM]
    case 0: //digital input
        _firmata->sendDigitalPinMode(pin, ARD_INPUT);
        onDigitalPinChanged(pin);
        break;
    case 1: //digital output
        _firmata->sendDigitalPinMode(pin, ARD_OUTPUT);
        break;
    case 2: //digital PWM
        _firmata->sendDigitalPinMode(pin, ARD_PWM);
        break;
    }
}

void ArcFirmata::onAnalogModeChange(uint pin, int mode)
{
    Q_ASSERT(pin<=5);
    int pin_ = pin + A0;
    qDebug()<<"Analog"<<pin<< "pinmode changed"<<mode;
    switch(mode){ //[0=AI 0=DI 1=DO]
    case 0: //analog input
        _firmata->sendDigitalPinMode(pin_, ARD_ANALOG);
        _firmata->sendAnalogPinReporting(pin, ARD_ON);
        onAnalogPinChanged(pin);
        break;
    case 1: //digital input
        _firmata->sendDigitalPinMode(pin_, ARD_INPUT);
        onDigitalPinChanged(pin_);
        break;
    case 2: //digital output
        _firmata->sendDigitalPinMode(pin_, ARD_OUTPUT);
        break;
    }
}

void ArcFirmata::onDigitalPwmChange(uint pin, int value)
{
    qDebug()<<"Digital pin"<<pin<<"PWM"<<value;
    _firmata->sendPWM(pin, value);
}

void ArcFirmata::onDigitalWriteChange(uint pin, bool value)
{
    qDebug()<<"Digital pin"<<pin<<"Write"<<value;
    _firmata->sendDigital(pin, value);
}

void ArcFirmata::onAnalogWriteDChange(uint pin, bool value)
{
    int pin_ = pin + A0;
    qDebug()<<"Analog pin"<<pin<<"Write"<<value;
    _firmata->sendDigital(pin_, value);
}

void ArcFirmata::onFirmwareVersionReceived(const int majorVersion, const int minorVersion)
{
    qDebug()<<"major"<<majorVersion<<"minor"<<minorVersion;
}

void ArcFirmata::onFirmwareNameReceived(QString firmwareName)
{
    qDebug()<<"firmware name"<<firmwareName;
}

void ArcFirmata::onInitialized(const int majorVersion, const int minorVersion, QString firmwareName)
{
    qDebug()<<"firmate initialized"<<majorVersion<<minorVersion<<firmwareName;
}

void ArcFirmata::onDigitalPinChanged(int pin) // board to this
{

    int value = _firmata->getDigital(pin);
    qDebug()<<"Digital pin"<<pin<<"value"<<value;
    if( pin>= A0){ //A0-A5
        int pin_ = pin-A0;
        if(pin_ < _analogPinVec.size())
            _analogPinVec[pin_]->setValue((bool)value);
    }
    else{ //D2-D13
        int pin_ = pin-2;
        if(pin_ < _digitalPinVec.size() && pin_ >= 0)
            _digitalPinVec[pin_]->setValue((bool)value);
    }
}

void ArcFirmata::onAnalogPinChanged(int pin) // board to this
{
    if(pin < _analogPinVec.size())
        _analogPinVec[pin]->setValue((uint)(_firmata->getAnalog(pin)));
}

void ArcFirmata::onConnectButtonClicked()
{
    QString portName = _portComboBox->currentText();
    if(_firmata->open(portName)){
        statusBar()->showMessage(tr("Connectting to %1!")
                                       .arg(portName), 2000);
        _connectButton->hide();
        _disconnectButton->show();
    }
    else{
        statusBar()->showMessage(tr("ERROR: %1 may be USING by other program!")
                                       .arg(portName));
    }
}

void ArcFirmata::onDisconnectButtonClicked()
{
    _firmata->close();
    setPinsEnable(false);
    _disconnectButton->hide();
    _connectButton->show();
    statusBar()->showMessage(tr("Successed to disconnect!"), 2000);
    qDebug()<<tr("Successed to disconnect!");
}

void ArcFirmata::onInitConnection()
{
    for(int i=0; i<_analogPinVec.size(); ++i){
        int pin = _analogPinVec[i]->getPin();
        _firmata->sendAnalogPinReporting(pin, ARD_ON);
        _analogPinVec[i]->clear();
//        onAnalogModeChange(pin, 0);
    }
    for(int i=0; i<_digitalPinVec.size(); ++i){
        int pin = _digitalPinVec[i]->getPin();
        _firmata->sendDigitalPinReporting(pin, ARD_ON);
        _digitalPinVec[i]->clear();
//        onDigitalModeChange(pin, 0);
    }
    setPinsEnable(true);
    statusBar()->showMessage(tr("Successed to connect!"), 2);
}

void ArcFirmata::onInitTask()
{
    onDisconnectButtonClicked();
    QString arduino_debug = ProfileReader::getInstance()->getArduino();
    try{
        SCPP_ASSERT_THROW(!arduino_debug.isEmpty(), "Arduino debuger not profiled yet!");
        arduino_debug = arduino_debug.replace('\\', '/');
        int ind = arduino_debug.lastIndexOf('/');
        SCPP_ASSERT_THROW(ind>=0, "Arduino debuger path have not profiled yet!");
        QString std_firmata = arduino_debug.left(ind)
                           + "/libraries/Firmata/examples/StandardFirmata/StandardFirmata.ino";
        QString portName = _portComboBox->currentText();
        SCPP_ASSERT_THROW(!portName.isEmpty(), "Haven't choose COM yet!");
        QStringList arglist = QStringList()
                               <<"--board"<<"arduino:avr:uno"
                               <<"--port"<<portName
                               <<"--upload"<<std_firmata;
        qDebug()<<arduino_debug<<arglist;
        /* set process */
        QProcess p;
        p.setProgram(arduino_debug);
        p.setArguments(arglist);
        connect(&p, &QProcess::readyReadStandardError, [&](){
            QString msg = QString(p.readAllStandardError());
            qDebug()<<msg;
            statusBar()->showMessage(msg);
        });
        connect(&p, &QProcess::readyReadStandardOutput, [&](){
            QString msg = QString(p.readAllStandardOutput());
            qDebug()<<msg;
            statusBar()->showMessage(msg);
        });
        p.start();
        statusBar()->showMessage(tr("Waiting for loading!"));
        p.waitForFinished(25000);
        if(p.exitStatus()==QProcess::NormalExit && p.exitCode()==0){
            /* success to upload */
            statusBar()->showMessage(tr("Successed to load!"), 2000);
            _connectButton->click();
        }
        else{
            /* error happend */
            statusBar()->showMessage(tr("ERROR: fail to load!"));
        }
    }
    catch(...){
        statusBar()->showMessage(tr("ERROR: fail to load!"));
    }
}

void ArcFirmata::initUI()
{
    /* main layout */
    QVBoxLayout * layout = new QVBoxLayout();
    this->centralWidget()->setLayout(layout);
    this->layout()->setSpacing(5);
    _pinsLayout = new QVBoxLayout();
    _pinsLayout->setSpacing(5);
    _pinsLayout->setMargin(0);

    /* Serial GUI */
    QHBoxLayout * lySerial = new QHBoxLayout();
    _portComboBox  = new QComboBox(this);
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        _portComboBox->addItem(info.portName());
    QPushButton * _initButton = new QPushButton(tr("initialize"));
    _connectButton = new QPushButton(tr("connect"),this);
    _disconnectButton = new QPushButton(tr("disconnect"),this);
    _disconnectButton->hide();
    lySerial->addWidget(new QLabel(tr("Serial:")));
    lySerial->addWidget(_portComboBox);
    lySerial->addWidget(_initButton);
    lySerial->addWidget(_connectButton);
    lySerial->addWidget(_disconnectButton);
    lySerial->addSpacerItem(new QSpacerItem(1,1,QSizePolicy::Expanding));
    layout->addLayout(lySerial);

    connect(_initButton, SIGNAL(clicked(bool)), this, SLOT(onInitTask()));
    connect(_connectButton, SIGNAL(clicked(bool)), this, SLOT(onConnectButtonClicked()));
    connect(_disconnectButton, SIGNAL(clicked(bool)), this, SLOT(onDisconnectButtonClicked()));

    /* GUI PIN_AI */
    for(int i=0; i<=5; ++i){
        PIN_AI * pin_ai = new PIN_AI(i);
        _pinsLayout->addWidget(pin_ai);
        _analogPinVec.push_back(pin_ai);
        connect(pin_ai, SIGNAL(pinModeChange(uint,int)),
                this,   SLOT(onAnalogModeChange(uint,int)));
        connect(pin_ai, SIGNAL(setDoValue(uint,bool)),
                this,   SLOT(onAnalogWriteDChange(uint,bool)));
    }
    _pinsLayout->addSpacerItem(new QSpacerItem(1, 30));

    /* GUI PIN_DO */
    for(int i=2; i<=13; ++i){
        PIN_DO * pin_do = new PIN_DO(i);
        _pinsLayout->addWidget(pin_do);
        _digitalPinVec.push_back(pin_do);
        connect(pin_do, SIGNAL(pinModeChange(uint,int)),
                this,   SLOT(onDigitalModeChange(uint,int)));
        connect(pin_do, SIGNAL(setDoValue(uint,bool)),
                this,   SLOT(onDigitalWriteChange(uint,bool)));
        connect(pin_do, SIGNAL(setPWMValue(uint,int)),
                this,   SLOT(onDigitalPwmChange(uint,int)));
    }
    layout->addLayout(_pinsLayout);

    /* firmate connection */
    connect(_firmata, SIGNAL(firmwareVersionReceived(int, int)), this, SLOT(onFirmwareVersionReceived(int,int)));
    connect(_firmata, SIGNAL(firmwareNameReceived(QString)), this, SLOT(onInitConnection()));
    connect(_firmata, SIGNAL(firmwareNameReceived(QString)), this, SLOT(onFirmwareNameReceived(QString)));
    connect(_firmata, SIGNAL(initialized(int,int,QString)), this, SLOT(onInitialized(int,int,QString)));
    connect(_firmata, SIGNAL(digitalPinChanged(int)), this, SLOT(onDigitalPinChanged(int)));
    connect(_firmata, SIGNAL(analogPinChanged(int)), this, SLOT(onAnalogPinChanged(int)));

    /* init GUI style */
    setPinsEnable(false);
    _disconnectButton->hide();
}

void ArcFirmata::setPinsEnable(bool isEnabled)
{
    foreach(PIN_DO *p, _digitalPinVec)
        p->setEnabled(isEnabled);
    foreach(PIN_AI *p, _analogPinVec)
        p->setEnabled(isEnabled);
}
