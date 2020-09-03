/*	VCCS and MUX control		revision 180801*/
//file:VCCS.C
//doc: https://docs.google.com/document/d/1ZBJTd9OM-Xu6Mw-oBpODHEHECTn4vxQP3YX1asOernU/edit

#include "VCCS.h"
#include <DioLib.h>
#include "mSubs.h"
int nTestVal1=6000;
int nTestVal2=3000;
extern void AD5592_Write (int);
extern void mADCRestart(uint8_t Alt);
extern void mADCAux_Start(void );

extern int32_t aIAmp[2];
state nState;





int DAC_IGain =3000;
int DAC_IOffset=0;
void mDAC_StimOut(int nMicroAmp){
	uint32_t Value;
	if (nMode.bits.STIMENABLE){
		Value=nMicroAmp;
    Value=nMicroAmp*DAC_IGain;			// Transformation from uA (0-100000uA) to 12bit value to convert !!!TODO
		if (Value>DAC_IOffset)					//Avoid overflow
				Value=Value-DAC_IOffset;
		else if(DAC_IOffset<0)
				Value=Value-DAC_IOffset;
		DacWr(0,Value);									//Output bitvalue on DAC
	}
}


//			Final State Machine 
void mOutputStimFSM(eAction act){ //last working version, creates artefacts
	// Description: Final State Machine stimulation function (Note180801)
	// Param:       act (kReset or kNext)      
	// +161125	To be invoked in an interrupt with kNext
	if (nMode.bits.SHUTDOWN) return;		//Shutting down, skip statemachine
	if(act==kReset){																// Reset Final State Machine
		mStartTimer1(aTimerReloadValues[kT0]);					//+161125					// Start the timer from param
		nState=kFSMState0;
		return;
	}
	else { 
		switch(nState){
			case kFSMState0:            //Prepare stimulation enabling swithches who has a delay of some us
			{
				mVCCS_Charge(0);						//FLOAT VCCS
				mSetMUX(kEN_SAPOS);                       	// First channel, positive
				mDAC_StimOut(0);				//No output
				nState=kFSMState1;
				GptLd(pADI_TM1,aTimerReloadValues[kT1]);									// reload value fot next timer
				break;
			}
			case kFSMState1:            // Positive pulse Ch  A
			{
				mSetMUX(kEN_SAPOS);          	// Keep A channel, positive
				mDAC_StimOut(aIAmp[0]);				//Output pulse
				nState=kFSMState2;
				GptLd(pADI_TM1,aTimerReloadValues[kT2]);									// reload value fot next timer
				break;
			}
			case kFSMState2:				//Interpulse 												// t1
			{
				mSetMUX(kEN_SAINV);                       	//Prepare next  switch
				mDAC_StimOut(0);
				nState=kStimPhase3;
				GptLd(pADI_TM1,aTimerReloadValues[kT3]); 									// reload value fot next timer
				break;
			}
			case kStimPhase3:				// CHANNEL A INVERTED PULSE
			{
				mSetMUX(kEN_SAINV);                       	//Confirm switch
				mDAC_StimOut(aIAmp[0]);	
				nState=kStimPhase4;
				GptLd(pADI_TM1,aTimerReloadValues[kT4]);									// reload value fot next timer
				break;
			}
		
			case kStimPhase4:				//INTERCHANNEL INTERVAL									// t3
			{
				mDAC_StimOut(0);
				mSetMUX(kEN_SBPOS);                       	//Prepare next  switch
				nState=kStimPhase5;
				GptLd(pADI_TM1,aTimerReloadValues[kT5]);									// reload value fot next timer
				break;
			}
			case kStimPhase5:					//CHANNEL B Positive								// t4
			{
				mSetMUX(kEN_SBPOS);                       	
				mDAC_StimOut(aIAmp[1]);	
				nState=kStimPhase6;
				GptLd(pADI_TM1,aTimerReloadValues[kT6]);									// reload value fot next timer
				break;
			}
		
			case kStimPhase6:			//CH B Interpulseinterval
			{
				mDAC_StimOut(0);	
				mSetMUX(kEN_SBINV);                       	//Prepare next  switch
				nState=kStimPhase7;
				GptLd(pADI_TM1,aTimerReloadValues[kT7]);									// reload value fot next timer
				break;
			}
			case kStimPhase7:		//CH B INVERTED
			{
				mSetMUX(kEN_SBINV); 	
				mDAC_StimOut(aIAmp[1]);	
				nState=kStimPhase8;//Index 7
				GptLd(pADI_TM1,aTimerReloadValues[kT8]);//TIme to phase 9									// reload value fot next timer
				break;
			}
			case kStimPhase8:		//ENd of Stimulation train
			{
				mDAC_StimOut(0);	
				mSetMUX(kEN_SxOPEN);  
				nState=kStimPhase9;
				GptLd(pADI_TM1,aTimerReloadValues[kTDurChrge]);									// reload value fot next timer
				break;
			}	
			case kStimPhase9:		//End of Stimulation train
			{
						mAUX_PIN(1);
				mVCCS_Charge(1);			//Charge VCCS
				mHVPS_CHARGE(1);
				nState=kStimPhase10;
				GptLd(pADI_TM1,aTimerReloadValues[kTResidual]);									// reload value fot next timer
				break;
			}
			case kStimPhase10:		//ENd of Stimulation train
			{
				mAUX_PIN(0);				
				mVCCS_Charge(0);			//Charge VCCS
				mHVPS_CHARGE(0);
				nState=kFSMState11;
				GptLd(pADI_TM1,aTimerReloadValues[kTResidual]);									// reload value so high that the cycle will end before and FSM will be reset
				break;
			}
		}
	}
}
int mGetStimQuartile(int nChannel){		//Return the quartile of the stimulation intensity for a given channel
	//the quartile is 0..3 with respect to IMax
	if (aIAmp[nChannel]<(IMax[nChannel]>>2))			//if i<IMax*1/4
		return 0;
	else if (aIAmp[nChannel]<(IMax[nChannel]>>1))			//if i<IMax*1/2
		return 1;
	else if (aIAmp[nChannel]<((IMax[nChannel]>>1) +(IMax[nChannel]>>2)))			//if i<IMax*3/4
		return 2;
	else 
		return 3;
}
void mStimCountUp(){
	if (nMode.bits.RESETCOUNT==1){
		nMode.bits.RESETCOUNT=0;
		StimCount[0]=0;
		StimCount[1]=0;
		StimCount[2]=0;
		StimCount[3]=0;
	}

	if(aIAmp[0]==0) {
		StimCount[0]++;
	}
	else if((aIAmp[0]>0)&&(aIAmp[0]<=100)) {
		StimCount[1]++;
	}
	else if((aIAmp[0]>100)&&(aIAmp[0]<=200)) {
		StimCount[2]++;
	}
	else if(aIAmp[0]>200) {
		StimCount[3]++;
	}
	StimCountEx[0]=(int) StimCount[0];
	StimCountEx[1]=(int) StimCount[1];
	StimCountEx[2]=(int) StimCount[2];
	StimCountEx[3]=(int) StimCount[3];
}
