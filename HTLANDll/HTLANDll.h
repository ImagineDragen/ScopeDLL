#ifndef _HT_LAN_DLL_H_H
#define _HT_LAN_DLL_H_H

#ifndef DLL_API
#define DLL_API extern "C" __declspec(dllimport)
#endif
#define WIN_API __stdcall


//����CH��ֱλ��
DLL_API WORD WINAPI dsoLANSetCHPos(WORD nDeviceIndex,WORD* pLevel,WORD nVoltDIV,WORD nPos,WORD nCH);

//����Trigger ��ֱ Level
DLL_API WORD WINAPI dsoLANSetVTriggerLevel(WORD nDeviceIndex,WORD* pLevel,WORD nPos);

//���ô������Ⱥ�Ԥ��������
DLL_API WORD WINAPI dsoLANSetHTriggerLength(WORD nDeviceIndex,ULONG nBufferLen,WORD HTriggerPos,WORD nTimeDIV,WORD nYTFormat);

//�����ڴ泤��
DLL_API WORD WINAPI dsoLANSetBufferSize(WORD nDeviceIndex,WORD nBufferSize);

//����CH��Trigger
DLL_API WORD WINAPI dsoLANSetCHAndTrigger(WORD nDeviceIndex,RELAYCONTROL RelayControl);
DLL_API WORD WINAPI dsoLANSetCHAndTriggerVB(WORD nDeviceIndex,WORD* pCHEnable,WORD* pCHVoltDIV,WORD* pCHCoupling,WORD* pCHBWLimit,WORD nTriggerSource,WORD nTriggerFilt,WORD nALT);


//����Trigger��ͬ�����
DLL_API WORD WINAPI dsoLANSetTriggerAndSyncOutput(WORD nDeviceIndex,WORD nTriggerMode,WORD nTriggerSlope,WORD nPWCondition,ULONG nPW,USHORT nVideoStandard,USHORT nVedioSyncSelect,USHORT nVideoHsyncNumOption,WORD nSync);

//���ò�����
DLL_API WORD WINAPI dsoLANSetSampleRate(WORD nDeviceIndex,WORD nTimeDIV,WORD nYTFormat);

//��ʼ��SDRam
DLL_API WORD WINAPI dsoLANInitSDRam(WORD nDeviceIndex);

//�����ɼ�
DLL_API WORD WINAPI dsoLANStartCollectData(WORD nDeviceIndex);

//��������
DLL_API WORD WINAPI dsoLANStartTrigger(WORD nDeviceIndex);

//ǿ�ƴ���
DLL_API WORD WINAPI dsoLANForceTrigger(WORD nDeviceIndex);

//��ȡ��ַ
DLL_API WORD WINAPI dsoLANGetState(WORD nDeviceIndex);

//��ȡ�Ƿ��Ѳɼ���512Bytes
DLL_API WORD WINAPI dsoLANGetPackState(WORD nDeviceIndex);

//�ж�SDRam�Ƿ��ʼ���ɹ�
DLL_API WORD WINAPI dsoLANGetSDRamInit(WORD nDeviceIndex);

//��ȡУ�Ե�ƽ����
DLL_API WORD WINAPI dsoLANReadCalibrationData(WORD nDeviceIndex,WORD* pLevel,WORD nLen);

//д��У�Ե�ƽ����
DLL_API WORD WINAPI dsoLANWriteCalibrationData(WORD nDeviceIndex,WORD* pLevel,WORD nLen);

//��ȡ����
DLL_API WORD WINAPI dsoLANGetData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl/*,ULONG* p1,ULONG* p2,ULONG* p3*/);

DLL_API WORD WINAPI dsoLANGetScanData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl);

DLL_API WORD WINAPI dsoLANOpenConnet(WORD DeviceIndex,WORD nMode);//�ر����ڣ���USB	//add by yt 20100709

DLL_API WORD WINAPI dsoLANResetWIFI(WORD DeviceIndex);//��λWIFI

DLL_API WORD WINAPI dsoLANGetFPGAVersion(WORD DeviceIndex);

DLL_API WORD WINAPI dsoLANGetRestartLANStatus(WORD DeviceIndex);//��ȡ�豸LAN�Ƿ��������		//add by yt 20100709

DLL_API WORD WINAPI dsoLANModeSetIPAddr(WORD DeviceIndex,ULONG nIP,ULONG nSubnetMask,ULONG nGateway,USHORT nPort,BYTE* pMac);//add by yt 20100709
DLL_API WORD WINAPI dsoLANModeGetIPAddr(WORD DeviceIndex,ULONG* pIP,ULONG* pSubnetMask,ULONG* pGateway,USHORT* pPort,BYTE* pMac);

//20091215
//��ȡ��������
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
//SDK��ȡ����--���ο����ɲο�2010-1-18���ҵı���
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
