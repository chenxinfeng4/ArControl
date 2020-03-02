///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////ArControl Style//////////////////////////////////////
// if echo task name
#define ECHO_TASKNAME   //echo this taskname(filename) in very begining
const char taskName[] = __FILE__;
// how to start ArControl
#define START_SOFT
// recording level
#define UNO_SPEEDUP //only helpful to ArControl_AllinOne.h, improve AI-scaning
#define AI2IN 1		//AIx -> INy
#define DO2OUT -1	//DOx -> OUTy
#include "F:/ArControl/ino/ArControl_AllinOne.h"

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////task info///////////////////////////////////////////
#define INFO "" \
		"@OUT1:#1\n" \
		"@OUT2:#NoGoCue\n" \
		"@OUT3:#3\n" \
		"@OUT4:#GoCue\n" \
		"@OUT5:#Airpuf\n" \
		"@OUT6:#Water\n" \
		"@OUT7:#7\n" \
		"@OUT8:#8\n" \
		"@IN1:#1\n" \
		"@IN2:#Lick\n" \
		"@IN3:#3\n" \
		"@IN4:#4\n" \
		"@IN5:#5\n" \
		"@IN6:#6\n" \
		"@C1:init licks\n" \
		"@C1S1:Wait lick\n" \
		"@C1S2:Water\n" \
		"@C1S3:\n" \
		"@C2:Go trial\n" \
		"@C2S1:time interval\n" \
		"@C2S2:Cue\n" \
		"@C2S3:wait lick\n" \
		"@C2S4:Water\n" \
		"@C2S5:Miss\n" \
		"@C2S6:\n" \
		"@C3:NoGo trial\n" \
		"@C3S1:time interval\n" \
		"@C3S2:Cue\n" \
		"@C3S3:wait lick\n" \
		"@C3S4:Airpuff\n" \
		"@C3S5:Miss\n" \
		"@C3S6:\n" \
		""

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////global vars/////////////////////////////////////////
double Var_1 = -2;
double Var_2 = 0;
double Var_3 = 1;

