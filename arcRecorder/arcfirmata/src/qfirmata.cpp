#include "qfirmata.h"

#define QT_NO_DEBUG_OUTPUT

#include <QDebug>

QString QFirmata::getVersionString()
{
    QString versionS;
    versionS += QString::number(QFIRMATA_MAJOR_VERSION);
    versionS += ".";
    versionS += QString::number(QFIRMATA_MINOR_VERSION);
    versionS += ".";
    versionS += QString::number(QFIRMATA_PATCH_VERSION);
    return versionS;
}

QFirmata::QFirmata(QObject* parent) :
//    QThread(parent)
  QObject(parent)

{
    _portStatus=-1;
    _waitForData=0;
    _analogHistoryLength = 2;
    _digitalHistoryLength = 2;
    _stringHistoryLength = 1;
    _sysExHistoryLength = 1;

    _majorProtocolVersion = 0;
    _minorProtocolVersion = 0;
    _majorFirmwareVersion = 0;
    _minorFirmwareVersion = 0;
    _firmwareVersionSum = 25;
    _firmwareName = "Unknown";

    bUseDelay = true;

    _serial = new QSerialPort(this);

    connect(_serial, SIGNAL(readyRead()), this, SLOT(readData()));
}

QFirmata::~QFirmata() {
    _serial->close();
}

bool QFirmata::open(QString portName)
{
    if(_serial->isOpen())
    {
        _serial->close();
    }
    _serial->setPortName(portName);
    _serial->setBaudRate(57600);
    _serial->setDataBits(QSerialPort::Data8);
    _serial->setParity(QSerialPort::NoParity);
    _serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!_serial->open(QIODevice::ReadWrite)) {
        //emit error(tr("Can't open %1, error code %2").arg(portName).arg(serial.error()));
        qDebug()<<"not connected to"<<portName<<"error"<<_serial->error();
        return false;
    }

    _serial->setDataTerminalReady(true); //reset arduino
    _serial->clear();
    initPins();

    qDebug()<<"connected to"<<portName;
//    start();
    return true;
}


// initialize pins once we get the Firmata version back from the Arduino board
// the version is sent automatically by the Arduino board on startup
void QFirmata::initPins() {
    int firstAnalogPin;

//    if (_initialized) return;   // already initialized

    // support Firmata 2.3/Arduino 1.0 with backwards compatibility
    // to previous protocol versions
    if (_firmwareVersionSum >= FIRMWARE2_3) {
        _totalDigitalPins = 20;
        firstAnalogPin = 14;
    } else {
        _totalDigitalPins = ARD_TOTAL_DIGITAL_PINS;
        firstAnalogPin = 16;
    }

    // ports
    for(int i=0; i<ARD_TOTAL_PORTS; ++i) {
        _digitalPortValue[i]=0;
        _digitalPortReporting[i] = ARD_OFF;
    }

    // digital pins
    for(int i=0; i<firstAnalogPin; ++i) {
        _digitalPinValue[i] = -1;
        _digitalPinMode[i] = ARD_OUTPUT;
        _digitalPinReporting[i] = ARD_OFF;
    }

    // analog in pins
    for (int i=firstAnalogPin; i<_totalDigitalPins; ++i) {
        _analogPinReporting[i-firstAnalogPin] = ARD_OFF;
        // analog pins used as digital
        _digitalPinMode[i]=ARD_ANALOG;
        _digitalPinValue[i] = -1;
    }

    for (int i=0; i<_totalDigitalPins; ++i) {
        _servoValue[i] = -1;
    }

    _initialized = true;
}


// this method is not recommended
// the preferred method is to listen for the EInitialized event in your application
bool QFirmata::isArduinoReady(){
    /*
    if(bUseDelay) {
        if (_initialized || (ofGetElapsedTimef() - connectTime) > OF_ARDUINO_DELAY_LENGTH) {
            initPins();
            connected = true;
        }
    }
    */
    return connected;
}

void  QFirmata::setUseDelay(bool bDelay){
    bUseDelay = bDelay;
}

void QFirmata::setDigitalHistoryLength(int length){
    if(length>=2)
        _digitalHistoryLength=length;
}

void QFirmata::setAnalogHistoryLength(int length){
    if(length>=2)
        _analogHistoryLength=length;
}

void QFirmata::setSysExHistoryLength(int length){
    if(length>=1)
        _sysExHistoryLength=length;
}

void QFirmata::setStringHistoryLength(int length){
    if(length>=1)
        _stringHistoryLength=length;
}

