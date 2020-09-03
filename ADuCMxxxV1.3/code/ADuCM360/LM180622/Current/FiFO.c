//	Description:
//	doc: 	https://docs.google.com/document/d/1Xb_LmRBxhW8xrABMmaYeDoR8EbGt8TDyCx-7DoS1y_E/edit
//	file: FiFO.c
//	owner: cProtocol.c
/* Circular buffer implementation*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <aducm360.h>
#include "FiFO.h"				 // The problem is double declarations
#include "mErrFlags.h"			// Error messages


#define INTENA  NVIC_EnableIRQ(UART_IRQn) 
#define INTDIS  NVIC_DisableIRQ(UART_IRQn)
//void mWaitMs(long int);
extern void UART_TX_Trigger (void);


/*------------------------- IMPLEMENTATION --------------------*/
// Initialize the buffer
tFIFO mFIFO_new( void* array,int size1) {
    tFIFO cb=malloc(sizeof(struct sFIFO));  //Mandatory
    cb->size  = size1;
    cb->start = 0;
    cb->end   = 0;
    cb->full = 0;
    cb->empty = 1;  // Initially the buffer is empty
    cb->elems = array;
    return cb;
}
 
tFIFO  oTX;
tFIFO  oRX;
 




// Return full flag
int mFIFO_isFull(tFIFO cb) {
    return cb->full; }


// Return empty flag
int mFIFO_isEmpty(tFIFO  cb) {
    return cb->empty; }




// Size of buffer
int mFIFO_size(tFIFO  cb) {
    return cb->size; }
// Size of buffer


int _mFIFO_FreeSub(tFIFO  cb) {
		// Its empty so return the buffersize
    if (cb->empty) 
				return cb->size;
		// Start is behind end so end can rollover (+size)
    if (cb->end>cb->start) 
				return cb->size-(cb->end-cb->start);
	// End is before start:start-end is free 
    if (cb->end<cb->start) 
				return cb->start-cb->end;
		if (cb->full) 
				return 0; 
	return 0; // (cb->full) 
}
int mFIFO_Free(tFIFO  cb) {
	int i;
		  INTDIS;
			i=_mFIFO_FreeSub( cb);			 
			INTENA;
			return  i;
}


// Push an element on the buffer, returns 1 when full else 0
int mFIFO_push1(tFIFO cb, uint8_t val ){
    cb->empty=0;                //Not empty anymore
    if (cb->full) {				//Buffer is full
				if (cb->full){		//Overflow condition (wait and hope)
//							INTENA;			//Enable and disable interrupt
//							mWaitMs(1);
							bErrFlags.errbits.bFifoOverflow=1;
//							INTDIS;
					}							//If full wait until empty
		}
    cb->elems[cb->end]  =  val;
    cb->end = (cb->end + 1);
    if (cb->end >= cb->size){   //Rollover
        cb->end=0;
    }
    if (cb->end == cb->start){   //Buffer full
        cb->full=1;
    }


    return cb->full;
}
int mFIFO_push(tFIFO cb, uint8_t val ){
	int i;
		   INTDIS;
			i=mFIFO_push1(cb, val );
			UART_TX_Trigger();			// Start transmission
			INTENA;
	return i;
}


// Check an element in the buffer, returns  o1 when full else 0
int mFIFO_peek(tFIFO cb, int offset ){
	int count=0,pos=0;
    count=(cb->end-cb->start);          // Number of elements in buffer
    if (count<0) count=count+cb->size;      //Rollover
    pos = (cb->start + offset);
    if (pos >= cb->size) {   //Rollover
        pos=pos-cb->size;
    }
    return cb->elems[pos];
}




// Get oldest element from the buffer.
int mFIFO_pop1(tFIFO cb ){
    int val=0;
    cb->full=0;                //Not full anymore
    val = cb->elems[cb->start];
    cb->start = (cb->start + 1);
    if (cb->start >= cb->size){   //Rollover
        cb->start=0;
    }
    if (cb->end == cb->start){   //Buffer full
        cb->empty=1;
    }
    return val;
}
int mFIFO_pop(tFIFO cb ){
	int i;
		   INTDIS;
			i= mFIFO_pop1(cb );
			INTENA;
		return i;
}

