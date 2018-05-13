///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////ArControl Style//////////////////////////////////////
// if echo task name
#define ECHO_TASKNAME   //echo this taskname(filename) in very begining
const char taskName[] = __FILE__;
// how to start ArControl
#define START_SOFT
// recording level
#include "E:/ArControl_ReportCollection/ArControl_github_realse/ino/ArControl.h"

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////task info///////////////////////////////////////////
#define INFO "" \
		"@OUT1:#Green\n" \
		"@OUT2:#Red\n" \
		"@OUT3:#Yellow\n" \
		"@OUT4:#4\n" \
		"@OUT5:#5\n" \
		"@OUT6:#6\n" \
		"@OUT7:#7\n" \
		"@OUT8:#8\n" \
		"@IN1:#LeftPoke\n" \
		"@IN2:#RightPoke\n" \
		"@IN3:#3\n" \
		"@IN4:#4\n" \
		"@IN5:#5\n" \
		"@IN6:#6\n" \
		"@C1:Light3 (loop=10)\n" \
		"@C1S1:Swither\n" \
		"@C1S2:Green 3sec\n" \
		"@C1S3:Red 2sec\n" \
		"@C1S4:Yellow 4sec\n" \
		""

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////global vars/////////////////////////////////////////
double Var_1 = -2;
double Var_2 = 0;
double Var_3 = 1;

///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Hardware Setup///////////////////////////////////////
const int OUT1 = 2; 	//#Green
const int OUT2 = 3; 	//#Red
const int OUT3 = 4; 	//#Yellow
const int OUT4 = 5; 	//#4
const int OUT5 = 6; 	//#5
const int OUT6 = 7; 	//#6
const int OUT7 = 8; 	//#7
const int OUT8 = 9; 	//#8
const int  IN1 = 0; 	//#LeftPoke
const int  IN2 = 1; 	//#RightPoke
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
#define numC1 4
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
///Component [1]:Light3 (loop=10)
	///C1S[1]: Swither
	C1S[1]->CountSet = []()-> int {return 10;}; //n=10 -> STOP
	C1S[1]->CountSetSTATE = []()-> State* {int n=0; return C1S[n];};
	C1S[1]->TimerSet = []()-> float {return 3;}; //t=3sec -> S3
	C1S[1]->TimerSetSTATE = []()-> State* {int n=3; return C1S[n];};
	C1S[1]->evtListener = []()-> bool {return cpp_ListenAI(IN1, HIGH);}; //leftPoke -> S2
	C1S[1]->evtListenerSTATE = []()-> State* {int n=2; return C1S[n];};
	C1S[1]->addlisten();
	C1S[1]->evtListener = []()-> bool {return cpp_ListenAI(IN2, HIGH);}; //rightPoke-> S4
	C1S[1]->evtListenerSTATE = []()-> State* {int n=4; return C1S[n];};
	C1S[1]->addlisten();
	///C1S[2]: Green 3sec
	C1S[2]->dofun = []()-> void {cpp_keepon(OUT1, 3);}; //Green 3sec
	C1S[2]->CountSet = []()-> int {return 1;}; //-> S1
	C1S[2]->CountSetSTATE = []()-> State* {int n=1; return C1S[n];};
	///C1S[3]: Red 2sec
	C1S[3]->dofun = []()-> void {cpp_keepon(OUT2, 2);}; //Red 2sec
	C1S[3]->CountSet = []()-> int {return 1;}; //->S1
	C1S[3]->CountSetSTATE = []()-> State* {int n=1; return C1S[n];};
	///C1S[4]: Yellow 4sec
	C1S[4]->dofun = []()-> void {cpp_keepon(OUT3, 4);}; //Yellow 4sec
	C1S[4]->CountSet = []()-> int {return 1;}; //-> S1
	C1S[4]->CountSetSTATE = []()-> State* {int n=1; return C1S[n];};

}
//finished ArControl task
