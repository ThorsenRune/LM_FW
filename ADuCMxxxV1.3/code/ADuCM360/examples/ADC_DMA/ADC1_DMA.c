/**
 *****************************************************************************
   @example     ADC1_DMA.c
   @brief     This example shows how to initialize the ADC1 for DMA operation
   - It implements the RTD demo similar to RTD_DEMO project except it uses DMA operation
   - This also sets up the SINC2 filter DMA output to measure across the RTD

   - ADC1 is used to measure an RTD input connected across AIN0 and AIN1.
   - AIN6 is used as the excitation current source for the RTD
   - REFIN+/REFIN- are the ADC external reference inputs 
   - 100ohm PT100 RTD expected connected to AIN0 and AIN1

   - To eliminate drift elements the following is the measurement sequence:
   - 1) Measure AIN1/AGND v Internal 1.2V reference to derive exact Excitation current value
   - 2) Measure AIN0/AGND as a diagnostic - should equal sum of steps 1 and 2.
   - 3) Measure AIN0/AIN1 v Internal Vref to determine voltage across RTD (vRTD)
   - 4) RRTD is determined and final RTD temeprature calculated.

   - The RTD reading is linearized and sent to the UART in a string format.
   - Default Baud rate is 9600
   - EVAL-ADuCM360MKZ or similar hardware is assumed
   - Results will be more accurate if System calibration is added.

   @version V0.2
   @author  ADI
   @date    February 2013

   @par     Revision History:
   - V0.1, October 2012: initial version. 
   - V0.2, February 2013: Fixed a bug in SendString().


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
#include <..\common\DmaLib.h>

void ADC1INIT(void);                          // Init ADC1
void UARTINIT (void);                         // initialise UART
void DMAINIT(void);                           // Setup DMA controller
void SendString(void);					              // Transmit string using UART
void IEXCINIT(void);                 	        // Setup Excitation Current sources
void SendResultToUART(void);			            // Send measurement results to UART - in ASCII String format
float CalculateRTDTemp(float r);              // Calculates final temperature based on RTD resistance measurement
void delay(long int);
//RTD constants
#define TMIN (-40)  					                // = minimum temperature in degC
#define TMAX (125)  					                // = maximum temperature in degC
#define RMIN (84.2707)  				              // = input resistance in ohms at -40 degC
#define RMAX (147.951)  				              // = input resistance in ohms at 125 degC
#define NSEG 30  						                  // = number of sections in table
#define RSEG 2.12269  					              // = (RMAX-RMIN)/NSEG = resistance  in ohms of each segment
//RTD lookup table
const float C_rtd[] = {-40.0006,-34.6322,-29.2542,-23.8669,-18.4704,-13.0649,-7.65042,-2.22714,3.20489,8.64565,14.0952,
						19.5536,25.0208,30.497,35.9821,41.4762,46.9794,52.4917,58.0131,63.5436,69.0834,74.6325,80.1909,
						85.7587,91.3359,96.9225,102.519,108.124,113.74,119.365,124.999};
float fVRTD = 0.0 ;								            // RTD voltage, 
float fRrtd = 0.0;								            // resistance of the RTD
float fTRTD = 0.0;								            // RTD temperature
float fVAIN0_AGND = 0.0;						          // Measures voltage across Rref	and RTD
float fVAIN1_AGND = 0.0;						          // Measures voltage across Rref
float fIexc0 = 0.0;                           // Measured Excitation current
volatile unsigned char bSendResultToUART = 0;	// Flag used to indicate ADC0 resutl ready to send to UART	
unsigned char szTemp[64] = "";					      // Used to store ADC0 result before printing to UART
unsigned char ucTxBufferEmpty  = 0;				    // Used to indicate that the UART Tx buffer is empty
int ulADC1Result = 0;		                      // Variable that ADC1DAT is read into in ADC1 IRQ
unsigned char ucCounter = 0;                  // variable used in averaging calculations
volatile unsigned char ucComRx = 0;
volatile unsigned char ucADC0ERR = 0;
volatile unsigned long ulDmaStatus = 0;
float  fVoltage = 0.0;   			                // ADC value converted to voltage
float fVolts = 0.0;
unsigned char nLen = 0;                       // Used to calculate string length
unsigned char i = 0;                          // variable Used to calculate string length

int uxADC0Data[16] = {0x0};	                  // Store 16 samples - filled via DMA controller
int uxADC1Data[] = {0x1,0x8C00F};	            // ADC1MSKI and ADC1CON values for demonstrating ADC1 DMA writes.
unsigned char ucAdc1Dma = 0;
int main (void)
{
   pADI_WDT ->T3CON = 0;                             // Disable the watchdog timer
   DioOen(pADI_GP1,0x8);                             // Set P1.3 as an output for test purposes
   WdtCfg(T3CON_PRE_DIV1,T3CON_IRQ_EN,T3CON_PD_DIS); // Disable Watchdog timer resets
   //Disable clock to unused peripherals
   ClkDis(CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|CLKDIS_DISI2CCLK|CLKDIS_DISPWMCLK|CLKDIS_DIST0CLK|CLKDIS_DIST1CLK);
   ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG); // Select CD0 for CPU clock
   ClkSel(CLK_CD7,CLK_CD7,CLK_CD0,CLK_CD7);          // Select CD0 for UART System clock
   AdcGo(pADI_ADC1,ADCMDE_ADCMD_IDLE);			         // Place ADC1 in Idle mode
   UARTINIT();									                     // Init Uart
   IEXCINIT();									                     // Setup Excitation current source
   DMAINIT();                                        // Setup ADC1 DMA channel	
   ADC1INIT();									                     // Setup ADC1
   
	 NVIC_EnableIRQ(DMA_ADC1_IRQn);
   NVIC_EnableIRQ(UART_IRQn);

   while (1)
   {
    
	  // reset calculation variables
     fVRTD =0;								
     fVAIN1_AGND = 0;
     fVAIN0_AGND = 0;
     fIexc0 = 0;
	   
		 AdcGo(pADI_ADC1,ADCMDE_ADCMD_IDLE);		         // Put ADC1 into idle mode to allow re-configuration of its control registers
	/* Step 1 - Measure AIN1 v AGND and derive exact Excitation current */
	   AdcRng(pADI_ADC1,ADCCON_ADCREF_INTREF,ADCMDE_PGA_G1,ADCCON_ADCCODE_INT); // Set G=1, Select internal reference
		 AdcBuf(pADI_ADC1,ADCCFG_EXTBUF_OFF,ADCCON_BUFBYPN|ADCCON_BUFBYPP);		 // Turn off input buffers
		 AdcPin(pADI_ADC1,ADCCON_ADCCN_AGND,ADCCON_ADCCP_AIN1);// Select AIN1 to AGND
		 bSendResultToUART = 0;
		 DmaClr(DMARMSKCLR_ADC1,0,0,0);                    // Clear Masking of ADC1 DMA channel
		 AdcGo(pADI_ADC1,ADCMDE_ADCMD_CONT);		         // Start ADC1 for continuous conversions
		 ucCounter = 0;
		 while (	bSendResultToUART == 0)			            // Wait for 8x samples to accumulate before calculating result
		 {}
		 AdcGo(pADI_ADC1,ADCMDE_ADCMD_IDLE);	            // Put ADC1 into idle mode to allow re-configuration of its control registers

		 for (ucCounter = 0; ucCounter < 16; ucCounter++)  // Accumulate 16x ADC1 readings from DMA destination
		 {
		    fVAIN1_AGND += (((float)uxADC0Data[ucCounter]*1.2)/268435456);
		 }
		  bSendResultToUART = 0;			 		
		fVAIN1_AGND = fVAIN1_AGND/16;		                  // Average our 16x samples
		fIexc0 = fVAIN1_AGND/5600;				               // Calculate Excitation current 

	/* Step 2 - Measure AIN0 v AGND as a check */
		AdcPin(pADI_ADC1,ADCCON_ADCCN_AGND,ADCCON_ADCCP_AIN0); // Select AIN0 to AGND for these measurements
		DmaClr(DMARMSKCLR_ADC1,0,0,0);                     // Clear Masking of ADC1 DMA channel
		DmaCycleCntCtrl(ADC1_C,16,DMA_DSTINC_WORD|
        DMA_SRCINC_NO|DMA_SIZE_WORD|DMA_BASIC);          // Update cycle control and number of values to transfer fields
		AdcGo(pADI_ADC1,ADCMDE_ADCMD_CONT);		            // Start ADC1 for continuous conversions
		ucCounter = 0;
	while (	bSendResultToUART == 0)				            // Wait for 16x samples to accumulate
		{}
		AdcGo(pADI_ADC1,ADCMDE_ADCMD_IDLE);		            // Put ADC1 into idle mode to allow re-configuration of its control registers
		for (ucCounter = 0; ucCounter < 16; ucCounter++)   // Accumulate 16x ADC1 readings from DMA destination
		{						
		    fVAIN0_AGND += (((float)uxADC0Data[ucCounter]*1.2)/ 268435456);	
		} 
	  	
		bSendResultToUART = 0;

		fVAIN0_AGND = fVAIN0_AGND/16;		                  // Calculate average value

	/* Step 3 - Measure AIN1 v AIN0 for voltage across RTD */
	    AdcBuf(pADI_ADC1,ADCCFG_EXTBUF_OFF,ADC_BUF_ON);	// Turn ADC input buffers on for this differential measurement
		AdcRng(pADI_ADC1,ADCCON_ADCREF_INTREF,ADCMDE_PGA_G32,ADCCON_ADCCODE_INT); // Set G=32 to measure across RTD
		AdcPin(pADI_ADC1,ADCCON_ADCCN_AIN1,ADCCON_ADCCP_AIN0);	 // Select AIN0/AIN1 as differential inputs
		DmaClr(DMARMSKCLR_ADC1,0,0,0);                     // Clear Masking of ADC1 DMA channel
		DmaCycleCntCtrl(ADC1_C,16,DMA_DSTINC_WORD|
        DMA_SRCINC_NO|DMA_SIZE_WORD|DMA_BASIC);          // Update cycle control and number of values to transfer fields
		AdcGo(pADI_ADC1,ADCMDE_ADCMD_CONT);			         // Start ADC1 for continuous conversions
   	ucCounter = 0;
		while (	bSendResultToUART == 0)					      // Wait for 16x samples to accumulate
		{}
		for (ucCounter = 0; ucCounter < 16; ucCounter++)   // Accumulate 16x ADC1 readings from DMA destination
		{
    	   fVRTD += (((float)uxADC0Data[ucCounter]*1.2)  / 268435456);
		}
	  	
		bSendResultToUART = 0;	 
		
		fVRTD = fVRTD/16;						  // Calcualte voltage across RTD
		AdcGo(pADI_ADC1,ADCMDE_ADCMD_IDLE);
	  /* Step 4 - Calculate RTD value in ohms and derive final temeprature */
		fRrtd = fVRTD/fIexc0;                		          // RTD resistance value
		fTRTD =	CalculateRTDTemp(fRrtd);			            // RTD temperature
		bSendResultToUART = 0;
		SendResultToUART();							                  // Send results to UART
	   DioTgl(pADI_GP1,0x8);						                  // Toggle LED, P1.3
		DmaCycleCntCtrl(ADC1_C,16,DMA_DSTINC_WORD|
        DMA_SRCINC_NO|DMA_SIZE_WORD|DMA_BASIC);          // Update cycle control and number of values to transfer fields
	}
}

