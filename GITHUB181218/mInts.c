/* (Rev.161028) 		INTERRUPTS AND SYSTEM CALLS					*/
// File: mInts.c
//doc: https://docs.google.com/document/d/1rCKpR4rUSjHY9sG1HBH6h85edKIod2TqvGwODXyCtLg/edit	 
#include <UrtLib.h>
#include <GptLib.h>
#include <WdtLib.h>
#include <WutLib.h>
extern void mOutputSineWave_TimerService(void);

int bADCDone[2] = {0};										// Flag used to indicate ADC0 resutl ready to send to UART

/*==========		INITIALIZATIONS		===========*/
 
void mStartTimer0(int nMicroSeconds){			//r161025
	// ClkDis must leave DIST0CLK=0
	// ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);            
	// Select CD0 for CPU clock - 16Mhz clock
	// RT: 16MHz System clock withPrescaling 4*16 = 0.25MHz =>250uS, count down mode
	GptLd(pADI_TM0,nMicroSeconds);					// 1ms timer 
	GptCfg(pADI_TM0,TCON_CLK_UCLK,TCON_PRE_DIV16,TCON_MOD_PERIODIC|TCON_UP_DIS|TCON_RLD_DIS|TCON_ENABLE);
	// TCON_RLD_DIS: Do not wait for interrupt to reload but reload automatically
	NVIC_EnableIRQ(TIMER0_IRQn);						// Enable Timer0 IRQ
}


void mStartTimer1(int nMicroSeconds){			// r161025
	//TIMER1 SETUP	
	GptCfg(pADI_TM1,TCON_CLK_UCLK,TCON_PRE_DIV16,TCON_MOD_PERIODIC|TCON_UP_DIS
		|TCON_RLD_EN										//+161125		//Enable reloading the counter from reload register
		|TCON_ENABLE);
	GptLd(pADI_TM1,nMicroSeconds); 		//+161125	//Set the countdown register
	GptClrInt(pADI_TM1,TSTA_TMOUT);		// Clear T1 interrupt and reload the counter
	NVIC_EnableIRQ(TIMER1_IRQn);			//Enable Timer1 IRQ
}




/*---------------------------*/
//+161102
void UART_Init (void)
{
   	//Select IO pins for UART.
//	pADI_GP0->GPCON |= 0x9000; 					          // Configure P0.6/P0.7 for UART
//orig    UrtCfg(pADI_UART,B9600,COMLCR_WLS_8BITS,0);        // setup baud rate for 9600, 8-bits
// to enable B115200
 UrtCfg(pADI_UART,B115200,COMLCR_WLS_8BITS,0);
		UrtMod(pADI_UART,COMMCR_DTR,0);  			          // Setup modem bits
    UrtIntCfg(pADI_UART,COMIEN_ERBFI|COMIEN_ETBEI|COMIEN_ELSI|COMIEN_EDSSI|COMIEN_EDMAT|COMIEN_EDMAR);  // Setup UART IRQ sources
/**	@param iIrq :{COMIEN_ERBFI| COMIEN_ETBEI| COMIEN_ELSI| COMIEN_EDSSI| COMIEN_EDMAT| COMIEN_EDMAR}
		- 0 to select none of the options.
		Or set to the bitwise or combination of
		- COMIEN_ERBFI to enable UART RX IRQ.
		- COMIEN_ETBEI to enable UART TX IRQ.
		- COMIEN_ELSI to enable UART Status IRQ.
		- COMIEN_EDSSI to enable UART Modem status IRQ.
		- COMIEN_EDMAT to enable UART DMA Tx IRQ.
		- COMIEN_EDMAR to enable UART DMA Rx IRQ.
 
**/
		NVIC_EnableIRQ(UART_IRQn);										//Enable the UART INTERRUPT
}
 


 


void Ext_Int2_Handler ()
{


}
void Ext_Int4_Handler ()
{


}
void WakeUp_Int_Handler(void)
{
	WutClrInt(T2CLRI_WUFD);          // Clear wakeup for D register interrupt
}
/*int uAmplitude=600;
int vDMACtrl[60]={0};
int vDMACtrlIdx=0;*/



//TIMER0_IRQn
void GP_Tmr0_Int_Handler(void){			// Called every 1 ms			R16101269
	// The nTimeInMs is incremented each millisecond. The 32 bit value will flip each 50 days
	// (we could also have used GptVal(pADI_TM0) to know the MCU internal timer count)
	nTimerInMs[0]++;									// Increase timer

	GptClrInt(pADI_TM0,TSTA_TMOUT);		// Clear interrupt
}

//TIMER1
void GP_Tmr1_Int_Handler(void)			// Called every 300 us used by mOutputStimFSM
{
	GptClrInt(pADI_TM1,TSTA_TMOUT);		// Clear T1 interrupt
	if ((nMode.bits.SINEGENERATOR)) 		//Be a sinus generator
			mOutputSineWave_TimerService();
	else if (nMode.bits.DEBUGGING)
			mOutputStimFSM_Debug(kNext);			//Final State Machine Stimulation
	
	else
		mOutputStimFSM(kNext);			//Final State Machine Stimulation

}


void WDog_Tmr_Int_Handler(void)
{
	WdtClrInt();											// clear watchdog timer interrupt if Interrupt mode is selected
}



 
int nADCCount[4];

