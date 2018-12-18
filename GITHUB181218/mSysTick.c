/* System tick timer module
		Read system clockcycles for timing function execution
*/


int TickTock(void);	//Returns clockcycles since last TickTock
void TickTockInit(void);		//Initialize system timer
		
		
// CORE COUNTER 
// Systick regs http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka8713.html
int *STCSR = (int *)0xE000E010;
int *STRVR = (int *)0xE000E014;
int *STCVR = (int *)0xE000E018;
volatile int nSysTick[2] ={0}; 
int dum;


void SysTimerInit(void){		//Returns clockcycles since last TickTock
	dum=*STCVR;
}


int SysTimer(void){					//Returns clockcycles since last TickTock
	return *STCVR;
}


void TickTockInit(void){
	// Configure Systick   
	*STRVR = 0xFFFFFF;  			// max count
	*STCVR = 0;        				// force a re-load of the counter value register
	*STCSR = 5;         			// enable FCLK count without interrupt
	TickTock();
	SysTimerInit();
}


int TickTock(void){					//Returns clockcycles since last TickTock
	nSysTick[0]=nSysTick[1];
	nSysTick[1]=*STCVR;
	return (nSysTick[0] - nSysTick[1] - 39);
}
//180726 UNCALIBRATED
void mWaitSysTick(volatile unsigned long systicks){					//Returns clockcycles since last TickTock
	volatile unsigned long nElapsedTime=0;
	volatile unsigned long nStart=*STCVR;//  STCVR is a down counter
	while (nElapsedTime<systicks){
		nElapsedTime=nStart-*STCVR;
	}
}

void mWaitMs(volatile int Duration){			//duration 1000 is approx 800us
	while(Duration>0){
		Duration=Duration-1;
		mWaitSysTick(16*1000);			
	}
}
void mWaitus(volatile int Duration){			//duration 1000 is approx 800us
	while(Duration>0){
		Duration=Duration-1;
		mWaitSysTick(16);			
	}
}
