/**
 *****************************************************************************
   @example  FlashSign.c
   @brief    This example calculates the CRC key for a range of flash pages
             stores the key at the end of the topmost flash page in the range
             and then starts flash integrity check feature on the part.
             The LED (P1.3) lights up (P1.3 goes low) if the flash integrity check
             returns a failure.

   @version  V0.1
   @author   ADI
   @date     October 2012 
              
All files for ADuCM360/361 provided by ADI, including this file, are
provided  as is without warranty of any kind, either expressed or implied.
The user assumes any and all risk from the use of this code.
It is the responsibility of the person integrating this code into an application
to ensure that the resulting application performs as required and is safe.

**/



#include <stdio.h>
#include <string.h>
#include <aducm360.h>

#include <..\common\AdcLib.h>
#include <..\common\IexcLib.h>
#include <..\common\DacLib.h>
#include <..\common\UrtLib.h>
#include <..\common\ClkLib.h>
#include <..\common\WutLib.h>
#include <..\common\WdtLib.h>
#include <..\common\GptLib.h>
#include <..\common\I2cLib.h>
#include <..\common\IntLib.h>
#include <..\common\PwmLib.h>
#include <..\common\DioLib.h>
#include <..\common\FeeLib.h>


volatile unsigned int uiFEESTA;
volatile unsigned int uiFEEADRAL;
volatile unsigned int uiFEESIG;


void delay(long int);
long int GenerateChecksumCRC24_D32(unsigned long startAddress,unsigned long endAddress,int iBigEndian);
static unsigned long CRC24_D32(const unsigned long old_CRC, const unsigned long Data);
void SwapEndianess(unsigned char * pbData,int iCount,int iObjectSize);

unsigned long ulSignUserFlash = 0;
volatile unsigned long * signatureAddr;

int main (void)
{
   WdtGo(T3CON_ENABLE_DIS);
   DioOenPin(pADI_GP1, PIN3, 1);                        // Set P1.3 as an output for test purposes
   DioSet(pADI_GP1, BIT3);
   WdtCfg(T3CON_PRE_DIV1,T3CON_IRQ_EN,T3CON_PD_DIS); // Disable Watchdog timer resets
   //Disable clock to unused peripherals
   ClkDis(CLKDIS_DISSPI0CLK|CLKDIS_DISSPI1CLK|CLKDIS_DISI2CCLK|CLKDIS_DISPWMCLK|CLKDIS_DIST0CLK|CLKDIS_DIST1CLK|CLKDIS_DISDMACLK|CLKDIS_DISUARTCLK);
   ClkCfg(CLK_CD0,CLK_HF,CLKSYSDIV_DIV2EN_DIS,CLK_UCLKCG);            // Select CD0 for CPU clock
  
   delay(100000);

   ulSignUserFlash = GenerateChecksumCRC24_D32(0x001ff, 0x2000, 0);  //This function gerates checksums for integer number of
                                    //pages because that's how the hardware does it when checking the checksum
                                    //if this case it generates a key for all the words up to 0x21fc (0x21fc isn't used as that's where the
                                    //crc key should be stored)

   FeeWrEn(1);
   signatureAddr = (volatile unsigned long *)0x21fc;
   *signatureAddr = ulSignUserFlash;      //store the crc key at the top of the last page verified
//    *signatureAddr = 0x123456;             //a random key can be stored instead to show that the LED does indeed light up
                                          //if a failure is detected
   FeeWrEn(0);
   uiFEESTA = FeeSta();
   while((uiFEESTA & FEESTA_CMDBUSY) == FEESTA_CMDBUSY)
      {uiFEESTA = FeeSta();}                 //wait for the write to finish

   FeeSign(0x01ff, 0x2000);            //
   uiFEESTA = FeeSta();
   if ((uiFEESTA & FEESTA_CMDRES_VERIFYERR) == FEESTA_CMDRES_VERIFYERR) {
      DioClr(pADI_GP1, BIT3);             //Turn Led on if there is a flash integrity problem
   }
   while((uiFEESTA & FEESTA_CMDBUSY) == FEESTA_CMDBUSY)
   {
      uiFEESTA = FeeSta();
      if ((uiFEESTA & FEESTA_CMDRES_VERIFYERR) == FEESTA_CMDRES_VERIFYERR) {
      DioClr(pADI_GP1, BIT3);             //Turn Led on
      }
   }

   uiFEESIG = FeeSig();  // this is the key calculated by hardware. It will be the same as ulSignUserFlash if there is no corruption

   while (1)
   {
      delay(2000000);
   }
}



// Function    : GenerateChecksumCRC24_D32
// Description : Generate checksum 
// This function returns the CRC result for the flash pages included by the startAddress and endAddress.
// If startAddress is not at the start of a page, all the addresses before it belonging to the same page are
// used for the CRC calculation. Similary all the addres after endAddress belocngfing to the same flash page as
// endAddress are used for the CRC calculcation

