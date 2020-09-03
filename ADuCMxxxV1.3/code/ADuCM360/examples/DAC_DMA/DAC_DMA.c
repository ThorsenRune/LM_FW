/**
 *****************************************************************************
   @example    DAC_DMA.c
   @brief      This example shows how to initialize the DAC for DMA operation
   - An array of DAC values to generate a Triangular wave is moved to DAC and triggered
      by Timer 1

   @version  V0.1
   @author   ADI
   @date     October 2012
   @par Revision History:
   - V0.1, October 2012: initial version. 


All files for ADuCM360/361 provided by ADI, including this file, are
provided  as is without warranty of any kind, either expressed or implied.
The user assumes any and all risk from the use of this code.
It is the responsibility of the person integrating this code into an application
to ensure that the resulting application performs as required and is safe.

**/

#include <stdio.h>
#include <string.h>
#include <aducm360.h>

#include <..\common\DacLib.h>
#include <..\common\DacLib.h>
#include <..\common\ClkLib.h>
#include <..\common\WdtLib.h>
#include <..\common\GptLib.h>
#include <..\common\IntLib.h>
#include <..\common\DioLib.h>
#include <..\common\DmaLib.h>
void DACINIT(void);
void DMAINIT(void);
void T1INIT(void);
void delay(long int);
int uxDACDMA[] = {0x00000000, 0x01000000, 0x02000000, 0x03000000,
							0x04000000, 0x05000000, 0x06000000, 0x07000000,
							0x08000000, 0x09000000, 0x0A000000, 0x0B000000,
							0x0C000000, 0x0D000000, 0x0E000000, 0x0F000000,
                     0x0FFF0000, 0x0f000000, 0x0E000000, 0x0D000000,
							0x0C000000, 0x0B000000, 0x0A000000, 0x09000000,
							0x08000000, 0x07000000, 0x06000000, 0x05000000,
							0x04000000, 0x03000000, 0x02000000, 0x01000000};
unsigned long ulDmaStatus = 0;
unsigned char ucTriggerDac = 0;                      // Flag used to indicate DAC DMA needs updating
unsigned char ucIrqCnt = 0;
int main (void)
{
   pADI_WDT ->T3CON = 0;                             // Disable the watchdog timer
   DioOen(pADI_GP1,0xC);                             // Set P1.3/P1.2 as an output for test purposes
   WdtCfg(T3CON_PRE_DIV1,T3CON_IRQ_EN,T3CON_PD_DIS); // Disable Watchdog timer resets
   //Disable clock to unused peripherals
   ClkDis(CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|CLKDIS_DISI2CCLK|CLKDIS_DIST0CLK);
   ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG); // Select CD0 for CPU clock
   DACINIT();									              // Setup DAC
   DMAINIT();                                        // Setup DAC DMA channel	
 	NVIC_EnableIRQ(DMA_DAC_IRQn);   
   T1INIT();                                         // Setup Timer 1 - used to trigger DAC DMA output   
   while (1)
   {
      if (ucTriggerDac == 1)
      {
         ucTriggerDac = 0;
         DmaClr(DMARMSKCLR_DAC,0,0,0);               // Disable masking of DAC DMA channel
         DmaCycleCntCtrl(DAC_C,32,DMA_DSTINC_NO|
           DMA_SRCINC_WORD|DMA_SIZE_WORD|DMA_BASIC); // Re-Enable DAC DMA output
      }

	}
}

void DACINIT(void)
{												
   DacCfg(DACCON_CLR_Off,DACCON_RNG_IntVref,
      DACCON_CLK_HCLK,DACCON_MDE_12bit);           //DAC range 0 to 1.2V, 12-bit mode
   DacDma(0,DACCON_DMAEN_On);                        // Enable DAC DMA
}

void DMAINIT(void)  
{
	DmaBase();
	DmaSet(0,DMAENSET_DAC,0,DMAPRISET_DAC);           // Enable DAC DMA primary structure
   DacDmaWriteSetup(DAC_C,DMA_DSTINC_NO|
     DMA_SRCINC_WORD|DMA_SIZE_WORD|DMA_BASIC,32,
     uxDACDMA);                                      // Setup DAC DMA structure.
}
void T1INIT(void)
{
   GptLd(pADI_TM1,0x88);
   GptCfg(pADI_TM1,TCON_CLK_UCLK,TCON_PRE_DIV1,
      TCON_MOD_PERIODIC|TCON_ENABLE);                // Enable Timer 1 
}

void Ext_Int2_Handler ()
{           

}
void Ext_Int4_Handler ()
{         
 
}
void GP_Tmr0_Int_Handler(void)
{

}

void GP_Tmr1_Int_Handler(void)
{
    GptClrInt(pADI_TM1,TSTA_TMOUT);  // Clear T1 interrupt   
    DioTgl(pADI_GP1,0x4);				 // Toggle P1.2   
}
void ADC0_Int_Handler()
{
   
}
void ADC1_Int_Handler ()
{

}
void DMA_DAC_Out_Int_Handler ()
{
   DmaSet(DMARMSKSET_DAC,DMAENSET_DAC,0,0);
   ucIrqCnt++;
   ucTriggerDac = 1;
  DioTgl(pADI_GP1,0x8);				 // Toggle P1.3
   
}

void DMA_Err_Int_Handler ()
{
	ulDmaStatus = DmaSta();
	DmaErr(DMA_ERR_CLR);
}

void UART_Int_Handler ()
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




