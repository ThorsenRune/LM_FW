//	Declarations related to INA circuits








//	doc:https://docs.google.com/document/d/1SEZXWRvTPabK7OtY0XR1rXtGvwDgT6o8Nku2bVSqIOE/edit
#ifndef __D161028__
#define __D161028__




//+ 	DEFINE DmaDescr from library
#if defined(__ARMCC_VERSION) || defined(__GNUC__)
extern DmaDesc dmaChanDesc     [CCD_SIZE * 2] __attribute__ ((aligned (DMACHAN_DSC_ALIGN)));      
#endif




#ifdef __ICCARM__
#pragma data_alignment=(DMACHAN_DSC_ALIGN)
extern DmaDesc dmaChanDesc     [CCD_SIZE * 2];
#endif








#endif  // __D161028__
