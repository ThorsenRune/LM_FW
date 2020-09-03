/**
 *****************************************************************************
   @example  SPIDMA_Slave.c
   @brief    SPI1 Slave DMA example
      Expects a 16-byte array on SPI1 RX input

   @version  V0.1
   @author   ADI
   @date     October 2012 


All files for ADuCM360/361 provided by ADI, including this file, are
provided  as is without warranty of any kind, either expressed or implied.
The user assumes any and all risk from the use of this code.
It is the responsibility of the person integrating this code into an application
to ensure that the resulting application performs as required and is safe.


**/



#include <stdio.h>
#include <string.h>
#include <ADuCM360.h>


#include <ClkLib.h>
#include <IntLib.h>
#include <DioLib.h>
#include <WdtLib.h>
#include <DioLib.h>
#include <DmaLib.h>
#include <SpiLib.h>

void delay(long int);
void SPI1INIT(void);
void DMAINIT(void);

unsigned char ucIrqCnt = 0;                              // Used to count number of SPI1 DMA interrupts
unsigned char ucSPIDMAEnable = 0;                        // Flag used to setup 2nd and subsequent transfers
unsigned char uxSPI1RxData[16];

int main (void)
{
   WdtCfg(T3CON_PRE_DIV1,T3CON_IRQ_EN,T3CON_PD_DIS);      // Disable Watchdog timer resets
   DioOen(pADI_GP1,0x8);                                  // Set P1.3 as an output to toggle the LED
   //Disable clock to unused peripherals
   ClkDis(CLKDIS_DISI2CCLK|CLKDIS_DISPWMCLK|
	   CLKDIS_DIST0CLK|CLKDIS_DIST1CLK|CLKDIS_DISDACCLK);   // Disable unused clock
   ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);// Select CD0 for CPU clock - 16Mhz clock
	 ClkSel(CLK_CD0,CLK_CD0,CLK_CD0,CLK_CD7);               // Select CD0 for SPI clocks
	 DMAINIT();                                             // Initialize DMA controller
   SPI1INIT();                                            // Initialize SPI1 block 
	 EiCfg(EXTINT4,INT_EN,INT_RISE);                        // Enable EINT4 - used to triger transfer
	 NVIC_EnableIRQ(EINT4_IRQn);                            // Enable IRQ4 interrupt
   NVIC_EnableIRQ(DMA_SPI1_RX_IRQn);                      // SPI1 Rx DMA interrupt enable
   while (1)
   {
	    
	 DioTgl(pADI_GP1,0x8);	// Toggle P1.3
	 delay(0x60000);		// Delay routine	
     if (ucSPIDMAEnable ==1)
		 {
      //  SetCycleCtrl(SPI1RX_C,16,DMA_DSTINC_BYTE|DMA_SRCINC_NO|DMA_SIZE_BYTE|DMA_BASIC); // re-Enable SPI RX DMA for next transfer
			 DmaStructPtrInSetup(SPI1RX_C,16,uxSPI1RxData);;    // Setup source/destination pointer fields
			 DmaCycleCntCtrl(SPI1RX_C,16,DMA_DSTINC_BYTE|
			    DMA_SRCINC_NO|DMA_SIZE_BYTE|DMA_BASIC);         // re-Enable SPI RX DMA for next transfer
			 DmaClr(DMARMSKCLR_SPI1RX,0,0,0);
	     DmaSet(0,DMAENSET_SPI1RX,0,
 	        DMAPRISET_SPI1RX);                              // Enable SPI1 DMA primary structure
			 ucSPIDMAEnable = 0;
     }			 
   }
}

void SPI1INIT(void)
{
	pADI_GP0->GPCON &= 0xFF00;                               
	pADI_GP0->GPCON |= 0x55; 					                      // Configure P0[3:0] for SPI1
	SpiBaud(pADI_SPI1,0x3F,SPIDIV_BCRST_DIS);               // Confiure SPI1 baud rate for 125kHz
	SpiCfg(pADI_SPI1,SPICON_MOD_TX1RX1,SPICON_MASEN_DIS,
	  SPICON_CON_EN|SPICON_SOEN_EN|SPICON_RXOF_EN|
	  SPICON_ZEN_EN|SPICON_CPHA_SAMPLETRAILING|
	  SPICON_ENABLE_EN);                                    // Enable SPI1 Slave mode, continuous transfer, 
	SpiDma(pADI_SPI1,SPIDMA_IENRXDMA_EN,SPIDMA_IENTXDMA_EN, // Enable SPI1 DMA operation - Tx and Rx
	   SPIDMA_ENABLE_EN);
	DmaPeripheralStructSetup(SPI1RX_C,DMA_DSTINC_BYTE|
	   DMA_SRCINC_NO|DMA_SIZE_BYTE);                        // Setup SPI1 Rx DMA structure
  DmaStructPtrInSetup(SPI1RX_C,16,uxSPI1RxData);          // Setup source/destination pointer fields
	DmaCycleCntCtrl(SPI1RX_C,16,DMA_DSTINC_BYTE|
	   DMA_SRCINC_NO|DMA_SIZE_BYTE|DMA_BASIC);              // Enable SPI RX DMA for next transfer
	DmaClr(DMARMSKCLR_SPI1RX,0,0,0);
	DmaSet(0,DMAENSET_SPI1RX,0,
 	     DMAPRISET_SPI1RX);                                 // Enable SPI1 DMA primary structure   
}
void DMAINIT(void)  
{
	DmaBase();
}
// Simple Delay routine
void delay (long int length)
{
	while (length >0)
    	length--;
}
void WakeUp_Int_Handler(void)
{
  
}
void Ext_Int2_Handler ()
{           

}
void Ext_Int4_Handler ()
{         
 
}
void GP_Tmr0_Int_Handler(void)
{

}

void GP_Tmr1_Int_Handler(void)
{
 
}
void ADC0_Int_Handler()
{
   
}
void ADC1_Int_Handler ()
{

}
void SINC2_Int_Handler()
{
 
}
void UART_Int_Handler ()
{

} 
void DMA_SPI1_RX_Int_Handler()
{
	DmaSet(DMARMSKSET_SPI1RX,DMAENSET_SPI1RX,0,0);
	ucIrqCnt++;
	ucSPIDMAEnable = 1;
} 
void I2C0_Slave_Int_Handler(void) 
{

}
void PWMTRIP_Int_Handler ()
{           
 
}
void PWM0_Int_Handler()
{

}
void PWM1_Int_Handler ()
{
 
}
void PWM2_Int_Handler()
{
  
}





