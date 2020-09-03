/**
 *****************************************************************************
   @example  Thermocouple_to_DAC.c
   @brief    This file expects a Thermocouple to be connected differentially to AIN2/AIN3 (J3 on the CN0300-EB1Z Demo board).
   - The RTD connected to AIN0/AIN1 will be used for Cold Junction compensation.
   - This file will measure the thermocouple/RTD inputs, convert this to an overall temperature.
   - This temperature is sent to the 4-20mA interface which is controlled by the VDAC in NPN mode.
   - ADC0 is used to measure a feedback voltage on AIN9 on the 4-20mA output circuit.
   - The voltage on this pin is linearlily related to the 4-20mA current. By measuring this voltage, the function
     void FineTuneDAC(void) adjusts the DAC output remove any linearity errors on the VDAC and the external voltage to current
     convertor circuit.

   - Thermocouple look-up tables assumes Type T thermocouple.
   - Temperature range is -200C to 350C.
   - Calibration options are included to calibrate the ADC. Modify the define calibrateADC1 below to review the different options
   - Calibration options are included to calibrate the DAC output. Modify the define calibrateDAC below to review the different options
   
   Baud rate of UART interface is 19200

   @version  V0.3
   @author   ADI
   @date     February 2013 
   @par Revision History:
   - V0.1, September 2010: initial version. 
   - V0.2, October 2012: Changed comments - 19200 baud for UART used.
         DAC value after initialization changed to ~120mA, was over 20mA
   - V0.3, February 2013: Corrected SystemFullCalibration() function.
                         Changed comments: ADC1 is used for temperature measurements.
                         Fixed a bug in SendString().

All files for ADuCM360/361 provided by ADI, including this file, are
provided  as is without warranty of any kind, either expressed or implied.
The user assumes any and all risk from the use of this code.
It is the responsibility of the person integrating this code into an application
to ensure that the resulting application performs as required and is safe.

**/

#include <stdio.h>
#include <string.h>
#include <ADuCM360.h>
#include "FlashEraseWrite.h"
#include "TempCalc.h"

#include <..\common\ClkLib.h>
#include <..\common\IexcLib.h>
#include <..\common\DioLib.h>
#include <..\common\WdtLib.h>
#include <..\common\UrtLib.h>
#include <..\common\GptLib.h>
#include <..\common\DioLib.h>
#include <..\common\AdcLib.h>
#include <..\common\DacLib.h>
#include <..\common\RstLib.h>


#define calibrateADC1	0		// Set to 0 if you don't want to calibrate
										      // Set to 1 if you want to calibrate externally	and save to flash
										      // set to 2 if you want to load previously saved values from
										      // flash
#define calibrateDAC	1		// Set to 0 if you don't want to calibrate and just use default values
										// Set to 1 if you want to calibrate externally	and save to flash
										// set to 2 if you want to load previously saved values from
										// flash										

#define THERMOCOUPLE 	0		// Used for switching ADC1 to Thermocouple channel
#define RTD 				1		// Used for switching ADC1 to RTD channel
#define SAMPLENO			0x5	// Number of samples to be taken between channel switching

// DAC Default Output Values
#define DEFAULT4mA 0xD800000          // DAC value that nominally gives 4mA
#define DEFAULT20mA 0x5280000         // DAC value that nominally gives 20mA

void ADC1INIT(void);									// Init ADC1
void ADC0INIT(void);									// Init ADC0
void UARTInit(void);			            // Enables UART
void IEXCINIT(void);	                // Setup Excitation Current sources
void DACINIT(void);                   // Init DAC for NPN mode
void delay(long int);								  // Simple delay function
void SendString(void);								// Transmit string using UART
void SystemZeroCalibration(void);			// Calibrate using external inputs
void SystemFullCalibration(void);			// Calibrate using external inputs

void ADC1RTDCfg(void);                // RTD ADC1 settings
void ADC1ThermocoupleCfg(void);       // Tc ADC1 settings
void SendResultToUART(void);			    // Send measurement results to UART - in ASCII String format
void UpdateDAC(void);                 // Convert Final temperature value to 4-20mA output current
void FineTuneDAC(void);               // Used to correct DAC output based on Feedback voltage on AIN9
void CalibrateDAC(void);              // Routine for calibrating DAC output - 4-20mA loop current must be monitored by precision Current meter.

