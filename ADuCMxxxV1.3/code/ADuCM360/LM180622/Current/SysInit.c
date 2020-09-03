/*	System initializations		rev 180928 
	https://docs.google.com/document/d/1RUQ-6eFk4NQqfGKo5GsLOMx5UhPk7BwsdwAAu_EFxKg/edit
	file:SysInit.c
*/

#include <DacLib.h>					// R161027	To use the DAC of ADUCM360
#include <WdtLib.h>
#include <ClkLib.h>
#include <DioLib.h>
#include <SpiLib.h>
#include "System.h"
#include "SysInit.h"
#include "GlobalData.h"
#include "mErrFlags.h"			// Error messages
#include "mInts.h"
#include "mDebug.h"
#include "VCCS.h"
#include "cProtocol.h"

#define NUM_CHAR1 64
#define NUM_CHAR2 44


extern void mCommInitialize(void);
extern void mHVPS_CHARGE(int bEnable);
extern void mSet_VPP_CHARGEv1 (eAction act);
extern void mSet_EN_INA_SUPPLY(eAction act);
extern void mVCCSPower(eAction act);
extern int isButtonPushed(void);
extern void mSetLedColor(	eLedMode nLedMode);

unsigned char ErasePage(unsigned long);
extern void WriteToFlash(unsigned long *, unsigned long, unsigned int);

 
int HVValue[1]={180};

typedef enum {
		kT0			//Switch on
		,kT1		//VCCS ON
		,kT2		//VCCS OFF 
		,kT3
		,kT4
		,kT5
		,kT6
		,kT7
		,kT8	//Interval kStimPhase8
		,kTDurChrge	//Charge duration phase 9-10
		,kTResidual  //Residual time in the stim cycle
		,kSizeTimerReloadValues} eTimerIndex;
int aTimerReloadValues[kSizeTimerReloadValues];

void mSetSystemValues(){		//Remember to anticipate switches on time by 300uS
	aTimerReloadValues[kT0]		=300;	   			//Pulse duration
	aTimerReloadValues[kT1]		=300;	   			//Pulse duration
	aTimerReloadValues[kT2]		=400;	   			//Interpulse Interval 180928 the Optocoupler cant do 300uS
	aTimerReloadValues[kT3]		=300;	   			//Pulse duration
	aTimerReloadValues[kT4]		=300;
	aTimerReloadValues[kT5]		=300;	 
	aTimerReloadValues[kT6]		=300;	 
	aTimerReloadValues[kT7]		=1300;
	aTimerReloadValues[kT8]		=300;	 
	aTimerReloadValues[kTDurChrge]		=20000;//Duration of charging time
	aTimerReloadValues[kTResidual]		=60000; 
	nVbattVolt=4000;		//Assume 4V battery for a start
	nShutDownTimer=0;
	nMode.bits.SHUTDOWN=0;
	nMode.bits.STIMENABLE=0;	//Start in pause mode
	nMode.bits.SR_FILTER	=0; 	//Do normal filtering
}

void AD5592_Write(int val16bit){
	// Description: SPI transfer of val16bit
	SpiFifoFlush(pADI_SPI0,SPICON_TFLUSH_EN,SPICON_RFLUSH_DIS); 
																					// can be removed for optimization
	SpiTx(pADI_SPI0,(val16bit >> 8)&0xFF);	// Write HIByte to FIFO
	SpiTx(pADI_SPI0,val16bit&0xFF);					// Write LOBYTE to FIFO
	
	// This routine can't be called two times within 200us 
}


int AD5592_Read(){
	// Description: 16 bit value transfer with SPI from AD5592 (U12)
   int RxVal;  
	
	RxVal = SpiRx(pADI_SPI0);			//Receive HIByte
	RxVal = (RxVal << 8);
	RxVal |= SpiRx(pADI_SPI0);		//Receive LOByte
	
	 return RxVal;
}


void DAC_Pin_Setup (){  
	// Description: Using SPI, AD5592 Pins IO4 and IO0 are set as DAC outputs
  // SetDAC=0|0101|000|00010001
	// D15: No write, low; D14-D11: Register address (DAC pin configuration is 0101); 
	// D10-D8: Reserved (all LOW); D7-D0: Select which pin you want as DAC output (we want 0 and 4)
	AD5592_Write(kSetDAC);
}


void ADC_Pin_Setup (){
	// Description: Using SPI, AD5592 Pins IO2, IO5 and IO6 are set as ADC inputs
	// SetADC=0|0100|000|01100100
	// D15: No write, low; D14-D11: Register address (ADC pin configuration is 0100); 
	// D10-D8: Reserved (all LOW); D7-D0: Select which pin you want as DAC output (we want 2, 5 and 6)
	AD5592_Write(kSetADC);
}


