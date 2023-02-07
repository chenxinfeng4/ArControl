#pragma once
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#ifndef ARCONTROL_ALLINONE
inline void pinScanning()
{
    ;
}
void edgeScanning()
{
    ;
}
#endif

/* whether echo taskName in the very beginning */
#ifndef ECHO_TASKNAME
const char taskName[] = "";
#endif

/* how to start ArControl */
#if !defined(START_SOON) && !defined(START_HARD) && !defined(START_SOFT) && !defined(START_HARD_SOFT)
#define START_SOFT
#endif

////////////////////////////////// Timer ////////////////////////////////////////////////
////////////////////////////////// 定时器--定义 /////////////////////////////////////////
class Timer {
private:
    unsigned long StartTime_; //Time when begin, unit = ms | 开始计时的时刻，单位ms
    unsigned long DurTime_; //Time duration, unit = ms | 设定的计时时间长，单位ms
public:
    // Returns time from Timer-Begin, unit = s.
    // 返回开始计时后的时间，单位s
    double RdTimer()
    {
        return (millis() - StartTime_) / (double)1000.0;
    }

    // Sets Timer long, unit = s.
    //     Setting t=0, means never isTimeOut().
    // 设置计时的时间长，单位s
    //     设置t=0, 代表永不超时
    void SetTimer(double t)
    {
        this->DurTime_ = t * 1000;
    }

    // Begin Timer
    // 开始计时
    void BgTimer()
    {
        this->StartTime_ = millis();
    }

    // Measures if Timer-Out, returns true if it's.
    // 是否超出计时时长, 返回true->超出了
    inline boolean isTimeOut()
    {
        return this->DurTime_ && (millis() - this->StartTime_) >= this->DurTime_;
    }

    // Measures the time from Timer-Begin, unit = ms.
    // 查看开始计时的时刻，单位ms
    unsigned long RdBgMillis()
    {
        return this->StartTime_;
    }

    // Initialization
    // 初始化构造
    Timer(double t = 0)
    {
        this->BgTimer();
        this->SetTimer(t);
    }
};

////////////////////////////////// Data Save ////////////////////////////////////////////////
////////////////////////////////// 保存数据 /////////////////////////////////////////////////
class Saver {
private:
    char datachar[40]; //Storage record | 存储记录
    unsigned long AppBeginTime; //SESSION time-baseline, unit = ms | 时间基线，单位ms
public:
    //Define the time baseline as the application begin.
    //定义时间基线
    void ResetTime(unsigned long t = millis())
    {
        this->AppBeginTime = t;
    }

    //Append a record to Serial
    //追加一组记录，输出到串口
    void attach(const int& Ci,
        const int& Si,
        const unsigned long& t)
    {
        /* //It's faster than 
          Serial.print('C');Serial.print(Ci);
          Serial.print('S');Serial.print(Si);
          Serial.print(':');Serial.println(t-this->AppBeginTime);
         //It's faster than 'Streaming. he
          Serial<<'C'<<Ci<<'S'<<Si<<':'<<t-this->AppBeginTime<<endl;
        */
        char temp[11];
        char* datachar = this->datachar;
        datachar[0] = '\0';
        strcat(datachar, "C");
        strcat(datachar, itoa(Ci, temp, 10));
        strcat(datachar, "S");
        strcat(datachar, itoa(Si, temp, 10));
        strcat(datachar, ":");
        strcat(datachar, ultoa(t - this->AppBeginTime, temp, 10));
        strcat(datachar, "\n");
        Serial.print(datachar);
    }

    // get the SESSION time-baseline, unit = ms
    // 查看时间基线，单位ms
    unsigned long getAppBeginTime()
    {
        return this->AppBeginTime;
    }
};
Saver mySaver;