volatile unsigned char bSendResultToUART = 0;	// Flag used to indicate ADC1 result ready to send to UART
volatile unsigned char ucComRx = 0;		// variable that ComRx is read into in UART IRQ
unsigned char szTemp[64] = "";				// Used to store string before printing to UART
unsigned char ucTxBufferEmpty  = 0;		// Used to indicate that the UART Tx buffer is empty
volatile  long ulADC1DATThermocouple[SAMPLENO];	// Variable that ADC1DAT is read into when sampling TC
volatile  long ulADC1DATRtd[SAMPLENO];// Variable that ADC1DAT is read into when sampling RTD
unsigned long ulADC1CONThermocouple;	// used to set ADC1CON which sets channel to thermocouple
unsigned long ulADC1CONRtd;			 			// used to set ADC1CON which sets channel to RTD
unsigned char ucADCInput;							// Used to indicate what channel the ADC1 is sampling
unsigned char ucSampleNo = 0;					// Used to keep track of when to switch channels
unsigned char ucIEXCCON = 0;					// Used to setup IEXCON
unsigned char ucIEXDAT = 0;						// Used to setup IEXDAT
unsigned char ucWaitForUart = 0;			// Used by calibration routines to wait for user input
volatile unsigned char ucADCERR = 0;	// Used to indicate an ADC error
unsigned int uiFEESTA;
volatile unsigned char ucFlashCmdStatus = 0;
volatile unsigned char ucWaitForCmdToComplete = 0;
unsigned long ulAbortAddress = 0;
unsigned long ulSelectPage;						// Used to store address of which page to erage
unsigned long szADC1[2];							// Used to store ADC1INTGN, ADC1OF after calibration
																      // and then loaded into flash
struct DAC_CAL
	{
    	unsigned long ul4mA_DACCODE;    // DAC output code that generates 4mA 			
    	unsigned long ul20mA_DACCODE;   // DAC output code that generates 20mA
		  long l4mA_AIN9CODE;        	 	  // ADC0 calibration reading for feedback voltage across Rload at 4mA
		  long l20mA_AIN9CODE;        	 	// ADC0 calibration reading for feedback voltage across Rload at 20mA
	};
struct DAC_CAL DAC_Calibration;		    // Create instance for this parts calibration values
struct DAC_CAL *ptr_DAC_Calibration = &DAC_Calibration;		// Create pointer to lowest member of the structure.																		

unsigned int uiArraySize = 0;					// size of array to be written to flash

long ulADC0DAT = 0;                   // Variable used to store ADC0 result. used to calculate compensation value for DAC output
 
float fVolts = 0.0;										// ADC to voltage constant
float fVThermocouple = 0.0;						// thermoucouple voltage
float fVRTD = 0.0 ;										// RTD voltage, 
float fRrtd = 0.0;										// resistance of the RTD
float fColdJVolt = 0.0;								// cold junction equivalent thermocouple voltage
float fFinalVoltage = 0.0;						// fFinalVoltage = thermocouple voltage + cold j voltage
float fTThermocouple = 0.0;					  // thermoucouple temperature
float fTRTD = 0.0;										// RTD temperature
float fFinalTemp = 0.0;								// Final temperature including cold j compensation
float fAIN9Voltage = 0.0;             // Used for determining AIN9 feedback voltage
float fCurrentOut = 0.0;              // Used for debug purposes to print expected output current
float fTempScale = 0.0;               // Variable used to determine fraction of full scale latest temperature is
float fCalVoltage = 0.0;              // Stores Expected AIN9 voltage for present Temperature reading
unsigned char nLen = 0;								// Used for sending strings to UART
unsigned char i = 0;									// counter used in SendString()
unsigned char ucCounter = 0;
unsigned long ulDefaultDAC4mA = 0xE300000; 
unsigned long ulDefaultDAC20mA = 0x5280000; 
unsigned long ulDacCode = 0;          // Latest Code written to VDAC to set loop current
unsigned long ul4mAVal = 0;           // Variable used to store DAC output code that generates 4mA
unsigned long ul20mAVal = 0;          // Variable used to store DAC output code that generates 20mA
unsigned char ucCalComplete = 0;      // Flag used in DAC calibration routine
unsigned char ucADC0Rdy = 0;          // Flag used in DAC calibration routine
unsigned char ucFirstLoop = 0;        // Flag used to indicate the first pass through the main while loop. On 1st pass, the VDAC is setup coursely, after this it is fine-tuned
long lDAC4mAAIN9 = 0;                 // AIN9 4mA calibration value
long lDAC20mAAIN9 = 0;                // AIN9 20mA calibration value
int uiNumCodesAdjust = 0;

