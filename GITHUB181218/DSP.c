/*	Description: Digital signal processing functions	*/
/*	
	doc: 
	https://docs.google.com/document/d/1_B1SbbP8oeTmGLVkSrVkCJzxTQxH8h7kGUbyv_rE8Po/edit
	*/
/*	file: DSP.C	*/

#include "CalcLib.h"
#include "DSP.h"





//****************************************************************************************
int mMean(int aX[kADCBuffSize]){
	int i,sum=0;
	for (i=0; i< kADCBuffSize; i++){
		sum=sum+(aX[i]>>8);
	}
	sum=(sum/kADCBuffSize)<<8;
	return sum;
}

void mX_2_Y(int Alt){						// Copy X to Y
	int i;
	int offs[2];
	offs[0]=mMean(xData);
	offs[1]=mMean(xData1);
	for (i=0; i< kADCBuffSize; i++){
			yData[i]=xData[i]-offs[0];	
			yData1[i]=xData1[i]-offs[1];	
		}
}

void mSignalProcessing(){		//3 'rd call in main
	// Digital signal processing
			cLoopCount[0]++;											// Loop Counter
		bFlip=(cLoopCount[0]&1);
		//R16114   Process the buffer having been filled by DMA in last main cycle
		mCombFilter(1-bFlip);			// HP IIR filter, LP FIR filter and blanking, creating signsl xdata and ydata		
 
		mBlanking();
		mRMS_IIRFilter();											// R16118	Calculate RMS and filter with IIR LP filter
		if ( nMode.bits.ENPLW&&nMode.bits.STIMENABLE) 
			mPWLinFun180925();										// Calculate stimulation level aIAmp
		mStimCountUp();												// STIM COUNT UP
}






int mLimit(int a,int x,int b){		//Limit x between a and b, but max b
	if (a>x)		x=a;
	if (x>b) 		x=b;
	return x;
}
 
void mBlanking(){// Apply blanking
	int i;
		for (i=1; i< kADCBuffSize; i++){
		if ((nBlankInterval[0]<i)&&(i<nBlankInterval[1])){			//First blanking interval
			yData[i]=0;
			yData1[i]=0;
		} else if (((nBlankInterval[2]<i)&&(i<nBlankInterval[3]))){			//Second blanking interval
			yData[i]=0;
			yData1[i]=0;

		}
	}
}	
void mCombFilter(uint8_t Alt){						// Comb filter and blank
	int i;
  {
			// COMB FIR fiter: y(n)= x(n)-x(n-bl)
			for (i=0; i< kADCBuffSize; i++){
				yData[i]=aADCBuffer[Alt].aABuff[i]-xData[i];		//y=x(n)-x(n-bl)
				yData1[i]=aADCBuffer[Alt].aBBuff[i]-xData1[i];	//y=x(n)-x(n-bl)
				xData[i]=aADCBuffer[Alt].aABuff[i];							//x(n(t+1))=x(n(t+1)-bl)
				xData1[i]=aADCBuffer[Alt].aBBuff[i];						//x(n(t+1))=x(n(t+1)-bl)
			}
		}
	}


void mRMS_IIRFilter(void){			//RMS LP Filter
	int dx,i;
	int x[kSignals];
	x[0]=mSqrt64(mVectorDotProduct(yData,yData,kADCBuffSize));	//Sum Squared (not divided by # samples
	x[1]=mSqrt64(mVectorDotProduct(yData1,yData1,kADCBuffSize));
	for (  i=0; i< kSignals; i++){
		x[i]=kADC2nVFactor*x[i];				//Convert into nV
		x[i]=mLimit(0,x[i],500000);			//Cant be more than 0.5mV (1000000nV)
		if (nMode.bits.SR_FILTER){		//Slewrate limiting filter or 
			dx=x[i]-aRMSfilt[i];
			dx=mLimit(-nRMS_SlewRateLimit,dx,nRMS_SlewRateLimit);			//Limits the change rate of the filter
			aRMSfilt[i]=aRMSfilt[i]+dx;
		} else {										//Lowpass filter
			aRMSfilt[i]=mIIR_1Tap(x[i], aRMSfilt[i],  alfa[i]);
		}
	}
}
void mPWLinFun180925(void){   // aIAmp=limit(imin,gain*(EMG-Offset),iMax)
	int i;									//Level of EMG   ù
	int nPWLVal;
	float nGain;
	for (  i=0; i< kSignals; i++){
		IMax[i]=mLimit(0,IMax[i],IMaxLimit[0]);
		nGain=Gain[i]*kGainScale;
		nPWLVal=nGain*(aRMSfilt[i]-Offset[i]);			//  apply gain so uV becomes uA
		aIAmp[i]=mLimit(IMin[i],nPWLVal,IMax[i]);			//Fix between limits
	}
}
void mIAmpLimit(int32_t *aIAmp){		//Pass by reference
if(aIAmp[0]>=IMax[0]){
		aIAmp[0]=IMax[0];
	}
	if(aIAmp[1]>=IMax[1]){
		aIAmp[1]=IMax[1];
	}
}

