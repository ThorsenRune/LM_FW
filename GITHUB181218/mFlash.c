// FUNCTION TO WRITE, READ AND ERASE FLASH MEMORY
// Doc: https://docs.google.com/document/d/1vi5VWuJWhKs0uoJ-eCrviuRaaS-kO06dCVKZh9Rffxw/edit

#include "mFlash.h"
//			Data on flash memory
unsigned long anFLASHData[13];	// Parameters read/write from flash memory

void WriteToFlash(unsigned long *pArray, unsigned long ulStartAddress, unsigned int uiSize)
{
   unsigned int uiPollFEESTA = 0;
   volatile unsigned long *flashAddress;
   unsigned int i = 0;
   
   flashAddress = ( unsigned long      *)ulStartAddress;
   FeeWrEn(1);
   uiPollFEESTA = FeeSta();						// Read Status to ensure it is clear
   for (i = 0; i < uiSize; i = i+4)
   { 
      uiPollFEESTA = 0;
      *flashAddress++  = *pArray++;
      do
         {uiPollFEESTA = FeeSta();}
       while((uiPollFEESTA & FEESTA_CMDBUSY) == FEESTA_CMDBUSY);
   }  
   FeeWrEn(0);       // disable a write to Flash memory
}


void ReadFromFlash(unsigned long *pArray, unsigned long ulStartAddress, unsigned int uiSize)
{
   volatile unsigned long *flashAddress;
   unsigned int i = 0;

   flashAddress = ( unsigned long      *)ulStartAddress;
   for (i = 0; i < uiSize; i = i+4)
   {
      *pArray++ = *flashAddress++;
   }
}


// Function to Erase Flash page defined by ulSelPage
unsigned char ErasePage(unsigned long ulSelPage)
{
 	volatile unsigned int uiPollFEESTA = 0;
	unsigned long ulFEEADR0L = 0;
	
   ulFEEADR0L = ulSelPage;					
   pADI_FEE->FEEKEY =  0xF456;					// Enter User key to allow access to flash
   pADI_FEE->FEEKEY =  0xF123;
   
   uiPollFEESTA = pADI_FEE->FEESTA;					// Read Status to ensure it is clear
   uiPollFEESTA = 0;
   pADI_FEE->FEEADR0L = ulFEEADR0L & 0xFFFF;			// Load FEEADR0L with lower 16 bits [15:0] of Flash page
   pADI_FEE->FEEADR0H = ulFEEADR0L >> 16;				// Load FEEADR0H with bit [16] of Flash page

   pADI_FEE->FEECMD = 0x1;							// Select Page erase command
//   FlashCmdAbort();
/*	Uncomment if using Polling instead of Flash interrupts
	while (((uiPollFEESTA & 0x4) == 0x0) &&		// Wait for Command to complete
    	  ((uiPollFEESTA & 0x30) != 0x30))		// Also check for an abort
	{
		uiPollFEESTA = FEESTA;
	}	*/
	return 1;	
}

void SaveParametersOnFlash(void){			//Save data to FLASH memory
	volatile int v;
//	mWaitMs(500);
	__disable_irq(); 
	v=ErasePage(0x8000);
	__enable_irq();
//	mWaitMs(500);
	anFLASHData[0]=(unsigned long) Offset[0];
	anFLASHData[1]=(unsigned long) Offset[1];
	anFLASHData[2]=(unsigned long) IMax[0];
	anFLASHData[3]=(unsigned long) IMax[1];
	anFLASHData[4]=(unsigned long) Gain[0];
	anFLASHData[5]=(unsigned long) Gain[1];
	anFLASHData[6]=(unsigned long) IMin[0];
	anFLASHData[7]=(unsigned long) IMin[1];
	anFLASHData[8]=(unsigned long) nBlankInterval[0];
	anFLASHData[9]=(unsigned long) nBlankInterval[1];
	anFLASHData[10]=(unsigned long) nBlankInterval[2];
	anFLASHData[11]=(unsigned long) nBlankInterval[3];
	anFLASHData[12]=(unsigned long) nMode.all_flags[0];
	mWaitMs(100);
	__disable_irq(); 
	WriteToFlash(anFLASHData, 0x8000, sizeof(anFLASHData));
	__enable_irq();
	mWaitMs(100);
}



void ReadSavedParameters(void){
	__disable_irq(); 
	ReadFromFlash(anFLASHData, 0x8000, sizeof(anFLASHData));
	__enable_irq();
	mWaitMs(500);
	Offset[0]=(int) anFLASHData[0];
	Offset[1]=(int) anFLASHData[1];
	IMax[0]=(int) anFLASHData[2];
	IMax[1]=(int) anFLASHData[3];
	Gain[0]=(int) anFLASHData[4];
	Gain[1]=(int) anFLASHData[5];
	IMin[0]=(int) anFLASHData[6];
	IMin[1]=(int) anFLASHData[7];
	nBlankInterval[0]=(int) anFLASHData[8];
	nBlankInterval[1]=(int) anFLASHData[9];
	nBlankInterval[2]=(int) anFLASHData[10];
	nBlankInterval[3]=(int) anFLASHData[11];
	nMode.all_flags[0] =(int) anFLASHData[12] ;
	IMaxLimit[0]=30000;					//Safety limit for stimulation in uA
	if (IMax[0]<0) nMode.bits.INVALIDSETUP=1;
	if (IMax[1]<0) nMode.bits.INVALIDSETUP=1;
	__disable_irq();	
	ReadFromFlash(StimCount, 0xA000, sizeof(StimCount));	
	__enable_irq();
}
