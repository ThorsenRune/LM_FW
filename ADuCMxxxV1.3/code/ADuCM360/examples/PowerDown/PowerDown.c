/**
 *****************************************************************************
   @example  PowerDown.c
   @brief    This Example shows how to place the ADuCM360/361 into power down mode
             - External interrupt 4 is used place the part into the power saving mode.
             - External interrupt 2 is enabled in this example and may be used to wake the part up.

   @version V0.1
   @author  ADI
   @date    February 2013

              
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
#include <..\common\RstLib.h>
#include <..\common\PwrLib.h>
#include <..\common\WdtLib.h>

void delay(long int);
int iGoToSleep = 0;

int main (void)
{
    volatile int iPwrMod = 0;  
	
	 WdtCfg(T3CON_PRE_DIV1,T3CON_IRQ_EN,T3CON_PD_DIS); // Disable Watchdog timer resets
   DioOen(pADI_GP1,0x8);                             // Set P1.3 as an output to toggle the LED
	  //Disable clock to unused peripherals
   ClkDis(CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|CLKDIS_DISI2CCLK|CLKDIS_DISUARTCLK|CLKDIS_DISPWMCLK|CLKDIS_DIST0CLK|CLKDIS_DIST1CLK|CLKDIS_DISDACCLK|CLKDIS_DISDMACLK|CLKDIS_DISADCCLK);
   ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);            // Select CD0 for CPU clock - 16Mhz clock
	 EiCfg(EXTINT2,INT_EN,INT_FALL);                    
	 EiCfg(EXTINT4,INT_EN,INT_FALL);
	 iGoToSleep = 0;
   NVIC_EnableIRQ(EINT4_IRQn);                       // Enable external interrupt 4 - used to wake-up MCU
	 NVIC_EnableIRQ(EINT2_IRQn);                       // Enable external interrupt 2 - used to put MCU into required sleep mode
   while (1)
   {	    
	   DioTgl(pADI_GP1,0x8);	// Toggle P1.3
	   delay(0x60000);		// Delay routine	
     if (iGoToSleep	!=0)
		 {
			 iGoToSleep = 0;
			 PwrCfg(PWRMOD_MOD_TOTALHALT);
			 //PwrCfg(PWRMOD_MOD_PERHALT);
		 }
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
  
}
void Ext_Int2_Handler ()
{           
	EiClr(EXTINT2);	
	iGoToSleep = 1;	
}
void Ext_Int4_Handler ()
{         
   EiClr(EXTINT4);
	iGoToSleep = 0;
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





