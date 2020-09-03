
#include <stdio.h>
#include <string.h>
#include <ADuCM360.h>

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



void DACINIT(void)
{
   // Configure DAC output for 0-1.2V output range, Normal 12-bit mode and immediate update.
   DacCfg(DACCON_CLR_Off,DACCON_RNG_IntVref,DACCON_CLK_HCLK,DACCON_MDE_12bit);	  
   DacWr(0,0x1FF0000);                 // Output value of 150mV
}

void DACSet(int value ){
	 DacWr(0,value<<16);
}