////////////////////////////////// Simple Chain(Node) ///////////////////////////////////
////////////////////////////////// 简单链表 /////////////////////////////////////////////
template <typename T1, typename T2>
class Node {
private:
    uint8_t pos; //(position), current No. ITEM within CHAIN, 0~len-1 | 链表中当前项，0~len-1
    uint8_t len; //(length),counts all ITEMs in CHAIN | 链表中所有项的总数
    uint8_t lenstg; //(length storage) of CHAIN predistributed | 链表预分配空间
    T1* Sub1; //array, store ITEMs-SUB1 | 储存 [所有项]-子项1
    T2* Sub2;

public:
    // Initialization
    // 初始化构造
    Node(void)
    {
        pos = 0;
        len = 1;
        lenstg = 1;
        Sub1 = new T1[lenstg];
        Sub2 = new T2[lenstg];
    }

    // Shift down an ITEM. Then judge whether available.
    //     If YES, return true, and be able to use getSub?() and setSub?().
    //     If NO, return false, and cycle the current ITEM at the first.
    // 下移一项，并判断该项是否可用
    //     可用则返回true，并能使用 getSub?() 和 setSub?()。
    //     不可用则返回false，轮转回到开头的项。
    boolean Next(void)
    {
        if (pos == len - 1) {
            pos = 0;
            return 0;
        }
        pos = (++pos) % len;
        return 1;
    }

    // Get current ITEM-SUB?.
    // 获取当前项下的数据
    T1 getSub1(void)
    {
        return Sub1[pos];
    }
    T2 getSub2(void)
    {
        return Sub2[pos];
    }

    // Set current ITEM-SUB?.
    // 设置当前项下的数据
    void setSub1(T1 sub)
    {
        this->Sub1[pos] = sub;
    }
    void setSub2(T2 sub)
    {
        this->Sub2[pos] = sub;
    }

    // Append a new ITEM
    // 添加新的成员项
    void addSubs(T1 t1, T2 t2=NULL)
    {
        if (len == lenstg) {
            lenstg = lenstg + 2;
            T1* temp1 = new T1[lenstg];
            T2* temp2 = new T2[lenstg];
            for (unsigned int i = 0; i < len; ++i) {
                temp1[i] = Sub1[i];
                temp2[i] = Sub2[i];
            }
            delete[] Sub1;
            delete[] Sub2;
            Sub1 = temp1;
            Sub2 = temp2;
        }
        Sub1[len] = t1;
        Sub2[len] = t2;
        ++len;
    }
};

class DoPinBase {
public:
    bool enable = true;
    float (*getDurSec)(void);
    DoPinBase(int PinNum_, float (*getDurSec_)(void) = NULL)
        : PinNum(PinNum_), getDurSec(getDurSec_)
    {
    }
    uint8_t PinNum;
    virtual bool dofun(unsigned long milliseconds) = 0;
    virtual void reset() = 0;
};

///////////////////////////////// define STATE-MACHINE //////////////////////////////////
////////////////////////////////// 状态机--定义 /////////////////////////////////////////
void myVOIDNULL(void)
{
}
int myINTNULL(void)
{
    return 0;
}
boolean myBOOLNULL(void)
{
    return false;
}
float myFLOATNULL(void)
{
    return 0;
}
class State;
State* mySTATENULL(void);
State* mySTATENULL(void)
{
    return NULL;
};
class State {
public:
    static State* NEXTSTATE; //NEXT STATE to be run | 下一个状态
    static State* ENDSTATE; //END STATE to be run | 最终状态，被初始化
private:
    unsigned int CountNow; //current count | 当前计数
    int _CountSet; //COUNTER for STATE max count | 暂时存 随机数
    Timer myTimer; //TIMER for STATE max duration | 计时时钟
    Node<boolean (*)(void), State* (*)(void)> myNode; //EVT for STATE trigger | 实时监听事件发生
public:
    uint8_t Ci,
        Si; //ID of each STATE | 状态机ID
    void (*dovar)(void); //FUNC.P.; DO-Function when STATE entry | 函数指针;当RUN时;自动执行对应函数
    Node<DoPinBase*, bool> doPinList;
    void (*dofun)(void); //FUNC.P.; DO-Function when STATE entry | 函数指针;当RUN时;自动执行对应函数
    boolean (*varListener)(void); //FUNC.P.; Listen-Event when STATE entry, once only | 函数指针;监听器，每状态只监听一次
    State* (*varListenerSTATE)(void); //FUNC.P.; reach above, transform STATE | 函数指针;监听器，当返回为true是→状态转化！
    int (*CountSet)(void); //FUNC.P.; COUNTER for STATE max count | 函数指针;计数设定
    State* (*CountSetSTATE)(void); //FUNC.P.; reach above, transform STATE | 函数指针;本State运行次数>计数设定→状态转化!
    float (*TimerSet)(void); //FUNC.P.; TIMER for STATE max duration, unit = s | 函数指针;超时设定,单位s
    State* (*TimerSetSTATE)(void); //FUNC.P.; reach above, transform STATE | 函数指针;当计时>超时设定→状态转化！
    boolean (*evtListener)(void); //FUNC.P.; Listen-Event when STATE entry, loop | 函数指针;监听器，循环执行
    State* (*evtListenerSTATE)(void); //FUNC.P.; reach above, transform STATE | 函数指针;监听器，当返回为true是→状态转化！

