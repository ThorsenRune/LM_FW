/*file: GlobalData.c
			Datastructures  visible globally to all methods
*/
//doc: https://docs.google.com/document/d/1prddsir9SDUNZOeupz5Ti0Hf6Lz4Rij-P-e_1oIO8eE/edit


#include "GlobalData.h"
#include "merrflags.h"							// Error messages

EX_FLAGS bErrFlags;									// Allocation for the Flags
int cLoopCount[1]={0};


//			Stimulation, gain, offset parameters
int aIAmp[2]={0,0};									// Stimulation level in  micro Ampere
int IMax[2]={0,0};									// Stimulation level higher limit microAmp
int aMLev[2]={0,0};									//Level of EMG   
int Offset[2]={100000,100000};			// Stimulation offset threshold
int Gain[2]={10,10};								// Stimulation level gain
int IMin[2]={0,0};									// Stimulation level lower limit
int IMaxLimit[1]={30000};							// Maximum limit of the current  )

//	Scaling EMG samples: divide CF	  to get uVolt
// Conversion factor: 20000
//	DC offset: 134 000 000


//			Signal Arrays
const int kSignals=2;							// Number of signal channels
int aRMSfilt[kSignals]={0};								// RMS values after the IIR filter <0x02
int aRMSshift[kSignals]={0};				// Obsolete RMS values after the shift at the beginning of the PW linear function
int xData[kADCBuffSize] = {0x0};		// Acquired data IIR HP filtered (first channel)
int xData1[kADCBuffSize] = {0x0};		// Acquired data IIR HP filtered (second channel)
int yData[kADCBuffSize] = {0x0};		// FIR LP filtered data (first channel)
int yData1[kADCBuffSize] = {0x0};		// FIR LP filtered data (first channel)
int nBlankInterval[4]={0,0,0,0};		// Interval for blanking
int diff;


//+			CONTROL DATA
int nTimerInMs[3]={0};							// Milliseconds 1= T0 start of cycle 2=actual ms, 3How many clockcycles are available as resource
int uMode[2]={0};										// Change mode of operation
uint8_t bFlip=0;
//+			Led counters
uint8_t contRed=0;									// Counter for led blinking red
uint8_t contGr=0;										// Counter for led blinking green
int swcont=0;												// Counter for button press duration


//+			Static data
tDMAStruct aADCBuffer[2]={0x0};			// Data acquisition


//			DAC CONTROL DATA
int DacIoffset[1]={154};						// Offset conversion value from mA to 12-bit value (stimulation current)
int DacIGain[1]={9};								// Gain conversion value from mA to 12-bit value (stimulation current
int DacVoffset[1]={-114};						// Offset conversion value from V to 12-bit value (HVPS)
int DacVGain[1]={468114};						// Gain conversion value (to be shifted in the funtion) from V to 12-bit value (HVPS)


//			BLUETOOTH CONTROL VARIABLES
char BTName[2]={"07"};							// Bluetooth name (after "LM-")

int BTsleepvar=0;


//			ADC READ VARIABLES
typedef enum {kAIN0,kAIN2, kAIN5, kAIN3, kvBattery,kAIN8,kAIN9} eAINChannels;
int nVbattVolt;	//Battery voltage scaled to mV

//			FLAG VARIABLES
int flagcont=0;											// Flag to activate the counter for button press duration
int blfl=0;													// Flag to reset blanking intervals to the original values

//			STIMULATION AND TIME COUNTER
unsigned long StimCount[4]={0};						// x=0 - 0>x>10 - 10>x>20 - x>20 [mA]
int StimCountEx[4]={0};
int Min5Count=0;

//			MODES VARIABLE
/* Description: a bitfield controlling various states of the device
								Signals to control: PLS_CTRL, WAKE_SW, CHRGVCCS_DRIVE, VPP_CHARGE,
																		EN_INA_SUPPLY, EN_ST, CMD_MLDP, LED_CTRL,
																		EN_CH0, EN_CH1, EN2_CH0 and EN2_CH1;
								SPI pins are controlled by SPI0INIT();
								UART_RX and UART_TX controlled other functions */

	// Bit related to PLS_CTRL (activates the stimulation in the Final State Machine)

	// Bit related to EN_BLUE_COMM (WAKE_SW, enables the BlueTooth communication)
ModeType nMode;