void QFirmata::close(){
    _serial->close();
//    terminate();
 }


void QFirmata::readData()
{
    QByteArray bytes = _serial->readAll();
    if(bytes.size() > 0)
    {
    }
    for(int i = 0; i < bytes.size(); i++)
    {
        processData(bytes.at(i));
    }
}

int QFirmata::getAnalog(int pin){
    if(_analogHistory[pin].size()>0)
        return _analogHistory[pin].front();
    else
        return -1;
}

int QFirmata::getDigital(int pin){
    if(_digitalPinMode[pin]==ARD_INPUT && _digitalHistory[pin].size()>0)
        return _digitalHistory[pin].front();
    else if (_digitalPinMode[pin]==ARD_OUTPUT)
        return _digitalPinValue[pin];
    else
        return -1;
}

int QFirmata::getPwm(int pin){
    if(_digitalPinMode[pin]==ARD_PWM)
        return _digitalPinValue[pin];
    else
        return -1;
}

QVector<unsigned char> QFirmata::getSysEx(){
    return _sysExHistory.front();
}

QString QFirmata::getString(){
    return _stringHistory.front();
}

int QFirmata::getDigitalPinMode(int pin){
    return _digitalPinMode[pin];
}

void QFirmata::sendDigital(int pin, int value, bool force){
    if((_digitalPinMode[pin]==ARD_INPUT || _digitalPinMode[pin]==ARD_OUTPUT) && (_digitalPinValue[pin]!=value || force)){

        _digitalPinValue[pin] = value;

        int port=0;
        int bit=0;
        int port1Offset;
        int port2Offset;

        // support Firmata 2.3/Arduino 1.0 with backwards compatibility
        // to previous protocol versions
        if (_firmwareVersionSum >= FIRMWARE2_3) {
            port1Offset = 16;
            port2Offset = 20;
        } else {
            port1Offset = 14;
            port2Offset = 22;
        }

        if(pin < 8 && pin >1){
            port=0;
            bit = pin;
        }
        else if(pin>7 && pin <port1Offset){
            port = 1;
            bit = pin-8;
        }
        else if(pin>15 && pin <port2Offset){
            port = 2;
            bit = pin-16;
        }

        // set the bit
        if(value==1)
            _digitalPortValue[port] |= (1 << bit);

        // clear the bit
        if(value==0)
            _digitalPortValue[port] &= ~(1 << bit);

        sendByte(FIRMATA_DIGITAL_MESSAGE+port);
        sendValueAsTwo7bitBytes(_digitalPortValue[port]);

//        QByteArray bytes;
//        bytes.append((unsigned char)(FIRMATA_DIGITAL_MESSAGE+port));
//        bytes.append((unsigned char)(value & 127)); // LSB
//        bytes.append((unsigned char)(value >> 7 & 127)); // MSB
//        send(bytes);
    }
}

void QFirmata::sendPWM(int pin, int value, bool force){
    QByteArray bytes;
    if(_digitalPinMode[pin]==ARD_PWM && (_digitalPinValue[pin]!=value || force)){
//        bytes.append(FIRMATA_ANALOG_MESSAGE+pin);
//        bytes.append(value & 127); // LSB
//        bytes.append(value >> 7 & 127); // MSB

        sendByte(FIRMATA_ANALOG_MESSAGE+pin);
        sendValueAsTwo7bitBytes(value);
        _digitalPinValue[pin] = value;

//        send(bytes);
    }
}

void QFirmata::sendSysEx(int command, QVector<unsigned char> data){
    QByteArray bytes;
    bytes.append(FIRMATA_START_SYSEX);
    bytes.append(command);
//    sendByte(FIRMATA_START_SYSEX);
//    sendByte(command);
    QVector<unsigned char>::iterator it = data.begin();
    while( it != data.end() ) {
        //sendByte(*it);	// need to split data into 2 bytes before sending
        sendValueAsTwo7bitBytes(*it);
        it++;
    }
    bytes.append(FIRMATA_END_SYSEX);
//    sendByte(FIRMATA_END_SYSEX);
}

void QFirmata::sendSysExBegin(){
    sendByte(FIRMATA_START_SYSEX);
}

void QFirmata::sendSysExEnd(){
    sendByte(FIRMATA_END_SYSEX);
}

void QFirmata::sendString(QString str){
    sendByte(FIRMATA_START_SYSEX);
    sendByte(FIRMATA_SYSEX_FIRMATA_STRING);
    QString::iterator it = str.begin();
    while( it != str.end() ) {
        sendValueAsTwo7bitBytes((*it).unicode());
        it++;
    }
    sendByte(FIRMATA_END_SYSEX);
}