int main (void)
{
	WdtCfg(T3CON_PRE_DIV1,T3CON_IRQ_EN,T3CON_PD_DIS);								// Turn off Watchdog timer
	ClkCfg(CLK_CD3,CLK_HF,CLKSYSDIV_DIV2EN,CLK_UCLKCG);					    // Set CPU clock to 1MHz
	ClkDis(0|CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|
	CLKDIS_DISI2CCLK|CLKDIS_DISPWMCLK);                             // Disable clock to unused peripherals
  ClkSel(CLK_CD7,CLK_CD7,CLK_CD0,CLK_CD7);				                // Enable UART clock - disable SPI/I2C/PWM clocks
	ucADCERR = 0;
	ulADC1CONRtd = 0x81001;				                                  // AIN0 = +ve input; AIN1 = -ve input, EXTREF
	//AdcPin(pADI_ADC1,ADCCON_ADCCN_AIN1,ADCCON_ADCCP_AIN0);
	ulADC1CONThermocouple = 0x80043;	                              // AIN2 = +ve input; AIN3 = -ve input, IntRef-AGND range
	//AdcPin(pADI_ADC1,ADCCON_ADCCN_AIN3,ADCCON_ADCCP_AIN2);
	DioOen(pADI_GP1,0x8);							                              // used for debug (pin 1.3)
	UARTInit();						                                          // Init UART to 9600	
  NVIC_EnableIRQ(FLASH_IRQn);					                            // Enable Flash and UART interrupt sources
	NVIC_EnableIRQ(UART_IRQn);
	ucADCInput = THERMOCOUPLE;			                                //	Indicate that ADC1 is sampling thermocouple
	ADC0INIT();								                                      // Init ADC0
	NVIC_EnableIRQ(ADC0_IRQn);					                            
	ADC1INIT();								                                      // Init ADC1	
	IEXCINIT();																										  // Init IEXC0 for 200uA on AIN5
	NVIC_EnableIRQ(ADC1_IRQn);					                            // ADC0/UART/ADC1 IRQ
	
	AdcGo(pADI_ADC0,ADCMDE_ADCMD_CONT);                             // Enable ADC0 in continuous mode
	DACINIT();
	sprintf ( (char*)szTemp, "Program Started. Please wait for the first temperature result\r\n");
	nLen = strlen((char*)szTemp);
	if (nLen <64)
 		SendString();

	fVolts	= (1.2 / 268435456);			                                      // Internal reference - calcualte LSB voltage value	
	ucFirstLoop = 1;
	while(1)
	{
   if(bSendResultToUART == 1)
		{
			fVThermocouple = 0;
			fVRTD =0;
			for (ucCounter = 0; ucCounter < SAMPLENO; ucCounter++)
			{
				fVThermocouple += (ulADC1DATThermocouple[ucCounter] * fVolts);  	// Thermocouple voltage
				fVRTD += ((float)ulADC1DATRtd[ucCounter]  / 268435456);	  		    // RTD voltage	in terms of reference voltage
			}
			fVThermocouple = fVThermocouple/SAMPLENO;					                  // Get the average of the results
			fVRTD = fVRTD/SAMPLENO;
			fRrtd = fVRTD * 5600;											                          // RTD resistance
			fTRTD =	CalculateRTDTemp(fRrtd);							                      // RTD temperature
			fColdJVolt = CalculateColdJVoltage(fTRTD);				                  // get an equvalent thermocouple voltage
			fFinalVoltage = fVThermocouple + fColdJVolt;
			//fTThermocouple = CalculateThermoCoupleTemp(fVThermocouple);
			fFinalTemp = CalculateThermoCoupleTemp(fFinalVoltage);	            // Thermocouple temperature
			if (ucFirstLoop == 1)
				UpdateDAC();
			else
			   FineTuneDAC();
			ucFirstLoop = 0;
			SendResultToUART();
      bSendResultToUART = 0;
		}
		if (ucADCERR != 0)
		{
		   if (ucADCERR == 1)
		   		sprintf ( (char*)szTemp, "ADC error on ADC0  \r\n");// Send error message to UART  
			if (ucADCERR == 2)
		   		sprintf ( (char*)szTemp, "ADC error on ADC1  \r\n");// Send error message to UART
		   if ((ucADCERR == 1) | (ucADCERR == 2))                          
			{	nLen = strlen((char*)szTemp);
			  	if (nLen <64)
		 			SendString();
	      }
			ucADCERR = 0;
	   }
	}
}
// Setup ADC1 to measure the RTD input
void ADC1RTDCfg(void)
{
	AdcPin(pADI_ADC1,ADCCON_ADCCN_AIN1,ADCCON_ADCCP_AIN0);                    // AIN0/AIN1 input channels
	AdcRng(pADI_ADC1,ADCCON_ADCREF_EXTREF,ADCMDE_PGA_G32,ADCCON_ADCCODE_INT); // External reference, G32 used
}
// Setup ADC1 to measure the Thermocouple input
void ADC1ThermocoupleCfg(void)
{
	AdcPin(pADI_ADC1,ADCCON_ADCCN_AIN3,ADCCON_ADCCP_AIN2);                    // Select AIn2/AIN3 as ADC inputs
	AdcRng(pADI_ADC1,ADCCON_ADCREF_INTREF,ADCMDE_PGA_G32,ADCCON_ADCCODE_INT); // Internal reference, Gain=32
}

