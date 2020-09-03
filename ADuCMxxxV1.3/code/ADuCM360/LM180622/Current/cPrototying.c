#include "cprototyping.h"
int nTest=10000;
int nTestStimTune=1000;



void mDeepSleepTest(){   //This really doesnt work very well since:
	/*testing deep sleep of up , but does nothing to power consumption
	on board AVDD must be connected to DVDD (IOVDD) so we cant power down AVDD
	AVDD is connected to INAS without shut down.
	*/
	int sleepms=5;
	int bToggle=0;
	//	Implementation of a sleep 
      //Disable clock to unused peripherals
	mSetMUX(kEN_SxOPEN);
//	ClkDis(0x1FF); //disable all clocks
//   ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);            // Select CD0 for CPU clock - 16Mhz clock
//	 ClkCfg(CLK_CD7,CLK_LF,CLKSYSDIV_DIV2EN_EN,CLK_LFO);
	AdcGo(pADI_ADC0,ADCMDE_ADCMD_OFF);
	AdcGo(pADI_ADC1,ADCMDE_ADCMD_OFF);
	DacCfg(DACCON_CLR_On,	DACCON_RNG_AVdd,DACCON_CLK_HCLK,DACCON_PD);	//Power down dac
	 WdtGo(T3CON_ENABLE_DIS); // disable watchdog timer
   WutGo(T2CON_ENABLE_DIS);  // resets the timer
  WutCfg(T2CON_MOD_PERIODIC,T2CON_WUEN_EN,T2CON_PRE_DIV32768,T2CON_CLK_LFOSC);  // config while timer is disabled
  WutLdWr(3,sleepms); // load 30s                                     // Set the timeout period 
	WutClrInt(T2CLRI_WUFD); 
  WutCfgInt(T2IEN_WUFD,1); // enables field D interrupt
   NVIC_DisableIRQ(TIMER0_IRQn);          //Disable timer interrupts
   NVIC_DisableIRQ(TIMER1_IRQn);          //Disable timer interrupts
   NVIC_EnableIRQ(WUT_IRQn);                             // Enable Timer2 IRQ
   WutGo(T2CON_ENABLE_EN);                               // Start wake-up timer
	
//	DioOen(pADI_GP0,0);
//	DioOen(pADI_GP2,0);
//	DioPul(pADI_GP0,0);
//	DioPul(pADI_GP2,0);
	 PwrCfg(PWRMOD_MOD_TOTALHALT); //Stops here until waked up
    while (1)
   {	
			if (bToggle==0) bToggle=1; else bToggle=0;
			if (bToggle==0)
				DioSet(pADI_GP2, BIT0);			// P2.0
			else 
				DioClr(pADI_GP2, BIT0);			// P2.0
			 PwrCfg(PWRMOD_MOD_TOTALHALT);  
	 }

}	


//This was used to test the possibility of a buzzer. However the sound was very low.
 int nMicroSeconds=300;
void ToggleBuzzer(){	
	mGPIO_Direction(pADI_GP2,1,0);	
	DioTgl(pADI_GP2,BIT1);
	GptLd(pADI_TM0,nMicroSeconds);		
}

void mOutputStimFSM_Debug (eAction act){ 
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
				if (nDebugMode.DEBUGBITS.A)
					mSetMUX(kEN_SxCLOSE);  
				else 
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