    // Initialization with Defaults
    // 利用默认值来初始化构造
    State(int ci = 0, int si = 0)
    {
        this->_CountSet = -1; //Means need-to-init | 标记，需要再被初始化
        this->Ci = ci;
        this->Si = si;
        this->CountNow = 0;
        this->dovar = myVOIDNULL;
        this->dofun = myVOIDNULL;
        this->varListener = myBOOLNULL;
        this->varListenerSTATE = mySTATENULL;
        this->CountSet = myINTNULL;
        this->TimerSet = myFLOATNULL;
        this->evtListener = myBOOLNULL;
        this->CountSetSTATE = mySTATENULL;
        this->TimerSetSTATE = mySTATENULL;
        this->evtListenerSTATE = mySTATENULL;
    }
    // Set the mark of this State
    // 设置状态机的标志
    State* setCiSi(int ci, int si)
    {
        this->Ci = ci;
        this->Si = si;
        return this;
    }
    // Append a new pair of evtListener, achive mult-evtListener.
    // 添加指定的evtListener对；用于实现多个监听
    void addlisten(boolean (*evtListener)(void), State* (*evtListenerSTATE)(void))
    {
        this->myNode.addSubs(evtListener, evtListenerSTATE);
    }
    void addlisten(void)
    {
        this->myNode.addSubs(this->evtListener, this->evtListenerSTATE);
    }

    // Core FUNC. of STATE
    // 状态机的核心函数
    void RUN()
    {
        //Start STATE timer
        myTimer.BgTimer();

        //run the <DO-Functions>
        dovar();
        dofun();
        while (doPinList.Next()) {
            doPinList.getSub1()->reset();
        }

        unsigned long tpass_ms = 0;
        for (bool wantContinue = true; wantContinue;) {
            wantContinue = false;
            while (this->doPinList.Next()) {
                auto doPinObj = doPinList.getSub1();
                if (doPinObj->enable) {
                    doPinObj->dofun(tpass_ms);
                }
                wantContinue = wantContinue || doPinObj->enable;
            }
            tpass_ms = millis() - myTimer.RdBgMillis();
#ifndef NULL_RECORD
            pinScanning();
#endif
        }

//popout State Infomation
//Serial.print("C1S3:1250\n").
//  C1S4 = STATE id
//  1250 = STATE begin millis()
#ifndef NULL_RECORD //NULL_RECORD : don't send message
        mySaver.attach(this->Ci, this->Si, this->myTimer.RdBgMillis());
#endif
        //whether <varListener> fit
        if (this->varListener()) {
            State::NEXTSTATE = this->varListenerSTATE(); //STATE-transform |状态跳转
            return;
        }

        //whether <CountSet> fit
        if (this->_CountSet == -1) { //Means need-to-init | 标记，需要再被初始化
            this->_CountSet = this->CountSet();
        }
        this->CountNow++;
        if (this->_CountSet != 0 && this->CountNow == this->_CountSet) {
            this->_CountSet = this->CountSet(); //reset | 复位
            this->CountNow = 0; //reset | 复位
            State::NEXTSTATE = this->CountSetSTATE(); //STATE-transform |状态跳转
            return;
        }

        //whether <TimerSet> fit
        myTimer.SetTimer(this->TimerSet());
        while (!myTimer.isTimeOut()) {
            //detect rising or downing edge
            edgeScanning();
            //whether <evtListener> fit
            while (this->myNode.Next()) {
                if (this->myNode.getSub1()()) {
                    State::NEXTSTATE = this->myNode.getSub2()(); //STATE-transform |状态跳转
                    return;
                }
            }

#ifndef NULL_RECORD //NULL_RECORD : don't send massage
            pinScanning();
#endif
        }

        //It must be <TimerSet> fit
        State::NEXTSTATE = this->TimerSetSTATE(); //STATE-transform |状态跳转
        return;
    }
};
State* State::NEXTSTATE = NULL;
State* State::ENDSTATE = NULL;