void QFirmata::sendProtocolVersionRequest(){
    sendByte(FIRMATA_REPORT_VERSION);
}

void QFirmata::sendFirmwareVersionRequest(){
    sendByte(FIRMATA_START_SYSEX);
    sendByte(FIRMATA_SYSEX_REPORT_FIRMWARE);
    sendByte(FIRMATA_END_SYSEX);
}

void QFirmata::sendReset(){
    sendByte(FIRMATA_SYSTEM_RESET);
}

void QFirmata::sendAnalogPinReporting(int pin, int mode){

    int firstAnalogPin;
    // support Firmata 2.3/Arduino 1.0 with backwards compatibility
    // to previous protocol versions
    if (_firmwareVersionSum >= FIRMWARE2_3) {
        firstAnalogPin = 14;
    } else {
        firstAnalogPin = 16;
    }

    // if this analog pin is set as a digital input, disable digital pin reporting
    if (_digitalPinReporting[pin + firstAnalogPin] == ARD_ON) {
        sendDigitalPinReporting(pin + firstAnalogPin, ARD_OFF);
    }

    _digitalPinMode[firstAnalogPin+pin]=ARD_ANALOG;

    sendByte(FIRMATA_REPORT_ANALOG+pin);
    sendByte(mode);
    _analogPinReporting[pin] = mode;
}

void QFirmata::sendDigitalPinMode(int pin, int mode){
    sendByte(FIRMATA_SET_PIN_MODE);
    sendByte(pin);
    sendByte(mode);
    _digitalPinMode[pin]=mode;

    // turn on or off reporting on the port
    if(mode==ARD_INPUT){
        sendDigitalPinReporting(pin, ARD_ON);
    }
    else {
        sendDigitalPinReporting(pin, ARD_OFF);
    }
}

int QFirmata::getAnalogPinReporting(int pin){
    return _analogPinReporting[pin];
}

QList<int>* QFirmata::getAnalogHistory(int pin){
    return &_analogHistory[pin];
}

QList<int>* QFirmata::getDigitalHistory(int pin){
    return &_digitalHistory[pin];
}

QList<QVector<unsigned char> >* QFirmata::getSysExHistory(){
    return &_sysExHistory;
}

QList<QString>* QFirmata::getStringHistory(){
    return &_stringHistory;
}

int QFirmata::getMajorProtocolVersion(){
    return _majorFirmwareVersion;
}

int QFirmata::getMinorProtocolVersion(){
    return _minorFirmwareVersion;
}

int QFirmata::getMajorFirmwareVersion(){
    return _majorFirmwareVersion;
}

int QFirmata::getMinorFirmwareVersion(){
    return _minorFirmwareVersion;
}

QString QFirmata::getFirmwareName(){
    return _firmwareName;
}

bool QFirmata::isInitialized(){
    return _initialized;
}

// ------------------------------ private functions