///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Hardware Setup///////////////////////////////////////
const int OUT1 = 2; 	//#1
const int OUT2 = 3; 	//#NoGoCue
const int OUT3 = 4; 	//#3
const int OUT4 = 5; 	//#GoCue
const int OUT5 = 6; 	//#Airpuf
const int OUT6 = 7; 	//#Water
const int OUT7 = 8; 	//#7
const int OUT8 = 9; 	//#8
const int  IN1 = 0; 	//#1
const int  IN2 = 1; 	//#Lick
const int  IN3 = 2; 	//#3
const int  IN4 = 3; 	//#4
const int  IN5 = 4; 	//#5
const int  IN6 = 5; 	//#6
void hardware_SETUP(){
	Serial.print(F(INFO));
	pinMode(OUT1, OUTPUT);
	pinMode(OUT2, OUTPUT);
	pinMode(OUT3, OUTPUT);
	pinMode(OUT4, OUTPUT);
	pinMode(OUT5, OUTPUT);
	pinMode(OUT6, OUTPUT);
	pinMode(OUT7, OUTPUT);
	pinMode(OUT8, OUTPUT);
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////State Setup/////////////////////////////////////////
#define numC1 3
#define numC2 6
#define numC3 6
State stateList[numC1 + numC2 + numC3];
State *C1S[numC1 + 1];
State *C2S[numC2 + 1];
State *C3S[numC3 + 1];
State *C[4];
void State_SETUP()
{
///Session 
	int i, j=0;
	for(i=1; i<=numC1; ++i)
		C1S[i]=stateList[j++].setCiSi(1,i); //pick "* State" one by one
	for(i=1; i<=numC2; ++i)
		C2S[i]=stateList[j++].setCiSi(2,i); //pick "* State" one by one
	for(i=1; i<=numC3; ++i)
		C3S[i]=stateList[j++].setCiSi(3,i); //pick "* State" one by one
	C1S[0] = C2S[0] = C3S[0] = C[0] = State::ENDSTATE;//End of Session
	C[1] = C1S[1]; C[2] = C2S[1]; C[3] = C3S[1]; //Entry of Component
	State::NEXTSTATE = C[1];//Entry of Session
///Component [1]:init licks
	///C1S[1]: Wait lick
	C1S[1]->evtListener = []()-> bool {return cpp_ListenAI(IN2, HIGH);}; //IN2 -> S2
	C1S[1]->evtListenerSTATE = []()-> State* {int n=2; return C1S[n];};
	C1S[1]->addlisten();
	///C1S[2]: Water
	C1S[2]->dofun = []()-> void {cpp_keepon(OUT6, 0.065);}; //OUT6 0.065s
	C1S[2]->CountSet = []()-> int {return 4;}; //n=4 -> S3
	C1S[2]->CountSetSTATE = []()-> State* {int n=3; return C1S[n];};
	C1S[2]->TimerSet = []()-> float {return 2;}; //t=2 -> S1
	C1S[2]->TimerSetSTATE = []()-> State* {int n=1; return C1S[n];};
	///C1S[3]: 
	C1S[3]->TimerSet = []()-> float {return 1;}; //t=1 -> C2/C3
	C1S[3]->TimerSetSTATE = []()-> State* {int n=cpp_Range_int(2,3); return C[n];};

///Component [2]:Go trial
	///C2S[1]: time interval
	C2S[1]->TimerSet = []()-> float {return cpp_Range_float(4,6);}; //t=4~6 -> S2
	C2S[1]->TimerSetSTATE = []()-> State* {int n=2; return C2S[n];};
	///C2S[2]: Cue
	C2S[2]->dofun = []()-> void {cpp_keepon(OUT4, 0.5);}; //OUT4 0.5sec
	C2S[2]->CountSet = []()-> int {return 1;}; //->S3
	C2S[2]->CountSetSTATE = []()-> State* {int n=3; return C2S[n];};
	///C2S[3]: wait lick
	C2S[3]->TimerSet = []()-> float {return 2;}; //t=2 -> S5
	C2S[3]->TimerSetSTATE = []()-> State* {int n=5; return C2S[n];};
	C2S[3]->evtListener = []()-> bool {return cpp_ListenAI(IN2, HIGH);}; //IN2 -> S4
	C2S[3]->evtListenerSTATE = []()-> State* {int n=4; return C2S[n];};
	C2S[3]->addlisten();
	///C2S[4]: Water
	C2S[4]->dofun = []()-> void {cpp_keepon(OUT6, 0.075);}; //OUT6 0.075sec
	C2S[4]->TimerSet = []()-> float {return 1.5;}; //t=1.5->S6
	C2S[4]->TimerSetSTATE = []()-> State* {int n=6; return C2S[n];};
	///C2S[5]: Miss
	C2S[5]->CountSet = []()-> int {return 1;}; //-> S6
	C2S[5]->CountSetSTATE = []()-> State* {int n=6; return C2S[n];};
	///C2S[6]: 
	C2S[6]->CountSet = []()-> int {return 101;}; //n=101 -> STOP
	C2S[6]->CountSetSTATE = []()-> State* {return C[0];};
	C2S[6]->TimerSet = []()-> float {return 0.1;}; //t=0.1->C2/C3
	C2S[6]->TimerSetSTATE = []()-> State* {int n=cpp_GoElse(50,2,3); return C[n];};

///Component [3]:NoGo trial
	///C3S[1]: time interval
	C3S[1]->TimerSet = []()-> float {return cpp_Range_float(4,6);}; //t=4~6 -> S2
	C3S[1]->TimerSetSTATE = []()-> State* {int n=2; return C3S[n];};
	///C3S[2]: Cue
	C3S[2]->dofun = []()-> void {cpp_keepon(OUT2, 0.5);}; //OUT2 0.5sec
	C3S[2]->CountSet = []()-> int {return 1;}; //->S3
	C3S[2]->CountSetSTATE = []()-> State* {int n=3; return C3S[n];};
	///C3S[3]: wait lick
	C3S[3]->TimerSet = []()-> float {return 2;}; //t=2 -> S5
	C3S[3]->TimerSetSTATE = []()-> State* {int n=5; return C3S[n];};
	C3S[3]->evtListener = []()-> bool {return cpp_ListenAI(IN2, HIGH);}; //IN2 -> S4
	C3S[3]->evtListenerSTATE = []()-> State* {int n=4; return C3S[n];};
	C3S[3]->addlisten();
	///C3S[4]: Airpuff
	C3S[4]->dofun = []()-> void {cpp_keepon(OUT5, 0.5);}; //OUT5 0.5sec
	C3S[4]->TimerSet = []()-> float {return 1.5;}; //t=1.5->S6
	C3S[4]->TimerSetSTATE = []()-> State* {int n=6; return C3S[n];};
	///C3S[5]: Miss
	C3S[5]->CountSet = []()-> int {return 1;}; //-> S6
	C3S[5]->CountSetSTATE = []()-> State* {int n=6; return C3S[n];};
	///C3S[6]: 
	C3S[6]->TimerSet = []()-> float {return 0.1;}; //t=0.1->C2/C3
	C3S[6]->TimerSetSTATE = []()-> State* {int n=cpp_GoElse(50,2,3); return C[n];};

}
//finished ArControl task
