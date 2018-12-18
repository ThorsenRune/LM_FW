/*
	Description: everything related to the powersupply on the board
	file:LVPS.c
	doc:https://docs.google.com/document/d/1KMO6a_Mq7j-GIDjQwsaHk9N7xyFVowgL6U5mHEgv84Q/edit#
*/


extern void SaveParametersOnFlash(void);

//typedef enum {Off, Green, Red, Orange, BlinkRed, BlinkGreen} LedMode;
void mBlueToothEnable(eAction act){
  //Description: not implemented in HW as of 180727
	//act can be kEnable or kDisable
			if (act==kEnable){
     //     DioSet(pADI_GP0, 0x08); // P0.3 (WAKE_SW) HIGH
			}
			else if (act==kDisable) {
     //     DioClr(pADI_GP0, 0x08); // P0.3 (WAKE_SW) LOW
			}

}


void mBlueToothConfig(eAction act){
	//Configure the BLE module as xmission or programming mode
 	//Description: Set HI or LO CMD_MLDP pin (P0.6)
	//act can be kEnable or kDisable

    if (act==kEnable){
          DioSet(pADI_GP0, 0x40); // P0.6 (CMD_MLDP) HIGH 
              }
    else if (act==kDisable){
          DioClr(pADI_GP0, 0x40); // P0.6 (CMD_MLDP) LOW
              }
}
