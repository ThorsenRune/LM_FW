/* Serial communication protocol
	Descrizione: vedi Documentation of Communication protocol LM
	doc: https://docs.google.com/document/d/1wuR4gnlNCaXn3vR_KdJ1YcMevpIwXOFL47MnkEutoEE/edit#
*/
#include "cProtocol.h"
#include "FiFO.c"				//Include a Fifo buffer implementation
#include "mErrFlags.h"			// Error messages
#include "mDataExch.h"			//161028


uint8_t aTX[kTXBufferSize];		// Transmit array
uint8_t aRX[kTXBufferSize];		//Receive array
tTXProt oTXProt;							//global object of tranmission protocol




// In host its received by ShowSignals


void mCommInitialize(void){
	//Initialize the communication protocol allocating memory for buffers 
	oTX=mFIFO_new(aTX,sizeof(aTX)); // get a new object
	oRX=mFIFO_new(aRX,sizeof(aRX)); // get a new object
}






void Ucom_Send24bit(void * oTX,int VarId, int *  x24Bit2Send,int Count){
	int i;
		while (mFIFO_Free(oTX)<3){}
				mFIFO_push(oTX,kSend24Bit);	// xmit header
				UART_TX_Trigger();			// Start transmission
				mFIFO_push(oTX,VarId);			// xmit VarId
				mFIFO_push(oTX,Count);			// xmit Count
		for (i=0; i< Count; i++){		//!!!implement overflow check
			while (mFIFO_Free(oTX)<3)
				{UART_TX_Trigger();
					bErrFlags.errbits.bTXOverflow =1;
				}
					mFIFO_push(oTX, x24Bit2Send[i]>>16);			//RT:Send ADC0 data (bit 16:24)
					mFIFO_push(oTX, x24Bit2Send[i]>>8);			//RT:Send ADC0 data (bit 16:24)
					mFIFO_push(oTX, x24Bit2Send[i]>>0);			//RT:Send ADC0 data (bit 16:24)
				}
}
// In host its received by ShowSignals
void Ucom_Send32bit(tFIFO oTX,int VarId, int *  Data2Send, int Count){
/*	@oTX: Pointer to the FIFO register
		@VarId:	Identifier of the variable
		@Count:	Number of elements in the array (1 is scalar) 
		@Data2Send:	The array of data to send as 32 bit data (4 bytes) 
*/
	int i;
		while (mFIFO_Free(oTX)<3)
			{UART_TX_Trigger();}
				mFIFO_push(oTX,kSend32Bit);	// xmit header
				UART_TX_Trigger();			// Start transmission
				mFIFO_push(oTX,VarId);			// xmit VarId
				mFIFO_push(oTX,Count);			// xmit Count
				for (i=0; i< Count; i++){		//!!!implement overflow check
		while (mFIFO_Free(oTX)<4)
					{UART_TX_Trigger();
						bErrFlags.errbits.bTX32Overflow =1;
					}
					mFIFO_push(oTX, Data2Send[i]>>24);			//RT:Send ADC0 data (bit 16:24)
					mFIFO_push(oTX, Data2Send[i]>>16);			//RT:Send ADC0 data (bit 16:24)
					mFIFO_push(oTX, Data2Send[i]>>8);			//RT:Send ADC0 data (bit 16:24)
					mFIFO_push(oTX, Data2Send[i]>>0);			//RT:Send ADC0 data (bit 16:24)
				}
}


/*
		Protocol manipulations using tTXComm types


*/
int8_t mTXVarId_Find(tTXProt* obj, char zVarId)   //return index of varid
/* 	@obj: 		the protocol object
		@zVarId:	the ID of the variable to find
		@return:	index of the sought variable
*/	
{
		char i;
    for (i=0; i<obj->VarCount ; i++ )
      {  if (zVarId==obj->VarId[i])return i;
      }
      return -1;
};
	 
	 
	
////////////////////////////	RX TX Dispatcher	////////////////////////////


void mDispatchTX(tTXProt*  obj)   //Run though object and send data witch have TXCount>0
/* 	@obj: 		the protocol object
using:void Ucom_Send32bit(tFIFO oTX,int VarId, int *  Data2Send, int Count)
*/	
{
		char i;
//!! todo
// if (sErrMsg[0]!='\0') iTXErrMsg(zTxState);	//Error messaging
//else if (0<oProtocol.zDoTxExpose) iD2HCommReset(zTxState);    // Expose variables to host
    for (i=0; i<obj->VarCount ; i++ )
      {  if (0< obj->TXCount[i])
				{
					//send the data
					if (obj->VarType[i]==kSend32Bit)
					{Ucom_Send32bit(oTX,obj->VarId[i], obj->VarPtr[i],obj->VarLen[i]);}
					else if (obj->VarType[i]==kSend24Bit)
					{Ucom_Send24bit(oTX,obj->VarId[i], obj->VarPtr[i],obj->VarLen[i]);}
					obj->TXCount[i]=obj->TXCount[i]-1;			//Decrease counter
				}
      }
	/*		if (bErrFlags.all_flags[0]) {
				Ucom_Send32bit(oTX,kUartErrMsg,bErrFlags.all_flags ,1);
			}	*/
}