void AD5592_Reset() {
	// Description: reset AD5592
	AD5592_Write(kResetAD);
}


void AD5592_EnVref() {
	// Desription: int ref mode AD5592
	AD5592_Write(kEnVrefAD);
}


void AD5592Init (){  
	// Description: AD5592 reset to default configurations and initialization (ADC INPUT/DAC OUTPUT PINS)
	AD5592_Reset();
	mWaitMs(1000);			// Wait for SPI to complete
	AD5592_EnVref();
	mWaitMs(1000);
 	DAC_Pin_Setup();
	mWaitMs(1000);
	ADC_Pin_Setup(); 
	mWaitMs(1000);
}


void SPI0INIT(){    	//REV161019
  // Description: Initialization of SPI0
	SpiCfg(pADI_SPI0,									// SPI0
				SPICON_MOD_TX2RX2,					// 2 bytes Rx and Tx FIFO size
				SPICON_MASEN_EN,						// SPI master mode
				SPICON_CON_EN|							// Enable continuous transfer  
				SPICON_RXOF_EN|							// Overwrite the valid data in the Rx register with the new serial byte received
				SPICON_ZEN_EN|							// Transmit 0x00 when there is no val
				SPICON_TIM_TXWR|						// Initiate transfer with a write to the SPI TX register
				SPICON_CPHA_SAMPLETRAILING|	// Serial clock pulses at the  start of the first data bit transfer
				SPICON_ENABLE_EN);					// Enable SPI
	
	SpiBaud(pADI_SPI0,								// SPI0
					7,												// 1MHz clock, 1MHz=16MHz/2*(iCLKDiv+1) ---> iCLKDiv = 7
					SPIDIV_BCRST_DIS);        // Disable CS error detection
}


void DACINIT(void){
	// Configure DAC output for 0-1.2V output range, Normal 12-bit mode and immediate update.
	DacCfg(DACCON_CLR_Off,
	DACCON_RNG_AVdd,										//Range 0-1.8V
	DACCON_CLK_HCLK,DACCON_MDE_12bit);	  
 
}


void mADC_Initialize(){

	AdcGo(pADI_ADC0,ADCMDE_ADCMD_IDLE);		         // Put ADC0 into idle mode to allow re-configuration of its control registers
	AdcGo(pADI_ADC1,ADCMDE_ADCMD_IDLE);		         // Put ADC1 into idle mode to allow re-configuration of its control registers
	// AdcMski(pADI_ADC0,ADCMSKI_RDY,1);											// Enable ADC ready interrupt source
	// AdcMski(pADI_ADC1,ADCMSKI_RDY,1);											// Enable ADC ready interrupt source/
	// ADC Ready interrupt not neede when using DMA
	AdcFlt(pADI_ADC0,sampfreq ,0,ADCFLT_SINC4EN_EN);					// ADC Sampling freq setup
	AdcFlt(pADI_ADC1,sampfreq ,0,ADCFLT_SINC4EN_EN); 					// ADC filter set for 1953Hz with sinc4
	// AdcFlt(pADI_ADC1,sampfreq ,0,0); //  sinc4 is performing better though
	// Turn off input buffers to ADC and external reference
	// MUST BE OFF BECAUSE WE USE GND AS NEGATIVE REFERENCE
	AdcBuf(pADI_ADC0,ADCCFG_EXTBUF_OFF,ADCCON_BUFBYPN|ADCCON_BUFBYPP|ADCCON_BUFPOWP|ADCCON_BUFPOWN);
	AdcBuf(pADI_ADC1,ADCCFG_EXTBUF_OFF,ADCCON_BUFBYPN|ADCCON_BUFBYPP|ADCCON_BUFPOWP|ADCCON_BUFPOWN);
	// Enable buffering of ADC0	
	// Turn ADC input buffers on for this differential measurement
	// AdcBuf(pADI_ADC0,ADCCFG_EXTBUF_OFF,ADC_BUF_ON);	
	// Use Ground reference (ADCCON_ADCCN_AGND). 
	AdcPin(pADI_ADC0,ADCCON_ADCCN_AGND,ADCCON_ADCCP_vINA_OUT1);	//INA_OUT1 Channel A to 
	// AdcBuf(pADI_ADC1,ADCCFG_EXTBUF_OFF,ADC_BUF_ON);	
	AdcPin(pADI_ADC1,ADCCON_ADCCN_AGND,ADCCON_ADCCP_vINA_OUT2);	//INA_OUT2
	//GAIN AND REFERENCE OF ADC
	// Reference ADCCON_ADCREF_INTREF or ADCCON_ADCREF_AVDDREF
	// ADCMod2 gain
	//	AdcRng(pADI_ADC0,ADCCON_ADCREF_AVDDREF,ADCMDE_PGA_G1|ADCMDE_ADCMOD2_MOD2ON,ADCCON_ADCCODE_INT);
	// 	AdcRng(pADI_ADC1,ADCCON_ADCREF_AVDDREF,ADCMDE_PGA_G1|ADCMDE_ADCMOD2_MOD2ON,ADCCON_ADCCODE_INT);
	//No Gain, No buff
	AdcRng(pADI_ADC0,ADCCON_ADCREF_AVDDREF,ADCMDE_PGA_G1,ADCCON_ADCCODE_INT);
 	AdcRng(pADI_ADC1,ADCCON_ADCREF_AVDDREF,ADCMDE_PGA_G1,ADCCON_ADCCODE_INT);
	// Set VBIAS(900 mV) to  AIN11
//180726 obsolete	AdcBias(pADI_ADC1,ADCCFG_PINSEL_AIN11,ADC_BIAS_X1,0);	        
	// Enable DMA reading of ADC
	AdcDmaCon(ADC0DMAREAD,1);
	AdcDmaCon(ADC1DMAREAD,1);
}

