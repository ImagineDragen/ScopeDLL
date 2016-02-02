
#ifndef _HT_DSO_FFT_H_
#define _HT_DSO_FFT_H_

#ifdef DLL_API
#else
#define DLL_API extern "C" _declspec(dllimport)
#endif
#define WINAPI __stdcall

typedef struct _HT_COMPEX
{
	double Real;//Êµ²¿
	double Cmpx;//Ðé²¿
}HTCOMPEX,*PHTCOMPEX;

DLL_API WORD WINAPI dsoHTFFT(double* pSourceData,ULONG nSourceDataLen,USHORT nWindowType,double *FFTResult,USHORT nScale);

#endif