void QFirmata::processData(unsigned char inputData){

    char msg[100];
    sprintf(msg, "Received Byte: %i", inputData);
    //Logger::get("Application").information(msg);

    // we have command data
    if(_waitForData>0 && inputData<128) {
        _waitForData--;

        // collect the data
        _storedInputData[_waitForData] = inputData;

        // we have all data executeMultiByteCommand
        if(_waitForData==0) {
            switch (_executeMultiByteCommand) {
                case FIRMATA_DIGITAL_MESSAGE:
                    processDigitalPort(_multiByteChannel, (_storedInputData[0] << 7) | _storedInputData[1]);
                break;
                case FIRMATA_REPORT_VERSION: // report version       
                    _majorProtocolVersion = _storedInputData[1];
                    _minorProtocolVersion = _storedInputData[0];
                    emit protocolVersionReceived(_majorFirmwareVersion, _minorFirmwareVersion);
                break;
                case FIRMATA_ANALOG_MESSAGE:
                    if(_analogHistory[_multiByteChannel].size()>0){
                        int previous = _analogHistory[_multiByteChannel].front();

                        _analogHistory[_multiByteChannel].push_front((_storedInputData[0] << 7) | _storedInputData[1]);
                        if((int)_analogHistory[_multiByteChannel].size()>_analogHistoryLength)
                            _analogHistory[_multiByteChannel].pop_back();

                        // trigger an event if the pin has changed value
                        if(_analogHistory[_multiByteChannel].front()!=previous)
                        {
                            emit analogPinChanged(_multiByteChannel);
                        }
                    }else{
                        _analogHistory[_multiByteChannel].push_front((_storedInputData[0] << 7) | _storedInputData[1]);
                        if((int)_analogHistory[_multiByteChannel].size()>_analogHistoryLength)
                            _analogHistory[_multiByteChannel].pop_back();
                    }
                break;
            }

        }
    }
    // we have SysEx command data
    else if(_waitForData<0){

        // we have all sysex data
        if(inputData==FIRMATA_END_SYSEX){
            _waitForData=0;
            processSysExData(_sysExData);
            _sysExData.clear();
        }
        // still have data, collect it
        else {
            _sysExData.push_back((unsigned char)inputData);
        }
    }
    // we have a command
    else{

        int command;

        // extract the command and channel info from a byte if it is less than 0xF0
        if(inputData < 0xF0) {
          command = inputData & 0xF0;
          _multiByteChannel = inputData & 0x0F;
        }
        else {
          // commands in the 0xF* range don't use channel data
          command = inputData;
        }

        switch (command) {
            case FIRMATA_REPORT_VERSION:
            case FIRMATA_DIGITAL_MESSAGE:
            case FIRMATA_ANALOG_MESSAGE:
                _waitForData = 2;  // 2 bytes needed
                _executeMultiByteCommand = command;
            break;
            case FIRMATA_START_SYSEX:
                _sysExData.clear();
                _waitForData = -1;  // n bytes needed, -1 is used to indicate sysex message
                _executeMultiByteCommand = command;
            break;
        }

    }
}

// sysex data is assumed to be 8-bit bytes split into two 7-bit bytes.
void QFirmata::processSysExData(QVector<unsigned char> data){

    QString str;

    QVector<unsigned char>::iterator it;
    unsigned char buffer;
    //int i = 1;

    // act on reserved sysEx messages (extended commands) or trigger SysEx event...
    switch(data.front()) { //first byte in buffer is command
        case FIRMATA_SYSEX_REPORT_FIRMWARE:
            it = data.begin();
            it++; // skip the first byte, which is the firmware version command
            _majorFirmwareVersion = *it;
            it++;
            _minorFirmwareVersion = *it;
            it++;

            while( it != data.end() ) {
                    buffer = *it;
                    it++;
                    buffer += *it << 7;
                    it++;
                    str+=buffer;
            }
            _firmwareName = str;

            _firmwareVersionSum = _majorFirmwareVersion * 10 + _minorFirmwareVersion;
            emit firmwareVersionReceived(_majorFirmwareVersion, _minorFirmwareVersion);
            emit firmwareNameReceived(_firmwareName);
            // trigger the initialization event
            if (!_initialized) {
                initPins();
//                ofNotifyEvent(EInitialized, _majorFirmwareVersion, this);
                emit initialized(_majorFirmwareVersion, _minorFirmwareVersion, _firmwareName);
            }

        break;
        case FIRMATA_SYSEX_FIRMATA_STRING:
            it = data.begin();
            it++; // skip the first byte, which is the string command
            while( it != data.end() ) {
                    buffer = *it;
                    it++;
                    buffer += *it << 7;
                    it++;
                    str+=buffer;
            }

            _stringHistory.push_front(str);
            if((int)_stringHistory.size()>_stringHistoryLength)
                    _stringHistory.pop_back();

//            ofNotifyEvent(EStringReceived, str, this);
            emit stringReceived(str);

        break;
        default: // the message isn't in Firmatas extended command set
            _sysExHistory.push_front(data);
            if((int)_sysExHistory.size()>_sysExHistoryLength)
                    _sysExHistory.pop_back();
//            ofNotifyEvent(ESysExReceived, data, this);
            emit sysExReceived(data);

        break;

    }
}