/////////////////////////////// FUNC. for HARDWARE //////////////////////////////////////
/////////////////////////////// 控制硬件设备 ////////////////////////////////////////////
//digitalWrite PIN HIGH/LOW
//    new DoPinSwitch(DO_pin, HIGH)
//开关某个端口
//    new DoPinSwitch(DO_pin, HIGH)
class DoPinSwitch : public DoPinBase {
public:
    bool PinState;
    DoPinSwitch(uint8_t PinNum_ = 0, bool PinState_ = HIGH)
        : PinState(PinState_)
        , DoPinBase(PinNum_)
    {
    }
    bool dofun(unsigned long milliseconds)
    {
        digitalWrite(PinNum, PinState);
        enable = false;
        return false;
    }
    void reset()
    {
        enable = true;
    }
};

//Keepon the PIN for getDurSec seconds
//    new DoPinKeepon(DO_pin, []()->float{return 4.0;}) keep DO_pin 4.0sec on
//持续开启某个端口
//    new DoPinKeepon(DO_pin, []()->float{return 4.0;}) 将端口开启4.0秒
class DoPinKeepon : public DoPinBase {
public:
    bool init = false;
    unsigned long Duration_mm = 0;
    DoPinKeepon(uint8_t PinNum_ = 0, float (*getDurSec_)(void)=myFLOATNULL)
        : DoPinBase(PinNum_, getDurSec_)
    {
    }
    bool dofun(unsigned long milliseconds)
    {
        if (unlikely(init == false)) {
            init = HIGH;
            digitalWrite(PinNum, HIGH);
            enable = true;
        }
        if (unlikely(milliseconds >= Duration_mm)) {
            digitalWrite(PinNum, LOW);
            enable = false;
            return false;
        }
        else {
            return true;
        }
    }
    void reset()
    {
        float tdur_sec = getDurSec();
        if (tdur_sec <= 0) {
            tdur_sec = 0.001;
        }
        Duration_mm = tdur_sec * 1000; //ms
        init = false;
        enable = true;
    }
};

//Blink the PIN for getDurSec seconds
//    new DoPinBlink(DO_pin, []()->float{return 4.0;}, Fs, Duty 0~100) keep DO_pin 4.0sec on
//持续开启某个端口
//    new DoPinBlink(DO_pin, []()->float{return 4.0;}) 将端口开启4.0秒
class DoPinBlink : public DoPinBase {
public:
    bool init = false;
    bool PinState;
    unsigned long Period_mm;
    unsigned long Period_HIGH_mm;
    unsigned long Duration_mm;

