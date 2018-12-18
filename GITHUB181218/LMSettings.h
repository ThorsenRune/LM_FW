/*  DOC: https://docs.google.com/document/d/18TuKtjrlLNaDMMJ283uIhfQmPMIR28pUscTYVev02uc 
		Local:C:\ADuCMxxxV1.3\code\ADuCM360\LM180622\Current\LMSettings.h

*/

#ifndef __LMSETTINGS__
#define __LMSETTINGS__
//			SYSTEM AND SIGNAL PROCESSING CONSTANTS
// Rev0. 180927
static const int kStimCycleInterval=60;						// Stimulation period in milliseconds	  R16101218

static const int kPowerWatchDogTimeOut=5*60*1000/kStimCycleInterval;		//5 min (5min*60sec/min*16cycl/sec) Number of cycles before entering power down
static const int kCommunicationTimeOut=20*1000/kStimCycleInterval;			//20 seconds before a communication timeout is triggered

/*			HARDWARE CONSTANTS			*/
static const float kADC2nVFactor=0.0125;			//Scale MES in nanoVolt (including the normalization by kADCBuffSize) 180926 empirically found to be 0.0125
static const int mAFactor	=1000;			//Use microamps as baseunit
//Battery low limit in mV
static const int kCellEmptyLevel= 900;			//At 900 mV the battery are near death
static const int kCellLowLevel  = 1100;			// 1.1V is nearly discharged

/*			DSP FILTER PARAMETERS */
static const float kGainScale=1.0/1000;					//gain multiplier 100.000nV -> 10.000uA * gain in %
static const int nRMS_SlewRateLimit=100000*kADC2nVFactor;	// slewrate of 10uV/sec
static const int alfa[2]={0.02 *q31One, 0.02 *q31One};									//alpha coefficent for IIR lp Filter





/*				CALCULATED CONSTANTS   */
//static const int kMainLoopIntervalInSystemTicks=960000;				//=60mS*16MHz    Number of clockcycles in the mainloop (stimulation) period. 
static const int kMainLoopIntervalInSystemTicks=kStimCycleInterval*16000;				//=60mS*16MHz    Number of clockcycles in the mainloop (stimulation) period. 


#endif  // __LMSETTINGS__
