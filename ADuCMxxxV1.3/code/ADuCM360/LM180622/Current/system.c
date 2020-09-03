
//	Description:HAL (Hardware Abstraction Layer)	LM hardware system specific low level methods
//	file:C:\ADuCMxxxV1.3\code\ADuCM360\LM180622\Current\system.c
//	doc:https://docs.google.com/document/d/1FhiNYHcy7hFc5mdThXESQLGEvH3yhldN5Snx2tx6K-g 		


//R161102
#include "System.h"
#include "GlobalData.h"
#include "CalcLib.h"
#include "mDebug.h"
//	HW LED connection
#define LED_GPIO pADI_GP0
#define LED_PIN 4
 
//	HW PushButton connection
#define PIN_PB_GPIO pADI_GP0
#define PIN_PB 5
extern int mGPIO_Direction(ADI_GPIO_TypeDef *pPort, int nPinNr,int direction);
extern int mGetStimQuartile(int nChannel);
extern void SaveParametersOnFlash(void);
eLedMode nLedState;
int nShutDownTimer=0;				//Timer for shutting down
int nPowerWatchDog=kPowerWatchDogTimeOut;

/*--------------------------------------------------------------*/
int mDioGet(ADI_GPIO_TypeDef *pPort, int bitname)//bitname:eg BIT3
	{
		int nBitPattern;
		nBitPattern=DioRd(pPort);
		return (nBitPattern&bitname);
	}

//Pushbutton managemnet
int isButtonPushed(void){
	volatile int ret;
		mGPIO_Direction(LED_GPIO,PIN_PB,1);	//Pushbutton as input
 	  ret=mDioGet(PIN_PB_GPIO,1<<PIN_PB);
		return (ret);	//Pushed because high
}
int nPushButton[3]={0,1,16};//Counter for the pushbutton, short timeout , long timeout
void mPushButtonRead(void){//Obsoleting  mPBActions
	nMode.bits.KEY_SHORTPRESS=0;		//Assume no press
	nMode.bits.KEY_LONGPRESS=0;
	nMode.bits.KEY_DOWN	=(	isButtonPushed()!=0x0			);
	if (!nMode.bits.KEY_DOWN){			//NO push, counting down
		if (nPushButton[0]>0){	//Release Action
			nPushButton[0]=nPushButton[0]-1;
			if (nPushButton[0]==nPushButton[1]) {		
				nMode.bits.KEY_SHORTPRESS=1;	
				nPushButton[0]=0;
			} else if (nPushButton[0]==nPushButton[2]){			//2 second is a long press
//				nMode.bits.KEY_LONGPRESS=1;	
				nPushButton[0]=0;					//Accept the action
			}
		} else {
				nMode.bits.KEY_SHORTPRESS=0;
				nMode.bits.KEY_LONGPRESS=0;					
		}
	}else{ //Pressed, start counting
 
		if (nPushButton[0]<=nPushButton[2])
			nPushButton[0]=nPushButton[0]+1;
			if (nPushButton[0]==nPushButton[2]){			//2 second is a long press
				nMode.bits.KEY_LONGPRESS=1;	
			}
	}
}

void mPBActions(){		//Actions to take on pushbutton activation
	if (nMode.bits.KEY_SHORTPRESS){
			nMode.bits.STIMENABLE=!nMode.bits.STIMENABLE;//Toggle pause/stimulation
			nMode.bits.HVPS_ON=nMode.bits.STIMENABLE;	
			bErrFlags.all_flags[0] = 0U;											// Clear all flags
		//R181023 Idea: cancel shutdown on keypress   (todo1:test)
			if (nMode.bits.SHUTDOWN)
				nMode.bits.SHUTDOWN=0;
	} else if (nMode.bits.KEY_LONGPRESS){
			if (nMode.bits.STIMENABLE) 
				nMode.bits.SAVESETTINGS=1; 
			nMode.bits.STIMENABLE=0;
			nMode.bits.SHUTDOWN=1;			//Start shutdown procedure
		
	}
}

int mGPIO_Direction(ADI_GPIO_TypeDef *pPort, int nPinNr,int direction)
	{//direction=1:inptut 0:output
	int bp=pPort->GPOEN;
	if (direction>0){
		bp=bp & ( ~(1<< nPinNr));	//Clear bit nPinNr
	}else {
		bp=bp | ((1<< nPinNr));	//SetClear bit nPinNr
	}
	pPort->GPOEN=bp;
	return bp;
	}

