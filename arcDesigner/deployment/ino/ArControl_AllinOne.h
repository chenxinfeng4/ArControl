#ifndef ARCONTROL_ALLINONE
#define ARCONTROL_ALLINONE

void pinScanning();
void edgeScanning();
void pinWriting(int, boolean);
void sendmsg(char [], int, unsigned long, unsigned long);
void sendmsg(char [], int);
#define delay(t) {unsigned long i = millis() + t; while(millis()<i) pinScanning();}
#define digitalWrite(pin,level) {digitalWrite(pin,level);pinWriting(pin,level);}
#define analogWrite(pin,level) {analogWrite(pin,level);pinWriting(pin,(boolean)level);}
#include "ArControl.h" //important!

#define SERIAL_PIN_CONTROL

#ifndef DO2OUT
#define DO2OUT -1    //DO2 -> OUT1
#endif
#ifndef AI2IN
#define AI2IN 1      //AI0 -> IN1
#endif
#ifndef MY_AIPIN
const int AIpin[] = {0,1,2,3,4,5}; //AI_PIN to pinScanning();
#else
extern const int AIpin[];
#endif


#if defined UNO_SPEEDUP || defined NANO_SPEEDUP
#define PIN_IN_6_1 PINC    //PINC = [NULL NULL A5<-A0] for repid read all input channel
#elif defined MEGA_SPEEDUP
#define PIN_IN_6_1 PINF    //PINF = [? ? A5<-A0]
#else
#define PIN_IN_6_1 0x00    //NULL
#endif



////////////////////////////////// edge Scan ////////////////////////////////////////////////
////////////////////////////////// 上升沿下降沿扫描 /////////////////////////////////////////
//Promote speed of AI-pinScanning for [UNO | NANO | MEGA] boards only. Recommend.
///AIpin choosed  counts        [NONE 1    2    3    4    5    6]
//SPEEDUP  edgeScanning():       [1.2  2    -    3    -    -    4]  us while work-pinScanning()-leisure.
byte risingEdge=0, downingEdge=0;
void edgeScanning()
{
  static byte pre_status = 0;
  byte now_status = PIN_IN_6_1 ^ AI_reverse;
  byte changed_status = pre_status ^ now_status;
  risingEdge = changed_status & now_status;
  downingEdge = changed_status & pre_status;
  pre_status = now_status;
}
boolean cpp_ListenAI_edge(int PinNum, boolean PinEdge = HIGH)
{
  return bitRead(PinEdge?risingEdge:downingEdge, PinNum); //read the Edge from variable
}

boolean const * out_pre_status;
#if defined SERIAL_PIN_CONTROL
void switchOutPin(char buff[], int len){
    int pin1base = buff[1]-'0';
    if(len==2 && buff[0]=='^' && pin1base>=1 && pin1base<=8){
		int pin = pin1base - DO2OUT;
		boolean nextLevel = !out_pre_status[pin];
        digitalWrite(pin,nextLevel);
    }
}
void reverseInPin(char buff[], int len){
	int pin1base = buff[1]-'0';
	if(len==2 && buff[0]=='&' && pin1base>=1 && pin1base<=8){
		int pin = pin1base - AI2IN;
		bitWrite(AI_reverse, pin, bitRead(AI_reverse, pin)==0);
    }
}
void serialCommandScanning(){
    static int i=0;
    static char buff[10]="";
    static int len = 0;
    if(++i<50){return;}  //run once every 50 loop
    i = 0;
    int nchar = Serial.available();
    for(int j=0; j<nchar; j++){
        char c = Serial.read();
        if(c=='\n' || c==';'){  //process a record
            buff[len+1] = '\0';
            if(buff[0]=='^'){
                switchOutPin(buff, len);
			}
			else if(buff[0]=='&'){
                reverseInPin(buff, len);
			}
			else{;}
            len = 0;
        }
        else{  //add into the record
            buff[len] = c;
            len++;
        }
    }
    
}
#else
void serialCommandScanning(){;}
#endif

