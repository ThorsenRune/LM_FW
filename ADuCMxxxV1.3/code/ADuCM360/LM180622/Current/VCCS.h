/* Low level calls to the VCCS circuit
// File: VCCS.h
 doc:	https://docs.google.com/document/d/19ir0kVwW2ZwKVjgPJRDLKKwYKq_6ZqZqTvQn0kgXl_8/edit
// #include "VCCS.h"
*/

#ifndef __VCCS161028__
#define __VCCS161028__



// Select register to set value of DAC pin out use for nPin in setDAC_OutputValue(123,kDACValueOfPin0 )


static const uint32_t kHVPSVoltage = 0x8000;
static const uint32_t kStimLevel = 0xC000; 

typedef enum {kFSMState0,
							kFSMState1, kFSMState2, kStimPhase3, kStimPhase4, kStimPhase5
							,kStimChangeChannels, kStimPhase6, kStimPhase7, kStimPhase8, 
							kStimPhase9, kStimPhase10, kFSMState11, ADC_CH1, ADC_CH2, ADC_CH3, CHRGVCCS_DIS} state; 
              // type that indicates the differents states of the Final State Machine
              // used for the stimulation

typedef enum {kChIO2_Add=0x2000, kChIO5_Add=0x5000, kChIO6_Add=0x6000} ADC_Ch_Add; 
																								//Channel address to check in ADC AD5592 Read FSM
int aADC5592Values[3];		//Array of ADC values from the input channels on ADC5592
typedef enum {VPP_SENSE_CH=0, VCCS_PEAK_SENS_CH=1, VCCSSENS_CH=2} ADC_Ch_Read;

static const int kADCSeqReg=0x1264;     //to send via SPI to AD5592 to set the right continous ADC channel sequence


 
typedef enum {kindext0, kindext1, kindext2,kindext3,kindext4,kindext5,kindext6,kindext7,
							kindext8,kindext9,kindext10,kindext11,kindext12,kindext13,kindext14} TimerIndex;
																													// Index for timer reload in every state
int StimNegDur[2]={300,300};		//OBSOLETE of 180928


extern void mSetMUX(int nMUXState);


#endif  // __VCCS161028__
