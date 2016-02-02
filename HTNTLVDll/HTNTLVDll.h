#ifdef DLL_API
#else
#define DLL_API extern "C" _declspec(dllimport)
#endif
#define WINAPI __stdcall

typedef HANDLE DSO_HANDLE;

//////////////////////////////////////////////////////////

DLL_API int WINAPI dsoSearch();
DLL_API BOOL WINAPI dsoSetFrequency(int index, double frequency, int* wavePointNum, int* TNum);		
DLL_API BOOL WINAPI dsoGetMeasure(int index, BOOL bFreq, double* measure);	
DLL_API BOOL WINAPI dsoSetSingleWave(int index, BOOL bSingle);				
DLL_API BOOL WINAPI dsoResetCounter(int index);	
DLL_API BOOL WINAPI dsoSetTrigger(int index, BOOL bInter, BOOL bEdge);	
DLL_API BOOL WINAPI dsoGetDigitalIn(int index, unsigned short* In);				
DLL_API BOOL WINAPI dsoSetDigitalOut(int index, unsigned short Out);		
DLL_API BOOL WINAPI dsoDownload(int index, unsigned short* buf, int number);
DLL_API BOOL WINAPI dsoSetDIOMode(int index, BOOL mode);	
DLL_API BOOL WINAPI dsoSetDivNum(int index, int num);		
DLL_API BOOL WINAPI dsoCheck(int index);					
DLL_API BOOL WINAPI dsoSetPowerOnOutput(int index, BOOL bOpen);	


DLL_API BOOL WINAPI dsoGetDivNum(int index);


/////////////////////////////////////////////////
DLL_API BOOL WINAPI dsoGetDevEnable(int index);
DLL_API USHORT WINAPI dsoSetCHNum(int index,USHORT nCHNum);
DLL_API USHORT WINAPI dsoSetCHOpen(int index,BOOL* pEnable);
DLL_API USHORT WINAPI dsoSetCHSample(int index,ULONG nSampleLen);
DLL_API USHORT WINAPI dsoSetCHVolt(int index,USHORT* pVolt,USHORT nVoltLen);
DLL_API USHORT WINAPI dsoSetSampleRate(int index,USHORT nSample);
DLL_API USHORT WINAPI dsoStartCaptureData(int index,USHORT nMode);
DLL_API USHORT WINAPI dsoCaptureFinish(int index);
DLL_API USHORT WINAPI dsoReadNormalData(int index,USHORT* pSrcData,ULONG nSrcLen);
DLL_API USHORT WINAPI dsoSetIOCtrl(int index,USHORT nIO1,USHORT nIO2);
DLL_API USHORT WINAPI dsoSetTrigLevel(int index,int nLevel,USHORT nVoltDIV,USHORT nCalData,float fAmpCalData);
DLL_API USHORT WINAPI dsoSetHTrigPos(int index,USHORT nTimeDIV,USHORT nOpenNum,USHORT nHTriggerPos);
DLL_API USHORT WINAPI dsoInitDevice(int index);
DLL_API USHORT WINAPI dsoForceTrigger(int index);
DLL_API USHORT WINAPI dsoSetTriggerInfo(int index,USHORT nTrigSrc,USHORT nSlope);
DLL_API USHORT WINAPI dsoTriggerEnable(int index);

DLL_API float WINAPI dsoGetVoltZoom(USHORT nVoltDIV);
DLL_API USHORT WINAPI dsoGetRealVoltDIV(USHORT nVoltDIV);
DLL_API float WINAPI dsoGetVoltDIVFactor(USHORT nVoltDIV);//获取放大倍数
DLL_API USHORT WINAPI dsoHTMiniReadNormalData(int index,MINICH* pMiniCH);
DLL_API USHORT WINAPI dsoHTMiniReadScanData(int index,MINICH* pMiniCH);
DLL_API USHORT WINAPI dsoHTMiniReadRollData(int index,MINICH* pMiniCH,double* pdbT);
DLL_API USHORT WINAPI dsoHTMiniReadAD(int index,MINICH* pMiniCH);
DLL_API USHORT WINAPI dsoWriteCalData(int index,USHORT* pCalData,ULONG nDataLen);
DLL_API USHORT WINAPI dsoReadCalData(int index,USHORT* pCalData,ULONG nDataLen);
DLL_API USHORT WINAPI dsoHTMiniFlash(int index,USHORT* pData,ULONG* pDataLen,short nPos,USHORT nType);
DLL_API USHORT WINAPI dsoSetGeneratorTimeUS(int index,double dbTime,USHORT nWaveNum);
DLL_API USHORT WINAPI dsoSetGeneratorOutput(int index,USHORT nPos,USHORT nOnOff,USHORT nWaveNum);
DLL_API USHORT WINAPI dsoSetIOEnableProgram(int index,USHORT nEnable);//是IO口可编程

DLL_API USHORT WINAPI dsoGetCalT(int index,double* pT);
DLL_API USHORT WINAPI dsoGetNowT(int index,double* pT);

DLL_API WORD WINAPI dsoSetLanguage(int index,WORD nSet);
DLL_API WORD WINAPI dsoGetLanguage(int index,WORD* pSet);//add by yt 20110729
DLL_API WORD WINAPI dsoGetARMVersion(int index,WORD* pVersion);//add by yt 20110903

DLL_API float WINAPI dsoGetAmpCalData(USHORT* pCalLevel,USHORT nCH,USHORT nVoltDIV);
DLL_API USHORT WINAPI dsoSDReadNormalData(int index,MINICH* pMiniCH,short** pSrcData,USHORT* pCalLevel);

DLL_API USHORT WINAPI dsoSaveToDevice(int index);

DLL_API USHORT WINAPI dsoWriteAmpCalData(int index,short* pCalData);

DLL_API USHORT WINAPI dsoReadAmpCalData(int index,short* pCalData);

DLL_API USHORT WINAPI dsoStartWatch(int index);

DLL_API USHORT WINAPI dsoEndWatch(int index);

DLL_API USHORT WINAPI dsoSetGND(int index);

DLL_API USHORT WINAPI dsoExitGND(int index);
//For test
DLL_API USHORT WINAPI dsoTest(DSO_HANDLE hDSO);
//
DLL_API USHORT WINAPI dsoSoftTrigger(short* pSrcDataLen,MINICH* pMiniCH,USHORT nCH,short nTrigLevel,USHORT nSlope,USHORT nHTrigPos);
DLL_API USHORT WINAPI dsoSoftCopy(short* pSrcData,ULONG nLen,USHORT nStartPt);
