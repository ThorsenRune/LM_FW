/**
 *****************************************************************************
   @example  FlashWrite.c
   @brief    
   - This example show how to use the flash functions.
   - It starts by erasing the flash page starting at 0x1f000
   - then writes some data to that page, 
   - after which the flash contents are checked and if they match the LED (P1.3) turns on.
   - Next a Failure Analysis key is written is written close to the top of user flash.
   - This key needs to be given to ADI if failure analisys needs to be performed on a part
     which has read protection enabled.
   
   - There is also code to write protect and read protect the flash, but this is commented out
     and these functions means that the part must be mass erased before they can be used again.
   
   - The part can be mass erased by triggering External IRQ1

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
#include <aducm360.h>

#include <..\common\AdcLib.h>
#include <..\common\IexcLib.h>
#include <..\common\DacLib.h>
#include <..\common\UrtLib.h>
#include <..\common\ClkLib.h>
#include <..\common\WutLib.h>
#include <..\common\WdtLib.h>
#include <..\common\GptLib.h>
#include <..\common\I2cLib.h>
#include <..\common\IntLib.h>
#include <..\common\PwmLib.h>
#include <..\common\DioLib.h>
#include <..\common\FeeLib.h>


volatile unsigned int uiFEESTA;
volatile unsigned int uiFEEADRAL;
volatile unsigned int uiFEESIG;


void delay(long int);
void WriteToFlash(unsigned long *pArray, unsigned long ulStartAddress, unsigned int uiSize);
void ReadFromFlash(unsigned long *pArray, unsigned long ulStartAddress, unsigned int uiSize);


unsigned long data[10] = {1,2,3,4,5,6,7,8,9,10};
unsigned long checkData[10];
unsigned int i=0;

int main (void)
{
   WdtGo(T3CON_ENABLE_DIS);
   DioOenPin(pADI_GP1, PIN3, 1);                        // Set P1.3 as an output for test purposes
   DioSet(pADI_GP1, BIT3);
   WdtCfg(T3CON_PRE_DIV1,T3CON_IRQ_EN,T3CON_PD_DIS); // Disable Watchdog timer resets
   //Disable clock to unused peripherals
   ClkDis(CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|CLKDIS_DISI2CCLK|CLKDIS_DISPWMCLK|CLKDIS_DIST0CLK|CLKDIS_DIST1CLK|CLKDIS_DISDMACLK);
   ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);            // Select CD0 for CPU clock
   ClkSel(CLK_CD7,CLK_CD7,CLK_CD0,CLK_CD7);     // Select CD0 for UART System clock

   EiCfg(EXTINT1, INT_EN, INT_FALL);  //configure and enable ext interupt 1 (P0.5)
   NVIC_EnableIRQ(EINT1_IRQn);  
   
   FeePErs(0x1f000);
   WriteToFlash(data, 0x1f000, sizeof(data));
   ReadFromFlash(checkData, 0x1f000, sizeof(checkData));
   for (i=0; i<(sizeof(checkData)/sizeof(checkData[0])); i++){
      if (checkData[i] != data[i])
         break;
   }
   if (i == ( sizeof(checkData)/sizeof(checkData[0]) )){
      DioClr(pADI_GP1, BIT3);             //Turn Led on
   }
   do
      uiFEESTA = FeeSta();
   while((uiFEESTA & FEESTA_CMDBUSY) == FEESTA_CMDBUSY);  //wait for the flash controller to finish
   
   FeeWrEn(1);
   FeeFAKey(0x123456789abcdef0);
   FeeWrEn(0);
   
//    FeeWrEn(1);
//    FeeWrPro(0x0);  // write protect all blocks
//    FeeWrEn(0);
   
//    FeeRdProTmp(FEECON1_DBG_DIS);       //enable read protection this can only be disabled by mass erasing with CM3WSD or EXT IRQ1
   
   while (1)
   {
      delay(2000000);
   }
}

void WriteToFlash(unsigned long *pArray, unsigned long ulStartAddress, unsigned int uiSize)
{
   unsigned int uiPollFEESTA = 0;
   volatile unsigned long *flashAddress;
   unsigned int i = 0;
   
   flashAddress = ( unsigned long      *)ulStartAddress;
   FeeWrEn(1);
   uiPollFEESTA = FeeSta();						// Read Status to ensure it is clear
   for (i = 0; i < uiSize; i = i+4)
   { 
      uiPollFEESTA = 0;
      *flashAddress++  = *pArray++;
      do
         {uiPollFEESTA = FeeSta();}
       while((uiPollFEESTA & FEESTA_CMDBUSY) == FEESTA_CMDBUSY);
   }  
   FeeWrEn(0);       // disable a write to Flash memory
}

void ReadFromFlash(unsigned long *pArray, unsigned long ulStartAddress, unsigned int uiSize)
{
   volatile unsigned long *flashAddress;
   unsigned int i = 0;

   flashAddress = ( unsigned long      *)ulStartAddress;
   for (i = 0; i < uiSize; i = i+4)
   {
      *pArray++ = *flashAddress++;
   }
}
// Simple Delay routine
void delay (long int length)
{
   while (length >0)
      length--;
}
void Ext_Int2_Handler ()
{           

}

void Ext_Int1_Handler ()
{           
   uiFEESTA = FeeSta();
   while ((uiFEESTA & FEESTA_CMDBUSY) == FEESTA_CMDBUSY){
      uiFEESTA = FeeSta();
   }
   FeeMErs();
}
void Ext_Int4_Handler ()
{         
 
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





