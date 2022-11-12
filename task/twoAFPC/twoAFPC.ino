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
		"@OUT1:#Unrewarded cue\n" \
		"@OUT2:#Go cue\n" \
		"@OUT3:#LeftReward\n" \
		"@OUT4:#RightReward\n" \
		"@OUT5:#5\n" \
		"@OUT6:#6\n" \
		"@OUT7:#7\n" \
		"@OUT8:#8\n" \
		"@IN1:#LeftLick\n" \
		"@IN2:#MidLick\n" \
		"@IN3:#RightLick\n" \
		"@IN4:#4\n" \
		"@IN5:#5\n" \
		"@IN6:#6\n" \
		"@C1:\n" \
		"@C1S1:init Vars\n" \
		"@C2:Left Block\n" \
		"@C2S1:Mid_lick\n" \
		"@C2S2:Choose Side\n" \
		"@C2S3:RWD\n" \
		"@C2S4:If Switch\n" \
		"@C2S5:TimeOut\n" \
		"@C2S6:ERR\n" \
		"@C2S7:MISS\n" \
		"@C3:Right Block\n" \
		"@C3S1:Mid_lick\n" \
		"@C3S2:Choose Side\n" \
		"@C3S3:RWD\n" \
		"@C3S4:If Switch\n" \
		"@C3S5:TimeOut\n" \
		"@C3S6:ERR\n" \
		"@C3S7:MISS\n" \
		""

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////global vars/////////////////////////////////////////
double Var_reward = 0;
double Var_Block = 0;
double Var_pumpL = 0;
double Var_pumpR = 0;
double Var_rewardNN = 0;
double Var_P_left = 0;
double Var_P_right = 0;
double Var_rewardN = 0;
double Var_BLen = 0;
double Var_missN = 0;
double Var_missC = 0;

///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Hardware Setup///////////////////////////////////////
const int OUT1 = 2; 	//#Unrewarded cue
const int OUT2 = 3; 	//#Go cue
const int OUT3 = 4; 	//#LeftReward
const int OUT4 = 5; 	//#RightReward
const int OUT5 = 6; 	//#5
const int OUT6 = 7; 	//#6
const int OUT7 = 8; 	//#7
const int OUT8 = 9; 	//#8
const int  IN1 = 0; 	//#LeftLick
const int  IN2 = 1; 	//#MidLick
const int  IN3 = 2; 	//#RightLick
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
#define numC1 1
#define numC2 7
#define numC3 7
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
///Component [1]:
	///C1S[1]: init Vars
	C1S[1]->dovar = []()-> void{
		//Write your Arduino C++ code below
		
		
		
		
		
		
		
		
		
		Var_P_right=75;
		
		
		
		
		
		
		
		
		
		Var_P_left=75;
		
		
		
		
		
		
		
		
		
		Var_pumpL=0.085;
		
		
		
		
		
		
		
		
		
		Var_pumpR=0.045;
		
		
		
		
		
		
		
		
		
		Var_BLen=cpp_Range_int(7, 14);
		
		
		
		
		
		
		
		
		
		Var_rewardNN=0;
		
		
		
		
		
		
		
		
		
		Var_Block = 'N';
	};
	C1S[1]->CountSet = []()-> int {return 1;}; //-> C2/C3
	C1S[1]->CountSetSTATE = []()-> State* {int n=cpp_GoElse(50,2,3); return C[n];};