long int GenerateChecksumCRC24_D32(unsigned long startAddress,unsigned long endAddress,int iBigEndian)
{
   unsigned long i,ulData,lfsr = 0xFFFFFF;
   unsigned long ulNumValues;
   unsigned long numberOfPages = (endAddress>>9) - (startAddress>>9) + 1;
   unsigned long *pulData = (unsigned long *) (startAddress>>9);
   ulNumValues = (numberOfPages*0x200)/4; //number of words
   ulNumValues -= 1;  //subtract 1 because the last word will be used for storing the CRC and therefore should not be used in the calculation.
   
   for (i= 0x0; i < ulNumValues;i++)
      {
      ulData = pulData[i];
      if(iBigEndian)
         SwapEndianess((unsigned char *)&ulData,     1,sizeof(unsigned long));
      lfsr = CRC24_D32(lfsr,ulData);
      }

   if(iBigEndian)
      SwapEndianess((unsigned char *)&lfsr,     1,sizeof(unsigned long));

   return lfsr;
}

static unsigned long CRC24_D32(const unsigned long old_CRC, const unsigned long Data)
{
   unsigned long D      [32];
   unsigned long C      [24];
   unsigned long NewCRC [24];
   unsigned long ulCRC24_D32;

   volatile unsigned long int f, tmp;
   unsigned long int bit_mask = 0x000001;

   tmp = 0x000000;
   // Convert previous CRC value to binary.
   bit_mask = 0x000001;
   for (f = 0; f <= 23; f++)
      {
      C[f]   = (old_CRC & bit_mask) >> f;
      bit_mask          = bit_mask << 1;
      }

   // Convert data to binary.
   bit_mask = 0x000001;
   for (f = 0; f <= 31; f++)
      {
      D[f]   = (Data & bit_mask) >> f;
      bit_mask       = bit_mask << 1;
      }

   // ^ = XOR
   // Calculate new LFSR value.
   // there's probably a smarter way of doing this but at least
   // this should be easier to update if necessary.
   NewCRC[0] = D[31] ^ D[30] ^ D[29] ^ D[28] ^ D[27] ^ D[26] ^ D[25] ^ 
               D[24] ^ D[23] ^ D[17] ^ D[16] ^ D[15] ^ D[14] ^ D[13] ^ 
               D[12] ^ D[11] ^ D[10] ^ D[9] ^ D[8] ^ D[7] ^ D[6] ^ 
               D[5] ^ D[4] ^ D[3] ^ D[2] ^ D[1] ^ D[0] ^ C[0] ^ C[1] ^ 
               C[2] ^ C[3] ^ C[4] ^ C[5] ^ C[6] ^ C[7] ^ C[8] ^ C[9] ^ 
               C[15] ^ C[16] ^ C[17] ^ C[18] ^ C[19] ^ C[20] ^ C[21] ^ 
               C[22] ^ C[23];
   NewCRC[1] = D[23] ^ D[18] ^ D[0] ^ C[10] ^ C[15];
   NewCRC[2] = D[24] ^ D[19] ^ D[1] ^ C[11] ^ C[16];
   NewCRC[3] = D[25] ^ D[20] ^ D[2] ^ C[12] ^ C[17];
   NewCRC[4] = D[26] ^ D[21] ^ D[3] ^ C[13] ^ C[18];
   NewCRC[5] = D[31] ^ D[30] ^ D[29] ^ D[28] ^ D[26] ^ D[25] ^ D[24] ^ 
               D[23] ^ D[22] ^ D[17] ^ D[16] ^ D[15] ^ D[14] ^ D[13] ^ 
               D[12] ^ D[11] ^ D[10] ^ D[9] ^ D[8] ^ D[7] ^ D[6] ^ 
               D[5] ^ D[3] ^ D[2] ^ D[1] ^ D[0] ^ C[0] ^ C[1] ^ C[2] ^ 
               C[3] ^ C[4] ^ C[5] ^ C[6] ^ C[7] ^ C[8] ^ C[9] ^ C[14] ^ 
               C[15] ^ C[16] ^ C[17] ^ C[18] ^ C[20] ^ C[21] ^ C[22] ^ 
               C[23];
   NewCRC[6] = D[28] ^ D[18] ^ D[5] ^ D[0] ^ C[10] ^ C[20];
   NewCRC[7] = D[29] ^ D[19] ^ D[6] ^ D[1] ^ C[11] ^ C[21];
   NewCRC[8] = D[30] ^ D[20] ^ D[7] ^ D[2] ^ C[12] ^ C[22];
   NewCRC[9] = D[31] ^ D[21] ^ D[8] ^ D[3] ^ C[0] ^ C[13] ^ C[23];
   NewCRC[10] = D[22] ^ D[9] ^ D[4] ^ C[1] ^ C[14];
   NewCRC[11] = D[23] ^ D[10] ^ D[5] ^ C[2] ^ C[15];
   NewCRC[12] = D[24] ^ D[11] ^ D[6] ^ C[3] ^ C[16];
   NewCRC[13] = D[25] ^ D[12] ^ D[7] ^ C[4] ^ C[17];
   NewCRC[14] = D[26] ^ D[13] ^ D[8] ^ C[0] ^ C[5] ^ C[18];
   NewCRC[15] = D[27] ^ D[14] ^ D[9] ^ C[1] ^ C[6] ^ C[19];
   NewCRC[16] = D[28] ^ D[15] ^ D[10] ^ C[2] ^ C[7] ^ C[20];
   NewCRC[17] = D[29] ^ D[16] ^ D[11] ^ C[3] ^ C[8] ^ C[21];
   NewCRC[18] = D[30] ^ D[17] ^ D[12] ^ C[4] ^ C[9] ^ C[22];
   NewCRC[19] = D[31] ^ D[18] ^ D[13] ^ C[5] ^ C[10] ^ C[23];
   NewCRC[20] = D[19] ^ D[14] ^ C[6] ^ C[11];
   NewCRC[21] = D[20] ^ D[15] ^ C[7] ^ C[12];
   NewCRC[22] = D[21] ^ D[16] ^ C[8] ^ C[13];
   NewCRC[23] = D[31] ^ D[30] ^ D[29] ^ D[28] ^ D[27] ^ D[26] ^ D[25] ^ 
                D[24] ^ D[23] ^ D[22] ^ D[16] ^ D[15] ^ D[14] ^ D[13] ^ 
                D[12] ^ D[11] ^ D[10] ^ D[9] ^ D[8] ^ D[7] ^ D[6] ^ 
                D[5] ^ D[4] ^ D[3] ^ D[2] ^ D[1] ^ D[0] ^ C[0] ^ C[1] ^ 
                C[2] ^ C[3] ^ C[4] ^ C[5] ^ C[6] ^ C[7] ^ C[8] ^ C[14] ^ 
                C[15] ^ C[16] ^ C[17] ^ C[18] ^ C[19] ^ C[20] ^ C[21] ^ 
                C[22] ^ C[23];




   ulCRC24_D32 = 0;
   // LFSR value from binary to hex.
   bit_mask = 0x000001;
   for (f = 0; f <= 23; f++)
      {
      ulCRC24_D32 = ulCRC24_D32 + NewCRC[f] * bit_mask;
      bit_mask = bit_mask << 1;
      }
   return(ulCRC24_D32 & 0x00FFFFFF);
}