void mSetLedColor(	eLedMode nLedMode){
	switch (nLedMode){
/*		case kLed2On:		//Led pin Low
		{
			mGPIO_Direction(LED_GPIO,LED_PIN,0);	
			DioClr(LED_GPIO,1<<LED_PIN);											// Set led pin P1.3 as output and high (red)
//			mGPIO_Direction(LED_GPIO,PIN_PB,0);		//Set pushbutton as out		180817 the idea of pusbutton for led is  a badidea. Gives trouble
//			DioSet(PIN_PB_GPIO,1<<PIN_PB);	//Set pushbutton line high
			break;
		} */
		case kLed1On:    //Led pin high
		{
			mGPIO_Direction(LED_GPIO,LED_PIN,0);			
			DioSet(LED_GPIO,1<<LED_PIN);											// Set led pin P1.3 as output and low (green)
			break;
		}
		case kLed1Off:    //Red pin low
		{
			DioClr(LED_GPIO,1<<LED_PIN);	
			break;
		}
		default:
		{
			//	Set led pin as an input to switch off all the leds		
			mGPIO_Direction(LED_GPIO,LED_PIN,1);			
  	   DioPul(LED_GPIO,0);								              // Disable all pullup on P0.7/0.6
			 DioOce(LED_GPIO,0);

			break;
		}
	}
}
//	LED BLINK MANAGEMENT
int nLedCntr=0;			//Blink counter
int kFastBlinkSpeed=4;
int kSlowBlinkSpeed=32;
int kBlinkOnDuration=1;
eLedMode nLedMode=kLed1On;
void mSetLed180726(){
	kFastBlinkSpeed=(4-mGetStimQuartile(0));
	//Description: Sets the led see MCU in Hardware
	
	if (nMode.bits.SHUTDOWN){								//Shutting down
		if (nLedMode==kLed1On)
			nLedMode=kLed1Off;
		else
			nLedMode=kLed1On;
	} else 	if (nMode.bits.STIMENABLE){			//Stimulation active
		nLedMode=kLed1BlinkFast;
	} else {																	//Pause
		nLedMode=kLed1BlinkSlow;
	}
	/* blink the led while keypress but turns out just to be frustrating
	if (nMode.bits.KEY_DOWN){
		nLedCntr=nLedCntr+1;		
		if (nLedCntr&1)
			mSetLedColor(kLed1On);//Show that we are pressing the button
		else
			mSetLedColor(kLed1Off);//Show that we are pressing the button
	} else 
	*/
	if (nLedMode==kLed1On){									//Constant light
			mSetLedColor(kLed1On);
	} else if (nLedMode==kLed1Off){					//Led off
			mSetLedColor(kLed1Off);
	} else if (nLedMode==kLed1BlinkFast){		//Fast blinking
			nLedCntr=nLedCntr+1;
			if (nLedCntr>(kFastBlinkSpeed+kBlinkOnDuration)) nLedCntr=0;
			else if (nLedCntr>kFastBlinkSpeed) mSetLedColor(kLed1On);
			else mSetLedColor(kLed1Off);
	}	else if (nLedMode==kLed1BlinkSlow)	{		//Slowblinking
			nLedCntr=nLedCntr+1;
			if (nLedCntr>(kSlowBlinkSpeed+kBlinkOnDuration)) nLedCntr=0;
			else if (nLedCntr>=kSlowBlinkSpeed) mSetLedColor(kLed1On);
			else mSetLedColor(kLed1Off);		
	}
}
//	HIGHVOLTAGE POWER SUPPLY = SMPS = HVPS generating VPP VNN
void mPS_ENA(eAction act){		//Master powersupply control
	if (act==kEnable){				//P0.0
		DioSet(pADI_GP0, BIT0);			//  
	}
	else  {
		DioClr(pADI_GP0, BIT0);			//  
	}
}


void mSystemCheck(){		//Check system status and initiate shutdown if needed
		if (nMode.bits.SHUTDOWN) 				mSystem_ShutDown();		//Shutting down, skip statemachine
		if (nMode.bits.DEBUGGING) return;  //Disabled in debug mode
		if (mPowerWatchDogTimeOut()>0)	mSystem_ShutDown();		//Watchdog timeout, start shutdown
		if (bErrFlags.errbits.BattLow)			mSystem_ShutDown();						//Battery low, start shutdown
		if (bErrFlags.errbits.bReceiveError)  nMode.bits.STIMENABLE=0;	//Stop stimulation on communication error
		if (nMode.bits.STIMENABLE) 
			nMode.bits.SINEGENERATOR=0;	//Turn off sinegenerator if stimulating R181107
		if (isCommTimeOut()) 
			bErrFlags.errbits.bReceiveError=1;			//Timeout on Bluetooth communication
}
void mPowerWatchDogReset(void){			//Reset the power off (idle) watchdog
	nPowerWatchDog=kPowerWatchDogTimeOut+kCommunicationTimeOut;
}
int isCommTimeOut(void) {				//Detects if more than kCommunicationTimeOut cycles since last communiation
	if (nPowerWatchDog==kPowerWatchDogTimeOut )		//Trip point
		return 1;
	else
		return 0;
}
int mPowerWatchDogTimeOut(void){		//Decrement the watchdog and return 1 on a timeout
	if (nPowerWatchDog<=0) return 1; 	//Timedout
	nPowerWatchDog=nPowerWatchDog-1;
	return 0;
}

