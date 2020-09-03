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

//RTD constants
#define TMIN (-40)  		// = minimum temperature in degC
#define TMAX (125)  		// = maximum temperature in degC
#define RMIN (84.2707)  // = input resistance in ohms at -40 degC
#define RMAX (147.951)  // = input resistance in ohms at 125 degC
#define NSEG 30  			// = number of sections in table
#define RSEG 2.12269  	// = (RMAX-RMIN)/NSEG = resistance  in ohms of each segment


float CalculateRTDTemp(float r);			// returns RTD Temperature reading
float CalculateThermoCoupleTemp(float v);		// returns Thermocouple Temperature reading
float CalculateColdJVoltage(float t);	// converts cold junction temperature to an equvalent thermocouple voltage