    DoPinBlink(int PinNum_, float (*getDurSec_)(void), float Fs, float Duty)
        : DoPinBase(PinNum_, getDurSec_)
    {
        Period_mm = 1000.0 / Fs;
        Period_HIGH_mm = 1000.0 / Fs * double(Duty) / 100.0;
    }
    bool dofun(unsigned long milliseconds)
    {
        if (unlikely(init == false)) {
            init = HIGH;
            PinState = HIGH;
            digitalWrite(PinNum, HIGH);
            enable = true;
        }
        if (unlikely(milliseconds >= Duration_mm)) {
            digitalWrite(PinNum, LOW);
            enable = false;
            return false;
        }
        else {
            bool level = (milliseconds % Period_mm) < Period_HIGH_mm;
            if (unlikely(level != PinState)) {
                PinState = level;
                digitalWrite(PinNum, PinState);
            }
            return true;
        }
    }
    void reset()
    {
        float tdur_sec = getDurSec();
        if (tdur_sec <= 0) {
            tdur_sec = 0.001;
        }
        Duration_mm = tdur_sec * 1000; //ms
        init = false;
        enable = true;
    }
};

//Keep certain Pin(PinNum) On for some seconds(duration).
//保持某个Pin亮一段时间，单位s
void cpp_keepon(int PinNum, float duration)
{
    unsigned long D = duration * 1000; //ms
    digitalWrite(PinNum, HIGH);
    delay(D);
    digitalWrite(PinNum, LOW);
}

//Random from range of [min,max](both include), 4 significant figures.
//    Before random, you need to init by append 'randomSeed(analogRead(0))' within 'void setup()'
//从 [min,max] 之间(包含)产生连续的随机数，4位有效位数。
//    使用前，需要在 'void setup()' 中利用 'randomSeed(analogRead(0))' 初始化 random 函数.
float cpp_Range_float(float min, float max)
{
    long ii = min * 10000;
    long kk = max * 10000;
    long xx = random(0, kk - ii + 1) + ii;
    float x = xx / 10000.0;
    return x;
}

//Random from range of [min,max](both include), return integer.
//从 [min,max] 之间(包含)产生连续的随机数，整数。
int cpp_Range_int(int min, int max)
{
    int x = random(0, max - min + 1) + min;
    return x;
}

//cpp_GoElse(int chances, float lucky, float dislucky), random from alternative.
//在两个选择之间随机。
float cpp_GoElse(int chances, float a, float b)
{
    int x = random(0, 100); //x belong to 0~99 | x属于0~99
    if (x < chances - 1) {
        return a;
    }
    else
        return b;
}

//cpp_ListenAI(int PinNum, int PinValue, boolean mode)
//    PinValue: = threshold; mode =1 beyond th,  =0 under th.
//    Suit for AI port
//cpp_ListenAI(int PinNum, int PinValue, boolean mode)
//    PinValue: = 阈值; mode =1 高于阈值, =0 低于阈值
//    适用于读取"模拟输入"端口
inline boolean cpp_ListenAI(int PinNum, boolean PinState = HIGH)
{
    return digitalRead(PinNum + A0) == PinState; //A0==14 for Arduino-UNO
}

///////////////////////////// SESSION Begin&End /////////////////////////////////////////
///////////////////////////// 状态机群开始&结束 /////////////////////////////////////////
//DI:0,1,11,12,13 PINs are occupied defaultly.
//  DI 0 1: Serial
//  DI 11 : Trigger to start SESSION
//  DI 12 : The indicator-light (accurate)
//  DI 13 : The indicator-light
//DI:0,1,2,12,13引脚已经被预先占用
//  DI 0 1: 串口
//  DI 11:  端口触发SEESION开始
//  DI 12: 工作状态指示灯 (准确的)
//  DI 13: 工作状态指示灯
const uint8_t waitPin = 11;
const uint8_t keepPin[] = {
    12,
    13
};
const char bgSignal = 'b'; //Trigger to start SESSION, from Serial | 串口触发SESSION开始

