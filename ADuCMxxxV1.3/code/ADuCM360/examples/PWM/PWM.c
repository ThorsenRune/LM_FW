/**
 *****************************************************************************
   @example PWM.c
   @brief   The PWM is configured to operate in standard mode.
   - PWM0 on P1.2 (mode 1)
   - PWM1 on P1.3 (mode 1)
   - PWM2 on P1.4 (mode 1)
   - PWM3 on P1.5 (mode 1)
   - PWM4 on P1.6 (mode 1)
   - PWM5 on P1.7 (mode 1)
   - PWMTRIP on P1.0 (mode 1)
   - PWMSYNC on P1.1 (mode 1)

   @version V0.2
   @author  ADI
   @date    April 2012 
              

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
#include <..\common\DioLib.h>
#include <..\common\PwmLib.h>
#include <..\common\WdtLib.h>
volatile unsigned int uiTime0 = 0;	   // Used for reading return value from PWMTime function
volatile unsigned int uiTime1 = 0;	   // Used for reading return value from PWMTime function
volatile unsigned int uiTime2 = 0;	   // Used for reading return value from PWMTime function
volatile int Error= 0;

int main (void)
{
    WdtCfg(T3CON_PRE_DIV1,T3CON_IRQ_EN,T3CON_PD_DIS); // Disable Watchdog timer resets
    //Disable clock to unused peripherals
   ClkDis(CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|CLKDIS_DISI2CCLK|CLKDIS_DISUARTCLK|CLKDIS_DISDACCLK|CLKDIS_DISDMACLK|CLKDIS_DISADCCLK);
   ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);            // Select CD0 for CPU clock - 16Mhz clock
   
   DioCfg(pADI_GP1,0x5555);                 // Setup P1.0 to P1.7 as PWMTRIP, PWMSYNC and PWM outputs
   PwmInit(UCLK_2,PWMCON0_PWMIEN_EN,PWMCON0_SYNC_EN,PWMCON1_TRIPEN_EN); //UCLK/2 to PWM, Enable IRQs, SYNC and Trip input
   
   uiTime0 = PwmTime(PWM0_1,200,150,40);        // 40kHz freq. PWM0 output for 75% high. PWM1 output 20% duty cycle.
   uiTime1 = PwmTime(PWM2_3,400,350,100);	// 20kHz freq. PWM2 output for 87.5% high. PWM3 output 25% dutycycle.
   uiTime2 = PwmTime(PWM4_5,800,400,399);	// 10kHz freq. PWM4 output for 50% high. PWM5 output 49.8% dutycycle.
   NVIC_EnableIRQ(PWM_TRIP_IRQn);			// Enable PWM Trip IRQ
   NVIC_EnableIRQ(PWM_PAIR0_IRQn);			// Enable pair 0 IRQ
   NVIC_EnableIRQ(PWM_PAIR1_IRQn);			// Enable pair 1 IRQ
   NVIC_EnableIRQ(PWM_PAIR2_IRQn);			// Enable pair 2 IRQ

   if (uiTime0 != 1)
		Error = 1;//		Error with PWM pair 0 
   if (uiTime1 != 1)
		Error = 1;//		Error with PWM pair 1
   if (uiTime2 != 1)
		Error = 1;//		Error with PWM pair 2	
   PwmGo(PWMCON0_ENABLE_EN,PWMCON0_MOD_DIS);            // Enable PWM outputs
   while (1)
   {
	
   }
}

void PWMTRIP_Int_Handler ()
{           
 PwmClrInt(PWMCLRI_TRIP);  // Automatically disable PWM peripheral
}
void PWM0_Int_Handler()
{
  PwmClrInt(PWMCLRI_PWM0);
}
void PWM1_Int_Handler ()
{
 PwmClrInt(PWMCLRI_PWM1);
}
void PWM2_Int_Handler()
{
  PwmClrInt(PWMCLRI_PWM2);
}





