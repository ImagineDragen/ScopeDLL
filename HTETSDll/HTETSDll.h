#ifndef _HT_ETS_DLL_H
#define _HT_ETS_DLL_H

#ifndef DLL_API
#define DLL_API extern "C" __declspec(dllimport)
#endif
#define WIN_API __stdcall


//ETS.........
DLL_API WORD WINAPI dsoHTSetTDCSel(WORD nDeviceIndex,WORD TDCSel,WORD nETS);

DLL_API long WINAPI dsoHTGetTDCData(WORD nDeviceIndex);

DLL_API WORD WINAPI dsoHTSetTDCCal(WORD nDeviceIndex,long TDCData);

DLL_API WORD WINAPI dsoHTWriteTDCCalData(WORD nDeviceIndex,long* pTDCCalData);

DLL_API WORD WINAPI dsoHTReadTDCCalData(WORD nDeviceIndex,long* pTDCCalData);

DLL_API WORD WINAPI dsoETSProc(USHORT* pDataIn,short* pDataOut,ULONG nInDataLen,USHORT nTimeBase,USHORT nHTriggerPos,long nTDC,long* pMaxMinTDC,long* pTDCOffset,USHORT nLeverPos,USHORT nCoupling,short nInvert);

#endif
