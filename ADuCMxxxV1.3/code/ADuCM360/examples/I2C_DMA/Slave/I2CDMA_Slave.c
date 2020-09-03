/**
 *****************************************************************************
   @example  I2CDMA_Slave.c
   @brief    I2C Slave DMA example. 
   - 16-byte array is set up to receive 
   - 16-byte array also setup for read requests

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
#include <DmaLib.h>
#include <i2cLib.h>

void delay(long int);
void I2CSLAVEINIT(void);
void DMAINIT(void);

unsigned char ucIrqRxCnt,ucIrqTxCnt = 0;                 // Used to count number of I2C DMA interrupts
unsigned char ucI2CDMARxEnable = 0;                      // Flag used to indicate I2X Slave rx interrupt occured
unsigned char ucI2CDMATxEnable = 0;                      // Flag used to indicate I2X Slave Tx interrupt occured
unsigned char uxI2CRxData[16];
unsigned char ucRxCnt = 0;
unsigned char uxI2CWrData[]={0x22,0x23,0x24,0x24,0x25,0x26,0x27,
	           0x38,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
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
   I2CSLAVEINIT();                                        // Initialize I2C Slave block 
	 EiCfg(EXTINT4,INT_EN,INT_RISE);                        // Enable EINT4 - used to triger transfer
	 NVIC_EnableIRQ(EINT4_IRQn);                            // Enable IRQ4 interrupt
   NVIC_EnableIRQ(DMA_I2CS_TX_IRQn);                      // I2C Slave Tx DMA interrupt enable
	 NVIC_EnableIRQ(DMA_I2CS_RX_IRQn);                      // I2C Slave Rx DMA interrupt enable
   while (1)
   {
	    
	 DioTgl(pADI_GP1,0x8);	// Toggle P1.3
	 delay(0x60000);		// Delay routine	
     if (ucI2CDMARxEnable ==1)
		 {
			  DmaStructPtrInSetup(I2CSRX_C,16,uxI2CRxData);     // Setup destination pointer fields
			  DmaCycleCntCtrl(I2CSRX_C,16,DMA_DSTINC_BYTE|
			    DMA_SRCINC_NO|DMA_SIZE_BYTE|DMA_BASIC);         // Setup i2C Master TX structure chnl_cfg fields and number of values
			  ucI2CDMARxEnable = 0;
			  DmaClr(DMARMSKCLR_I2CSRX,0,0,0);                  // Disable I2C DMA channel mask bits
     }		
    if (ucI2CDMATxEnable == 1)
		{
			 	DmaStructPtrOutSetup(I2CSTX_C,16,uxI2CWrData);    // Setup destination pointer fields
	      DmaCycleCntCtrl(I2CSTX_C,16,DMA_DSTINC_NO|
	        DMA_SRCINC_BYTE|DMA_SIZE_BYTE|DMA_BASIC);       // Setup i2C Slave TX structure chnl_cfg fields and number of values
			 ucI2CDMATxEnable = 0;
			 DmaClr(DMARMSKCLR_I2CSTX,0,0,0);                  // Disable I2C DMA channel mask bits
		}
   }
}

void I2CSLAVEINIT(void)
{
	pADI_GP0->GPCON &= 0xFFC3;                               
	pADI_GP0->GPCON |= 0x28; 					                      // Configure P0[2:1] for I2C
	I2cSIDCfg(0xA0,0xA2,0xA4,0xA0);                         // Setup i2C network ID register
	I2cSCfg(I2CSCON_TXDMA|I2CSCON_RXDMA|I2CSCON_IENRX_EN,0,I2CSCON_SLV_EN);  // I2C Slave DMA enable
	
	// I2C Slave DMA rx structure
  DmaPeripheralStructSetup(I2CSRX_C,DMA_DSTINC_BYTE|
	   DMA_SRCINC_NO|DMA_SIZE_BYTE);                        // Setup I2C Slave Rx DMA structure
	DmaStructPtrInSetup(I2CSRX_C,16,uxI2CRxData);           // Setup destination pointer fields
	DmaCycleCntCtrl(I2CSRX_C,16,DMA_DSTINC_BYTE|
		 DMA_SRCINC_NO|DMA_SIZE_BYTE|DMA_BASIC);              // Setup i2C Slave RX structure chnl_cfg fields and number of values	
	DmaClr(DMARMSKCLR_I2CSRX|DMARMSKCLR_I2CSTX,0,0,0);      // Disable I2C DMA channel mask bits
	DmaSet(0,0|DMAENSET_I2CSRX|DMAENSET_I2CSRX,0,0);        // Enable I2C Slave DMA channels
	
		// I2C Slave DMA Tx structure
  DmaPeripheralStructSetup(I2CSTX_C,DMA_DSTINC_NO|
	   DMA_SRCINC_BYTE|DMA_SIZE_BYTE);                      // Setup I2C Slave Tx DMA structure
	DmaStructPtrOutSetup(I2CSTX_C,16,uxI2CWrData);          // Setup destination pointer fields
	DmaCycleCntCtrl(I2CSTX_C,16,DMA_DSTINC_NO|
	   DMA_SRCINC_BYTE|DMA_SIZE_BYTE|DMA_BASIC);            // Setup i2C Slave TX structure chnl_cfg fields and number of values	
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

	EiClr(EXTINT4);                                          // Clear EINT4 interrupt flag
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
void DMA_I2C0_SRX_Int_Handler()
{
	DmaSet(DMARMSKSET_I2CSRX,DMAENSET_I2CSRX,0,0);
	ucIrqRxCnt++;
	ucI2CDMARxEnable = 1;
} 
void DMA_I2C0_STX_Int_Handler()
{
	DmaSet(DMARMSKSET_I2CSTX,DMAENSET_I2CSTX,0,0);
	ucIrqTxCnt++;
	ucI2CDMATxEnable = 1;
} 
void I2C0_Slave_Int_Handler() 
{
   unsigned int uiStatus;
	
	uiStatus = I2cSta(SLAVE);
	if (uiStatus & 0x8)
	{
		uxI2CRxData[ucRxCnt++]= I2cRx(SLAVE);
		if (ucRxCnt == 16)
			ucRxCnt = 0;
	}
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





