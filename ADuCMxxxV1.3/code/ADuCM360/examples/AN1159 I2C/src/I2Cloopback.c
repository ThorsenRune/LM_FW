/**
 *****************************************************************************
   @example  I2Cloopback.c
   @brief    I2C is configured in loopback mode.
   - The master and slave are configured to receive and transmit data.

   - It is not necessary to configure the GPIOs to operate in loopback mode.
   - In this example data and clock signals can be monitored:
   - SCL on P0.1 (mode 2)
   - SDA on P0.2 (mode 2)

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

void delay(int);

unsigned int uiMasterRxIndex = 0;
unsigned int uiMasterTxIndex = 0;
unsigned char ucMasterTxDat[5] = {0x55,2,0xAA,1,0x33};
unsigned char ucMasterRxDat[5] = {0,0,0,0,0};

unsigned int uiSlaveRxIndex = 0;
unsigned int uiSlaveTxIndex = 0;
unsigned char ucSlaveRxDat[5] = {0,0,0,0,0};
unsigned char ucSlaveTxDat[5] = {0x01, 0xFE, 0x02, 0xBC, 0x03};

unsigned char ucComplete = 0;

int main(void)
{
  WdtGo(T3CON_ENABLE_DIS);   
  //DioCfg(pADI_GP0, 0x28);     // Configure P0.1/P0.2 as I2C pins
  //DioPul(pADI_GP0, 0xF9);     // External pull ups required externally
  
  // configure the clocks
  ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);
  ClkSel(CLK_CD0,CLK_CD0,CLK_CD0,CLK_CD0);
  ClkDis(CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|CLKDIS_DISUARTCLK|CLKDIS_DISPWMCLK|CLKDIS_DIST0CLK|CLKDIS_DIST1CLK|CLKDIS_DISDACCLK|CLKDIS_DISDMACLK|CLKDIS_DISADCCLK);  
  
  I2cMCfg(0,I2CMCON_IENCMP|I2CMCON_IENTX_EN|I2CMCON_IENRX_EN,I2CMCON_LOOPBACK_EN|I2CMCON_MAS_EN);
  I2cBaud(0x4E,0x4F); // 100kHz clock
  NVIC_EnableIRQ(I2CM_IRQn);
  
    
  I2cSCfg(0,I2CSCON_IENTX_EN|I2CSCON_IENRX_EN,I2CSCON_SLV_EN);
  I2cSIDCfg(0xA0,0,0,0); // using 8-bit address in this example
  NVIC_EnableIRQ(I2CS_IRQn);
	
  // Transmit
  I2cMWrCfg(0xA0);
  I2cTx(MASTER, ucMasterTxDat[uiMasterTxIndex++]);            		// send 1st data
	 
  while (!ucComplete){}
  ucComplete = 0;
  delay(2000);	 

  // Receive
  I2cTx(SLAVE, ucSlaveTxDat[uiSlaveTxIndex++]);   // load for byte of data in slave Tx FIFO
  I2cMRdCfg(0xA0, sizeof(ucMasterRxDat), DISABLE);

  while (!ucComplete){}
  ucComplete = 0;
  while (1){}
}

// Simple Delay routine
void delay (int length)
{
  while (length >0)
    length--;
}


///////////////////////////////////////////////////////////////////////////
// I2C0 master handler 
///////////////////////////////////////////////////////////////////////////
void I2C0_Master_Int_Handler(void) {
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
      I2cMCfg(0, I2CMCON_IENCMP|I2CMCON_IENRX|I2CMCON_IENTX_DIS, I2CMCON_LOOPBACK_EN|I2CMCON_MAS_EN);    // TXREQ disabled to avoid multiple unecessary interrupts   
  }	
    
  if((uiStatus & I2CMSTA_TCOMP_SET) == I2CMSTA_TCOMP_SET) // communication complete	
  {
    I2cMCfg(0,I2CMCON_IENCMP|I2CMCON_IENTX_EN|I2CMCON_IENRX_EN,I2CMCON_LOOPBACK_EN|I2CMCON_MAS_EN); // TXREQ enabled for future master transmissions    
    ucComplete = 1;
  }

}

///////////////////////////////////////////////////////////////////////////
// I2C0 slave handler 
///////////////////////////////////////////////////////////////////////////
void I2C0_Slave_Int_Handler(void) 
{
  unsigned int uiStatus; 
  uiStatus = I2cSta(SLAVE);
	
  if ((uiStatus & I2CSSTA_RXREQ)==I2CSSTA_RXREQ)   // Slave Receive IRQ
  {
    ucSlaveRxDat[uiSlaveRxIndex++] = I2cRx(SLAVE);
    if(uiSlaveRxIndex > (sizeof(ucSlaveRxDat)-1))  // Resetting value of i if it has been incremented by RX
      uiSlaveRxIndex = 0;
  }

  if ((uiStatus & I2CSSTA_TXREQ)==I2CSSTA_TXREQ)   // Slave Transmit IRQ
  {
    if (uiSlaveTxIndex < sizeof(ucSlaveTxDat))
      I2cTx(SLAVE, ucSlaveTxDat[uiSlaveTxIndex++]);
  }
}



