// RT Error flags (sticky)
#ifndef ___HERRORFLAGS
#define ___HERRORFLAGS


#include <stdint.h>
typedef union
{
 int     all_flags[1];      /* Allows us to refer to the flags 'en masse' */
 struct
 {
  uint32_t
		bFifoOverflow : 1,     /* overflow in mFIFO_push1 */
		bTXOverflow : 1,     /* overflow */
		bTX32Overflow : 1,     /* overflow */
		bRXOverflow : 1,     /*Buffer overflow Error in receiving data,  */
		Overflow_mVectorDotProduct : 1,     /*  mVectorDotProduct caused a near overflow*/
		bReceiveError : 1,     /* Error in rx dispatcher */
		b12BitUnderflow : 1,     /* underflow in DAC */
		b12BitOverflow: 1,     	/* overflow in DAC */
		ADCReadChError : 1,     /* Channel error during read from AD5592 ADC  */
		INA0LODPos : 1,     /* Lead off detection value over limit, problem with positive electrode in INA0 */
		INA0LODNeg : 1,     /* Lead off detection value over limit, problem with negative electrode in INA0 */
		INA1LODPos : 1,     /* Lead off detection value over limit, problem with positive electrode in INA1 */
		INA1LODNeg : 1,     /* Lead off detection value over limit, problem with negative electrode in INA1 */
		BattLow : 1,     /* Battery is low */
		TxBufferFull : 1,     /* unsuccesful write */
		spare16 : 1,     /* Unused */
		spare15 : 1,     /* Unused */
		spare14 : 1,     /* Unused */
		spare13 : 1,     /* Unused */
		spare12 : 1,     /* Unused */
		spare11 : 1,     /* Unused */
		spare10 : 1,     /* Unused */
		spare9 : 1,     /* Unused */
		spare8 : 1,     /* Unused */
		spare7 : 1,     /* Unused */
		spare6 : 1,     /* Unused */
		spare5 : 1,     /* Unused */
		spare4 : 1,     /* Unused */
		spare3 : 1,     /* Unused */
		spare2 : 1,     /* Unused */
		spare1 : 1,     /* Unused */
		spare0 : 1;     /* Unused */
	} errbits;
} EX_FLAGS;


extern EX_FLAGS bErrFlags;  /* Allocation for the Flags */


#endif	// ___HERRORFLAGS
