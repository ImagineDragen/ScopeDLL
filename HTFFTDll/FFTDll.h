
#ifndef _HT_DSO_FFT_H_
#define _HT_DSO_FFT_H_

#ifdef DLL_API
#else
#define DLL_API extern "C" _declspec(dllimport)
#endif
#define WINAPI __stdcall

typedef struct _HT_COMPEX
{
	double Real;//ʵ��
	double Cmpx;//�鲿
}HTCOMPEX,*PHTCOMPEX;

DLL_API WORD WINAPI dsoHTFFT(double* pSourceData,ULONG nSourceDataLen,USHORT nWindowType,double *FFTResult,USHORT nScale);

#endif