void mAUX_PIN(int act){			//Set auxillary pin on side header pin2.1 on headerpin4 from the left
	if (act)
		DioSet(pADI_GP2,1<<PIN1);
	else
		DioClr(pADI_GP2,1<<PIN1);

}



void mClockSetup(){
	// Disable clock to unused peripherals
  // Leave enabled Timer0 clock, SPI0 clock, Timer1 clock
	ClkDis(CLKDIS_DISSPI1CLK|CLKDIS_DISI2CCLK|CLKDIS_DISPWMCLK);
  //Internal oscillator 16MHz
	ClkCfg(CLK_CD0,CLK_HF,	CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);		
	// Select CD0 for CPU clock, 16MHz clock
	ClkSel(CLK_CD0,	CLK_CD7,CLK_CD0,CLK_CD7);	
	// Select CD0 for UART System clock   														// SPI Clock division R161019232

}


// Setup the MCU pins
void mPinsSetup(){

	//Description: setup of pins as GPIO/SPI/UART, as in/out and high/low
	//Documentation - see ref:cp41t4sc1sce -https://docs.google.com/document/d/1bKBs6AZuaRwsdVSNGy2KcvT2cGvAZ_Jd5370qR_vZKg/edit#
	//see UG-367 Table 116. GPIO Multiplex Table
	DioCfg(pADI_GP0,0x403C);	//bit7:GPIO,6:GPIO,5:GPIO,4:GPIO,3:GPIO,2&1:UART,0:GPIO
	DioOen(pADI_GP0,0xDD);  	//bit 7-0:O,O,I,O    O O ,TX,RX,O 
	DioPul(pADI_GP0,0);
	DioCfg(pADI_GP1,0xAA00);	//SPI and bit 3,2,1,0;outputs
	DioOen(pADI_GP1,0xEF);				// Pin configuration as input/output R171016A
	DioCfg(pADI_GP2,0x0000);	//bit 2,1,0 outputs	DioOen(pADI_GP2,0x07);
	DioOen(pADI_GP2,BIT1|BIT0); //Set P2.1,P2.0 as outputs
	DioPul(pADI_GP2,0);
	//180724			Set PS_ENA 
	while (!isButtonPushed()){		//Die if no keypress
		mPS_ENA(kDisable);	
	}
	mPS_ENA(kEnable);			// P0.0 (PS_ENA) HIGH only if pushbutton is pushed
	mHVPS_CHARGE(0);		//Disable HVPS (LT3484)
	mVCCS_Charge(0);
	mSetMUX(kEN_SxOPEN);
 		TickTockInit();//Configures a microtimer for debugging purpose
}