///Component [2]:Left Block
	///C2S[1]: Mid_lick
	C2S[1]->dovar = []()-> void{
		//Write your Arduino C++ code below
		
		
		
		
		Var_reward = 0;
		
		
		
		
		
		
		
		
		
		if (Var_missN>=2) {
		
		
		
		
			Var_missC = 3;
		
		
		
		
		}
		
		
		
		
		else {
		
		
		
		
			Var_missC = 7;
		
		
		
		
		}
	};
	C2S[1]->varListener = []()-> bool {
		//Write your Arduino C++ code below
		
		
		
		
		boolean ShouldgotoState=false;
		
		
		
		
		return Var_rewardNN>=200;;
	}; //rwd=200 -> Stop
	C2S[1]->varListenerSTATE = []()-> State* {return C[0];};
	C2S[1]->TimerSet = []()-> float {return 25;}; //t=25 -> S2
	C2S[1]->TimerSetSTATE = []()-> State* {int n=2; return C2S[n];};
	C2S[1]->evtListener = []()-> bool {return cpp_ListenAI(IN2, HIGH);}; //M_lick -> S2
	C2S[1]->evtListenerSTATE = []()-> State* {int n=2; return C2S[n];};
	C2S[1]->addlisten();
	///C2S[2]: Choose Side
	C2S[2]->dofun = []()-> void {cpp_keepon(OUT2, 0.5);}; //GoCue 0.5s
	C2S[2]->evtListener = []()-> bool {return cpp_ListenAI(IN3, HIGH);}; //R_lick -> S6
	C2S[2]->evtListenerSTATE = []()-> State* {int n=6; return C2S[n];};
	C2S[2]->addlisten();
	C2S[2]->evtListener = []()-> bool {return cpp_ListenAI(IN1, HIGH);}; //L_Lick -> S3/S7
	C2S[2]->evtListenerSTATE = []()-> State* {int n=cpp_GoElse(Var_P_left,3,Var_missC); return C2S[n];};
	C2S[2]->addlisten();
	///C2S[3]: RWD
	C2S[3]->dovar = []()-> void{
		//Write your Arduino C++ code below
		
		
		
		
		Var_reward = 1;
		
		
		
		
		Var_missN = 0;
		
		
		
		
		Var_rewardN++;
		
		
		
		
		Var_rewardNN++;
		
		
		
		
		
		
		
		
		
		if(Var_Block!='L'){ //Block first trial
		
		
		
		
			//cpp_keepon(OUT3, Var_pumpL*0.66); //left pump
		
		
		
		
			Var_Block = 'L';
		
		
		
		
		}
	};
	C2S[3]->dofun = []()-> void {cpp_keepon(OUT3, Var_pumpL);}; //Left_RWD on
	C2S[3]->CountSet = []()-> int {return 1;}; //-> S4
	C2S[3]->CountSetSTATE = []()-> State* {int n=4; return C2S[n];};
	///C2S[4]: If Switch
	C2S[4]->varListener = []()-> bool {
		//Write your Arduino C++ code below
		
		
		
		
		boolean ShouldgotoState=false;
		
		
		
		
		
		
		
		
		
		if (Var_reward==1 && Var_rewardN>=Var_BLen) {
		
		
		
		
			Var_rewardN=0;
		
		
		
		
			Var_missN=1;
		
		
		
		
			Var_BLen=cpp_Range_int(7, 14);
		
		
		
		
			ShouldgotoState = true;
		
		
		
		
		}
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		return ShouldgotoState;;
	}; //switch -> C3
	C2S[4]->varListenerSTATE = []()-> State* {int n=3; return C[n];};
	C2S[4]->TimerSet = []()-> float {return 1.5;}; //t= 1.5 -> S1
	C2S[4]->TimerSetSTATE = []()-> State* {int n=1; return C2S[n];};
	///C2S[5]: TimeOut
	C2S[5]->dofun = []()-> void {cpp_keepon(OUT1, 3);}; //OUT1 3s
	C2S[5]->CountSet = []()-> int {return 1;}; //-> S1
	C2S[5]->CountSetSTATE = []()-> State* {int n=1; return C2S[n];};
	///C2S[6]: ERR
	C2S[6]->CountSet = []()-> int {return 1;}; //-> S5
	C2S[6]->CountSetSTATE = []()-> State* {int n=5; return C2S[n];};
	///C2S[7]: MISS
	C2S[7]->dovar = []()-> void{
		//Write your Arduino C++ code below
		
		
		
		
		
		
		
		
		
		++Var_missN;
	};
	C2S[7]->CountSet = []()-> int {return 1;}; //-> S5
	C2S[7]->CountSetSTATE = []()-> State* {int n=5; return C2S[n];};

