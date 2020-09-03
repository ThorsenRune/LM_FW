//	For TESTING PURPOSE
//doc: https://docs.google.com/document/d/1-nsTuq9FZeSd8QpIQtY-Q66j8P_odWR-Z_HK5ZeI8NQ/edit			
//file: mProts.c
/*	
	Temporary Prototype functions	here
	When they are debugged move into respective modules
	Link:main
*/
//#include "mProts.h"
#include "VCCS.h"
#include <IexcLib.h>		//If you use excitation outputs


//161031 		Under construction. A method for setting the next auxillary channel to be converted for battery, LOD etc detection
//	An enumerator witll assign a channel to an element in a 6  cell array.
//	Single shot ADC? 
//int nAInAux[6];			//Auxialary inputs to ADC on MCU R161102



typedef enum {kState1,kState2, kState3, kState4, kState5,kState6,kState7,kNoOperation} eADC_AUX_STATES;
int nADCResult[kAIN9];  //ADC Results in 0-1.2V range. 
eADC_AUX_STATES nAInAuxIdx=kState1;
eADC_AUX_STATES mADC_Aux_Next(eADC_AUX_STATES nState){				//R161102	Set two adc channels
	switch (nState)
		{
			case kState1:{
				AdcPin(pADI_ADC1,ADCCON_ADCCN_AGND,ADCCON_ADCCP_AIN9);	//R121016A
				return kState2;
			}
			case kState2:{//AIN0
				AdcPin(pADI_ADC1,ADCCON_ADCCN_AGND,ADCCON_ADCCP_VCCS_PEAK_SENS);	//R121016A
				return kState3;
			}
			case kState3:{
				AdcPin(pADI_ADC1,ADCCON_ADCCN_AGND,ADCCON_ADCCP_vBattery);	//R180726	R121016A
				return kState4;
			}
			case kState4:{
				AdcPin(pADI_ADC1,ADCCON_ADCCN_AGND,ADCCON_ADCCP_vLOD1N);	//R121016A
				return kState5;
			}
			case kState5:{
				AdcPin(pADI_ADC1,ADCCON_ADCCN_AGND,ADCCON_ADCCP_AIN5);	//R121016A
				return kState6;
			}
			case kState6:{
				AdcPin(pADI_ADC1,ADCCON_ADCCN_AGND,ADCCON_ADCCP_AIN8);	//R121016A
				return kState1;
			}
			default:{
				return kState1;
			}
		}		
}

void mADC_Aux_Get(void){
	/*
	//AdcDmaCon(ADC0DMAREAD,0);									//DISABLE DMA
	//AdcGo(pADI_ADC0,ADCMDE_ADCMD_IDLE);	
	AdcGo(pADI_ADC1,ADCMDE_ADCMD_IDLE);	
	//AdcFlt(pADI_ADC0,0 ,0,ADCFLT_SINC4EN_EN); // ADC Sampling freq setup
	AdcFlt(pADI_ADC1,0 ,0,ADCFLT_SINC4EN_EN); // ADC filter set for 1953Hz with sinc4
	nAInAuxIdx=mADC_Aux_Next(nAInAuxIdx);
	//AdcPin(pADI_ADC1,ADCCON_ADCCN_AGND,ADCCON_ADCCP_AIN7);
	//AdcMski(pADI_ADC0,ADCMSKI_RDY,1);              // Enable ADC ready interrupt source		
	AdcMski(pADI_ADC1,ADCMSKI_RDY,1);              // Enable ADC ready interrupt source		
   // AdcDmaCon(ADC1DMAREAD,0);
//		NVIC_EnableIRQ(ADC0_IRQn);           // Enable ADC0 and ADC1 interrupts
	NVIC_EnableIRQ(ADC1_IRQn);					//Lets do it all in first interrupt
	
	nADCResult[nAInAuxIdx]= AdcRd(pADI_ADC1);		// read ADC result register
			case kState2:{
			nADCResult[kState2]= AdcRd(pADI_ADC1);		// read ADC result register
			break;
		}
		case kAIN3:{
			nADCResult[kAIN3]= AdcRd(pADI_ADC1);		// read ADC result register
			break;
		}
		case kState5:{
			nADCResult[kState5]= AdcRd(pADI_ADC1);		// read ADC result register
			break;
		}
	}
	

	AdcGo(pADI_ADC1,ADCMDE_ADCMD_IDLE);				//Returns automatically to idle mode
	nADCCount[1]=TickTock();
	nADCCount[2]=nADCCount[1]-nADCCount[0];
	
	
		//AdcGo(pADI_ADC0,ADCMDE_ADCMD_SINGLE);		//Make a single sample
		AdcGo(pADI_ADC1,ADCMDE_ADCMD_SINGLE);	
	
//	AdcDmaCon(ADC0DMAREAD,1);									//ENABLE DMA
	//	NVIC_DisableIRQ(ADC1_IRQn);
	*/
}



