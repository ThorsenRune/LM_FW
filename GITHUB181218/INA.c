//			CONTROL OF INSTRUMENTATION AMPLIFIERS
//	doc:https://docs.google.com/document/d/12ccYno_Xh6iq3wZpqC_kozefIZjfTpl_ByutIdGPjAE/edit

#include "GlobalData.h"
#include "INA.H"			//R1610276
#include <DacLib.h>			//R161027	To use the DAC of ADUCM360
int DMA_n_minus_1(unsigned int iChan)
{      	// USe ADC1_C and ADC1_C+Alternate which are 1 larger than the channel index
        	return dmaChanDesc[iChan-1].ctrlCfg.Bits.n_minus_1  ;
        	// Therefore we subtract 1 from iChan
}
int bCurrentADCTask=0;
static const int kSampleFreqCode=3;			//Converter instruction for sampling at 1963 HZ
void mADCRestart(uint8_t Alt){									//R16101223
	bCurrentADCTask=2;
	NVIC_ClearPendingIRQ(DMA_ADC0_IRQn);		//When done dont do anything
	//NVIC_DisableIRQ(DMA_ADC0_IRQn);
	
	AdcGo(pADI_ADC0,ADCMDE_ADCMD_IDLE);		         // Put ADCx into idle mode to allow re-configuration of its control registers
	AdcGo(pADI_ADC1,ADCMDE_ADCMD_IDLE);		         // Put ADCx into idle mode to allow re-configuration of its control registers

	// Set pointers to ADC buffer
	AdcDmaReadSetup(ADC0DMAREAD,DMA_SIZE_WORD|DMA_DSTINC_WORD|	DMA_SRCINC_NO|DMA_BASIC,
	kADCBuffSize,aADCBuffer[Alt].aABuff);
  AdcDmaReadSetup(ADC1DMAREAD,DMA_SIZE_WORD|DMA_DSTINC_WORD|	DMA_SRCINC_NO|DMA_BASIC,
	kADCBuffSize,aADCBuffer[Alt].aBBuff);
	//Sample frequeencies
	AdcFlt(pADI_ADC0,kSampleFreqCode ,0,ADCFLT_SINC4EN_EN);					// ADC Sampling freq setup
	AdcFlt(pADI_ADC1,kSampleFreqCode ,0,ADCFLT_SINC4EN_EN); 					// ADC filter set for 1953Hz with sinc4
		//Connect pins
	AdcPin(pADI_ADC0,ADCCON_ADCCN_AGND,ADCCON_ADCCP_vINA_OUT1);	//INA_OUT1 Channel A to 
	AdcPin(pADI_ADC1,ADCCON_ADCCN_AGND,ADCCON_ADCCP_vINA_OUT2);	//INA_OUT2

	// Start converters synchronizes with 
 	AdcGo(pADI_ADC0,ADCMDE_ADCMD_CONT);		         // Start ADC0 for continuous conversions
	AdcGo(pADI_ADC1,ADCMDE_ADCMD_CONT);		         // Start ADC1 for continuous conversions
	// Clear Masking of DMA channel- Transfer begins
	// aTestData[5]=DMA_ctrl(ADC0_C)& 0xFF;			//Testline to remove R000
//  DmaClr(DMARMSKCLR_ADC0 ,0,0,0);
//  DmaClr(DMARMSKCLR_ADC1 ,0,0,0);
	DmaSet(0,DMAENSET_ADC0,0,DMAPRISET_ADC0);// Enable ADC0 DMA primary structure HIGH PRIORITY
	DmaSet(0,DMAENSET_ADC1,0,DMAPRISET_ADC1);// Enable ADC0 DMA primary structure

}




typedef enum {kINA0, kINA1} nIna;
typedef enum {kPOS, kNEG} ElectrodePol;
extern int nADCResult[kAIN9];


//	Experimental 180803		Set up a buffer for sampling the stimulation shape
static const int kAuxBufferSize =6;
int aAuxBuffer0[kAuxBufferSize];
int aAuxBuffer1[kAuxBufferSize];
int nADCSampleRate=1;
int nOption=0;

void mADCAux_Start( ){									//Start AD conversion of other channels
	bCurrentADCTask=1;
	AdcGo(pADI_ADC0,ADCMDE_ADCMD_IDLE);		         // Put ADCx into idle mode to allow re-configuration of its control registers
	AdcGo(pADI_ADC1,ADCMDE_ADCMD_IDLE);		         // Put ADCx into idle mode to allow re-configuration of its control registers
	//Configure BUFFERing
	AdcBuf(pADI_ADC0,ADCCFG_EXTBUF_OFF,ADCCON_BUFBYPN|ADCCON_BUFBYPP|ADCCON_BUFPOWP|ADCCON_BUFPOWN);
	AdcBuf(pADI_ADC1,ADCCFG_EXTBUF_OFF,ADCCON_BUFBYPN|ADCCON_BUFBYPP|ADCCON_BUFPOWP|ADCCON_BUFPOWN);

	AdcFlt(pADI_ADC0,1 ,0,ADCFLT_SINC4EN_DIS);					// ADC Sampling freq setup
	AdcFlt(pADI_ADC1,1 ,0,ADCFLT_SINC4EN_DIS); 					// ADC filter set for 1953Hz with sinc4
	
	//CONFIGURE ADC PIN CONNECTION
	AdcPin(pADI_ADC0,ADCCON_ADCCN_AGND,ADCCON_ADCCP_VCCS_PEAK_SENS);	//R121016A
	AdcPin(pADI_ADC1,ADCCON_ADCCN_AGND,ADCCON_ADCCP_vBattery);				//

	// Set pointers to ADC buffer kAuxBufferSize
	AdcDmaReadSetup(ADC0DMAREAD,DMA_SIZE_WORD|DMA_DSTINC_WORD|DMA_SRCINC_NO|DMA_BASIC,
	kAuxBufferSize,aAuxBuffer0);
  AdcDmaReadSetup(ADC1DMAREAD,DMA_SIZE_WORD|DMA_DSTINC_WORD|DMA_SRCINC_NO|DMA_BASIC,
	kAuxBufferSize,aAuxBuffer1);

	// Enable DMA channel- Transfer begins
	DmaSet(0,DMAENSET_ADC0,0,DMAPRISET_ADC0);// Enable ADC0 DMA primary structure HIGH PRIORITY
	DmaSet(0,DMAENSET_ADC1,0,DMAPRISET_ADC1);// Enable ADC0 DMA primary structure
	// Start converters synchronizes 
 	AdcGo(pADI_ADC0,ADCMDE_ADCMD_CONT);		         // Start ADC0 for continuous conversions
	AdcGo(pADI_ADC1,ADCMDE_ADCMD_CONT);		         // Start ADC1 for continuous conversions

	NVIC_ClearPendingIRQ(DMA_ADC0_IRQn);		//When done start the mADCRestart
	NVIC_EnableIRQ(DMA_ADC0_IRQn);
 }



