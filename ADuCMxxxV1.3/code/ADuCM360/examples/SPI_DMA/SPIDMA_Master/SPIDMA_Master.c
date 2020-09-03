/**
 *****************************************************************************
   @example  SPIDMA_Master.c
   @brief    SPI1 master DMA example
      16-byte array is transmitted when EINT4 is asserted

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
unsigned char uxSPI1WrData[]={0x1,0x2,0x3,0x4,0x5,0x6,0x7,
	           0x8,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18};

int main (void)
{
   WdtCfg(T3CON_PRE_DIV1,T3CON_IRQ_EN,T3CON_PD_DIS);      // Disable Watchdog timer resets
   DioOenPin(pADI_GP1,PIN3, 1);                           // Set P1.3 as an output to toggle the LED
   //Disable clock to unused peripherals
   ClkDis(CLKDIS_DISI2CCLK|CLKDIS_DISPWMCLK|
      CLKDIS_DIST0CLK|CLKDIS_DIST1CLK|CLKDIS_DISDACCLK);  // Disable unused clock
   ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);// Select CD0 for CPU clock - 16Mhz clock
   ClkSel(CLK_CD0,CLK_CD0,CLK_CD0,CLK_CD7);               // Select CD0 for SPI clocks
   DMAINIT();                                             // Initialize DMA controller
   SPI1INIT();                                            // Initialize SPI1 block 
   EiCfg(EXTINT4,INT_EN,INT_RISE);                        // Enable EINT4 - used to triger transfer
   NVIC_EnableIRQ(EINT4_IRQn);                            // Enable IRQ4 interrupt
   NVIC_EnableIRQ(DMA_SPI1_TX_IRQn);                      // SPI1 Tx DMA interrupt enable
   
   while (1)
   {
   
   DioTgl(pADI_GP1,0x8);   // Toggle P1.3
   delay(0x60000);         // Delay routine
      if (ucSPIDMAEnable ==1)
      {
         //SetCycleCtrl(SPI1TX_C,16,DMA_DSTINC_NO|DMA_SRCINC_BYTE|DMA_SIZE_BYTE|DMA_BASIC);
         DmaStructPtrOutSetup(SPI1TX_C,16,uxSPI1WrData);   // Setup source/destination pointer fields
         DmaCycleCntCtrl(SPI1TX_C,16,DMA_DSTINC_NO|
         DMA_SRCINC_BYTE|DMA_SIZE_BYTE|DMA_BASIC);       // Setup SPI1TX structure chnl_cfg fields and number of values
         ucSPIDMAEnable = 0;
      }
   }
}

void SPI1INIT(void)
{                    
   DioCfgPin(pADI_GP0, PIN0, 1);                                // Configure P0[3:0] for SPI1
   DioCfgPin(pADI_GP0, PIN1, 1);
   DioCfgPin(pADI_GP0, PIN2, 1);
   DioCfgPin(pADI_GP0, PIN3, 1);
   SpiBaud(pADI_SPI1,0x3F,SPIDIV_BCRST_DIS);                   // Confiure SPI1 baud rate for 125kHz
   SpiCfg(pADI_SPI1,SPICON_MOD_TX1RX1,SPICON_MASEN_EN,
      SPICON_CON_EN|SPICON_RXOF_EN|SPICON_ZEN_EN|
      SPICON_TIM_TXWR|SPICON_CPHA_SAMPLETRAILING|
      SPICON_ENABLE_EN);                                   // Enable SPI1 master mode, continuous transfer, start on write to Tx
   SpiDma(pADI_SPI1,SPIDMA_IENRXDMA_EN,SPIDMA_IENTXDMA_EN, // Enable SPI1 DMA operation - Tx and Rx
      SPIDMA_ENABLE_EN);
   DmaPeripheralStructSetup(SPI1TX_C,DMA_DSTINC_NO|
      DMA_SRCINC_BYTE|DMA_SIZE_BYTE);                      // Setup SPI1 Tx DMA structure
   DmaStructPtrOutSetup(SPI1TX_C,16,uxSPI1WrData);
   DmaCycleCntCtrl(SPI1TX_C,16,DMA_DSTINC_NO|
         DMA_SRCINC_BYTE|DMA_SIZE_BYTE|DMA_BASIC);

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
 // pADI_SPI1->SPITX= uxSPI1WrData[0];
   DmaClr(DMARMSKCLR_SPI1TX,0,0,0);
   DmaSet(0,DMAENSET_SPI1TX,0,
      DMAPRISET_SPI1TX);                                  // Enable SPI1 DMA primary structure
   EiClr(EXTINT4);                                          // Clear EINT4 interrupt flag
  
}

void UART_Int_Handler ()
{

} 
void DMA_SPI1_TX_Int_Handler()
{
   DmaSet(DMARMSKSET_SPI1TX,DMAENSET_SPI1TX,0,0);
   ucIrqCnt++;
   if (ucIrqCnt > 1) // Ignore first interrupt
      ucSPIDMAEnable = 1;
}



