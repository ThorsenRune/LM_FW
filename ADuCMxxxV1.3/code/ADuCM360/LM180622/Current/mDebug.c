/*file:mDebug.c							things for debugging purpose
*/
//Bitmanipulation
#define CLEARBIT(x,y) x &= ~(1<< y)
//Set bit y (0-indexed) of x to '1' by generating a a mask with a '1' in the proper bit location and ORing x with the mask.
#define SET(x,y) x |= (1 << y)
//Set bit y (0-indexed) of x to '0' by generating a mask with a '0' in the y position and 1's elsewhere then ANDing the mask with x.
#define CLEAR(x,y) x &= ~(1<< y)
//Return '1' if the bit value at position y within x is '1' and '0' if it's 0 by ANDing x with a bit mask where the bit in y's position is '1' and '0' elsewhere and comparing it to all 0's.  Returns '1' in least significant bit position if the value of the bit is '1', '0' if it was '0'.
#define READ(x,y) ((0u == (x & (1<<y)))?0u:1u)
//Toggle bit y (0-index) of x to the inverse: '0' becomes '1', '1' becomes '0' by XORing x with a bitmask where the bit in position y is '1' and all others are '0'.
#define TOGGLE(x,y) (x ^= (1<<y))
//Return '1' if the bit value at position y within x is '1' and '0' if it's 0 by ANDing x with a bit mask where the bit in y's position is '1' and '0' elsewhere and comparing it to all 0's.  Returns '1' in least significant bit position if the value of the bit is '1', '0' if it was '0'.


extern int nTimerInMs[3];



void mTXFullError(){
	bErrFlags.errbits.TxBufferFull=1;
}

void mDebugHold(){
	while (1){  //Infinite Loop
		pADI_GP0->GPSET = BIT4;
		pADI_GP0->GPCLR = BIT4;
		pADI_GP0->GPSET = BIT4;
		pADI_GP0->GPCLR = BIT4;
		pADI_GP0->GPSET = BIT4;
		pADI_GP0->GPCLR = BIT4;
		pADI_GP0->GPSET = BIT4;
		pADI_GP0->GPCLR = BIT4;

	}
}



typedef union			//Just for debugging purpose to 
{
 int     DBState[1];      				/* Allows us to refer to the flags 'en masse' */
 struct
 {
  uint32_t
		A					: 1,							/*Bit 0: Enable piecewise linear function */
		B		: 1,     					/*Bit 1: Enable CHRGVCCS, Enable stimulation */
		C		: 1,							/* Enable MUX Switches */
		D				: 1,							/* Enable High Voltage Power Supply */
		E				: 1,							/* Unused as of 2018 */
		F		: 1,							/* Enable PLS CTRL on channel 1 */
		G		: 1,							/* Enable PLS CTRL on channel 2 */
		H			: 1,							/* Enable VPPCHARGE */
		I		: 1,							/* Increase or decrease aIAmp depending ALSO on RMSnew-RMSold */
		J		: 1,							/* Blanking enable (first 20 samples) */
		K		: 1,							/*Bit 10: First HP filter enable */
		L			: 1,							/* Two in channels, one out channel */
		M			: 1,							/* ??? */
		N		: 1,							/* ???*/
		O				: 1,							/* Unused */
		P				: 1,							/* Unused */
		Q				: 1,							/* Unused */
		R				: 1,							/* Unused */
		S				: 1,							/* Unused */
		T				: 1,							/* Unused */
		U				: 1,							/*Bit 20: Unused */
		V				: 1,							/* Unused */
		W				: 1,							/* Unused */
		X				: 1,							/* Unused */
		Y				: 1,							/*Bit 24: Low battery level*/
		Z			: 1,							/*Bit 25: Battery is full */
		AB	: 1,							/*Bit 26: Settings are invalid when read from FLASH */
		AC			: 1,							/*Bit 27: Shutting down the device								 	*/
		AD		: 1,							/* Enable stimulation output or PAUSE      	*/
		AE			: 1,							/*Bit 29: Key is down */
		AF	: 1,						/*Bit 30: Short press on pushbutton 								*/
		AG: 1;								/* Long press on pushbutton 								*/
	} DEBUGBITS;
} DEBUGSTATE;

DEBUGSTATE nDebugMode;


	


