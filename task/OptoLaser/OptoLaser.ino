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
#include "C:/Users/666/Documents/ArControl-release/ino/ArControl_AllinOne.h"

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////task info///////////////////////////////////////////
#define INFO "" \
		"@OUT1:#Laser\n" \
		"@OUT2:#2\n" \
		"@OUT3:#3\n" \
		"@OUT4:#4\n" \
		"@OUT5:#5\n" \
		"@OUT6:#6\n" \
		"@OUT7:#7\n" \
		"@OUT8:#8\n" \
		"@IN1:#1\n" \
		"@IN2:#2\n" \
		"@IN3:#3\n" \
		"@IN4:#4\n" \
		"@IN5:#5\n" \
		"@IN6:#6\n" \
		"@C1:Optogenetic Laser Pattern\n" \
		"@C1S1:Active: 20Hz-5ms 1min\n" \
		"@C1S2:Resting: 4-5min\n" \
		""

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////global vars/////////////////////////////////////////
double Var_1 = -2;
double Var_2 = 0;
double Var_3 = 1;

///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Hardware Setup///////////////////////////////////////
const int OUT1 = 2; 	//#Laser
const int OUT2 = 3; 	//#2
const int OUT3 = 4; 	//#3
const int OUT4 = 5; 	//#4
const int OUT5 = 6; 	//#5
const int OUT6 = 7; 	//#6
const int OUT7 = 8; 	//#7
const int OUT8 = 9; 	//#8
const int  IN1 = 0; 	//#1
const int  IN2 = 1; 	//#2
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
#define numC1 2
State stateList[numC1];
State *C1S[numC1 + 1];
State *C[2];
void State_SETUP()
{
///Session 
	int i, j=0;
	for(i=1; i<=numC1; ++i)
		C1S[i]=stateList[j++].setCiSi(1,i); //pick "* State" one by one
	C1S[0] = C[0] = State::ENDSTATE;//End of Session
	C[1] = C1S[1]; //Entry of Component
	State::NEXTSTATE = C[1];//Entry of Session
///Component [1]:Optogenetic Laser Pattern
	///C1S[1]: Active: 20Hz-5ms 1min
	C1S[1]->dofun = []()-> void {cpp_Hz_A(OUT1, 60, 20, 10);}; //blink (20Hz-5ms 1min)
	C1S[1]->CountSet = []()-> int {return 1;}; //->S2
	C1S[1]->CountSetSTATE = []()-> State* {int n=2; return C1S[n];};
	///C1S[2]: Resting: 4-5min
	C1S[2]->CountSet = []()-> int {return 5;}; //n=5 -> STOP
	C1S[2]->CountSetSTATE = []()-> State* {return C[0];};
	C1S[2]->TimerSet = []()-> float {return cpp_Range_float(240,360);}; //t=4-6min -> S1
	C1S[2]->TimerSetSTATE = []()-> State* {int n=1; return C1S[n];};

}
//finished ArControl task