// This function is called only on the first pass through the main while loop - used to set first current output value
void UpdateDAC(void)
{

	unsigned long ulDACRange = 0;
	float fCalV_Range = 0.0; // Calibration voltage range = AIN9 voltage @4mA - AIN9 voltage @20mA
	
	if ((fFinalTemp > 370)|(fFinalTemp < -220))
		// Temperature outside range - set error current
	   DacWr(0,0xF000000); // Set fault current level< 4mA
	else 
	{
		fTempScale = (fFinalTemp+200)/550;                            // Sets ratio value of measured temperature v overall Thermocouple range
		fAIN9Voltage = (float)(ulADC0DAT*fVolts);
		fCalV_Range = (float)((lDAC4mAAIN9 *fVolts)-(lDAC20mAAIN9*fVolts));
		fCalVoltage = (float)((lDAC4mAAIN9 *fVolts)- (fCalV_Range*fTempScale));
		ulDACRange = (ul4mAVal>>16) -(ul20mAVal>>16);                 // Convert temperature to DAC code that sets 4-20mA loop value
		ulDacCode = (unsigned long) (ulDACRange*fTempScale );
		ulDacCode = ul4mAVal - (ulDacCode << 16);
		fCurrentOut = (fTempScale*16.0)+4.0;                          // Used for debug purposes to print expected output current
		DacWr(0,ulDacCode);                                           // Set output current based on Final Temperature measured
	}
}
// DAC output is updated based on the measured feedback voltage of AIN9
void FineTuneDAC(void)
{
	unsigned long ulDacDat = 0;
	unsigned long ulDACAdjust = 0;
	float fCalV_Range = 0.0; // Calibration voltage range = AIN9 voltage @4mA - AIN9 voltage @20mA
	float fErrorVoltage = 0.0;

    fTempScale = (fFinalTemp+200)/550;                            // Sets ratio value of measured temperature v overall Thermocouple range
		fAIN9Voltage = (float)(ulADC0DAT*fVolts);
		fCalV_Range = (float)((lDAC4mAAIN9 *fVolts)-(lDAC20mAAIN9*fVolts));
		fCalVoltage = (float)((lDAC4mAAIN9 *fVolts)- (fCalV_Range*fTempScale));
	  ulDacDat = pADI_DAC->DACDAT;                                  // read latest DAC output data value
		fCurrentOut = (fTempScale*16.0)+4.0;                          // Used for debug purposes to print expected output current
	  if (fAIN9Voltage > fCalVoltage)                               // Output Current is too low - need to reduce DAC voltage
		{
	      fErrorVoltage = fAIN9Voltage - fCalVoltage;               // Find the error
			  ulDACAdjust = (unsigned long)((fErrorVoltage/1.2)*0xFFF); // Convert error to DAC codes
         uiNumCodesAdjust = ulDACAdjust;
			  ulDACAdjust = (ulDACAdjust << 16);
			  ulDacDat = ulDacDat - ulDACAdjust;                      // Correct DAC output
		}
		else if (fAIN9Voltage < fCalVoltage)                          // Output Current is too High - need to increase DAC voltage
		{
	      fErrorVoltage = fCalVoltage - fAIN9Voltage;               // Find the error
			  ulDACAdjust = (unsigned long)((fErrorVoltage/1.2)*0xFFF); // Convert error to DAC codes
         uiNumCodesAdjust = ulDACAdjust;
			  ulDACAdjust = (ulDACAdjust << 16);
			  ulDacDat = ulDacDat + ulDACAdjust;                      // Correct DAC output
		}
		DacWr(0,ulDacDat);                                           // Set output current based on Final Temperature measured
	
}
// System offset calibration for ADC1
void SystemZeroCalibration(void)
{
	ucWaitForUart = 1;
	sprintf ( (char*)szTemp, "Set Zero Scale Voltage - Press return when ready \r\n");                         
	nLen = strlen((char*)szTemp);
 	if (nLen <64)
		 	SendString();
	while (ucWaitForUart == 1)
	{}
	AdcGo(pADI_ADC1,ADCMDE_ADCMD_SYSOCAL);	// ADC1 System Zero scale calibration
	while ((AdcSta(pADI_ADC1) &0x20) != 0x20)			// bit 5 set by adc when calibration is complete
	{}
}
// System Full-scale  calibration for ADC1
void SystemFullCalibration(void)
{
	ucWaitForUart = 1;
	sprintf ( (char*)szTemp, "Set Full Scale Voltage - Press return when ready \r\n");                         
	nLen = strlen((char*)szTemp);
 	if (nLen <64)
		 	SendString();
	while (ucWaitForUart == 1)
	{}
	AdcGo(pADI_ADC1,ADCMDE_ADCMD_SYSGCAL);							// ADC1 System Full scale calibration
	while ((AdcSta(pADI_ADC1) &0x20) != 0x20)			// bit 5 set by adc when calibration is complete
	{}
}
//ADC1 will measure Thermocouple on AIN2/3 and RTD on AIN0/1
void ADC1INIT(void)
{
	unsigned long *pWrite;
	volatile unsigned char ucEraseSuccess = 0;
	AdcBias(pADI_ADC1,ADCCFG_PINSEL_AIN7,ADC_BIAS_X1,0);	          //vbias ain7 buffers on
  AdcBuf(pADI_ADC1,ADCCFG_EXTBUF_VREFPN,ADC_BUF_ON);              //External reference buffers on
	AdcMski(pADI_ADC1,ADCMSKI_RDY,1);						                  	// Enable ADC1 /rdy IRQ
	// Thermocouple settings
	AdcPin(pADI_ADC1,ADCCON_ADCCN_AIN3,ADCCON_ADCCP_AIN2);          // Select AIn2/AIN3 as ADC inputs
	AdcRng(pADI_ADC1,ADCCON_ADCREF_INTREF,ADCMDE_PGA_G32,ADCCON_ADCCODE_INT); // Internal reference, Gain=32
//	ADC1CON = ulADC1CONThermocouple;
	AdcFlt(pADI_ADC1,124,0xD00,FLT_NORMAL);							              // Chop On, 3.75Hz sampling rate, chop on
	AdcGo(pADI_ADC1,ADCMDE_ADCMD_IDLE);                             // Set ADC1 for Idle mode
	delay(0xFFF);
	if (calibrateADC1 == 1)
	{			
		SystemZeroCalibration();
		SystemFullCalibration();
		ulSelectPage = 0x1FC00;
		ucEraseSuccess = ErasePage(ulSelectPage);
		szADC1[0] = pADI_ADC1->INTGN;                                // store Internal gain cal
		szADC1[1] = pADI_ADC1->OF;                                   // store Offset cal
		pWrite = &szADC1[0];
		uiArraySize = sizeof(szADC1);
		WriteToFlash(pWrite,ulSelectPage,uiArraySize);
	}
	else if (calibrateADC1 == 2)
	{
		pADI_ADC1->INTGN = *( unsigned long      *)0x0001FC00;
		pADI_ADC1->OF = *( unsigned long      *)0x0001FC04;
	}
	delay(0xFFF);
	AdcGo(pADI_ADC1,ADCMDE_ADCMD_CONT);							// PGA = 32	 continuous receive mode
}
//ADC0 will measure AIN9 input - used to improve DAC output linearity performance
void ADC0INIT(void)
{
	AdcRng(pADI_ADC0,ADCCON_ADCREF_INTREF,ADCMDE_PGA_G2,ADCCON_ADCCODE_INT); // Internal Reference selected, PGA gain of 2
	AdcBuf(pADI_ADC0,ADCCFG_EXTBUF_VREFPN,ADC_BUF_ON);                       // ADC input buffer on. Leave External reference buffer on for RTD measurement
	AdcPin(pADI_ADC0,ADCCON_ADCCN_AGND,ADCCON_ADCCP_AIN9);                   // Select AIN9 as the positive input; AGND as negative input
	AdcFlt(pADI_ADC0,124,0xD00,FLT_NORMAL);                                  // Select 3.75Hz sampling rate
	AdcMski(pADI_ADC0,ADCMSKI_RDY,1);                                        // Enable ADC0 interrupt
	AdcGo(pADI_ADC0,ADCMDE_ADCMD_IDLE);                                      // Enable ADC0 in Idle mode
}
// Setup UART for 19200 Baud
void UARTInit(void)
{
   UrtCfg(pADI_UART,B9600*2,COMLCR_WLS_8BITS,0);   // setup baud rate for 19200, 8-bits
   UrtMod(pADI_UART,COMMCR_DTR,0);  			  // Setup modem bits
   UrtIntCfg(pADI_UART,COMIEN_ERBFI|COMIEN_ETBEI|COMIEN_ELSI|COMIEN_EDSSI|COMIEN_EDMAT|COMIEN_EDMAR);  // Setup UART IRQ sources
   DioPul(pADI_GP0,0xFF);								              // Enable pullup on P0.7/0.6
   DioCfg(pADI_GP0,0x3C);								              // Configure P0.2/P0.1 for UART
}