#if defined UNO_SPEEDUP || defined NANO_SPEEDUP || defined MEGA_SPEEDUP
//Promote speed of AI-pinScanning for [UNO | NANO | MEGA] board. Recommend.
///AIpin choosed  counts        [NONE 1    2    3    4    5    6]
//UNO_SPEEDUP  pinScanning():    [1.2  7    7    7    7    7    7]  us while work-pinScanning()-leisure.
//                              [0.8  16   16   16   16   16  16]  us while work-pinScanning()-engaged (at least).
//NONE UNO_SPEEDUP pinScanning():[1.2  9    18   24   29   34  40]  us while work-pinScanning()-leisure.
//                              [1.2  9    18   24   29   34  40]  us while work-pinScanning()-engaged (at least).
void pinScanning()
{
    static boolean doinit = 1;
    static unsigned long t_raise[6];
    static byte pre_status = 0; //[NULL NULL A5<-A0]
    char prefix[] = "IN";
    if(doinit) {                       // do init, the first time
        doinit = 0;
        unsigned long AppBeginTime = mySaver.getAppBeginTime();
        for(int i = 0; i < 6; ++i) {
            t_raise[i] = AppBeginTime;   //when pin start with HIGH
        }
        for(int i = 0; i < sizeof(AIpin) / sizeof(int); ++i) {
            bitWrite(AI_reverse, AIpin[i], 0);
        }
    }
    byte now_status = PIN_IN_6_1 ^ AI_reverse;
    byte changed_status = pre_status ^ now_status;
    if(changed_status !=  0) {   //reduce time consume
        unsigned long now_time = millis();
        for(int i = 0; i < 6; ++i) {
            if(bitRead(changed_status, i)) {
                if(bitRead(now_status, i)) { //up slope
                    t_raise[i] = now_time;
                    sendmsg("$", AIpin[i] + AI2IN);
                }
                else {              //down slope
                    sendmsg(prefix, i + AI2IN, t_raise[i], now_time);
                }
            }
        }
        pre_status = now_status;
    }
    serialCommandScanning();
}
#else
void pinScanning()
{
    static boolean doinit = 1;
    static int count = sizeof(AIpin) / sizeof(int);
    static int * AI2DIpin;
    static boolean * pre_status;
    static unsigned long * t_raise;
    char prefix[] = "IN";
    if(doinit) {                       // do init, the first time
        doinit = 0;
        unsigned long AppBeginTime = mySaver.getAppBeginTime();
        AI2DIpin = new int [count];
        pre_status = new boolean[count];
        t_raise = new unsigned long[count];
        for(int i = 0; i < count; ++i) {
            AI2DIpin[i] = AIpin[i] + A0; //A0 =  = 14 for Arduino-UNO
            pinMode(AI2DIpin[i], INPUT);
            pre_status[i] = LOW;
            t_raise[i] = AppBeginTime;   //when pin start with HIGH
        }
    }
    boolean now_status;
    for(int i = 0; i < count; ++i)
    {
        now_status = digitalRead(AI2DIpin[i]); //HIGH-V is signal
        unsigned long now_time = millis();
        if(pre_status[i] == LOW && now_status == HIGH) {
            t_raise[i] = now_time;
            pre_status[i] = now_status;
            sendmsg("$", AIpin[i] + AI2IN);
        }
        else if(pre_status[i] == HIGH && now_status == LOW) {
            pre_status[i] = now_status;
            sendmsg(prefix, AIpin[i] + AI2IN, t_raise[i], now_time);
        }
    }
    serialCommandScanning();
}
#endif


// DO_PIN for pinWriting()?
const boolean DOrecord[] = {0,0,1,1,1,1,1,1,1,1,1,0,0,0};

// When 'digitalWrite' or 'analogWrite', Record its' time!
void pinWriting(int pin, boolean level)
{
    static boolean doinit = 1;
    static int count = sizeof(DOrecord) / sizeof(boolean);
    static int * DOpin; //0 to DOcount-1
    static boolean * pre_status;
    static unsigned long * t_raise;
    static unsigned long * t_decline;
    char prefix[] = "OUT";
    if(doinit) {
        doinit = 0;
        DOpin = new int [count];
        pre_status = new boolean [count];
        out_pre_status = pre_status;
        t_raise = new unsigned long [count];
        t_decline = new unsigned long [count];
        for(int i = 0; i < count; ++i) {
            pre_status[i] = LOW;
            DOpin[i] = i;
        }
    }
    if(!DOrecord[pin]) {return;}
    boolean now_status = level;
    int i = pin;
    if(pre_status[i] == LOW && now_status == HIGH) {
        t_raise[i] = millis();
        pre_status[i] = now_status;
        sendmsg("%", DOpin[i] + DO2OUT);
    }
    else if( pre_status[i] == HIGH && now_status == LOW ) {
        t_decline[i] = millis();
        pre_status[i] = now_status;
        sendmsg(prefix, DOpin[i] + DO2OUT, t_raise[i], t_decline[i]);
    }
}

// Serial.print for 'pinScanning()', 'pinWriting()'
// "OUT3:100 300": D3, turn on at 100ms, off at 300ms, duration as 200ms
// "IN3:100 300" : AI3, turn on at 100ms, off at 300ms, duration as 200ms
void sendmsg(char prefix[], int pin, unsigned long t_raise, unsigned long t_decline)
{
    char buf[40], temp[11];  //long is 10 char + '\0';
    unsigned long AppBeginTime = mySaver.getAppBeginTime();
    buf[0] = '\0';
    strcat(buf, prefix);
    strcat(buf,itoa(pin, temp,10));
    strcat(buf, ":");
    strcat(buf, ultoa(t_raise - AppBeginTime, temp, 10));
    strcat(buf, " ");
    strcat(buf, ultoa(t_decline - t_raise, temp, 10));
    strcat(buf, "\n");
    Serial.print(buf);
}
void sendmsg(char prefix[], int pin)
{
    char buf[7], temp[3];
    buf[0] = '\0';
    strcat(buf, prefix);
    strcat(buf, itoa(pin, temp, 10));
    strcat(buf, "\n");
    Serial.print(buf);
}
#endif