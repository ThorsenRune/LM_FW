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


*/
#include <ADuCM360.h>
#include <stdio.h>
#include "FlashEraseWrite.h"

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

// Function to Mass erase all of Flash memory
unsigned char MassEraseFlash(void)
{
   unsigned int uiPollFEESTA = 0;

   pADI_FEE->FEEKEY =  0xF456;						// Enter User key to allow access to flash
   pADI_FEE->FEEKEY =  0xF123;

   pADI_FEE->FEECMD = 0x3;						    // Select Mass erase command
	
	while (((uiPollFEESTA & 0x4) == 0x0) &&	// Wait for Command to complete
		  ((uiPollFEESTA & 0x30) != 0x30))	// Also check for an abort
	{
		uiPollFEESTA = pADI_FEE->FEESTA;
	}
	return 1;	
}
void WriteToFlash(unsigned long *pArray, unsigned long ulFlashPage, unsigned int uiSize)
{
 	unsigned int uiPollFEESTA = 0;
	volatile unsigned long *ulStartAddress;
	unsigned int Size = 0;
	unsigned int n = 0;
	
	Size = uiSize;
	ulStartAddress = ( unsigned long      *)ulFlashPage;
	pADI_FEE->FEECON0 |= 0x4;								// Enable a write to Flash memory
	for (n = 0; n < Size; n = n+4)
	{ 
		uiPollFEESTA = 0;
		*ulStartAddress  = *pArray;				// Value from ram copied to flash at address ulStartAddress
		while ((uiPollFEESTA & 0x8) == 0x0)		// Wait for Command to complete
		{
			uiPollFEESTA = pADI_FEE->FEESTA;
		}
		ulStartAddress = ulStartAddress++;
		pArray++;
	}  
	pADI_FEE->FEECON0 &= 0xFB;							// disable a write to Flash memory
}
