/**
 ***************************************************************************** 
   @example  AD5421.c
   @brief    This file contains functions written for the ADuCM360 that excercie the AD5421 via the SPI1 bus.
   @version  V0.2
   @author   ADI
   @date     January 2013

   @par Revision History:
   - V0.1, May 2012: initial version. 
   - V0.2, January 2013: added Doxygen comments


All files for ADuCM360/361 provided by ADI, including this file, are
provided  as is without warranty of any kind, either expressed or implied.
The user assumes any and all risk from the use of this code.
It is the responsibility of the person integrating this code into an application
to ensure that the resulting application performs as required and is safe.

**/

#include <stdio.h>
#include <string.h>
#include <ADuCM360.h>
#include <..\common\SpiLib.h>
#include "AD5421.h"
 
 // Init. ADuCM360 to AD5421 interface.
 // If using ADuCM360EBZ evaluation board, ensure all S6 switches are "ON"
 extern unsigned char ucTxComplete;
 extern unsigned long ulRxData;
 void SPI1INIT(void);            // Init SPI1 Interface
 
 void SPI1INIT (void)
{
	pADI_GP0->GPCON &= 0xFF00;
	pADI_GP0->GPCON |= 0x55; 					  // Configure P0[3:0] for SPI1
	SpiBaud(pADI_SPI1,10,SPIDIV_BCRST_DIS);
//	SpiCfg(pADI_SPI1,SPICON_MOD_TX3RX3,SPICON_MASEN_EN,SPICON_CON_EN|SPICON_RXOF_EN|
//	       SPICON_ZEN_EN|SPICON_TIM_TXWR|SPICON_CPOL_HIGH|SPICON_CPHA_SAMPLETRAILING|SPICON_ENABLE_EN);
	SpiCfg(pADI_SPI1,SPICON_MOD_TX3RX3,SPICON_MASEN_EN,SPICON_CON_EN|
	    SPICON_RXOF_EN|SPICON_ZEN_EN|SPICON_TIM_TXWR|SPICON_CPHA_SAMPLETRAILING|SPICON_ENABLE_EN);
}
void AD5421INIT(void)
{
	pADI_GP0->GPOEN |= 0x20;					// Enable P0.5 as an output - controls /LDAC pin of AD5421
	pADI_GP0->GPCLR |= 0x20;					// Clear P0.5 to clear /LDAC=0
	pADI_GP1->GPPUL |= 0x4;					  // Enable Pull-up on P1.2 to read Fault pin level
	pADI_GP1->GPOEN &= 0xFC;					// Enable P1.2 as an input to read Fault pin level
	SPI1INIT();
}

 // Write to IDAC data register								
 void AD5421_WriteToDAC(unsigned long ulDACValue)
 {
	unsigned long ulCMD = 0;
	unsigned long ulValue = 0; 	

	SpiFifoFlush(pADI_SPI1,SPICON_TFLUSH_EN,SPICON_RFLUSH_DIS);
	ulCMD = ((WRITEDAC)<<16);
	ulValue = ulDACValue;
	ulValue = (ulValue & 0x0000FFFF);	 // Take 16-bit Register value
	ulValue |= ulCMD;					 // Append Command to bits 23-16.
	SpiTx(pADI_SPI1,(unsigned char)(ulValue >> 16));
	SpiTx(pADI_SPI1,(unsigned char)(ulValue >> 8));
	SpiTx(pADI_SPI1,(unsigned char)(ulValue));
   ucTxComplete = 0;
	while (ucTxComplete == 0) {}
	ucTxComplete = 0;
 }
 // Write to IDAC Control register		
 void AD5421_WriteToCon(unsigned long ulConValue)
 {
 	unsigned long ulCMD = 0;
	unsigned long ulValue = 0; 	
	SpiFifoFlush(pADI_SPI1,SPICON_TFLUSH_EN,SPICON_RFLUSH_DIS);
	ulCMD = ((WRITECON)<<16);
	ulValue = ulConValue;
	ulValue = (ulValue & 0x0000FFFF);	 // Take 16-bit Register value
	ulValue |= ulCMD;					 // Append Command to bits 23-16.
	SpiFifoFlush(pADI_SPI1,SPICON_TFLUSH_DIS,SPICON_RFLUSH_DIS);
  SpiTx(pADI_SPI1,(unsigned char)(ulValue >> 16));
	SpiTx(pADI_SPI1,(unsigned char)(ulValue >> 8));
	SpiTx(pADI_SPI1,(unsigned char)(ulValue));

	while (ucTxComplete == 0) {}
	ucTxComplete = 0;

 }
 // Write to IDAC Offset adjust register		
 void AD5421_WriteToOffAdj(unsigned long ulOffAdjValue)
 {

 }
 // Write to IDAC Gain adjust register
 void AD5421_WriteToGnAdj(unsigned long ulDACValue)	
 {
 }
 // Load the IDAC output
 void AD5421_LoadDac(void)
 {
 
 }
 // force Alarm condition on IDAC output								
 void AD5421_ForceAlarm(void)
 {
    unsigned long ulCMD = 0;

	 ulCMD = ((FORCEALARM)<<16);
	 SpiFifoFlush(pADI_SPI1,SPICON_TFLUSH_EN,SPICON_RFLUSH_DIS);
	 SpiTx(pADI_SPI1,(unsigned char)(ulCMD >> 16));
	 SpiTx(pADI_SPI1,(unsigned char)(ulCMD >> 8));
	 SpiTx(pADI_SPI1,(unsigned char)(ulCMD));
	 while (ucTxComplete == 0) {}
	 ucTxComplete = 0;

	 SpiFifoFlush(pADI_SPI1,SPICON_TFLUSH_EN,SPICON_RFLUSH_DIS);
	 SpiTx(pADI_SPI1,(unsigned char)(ulCMD >> 16));
	 SpiTx(pADI_SPI1,(unsigned char)(ulCMD >> 8));
	 SpiTx(pADI_SPI1,(unsigned char)(ulCMD));
 }
 // Reset AD5421							
 void AD5421_Reset(void)
 {
 	unsigned long ulCMD = 0;

	ulCMD = ((AD5421RESET)<<16);
	SpiFifoFlush(pADI_SPI1,SPICON_TFLUSH_EN,SPICON_RFLUSH_DIS);
//	SpiFifoFlush(pADI_SPI1,SPICON_TFLUSH_DIS,SPICON_RFLUSH_DIS);
	SpiTx(pADI_SPI1,(unsigned char)(ulCMD >> 16));
	SpiTx(pADI_SPI1,(unsigned char)(ulCMD >> 8));
	SpiTx(pADI_SPI1,(unsigned char)(ulCMD));
	while (ucTxComplete == 0) {}
	ucTxComplete = 0;

	SpiFifoFlush(pADI_SPI1,SPICON_TFLUSH_EN,SPICON_RFLUSH_DIS);
//	SpiFifoFlush(pADI_SPI1,SPICON_TFLUSH_DIS,SPICON_RFLUSH_DIS);
	SpiTx(pADI_SPI1,(unsigned char)(ulCMD >> 16));
	SpiTx(pADI_SPI1,(unsigned char)(ulCMD >> 8));
	SpiTx(pADI_SPI1,(unsigned char)(ulCMD));
 }
 // Measure Vloop or die temp via ADC								
 void AD5421_InitADC(void)
 {
 
 }
 // Read to IDAC data register								
 unsigned long AD5421_ReadDAC(unsigned long ulDACValue)
 {
  	unsigned long ulCMD = 0;
	unsigned long ulValue = 0;
	unsigned long ulData = 0; 	
	
	SpiFifoFlush(pADI_SPI1,SPICON_TFLUSH_EN,SPICON_RFLUSH_EN);

	ulCMD = ((READDAC)<<16);
	ulValue = ulDACValue;
	ulValue = (ulValue & 0x0000FFFF);	 // Take 16-bit Register value
	ulValue |= ulCMD;					 // Append Command to bits 23-16.
	
	SpiTx(pADI_SPI1,(unsigned char)(ulValue >> 16));
	SpiTx(pADI_SPI1,(unsigned char)(ulValue >> 8));
	SpiTx(pADI_SPI1,(unsigned char)(ulValue));
	while (ucTxComplete == 0) {}
	ucTxComplete = 0;

	ulData = ulRxData;
	
	return ulData;
 }	
 // Read IDAC Control register		
 unsigned long AD5421_ReadCon(unsigned long ulConValue)
 {
 	unsigned long ulCMD = 0;
	unsigned long ulValue = 0; 	
	unsigned long ulData = 0;
	
	SpiFifoFlush(pADI_SPI1,SPICON_TFLUSH_EN,SPICON_RFLUSH_EN);
	ulCMD = ((READCON)<<16);
	ulValue = ulConValue;
	ulValue = (ulValue & 0x0000FFFF);	 // Take 16-bit Register value
	ulValue |= ulCMD;					 // Append Command to bits 23-16.

  SpiTx(pADI_SPI1,(unsigned char)(ulValue >> 16));
	SpiTx(pADI_SPI1,(unsigned char)(ulValue >> 8));
	SpiTx(pADI_SPI1,(unsigned char)(ulValue));
	while (ucTxComplete == 0) {}
	ucTxComplete = 0;

	ulData = ulRxData;

	return ulData;
 }		
 // Read IDAC Offset adjust register	
 void AD5421_ReadOffAdj(unsigned long ulOffAdjValue)
 {
 
 }	
 // Read IDAC Gain adjust register
 void AD5421_ReadGnAdj(unsigned long ulDACValue)
 {
 
 }
 // Read Fault register		
 unsigned long  AD5421_ReadFault(unsigned long ulDACValue)
 {
 	unsigned long ulCMD = 0;
	unsigned long ulValue = 0;
	unsigned long ulData = 0; 	
	
	SpiFifoFlush(pADI_SPI1,SPICON_TFLUSH_EN,SPICON_RFLUSH_DIS);
	SpiFifoFlush(pADI_SPI1,SPICON_TFLUSH_DIS,SPICON_RFLUSH_DIS);
	ulCMD = ((READFAULT)<<16);
	ulValue = ulDACValue;
	ulValue = (ulValue & 0x0000FFFF);	 // Take 16-bit Register value
	ulValue |= ulCMD;					 // Append Command to bits 23-16.

  SpiTx(pADI_SPI1,(unsigned char)(ulValue >> 16));
	SpiTx(pADI_SPI1,(unsigned char)(ulValue >> 8));
	SpiTx(pADI_SPI1,(unsigned char)(ulValue));
	while (ucTxComplete == 0) {}
	ucTxComplete = 0;

	SpiFifoFlush(pADI_SPI1,SPICON_TFLUSH_DIS,SPICON_RFLUSH_EN);
	SpiFifoFlush(pADI_SPI1,SPICON_TFLUSH_DIS,SPICON_RFLUSH_DIS);
	SpiTx(pADI_SPI1,0);
	SpiTx(pADI_SPI1,0);
	SpiTx(pADI_SPI1,0);
	while (ucTxComplete == 0) {}
	ulData = ulRxData;
	
	return ulData;
 }		
 

 