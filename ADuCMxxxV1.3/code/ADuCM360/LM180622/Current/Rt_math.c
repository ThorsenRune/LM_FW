#include "mErrFlags.h"			// Error messages
/**
	* @brief 32-bit fractional data type in 1.31 format.
   */
  typedef int32_t q31_t;

  /**
   * @brief 64-bit fractional data type in 1.63 format.
   */
  typedef int64_t q63_t;




__inline int mIIR_1Tap(int x, int y,int alfa){		// both in q31 format a31=decimal*q31One
	// Hand optimized simple filter
	// to produce alfa coefficient use 
	//  alfa=(int)({fractional value} *q31One);	//q31 format
	//q31*q31=q62 leftshift 31 bits to discard signbit and lower bits
		//Z=x*alfa+(1-alfa)*y
		// Reducing form to one multiplication only
		return ((((int64_t)(x-y)*(int64_t)alfa)>>31)+y);
}


/*
Implementation of a 64 bits squareroot
(Rev.140606/RT)


*/
 
__asm uint32_t 	mSqrt64_sub(uint64_t op)	//64 bit squareroot
{
	// r0(ax2 lo), r1(ax1 hi) are input values
	// r2=ay1							current guess
	// r5, r6 is accumulator and return value
	PUSH     {r4-r8,lr}
		MOV  r2, #0  //ay1 = 0;  best match
//	  MOV  r3, #0  //ay2 = 0;  next guess
//		MOV  r4, #0x80000000  //  k=R4  loop counter and bit mask
	//(Rev.140606) Tested and produces valid result in less than 366 clk = 23us @16Mh cortex M3
//Unrolled loop
//0x8	
				ADD  r3,r2,#0x80000000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???

				//---	
				ADD  r3,r2,#0x40000000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
 
				ADD  r3,r2,#0x20000000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
 
				ADD  r3,r2,#0x10000000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
//0x4
				ADD  r3,r2,#0x08000000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???

				//---	
				ADD  r3,r2,#0x04000000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
 
				ADD  r3,r2,#0x02000000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
 
				ADD  r3,r2,#0x01000000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
//0x2
				ADD  r3,r2,#0x00800000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???

				ADD  r3,r2,#0x00400000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
 
				ADD  r3,r2,#0x00200000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
 
				ADD  r3,r2,#0x00100000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
//4
				ADD  r3,r2,#0x00080000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???

				ADD  r3,r2,#0x00040000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
 
				ADD  r3,r2,#0x00020000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
 
				ADD  r3,r2,#0x00010000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
//5
				ADD  r3,r2,#0x0008000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???

				ADD  r3,r2,#0x0004000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
 
				ADD  r3,r2,#0x0002000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
 
				ADD  r3,r2,#0x00001000 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???				
//6
				ADD  r3,r2,#0x00000800 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???

				ADD  r3,r2,#0x00000400 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
 
				ADD  r3,r2,#0x00000200 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
 
				ADD  r3,r2,#0x00000100 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???				
//7
				ADD  r3,r2,#0x0000080 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???

				ADD  r3,r2,#0x0000040 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
 
				ADD  r3,r2,#0x0000020 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
																				//	compare lo bytes
				MOVCS r2,r3					//2 cycles ???
 
				ADD  r3,r2,#0x0000010 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
	 			CMPEQ r0,r5											//if hibytes are the same 
																				//	compare lo bytes
				MOVCS r2,r3					//2 cycles ???				
//8
				ADD  r3,r2,#0x0000008 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???

				ADD  r3,r2,#0x0000004 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
 
				ADD  r3,r2,#0x0000002 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???
 
				ADD  r3,r2,#0x00000001 				//y2(r3)=r2+k
	 			umull r5, r6, r3, r3 		 //sqr=y2^2
	 			CMP r1,r6												//ax(hi)-y2(hi)>0
	 			CMPEQ r0,r5											//if hibytes are the same 
				MOVCS r2,r3					//2 cycles ???													


	
//-------END OF UNROLL	
	mov     r0, r2           // Return values R0=ay1   
  POP     {r4-r8,PC}
	bx lr
}		
__inline uint32_t 	mSqrt64(int64_t op){			//Wrap of square root of 64 bit number
	if (op<0) return INT32_MIN;
	return mSqrt64_sub(op);

}

