 void AD5421INIT(void);								                // Init. ADuCM360 to AD5421 interface.
 void AD5421_WriteToDAC(unsigned long ulDACValue);	   	// Write to IDAC data register
 void AD5421_WriteToCon(unsigned long ulConValue);	  	// Write to IDAC Control register
 void AD5421_WriteToOffAdj(unsigned long ulOffAdjValue);// Write to IDAC Offset adjust register
 void AD5421_WriteToGnAdj(unsigned long ulDACValue);	  // Write to IDAC Gain adjust register
 void AD5421_LoadDac(void);								              // Load the IDAC output
 void AD5421_ForceAlarm(void);							            // force Alarm condition on IDAC output
 void AD5421_Reset(void);								                // Reset AD5421
 void AD5421_InitADC(void);								              // Measure Vloop or die temp via ADC
 unsigned long AD5421_ReadDAC(unsigned long ulDACValue);// Read to IDAC data register
 unsigned long  AD5421_ReadCon(unsigned long ulConValue);// Read IDAC Control register
 void AD5421_ReadOffAdj(unsigned long ulOffAdjValue);	  // Read IDAC Offset adjust register
 void AD5421_ReadGnAdj(unsigned long ulDACValue);		    // Read IDAC Gain adjust register
 unsigned long  AD5421_ReadFault(unsigned long ulDACValue);// Read Fault register

 #define WRITEDAC 			1
 #define WRITECON 			2
 #define WRITEOFFADJ		3
 #define WRITEGNADJ 		4
 #define LOADDAC 			5
 #define FORCEALARM 		6
 #define AD5421RESET 		7
 #define INITADC 			8
 #define READDAC 			0x81
 #define READCON 			0x82
 #define READOFFADJ			0x83
 #define READGNADJ 			0x84
 #define READFAULT 			0x85

 static unsigned long ul5421CON = 0;
 static unsigned long ul5421DAT = 0;
 static unsigned long ul5421FAULT = 0;
