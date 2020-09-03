//	Description:			low level calls to peripherals
//	doc: 	
//	file:System.h				 
#ifndef __SYSTEM__
#define __SYSTEM__



/* READ FROM AD5592 ADC FUNCTIONS (to complete) */
typedef enum {kADC_read= 0x1000, kRead_IO2= 0x0004,kRead_IO5= 0x0020, kRead_IO6= 0x0040 } eADCAction;



void mPowerWatchDogReset(void);			//Reset the power off (idle) watchdog
int mPowerWatchDogTimeOut(void);		//Decrement the watchdog and return 1 on a timeout
void mDAC_StimOut(int nMicroAmp);
void mSystem_ShutDown(void);				//(Save) and Power down the system
int isCommTimeOut(void);						//Detects if more than kCommunicationTimeOut cycles since last communiation
typedef enum {kLed1Off, kLed1On,kLed1BlinkSlow , kLed1BlinkFast} eLedMode;

/*	Bit definitions for MULTIPLEXER PART OF VCCS see //R16101823	*/
#define kEN_SAPOS 0x08			// Pin	P1.3
#define kEN_SAINV 0x04			// Pin	P1.2 VCCS from SA2 to SA1
#define kEN_SBPOS 0x02			// Pin	P1.1 obs different port
#define kEN_SBINV 0x01			// Pin	P1.0
#define kEN_SxOPEN 0x00			// leave open
#define kEN_SxCLOSE 0x0F			// Close all


#endif  // __SYSTEM__