void UARTINIT (void)
{
   	//Select IO pins for UART.
	  pADI_GP0->GPCON |= 0x3C;					            // Configure P0.1/P0.2 for UART		
//	pADI_GP0->GPCON |= 0x9000; 					          // Configure P0.6/P0.7 for UART
    UrtCfg(pADI_UART,B9600,COMLCR_WLS_8BITS,0);   // setup baud rate for 9600, 8-bits
    UrtMod(pADI_UART,COMMCR_DTR,0);  			        // Setup modem bits
    UrtIntCfg(pADI_UART,COMIEN_ERBFI|COMIEN_ETBEI|COMIEN_ELSI|COMIEN_EDSSI|COMIEN_EDMAT|COMIEN_EDMAR);  // Setup UART IRQ sources
}

void ADC1INIT(void)
{												
	AdcMski(pADI_ADC1,ADCMSKI_RDY,1);               // Enable ADC ready interrupt source		
  AdcFlt(pADI_ADC1,124,14,FLT_NORMAL|ADCFLT_NOTCH2|ADCFLT_CHOP); // ADC filter set for 3.75Hz update rate with chop on enabled
  AdcRng(pADI_ADC1,ADCCON_ADCREF_INTREF,ADCMDE_PGA_G32,ADCCON_ADCCODE_INT); // Internal reference selected, Gain of 32, Signed integer output
	// Turn off input buffers to ADC and external reference											
  AdcBuf(pADI_ADC1,ADCCFG_EXTBUF_OFF,ADCCON_BUFBYPN|ADCCON_BUFBYPP|ADCCON_BUFPOWP|ADCCON_BUFPOWN); 
	AdcPin(pADI_ADC1,ADCCON_ADCCN_AIN1,ADCCON_ADCCP_AIN0); // Select AIN0 as postive input and AIN1 as negative input
	AdcDmaCon(ADC1DMAREAD,1);     // Call function to init ADc1 for DMA reads
	AdcDmaReadSetup(ADC1DMAREAD,DMA_SIZE_WORD|DMA_DSTINC_WORD|DMA_SRCINC_NO|DMA_BASIC,16,uxADC0Data);
}
void DMAINIT(void)  
{
	DmaBase();
	DmaSet(0,DMAENSET_ADC1,0,DMAPRISET_ADC1);       // Enable ADC1 DMA primary structure
}
void IEXCINIT(void)
{
	IexcDat(IEXCDAT_IDAT_200uA,IDAT0En);         // Set output for 200uA
	IexcCfg(IEXCCON_PD_off,IEXCCON_REFSEL_Int,IEXCCON_IPSEL1_Off,IEXCCON_IPSEL0_AIN6); //Setup IEXC) for AIN6
}
void SendString (void)
{
   for ( i = 0 ; i < nLen ; i++ )	// loop to send ADC0 result
	{
  		 ucTxBufferEmpty = 0;
		 UrtTx(pADI_UART,szTemp[i]);
  		 while (ucTxBufferEmpty == 0)
  		 {
  		 }
	}
} 
void SendResultToUART(void)
{
	sprintf ( (char*)szTemp, "Voltage across Rref: %fV \r\n",fVAIN1_AGND );                          
	nLen = strlen((char*)szTemp);
	if (nLen <64)
 		SendString();
	
	sprintf ( (char*)szTemp, "Voltage across RTD : %fV \r\n",fVRTD );                          
	nLen = strlen((char*)szTemp);
	if (nLen <64)
 		SendString();
	
	sprintf ( (char*)szTemp, "RTD Resistance: %fOhms \r\n",fRrtd );                          
	nLen = strlen((char*)szTemp);
	if (nLen <64)
 		SendString();

	sprintf ( (char*)szTemp, "Excitation current: %fA \r\n",fIexc0 );                          
	nLen = strlen((char*)szTemp);
	if (nLen <64)
 		SendString();
	
	sprintf ( (char*)szTemp, "RTD Temperature: %fC \r\n\n\n",fTRTD );                          
	nLen = strlen((char*)szTemp);
	if (nLen <64)
 		SendString();
}
float CalculateRTDTemp(float r) 
{
	float t;
	int j;
	j=(int)((r-RMIN)/RSEG);       // determine which coefficients to use
	if (j<0)                      // if input is under-range..
		j=0;                        // ..then use lowest coefficients
	else if (j>NSEG-1)            // if input is over-range..
		j=NSEG-1;                   // ..then use highest coefficients
	t = C_rtd[j]+(r-(RMIN+RSEG*j))*(C_rtd[j+1]-C_rtd[j])/RSEG;
	return t;
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

}
void DMA_ADC1_Int_Handler ()
{
	DioTgl(pADI_GP1,0x8);				// Toggle P1.3
	ulDmaStatus = DmaSta();
	bSendResultToUART = 1;
	DmaSet(DMARMSKSET_ADC1,DMAENSET_ADC1,0,DMAPRISET_ADC1);
}
void DMA_Err_Int_Handler ()
{
	ulDmaStatus = DmaSta();
	DmaErr(DMA_ERR_CLR);
}
void SINC2_Int_Handler()
{
 
}
void UART_Int_Handler ()
{
   	volatile unsigned char ucCOMSTA0 = 0;
	volatile unsigned char ucCOMIID0 = 0;
	volatile unsigned int uiUartCapTime = 0;
	
	ucCOMSTA0 = UrtLinSta(pADI_UART);			// Read Line Status register
	ucCOMIID0 = UrtIntSta(pADI_UART);			// Read UART Interrupt ID register			
	if ((ucCOMIID0 & 0x2) == 0x2)	  			// Transmit buffer empty
	{
	  ucTxBufferEmpty = 1;
	}
	if ((ucCOMIID0 & 0x4) == 0x4)	  			// Receive byte
	{
		ucComRx	= UrtRx(pADI_UART);
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





