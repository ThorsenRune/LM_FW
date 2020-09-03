//		FLASH MEMORY HEADER FILE
//		Doc: https://docs.google.com/document/d/1aTjkl3xuJJsGRroTmT75OOpIK1fVRWfMd2-23w2Qq7M/edit


int FeeWrEn(int iMde)
{
   if (iMde)
      pADI_FEE->FEECON0 |= 0x4;
   else
      pADI_FEE->FEECON0 &= 0xFB;
   
   return 1;
}

int FeeSta(void)
{
   return pADI_FEE->FEESTA;
}