void ADC0_Int_Handler()													//r161102
{
	volatile unsigned int uiADCSTA0 = 0;
	volatile unsigned int uiADCSTA1 = 0;
	uiADCSTA0 = pADI_ADC0->STA;										// read ADC status register
	uiADCSTA1 = pADI_ADC1->STA;										// read ADC status register
}

void ADC1_Int_Handler ()
{
	volatile unsigned int uiADCSTA0 = 0;
	volatile unsigned int uiADCSTA1 = 0;
	uiADCSTA0 = pADI_ADC0->STA;										// read ADC status register
	uiADCSTA1 = pADI_ADC1->STA;										// read ADC status register
}

//We don't use the DMA interrupts but rely on timing
void DMA_ADC0_Int_Handler()//A DMA transfer from the ADC is complete
{
 	NVIC_ClearPendingIRQ(DMA_ADC0_IRQn);			//Clear cascading interrupts
	if (bCurrentADCTask==1)
	{
		bADCDone[0]++;
		mADCRestart(bFlip);	
	}else{
		bADCDone[1]++;
		AdcGo(pADI_ADC0,ADCMDE_ADCMD_IDLE);		         // Put ADCx into idle mode to allow re-configuration of its control registers
		AdcGo(pADI_ADC1,ADCMDE_ADCMD_IDLE);		         // Put ADCx into idle mode to allow re-configuration of its control registers
	}
}


void DMA_ADC1_Int_Handler (){
	// bADCDone[1]++;	test to see if interrupt was taken
	DmaSet(DMARMSKSET_ADC1,DMAENSET_ADC1,0,0);	// Stop DMA
}


/*


void DMA_ADC1_Int_Handler_pingpong ()
{//Prepare next buffer for DMA
	int x1,x2; 
	NVIC_DisableIRQ(DMA_ADC1_IRQn);
	bADC1Done++;
	x1=DMA_ctrl(ADC1_C);
	x2=DMA_ctrl(ADC1_C+ALTERNATE);
	if  (0==(x1&0x3))	//Buffer is done? Test if  DMA is Pong
		//Reset DMA Pointer
	{	
		DmaCycleCntCtrl(ADC1_C,kADCBuffSize,DMA_DSTINC_WORD|
        DMA_SRCINC_NO|DMA_SIZE_WORD|DMA_BASIC|DMA_PING);         // Update cycle control and number of values to transfer fields		
	}
	else if (0x0==(x2&0x3))	//Buffer is done?
	{
		 //Primary register
	DmaCycleCntCtrl(ADC1_C+ALTERNATE,kADCBuffSize,DMA_DSTINC_WORD|
        DMA_SRCINC_NO|DMA_SIZE_WORD|DMA_BASIC|DMA_PING);         // Update cycle control and number of values to transfer fields		
}
	else {
 nCycleStage++;
	}
		NVIC_ClearPendingIRQ(DMA_ADC1_IRQn);
	
// 	  NVIC_EnableIRQ(DMA_ADC1_IRQn);
}
*/


void DMA_SINC2_Int_Handler()
{
	DmaSet(DMARMSKSET_SINC2,DMAENSET_SINC2,0,0);
}
void DMA_Err_Int_Handler (){
	//doc:https://docs.google.com/document/d/1rCKpR4rUSjHY9sG1HBH6h85edKIod2TqvGwODXyCtLg/edit#heading=h.3amz5hw9sru9

	volatile unsigned long ulDmaStatus = 0;			// In error check routine
	ulDmaStatus = DmaSta();
	while (ulDmaStatus) {}
	DmaErr(DMA_ERR_CLR);
}


void SINC2_Int_Handler()
{


}




void UART_Int_Handler (){
/*	interrupt from UART to send/receive a character
*/
	volatile unsigned char ucCOMSTA0 = 0;
	volatile unsigned char ucCOMIID0 = 0;
	volatile unsigned int uiUartCapTime = 0;


	ucCOMSTA0 = UrtLinSta(pADI_UART);			// Read Line Status register
	ucCOMIID0 = UrtIntSta(pADI_UART);			// Read UART Interrupt ID register


	if ((ucCOMIID0 & 0x4) == 0x4)
	{
// Receive byte. If buffer is full then discard and send a ErrMsg
				
			mFIFO_push(oRX,UrtRx(pADI_UART));
			if (mFIFO_isFull(oRX)) 					
				bErrFlags.errbits.bRXOverflow =1;	//Receive overflow
	}
	else if ((ucCOMIID0 & 0x2) == 0x2)	  			// Transmit buffer empty
	{
// Transmit data
		if (mFIFO_isEmpty(oTX)==0)
		{
			UrtTx(pADI_UART,mFIFO_pop(oTX));
		}
	}
	else if ((ucCOMSTA0 & COMLSR_THRE)== COMLSR_THRE) { //Queue is empty
		if (mFIFO_isEmpty(oTX)==0)
		{
			UrtTx(pADI_UART,mFIFO_pop(oTX));
		}
	}
}
//				------------------------------------------------------------------------------
volatile uint32_t iX;
void UART_TX_Trigger ()//Set the IRQ pending flag if something to transmit
{		
		iX= NVIC_GetPendingIRQ(UART_IRQn);
		if (iX>0)
		{iX=10;};
		if (mFIFO_isEmpty(oTX)==0)	
			 NVIC_SetPendingIRQ(UART_IRQn);
}
void PWMTRIP_Int_Handler (){}
void PWM0_Int_Handler(){}
void PWM1_Int_Handler (){}
void PWM2_Int_Handler(){}
	

