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

void delay(long int);


int main (void)
{
   WdtCfg(T3CON_PRE_DIV256,T3CON_IRQ_DIS,T3CON_PD_EN);   // Enable Watchdog timer to reset CPU on time-out
   WdtLd(0x1000);                                        // Set timeout period to ~32 seconds
   WdtGo(T3CON_ENABLE_EN);                               // Start the watchdog timer
      //Disable clock to unused peripherals
   ClkDis(CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|CLKDIS_DISI2CCLK|CLKDIS_DISUARTCLK|CLKDIS_DISPWMCLK|CLKDIS_DISDACCLK|CLKDIS_DISDMACLK|CLKDIS_DISADCCLK);
   ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);            // Select CD0 for CPU clock - 16Mhz clock
 
  //Timer 0 setup to re-start every 64mS
   GptLd(pADI_TM0,0x1000);								 // Time-out period of 256 clock pulese				
   GptCfg(pADI_TM0,TCON_CLK_UCLK,TCON_PRE_DIV256,TCON_MOD|TCON_RLD|TCON_ENABLE);  // T0 config, Uclk/256, 
 
  //Timer 1 setup
   GptLd(pADI_TM1,0xFFFA);                                  // Set timeout period for 5 seconds
   GptCfg(pADI_TM1,TCON_CLK_LFOSC,TCON_PRE_DIV32768,TCON_MOD_PERIODIC|TCON_UP|TCON_RLD|TCON_ENABLE);

   //Timer 2 setup
   WutCfg(T2CON_MOD_PERIODIC,T2CON_WUEN_DIS,T2CON_PRE_DIV16,T2CON_CLK_PCLK); // T2 config, PCLK/16, periodic mode, 
   WutCfgInt(T2IEN_WUFD,1);                              // Interrupt on compare to D register
   WutLdWr(3,1000);                                      // Set the timeout period as 1mS
   WutGo(T2CON_ENABLE_EN);                               // Start wake-up timer

   DioOen(pADI_GP1,0x8);                                 // Set P1.3 as an output to toggle the LED
   DioOen(pADI_GP0,0x7);                                 // P0.[2:0] selected as outputs.
   NVIC_EnableIRQ(TIMER0_IRQn);                          // Enable Timer0 IRQ
   NVIC_EnableIRQ(TIMER1_IRQn);                          // Enable Timer1 IRQ
   NVIC_EnableIRQ(WUT_IRQn);                             // Enable Timer2 IRQ
   while (1)
   {
      WdtClrInt();  	         // Refresh watchdog timer
      DioTgl(pADI_GP1,0x8);	// Toggle P1.3
      delay(0x60000);		   // Delay routine		
   }
}


// Simple Delay routine
void delay (long int length)
{
   while (length >0)
      length--;
}
void WakeUp_Int_Handler(void)
{
   WutClrInt(T2CLRI_WUFD);          // Clear wakeup for D register interrupt
   DioTgl(pADI_GP0,0x4);			   // Toggle P0.2
}
void GP_Tmr0_Int_Handler(void)
{
   GptClrInt(pADI_TM0,TSTA_TMOUT);
   DioTgl(pADI_GP0,0x1);
}

void GP_Tmr1_Int_Handler(void)
{
   GptClrInt(pADI_TM1,TSTA_TMOUT);  // Clear T1 interrupt
   DioTgl(pADI_GP0,0x2);			   // Toggle P0.1
}
void WDog_Tmr_Int_Handler(void)
{
   WdtClrInt();        // clear watchdog timer interrupt if Interrupt mode is selected
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
void I2C0_Slave_Int_Handler(void)
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





