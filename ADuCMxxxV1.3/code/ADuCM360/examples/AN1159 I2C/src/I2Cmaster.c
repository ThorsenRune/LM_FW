/**
 *****************************************************************************
   @example  I2Cmaster.c
   @brief    I2C is configured in master mode.
   - SCL on P0.1 (mode 2)
   - SDA on P0.2 (mode 2)

   - For use with I2Cslave.c 

   @version  V0.2
   @author   ADI
   @date     October 2012 

   @par Revision History:
   - V0.1, May 2012: initial version. 
   - V0.2, October 2012: using new version of ClkLib

All files for ADuCM360/361 provided by ADI, including this file, are
provided  as is without warranty of any kind, either expressed or implied.
The user assumes any and all risk from the use of this code.
It is the responsibility of the person integrating this code into an application
to ensure that the resulting application performs as required and is safe.

**/
#include "ADuCM360.h"
#include <..\common\ClkLib.h>
#include <..\common\IntLib.h>
#include <..\common\DioLib.h>
#include <..\common\WdtLib.h>
#include <..\common\DioLib.h>
#include <..\common\I2cLib.h>

unsigned int uiMasterRxIndex = 0;
unsigned int uiMasterTxIndex = 0;
unsigned char ucMasterTxDat[5] = {0x55,2,0xAA,1,0x33};
unsigned char ucMasterRxDat[5] = {0,0,0,0,0};
unsigned char ucComplete = 0;

void delay(int);

int main(void)
{
  WdtGo(T3CON_ENABLE_DIS); 
  
  DioCfg(pADI_GP0, 0x28);     // Configure P0.1/P0.2 as I2C pins
  DioPul(pADI_GP0, 0xF9);     // External pull ups required externally
  
  // configure the clocks
  ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);
  ClkSel(CLK_CD0,CLK_CD0,CLK_CD0,CLK_CD0);
  ClkDis(CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|CLKDIS_DISUARTCLK|CLKDIS_DISPWMCLK|CLKDIS_DIST0CLK|CLKDIS_DIST1CLK|CLKDIS_DISDACCLK|CLKDIS_DISDMACLK|CLKDIS_DISADCCLK);  

  // Enable I2C Master mode, baud rate and interrupt sources
  I2cMCfg(I2CMCON_TXDMA_DIS|I2CMCON_RXDMA_DIS, I2CMCON_IENCMP|I2CMCON_IENRX|I2CMCON_IENTX, I2CMCON_MAS_EN); 
  I2cBaud(0x4E,0x4F); // 100kHz clock
  	
  NVIC_EnableIRQ(I2CM_IRQn);
  
  // Transmit
  uiMasterTxIndex = 0;
  I2cTx(MASTER, ucMasterTxDat[uiMasterTxIndex++]);            		// send 1st data
  I2cMWrCfg(0xA0);
  
  while (1)
  {
    while (!ucComplete){}
    ucComplete = 0;
    delay(20000);
  
    // Transmit
    uiMasterTxIndex = 0;
    I2cTx(MASTER, ucMasterTxDat[uiMasterTxIndex++]);            		// send 1st data
    I2cMWrCfg(0xA0);
    while (!ucComplete){}
    ucComplete = 0;
    delay(20000);
  
    // Receive
    uiMasterRxIndex = 0;
    I2cMRdCfg(0xA0, sizeof(ucMasterRxDat), DISABLE);
    I2cMRdCfg(0xA0, sizeof(ucMasterRxDat), DISABLE);
  }
}


///////////////////////////////////////////////////////////////////////////
// I2C0 master handler 
///////////////////////////////////////////////////////////////////////////
void I2C0_Master_Int_Handler(void) 
{
  unsigned int uiStatus;
  uiStatus = I2cSta(MASTER);

  if((uiStatus & I2CMSTA_RXREQ) == I2CMSTA_RXREQ)	// Master Recieve IRQ
  {
    ucMasterRxDat[uiMasterRxIndex++] = I2cRx(MASTER);    
    if(uiMasterRxIndex > (sizeof(ucMasterRxDat)-1) ) 			 // Resetting value of i if it has been incremented by RX
      uiMasterRxIndex = 0;
  }
  
  if((uiStatus & I2CMSTA_TXREQ) == I2CMSTA_TXREQ) // Master Transmit IRQ	
  {
    if (uiMasterTxIndex < sizeof(ucMasterTxDat) )
      I2cTx(MASTER, ucMasterTxDat[uiMasterTxIndex++]);
    else
      I2cMCfg(I2CMCON_TXDMA_DIS|I2CMCON_RXDMA_DIS, I2CMCON_IENCMP|I2CMCON_IENRX|I2CMCON_IENTX_DIS, I2CMCON_MAS_EN);    // TXREQ disabled to avoid multiple unecessary interrupts   
  }	
  if((uiStatus & I2CMSTA_TCOMP_SET) == I2CMSTA_TCOMP_SET) // communication complete	
  {
    I2cMCfg(I2CMCON_TXDMA_DIS|I2CMCON_RXDMA_DIS, I2CMCON_IENCMP|I2CMCON_IENRX|I2CMCON_IENTX_EN, I2CMCON_MAS_EN);   // TXREQ enabled for future master transmissions    
    ucComplete = 1;
  }
}


// Simple Delay routine
void delay (int length)
{
  while (length >0)
    length--;
}