int nADCCount[4];

/*
typedef union			
{ 
	int 			all[1];
	struct
	{
		uint32_t
						EN2_CH1					: 1,
						EN2_CH0					: 1,
						EN_CH1					: 1,
						EN_CH0					: 1,
						LED_CTRL				: 1,
						CMD_MLDP				: 1,
						EN_ST						: 1,
						EN_INA_SUPPLY		: 1,
						EN_HVPS					: 1,
						CHRGVCCS_DRIVE	: 1,
						EN_BLUE_COMM		: 1,
						PLS_CTRL				: 1,
						spare12					: 1,
						spare13					: 1,
						spare14					: 1,
						spare15					: 1,
						spare16					: 1,
						spare17					: 1,
						spare18					: 1,
						spare19					: 1,
						spare20					: 1,
						spare21					: 1,
						spare22					: 1,
						spare23					: 1,
						spare24					: 1,
						spare25					: 1,
						spare26					: 1,
						spare27					: 1,
						spare28					: 1,
						spare29					: 1,
						spare30					: 1,
						spare31					: 1;
	} ModeBits;

} ModeFlags;

ModeFlags Mode;


void mPinActivationMode(ModeFlags mode){

	if (mode.ModeBits.EN_BLUE_COMM){
		mBlueToothEnable(kEnable);
	}
	else {
		mBlueToothEnable(kDisable);
	}
	// Bit related to CHRGVCCS_DRIVE

	// Bit related to EN_HVPS (VPP_CHARGE, enables HVPS)
	if (mode.ModeBits.EN_HVPS){
		
	}
	else {

	}
	// Bit related to EN_INA_SUPPLY (controls power supply to analog circuits)
	if (mode.ModeBits.EN_INA_SUPPLY){
//Obsolete		mSet_EN_INA_SUPPLY(kEnable);
	}
	else {
//Obsolete		mSet_EN_INA_SUPPLY(kDisable);
	}
	// Bit related to EN_ST (powers the stimulator circuits on VCCS)
	if (mode.ModeBits.EN_ST){
	//Obsolete	mVCCSPower(kEnable);
	}
	else {
//Obsolete		mVCCSPower(kDisable);
	}
	// Bit related to CMD_MLDP (configures the BlueTooth module as xmission or programming mode)
	if (mode.ModeBits.CMD_MLDP){
		mBlueToothConfig(kEnable);
	}
	else {
		mBlueToothConfig(kDisable);
	}
	// Bit related to LED_CTRL (controls the LED)				!!! But how do we use the LED? See also mSetLed fuction in LVPS.c
	if (mode.ModeBits.LED_CTRL){

	}
	else {

	}
	// Bit related to EN_CH0 (MUX)
	if (mode.ModeBits.EN_CH0){

	}
	else {

	}
	// Bit related to EN_CH1 (MUX)
	if (mode.ModeBits.EN_CH1){

	}
	else {

	}
	// Bit related to EN2_CH0 (MUX)
	if (mode.ModeBits.EN2_CH0){
		DioSet(pADI_GP0,0x10);
	}
	else {
		DioClr(pADI_GP0,0x10);
	}
	// Bit related to EH2_CH1 (MUX)
	if (mode.ModeBits.EN2_CH1){

	}
	else {

	}
}

void IEXCINIT(void){
	
	IexcCfg(IEXCCON_PD_off,IEXCCON_REFSEL_Int,IEXCCON_IPSEL1_Off,IEXCCON_IPSEL0_AIN5); //Setup IEXC for AIN5
	IexcDat(IEXCDAT_IDAT_0uA,IDAT0En);         // Set output for 10uA
}


 
void displayvalues(void) {
  int idx;

  for (idx = 0; idx < 108; idx++) {
    printf ("%02X\n", yData[idx]);
  }
}
*/
