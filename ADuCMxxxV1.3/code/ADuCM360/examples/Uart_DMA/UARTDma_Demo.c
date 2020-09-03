/**
 *****************************************************************************
   @example  UARTDma_Demo.c
   @brief    UART DMA example
	Trigerring external IRQ4 sends the number of times the IRQ was triggered
   over the uart

   @version V0.2
   @author  ADI
   @date    February 2013

   @par     Revision History:
   - V0.1, October 2012: initial version. 
   - V0.2, February 2013: Removed unused function SendString() and variable 
   ucTxBufferEmpty.
              
All files for ADuCM360/361 provided by ADI, including this file, are
provided  as is without warranty of any kind, either expressed or implied.
The user assumes any and all risk from the use of this code.
It is the responsibility of the person integrating this code into an application
to ensure that the resulting application performs as required and is safe.


**/



#include <stdio.h>
#include <string.h>
#include <ADuCM360.h>

#include <..\common\UrtLib.h>
#include <..\common\ClkLib.h>
#include <..\common\WdtLib.h>
#include <..\common\IntLib.h>
#include <..\common\DmaLib.h>
#include <..\common\DioLib.h>

void UARTINIT (void);
void SendResultToUART(void);                         // Send String to UART - in ASCII String format
void delay(long int);
void DMAINIT(void);
// UART-based external variables
unsigned char szTemp[64] = "";                       // Used to store string before printing to UART
unsigned char szPacketIn[32];	                       // Used to store received bytes from UART
unsigned char ucPacketRcvd = 0;                      // Flag to indicate Received packet
unsigned char nLen = 0;
unsigned char i = 0;
unsigned char ucSendString  = 0;                     // Used to trigger sending string to UART
unsigned char ucIRQCnt = 0;                          // Counts number of IRQ4 interrutps
int main (void)
{
   //unsigned int szString[64];
   DioOen(pADI_GP1,0x8);                             // Set P1.3 as an output for test purposes
   WdtCfg(T3CON_PRE_DIV1,T3CON_IRQ_EN,T3CON_PD_DIS); // Disable Watchdog timer resets
   //Disable clock to unused peripherals
   ClkDis(CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|CLKDIS_DISI2CCLK|CLKDIS_DISPWMCLK|CLKDIS_DIST0CLK|CLKDIS_DIST1CLK); // Only enable clock to used blocks
   ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN,CLK_UCLKCG);// Select CD0 for CPU clock with 8MHz UCLK (div2 enabled on oscillator output)
   ClkSel(CLK_CD7,CLK_CD7,CLK_CD0,CLK_CD7);           // Select CD0 for UART System clock
	 DMAINIT();                                         // Initialize DMA controller
   UARTINIT();                                        // Init Uart
   EiCfg(EXTINT4,INT_EN,INT_RISE);                    // Enable EINT4 - used to triger transfer
	 NVIC_EnableIRQ(EINT4_IRQn);                        // Enable IRQ4 interrupt
   NVIC_EnableIRQ(DMA_UART_TX_IRQn);                  // Enable UART Tx DMA interrupts
   NVIC_EnableIRQ(DMA_UART_RX_IRQn);                  // Enable UART Rx DMA interrupts
   ucSendString = 0;
   while (1)
   {
      if (ucSendString == 1)
			{
		     ucSendString = 0;				
  				sprintf ( (char*)szTemp, "External IRQ4 count: %d \r\n",ucIRQCnt ); 
         nLen = strlen((char*)szTemp);
         if (nLen <64)
				 {
          
					 DmaStructPtrOutSetup(UARTTX_C,64,szTemp);  // Setup UARTTX source/destination pointers and number of bytes to send
					 DmaCycleCntCtrl(UARTTX_C,64,DMA_DSTINC_NO| // Setup CHNL_CFG settings
				   	  DMA_SRCINC_BYTE|DMA_SIZE_BYTE|DMA_BASIC);
					 DmaClr(DMARMSKCLR_UARTTX,0,0,0);           // Disable masking of UARTTX DMA channel
	         DmaSet(0,DMAENSET_UARTTX,0,                // Enable UART TX DMA channel
 	            DMAPRISET_UARTTX);                      // Enable UART DMA primary structure
					 UrtTx(pADI_UART,0);                        // Send byte 0 to the UART to start transfer
				 }
			 }
			 if (ucPacketRcvd == 1)
			 {
				 ucPacketRcvd = 0;
				 // Send out received packet first
				 DmaStructPtrOutSetup(UARTTX_C,32,szPacketIn);//Setup UARTRX source/destination pointers and number of bytes to receive
				 DmaCycleCntCtrl(UARTTX_C,32,DMA_DSTINC_NO|   // Setup CHNL_CFG settings
				   	  DMA_SRCINC_BYTE|DMA_SIZE_BYTE|DMA_BASIC);
					 DmaClr(DMARMSKCLR_UARTTX,0,0,0);           // Enable UART RX DMA channel
	         DmaSet(0,DMAENSET_UARTTX,0,
 	            DMAPRISET_UARTTX);                      // Enable UART DMA primary structure
					 UrtTx(pADI_UART,0);                        // Send byte 0 to the UART to start transfer
				 // re-enable UARTRX DMA
				 	DmaStructPtrInSetup(UARTRX_C,32,szPacketIn);
	        DmaCycleCntCtrl(UARTRX_C,32,DMA_DSTINC_BYTE|
		        DMA_SRCINC_NO|DMA_SIZE_BYTE|DMA_BASIC);
	        DmaClr(DMARMSKCLR_UARTRX,0,0,0);
	        DmaSet(0,DMAENSET_UARTRX,0,0);              // Enable UART DMA primary structure
			 }
      delay(0x60000);		// Delay routine
			DioTgl(pADI_GP1,0x8);                           // Toggle LED, P1.3
   }
}

