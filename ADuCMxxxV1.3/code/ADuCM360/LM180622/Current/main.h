/*	Global declarations for main.c									*/
//doc:	https://docs.google.com/document/d/1J7WfNNQ_wmlb0BNaZipmqmmzmAiU5so7l5LrrJCQ6LY/edit
//***************************			GENERAL CONSTANTS			******
//static int const var = 5; //  	is type safe and respects scope

static const uint32_t q31One= 0x80000000;		//R161215

static int const sampfreq= 3;		// Sampling frequency SEE ADUCM360 pg 9
extern void mCombFilter(uint8_t Alt);
extern void mBlanking(void);
extern void mCombFilter(uint8_t Alt);
extern void mRMS_IIRFilter(void);
extern void mPWLinFun(void);
extern void mIAmpLimit(int32_t *aIAmp);
extern void mHPFilter(uint8_t Alt);
extern void mSystemCheck(void);
extern int mLowBattCheck(void);
extern 	void mWait(volatile int Duration);