void SwapEndianess(unsigned char * pbData,int iCount,int iObjectSize)
{
   unsigned char     ucData          = 0x0;
   unsigned char *   pbCurObject     = pbData;
   int               iByteLoop       = 0x0;
   int               iLoopEnd        = 0x0;
   int               iHiByteIndex    = 0x0;

   // This routine will only work for suitable object sizes
   if (iObjectSize %2)
      {
      switch (iObjectSize)
         {
         case 0:
         case 1:
            // These are ok, don't need to be switched
            break;
         default:
            // Not a valid object size
            break;
         }
      return;
      }

   while (iCount)  // For each objects
      {
      iLoopEnd = (iObjectSize/2);

      for (iByteLoop = 0x0;iByteLoop < iLoopEnd;iByteLoop++)
         {
         iHiByteIndex               = iObjectSize - 0x1 - iByteLoop;
         ucData                     = pbCurObject[iHiByteIndex];
         pbCurObject[iHiByteIndex]  = pbCurObject[iByteLoop];
         pbCurObject[iByteLoop]     = ucData;
         }
      pbCurObject += iObjectSize;
       iCount --;
      }
}

// Simple Delay routine
void delay (long int length)
{
   while (length >0)
      length--;
}
void Ext_Int2_Handler ()
{           

}

void Ext_Int1_Handler ()
{           

}
void Ext_Int4_Handler ()
{         
 
}
void GP_Tmr0_Int_Handler(void)
{

}

void GP_Tmr1_Int_Handler(void)
{
 
}
void ADC0_Int_Handler()
{
   
}
void ADC1_Int_Handler ()
{
   
}
void SINC2_Int_Handler()
{
 
}
void UART_Int_Handler ()
{

} 

void PWMTRIP_Int_Handler ()
{

}
void PWM0_Int_Handler()
{

}
void PWM1_Int_Handler ()
{
 
}
void PWM2_Int_Handler()
{
  
}





