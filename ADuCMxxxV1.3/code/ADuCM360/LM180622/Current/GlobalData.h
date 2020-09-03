//							GLOBAL DEFINITIONS		
//doc:	https://docs.google.com/document/d/1EVXDJ8jrGSRPFItsheHzVzc0pflQFLABdPCY5DpSXss/edit
//#include "GlobalData.h"


#ifndef __GLOBALDATA__
#define __GLOBALDATA__

static const int kADCBuffSize= 100;			//kADCBuffSize/1953Hz = 51ms of data  ref180925
static const int t0count=1000;


int t1count;

typedef struct DoubleBufferType {
	int aABuff[kADCBuffSize];
	int aBBuff[kADCBuffSize];
} tDMAStruct;


typedef enum {kReset, kNext,kEnable,kDisable} eAction;	// Type used to enable and disable all the pins and to choose 
																												// the modality of the final state machine (reset state or next state)
typedef union {			//Flags for the operation modes
 int     all_flags[1];      				/* Allows us to refer to the flags 'en masse' */
 struct
 {
  uint32_t
		ENPLW					: 1,							/*Bit 0: Enable piecewise linear function */
		ENCHRGVCCS		: 1,     					/*Bit 1: Enable CHRGVCCS, Enable stimulation */
		ENSWITCHES		: 1,							/* Enable MUX Switches */
		HVPS_ON 				: 1,							/*3 Enable HVPS charging VPP*/
		DEBUGGING			: 1,							/*4: Debug									 */
		SINEGENERATOR	: 1,							/*5:	Output a sine, set to null by  STIMENABLE											*/
		spare6				: 1,							/* Enable PLS CTRL on channel 2 */
		spare7				: 1,							/*  */
		spare8 		: 1,							/* Increase or decrease aIAmp depending ALSO on RMSnew-RMSold */
		spare9		: 1,							/* Blanking enable (first 20 samples) */
		SR_FILTER			: 1,							/*Bit 10: Enable Slewrate filtering rather than IIR filter of RMS */
		ONECHSUM			: 1,							/* Two in channels, one out channel */
		SAVECOUNT			: 1,							/* ??? */
		RESETCOUNT		: 1,							/* ???*/
		spare17				: 1,							/* Unused */
		spare16				: 1,							/* Unused */
		spare15				: 1,							/* Unused */
		spare14				: 1,							/* Unused */
		spare13				: 1,							/* Unused */
		spare12				: 1,							/* Unused */
		spare11				: 1,							/*Bit 20: Unused */
		spare10				: 1,							/* Unused */
		SAVESETTINGS	: 1,							/* B22		Save current settings in FLASH */
		THREEBATTERIES	: 1,							/* Bit 23 Unused */
		BATTLOW				: 1,							/*Bit 24: Low battery level*/
		BATTFULL			: 1,							/*Bit 25: Battery is full */
		INVALIDSETUP	: 1,							/*Bit 26: Settings are invalid when read from FLASH */
		SHUTDOWN			: 1,							/*Bit 27: Shutting down the device								 	*/
		STIMENABLE		: 1,							/* Enable stimulation output or PAUSE      	*/
		KEY_DOWN			: 1,							/*Bit 29: Key is down */
		KEY_SHORTPRESS	: 1,						/*Bit 30: Short press on pushbutton 								*/
		KEY_LONGPRESS: 1;								/* Long press on pushbutton 								*/
	} bits;
} ModeType;


#endif  // __GLOBALDATA__