void QFirmata::processDigitalPort(int port, unsigned char value){

    unsigned char mask;
    int previous;
    int i;
    int pin;
    int port1Pins;
    int port2Pins;

    // support Firmata 2.3/Arduino 1.0 with backwards compatibility to previous protocol versions
    if (_firmwareVersionSum >= FIRMWARE2_3) {
        port1Pins = 8;
        port2Pins = 4;
    } else {
        port1Pins = 6;
        port2Pins = 6;
    }

    switch(port) {
    case 0: // pins 2-7  (0,1 are ignored as serial RX/TX)
        for(i=2; i<8; ++i) {
            pin = i;
            previous = -1;
            if(_digitalPinMode[pin]==ARD_INPUT){
              if (_digitalHistory[pin].size() > 0)
                previous = _digitalHistory[pin].front();

                mask = 1 << i;
                _digitalHistory[pin].push_front((value & mask)>>i);

                if((int)_digitalHistory[pin].size()>_digitalHistoryLength)
                        _digitalHistory[pin].pop_back();

                // trigger an event if the pin has changed value
                if(_digitalHistory[pin].front()!=previous){
                    emit digitalPinChanged(pin);
                }
            }
        }
        break;
    case 1: // pins 8-13 (in Firmata 2.3/Arduino 1.0, pins 14 and 15 are analog 0 and 1)
        for(i=0; i<port1Pins; ++i) {
            pin = i+8;
            previous = -1;
            if(_digitalPinMode[pin]==ARD_INPUT){
              if (_digitalHistory[pin].size() > 0)
                previous = _digitalHistory[pin].front();

                mask = 1 << i;
                _digitalHistory[pin].push_front((value & mask)>>i);

                if((int)_digitalHistory[pin].size()>_digitalHistoryLength)
                        _digitalHistory[pin].pop_back();

                // trigger an event if the pin has changed value
                if(_digitalHistory[pin].front()!=previous){
                    emit digitalPinChanged(pin);
                }
            }
        }
        break;
    case 2: // analog pins used as digital pins 16-21 (in Firmata 2.3/Arduino 1.0, digital pins 14 - 19)
        for(i=0; i<port2Pins; ++i) {
            //pin = i+analogOffset;
            pin = i+16;
                  previous = -1;
            if(_digitalPinMode[pin]==ARD_INPUT){
              if (_digitalHistory[pin].size() > 0)
                previous = _digitalHistory[pin].front();

                mask = 1 << i;
                _digitalHistory[pin].push_front((value & mask)>>i);

                if((int)_digitalHistory[pin].size()>_digitalHistoryLength)
                        _digitalHistory[pin].pop_back();

                // trigger an event if the pin has changed value
                if(_digitalHistory[pin].front()!=previous){
                    emit digitalPinChanged(pin);

                }
            }
        }
        break;
    }
}

// port 0: pins 2-7  (0,1 are serial RX/TX, don't change their values)
// port 1: pins 8-13 (in Firmata 2.3/Arduino 1.0, pins 14 and 15 are analog pins 0 and 1 used as digital pins)
// port 2: pins 16-21 analog pins used as digital (in Firmata 2.3/Arduino 1.0, pins 14 - 19),
//         all analog reporting will be turned off if this is set to ARD_ON

void QFirmata::sendDigitalPortReporting(int port, int mode){
    sendByte(FIRMATA_REPORT_DIGITAL+port);
    sendByte(mode);
    _digitalPortReporting[port] = mode;
    int offset;

    if (_firmwareVersionSum >= FIRMWARE2_3) {
        offset = 2;
    } else {
        offset = 0;
    }

    // for Firmata 2.3 and higher:
    if(port==1 && mode==ARD_ON) {
        for (int i=0; i<2; i++) {
            _analogPinReporting[i] = ARD_OFF;
        }
    }

    // for Firmata 2.3 and all prior Firmata protocol versions:
    if(port==2 && mode==ARD_ON){ // if reporting is turned on on port 2 then StandardFirmata on the Arduino disables all analog reporting

        for (int i=offset; i<ARD_TOTAL_ANALOG_PINS; i++) {
                _analogPinReporting[i] = ARD_OFF;
        }
    }
}

void QFirmata::sendDigitalPinReporting(int pin, int mode){
    _digitalPinReporting[pin] = mode;
    int port1Offset;
    int port2Offset;

    // Firmata backwards compatibility mess
    if (_firmwareVersionSum >= FIRMWARE2_3) {
        port1Offset = 15;
        port2Offset = 19;
    } else {
        port1Offset = 13;
        port2Offset = 21;
    }

    if(mode==ARD_ON){	// enable reporting for the port
        if(pin<=7 && pin>=2)
            sendDigitalPortReporting(0, ARD_ON);
        // Firmata backwards compatibility mess
        if(pin<=port1Offset && pin>=8)
            sendDigitalPortReporting(1, ARD_ON);
        if(pin<=port2Offset && pin>=16)
            sendDigitalPortReporting(2, ARD_ON);
    }
    else if(mode==ARD_OFF){
        int i;
        bool send=true;
        if(pin<=7 && pin>=2){    // check if all pins on the port are off, if so set port reporting to off..
            for(i=2; i<8; ++i) {
                if(_digitalPinReporting[i]==ARD_ON)
                        send=false;
            }
            if(send)
                sendDigitalPortReporting(0, ARD_OFF);
        }
        // Firmata backwards compatibility mess
        if(pin<=port1Offset && pin>=8){
            for(i=8; i<=port1Offset; ++i) {
                if(_digitalPinReporting[i]==ARD_ON)
                        send=false;
            }
            if(send)
                sendDigitalPortReporting(1, ARD_OFF);
        }
        if(pin<=port2Offset && pin>=16){
            for(i=16; i<=port2Offset; ++i) {
                if(_digitalPinReporting[i]==ARD_ON)
                        send=false;
            }
            if(send)
                sendDigitalPortReporting(2, ARD_OFF);
        }
    }
}