void mBluetoothProgram (char name[]){//180726    Program the bluetooth device
	char CommString1[NUM_CHAR1]={"SET PROFILE SPP ON\rSET CONTROL BAUD 115200,8n1\rSET BT NAME LM-BT"};
	char CommString2[NUM_CHAR2]={"\rSET BT AUTH * 1234\rSET CONTROL ECHO 0\rRESET"};
	char PlusString[3]={"+++"};
	int i=0,j, length1, lengthname, length2, txflag=0;
	__disable_irq();	//Don't interrupt this programming sequence
		
	length1= sizeof CommString1 / sizeof CommString1[0];
	lengthname= sizeof name / sizeof name[0];
	length2= sizeof CommString2 / sizeof CommString2[0];

	mWaitMs(1000);														// 1 second wait
	
	for (j=0; j<3; j++){
		txflag=UrtTx(pADI_UART,PlusString[i]);
		if (txflag==0){
			mTXFullError();
		}
		mWaitMs(1);														// 1ms wait
	}
	
	mWaitMs (1000);														// 1 second wait
	//Send command string	
	for (i=0; i<length1; i++){
		txflag=UrtTx(pADI_UART,CommString1[i]);
		if (txflag==0){
			mTXFullError();
		}
		mWaitus(150);																// 150 us wait
	}
	//Append the name	
	for (i=0; i<lengthname; i++){
		txflag=UrtTx(pADI_UART,name[i]);
		if (txflag==0){
			mTXFullError();
		}
		mWaitus(150);																// 150 us wait
	}
		
	for (i=0; i<length2; i++){
		txflag=UrtTx(pADI_UART,CommString2[i]);
		if (txflag==0){
			mTXFullError();
		}
		mWaitus(150);																// 150 us wait
	}
		
	mWaitMs (1000);														// 1 second wait
	__enable_irq();
}


void mBluetoothDeepSleep (void){

	char String[10]={"SLEEP"};
	char PlusString[3]={"+++"};
	int i=0,j, length, txflag;

	length= sizeof String / sizeof String[0];

	mWaitMs (1300);														// 1 second wait
	
	for (j=0; j<3; j++){
		txflag=UrtTx(pADI_UART,PlusString[i]);
		if (txflag==0){
			mTXFullError();
		}
		mWaitMs(200);																// 150 us wait
	}
	
	mWaitMs(1300000);														// 1 second wait
	
	for (i=0; i<length; i++){
		txflag=UrtTx(pADI_UART,String[i]);
		if (txflag==0){
			mTXFullError();
		}
		mWaitMs(1);																// 150 us wait
	}
	
	mWaitMs (1300);														// 1 second wait
}


void mBluetoothReset (void){
	char String[5]={"RESET"};
	char PlusString[3]={"+++"};
	int i=0,j, length, txflag;

	length= sizeof String / sizeof String[0];
	mWaitMs (1300);														// 1 second wait
	for (j=0; j<3; j++){
		txflag=UrtTx(pADI_UART,PlusString[i]);
		if (txflag==0){
			mTXFullError();
		}
		mWaitMs(200);																// 150 us wait
	}
	
	mWaitMs(1000);														// 1 second wait
	
	for (i=0; i<length; i++){
		txflag=UrtTx(pADI_UART,String[i]);
		if (txflag==0){
			mTXFullError();
		}
		mWaitMs(200);																// 150 us wait
	}
	
	mWaitMs(1000);														// 1 second wait
}


void MainSetup(){		
		mClockSetup();	//Setup the 16MHz clock
		mPinsSetup();	//This sets the power pin on													// Pins configuration (GPIO/SPI/UART, IN/OUT, HI/LO)
	// link:	https://docs.google.com/document/d/1C-8l3cpXOS7QJIlOkwGrsjPzG1upqzH8fwQZ4iV09D4/edit#
	mCommInitialize();//Initialize the protocol communication with HOST
	bErrFlags.all_flags[0] = 0U;											// Clear all flags
	pADI_WDT ->T3CON = 0;															// Disable the watchdog timer
	WdtCfg(T3CON_PRE_DIV1,T3CON_IRQ_EN,T3CON_PD_DIS);	// Disable Watchdog timer resets
  // R161028
	UART_Init();																			// UART initialization
	DmaBase();																				// Setup ADC0 DMA channel
	mStartTimer0(t0count);														// Start timer0
	DACINIT();																				// Initialize internal DAC
	mADC_Initialize();//
	mPowerWatchDogReset();
}


void mBluetoothCheck000 (void){
	// Check using flag if we want to reset/sleep Bluetooth
		
		if (BTsleepvar==1){										// Bluetooth Deep Sleep Mode

			__disable_irq();
			mBluetoothDeepSleep();
			BTsleepvar=0;
			__enable_irq();
		}
}


void mSaveCountCheck(void){
	if (Min5Count>=300000){
		Min5Count=0;
		nMode.bits.SAVECOUNT=1;
	}
		
	if (nMode.bits.SAVECOUNT==1){
		nMode.bits.SAVECOUNT=0;
		
		__disable_irq();
		ErasePage(0xA000);									// ERASE AND WRITE TO FLASH			
		WriteToFlash(StimCount,0xA000,sizeof(StimCount));
		__enable_irq();
	}
}
