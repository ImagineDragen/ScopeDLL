#ifndef DSO_DLL_API_H
#define DSO_DLL_API_H

#define EXPORTDLL extern "C" __declspec (dllimport) 



//Functions For Softwear
EXPORTDLL WORD __stdcall dsoGetChannelData(WORD DeviceIndex,short* ch1_data, short* ch2_data,
										   struct ControlStruct* Control_Data1, int Ram_Addr,WORD calData);
//Function For Hardware
EXPORTDLL WORD __stdcall dsoSearchDevice(WORD DeviceIndex);
EXPORTDLL WORD __stdcall dsoGetDeviceAddress(WORD DeviceIndex,WORD* Address);
EXPORTDLL WORD __stdcall dsoSetTriggerAndSampleRate(WORD,WORD,struct ControlStruct *);
EXPORTDLL WORD __stdcall dsoSetTriggerAndSampleRateNew(WORD,WORD,struct ControlStruct *,WORD ,WORD ,WORD ,WORD ,long );
EXPORTDLL WORD __stdcall dsoGetLogicData(WORD);
EXPORTDLL WORD __stdcall dsoSetVoltageAndCoupling(WORD,WORD,WORD,WORD,WORD,WORD);
EXPORTDLL BOOL __stdcall dsoSetOffset(WORD ,struct LeversStruct * ,WORD ,WORD ,WORD ,WORD* );
EXPORTDLL WORD __stdcall dsoCaptureStart(WORD);
EXPORTDLL long __stdcall dsoTriggerEnabled(WORD);
EXPORTDLL WORD __stdcall dsoGetChannelLevel(WORD DeviceIndex,WORD * level,WORD length);
EXPORTDLL WORD __stdcall dsoSetChannelLevel(WORD DeviceIndex,WORD * level,WORD length);
EXPORTDLL WORD __stdcall dsoGetCalData(WORD DeviceIndex,WORD * calData);
EXPORTDLL WORD __stdcall dsoGetCaptureState(WORD DeviceIndex,int *Ram_Addr);
EXPORTDLL BOOL __stdcall dsoForceTrigger(WORD DeviceIndex);
EXPORTDLL WORD __stdcall dsoSetFilt(WORD DeviceIndex,WORD Ch1_Filt,WORD Ch2_Filt,WORD Trigger_Filt);

EXPORTDLL WORD __stdcall dsoFFT(struct FFTStruct* FFTControl,short* Data,WORD lever,double* FFTResult);
EXPORTDLL WORD __stdcall dsoFFTGetSamples(short*,WORD,short*);
EXPORTDLL WORD __stdcall dsoFFTBuffer(struct ControlStruct*,WORD TIMEBASE, short* ,short*);
EXPORTDLL BOOL __stdcall dsoGetDeviceID(WORD DeviceIndex,WORD* ID);
EXPORTDLL BOOL __stdcall dsoSetDeviceID(WORD DeviceIndex,WORD ID);
EXPORTDLL WORD __stdcall dsoSetCalData(WORD DeviceIndex,WORD calData);
EXPORTDLL int  __stdcall dsoGetCalTrigState(WORD DeviceIndex);
EXPORTDLL void __stdcall InitLevelRange(WORD DeviceIndex);
EXPORTDLL BOOL __stdcall dsoReadFlash(WORD DeviceIndex,UCHAR* pBuffer);
EXPORTDLL BOOL __stdcall dsoWriteFlash(WORD DeviceIndex,UCHAR* pBuffer);
#endif