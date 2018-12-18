/* mSubs.c		//REv 180928
 drive: https://docs.google.com/document/d/1TQW26vanBpdOV9MyIOa9OLxb4MQ9kpXpRZGvvm4_dkg/edit
	Generic methods for general use
*/ 
//	Forward declarations
//	#include <DmaLib.h>
#include <C:\Keil\ARM\ARMCC\include\math.h>
#define M_PI 3.14159265358979323846


int LastSysTick;							//24bit system tick timer
unsigned int  DMA_ctrl(unsigned int iChan);				//R161012686



int aDMAs[3]={0};

void mWaitCycleStart(void){						 	// Wait using the system clock 
	int i;
	nTimerInMs[2]=60-(nTimerInMs[0]-nTimerInMs[1]);	//Time since epoch start = the spare CPU time
	i=SysTimer();
	while ((LastSysTick-kMainLoopIntervalInSystemTicks)<i) {
		if (LastSysTick<i){
			LastSysTick=LastSysTick+0x00FFFFFF;					// i roll over, add 24 bit to LastSysTick
		}
		i=SysTimer();
	}
	LastSysTick=i;
	nTimerInMs[1]=nTimerInMs[0];							//Start of cycle time nTimerInMs[1] is the current time in mS
}


uint32_t mLimit12Bit(uint32_t val32bit){				
																									//Saturation of data underflow
	if (val32bit>0x8000){														// A negative number has been casted 
		val32bit=0x0;
		bErrFlags.errbits.b12BitUnderflow =1;
	}
	else if (val32bit>0x0FFF){											// Saturation of data overflow
		val32bit=0x0FFF;
		bErrFlags.errbits.b12BitOverflow=1;
	}
	return val32bit;
}
//**************************			BATTERY CHECKS  ************************
#define kBattChangeValue 5
int mLowBattCheck(void){
	//Checks if battery is low
	//mV*ADC/ADCCoefficient

	if(nVbattVolt<(36*aAuxBuffer1[4]/430620)){	//Remember Saturates at 5 volts and becomes negative
		nVbattVolt=nVbattVolt+kBattChangeValue;	//Primitive filtering of vBatt
	}else {
		nVbattVolt=nVbattVolt-kBattChangeValue;
	}
	if (nMode.bits.THREEBATTERIES){					//Triggerlevel for battery empty
		nMode.bits.BATTLOW=(nVbattVolt<3*kCellEmptyLevel);		//Below empty level
		nMode.bits.BATTFULL=(nVbattVolt>3*kCellLowLevel);			//Above near empty level
	} else {														//Two battery cells
		nMode.bits.BATTLOW=(nVbattVolt<2*kCellEmptyLevel);		//Below empty level
		nMode.bits.BATTFULL=(nVbattVolt>2*kCellLowLevel);			//Above near empty level
	}
	if (nMode.bits.BATTLOW){
			nMode.bits.HVPS_ON=0;		//Switch off HV Generation 180727
			bErrFlags.errbits.BattLow=1;
			mSetMUX(kEN_SxCLOSE);				//SHORT ALL SWITCHES
	}

	return -1;
}









void mSystemActions(){			//4'th call. System management
		mPushButtonRead();				// Pushbutton actions, replaced by mPBActions()
		mPBActions();						//Actions on the pushbutton
		mSetLed180726();				//Or we can set a nMode like SleepMode and the led activation will be in the main cycle
		mSystemCheck();
		mSaveCountCheck();
		mLowBattCheck();
		//mBluetoothCheck();//TODO5 Revise			// Reset/Sleep Bluetooth
		//mModesCheck();
		// R16122	Set high voltage outputs
		// R16120	Set timed interrupt for "Output Stim Finite state machine"
}
void mCommunication(void){
		mDispatchTX(&oTXProt);
		mDispatchRX(&oTXProt);

}


void mFirstTimeSetup(void){	//First time setup
	nMode.bits.THREEBATTERIES=0;	//Assume two battery system, change from host
	nMode.bits.HVPS_ON=0;		//Non enable SMPS
	nMode.bits.ENSWITCHES=1;	//Enable the MULTIPLEXER
	nMode.bits.ENCHRGVCCS=1;	//VCCS Capacitor charging
	//Piecewice function or fixed IAMP
	nMode.bits.ENPLW=1;				//Enable MES control
	nMode.bits.INVALIDSETUP=0;	//The data are not from FLASH
		//Now assume that the setupfile will be valid
	aIAmp[0]=0;	//Zero stimulation level
	aIAmp[1]=0;	
	IMax[0]=10000;	//Max 10mA
	IMax[1]=00000;
	IMin[0]=0;
	IMin[1]=0;
	Gain[0]=0;
	Gain[1]=0;
	Offset[0]=100;
	Offset[0]=200;
	nBlankInterval[0]=1;
	nBlankInterval[1]=20;
	nBlankInterval[2]=60;
	nBlankInterval[3]=70;
	mBluetoothProgram(BTName); 	//Uncomment this to program the BT device
}




//**************************GENERATE A SINE WAVE ON IOUT ***************************
/*{ALGORITME:
	s=0;
	c=100;
	k=2*pi*0.01;
	t=(0:1000)*k;
	for x=1:length(t)-1;
		s(x)=round(s(x));    
		c(x)=round(c(x));    
    if abs(s(x))>c(1)
			s(x)=c(1)*sign(s(x));
    end
		s(x+1)=s(x)+k*c(x);
	c(x+1)=c(x)-k*s(x+1);
	p=t(x)/max(t);
 
}
*/
int sSin,sCos,sdt;
void cSinInit(){
	sSin=0;
	sCos=0X7FFF;
	sdt= 0X023C;			//{1 deg/call=0.01746;}
	return;
}
void cSinStep(int sdt){			//Iterative approximation of Sine/cosine using the derivative of sine=cosine
	sSin=sSin+((sdt*sCos)>>16);
	sCos=sCos-((sdt*sSin)>>16);
	if (sCos>0X7FFF){	//Correct bit errors if overflowing
			sSin=0;
			sCos=0X7FFF;
	}
	if (sSin>0X7FFF){
			sSin=0X7FFF;
			sCos=0;
	}
}



static const	int nMidpoint=0x08000000;		//DAC Midscale
static const int kGain2HZ=43;
//static const 
float kIAMP2mV=70.0;		//eq 10mV pp @ Offset=10.000
	int nSample=0;
void mOutputSineWave_TimerService(){			//Make a sinewave on DAC output (IAMP OUT)
		float nAmp;
		if (nSample==0){
			cSinInit();
			mStartTimer1(100);			//micro second timer
		}
		DacWr(0,nSample);									//Output bitvalue on DAC
		sdt= Gain[0]*kGain2HZ;					//Conversion so 100 is 100 Hz
		cSinStep(sdt);	//Calculate next step
		nAmp=( kIAMP2mV*(float)Offset[0]);
		nSample=nMidpoint+nAmp*sSin/(1<<15);	
}
void mOutputSineWave(){		
		//mPowerWatchDogReset();	//Disable timeout
		//nVbattVolt=4000;			//Disable battery check
		mVCCS_Charge(0);			//		disable all HIGH Voltage signals				mVPP_CHARGE(0);	
		mSetMUX(kEN_SxOPEN); 	//		disable switches.
		nMode.bits.ENPLW=0;			//Disable stimulation calculation
		if (nSample==0){
			cSinInit();
			mStartTimer1(100);			//micro second timer
	}

}
/*******************END SINEGENERATOR ***********************************/


	
