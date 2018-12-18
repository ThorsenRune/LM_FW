/**
 *****************************************************************************
   @example  Timers.c
   @brief    This Example shows how to setup Timer0, Timer1, the wakeup timer and watchdog timer
   - Timer 0 is setup for a timeout period of ~64mS - P0.0 will toggle every 64mS
   - Timer 1 is setup for a timeout period of ~5S - P0.1 will toggle every 5S
   - Timer 2 is setup for a timeout period of ~1mS - P0.2 will toggle every 1mS

   @version  V0.1
   @author   ADI
   @date     September 2012 

All files for ADuCM360/361 provided by ADI, including this file, are
provided  as is without warranty of any kind, either expressed or implied.
The user assumes any and all risk from the use of this code.
It is the responsibility of the person integrating this code into an application
to ensure that the resulting application performs as required and is safe.

**/


#include <stdio.h>
#include <string.h>
#include <ADuCM360.h>


#include <..\common\ClkLib.h>
#include <..\common\IntLib.h>
#include <..\common\DioLib.h>
#include <..\common\WdtLib.h>
#include <..\common\WutLib.h>
#include <..\common\GptLib.h>
#include <..\common\DioLib.h>


int bFlg1=0;
void mStartTimer0(int t0count){
		// ClkDis must leave DIST0CLK=0
 //  ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);            // Select CD0 for CPU clock - 16Mhz clock

	// RT: 16MHz System clock withPrescaling 4*16 = 0.25MHz =>250uS, count down mode
		GptLd(pADI_TM0,1000); // 1ms timer 
		GptCfg(pADI_TM0,TCON_CLK_UCLK,TCON_PRE_DIV16,TCON_MOD_PERIODIC|TCON_UP_DIS|TCON_RLD_DIS|TCON_ENABLE);
	// TCON_RLD_DIS: Do not wait for interrupt to reload but reload automatically

  NVIC_EnableIRQ(TIMER0_IRQn);                          // Enable Timer0 IRQ
	
	
	
 
}
void mStartTimer1(int t1count){
//TIMER1 SETUP
	GptCfg(pADI_TM1,TCON_CLK_UCLK,TCON_PRE_DIV16,TCON_MOD_PERIODIC|TCON_UP_DIS|TCON_RLD_DIS|TCON_ENABLE);
	GptLd(pADI_TM1,300); //300us timer
	NVIC_EnableIRQ(TIMER1_IRQn); //Enable Timer1 IRQ
}
int mainTimer (void)
{
   WdtCfg(T3CON_PRE_DIV256,T3CON_IRQ_DIS,T3CON_PD_EN);   // Enable Watchdog timer to reset CPU on time-out
   WdtLd(0x1000);                                        // Set timeout period to ~32 seconds
   WdtGo(T3CON_ENABLE_EN);                               // Start the watchdog timer
      //Disable clock to unused peripherals
   ClkDis(CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|CLKDIS_DISI2CCLK|CLKDIS_DISUARTCLK|CLKDIS_DISPWMCLK|CLKDIS_DISDACCLK|CLKDIS_DISDMACLK|CLKDIS_DISADCCLK);
   ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);            // Select CD0 for CPU clock - 16Mhz clock
 
  //Timer 0 setup to re-start every 64mS
//   GptLd(pADI_TM0,0x1000);								 // Time-out period of 256 clock pulese				
//   GptCfg(pADI_TM0,TCON_CLK_UCLK,TCON_PRE_DIV256,TCON_MOD|TCON_RLD|TCON_ENABLE);  // T0 config, Uclk/256, 
 

	
  //Timer 1 setup
   GptLd(pADI_TM1,10);        //10 sec                          // Set timeout period for 5 seconds
   GptCfg(pADI_TM1,TCON_CLK_LFOSC,TCON_PRE_DIV32768,TCON_MOD_PERIODIC|TCON_UP_DIS|TCON_RLD|TCON_ENABLE);
	
	
   //Timer 2 setup
   WutCfg(T2CON_MOD_PERIODIC,T2CON_WUEN_DIS,T2CON_PRE_DIV16,T2CON_CLK_PCLK); // T2 config, PCLK/16, periodic mode, 
   WutCfgInt(T2IEN_WUFD,1);                              // Interrupt on compare to D register
   WutLdWr(3,1000);                                      // Set the timeout period as 1mS
   WutGo(T2CON_ENABLE_EN);                               // Start wake-up timer
   NVIC_EnableIRQ(TIMER0_IRQn);                          // Enable Timer0 IRQ
   NVIC_EnableIRQ(TIMER1_IRQn);                          // Enable Timer1 IRQ
   NVIC_EnableIRQ(WUT_IRQn);                             // Enable Timer2 IRQ
   while (1)
   {
      WdtClrInt();  	         // Refresh watchdog timer
   }
}
