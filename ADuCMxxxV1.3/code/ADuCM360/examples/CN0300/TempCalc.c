/*
THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES INC. ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE
DISCLAIMED. IN NO EVENT SHALL ANALOG DEVICES INC. BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

YOU ASSUME ANY AND ALL RISK FROM THE USE OF THIS CODE OR SUPPORT FILE.

IT IS THE RESPONSIBILITY OF THE PERSON INTEGRATING THIS CODE INTO AN APPLICATION
TO ENSURE THAT THE RESULTING APPLICATION PERFORMS AS REQUIRED AND IS SAFE.
Description:
This file contains the functions to convert ADC results to temperature 
based on measurements of the RTD and thermocouple.

*/
#include "TempCalc.h"

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
//Cold junction lookup table. Used for converting cold junction temperature to thermocouple voltage
const float C_cold_junctionP[COLDJ_N_SEG_P+1] = {0.0,	0.2435,	0.4899,	0.7393,	0.9920,	1.2479, 1.5072,	1.7698,
												2.0357,	2.3050,	2.5776,	2.8534,	3.1323,	3.4144,	3.6995, 3.9875,
												4.2785,	4.5723,	4.8689,	5.1683,	5.4703};
const float C_cold_junctionN[COLDJ_N_SEG_N+1] = {0.0,		-0.1543,	-0.3072,	-0.4586,	-0.6085,
												-0.7568,	-0.9036,	-1.0489,	-1.1925,	-1.3345,
												-1.4750};
//RTD lookup table
const float C_rtd[] = {-40.0006,-34.6322,-29.2542,-23.8669,-18.4704,-13.0649,-7.65042,-2.22714,3.20489,8.64565,14.0952,
						19.5536,25.0208,30.497,35.9821,41.4762,46.9794,52.4917,58.0131,63.5436,69.0834,74.6325,80.1909,
						85.7587,91.3359,96.9225,102.519,108.124,113.74,119.365,124.999};
// Calculate RTD temperature
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

// Calculate thermocouple temperature
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

// Convert RTD temperature to its thermocouple equivalent voltage
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