// Setup Excitation Current for 200uA on AIN5 pin - used for RTD
void IEXCINIT(void)
{
	IexcCfg(IEXCCON_PD_off,IEXCCON_REFSEL_Int,IEXCCON_IPSEL1_Off,IEXCCON_IPSEL0_AIN5);  // AIN5 for 200uA excitation current source - CN221 & CN300 circuit boards
  //IexcCfg(IEXCCON_PD_off,IEXCCON_REFSEL_Int,IEXCCON_IPSEL1_Off,IEXCCON_IPSEL0_AIN6);   // ADuCM360MKZ board
	IexcDat(IEXCDAT_IDAT_200uA,IDAT0Dis);					 	
}
// Setup DAC for NPN output mode. Calibrate if necessary
void DACINIT(void)
{
	DacCfg(DACCON_CLR_Off,DACCON_RNG_IntVref,DACCON_CLK_HCLK|DACCON_NPN,DACCON_MDE_12bit);
	
	// Calibration Routine
	 DacWr(0,0xA000000);
	if (calibrateDAC == 0)                          // Use default values
	{
		ul4mAVal = DEFAULT4mA;                        // Use Default DAC output to generate 4mA output
    ul20mAVal = DEFAULT20mA;                      // Use Default DAC output to generate 20mA output
	}
	if (calibrateDAC == 1)                          // Calibrate DAC and use these values
	{
		CalibrateDAC();
	}
	if (calibrateDAC == 2)                          // Load calibration values from Flash 
	{
    ul4mAVal = *( unsigned long *)0x0001F000;     // Use Pre-stored 4mA DAC calibration value
    ul20mAVal = *( unsigned long *)0x0001F004;    // Use Pre-stored 20mA DAC calibration value
		lDAC4mAAIN9 = *( unsigned long *)0x0001F008;  // Use Pre-stored 4mA AIN9 voltage value	
    lDAC20mAAIN9 = *( unsigned long *)0x0001F00C; // Use Pre-stored 20mA AIN9 voltage value
	}

}
// Function for calibrating DAC  for 4-20mA output
// 4-20mA output must be measured by accurate current meter 
void CalibrateDAC(void)
{
	volatile unsigned char ucEraseSuccess = 0;
	unsigned long *pWrite;
	// Calibrate 4mA first
	ul4mAVal = DEFAULT4mA;
	DacWr(0,ul4mAVal);
	ucCalComplete = 0;
	sprintf ( (char*)szTemp, "DAC Calibration Routine - calibrate to 4mA \r\n");                         
	nLen = strlen((char*)szTemp);
 	if (nLen <64)
		 	SendString();
	sprintf ( (char*)szTemp, "Press 1 to increase Output current - Press return when ready \r\n");                         
	nLen = strlen((char*)szTemp);
 	if (nLen <64)
		 	SendString();
	sprintf ( (char*)szTemp, "Press 0 to Decrease Output current - Press return when ready \r\n");                         
	nLen = strlen((char*)szTemp);
 	if (nLen <64)
		 	SendString();
	sprintf ( (char*)szTemp, "Press return when Complete - Press return when ready \r\n\n\n");                         
	nLen = strlen((char*)szTemp);
 	if (nLen <64)
		 	SendString();
	while (ucCalComplete == 0)
	{
		if (ucComRx == 0x31)                                    // Character "1" received, so increase output current
		{

			ul4mAVal = (ul4mAVal-0x10000);                        // Current output increases when DAC voltage decreases 
			DacWr(0,ul4mAVal);
			ucComRx = 0;
		}
		if (ucComRx == 0x30)                                    // Character "0" received, so Decrease output current
		{
			ul4mAVal = (ul4mAVal+0x10000);
			DacWr(0,ul4mAVal);                                    // Current output decreases when DAC voltage increases
			ucComRx = 0;
		}
	}
	
	AdcGo(pADI_ADC0,ADCMDE_ADCMD_IDLE);                             // Enable ADC0 in Idle mode
	delay (0xFF); // delay for AIN9 measurement to update
	ucADC0Rdy = 0;
	AdcGo(pADI_ADC0,ADCMDE_ADCMD_CONT);                             // Enable ADC0 in continuous mode
	while (ucADC0Rdy == 0){}
	lDAC4mAAIN9 = ulADC0DAT;
	sprintf ( (char*)szTemp, "4mA AIN9 voltage: %ul \r\n",(lDAC4mAAIN9) );// Send the Result to the UART                          
	nLen = strlen((char*)szTemp);
 	if (nLen <64)
		 	SendString();	
		
			// Calibrate 20mA Input
	ul20mAVal = DEFAULT20mA;
	DacWr(0,ul20mAVal);
	ucCalComplete = 0;
	sprintf ( (char*)szTemp, "DAC Calibration Routine - calibrate to 20mA \r\n");                         
	nLen = strlen((char*)szTemp);
 	if (nLen <64)
		 	SendString();
	sprintf ( (char*)szTemp, "Press 1 to increase Output current - Press return when ready \r\n");                         
	nLen = strlen((char*)szTemp);
 	if (nLen <64)
		 	SendString();
	sprintf ( (char*)szTemp, "Press 0 to Decrease Output current - Press return when ready \r\n");                         
	nLen = strlen((char*)szTemp);
 	if (nLen <64)
		 	SendString();
	sprintf ( (char*)szTemp, "Press return when Complete - Press return when ready \r\n");                         
	nLen = strlen((char*)szTemp);
 	if (nLen <64)
		 	SendString();
	while (ucCalComplete == 0)
	{
		if (ucComRx == 0x31)                                    // Character "1" received, so increase output current
		{

			ul20mAVal = (ul20mAVal-0x10000);                      // Current output increases when DAC voltage decreases 
			DacWr(0,ul20mAVal);
			ucComRx = 0;
		}
		if (ucComRx == 0x30)                                    // Character "0" received, so Decrease output current
		{
			ul20mAVal = (ul20mAVal+0x10000);
			DacWr(0,ul20mAVal);                                   // Current output decreases when DAC voltage increases
			ucComRx = 0;
		}
	}
	
	AdcGo(pADI_ADC0,ADCMDE_ADCMD_IDLE);                       // Enable ADC0 in Idle mode
	delay (0xFF);                                             // delay for AIN9 measurement to update
	ucADC0Rdy = 0;
	AdcGo(pADI_ADC0,ADCMDE_ADCMD_CONT);                       // Enable ADC0 in continuous mode
	while (ucADC0Rdy == 0){}
	lDAC20mAAIN9 = ulADC0DAT;
	sprintf ( (char*)szTemp, "20mA AIN9 voltage: %ul \r\n",(lDAC20mAAIN9) );// Send the Result to the UART                          
	nLen = strlen((char*)szTemp);
 	if (nLen <64)
		 	SendString();

// Write the calibration values for the DAC to Flash page 0x1F000	
	ulSelectPage = 0x1F000;
	ucEraseSuccess = ErasePage(ulSelectPage);                     // Erase flash page used for calibrating the VDAC
	pWrite = (unsigned long *)ptr_DAC_Calibration;
	ptr_DAC_Calibration->	ul4mA_DACCODE = ul4mAVal;
  ptr_DAC_Calibration->	ul20mA_DACCODE = ul20mAVal;
  ptr_DAC_Calibration->	l4mA_AIN9CODE = lDAC4mAAIN9;	
  ptr_DAC_Calibration->	l20mA_AIN9CODE = lDAC20mAAIN9;
	
	uiArraySize = sizeof(DAC_Calibration);
	WriteToFlash(pWrite,ulSelectPage,uiArraySize);
	}
