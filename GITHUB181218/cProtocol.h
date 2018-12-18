//	bla bla
//#include "cProtocol.h"
//doc: https://docs.google.com/document/d/16H3fB_DwdOqdSNWW9mhAr_vsrxmBgVp5vI-F5g0VoQs/edit
#ifndef ___cProtocol
#define ___cProtocol
#include <stdint.h>
#include "fifo.h"
// A MACRO THAT DEFINE EXPOSE TO PROTOCOL for vectors  
#define EXPOSEARRAY(obj,arg1) mTXExposeArray(obj,#arg1,arg1,sizeof(arg1)/sizeof(arg1[0]),kSend32Bit);
/* Expands to something like
	mTXExposeArray(&oTXProt,"xData",		   data name 
		xData,														//Var pointer	
		sizeof(xData)/sizeof(xData[0]),		//length kADCBuffsize
		kSend32Bit);	
*/
//#define EXPOSESCALAR(arg1) ExposeScalar(#arg1,arg1,1,sizeof(arg1));
//***** IMPORTING (Requiring the following methods to be defined)
void Expose2Protocol(void);					//+ A method that initializes the protocol by sending all variable names to host
void mSendVersionInfo(void);	
// Communication constants
typedef enum commHeaders {
	kReady=0,
	kUartErrMsg=13,
  kCommInit = 101 ,//      '"Initialize Protocol request to DSP and preample for returned data [SymbolicVarName, VarId,ArraySize and Type]
  kHandshake = 104,//       '"Handshake"
  kSendByte=208,
   kSetReq = 102, //   		'Host write to uP memory
   kGetReq  = 111 ,//    	'Request to read uP memory
//   kDSP2HostData = 201//    'Response to DSP2HostCmd with data from DSP
	kSend24Bit=224,						//'Wordlength to send
	kSend32Bit=232
} tUartCmd;


/* -----------------------   PROTOCOL DEFINITIONS  ------------*/
#define kMaxVarCount 20         //Possible number of variables in the protocol
#define kIDOffset 64						// First id number (could also be an array of possible ids)
typedef struct {
    char VarCount;    // Number of variables registered
    // The variables name is not preserved it is sent by Expose method and serves no more
    char VarId[kMaxVarCount];          // Unique identifier for host
    char VarLen[kMaxVarCount];         // vectorlength of data
    void* VarPtr[kMaxVarCount];          //Ptr to  Address variable
    tUartCmd VarType[kMaxVarCount];        // bytelength of data
		char TXCount[kMaxVarCount];					//Downcounter for transmissions
																				// when it reaches zero no more transmissions will be performed
}   tTXProt ;			//protocol type for RX/TX (before it was CLSProtPack)
extern tTXProt oTXProt;			//global object of tranmission protocol
/* END-------------------   PROTOCOL DEFINITIONS  ------------*/
// Packs
//kSend24Bit,VarId,Count,24bit data[count]


/********************* METHODS *************************/
	void mDispatchRX(tTXProt*  obj);   	//Receive UART data and process it
	void mDispatchTX(tTXProt*  obj);   
	//Run though object and send data witch have TXCount>0
	void mTXExposeArray(tTXProt*  obj,const char* name,int Arr[],uint8_t  ArrLength, tUartCmd  ArrType);
	// Register a variable and expose it to host 
	void mProtocol_Init(tTXProt*  obj);			//Initializing the protocol		




/****************PRIVATE******************************/
extern void UART_TX_Trigger (void );			//Sends a datum if TX FIFO buffer is not empty and no data are being transmitted
	void Ucom_Send24bit(void* oTX,int VarId, int *  x24Bit2Send,int Count);
	void Ucom_Send32bit(tFIFO oTX,int VarId, int *  Data2Send,int Count);
void mCommInitialize(void);	//	Initialize the communication buffer




#endif
