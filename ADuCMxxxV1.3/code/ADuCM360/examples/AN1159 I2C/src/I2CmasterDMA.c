/**
 *****************************************************************************
   @example  I2CmasterDMA.c
   @brief    I2C is configured in master mode.
   - SCL on P0.1 (mode 2)
   - SDA on P0.2 (mode 2)

   - Configures the ADuCM301 for I2C DMA transfer. 
   - Code to use with I2Cslave.c

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

#define MAX_COUNT_TX   0x10
 
typedef struct ctrl_cfg 
{
   unsigned int cycle_ctrl:3;
   unsigned int next_useburst:1;
   unsigned int n_minus_1:10;
   unsigned int r_power:4;
   unsigned int src_prot_ctrl:3;
   unsigned int dst_prot_ctrl:3;
   unsigned int src_size:2;
   unsigned int src_inc:2;
   unsigned int dst_size:2;
   unsigned int dst_inc:2;
} CtrlCfg;

typedef struct dma_desc // Define the structure of a DMA descriptor.
{
   unsigned int srcEndPtr;
   unsigned int destEndPtr;
   CtrlCfg ctrlCfg;
   unsigned int reserved4Bytes;
} DmaDesc;

#define CCD_SIZE 16
#define DMACHAN_DSC_ALIGN 0x200

#if defined(__ARMCC_VERSION) || defined(__GNUC__)
DmaDesc dmaChanDesc     [CCD_SIZE * 2] __attribute__ ((aligned (DMACHAN_DSC_ALIGN)));      
#endif

#ifdef __ICCARM__
#pragma data_alignment=(DMACHAN_DSC_ALIGN)
DmaDesc dmaChanDesc     [CCD_SIZE * 2];
#endif
DmaDesc *I2CMTxDmaDesc, *I2CMRxDmaDesc;
  
volatile unsigned char ucTxBuffer[] = {0x01,0x02,0x03,0x04,0x05};
volatile unsigned char DMA_TRAN_COUNT = sizeof(ucTxBuffer);
volatile unsigned char ucRxBuffer[5];
volatile unsigned char DMA_RX_COUNT = sizeof(ucRxBuffer);

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
  ClkDis(CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|CLKDIS_DISUARTCLK|CLKDIS_DISPWMCLK|CLKDIS_DIST0CLK|CLKDIS_DIST1CLK|CLKDIS_DISDACCLK|CLKDIS_DISADCCLK);  

  // I2C master receive  
  I2CMRxDmaDesc = &dmaChanDesc[7];	
  I2CMRxDmaDesc->srcEndPtr = (unsigned int) &pADI_I2C->I2CMRX;
  I2CMRxDmaDesc->destEndPtr =  (unsigned int)(ucRxBuffer + DMA_RX_COUNT -1 );
  I2CMRxDmaDesc->ctrlCfg.dst_inc = 0;   // no increment as the dest is a reg
  I2CMRxDmaDesc->ctrlCfg.dst_size = 0;  // byte data
  I2CMRxDmaDesc->ctrlCfg.src_inc = 3;   // byte incr. 
  I2CMRxDmaDesc->ctrlCfg.src_size = 0;  // byte data
  I2CMRxDmaDesc->ctrlCfg.dst_prot_ctrl = 0;
  I2CMRxDmaDesc->ctrlCfg.src_prot_ctrl = 0;
  I2CMRxDmaDesc->ctrlCfg.r_power = 0;
  I2CMRxDmaDesc->ctrlCfg.n_minus_1 = DMA_RX_COUNT - 1;
  I2CMRxDmaDesc->ctrlCfg.next_useburst = 0;
  I2CMRxDmaDesc->ctrlCfg.cycle_ctrl = 1; //  Basic DMA transfer  

  // I2C master transmit  
  I2CMTxDmaDesc = &dmaChanDesc[6];	
  I2CMTxDmaDesc->srcEndPtr = (unsigned int)(ucTxBuffer + DMA_TRAN_COUNT -1 );
  I2CMTxDmaDesc->destEndPtr = (unsigned int) &pADI_I2C->I2CMTX; 
  I2CMTxDmaDesc->ctrlCfg.dst_inc = 3;   // no increment as the dest is a reg
  I2CMTxDmaDesc->ctrlCfg.dst_size = 0;  // byte data
  I2CMTxDmaDesc->ctrlCfg.src_inc = 0;   // byte incr. 
  I2CMTxDmaDesc->ctrlCfg.src_size = 0;  // byte data
  I2CMTxDmaDesc->ctrlCfg.dst_prot_ctrl = 0;
  I2CMTxDmaDesc->ctrlCfg.src_prot_ctrl = 0;
  I2CMTxDmaDesc->ctrlCfg.r_power = 0;
  I2CMTxDmaDesc->ctrlCfg.n_minus_1 = DMA_TRAN_COUNT - 1;
  I2CMTxDmaDesc->ctrlCfg.next_useburst = 0;
  I2CMTxDmaDesc->ctrlCfg.cycle_ctrl = 1; //  Basic DMA transfer  

  I2cMCfg(I2CMCON_TXDMA|I2CMCON_RXDMA,I2CMCON_IENCMP,I2CMCON_MAS_EN);
  I2cBaud(0x4E,0x4F); // 100kHz clock
  NVIC_EnableIRQ(DMA_I2CM_TX_IRQn);  
  NVIC_EnableIRQ(DMA_I2CM_RX_IRQn);  
  NVIC_EnableIRQ(I2CM_IRQn);  

  // I2C master transmit  
  I2cMWrCfg(0xA0);  // initiate DMA transfer
  pADI_DMA->DMAPDBPTR = (unsigned int) &dmaChanDesc; // Setup the DMA base pointer.
  pADI_DMA->DMAENSET = DMAENSET_I2CMTX; // Enable DMA channel  
  pADI_DMA->DMACFG = DMACFG_ENABLE_EN;   // Enable the  uDMA  

  while (!ucComplete){}
  ucComplete = 0;
  delay(2000);
  
  // master receive
  I2cMRdCfg(0xA0, DMA_RX_COUNT, 0);  // initiate DMA transfer
  pADI_DMA->DMAPDBPTR = (unsigned int) &dmaChanDesc; // Setup the DMA base pointer.
  pADI_DMA->DMAENSET = DMAENSET_I2CMRX; // Enable DMA channel  
  
  while (1)
  {
    while (!ucComplete){}
    ucComplete = 0;
    delay(2000);

    // I2C master transmit  
    pADI_DMA->DMARMSKCLR |= DMARMSKCLR_I2CMTX; // re-enable DMA request from I2C Tx
    I2cMWrCfg(0xA0);  // initiate DMA transfer
    
    while (!ucComplete){}
    ucComplete = 0;
    delay(2000);
    
    // I2C master receive  
    I2cMRdCfg(0xA0, DMA_RX_COUNT, 0);  // initiate DMA Rx transfer
  }
}

// Simple Delay routine
void delay (int length)
{
  while (length >0)
    length--;
}
///////////////////////////////////////////////////////////////////////////
// DMA I2C Master TX handler 
///////////////////////////////////////////////////////////////////////////
void DMA_I2C0_MTX_Int_Handler ()
{
  I2CMTxDmaDesc->ctrlCfg.n_minus_1 = DMA_TRAN_COUNT - 1;
  I2CMTxDmaDesc->ctrlCfg.cycle_ctrl = 1; //  Basic DMA transfer  
  pADI_DMA->DMAENSET |= DMAENSET_I2CMTX; // Enable DMA channel
  pADI_DMA->DMARMSKSET |= DMARMSKSET_I2CMTX; //Disable further DMA requests from the I2C peripheral
}

///////////////////////////////////////////////////////////////////////////
// DMA I2C Master RX handler 
///////////////////////////////////////////////////////////////////////////
void DMA_I2C0_MRX_Int_Handler ()
{
  I2CMRxDmaDesc->ctrlCfg.n_minus_1 = DMA_RX_COUNT - 1;
  I2CMRxDmaDesc->ctrlCfg.cycle_ctrl = 1; //  Basic DMA transfer  
  pADI_DMA->DMAENSET |= DMAENSET_I2CMRX; // Enable DMA channel
}

///////////////////////////////////////////////////////////////////////////
// I2C0 master handler 
///////////////////////////////////////////////////////////////////////////
void I2C0_Master_Int_Handler(void) 
{
  if((I2cSta(MASTER) & I2CMSTA_TCOMP_SET) == I2CMSTA_TCOMP_SET) // communication complete	
  {
    ucComplete = 1;
  }
}



