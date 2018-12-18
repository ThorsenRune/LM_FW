/**MAIN.C	 180803	link:FW.doc
  C:\ADuCMxxxV1.3\code\ADuCM360\LM180622\Current\main.c
	doc: https://docs.google.com/document/d/1SjhUHHc4YTPwqucyKHSBKvjZWQEx2QSfKWNZfM7m9-M/edit
 *****************************************************************************
Firmware for the LiberMano hardware
Version 161012	under development mapping from development kit to Hardware version 1
Version 161018	revisions for LM_HARDWARE V1.0.2
**/


//		STANDARD 				LIBRARIES 			

//#include <stdio.h>				not needed R161027
//#include <string.h>				not needed R161027
#include <aducm360.h>
#include <stdlib.h>
#include <AdcLib.h>					// Just add ..\..\common\ to Project options-c/c++ - include paths
#include <DacLib.h>
#include <UrtLib.h>
#include <ClkLib.h>
#include <WutLib.h>
#include <WdtLib.h>
#include <GptLib.h>
#include <I2cLib.h>
#include <IntLib.h>
#include <PwmLib.h>
#include <PwrLib.h>
#include <DioLib.h>
#include <DmaLib.h>					// 161020			
#include <SpiLib.h>


#define ARM_MATH_CM3 
 
//******** LIBERMANO DECLARATIONS ********
//						Global datastructures (" means in same folder as this file)
#include "main.h"						// General global declarations	
//			****	PARAMETERS		SIGNAL PROCESSIING AND SYSTEM SETTINGS 
#include "LMSettings.h"			
#include "GlobalData.c"			// Public data
#include "mDebug.c"					// Debugging methods
#include "cprototyping.h"
#include "mSysTick.c"				// System timer module
#include "system.c"					// Lowlevel system management
#include "SysInit.c"				// Initialization of system and peripheral circuits
#include "cProtocol.c"			// Communication with host
#include "mDataExch.c"			// Data interface to host
#include "VCCS.c"						// Methods related to Hardware VCCS
#include "INA.c"						// Sampling of MES control of instrumentation amplifiers
#include "mSubs.c"					// Confirmed methods
#include "mErrFlags.h"			// Error messages
#include "CalcLib.c"				// Library for calculations / Math	e.g. RMS SQRT etc
#include "mInts.c"					// Interrupt Service Routines
#include "LVPS.c"						// Low volt. power supply. Motherboard and on/off 
#include "mProts.c"					// Prototyping of methods
#include "DSP.c"						// Digital signal processing. Filtering MES and calculating IAmp
#include "mFlash.c"					// Write, read and erase flash memory
#include "cPrototying.c"		//Prototying methods here

int main (void)
{	
	MainSetup();		//Setup the system, R180809
	mSetLedColor(kLed1On);	//Show the we are alive
	mWaitMs(1*1000);		//Give time for releasing the button 10sec											//   wait for powering up bluetooth before talking to it (Note180809)
	ReadSavedParameters();									// Reads parameters saved on flash memory
	if (nMode.bits.INVALIDSETUP) mFirstTimeSetup();
	mSetSystemValues();	
	mSendVersionInfo();

	//Set hardware dependent values
	//mBluetoothDeepSleep();	//May reduce 2 mA	
 
/******************************* MAIN WHILE LOOP *******************************/
	while (1)
  {
		mWaitCycleStart();							// 1.  Wait for a block start using system clocks
		mADCAux_Start();				//Todo1 maybe this corrupts EMG?
		if ( nMode.bits.SINEGENERATOR){
		  mOutputSineWave();
			mWaitMs(3);				//Wait for AUX sampling to finish
			mADCRestart(bFlip);							
		}else 	{
			mOutputStimFSM_Debug(kReset);					//2. Reset statemachine for stimulaion and signals timing
		}
		mSignalProcessing();					//3. Signal Processing
		mSystemActions();						//4.System management					
		mCommunication();						//5.  Data transmission									// R16123	
/******************************* END MAIN WHILE(1) *******************************/
	}
}
