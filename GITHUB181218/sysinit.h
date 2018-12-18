/*	HEADERFILE			*/
//file: SysInit.h
//doc:	https://docs.google.com/document/d/1qKgzBgnC24Gj2rvJE5g9NClKeTgpNj85Rmouxsw-nS8/edit
#ifndef __SysInit__
#define __SysInit__
static const int kSetDAC= 0x2811;			// SetDAC=0|0101|000|00010001		IO0 and IO4 set as DAC output
static const int kSetADC= 0x2064;			// SetADC=0|0100|000|01100100		IO2, IO5 and IO6 set as ADC input 
static const int kResetAD=0x7DAC;			// 0|1111|101|10101100 					Reset AD5592
static const int kEnVrefAD=0x5AEE; 		// 0|1011|0|1|0|11101110				PD mode

unsigned char ucTxComplete = 0;				// Flag used to indicate SPI transfer complete

void UART_Init(void);									// initialize UART

//HARDWARE DEFINES of ADC pin connections
#define ADCCON_ADCCP_vBattery   ADCCON_ADCCP_AIN11
#define ADCCON_ADCCP_vLOD1P 		ADCCON_ADCCP_AIN2
#define ADCCON_ADCCP_vLOD1N 		ADCCON_ADCCP_AIN3
#define ADCCON_ADCCP_vINA_OUT1	ADCCON_ADCCP_AIN4
#define ADCCON_ADCCP_vINA_OUT2	ADCCON_ADCCP_AIN10
#define ADCCON_ADCCP_vLOD2N 		ADCCON_ADCCP_AIN8
#define ADCCON_ADCCP_vLOD2P 		ADCCON_ADCCP_AIN9
#define ADCCON_ADCCP_VCCS_PEAK_SENS 			 		ADCCON_ADCCP_AIN0


#endif  // __SysInit__