///Component [3]:Right Block
	///C3S[1]: Mid_lick
	C3S[1]->dovar = []()-> void{
		//Write your Arduino C++ code below
		
		
		
		
		Var_reward = 0;
		
		
		
		
		
		
		
		
		
		if (Var_missN>=2) {
		
		
		
		
			Var_missC = 3;
		
		
		
		
		}
		
		
		
		
		else {
		
		
		
		
			Var_missC = 7;
		
		
		
		
		}
	};
	C3S[1]->varListener = []()-> bool {
		//Write your Arduino C++ code below
		
		
		
		
		boolean ShouldgotoState=false;
		
		
		
		
		return Var_rewardNN>=200;;
	}; //rwd=200 -> Stop
	C3S[1]->varListenerSTATE = []()-> State* {return C[0];};
	C3S[1]->TimerSet = []()-> float {return 25;}; //t=25 -> S2
	C3S[1]->TimerSetSTATE = []()-> State* {int n=2; return C3S[n];};
	C3S[1]->evtListener = []()-> bool {return cpp_ListenAI_edge(IN2, LOW);}; //M_lick -> S2
	C3S[1]->evtListenerSTATE = []()-> State* {int n=2; return C3S[n];};
	C3S[1]->addlisten();
	///C3S[2]: Choose Side
	C3S[2]->dofun = []()-> void {cpp_keepon(OUT2, 0.5);}; //GoCue 0.5s
	C3S[2]->evtListener = []()-> bool {return cpp_ListenAI_edge(IN1, LOW);}; //L_lick -> S6
	C3S[2]->evtListenerSTATE = []()-> State* {int n=6; return C3S[n];};
	C3S[2]->addlisten();
	C3S[2]->evtListener = []()-> bool {return cpp_ListenAI_edge(IN3, LOW);}; //R_Lick -> S3/S7
	C3S[2]->evtListenerSTATE = []()-> State* {int n=cpp_GoElse(Var_P_right,3,Var_missC); return C3S[n];};
	C3S[2]->addlisten();
	///C3S[3]: RWD
	C3S[3]->dovar = []()-> void{
		//Write your Arduino C++ code below
		
		
		
		
		Var_reward = 1;
		
		
		
		
		Var_missN = 0;
		
		
		
		
		Var_rewardN++;
		
		
		
		
		Var_rewardNN++;
		
		
		
		
		
		
		
		
		
		if(Var_Block!='R'){ //Block first trial
		
		
		
		
			//cpp_keepon(OUT4, Var_pumpR*0.66); //Right pump
		
		
		
		
			Var_Block = 'R';
		
		
		
		
		}
	};
	C3S[3]->dofun = []()-> void {cpp_keepon(OUT3, Var_pumpL);}; //Left_RWD on
	C3S[3]->CountSet = []()-> int {return 1;}; //-> S4
	C3S[3]->CountSetSTATE = []()-> State* {int n=4; return C3S[n];};
	///C3S[4]: If Switch
	C3S[4]->varListener = []()-> bool {
		//Write your Arduino C++ code below
		
		
		
		
		boolean ShouldgotoState=false;
		
		
		
		
		if (Var_reward==1 && Var_rewardN>=Var_BLen) {
		
		
		
		
			Var_rewardN=0;
		
		
		
		
		  Var_BLen=cpp_Range_int(7, 14);
		
		
		
		
			Var_missN=1;
		
		
		
		
			ShouldgotoState = true;
		
		
		
		
		}
		
		
		
		
		return ShouldgotoState;;
	}; //switch -> C3
	C3S[4]->varListenerSTATE = []()-> State* {int n=2; return C[n];};
	C3S[4]->TimerSet = []()-> float {return 1.5;}; //t= 1.5 -> S1
	C3S[4]->TimerSetSTATE = []()-> State* {int n=1; return C3S[n];};
	///C3S[5]: TimeOut
	C3S[5]->dofun = []()-> void {cpp_keepon(OUT1, 3);}; //OUT1 3s
	C3S[5]->CountSet = []()-> int {return 1;}; //-> S1
	C3S[5]->CountSetSTATE = []()-> State* {int n=1; return C3S[n];};
	///C3S[6]: ERR
	C3S[6]->CountSet = []()-> int {return 1;}; //-> S5
	C3S[6]->CountSetSTATE = []()-> State* {int n=5; return C3S[n];};
	///C3S[7]: MISS
	C3S[7]->dovar = []()-> void{
		//Write your Arduino C++ code below
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		++Var_missN;
	};
	C3S[7]->CountSet = []()-> int {return 1;}; //-> S5
	C3S[7]->CountSetSTATE = []()-> State* {int n=5; return C3S[n];};

}
//finished ArControl task
