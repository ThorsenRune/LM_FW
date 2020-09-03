/**
 *****************************************************************************
   @example  Thermocouple_to_UART.c
   @brief    This file expects a Thermocouple to be connected differentially to AIN2/AIN3.
   - The RTD connected to AIN0/AIN1 will be used for Cold Junction compensation.
   - This file will measure the thermocouple/RTD inputs and send the measured voltages and
     temperature to the UART (9600 baud by default).
   - For this simple example, the internal reference will used for the thermocouple measurement
     and a precision 5k6 resistor as the reference for the RTD

   @version V0.2
   @author  ADI
   @date    February 2013

   @par     Revision History:
   - V0.1, September 2012: initial version. 
   - V0.2, February 2013: Fixed a bug in SendString().
                          Corrected C_cold_junctionN variable.

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
										// set to 2 if you want to load previosly saved values from
										// flash

#define THERMOCOUPLE 	0		// Used for switching ADC0 to Thermocouple channel
#define RTD 				1		// Used for switching ADC0 to RTD channel
#define SAMPLENO			0x5	// Number of samples to be taken between channel switching


// Thermocouple temperature constants
#define THER_T_MIN_P (0)  			// = minimum positive temperature in degC 
#define THER_T_MAX_P (350) 	 	// = maximum positive temperature in degC
#define THER_V_MIN_P (0)  			// = input voltage in mV at 0 degC
#define THER_V_MAX_P (17.819)  	// = input voltage in mV at 350 degC
#define THER_N_SEG_P (30)  		// = number of sections in table
#define THER_V_SEG_P (0.59397) 	// = (THER_V_MAX-THER_V_MIN)/THER_N_SEG = Voltage in mV of each segment
#define THER_T_MIN_N (0)  			// = minimum negative temperature in degC
#define THER_T_MAX_N (-200)	  	// = maximum negative temperature in degC
#define THER_V_MIN_N (0)  			// = input voltage in mV at 0 degC
#define THER_V_MAX_N (-5.603)  	// = input voltage in mV at -200 degC
#define THER_N_SEG_N (20)  		// = number of sections in table
#define THER_V_SEG_N (-0.28015) 	// = (THER_V_MAX-THER_V_MIN)/THER_N_SEG = Voltage in mV of each segment
// Thermocouple lookup tables....
const float C_themocoupleP[THER_N_SEG_P+1] = {0.0, 	15.1417, 	29.8016, 	44.0289, 	57.8675, 	71.3563, 
									84.5295, 	97.4175, 	110.047, 	122.441, 	134.62,		146.602, 
									158.402, 	170.034, 	181.51, 	192.841, 	204.035, 	215.101, 
									226.046, 	236.877, 	247.6,		258.221, 	268.745, 	279.177, 
									289.522, 	299.784, 	309.969, 	320.079, 	330.119, 	340.092, 
									350.001};
const float C_themocoupleN[THER_N_SEG_N+1] = {0.0,		-7.30137,	-14.7101,	-22.2655, 
									-29.9855, 	-37.8791, 	-45.9548, 	-54.2258, 
									-62.7115, 	-71.4378, 	-80.4368,	-89.7453, 
									-99.4048, 	-109.463, 	-119.978, 	-131.025, 
									-142.707, 	-155.173, 	-168.641, 	-183.422, 
									-199.964};
// Cold Junction constants
#define COLDJ_T_MIN_P (0)  		// = minimum positive temperature in degC 
#define COLDJ_T_MAX_P (125)		// = maximum positive temperature in degC
#define COLDJ_V_MIN_P (0)  		// = input voltage in mV at 0 degC
#define COLDJ_V_MAX_P (5.470)  	// = input voltage in mV at 125 degC
#define COLDJ_N_SEG_P (20) 		// = number of sections in table
#define COLDJ_T_SEG_P (6.25)		// = (COLDJ_T_MAX-COLDJ_T_MIN)/COLDJ_N_SEG = Temperature in degC of each segment
#define COLDJ_T_MIN_N (0)  		// = minimum negative temperature in degC
#define COLDJ_T_MAX_N (-40)	  	// = maximum negative temperature in degC
#define COLDJ_V_MIN_N (0)  		// = input voltage in mV at 0 degC
#define COLDJ_V_MAX_N (-1.475)	// = input voltage in mV at -40 degC
#define COLDJ_N_SEG_N (10) 		// = number of sections in table
#define COLDJ_T_SEG_N (-4) 		// = (COLDJ_T_MAX-COLDJ_T_MIN)/COLDJ_N_SEG = Temperature in degC of each segment
//Cold junction lookup table. Used for converting cold junction temperature to thermocouple voltage
const float C_cold_junctionP[COLDJ_N_SEG_P+1] = {0.0,	0.2435,	0.4899,	0.7393,	0.9920,	1.2479, 1.5072,	1.7698,
												2.0357,	2.3050,	2.5776,	2.8534,	3.1323,	3.4144,	3.6995, 3.9875,
												4.2785,	4.5723,	4.8689,	5.1683,	5.4703};
const float C_cold_junctionN[COLDJ_N_SEG_N+1] = {0.0,		-0.1543,	-0.3072,	-0.4586,	-0.6085,
												-0.7568,	-0.9036,	-1.0489,	-1.1925,	-1.3345,
												-1.4750};
//RTD constants
#define TMIN (-40)  		// = minimum temperature in degC
#define TMAX (125)  		// = maximum temperature in degC
#define RMIN (84.2707)  // = input resistance in ohms at -40 degC
#define RMAX (147.951)  // = input resistance in ohms at 125 degC
#define NSEG 30  			// = number of sections in table
#define RSEG 2.12269  	// = (RMAX-RMIN)/NSEG = resistance  in ohms of each segment
//RTD lookup table
const float C_rtd[] = {-40.0006,-34.6322,-29.2542,-23.8669,-18.4704,-13.0649,-7.65042,-2.22714,3.20489,8.64565,14.0952,
						19.5536,25.0208,30.497,35.9821,41.4762,46.9794,52.4917,58.0131,63.5436,69.0834,74.6325,80.1909,
						85.7587,91.3359,96.9225,102.519,108.124,113.74,119.365,124.999};



void ADC1INIT(void);									// Init ADC1
void UARTInit(void);			            // Enables UART
void IEXCINIT(void);	                // Setup Excitation Current sources
void delay(long int);								  // Simple delay function
void SendString(void);								// Transmit string using UART
void SystemZeroCalibration(void);			// Calibrate using external inputs
void SystemFullCalibration(void);			// Calibrate using external inputs
float CalculateRTDTemp(float r);			// returns RTD Temperature reading
float CalculateThermoCoupleTemp(float v);		// returns Thermocouple Temperature reading
float CalculateColdJVoltage(float t);	// converts cold junction temperature to an equvalent thermocouple voltage
void ADC1RTDCfg(void);                // RTD ADC1 settings
void ADC1ThermocoupleCfg(void);       // Tc ADC1 settings
void SendString(void);					// Transmit string using UART
void SendResultToUART(void);			// Send measurement results to UART - in ASCII String format

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
unsigned int uiArraySize = 0;					// size of array to be written to flash
 
float fVolts = 0.0;										// ADC to voltage constant
float fVThermocouple = 0.0;						// thermoucouple voltage
float fVRTD = 0.0 ;										// RTD voltage, 
float fRrtd = 0.0;										// resistance of the RTD
float fColdJVolt = 0.0;								// cold junction equivalent thermocouple voltage
float fFinalVoltage = 0.0;						// fFinalVoltage = thermocouple voltage + cold j voltage
float fTThermocouple = 0.0;					  // thermoucouple temperature
float fTRTD = 0.0;										// RTD temperature
float fFinalTemp = 0.0;								// Final temperature including cold j compensation
unsigned char nLen = 0;								// Used for sending strings to UART
unsigned char i = 0;									// counter used in SendString()
unsigned char ucCounter = 0;

int main (void)
{
	WdtCfg(T3CON_PRE_DIV1,T3CON_IRQ_EN,T3CON_PD_DIS);								// Turn off Watchdog timer
	ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);					// Set CPU clock to 16MHz
	ClkDis(0);                                                                      // Disable clock to unused peripherals
        ClkSel(CLK_CD7,CLK_CD7,CLK_CD0,CLK_CD7);				       // Enable UART clock - disable SPI/I2C/PWM clocks
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
	ADC1INIT();								                                      // Init ADC1
	IEXCINIT();																										  // Init IEXC0 for 200uA on AIN5
	NVIC_EnableIRQ(ADC1_IRQn);					                            // Flash/UART/ADC1 IRQ
	sprintf ( (char*)szTemp, "Program Started. Please wait for the first temperature result\r\n");
	nLen = strlen((char*)szTemp);
	if (nLen <64)
 		SendString();

	fVolts	= (1.2 / 268435456);			// Internal reference	
	while(1)
	{
		delay(0x1FFFFF);
		if(bSendResultToUART == 1)
		{
			fVThermocouple = 0;
			fVRTD =0;
			for (ucCounter = 0; ucCounter < SAMPLENO; ucCounter++)
			{
				fVThermocouple += (ulADC1DATThermocouple[ucCounter] * fVolts);  	// Thermocouple voltage
				fVRTD += ((float)ulADC1DATRtd[ucCounter]  / 268435456);	  		   // RTD voltage	in terms of reference voltage
			}
			fVThermocouple = fVThermocouple/SAMPLENO;					// Get the average of the results
			fVRTD = fVRTD/SAMPLENO;
			fRrtd = fVRTD * 5600;											// RTD resistance
			fTRTD =	CalculateRTDTemp(fRrtd);							// RTD temperature
			fColdJVolt = CalculateColdJVoltage(fTRTD);				// get an equvalent thermocouple voltage
			fFinalVoltage = fVThermocouple + fColdJVolt;
			//fTThermocouple = CalculateThermoCoupleTemp(fVThermocouple);
			fFinalTemp = CalculateThermoCoupleTemp(fFinalVoltage);	// Thermocouple temperature
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
void ADC1RTDCfg(void)
{
	AdcPin(pADI_ADC1,ADCCON_ADCCN_AIN1,ADCCON_ADCCP_AIN0);
	AdcRng(pADI_ADC1,ADCCON_ADCREF_EXTREF,ADCMDE_PGA_G32,ADCCON_ADCCODE_INT);
}
void ADC1ThermocoupleCfg(void)
{
	AdcPin(pADI_ADC1,ADCCON_ADCCN_AIN3,ADCCON_ADCCP_AIN2);          // Select AIn2/AIN3 as ADC inputs
	AdcRng(pADI_ADC1,ADCCON_ADCREF_INTREF,ADCMDE_PGA_G32,ADCCON_ADCCODE_INT); // Internal reference, Gain=32
}
float CalculateRTDTemp(float r) {
	float t;
	int j;
	j=(float)(r-RMIN)/RSEG;       // determine which coefficients to use
	if (j<0)               // if input is under-range..
		j=0;                // ..then use lowest coefficients
	else if (j>NSEG-1)     // if input is over-range..
		j=NSEG-1;            // ..then use highest coefficients
	t = C_rtd[j]+(r-(RMIN+RSEG*j))*(C_rtd[j+1]-C_rtd[j])/RSEG;
	return t;
} 
float CalculateThermoCoupleTemp(float v)
{	
	float fresult = 0;
	signed int j = 0;
	float fMVthermocouple = v*1000;			//thermocouple voltage in mV
	if (fMVthermocouple >= 0)
	{
  		j=(fMVthermocouple - THER_V_MIN_P) / THER_V_SEG_P;			// determine which coefficient to use
  		if (j>THER_N_SEG_P-1)     									// if input is over-range..
    		j=THER_N_SEG_P-1;            							// ..then use highest coefficients
		
		// Use the closest known temperature value and then use a linear approximation beetween the
		// enclosing data points
  		fresult = C_themocoupleP[j] + (fMVthermocouple - (THER_V_MIN_P+THER_V_SEG_P*j) )*
			(C_themocoupleP[j+1]-C_themocoupleP[j]) / THER_V_SEG_P;		 // Slope
	}
	else if (fMVthermocouple < 0)
	{
		j=(fMVthermocouple - THER_V_MIN_N) / THER_V_SEG_N;		// determine which coefficient to use
		if (j>THER_N_SEG_N-1)    								// if input is over-range..
    		j=THER_N_SEG_N-1;          							// ..then use highest coefficients
		fresult = C_themocoupleN[j] + (fMVthermocouple- (THER_V_MIN_N+THER_V_SEG_N*j) )*
			(C_themocoupleN[j+1]-C_themocoupleN[j]) / THER_V_SEG_N;
	} 
   	return  fresult;
}

float CalculateColdJVoltage(float t)
{
	float fresult = 0;
	signed int j = 0;
	if (t >= 0)
	{
  		j=(t - COLDJ_T_MIN_P) / COLDJ_T_SEG_P;				// determine which coefficient to use
  		if (j>COLDJ_N_SEG_P-1)     							// if input is over-range..
    		j=COLDJ_N_SEG_P-1;            					// ..then use highest coefficients
		
		// Use the closest known voltage and then use a linear approximation beetween the
		// enclosing data points
  		fresult = C_cold_junctionP[j] + (t - (COLDJ_T_MIN_P+COLDJ_T_SEG_P*j) )*
			(C_cold_junctionP[j+1]-C_cold_junctionP[j]) / COLDJ_T_SEG_P;		 // Slope
	
	}
	else if (t < 0)
	{
		j=(j - COLDJ_T_MIN_N) / COLDJ_T_SEG_N;				// determine which coefficient to use
		if (j>COLDJ_N_SEG_N-1)    							// if input is over-range..
    		j=COLDJ_N_SEG_N-1;         						// ..then use highest coefficients

		fresult = C_cold_junctionN[j] + (t - (COLDJ_T_MIN_N+COLDJ_T_SEG_N*j) )*
			(C_cold_junctionN[j+1]-C_cold_junctionN[j]) / COLDJ_T_SEG_N;
	}
	return fresult/1000.0;
}

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
void SystemFullCalibration(void)
{
	ucWaitForUart = 1;
	sprintf ( (char*)szTemp, "Set Full Scale Voltage - Press return when ready \r\n");                         
	nLen = strlen((char*)szTemp);
 	if (nLen <64)
		 	SendString();
	while (ucWaitForUart == 1)
	{}
	AdcGo(pADI_ADC0,ADCMDE_ADCMD_SYSGCAL);							// ADC1 System Full scale calibration
	while ((AdcSta(pADI_ADC1) &0x20) != 0x20)			// bit 5 set by adc when calibration is complete
	{}
}
//ADC1 will meansure Thermocouple on AIN2/3 and RTD on AIN0/1
void ADC1INIT(void)
{
	unsigned long *pWrite;
	volatile unsigned char ucEraseSuccess = 0;
	AdcBias(pADI_ADC1,ADCCFG_PINSEL_AIN7,ADC_BIAS_X1,0);	//vbias ain7 buffers on
  AdcBuf(pADI_ADC1,ADCCFG_EXTBUF_VREFPN,ADC_BUF_ON);              //External reference buffers on
	AdcMski(pADI_ADC1,ADCMSKI_RDY,1);						                  	// Enable ADC1 /rdy IRQ
	// Thermocouple settings
	AdcPin(pADI_ADC1,ADCCON_ADCCN_AIN3,ADCCON_ADCCP_AIN2);          // Select AIn2/AIN3 as ADC inputs
	AdcRng(pADI_ADC1,ADCCON_ADCREF_INTREF,ADCMDE_PGA_G32,ADCCON_ADCCODE_INT); // Internal reference, Gain=32
//	ADC1CON = ulADC1CONThermocouple;
	AdcFlt(pADI_ADC1,124,0xD00,FLT_NORMAL);							              // Chop On, 3.75Hz sampling rate, chop on
	AdcGo(pADI_ADC1,ADCMDE_ADCMD_IDLE);                             // Set ADC1 for Idle mode
	delay(0xFFFFF);
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
	delay(0xFFFFF);
	AdcGo(pADI_ADC1,ADCMDE_ADCMD_CONT);							// PGA = 32	 continuous receive mode
}
void UARTInit(void)
{
   UrtCfg(pADI_UART,B9600,COMLCR_WLS_8BITS,0);   // setup baud rate for 9600, 8-bits
   UrtMod(pADI_UART,COMMCR_DTR,0);  			  // Setup modem bits
   UrtIntCfg(pADI_UART,COMIEN_ERBFI|COMIEN_ETBEI|COMIEN_ELSI|COMIEN_EDSSI|COMIEN_EDMAT|COMIEN_EDMAR);  // Setup UART IRQ sources
   DioPul(pADI_GP0,0xFF);								              // Enable pullup on P0.7/0.6
   DioCfg(pADI_GP0,0x3C);								              // Configure P0.2/P0.1 for UART
}

void IEXCINIT(void)
{
	//IexcCfg(IEXCCON_PD_off,IEXCCON_REFSEL_Int,IEXCCON_IPSEL1_Off,IEXCCON_IPSEL0_AIN5);  // AIN5 for 200uA excitation current source - CN221 circuit board
        IexcCfg(IEXCCON_PD_off,IEXCCON_REFSEL_Int,IEXCCON_IPSEL1_Off,IEXCCON_IPSEL0_AIN6);   // ADuCM360MKZ board
	IexcDat(IEXCDAT_IDAT_200uA,IDAT0Dis);					 	
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

    /*sprintf ( (char*)szTemp, "TC Temperature: %fC \r\n",fTThermocouple );// Used for evaluating TC                          
    nLen = strlen((char*)szTemp);
    if (nLen <64)
            SendString();*/ 
    
    sprintf ( (char*)szTemp, "Final Temperature: %fC \r\n\n\n\n",fFinalTemp );// Send the Result to the UART                          
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
{}
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