void UARTINIT (void)
{
   //Select IO pins for UART.
   pADI_GP0->GPCON |= 0x3C;                           // Configure P0.1/P0.2 for UART		
//    pADI_GP0->GPCON |= 0x9000;                      // Configure P0.6/P0.7 for UART
   UrtCfg(pADI_UART,B9600,COMLCR_WLS_8BITS,0);        // setup baud rate for 9600, 8-bits
   UrtMod(pADI_UART,COMMCR_DTR,0);                    // Setup modem bits
   UrtIntCfg(pADI_UART,COMIEN_ERBFI|COMIEN_ETBEI|
	   COMIEN_ELSI|COMIEN_EDSSI|COMIEN_EDMAT|
	   COMIEN_EDMAR);                                   // Setup UART IRQ sources
	UrtDma(pADI_UART,COMIEN_EDMAT|COMIEN_EDMAR);        // Enable UART DMA interrupts	
	DmaPeripheralStructSetup(UARTTX_C,DMA_DSTINC_NO|    // Enable DMA write channel
	    DMA_SRCINC_BYTE|DMA_SIZE_BYTE);
	DmaPeripheralStructSetup(UARTRX_C,DMA_DSTINC_BYTE|  // Enable DMA UART Read channel
	    DMA_SRCINC_NO|DMA_SIZE_BYTE);
	DmaStructPtrInSetup(UARTRX_C,32,szPacketIn);
	DmaCycleCntCtrl(UARTRX_C,32,DMA_DSTINC_BYTE|
		DMA_SRCINC_NO|DMA_SIZE_BYTE|DMA_BASIC);
	DmaClr(DMARMSKCLR_UARTRX,0,0,0);
	DmaSet(0,DMAENSET_UARTRX,0,0);                      // Enable UART RX DMA primary structure
	
}
void DMAINIT(void)  
{
	DmaBase();
}

void WakeUp_Int_Handler(void)
{
  
}
void Ext_Int2_Handler ()
{           

}
void Ext_Int4_Handler ()
{         
   	EiClr(EXTINT4); 
	  ucSendString = 1;
	  ucIRQCnt++;
	  
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
void DMA_UART_TX_Int_Handler()
{
	volatile unsigned char ucCOMSTA0 = 0;
   volatile unsigned char ucCOMIID0 = 0;

   ucCOMIID0 = UrtIntSta(pADI_UART);                // Read UART Interrupt ID register 
	 DmaSet(DMARMSKSET_UARTTX,DMAENSET_UARTTX,0,0);   // MASK UART DMA channel to prevent further interrupts triggering - unmask when ready to re-transmit
  
} 
void DMA_UART_RX_Int_Handler()
{
	volatile unsigned char ucCOMSTA0 = 0;
   volatile unsigned char ucCOMIID0 = 0;

  DmaSet(DMARMSKSET_UARTRX,DMAENSET_UARTRX,0,0);    // MASK UART DMA RX channel to prevent further interrupts triggering - unmask when ready to re-transmit 
	ucCOMIID0 = UrtIntSta(pADI_UART);                 // Read UART Interrupt ID register 
	 ucPacketRcvd = 1;                                // Flag to indicate packet received
	 
  
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
// Simple Delay routine
void delay (long int length)
{
	while (length >0)
    	length--;
}




