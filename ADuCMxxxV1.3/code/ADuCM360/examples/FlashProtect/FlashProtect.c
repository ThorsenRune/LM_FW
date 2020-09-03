/**
 *****************************************************************************
   @example  FlashProtect.c
   @brief    
   - WARNING THIS PROJECT WILL LOCK YOUR PART AND CAN ONLY BE RECOVERED USING A
   MASS ERASE WITH THE CM3WSD, OR BY TRIGGERING A MASS ERASE WITH AN IRQ
   - This example show how to store the FAKEY and write protection key at specific
   locations in flash
   - the startup file for this has been modified to load the read protection as soon
   as user code runs
   
   The part can be mass erased by triggering External IRQ4 and this will re-enable
   serial wire after a power cycle.

   @version  V0.2
   @author   ADI
   @date     November 2012
   @par Revision History:
   - V0.1, October 2012: initial version. 
   - V0.2, November 2012: Added working protection code for IAR
              
All files for ADuCM360/361 provided by ADI, including this file, are
provided  as is without warranty of any kind, either expressed or implied.
The user assumes any and all risk from the use of this code.
It is the responsibility of the person integrating this code into an application
to ensure that the resulting application performs as required and is safe.

**/


#include <stdio.h>
#include <string.h>
#include <aducm360.h>


#include <..\common\ClkLib.h>
#include <..\common\WutLib.h>
#include <..\common\WdtLib.h>
#include <..\common\IntLib.h>
#include <..\common\DioLib.h>
#include <..\common\FeeLib.h>

#ifdef __ARMCC_VERSION
//this code will be used for keil uvision
unsigned long flash_key[4] __attribute__((section(".ARM.__at_0x1FFEC"))) = {0x16032010, 0xFAFAFAFA, 0xAFAFAFAF, 0x00000000};  //first value isn't used, next 2 values are the FAKEY and the last value is the write protection
//note that on 64k parts the section address must be changed to 0x0FFEC
//and also the scatter file must be modified accordingly by changes addresses from 0x1FFEC to 0x0FFEC
#endif // __ARMCC_VERSION

#ifdef __ICCARM__
//this code will be used for IAR
__root const unsigned long UserProtAndSig[4] @ ".sigprot" = {0x16032010,0xFAFAFAFA,0xFAFAFAFA,0x00000000};  //first value isn't used, the next two are the FAKEY 
                                                                                                            //and the last value is the write protection
//in IAR the linker file already has a .sigprot section which can be used to store the FAKEY and wr prot key. This section starts
//at 0x1FFEC so the array must have an extra value at the start that isn't actually used
//on 64k parts a different linker must be used which has the .sigprot section at 0x0FFEC

#endif // __ICCARM__

volatile unsigned int uiFEESTA;

void delay(long int);


int main (void)
{
   WdtGo(T3CON_ENABLE_DIS);
   DioOenPin(pADI_GP1, PIN3, 1);                        // Set P1.3 as an output for test purposes
   DioSet(pADI_GP1, BIT3);
   WdtCfg(T3CON_PRE_DIV1,T3CON_IRQ_EN,T3CON_PD_DIS); // Disable Watchdog timer resets
   //Disable clock to unused peripherals
   ClkDis(CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|CLKDIS_DISI2CCLK|CLKDIS_DISPWMCLK|CLKDIS_DIST0CLK|CLKDIS_DIST1CLK|CLKDIS_DISDMACLK);
   ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);            // Select CD0 for CPU clock
   ClkSel(CLK_CD7,CLK_CD7,CLK_CD7,CLK_CD7);     
   
   EiCfg(EXTINT4, INT_EN, INT_FALL);  //configure and enable ext interupt 4 (P1.1)
   NVIC_EnableIRQ(EINT4_IRQn);  
   
   while (1)
   {
      delay(2000000);
      DioTgl(pADI_GP1, BIT3);
   }
}

// Simple Delay routine
void delay (long int length)
{
   while (length >0)
      length--;
}


void Ext_Int4_Handler ()
{           
   uiFEESTA = FeeSta();
   while ((uiFEESTA & FEESTA_CMDBUSY) == FEESTA_CMDBUSY){
      uiFEESTA = FeeSta();
   }
   FeeMErs();
}