void QFirmata::send(QByteArray bytes)
{
    if(_serial->isOpen())
    {
        _serial->write(bytes);
        qDebug()<<"successfully sent bytes"<<bytes;
    }
    else
    {
        qDebug()<<"not connected";
    }
}

void QFirmata::sendByte(unsigned char byte){
    //char msg[100];
    //sprintf(msg, "Sending Byte: %i", byte);
    //Logger::get("Application").information(msg);


    //_port.writeByte(byte);
    QByteArray byteArray;
    byteArray.append(byte);
    if(_serial->isOpen())
    {
        _serial->write(byteArray);
    }
    else
    {
        qDebug()<<"not connected";
    }
}

// in Firmata (and MIDI) data bytes are 7-bits. The 8th bit serves as a flag to mark a byte as either command or data.
// therefore you need two data bytes to send 8-bits (a char).
void QFirmata::sendValueAsTwo7bitBytes(int value)
{
    sendByte(value & 127); // LSB
    sendByte(value >> 7 & 127); // MSB
}

// SysEx data is sent as 8-bit bytes split into two 7-bit bytes, this function merges two 7-bit bytes back into one 8-bit byte.
int QFirmata::getValueFromTwo7bitBytes(unsigned char lsb, unsigned char msb){
   return (msb << 7) | lsb;
}

void QFirmata::sendServo(int pin, int value, bool force)
{
    // for firmata v2.2 and greater
    if (_firmwareVersionSum >= FIRMWARE2_2) {
        if(_digitalPinMode[pin]==ARD_SERVO && (_digitalPinValue[pin]!=value || force)){
            sendByte(FIRMATA_ANALOG_MESSAGE+pin);
            sendValueAsTwo7bitBytes(value);
            _digitalPinValue[pin] = value;
        }
    }
    // for versions prior to 2.2
    else {
        if(_digitalPinMode[pin]==ARD_SERVO && (_servoValue[pin]!=value || force)){
            sendByte(FIRMATA_START_SYSEX);
            sendByte(SYSEX_SERVO_WRITE);
            sendByte(pin);
            sendValueAsTwo7bitBytes(value);
            sendByte(FIRMATA_END_SYSEX);
            _servoValue[pin]=value;
        }
    }
}

// angle parameter is no longer supported. keeping for backwards compatibility
void QFirmata::sendServoAttach(int pin, int minPulse, int maxPulse, int angle) {
    sendByte(FIRMATA_START_SYSEX);
    // for firmata v2.2 and greater
    if (_firmwareVersionSum >= FIRMWARE2_2) {
        sendByte(FIRMATA_SYSEX_SERVO_CONFIG);
    }
    // for versions prior to 2.2
    else {
        sendByte(SYSEX_SERVO_ATTACH);
    }
    sendByte(pin);
    sendValueAsTwo7bitBytes(minPulse);
    sendValueAsTwo7bitBytes(maxPulse);
    sendByte(FIRMATA_END_SYSEX);
    _digitalPinMode[pin]=ARD_SERVO;
}

// sendServoDetach depricated as of Firmata 2.2
void QFirmata::sendServoDetach(int pin)
{
    sendByte(FIRMATA_START_SYSEX);
    sendByte(SYSEX_SERVO_DETACH);
    sendByte(pin);
    sendByte(FIRMATA_END_SYSEX);
    _digitalPinMode[pin]=ARD_OUTPUT;
}

int QFirmata::getServo(int pin){
    if(_digitalPinMode[pin]==ARD_SERVO)
        // for firmata v2.2 and greater
        if (_firmwareVersionSum >= FIRMWARE2_2) {
            return _digitalPinValue[pin];
        }
        // for versions prior to 2.2
        else {
            return _servoValue[pin];
        }
    else
        return -1;
}