void mDispatchRX(tTXProt*  obj)   //Receiving data and dispatch commands
/* 	@obj: 		the protocol object
   //Processing the serial receive data and
    // puts them in the protocol buffer
Revisions:
*/	
{
static uint8_t rcv1,idx;			//Data received
static tUartCmd rxCmd=kReady;     // Current cmd state
static uint8_t zState;				// State machine indicator
static int *  DestData;
   while (mFIFO_isEmpty(oRX)==0) {     // Char in buffer, go read and process it
     rcv1=mFIFO_pop(oRX);       //Get new data
		 if (0==rxCmd) 			//Set command as receive data
			 {
				 rxCmd=(tUartCmd) rcv1;
				 zState=0;			//Restart statemachine
				 // Process single command procedures
					if  (kCommInit ==rxCmd)				// Reset the protocol by sending exposed variables
					{		
						Expose2Protocol();
						rxCmd=kReady;											//Command is processed
					} else if (kHandshake==rxCmd)
					{
						mSendVersionInfo();
						bErrFlags.all_flags[0] = 0U	;		//Reset error flags
					}
			 }
		  //Demux header
		 else if (kSetReq ==rxCmd)					//Set values of variable
			{ 
				if (0==zState){				// Get variable
					idx=mTXVarId_Find(&oTXProt,rcv1);
					DestData=oTXProt.VarPtr[idx];	//Make a pointer to the address
					zState=1;
				}
				else if (1==zState)		//Index of array to write to
					{
					idx=rcv1;
					zState++;
					}
				else if (2==zState)
				{
					DestData[idx]=rcv1<<24;
					zState++;
				}
				else if (3==zState)
				{
					DestData[idx]+=rcv1<<16;
					zState++;
				}	
				else if (4==zState)
				{
					DestData[idx]+=rcv1<<8;
					zState++;
				}			
				else if (5==zState){
					DestData[idx]+=rcv1;
					rxCmd=kReady;											//Command is processed
				}
			}
		 else if (kGetReq ==rxCmd)					//Request data command
		 {		//Expect ID and increase its send counter
				idx=mTXVarId_Find(&oTXProt,rcv1);
				oTXProt.TXCount[idx]++; //Make it send
				zState=0;				//End  statemachine
				rxCmd=kReady;	
			 	mPowerWatchDogReset();		//R180920 Reset the watchdog for power timeout
			}
			else {
				//This would be a reveive error because rxCmd was not recognized
				bErrFlags.errbits.bReceiveError =1;
				zState=0;				//End  statemachine
				rxCmd=kReady;	
			}
		}
}
//Send a string(21/11/2012)
void mTX_PushTxt(const char* PushTxt) 
{ 	uint8_t zlen,j;
		for (zlen=0;((PushTxt[zlen]>=32)&(zlen<50));zlen++){}; 	// Find length of string
    mFIFO_push(oTX,zlen);	
    for (  j=0;j<zlen;j++){
      mFIFO_push(oTX,PushTxt[j]);   // Push on TX buffer
		}
}
void ExposeVars2Host(const char* VarName,uint8_t VarId,uint8_t ArrLen,uint8_t VarType)    // Send the k
{
//  dbprint(VarName,VarId,ArrLen,VarType);		
		mFIFO_push(oTX,kCommInit);				// Header 
		mTX_PushTxt((char*) VarName);							//Send symbolic name
		mFIFO_push(oTX,VarId);							// Send identifier
 		mFIFO_push(oTX,ArrLen);							//Send ArrLen
 		mFIFO_push(oTX,VarType);   //Send VarType         
}


void mTXExposeArray(tTXProt*  obj,const char* name,int Arr[],uint8_t  ArrLength, tUartCmd  ArrType)
// Register a variable and expose it to host 
{
    if (obj->VarCount<kMaxVarCount){	//Only if not full
          obj->VarId[obj->VarCount]=obj->VarCount+kIDOffset;
          obj->VarPtr[obj->VarCount]= Arr;
          obj->VarLen[obj->VarCount]=ArrLength;
          obj->VarType[obj->VarCount]=ArrType;
          ExposeVars2Host(name,obj->VarId[obj->VarCount],
                          ArrLength,
                          ArrType);
          obj->VarCount++;                     //Increment number of registered elements
          };
}	 
void mProtocol_Init(tTXProt*  obj)						//Initialize protocol object
{				
	obj->VarCount=0;
}