void delay (long int length)
{
	while (length >0)

    	length--;
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
    sprintf ( (char*)szTemp, "RTD Resistance: %fOhms \r\n",fRrtd );                          
    nLen = strlen((char*)szTemp);
    if (nLen <64)
            SendString();
    
    sprintf ( (char*)szTemp, "RTD Temperature: %fC \r\n",fTRTD );// Send the Result to the UART                          
    nLen = strlen((char*)szTemp);
    if (nLen <64)
            SendString();
    
    sprintf ( (char*)szTemp, "Cold Junction Voltage: %fmV \r\n",(fColdJVolt*1000) );// Send the Result to the UART                          
    nLen = strlen((char*)szTemp);
    if (nLen <64)
            SendString();
    
    sprintf ( (char*)szTemp, "Thermoucouple Voltage: %fmV \r\n",(fVThermocouple*1000) );// Send the Result to the UART                          
    nLen = strlen((char*)szTemp);
    if (nLen <64)
            SendString();

    sprintf ( (char*)szTemp, "Expected DAC Current: %fmA \r\n",fCurrentOut );// Used for debugging DAC output                          
    nLen = strlen((char*)szTemp);
    if (nLen <64)
            SendString(); 
    
    sprintf ( (char*)szTemp, "Final Temperature: %fC \r\n",fFinalTemp );// Send the Result to the UART                          
    nLen = strlen((char*)szTemp);
    if (nLen <64)
            SendString();
		sprintf ( (char*)szTemp, "Expected AIN9 Voltage: %fV \r\n",fCalVoltage );// Send the Result to the UART                          
    nLen = strlen((char*)szTemp);
    if (nLen <64)
            SendString();
    
    sprintf ( (char*)szTemp, "Number of DAC codes changed: %d \r\n",uiNumCodesAdjust );// Send the Result to the UART                          
    nLen = strlen((char*)szTemp);
    if (nLen <64)
            SendString();

		sprintf ( (char*)szTemp, "AIN9 Voltage: %fV \r\n\n\n\n",fAIN9Voltage );// Send the Result to the UART                          
    nLen = strlen((char*)szTemp);
    if (nLen <64)
            SendString();
}
void ExtIntEnable(void) 
{
   
}
void RESET_EXCPT_HNDLR         ()
{
	#ifdef __GNUC__
    unsigned long *pulSrc, *pulDest;

    // Copy initialised data from flash into RAM
    pulSrc = &_etext;
    for(pulDest = &_data; pulDest < &_edata; )
    {
        *pulDest++ = *pulSrc++;
    }

    // Clear the bss segment
    for(pulDest = &_bss; pulDest < &_ebss; )
    {
        *pulDest++ = 0;
    }
#endif
    // Call application main.
    main();

    // Stick here if main returns
    while(1);
} 
void SysTick_Handler           () 
{}
void NmiSR                     () 
{}
void FaultISR                  ()
{} 
void MemManage_Handler         () 
{}
void BusFault_Handler          () 
{}
void UsageFault_Handler        ()
{} 
void SVC_Handler               () 
{}
void DebugMon_Handler          () 
{}
void PendSV_Handler            ()
{} 
void WakeUp_Int_Handler()
{
}
void Ext_Int0_Handler ()
{           
}
void Ext_Int1_Handler ()
{           
}
void Ext_Int2_Handler ()
{   
}
void Ext_Int3_Handler ()
{           
}
void Ext_Int4_Handler ()
{           
}
void Ext_Int5_Handler ()
{           
}
void Ext_Int6_Handler ()
{           
}
void Ext_Int7_Handler ()
{           
}
void WDog_Tmr_Int_Handler()
{}
void Test_OSC_Int_Handler()
{}
void GP_Tmr0_Int_Handler()
{}
void GP_Tmr1_Int_Handler()
{}
void ADC0_Int_Handler()
{
   volatile unsigned int uiADCSTA = 0;
   

   uiADCSTA = AdcSta(pADI_ADC0);
   if ((uiADCSTA & 0x10) == 0x10)			// Check for an error condition
   		ucADCERR = 2;
	 ulADC0DAT = AdcRd(pADI_ADC0);
	 ucADC0Rdy = 1;
}
void ADC1_Int_Handler ()
{
   volatile unsigned int uiADCSTA = 0;
   volatile long ulADC1DAT = 0;

   uiADCSTA = AdcSta(pADI_ADC1);
   if ((uiADCSTA & 0x10) == 0x10)			// Check for an error condition
   		ucADCERR = 2;
	ulADC1DAT = AdcRd(pADI_ADC1);
	if( bSendResultToUART == 0 )
	{
		if(ucSampleNo < SAMPLENO){
			if (ucADCInput == THERMOCOUPLE){
				ulADC1DATThermocouple[ucSampleNo] = ulADC1DAT;}
			else{   
				ulADC1DATRtd[ucSampleNo] = ulADC1DAT;}
			ucSampleNo++;	
	  	}
	  	else{
	   	ucSampleNo = 0;
			if (ucADCInput == THERMOCOUPLE)
			{
				ADC1RTDCfg();
				ucADCInput = RTD;
			}
			else
			{
				ADC1ThermocoupleCfg();
				ucADCInput = THERMOCOUPLE;
				bSendResultToUART = 1;
      }
		}
	}
}
void SINC2_Int_Handler ()
{
} 
void Flsh_Int_Handler ()
{
	uiFEESTA = 0;
	uiFEESTA = pADI_FEE->FEESTA;

	if ((uiFEESTA & 0x30) == 0x00)	// Command completed Successfully
	{
	   ucFlashCmdStatus = 0;			// Command passed
	}
	if ((uiFEESTA & 0x30) == 0x10)	// Error: Attempted erase of protected location
	{
	   ucFlashCmdStatus = 1;			// Command failed - protection error
	}
	if ((uiFEESTA & 0x30) == 0x20)	// Error: Sign error or, Erase error
	{
	   	   ucFlashCmdStatus = 2;	// Command failed - Sign/Erase error
	}
	if ((uiFEESTA & 0x30) == 0x30)	// Error: Command aborted.
	{
		 ulAbortAddress = pADI_FEE->FEEADRAH;
		 ulAbortAddress = (ulAbortAddress << 16);
		 ulAbortAddress |= pADI_FEE->FEEADRAL;
		 ucFlashCmdStatus = 3;			// Command failed - Command aborted before complete
	}
	if ((uiFEESTA & 0x8) == 0x8)		// Write Complete
	{
		 
	}
	if ((uiFEESTA & 0x4) == 0x4)		// Command Complete
	{
		 
	}
	ucWaitForCmdToComplete = 0;
}   
void UART_Int_Handler ()
{
   volatile unsigned char ucCOMSTA0 = 0;
	volatile unsigned char ucCOMIID0 = 0;
	
	ucCOMSTA0 = UrtLinSta(pADI_UART);			// Read Line Status register
	ucCOMIID0 = UrtIntSta(pADI_UART);			// Read UART Interrupt ID register
	if ((ucCOMIID0 & 0x2) == 0x2)	  			// Transmit buffer empty
	{
	  ucTxBufferEmpty = 1;
	}
	if ((ucCOMIID0 & 0x4) == 0x4)	  			// Receive byte
	{
		ucComRx	= UrtRx(pADI_UART);
		ucWaitForUart = 0;
		if (ucComRx == 0xD)                 // "Carriage return" detected
			ucCalComplete = 1;
	}
} 
void SPI0_Int_Handler ()
{
}
void SPI1_Int_Handler ()
{
}
void I2C0_Slave_Int_Handler ()
{
}
void I2C0_Master_Int_Handler ()
{
}
void DMA_Err_Int_Handler ()
{
}
void DMA_SPI1_TX_Int_Handler ()
{
}
void DMA_SPI1_RX_Int_Handler ()
{
}
void DMA_UART_TX_Int_Handler ()
{
}

void DMA_I2C0_STX_Int_Handler ()
{
}

void DMA_I2C0_SRX_Int_Handler ()
{
}
void DMA_I2C0_MTX_Int_Handler ()
{
}
void DMA_UART_RX_Int_Handler ()
{
}
void DMA_I2C0_MRX_Int_Handler ()
{
}
void DMA_ADC0_Int_Handler ()
{
}
void DMA_ADC1_Int_Handler ()
{
}
void DMA_DAC_Out_Int_Handler ()
{
}
void DMA_SINC2_Int_Handler ()
{
}
void PWM0_Int_Handler ()
{
}
void PWM1_Int_Handler ()
{
}
void PWM2_Int_Handler ()
{
}
void PWM3_Int_Handler ()
{
}
void PWMTRIP_Int_Handler ()
{
}

