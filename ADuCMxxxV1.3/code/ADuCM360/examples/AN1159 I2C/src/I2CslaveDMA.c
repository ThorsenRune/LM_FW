/**
 *****************************************************************************
   @example  I2CslaveDMA.c
   @brief    I2C is configured in master mode.
   - SCL on P0.1 (mode 2)
   - SDA on P0.2 (mode 2)

   - Configures the ADuCM301 for I2C DMA transfer. 
   - Code to use with I2Cmaster.c

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
DmaDesc *I2CSRxDmaDesc, *I2CSTxDmaDesc;
  
volatile unsigned char ucRxBuffer[5];
volatile unsigned char DMA_RX_COUNT = sizeof(ucRxBuffer);

volatile unsigned char ucTxBuffer[] = {0x01,0x02,0x03,0x04,0x05};
volatile unsigned char DMA_TX_COUNT = sizeof(ucTxBuffer);


int main(void)
{
  WdtGo(T3CON_ENABLE_DIS);   
  DioCfg(pADI_GP0, 0x28);     // Configure P0.1/P0.2 as I2C pins
  DioPul(pADI_GP0, 0xF9);     // External pull ups required externally
  
  // configure the clocks
  ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);
  ClkSel(CLK_CD0,CLK_CD0,CLK_CD0,CLK_CD0);
  ClkDis(CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|CLKDIS_DISUARTCLK|CLKDIS_DISPWMCLK|CLKDIS_DIST0CLK|CLKDIS_DIST1CLK|CLKDIS_DISDACCLK|CLKDIS_DISADCCLK);  

// I2C slave receive  
  I2CSRxDmaDesc = &dmaChanDesc[5];	
  I2CSRxDmaDesc->srcEndPtr = (unsigned int) &pADI_I2C->I2CSRX;
  I2CSRxDmaDesc->destEndPtr =  (unsigned int)(ucRxBuffer + DMA_RX_COUNT -1 );
  I2CSRxDmaDesc->ctrlCfg.dst_inc = 0;   // no increment as the dest is a reg
  I2CSRxDmaDesc->ctrlCfg.dst_size = 0;  // byte data
  I2CSRxDmaDesc->ctrlCfg.src_inc = 3;   // byte incr. 
  I2CSRxDmaDesc->ctrlCfg.src_size = 0;  // byte data
  I2CSRxDmaDesc->ctrlCfg.dst_prot_ctrl = 0;
  I2CSRxDmaDesc->ctrlCfg.src_prot_ctrl = 0;
  I2CSRxDmaDesc->ctrlCfg.r_power = 0;
  I2CSRxDmaDesc->ctrlCfg.n_minus_1 = DMA_RX_COUNT - 1;
  I2CSRxDmaDesc->ctrlCfg.next_useburst = 0;
  I2CSRxDmaDesc->ctrlCfg.cycle_ctrl = 1; //  Basic DMA transfer  

// I2C slave transmit
  I2CSTxDmaDesc = &dmaChanDesc[4];	
  I2CSTxDmaDesc->srcEndPtr = (unsigned int)(ucTxBuffer + DMA_TX_COUNT -1 );
  I2CSTxDmaDesc->destEndPtr = (unsigned int) &pADI_I2C->I2CSTX; 
  I2CSTxDmaDesc->ctrlCfg.dst_inc = 3;   // no increment as the dest is a reg
  I2CSTxDmaDesc->ctrlCfg.dst_size = 0;  // byte data
  I2CSTxDmaDesc->ctrlCfg.src_inc = 0;   // byte incr. 
  I2CSTxDmaDesc->ctrlCfg.src_size = 0;  // byte data
  I2CSTxDmaDesc->ctrlCfg.dst_prot_ctrl = 0;
  I2CSTxDmaDesc->ctrlCfg.src_prot_ctrl = 0;
  I2CSTxDmaDesc->ctrlCfg.r_power = 0;
  I2CSTxDmaDesc->ctrlCfg.n_minus_1 = DMA_TX_COUNT - 1;
  I2CSTxDmaDesc->ctrlCfg.next_useburst = 0;
  I2CSTxDmaDesc->ctrlCfg.cycle_ctrl = 1; //  Basic DMA transfer  

  I2cSCfg(I2CSCON_RXDMA|I2CSCON_TXDMA, 0, I2CSCON_SLV); // configure I2C Slave mode
  I2cSIDCfg(0xA0,0,0,0); // only using 1 address in this example

  NVIC_EnableIRQ(DMA_I2CS_RX_IRQn);  
  NVIC_EnableIRQ(DMA_I2CS_TX_IRQn);  
  pADI_DMA->DMAPDBPTR = (unsigned int) &dmaChanDesc; // Setup the DMA base pointer.
  pADI_DMA->DMAENSET = DMAENSET_I2CSRX|DMAENSET_I2CSTX; // Enable DMA channel  
  pADI_DMA->DMACFG = DMACFG_ENABLE_EN;   // Enable the  uDMA  

  while (1)
  {
  }
}


///////////////////////////////////////////////////////////////////////////
// DMA I2C Slave RX handler 
///////////////////////////////////////////////////////////////////////////
void DMA_I2C0_SRX_Int_Handler ()
{
  I2CSRxDmaDesc->ctrlCfg.n_minus_1 = DMA_RX_COUNT - 1;
  I2CSRxDmaDesc->ctrlCfg.cycle_ctrl = 1; //  Basic DMA transfer  
  pADI_DMA->DMAENSET |= DMAENSET_I2CSRX; // Enable DMA channel  
}

///////////////////////////////////////////////////////////////////////////
// DMA I2C Slave TX handler 
///////////////////////////////////////////////////////////////////////////
void DMA_I2C0_STX_Int_Handler ()
{
  I2CSTxDmaDesc->ctrlCfg.n_minus_1 = DMA_TX_COUNT - 1;
  I2CSTxDmaDesc->ctrlCfg.cycle_ctrl = 1; //  Basic DMA transfer  
  pADI_DMA->DMAENSET |= DMAENSET_I2CSTX; // Enable DMA channel
}

