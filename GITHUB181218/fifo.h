//	Description:	Definistion of a FIFO buffer for serial communication with HOST
//	doc: 	https://docs.google.com/document/d/1vE58OZHDy1X-wGKQ4NZ-dkIybI7iQLg9qaZRSy9Vqjc/edit
//	file:FIFO.h				 


#ifndef FIFO_H_INCLUDED
#define FIFO_H_INCLUDED
#include <stdint.h>
// Type of elements to buffer
typedef struct { uint8_t value; } tBuffElem;
//161028
static const int kTXBufferSize=600; //R161028: Size of UART sending buffer at 115kbaud, 690 bytes/cycle can be sent
/* Circular buffer object */
typedef struct sFIFO{
volatile    int         size ;   /* maximum number of elements           */
volatile    int         start;  /* index of oldest element              */
volatile    int         end;    /* index at which to write new element  */
volatile    char         full;   // Flag for buffer beeing full
volatile    char         empty;  // Flag for buffer beeing empty
volatile    uint8_t*			 elems;  /* vector of elements                   */
} *tFIFO;


extern tFIFO  oTX;
extern tFIFO  oRX;


/*------------- INTERFACE -----------------------*/
tFIFO mFIFO_new( void* array,int size1);        //Returns an object of FIFO data
void mFIFO_del(tFIFO  cb);          // Destructor
// METHODS
int mFIFO_push(tFIFO  cb, uint8_t elem );
int mFIFO_peek(tFIFO cb,   int offset );
int mFIFO_pop(tFIFO cb );
int mFIFO_size( tFIFO cb);					// Return size of buffer
int mFIFO_isEmpty( tFIFO cb);				// Returns 1 if empty
int mFIFO_isFull(tFIFO  cb);				//FIFO is full
int mFIFO_Free(tFIFO   cb);                  // Number of free elements in buffer
#endif // FIFO_H_INCLUDED
