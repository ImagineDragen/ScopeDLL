#ifndef _HT_LAN_DLL_H_H
#define _HT_LAN_DLL_H_H

#ifndef DLL_API
#define DLL_API extern "C" __declspec(dllimport)
#endif
#define WIN_API __stdcall


//设置CH垂直位置
DLL_API WORD WINAPI dsoLANSetCHPos(WORD nDeviceIndex,WORD* pLevel,WORD nVoltDIV,WORD nPos,WORD nCH);

//设置Trigger 垂直 Level
DLL_API WORD WINAPI dsoLANSetVTriggerLevel(WORD nDeviceIndex,WORD* pLevel,WORD nPos);

//设置触发长度和预触发长度
DLL_API WORD WINAPI dsoLANSetHTriggerLength(WORD nDeviceIndex,ULONG nBufferLen,WORD HTriggerPos,WORD nTimeDIV,WORD nYTFormat);

//设置内存长度
DLL_API WORD WINAPI dsoLANSetBufferSize(WORD nDeviceIndex,WORD nBufferSize);

//设置CH和Trigger
DLL_API WORD WINAPI dsoLANSetCHAndTrigger(WORD nDeviceIndex,RELAYCONTROL RelayControl);
DLL_API WORD WINAPI dsoLANSetCHAndTriggerVB(WORD nDeviceIndex,WORD* pCHEnable,WORD* pCHVoltDIV,WORD* pCHCoupling,WORD* pCHBWLimit,WORD nTriggerSource,WORD nTriggerFilt,WORD nALT);


//设置Trigger和同步输出
DLL_API WORD WINAPI dsoLANSetTriggerAndSyncOutput(WORD nDeviceIndex,WORD nTriggerMode,WORD nTriggerSlope,WORD nPWCondition,ULONG nPW,USHORT nVideoStandard,USHORT nVedioSyncSelect,USHORT nVideoHsyncNumOption,WORD nSync);

//设置采样率
DLL_API WORD WINAPI dsoLANSetSampleRate(WORD nDeviceIndex,WORD nTimeDIV,WORD nYTFormat);

//初始化SDRam
DLL_API WORD WINAPI dsoLANInitSDRam(WORD nDeviceIndex);

//启动采集
DLL_API WORD WINAPI dsoLANStartCollectData(WORD nDeviceIndex);

//启动触发
DLL_API WORD WINAPI dsoLANStartTrigger(WORD nDeviceIndex);

//强制触发
DLL_API WORD WINAPI dsoLANForceTrigger(WORD nDeviceIndex);

//获取地址
DLL_API WORD WINAPI dsoLANGetState(WORD nDeviceIndex);

//获取是否已采集满512Bytes
DLL_API WORD WINAPI dsoLANGetPackState(WORD nDeviceIndex);

//判断SDRam是否初始化成功
DLL_API WORD WINAPI dsoLANGetSDRamInit(WORD nDeviceIndex);

//读取校对电平数据
DLL_API WORD WINAPI dsoLANReadCalibrationData(WORD nDeviceIndex,WORD* pLevel,WORD nLen);

//写入校对电平数据
DLL_API WORD WINAPI dsoLANWriteCalibrationData(WORD nDeviceIndex,WORD* pLevel,WORD nLen);

//读取数据
DLL_API WORD WINAPI dsoLANGetData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl/*,ULONG* p1,ULONG* p2,ULONG* p3*/);

DLL_API WORD WINAPI dsoLANGetScanData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl);

DLL_API WORD WINAPI dsoLANOpenConnet(WORD DeviceIndex,WORD nMode);//关闭网口，打开USB	//add by yt 20100709

DLL_API WORD WINAPI dsoLANResetWIFI(WORD DeviceIndex);//复位WIFI

DLL_API WORD WINAPI dsoLANGetFPGAVersion(WORD DeviceIndex);

DLL_API WORD WINAPI dsoLANGetRestartLANStatus(WORD DeviceIndex);//获取设备LAN是否启动完毕		//add by yt 20100709

DLL_API WORD WINAPI dsoLANModeSetIPAddr(WORD DeviceIndex,ULONG nIP,ULONG nSubnetMask,ULONG nGateway,USHORT nPort,BYTE* pMac);//add by yt 20100709
DLL_API WORD WINAPI dsoLANModeGetIPAddr(WORD DeviceIndex,ULONG* pIP,ULONG* pSubnetMask,ULONG* pGateway,USHORT* pPort,BYTE* pMac);

//20091215
//读取滚动数据
DLL_API WORD WINAPI dsoLANGetRollData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl);

DLL_API WORD WINAPI dsoLANOpenRollMode(WORD nDeviceIndex);

DLL_API WORD WINAPI dsoLANCloseRollMode(WORD nDeviceIndex);

DLL_API WORD WINAPI dsoLANSetPeakDetect(WORD nDeviceIndex,WORD nTimeDIV,WORD nYTFormat);

DLL_API WORD WINAPI dsoLANClosePeakDetect(WORD nDeviceIndex);

DLL_API ULONG WINAPI dsoLANGetHardFC(WORD nDeviceIndex);

DLL_API WORD WINAPI dsoLANSetHardFC(WORD nDeviceIndex,WORD nType);

DLL_API WORD WINAPI dsoLANResetCnter(WORD nDeviceIndex);

DLL_API WORD WINAPI dsoLANStartRoll(WORD nDeviceIndex);

DLL_API ULONG WINAPI dsoLANGetHardVersion(WORD DeviceIndex);

DLL_API WORD WINAPI dsoLANGetLanEnable(WORD DeviceIndex,short* pEnable);

//
//For SDK........................
//SDK读取数据--二次开发可参考2010-1-18左右的备份
DLL_API WORD WINAPI dsoSDLANGetData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl,WORD nInsertMode);
DLL_API WORD WINAPI dsoSDLANGetRollData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl);
DLL_API WORD WINAPI dsoSDLANGetScanData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl);

//20100420
DLL_API WORD WINAPI dsoLANInit(WORD nDeviceIndex,char* szlpV4,USHORT iPort);
DLL_API WORD WINAPI dsoLANInitSocket(WORD nDeviceIndex,USHORT* pIP,USHORT iPort);

DLL_API WORD WINAPI dsoLANClose(WORD nDeviceIndex);

DLL_API ULONG WINAPI ddsLANSetFrequency(WORD DeviceIndex,double dbFre,WORD* pWaveNum,WORD* pPeriodNum);
DLL_API ULONG WINAPI ddsLANSetCmd(WORD DeviceIndex, USHORT nSingle);
DLL_API ULONG WINAPI ddsLANSetSyncOut(WORD DeviceIndex, BOOL bEnable);
DLL_API ULONG WINAPI ddsLANDownload(WORD DeviceIndex,WORD iWaveNum, WORD* pData);
DLL_API ULONG WINAPI ddsLANSetOnOff(WORD DeviceIndex,short nOnOff);



#endif