//			*** SMPS AND VCCS SWITCHES

/**   MULTIPLEXER MANAGEMENT
 *****************************************************************************  
   @brief      Set MUX switches
   - kEN_SAINV	connect ELECTRODE B->A
   Ref //R16101823
 **/

void mSetMUX(int nMUXState){	
	if (nMode.bits.ENSWITCHES){
	switch (nMUXState)
  {		case kEN_SAPOS:			/*Pin	P1.0 */
		{    
			DioSet(pADI_GP1,kEN_SAPOS);
			DioClr(pADI_GP1,kEN_SAINV);
			DioClr(pADI_GP1,kEN_SBPOS);
			DioClr(pADI_GP1,kEN_SBINV);
			break;
		}
		case kEN_SAINV:			/*Pin	P1.1 */
		{
			DioClr(pADI_GP1,kEN_SAPOS);
			DioSet(pADI_GP1,kEN_SAINV);
			DioClr(pADI_GP1,kEN_SBPOS);
			DioClr(pADI_GP1,kEN_SBINV);
			break;
		}

		case kEN_SBPOS:			/*Pin	P0.4 */
		{    
			DioClr(pADI_GP1,kEN_SAPOS);
			DioClr(pADI_GP1,kEN_SAINV);
			DioSet(pADI_GP1,kEN_SBPOS);
			DioClr(pADI_GP1,kEN_SBINV);
			break;
		}
		case kEN_SBINV:			/*Pin	P1.2 */
		{    
			DioClr(pADI_GP1,kEN_SAPOS);
			DioClr(pADI_GP1,kEN_SAINV);
			DioClr(pADI_GP1,kEN_SBPOS);
			DioSet(pADI_GP1,kEN_SBINV);
			break;
		}
		case kEN_SxOPEN:	//OPEN ALL 
		{    
			DioClr(pADI_GP1,kEN_SAPOS);
			DioClr(pADI_GP1,kEN_SAINV);
			DioClr(pADI_GP1,kEN_SBPOS);
			DioClr(pADI_GP1,kEN_SBINV);
			break;
		}case kEN_SxCLOSE:	//CLOSE ALL 		
		{    
			DioSet(pADI_GP1,kEN_SAPOS);
			DioSet(pADI_GP1,kEN_SAINV);
			DioSet(pADI_GP1,kEN_SBPOS);
			DioSet(pADI_GP1,kEN_SBINV);
			break;
		}	}
	}
	else//OPEN ALL 
		{    
			DioClr(pADI_GP1,kEN_SAPOS);
			DioClr(pADI_GP1,kEN_SAINV);
			DioClr(pADI_GP1,kEN_SBPOS);
			DioClr(pADI_GP1,kEN_SBINV);
		return;
	}
}
 
 
//		Control the switch for charging the VCCS 
void 		mVCCS_Charge(int bEnable){//Rewrite of mSet_CHRGVCCS_DRIVE 
	//Description: Set HI or LO CHRGVCCS_DRIVE pin (P0.7)
	//act can be kEnable or kDisable
	if (bEnable && nMode.bits.ENCHRGVCCS){
		DioSet(pADI_GP2, BIT0);			// P2.0
	}
	else {
		DioClr(pADI_GP2, BIT0);			// P2.0
	}
}
void mHVPS_CHARGE (int act){//0 disable 1:enable photoflash charger HVPS
	//Description: Set HI or LO VPP_CHARGE pin (P2.0)
	//act can be kEnable or kDisable
	if (act&nMode.bits.HVPS_ON){
		DioSet(pADI_GP0, BIT3);			// (VPP_CHARGE) HIGH 
	}
	else {
		DioClr(pADI_GP0, BIT3);			//  (VPP_CHARGE) LOW
	}
}
/*SYSTEM MANAGEMENT*/

void mSystem_ShutDown(){			//Shut down the system, (save settings) and power off
	nMode.bits.SHUTDOWN=1;	//Make sure the shutdown mode is set
	if (	nMode.bits.SAVESETTINGS){
		nMode.bits.SAVESETTINGS=0;		//Clear the save request bit
		SaveParametersOnFlash();		//If the stimulation was active then save settings
	}
	mHVPS_CHARGE(0);		//Disable HVPS (LT3484)
	mVCCS_Charge(1);			//Charge the VCCS while delivering 5mA will discharge all caps
	mSetMUX(kEN_SxCLOSE);
	mWaitMs(10);				//Wait for the swiches to close.
	mDAC_StimOut(10*mAFactor);				//  discharging of the VCCS
	nShutDownTimer++;
	// see R181023
	if (nShutDownTimer>1*1000/kStimCycleInterval)
				mPS_ENA(kDisable);			// P0.0 (PS_ENA) HIGH 
}
