/**
 *****************************************************************************
   @example  I2CDMA_Master.c
   @brief    I2C master DMA example.
   - 16-byte array is transmitted when EINT4 is asserted.
   - 16-byte Transmits triggered by EINT4.
   - 16-Byte receive triggered by EINT2.

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
#include <i2cLib.h>

void delay(long int);
void I2CMASTERINIT(void);
void DMAINIT(void);

unsigned char ucIrqRxCnt,ucIrqTxCnt = 0;                 // Used to count number of I2C DMA interrupts
unsigned char ucI2CDMATxEnable,ucI2CDMARxEnable = 0;     // Flags used to setup 2nd and subsequent transfers
unsigned char uxI2CWrData[]={0x1,0x2,0x3,0x4,0x5,0x6,0x7,// 16-byte array used for transmitting
	           0x8,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18};
unsigned char uxI2CRxData[16];                           // Used to receive i2C data

int main (void)
{
   WdtCfg(T3CON_PRE_DIV1,T3CON_IRQ_EN,T3CON_PD_DIS);      // Disable Watchdog timer resets
   DioOen(pADI_GP1,0x8);                                  // Set P1.3 as an output to toggle the LED
   //Disable clock to unused peripherals
   ClkDis(CLKDIS_DISPWMCLK|CLKDIS_DIST0CLK|CLKDIS_DIST1CLK
	     |CLKDIS_DISDACCLK);                                // Disable unused clock
   ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);// Select CD0 for CPU clock - 16Mhz clock
	 ClkSel(CLK_CD0,CLK_CD0,CLK_CD0,CLK_CD0);               // Select CD0 for I2C and UART clocks
	 DMAINIT();                                             // Initialize DMA controller
   I2CMASTERINIT();                                        // Initialize I2C Master block 
	 EiCfg(EXTINT4,INT_EN,INT_RISE);                        // Enable EINT4 - used to triger transfer
	 EiCfg(EXTINT2,INT_EN,INT_RISE);                        // Enable EINT2 - used to triger transfer
	 NVIC_EnableIRQ(EINT4_IRQn);                            // Enable IRQ4 interrupt
	 NVIC_EnableIRQ(EINT2_IRQn);                            // Enable IRQ2 interrupt
    NVIC_EnableIRQ(DMA_I2CM_TX_IRQn);                      // I2C Master Tx DMA interrupt enable
	 NVIC_EnableIRQ(DMA_I2CM_RX_IRQn);                      // I2C Master Rx DMA interrupt enable
   while (1)
   {
	    
	 DioTgl(pADI_GP1,0x8);	// Toggle P1.3
	 delay(0x60000);		// Delay routine	
     if (ucI2CDMATxEnable ==1)
		 {
			  delay(0xF0000);                                   // Delay only for deglitch purposes on external IRQ
           DmaStructPtrOutSetup(I2CMTX_C,16,uxI2CWrData);    // Setup destination pointer fields
			  DmaCycleCntCtrl(I2CMTX_C,16,DMA_DSTINC_NO|
			    DMA_SRCINC_BYTE|DMA_SIZE_BYTE|DMA_BASIC);       // Setup i2C Master TX structure chnl_cfg fields and number of values
			  I2cMWrCfg(0xA0);                                  // Initiate write to Slave address 0xA0
			  ucI2CDMATxEnable = 0;         
          NVIC_EnableIRQ(EINT4_IRQn);                        // Enable IRQ4 interrupt
     }	
     if (ucI2CDMARxEnable == 1)
		 {
          delay(0xA0000);                                    // Delay only for deglitch purposes on external IRQ
          ucI2CDMARxEnable = 0;
			 DmaStructPtrInSetup(I2CMRX_C,16,uxI2CRxData);      // Setup i2C master rx desitination address
			 DmaClr(DMARMSKCLR_I2CMRX,0,0,0);                   // Disable masking of I2C master rx channel
			 DmaCycleCntCtrl(I2CMRX_C,16,DMA_DSTINC_BYTE|
			   DMA_SRCINC_NO|DMA_SIZE_BYTE|DMA_BASIC);
          
          NVIC_EnableIRQ(EINT2_IRQn);                        // Enable IRQ2 interrupt
     }			 
   }
}

void I2CMASTERINIT(void)
{
	pADI_GP0->GPCON &= 0xFFC3;                               
	pADI_GP0->GPCON |= 0x28; 					                 // Configure P0[2:1] for I2C
	I2cBaud(0x4F,0x4E);                                     // Confiure I2C baud rate for 100kHz
	I2cMCfg(I2CMCON_TXDMA|I2CMCON_RXDMA,I2CMCON_IENCMP_EN|
	  I2CMCON_IENNACK_EN|I2CMCON_IENALOST_EN|
	  I2CMCON_IENTX_EN|I2CMCON_IENRX_EN,I2CMCON_MAS_EN);    // Enable I2C master mode, Enable DMA on tx and Rx
	
	// Setup I2C Master Tx DMA structure
	DmaPeripheralStructSetup(I2CMTX_C,DMA_DSTINC_NO|
	   DMA_SRCINC_BYTE|DMA_SIZE_BYTE);                      // Setup I2C Master Tx DMA structure
	// Setup I2C Master Rx DMA structure
	DmaPeripheralStructSetup(I2CMRX_C,DMA_DSTINC_BYTE|      // Setup I2C master Rx DMA structure - basic transfers
	   DMA_SRCINC_NO|DMA_SIZE_BYTE);
	DmaStructPtrInSetup(I2CMRX_C,16,uxI2CRxData);           // Setup i2C master Rx destination
	DmaCycleCntCtrl(I2CMRX_C,16,DMA_DSTINC_BYTE|
	   DMA_SRCINC_NO|DMA_SIZE_BYTE|DMA_BASIC);
	DmaClr(DMARMSKCLR_I2CMRX,0,0,0);                        // Disable masking of I2C master rx channel
	DmaSet(0,DMAENSET_I2CMRX,0,0);                          // Enable i2C master Rx DMA channel
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
   DmaClr(DMARMSKCLR_I2CMRX,0,0,0);
	DmaSet(0,DMAENSET_I2CMRX,0,0);                           // Enable I2C Master Rx DMA primary structure
   EiClr(EXTINT2);                                          // Clear EINT2 interrupt flag
	I2cMRdCfg(0xA0,16,DISABLE);                              // Initiate reading of 16 bytes from Slave 0xA0 -
   NVIC_DisableIRQ(EINT2_IRQn);                             // Disable IRQ2 interrupt - only for deglitch purpose  
}
void Ext_Int4_Handler ()
{         
	DmaClr(DMARMSKCLR_I2CMTX,0,0,0);
	DmaSet(0,DMAENSET_I2CMTX,0,DMAPRISET_I2CMTX);            // Enable I2C Master Tx DMA primary structure
	EiClr(EXTINT4);                                          // Clear EINT4 interrupt flag
   ucI2CDMATxEnable = 1;                                    // Set flag to call code that starts i2C transmit
   NVIC_DisableIRQ(EINT4_IRQn);                             // Disable IRQ4 interrupt - only for deglitch purpose
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
void DMA_I2C0_MTX_Int_Handler()
{
	DmaSet(DMARMSKSET_I2CMTX,DMAENSET_I2CMTX,0,0);
	ucIrqRxCnt++;
	
} 
void DMA_I2C0_MRX_Int_Handler(void) 
{
	DmaSet(DMARMSKSET_I2CMRX,DMAENSET_I2CMRX,0,0);
	ucIrqTxCnt++;
	ucI2CDMARxEnable = 1;
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