//*******************   MODIFIED DOT PRODUCT  *************************
/* The DOT product is having a poor resolution loosing 8 bits
		(Rev.140609RT)
		
		----------------------------------------------------------------------
* Copyright (C) 2010 ARM Limited. All rights reserved.
*
* $Date: 15. February 2012
* $Revision: V1.1.0
*
* Project: CMSIS DSP Library
* Title: arm_dot_prod_q31.c
*
* Description: Q31 dot product.
*
* Target Processor: Cortex-M4/Cortex-M3/Cortex-M0
*
* Version 1.1.0 2012/02/15
* Updated with more optimizations, bug fixes and minor API changes.
*
* Version 1.0.10 2011/7/15
* Big Endian support added and Merged M0 and M3/M4 Source code.
*
* Version 1.0.3 2010/11/29
* Re-organized the CMSIS folders and updated documentation.
*
* Version 1.0.2 2010/11/11
* Documentation updated.
*
* Version 1.0.1 2010/10/05
* Production release and review comments incorporated.
*
* Version 1.0.0 2010/09/20
* Production release and review comments incorporated.
*
* Version 0.0.7 2010/06/10
* Misra-C changes done
* -------------------------------------------------------------------- */
//#include "arm_math.h"
/**
* @ingroup groupMath
*/
/**
* @addtogroup dot_prod
* @{
*/
/**
* @brief Dot product of Q31 vectors.
* @param[in] *pSrcA points to the first input vector
* @param[in] *pSrcB points to the second input vector
* @param[in] blockSize number of samples in each vector
* @param[out] *result output result returned here
* @return none.
*
* <b>Scaling and Overflow Behavior:</b>
* \par
* The intermediate multiplications are in 1.31 x 1.31 = 2.62 format and these
* are truncated to 2.48 format by discarding the lower 14 bits.
* The 2.48 result is then added without saturation to a 64-bit accumulator in 16.48 format.
* There are 15 guard bits in the accumulator and there is no risk of overflow as long as
* the length of the vectors is less than 2^16 elements.
* The return result is in 16.48 format.
*/

q63_t mVectorDotProduct(q31_t * pSrcA,q31_t * pSrcB,uint32_t blockSize){
/*		the q31 dot product of two vectors with a q62 result. If intermediate result has more than
		SAFEBITRANGE+32 bits the overflow flag is set and tha WINT_MIN is returned
	*/	
	
	
#define SAFEBITRANGE 29				// Number of significant bits  before alerting overflow
#define SUM_MIN -(1<<SAFEBITRANGE)
#define SUM_MAX (1<<SAFEBITRANGE)
int32_t tmp;						// Local temporary data will probably be using a coreregister
q63_t sum = 0; /* Temporary result storage */
uint32_t blkCnt; /* loop counter */
#ifndef ARM_MATH_CM0
/* Run the below code for Cortex-M4 and Cortex-M3 */
q31_t inA1, inA2, inA3, inA4;
q31_t inB1, inB2, inB3, inB4;
/*loop Unrolling */
blkCnt = blockSize >> 2u;
/* First part of the processing with loop unrolling. Compute 4 outputs at a time.
** a second loop below computes the remaining 1 to 3 samples. */
while(blkCnt > 0u)
{
/* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
/* Calculate dot product and then store the result in a temporary buffer. */
inA1 = *pSrcA++;
inA2 = *pSrcA++;
inA3 = *pSrcA++;
inA4 = *pSrcA++;
inB1 = *pSrcB++;
inB2 = *pSrcB++;
inB3 = *pSrcB++;
inB4 = *pSrcB++;
/*sum += ((q63_t) inA1 * inB1) >> 14u;
sum += ((q63_t) inA2 * inB2) >> 14u;
sum += ((q63_t) inA3 * inB3) >> 14u;
sum += ((q63_t) inA4 * inB4) >> 14u;*/
sum += ((q63_t) inA1 * inB1) ;
sum += ((q63_t) inA2 * inB2) ;
sum += ((q63_t) inA3 * inB3) ;
sum += ((q63_t) inA4 * inB4) ;
	tmp=(int32_t)(sum>>32);			// Hi 32bit
if ((tmp<SUM_MIN)||(tmp>SUM_MAX)){
		bErrFlags.bits.Overflow_mVectorDotProduct=1;
																	//Overflow - fatal
		return INT64_MIN;// Saturated negative number
}
/* Decrement the loop counter */
blkCnt--;
}
/* If the blockSize is not a multiple of 4, compute any remaining output samples here.
** No loop unrolling is used. */
blkCnt = blockSize % 0x4u;
#else
/* Run the below code for Cortex-M0 */
/* Initialize blkCnt with number of samples */
blkCnt = blockSize;
#endif /* #ifndef ARM_MATH_CM0 */
while(blkCnt > 0u)
{
/* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
/* Calculate dot product and then store the result in a temporary buffer. */
//sum += ((q63_t) * pSrcA++ * *pSrcB++) >> 14u;
	sum += ((q63_t) * pSrcA++ * *pSrcB++);
/* Decrement the loop counter */
blkCnt--;
}
/* Store the result in the destination buffer in 16.48 format */
 return sum;
}
/**
* @} end of dot_prod group
*/
