/*	Description:
*/
//file:CalcLib.C
//doc: https://docs.google.com/document/d/1rjnZofGtDpsplmIS-EEPfOyJbprwF7POoSeFoblEcTU/edit




#ifndef __RTMATH__
#define __RTMATH__
/**
	* @brief 32-bit fractional data type in 1.31 format.
   */
typedef int32_t q31_t;


  /**
   * @brief 64-bit fractional data type in 1.63 format.
   */
typedef int64_t q63_t;




uint32_t 	mSqrt64(int64_t op);
q63_t mVectorDotProduct(q31_t * pSrcA,q31_t * pSrcB,uint32_t blockSize);
int mIIR_1Tap(int x, int y,int alfa);


#endif  // __RTMATH__
