/* DATA EXCHANGE INTERFACE */
/*	doc:https://docs.google.com/document/d/1wCabyWoKCb97PyVLtXw29vbsvwkai80EguKox1GC-EU/edit			*/


#include "cProtocol.h"
#include "mDataExch.h"
static const char FWversion[]="LM181009, FW:" __DATE__; 
//A string constant with the compilation date;
 

int iFilterCoeff[2]={1, 1};


/***************** COMMUNICATION PROTOCOL SETUP *****************/
void Expose2Protocol(){
	// Define and transmit vars to HOST
	/*	INSERT VARIABLES THAT YOU WANT TO EXPOSE BELOW	*/

	mProtocol_Init (&oTXProt);			//Initialize protocol object
										//Type of variable

	EXPOSEARRAY(&oTXProt,nMode.all_flags);
	EXPOSEARRAY(&oTXProt,bErrFlags.all_flags);
	EXPOSEARRAY(&oTXProt,nTimerInMs);
	EXPOSEARRAY(&oTXProt,StimCountEx);

	EXPOSEARRAY(&oTXProt,aRMSfilt);
//	EXPOSEARRAY(&oTXProt,aRMSshift);	//Todo OBsolete
	
	EXPOSEARRAY(&oTXProt,xData);
	EXPOSEARRAY(&oTXProt,yData);
	EXPOSEARRAY(&oTXProt,xData1);
	EXPOSEARRAY(&oTXProt,yData1);
	
	EXPOSEARRAY(&oTXProt,IMin);
	EXPOSEARRAY(&oTXProt,aIAmp);
	EXPOSEARRAY(&oTXProt,aMLev);
	EXPOSEARRAY(&oTXProt,Offset);
	EXPOSEARRAY(&oTXProt,Gain);
	EXPOSEARRAY(&oTXProt,IMax);
	EXPOSEARRAY(&oTXProt,IMaxLimit);
	
	EXPOSEARRAY(&oTXProt,StimNegDur);
	EXPOSEARRAY(&oTXProt,HVValue);
	EXPOSEARRAY(&oTXProt,nBlankInterval);
	
};
//Handshake
void mSendVersionInfo(){//180918 Send information about the firmware
		mFIFO_push(oTX,kHandshake);				// Header 
		mTX_PushTxt((char*) FWversion);							//Send Firmware information
}

