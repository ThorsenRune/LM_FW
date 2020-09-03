/**
 *****************************************************************************
   @example  ADCMeter.c
   @brief    This Simple ADC example shows how to initialize the ADC1 for 
     continuous sampling using differential inputs.
   - Inputs selected are AIN3 for +ve and AIN2 for -ve
   - When using the ADuCM360 Evaluation board, it will digitize the ADC1 reading, convert it to 
     a voltage and send this to the UART.
   - ADC Gain of 4 is used
   - The DAC is also initialised to output 150mV.
   - The DAC output may be connected to AIN1, AIN0 to ground for test purposes 
   - Default Baud rate is 9600 

   @version V0.2
   @author  ADI
   @date    February 2013

   @par     Revision History:
   - V0.1, September 2012: initial version. 
   - V0.2, February 2013: Fixed a bug with ucTxBufferEmpty.
              
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

void ADC1INIT(void);
void DACINIT(void);
void UARTINIT (void);


void delay(long int);
volatile unsigned char bSendResultToUART = 0;	// Flag used to indicate ADC0 resutl ready to send to UART	
unsigned char szTemp[64] = "";					// Used to store ADC0 result before printing to UART
unsigned char ucTxBufferEmpty  = 0;				// Used to indicate that the UART Tx buffer is empty
volatile  long ulADC1Result = 0;		        // Variable that ADC1DAT is read into in ADC1 IRQ
volatile unsigned char ucComRx = 0;
volatile unsigned char ucADC0ERR = 0;
float  fVoltage = 0.0;   			            // ADC value converted to voltage
float fVolts = 0.0;

int main (void)
{
   unsigned char i = 0;
   unsigned char nLen = 0;

   pADI_WDT ->T3CON = 0;
   DioOen(pADI_GP1,0x8);                        // Set P1.3 as an output for test purposes
   WdtCfg(T3CON_PRE_DIV1,T3CON_IRQ_EN,T3CON_PD_DIS); // Disable Watchdog timer resets
   //Disable clock to unused peripherals
   ClkDis(CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|CLKDIS_DISI2CCLK|CLKDIS_DISPWMCLK|CLKDIS_DIST0CLK|CLKDIS_DIST1CLK|CLKDIS_DISDMACLK);
   ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);            // Select CD0 for CPU clock
   ClkSel(CLK_CD6,CLK_CD7,CLK_CD0,CLK_CD7);     // Select CD0 for UART System clock
   AdcGo(pADI_ADC1,ADCMDE_ADCMD_IDLE);			// Place ADC1 in Idle mode
   UARTINIT();									// Init Uart
   DACINIT();                                   // Configure DAC output
   ADC1INIT();									// Setup ADC1
   AdcGo(pADI_ADC1,ADCMDE_ADCMD_CONT);			// Start ADC1 for continuous conversions
   NVIC_EnableIRQ(ADC1_IRQn);					// Enable ADC1 and UART interrupt sources
   NVIC_EnableIRQ(UART_IRQn);

   while (1)
   {
      if (bSendResultToUART == 1) 				// ADC result ready to be sent to UART
      {
         DioTgl(pADI_GP1,0x8);            // Toggle P1.3
         bSendResultToUART = 0;            // Clear flag
         fVolts = fVoltage;
         fVolts   = (1.2 / 268435456);      // Internal reference, calculate lsb size in volts
         fVoltage = (ulADC1Result * fVolts);   // Calculate ADC result in volts
         sprintf ( (char*)szTemp, "Voltage: %fV \r\n",fVoltage );// Scan string with the Temperature Result                           
         nLen = strlen((char*)szTemp);      // Call function to calcualte the length of scanned string
         if (nLen <64)
         {
            for ( i = 0 ; i < nLen ; i++ )	// loop to send ADC1 result	to UART
            {
               ucTxBufferEmpty = 0;	   // Clear flag
               UrtTx(pADI_UART,szTemp[i]);// Load UART Tx register.
               while (ucTxBufferEmpty == 0)// Wait for UART Tx interrupt
               {
               }
            }
         }
      }
      
   }
}

void UARTINIT (void)
{
   //Select IO pins for UART.
   pADI_GP0->GPCON |= 0x3C;                     // Configure P0.1/P0.2 for UART
//    pADI_GP0->GPCON |= 0x9000;                   // Configure P0.6/P0.7 for UART
   UrtCfg(pADI_UART,B9600,COMLCR_WLS_8BITS,0);  // setup baud rate for 9600, 8-bits
   UrtMod(pADI_UART,COMMCR_DTR,0);              // Setup modem bits
   UrtIntCfg(pADI_UART,COMIEN_ERBFI|COMIEN_ETBEI|COMIEN_ELSI|COMIEN_EDSSI|COMIEN_EDMAT|COMIEN_EDMAR);  // Setup UART IRQ sources
}

void ADC1INIT(void)
{
   AdcMski(pADI_ADC1,ADCMSKI_RDY,1);              // Enable ADC ready interrupt source		
   AdcFlt(pADI_ADC1,124,14,FLT_NORMAL|ADCFLT_NOTCH2|ADCFLT_CHOP); // ADC filter set for 3.75Hz update rate with chop on enabled
   AdcRng(pADI_ADC1,ADCCON_ADCREF_INTREF,ADCMDE_PGA_G4,ADCCON_ADCCODE_INT); // Internal reference selected, Gain of 4, Signed integer output
   // Turn off input buffers to ADC and external reference	
   AdcBuf(pADI_ADC1,ADCCFG_EXTBUF_OFF,ADCCON_BUFBYPN|ADCCON_BUFBYPP|ADCCON_BUFPOWP|ADCCON_BUFPOWN); 
   AdcPin(pADI_ADC1,ADCCON_ADCCN_AIN0,ADCCON_ADCCP_AIN1); // Select AIN1 as postive input and AIN0 as negative input
}
void DACINIT(void)
{
   // Configure DAC output for 0-1.2V output range, Normal 12-bit mode and immediate update.
   DacCfg(DACCON_CLR_Off,DACCON_RNG_IntVref,DACCON_CLK_HCLK,DACCON_MDE_12bit);	  
   DacWr(0,0x1FF0000);		               // Output value of 150mV
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
   volatile unsigned int uiADCSTA = 0;
   
   uiADCSTA = pADI_ADC1->STA;               // read ADC status register
   ulADC1Result = AdcRd(pADI_ADC1);            // read ADC result register
   bSendResultToUART = 1;                  // Set flag to indicate ready to send result to UART
}
void SINC2_Int_Handler()
{
 
}
void UART_Int_Handler ()
{
   volatile unsigned char ucCOMSTA0 = 0;
   volatile unsigned char ucCOMIID0 = 0;
   volatile unsigned int uiUartCapTime = 0;
   
   ucCOMSTA0 = UrtLinSta(pADI_UART);         // Read Line Status register
   ucCOMIID0 = UrtIntSta(pADI_UART);         // Read UART Interrupt ID register         
   if ((ucCOMIID0 & 0x2) == 0x2)             // Transmit buffer empty
   {
      ucTxBufferEmpty = 1;
   }
   if ((ucCOMIID0 & 0x4) == 0x4)             // Receive byte
   {
      ucComRx   = UrtRx(pADI_UART);
   }
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