//Trigger to start SESSION, form WIATPIN or BGSINGAL
//设置状态机起点，被硬件引脚或串口触发
#if defined(START_HARD_SOFT) //[START_SOON || START_HARD || START_SOFT || START_HARD_SOFT]
void keep_WAIT()
{
    pinMode(waitPin, INPUT);
    while (1) {
        while (Serial.available()) {
            if (Serial.read() == bgSignal) {
                while (Serial.available()) {
                    Serial.read();
                }
                return;
            }
        }
        if (digitalRead(waitPin) == HIGH) {
            return;
        }
    }
}
#elif defined(START_HARD)
void keep_WAIT()
{
    pinMode(waitPin, INPUT);
    while (1)
        if (digitalRead(waitPin) == HIGH)
            return;
}
#elif defined(START_SOFT)
void keep_WAIT()
{
    while (1) {
        while (Serial.available()) {
            if (Serial.read() == bgSignal) {
                while (Serial.available()) {
                    Serial.read();
                }
                return;
            }
        }
    }
}
#elif defined(START_SOON)
void keep_WAIT()
{
    return;
}
#endif
//Profile "State::ENDSTATE".
//设置状态机结束节点"State::ENDSTATE"
void ENDSTATE_SETUP()
{
    //ENDSTATE：
    //  Print "C0S0:<time>"      |  打印"C0S0:<时间>"
    //  Turn off indicator-light |  指示灯灭
    //  halt the progress        |  程序永远挂机
    State::ENDSTATE = new State(0, 0);
    State::ENDSTATE->CountSet = []() -> int {
        digitalWrite(keepPin[0], LOW);
        digitalWrite(keepPin[1], LOW);
        Serial.println("ArC-end");
        while (1) {
            ;
        }
        return 0;
    };
}

///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// Program Entry ////////////////////////////////////////////
//////////////////////////////// 程序的入口 ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
void hardware_SETUP(); //Definition outside | 在外部实现
void State_SETUP(); //Definition outside | 在外部实现
void printFilename();
void setup()
{
    Serial.begin(250000);
    Serial.println("ArControl");
    hardware_SETUP();
    printFilename();
    State_SETUP();
    ENDSTATE_SETUP();
    pinMode(keepPin[0], OUTPUT);
    pinMode(keepPin[1], OUTPUT);
    digitalWrite(keepPin[0], LOW); //Turn off indicator-light | 指示灭
    digitalWrite(keepPin[1], LOW);
    keep_WAIT(); //Wait for SESSION start by trigger | 等待触发开启
    randomSeed(micros()); //Init randomseed (waitPin shouldn't trigger immediately) | 初始化随机种子(但谨防被waitPin马上触发)
    Serial.println("ArC-bg");
    mySaver.ResetTime(); //Set as SESSION baseline time | 状态机时间基线设定
    digitalWrite(keepPin[0], HIGH); //Turn on indicator-light | 指示亮
    digitalWrite(keepPin[1], HIGH);
}

//Run STATE-MACHINE by loop "State::NEXTSTATE->RUN()"
//不断地运行状态机，通过循环执行"State::NEXTSTATE->RUN()"
void loop()
{
    if (State::NEXTSTATE != NULL) {
        State::NEXTSTATE->RUN();
    }
    else {
        State::NEXTSTATE = State::ENDSTATE;
    }
}

void printFilename() //Print task name    | 打印出文件名字
{
#ifndef ECHO_TASKNAME
    return;
#endif
    char* str = new char[strlen(taskName) + 1]; //such as "C:\Data\demo.ino", "./demo.ino", "/home/chen/demo.ino"
    strcpy(str, taskName);
    int ind_bg = 0, ind_end = 0;
    for (int i = 0; i < strlen(str); ++i) {
        if (str[i] == '\\' || str[i] == '/')
            ind_bg = i;
        else if (str[i] == '.')
            ind_end = i;
    }
    str[ind_end] = '\0';
    Serial.print("-----");
    if (ind_bg >= ind_end)
        Serial.print("Untitled"); //unreachable | 不可能
    else
        Serial.print(str + ind_bg + 1);
    Serial.print("-----\n");
    delete[] str;
}
