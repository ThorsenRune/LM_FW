/**
 *****************************************************************************
   @example  I2Cslave.c
   @brief    I2C is configured in master mode.
   - SCL on P0.1 (mode 2)
   - SDA on P0.2 (mode 2)

   - For use with I2Cmaster.c 

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

unsigned int uiSlaveRxIndex = 0;
unsigned int uiSlaveTxIndex = 0;
unsigned char ucSlaveRxDat[5] = {0,0,0,0,0};
unsigned char ucSlaveTxDat[5] = {0x01, 0xFE, 0x02, 0xBC, 0x03};


int main()
{
  WdtGo(T3CON_ENABLE_DIS);   
  DioCfg(pADI_GP0, 0x28);     // Configure P0.1/P0.2 as I2C pins
  DioPul(pADI_GP0, 0xF9);     // External pull ups required externally
  
  // configure the clocks
  ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);
  ClkSel(CLK_CD0,CLK_CD0,CLK_CD0,CLK_CD0);
  ClkDis(CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|CLKDIS_DISUARTCLK|CLKDIS_DISPWMCLK|CLKDIS_DIST0CLK|CLKDIS_DIST1CLK|CLKDIS_DISDACCLK|CLKDIS_DISDMACLK|CLKDIS_DISADCCLK);  
  
  // Enable I2C Slave mode
  I2cSCfg(0,I2CSCON_IENSTOP|I2CSCON_IENTX_EN|I2CSCON_IENRX,I2CSCON_SLV_EN);
  I2cSIDCfg(0xA0,0,0,0); // using 8-bit address in this example
  I2cTx(SLAVE, ucSlaveTxDat[uiSlaveTxIndex++]);
  	
  NVIC_EnableIRQ(I2CS_IRQn);
	
  while (1){}
}


///////////////////////////////////////////////////////////////////////////
// I2C0 slave handler 
///////////////////////////////////////////////////////////////////////////
void I2C0_Slave_Int_Handler(void) 
{
  unsigned int uiStatus; 
  uiStatus = I2cSta(SLAVE);
	
  if ((uiStatus & I2CSSTA_RXREQ)==I2CSSTA_RXREQ)   // Slave Recieve IRQ
  {
  pADI_GP1->GPTGL = BIT0;
    ucSlaveRxDat[uiSlaveRxIndex++] = I2cRx(SLAVE);
    if(uiSlaveRxIndex > (sizeof(ucSlaveRxDat)-1))  // Resetting value of i if it has been incremented by RX
      uiSlaveRxIndex = 0;
  }

  if ((uiStatus & I2CSSTA_TXREQ)==I2CSSTA_TXREQ)   // Slave Transmit IRQ
  {
    if (uiSlaveTxIndex < sizeof(ucSlaveTxDat))
      I2cTx(SLAVE, ucSlaveTxDat[uiSlaveTxIndex++]);
    else 			 
      uiSlaveTxIndex = 0;
  }
  
  if ((uiStatus & I2CSSTA_STOP)==I2CSSTA_STOP)   // stop condition
  {
    I2cSCfg(0,I2CSCON_IENSTOP|I2CSCON_IENTX_EN|I2CSCON_IENRX,I2CSCON_SLV_EN);  
  }
  
  if ((uiStatus & I2CSSTA_RXOF)==I2CSSTA_RXOF)   // Slave Recieve IRQ
  {
  }
  
}
