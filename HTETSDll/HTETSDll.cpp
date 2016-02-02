// HTETSDll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

//////////////////////////////////////////////////////////
//请求方向
#define DIR_IN		1		//device to host
#define DIR_OUT		0		//host to device

#ifndef MINISCOPE
//#define _ASY
#endif

#ifdef _ASY
CCyUSBDevice *pUSBDevice = NULL;
#define OUT_ENDPT	0x01	//端点：host to device
#define IN_ENDPT	0x02	//端点：device to host

BOOL TransferDataToDevice(PUCHAR buf,LONG len,ULONG TimeOut = 1000)
{
	BOOL status = FALSE;
	OVERLAPPED outOvLap;
	UCHAR  *outContext = NULL;
	CCyUSBEndPoint *OutEndpt = NULL;

	if(pUSBDevice->EndPointCount() > 0)
	{
		outOvLap.hEvent  = CreateEvent(NULL, false, false, "CYUSB_OUT"); 
		OutEndpt = pUSBDevice->EndPoints[OUT_ENDPT];
		if(OutEndpt != NULL)
		{
			outContext = OutEndpt->BeginDataXfer(buf,len,&outOvLap);
			OutEndpt->WaitForXfer(&outOvLap,TimeOut); 
			status = OutEndpt->FinishDataXfer(buf, (LONG&)len, &outOvLap,outContext); 
		}
		else
		{
			status = FALSE;
		}
		CloseHandle(outOvLap.hEvent);
	}
	else
	{
		status = FALSE;
	}
	return status;
}


BOOL TransferDataFromDevice(PUCHAR buf,LONG len,ULONG TimeOut = 1000)
{
	BOOL status = FALSE;
	OVERLAPPED inOvLap;
	CCyUSBEndPoint *InEndpt = NULL;
	UCHAR  *InContext = NULL;

	if(pUSBDevice->EndPointCount() > 0)
	{
		inOvLap.hEvent  = CreateEvent(NULL, false, false, "CYUSB_IN"); 
		InEndpt = pUSBDevice->EndPoints[IN_ENDPT];
		if(InEndpt != NULL)
		{
			InContext = InEndpt->BeginDataXfer(buf,len,&inOvLap);
			InEndpt->WaitForXfer(&inOvLap,TimeOut); 
			status = InEndpt->FinishDataXfer(buf, (LONG&)len, &inOvLap,InContext); 
		}
		else
		{
			status = FALSE;
		}
		CloseHandle(inOvLap.hEvent);
	}
	else
	{
		status = FALSE;
	}

	return status;
}

/*
int GetDevices(USHORT pID)
{
	int i = 0;
	int cyUSBDevice = 0;
	USB_CONFIGURATION_DESCRIPTOR ConfDesc;

	g_DevNum = 0;
	cyUSBDevice = pUSBDevice->DeviceCount();//获取CYPRESS设备数
	if(cyUSBDevice > 0)
	{
		for(i=0;i<cyUSBDevice;i++)
		{
			if(pUSBDevice->Open(i))
			{
				pUSBDevice->GetConfigDescriptor(&ConfDesc);
				if(pUSBDevice->ProductID == pID && pUSBDevice->VendorID == 0x04B5)
				{
					g_DevIndex[g_DevNum++] = i; //记住CYPRESS设备列表中的索引值
				}
				pUSBDevice->Close();
			}
		}
	}
	return g_DevNum;
}*/

#endif

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
#ifdef _ASY
	pUSBDevice = new CCyUSBDevice(NULL);
#endif
    return TRUE;
}


//收发线程参数
typedef struct _THREAD_CONTROL
{
   HANDLE hThread;
   HANDLE hDevice;
   DWORD Ioctl;
   PVOID InBuffer;
   DWORD InBufferSize;
   PVOID OutBuffer;
   DWORD OutBufferSize;
   DWORD BytesReturned;
   HANDLE completionEvent;
   BOOL  status;
} THREAD_CONTROL, *PTHREAD_CONTROL;
/*
读写线程函数，改变因为读写堵塞而使程序死机
*/
DWORD WINAPI TransferThread(void* pParam )
{
    PTHREAD_CONTROL threadControl;
	threadControl=(PTHREAD_CONTROL)pParam;
   // perform the ISO transfer
   threadControl->status = DeviceIoControl ( threadControl->hDevice,
                                             threadControl->Ioctl,
                                             threadControl->InBuffer,
                                             threadControl->InBufferSize,
                                             threadControl->OutBuffer,
                                             threadControl->OutBufferSize,
                                             &threadControl->BytesReturned,
                                             NULL);
   // if an event exists, set it
   if(threadControl->completionEvent)
   {
      SetEvent(threadControl->completionEvent);
   }
   return 0;
}

BOOL VendRequest(HANDLE hDevice,UCHAR VendNo,USHORT value,BOOL bDir,UCHAR*pdat,ULONG ncnt)
{
#ifdef _ASY
	BOOL status = FALSE;
	if(pUSBDevice != NULL && pUSBDevice->ControlEndPt != NULL)
	{
		CCyControlEndPoint* ept = pUSBDevice->ControlEndPt;
		ept->Target   = TGT_DEVICE;
		ept->ReqType  = REQ_VENDOR;
		ept->ReqCode  = VendNo;
		ept->Value    = value;//0x1900;
		ept->Index    = 0x00;

		if(DIR_IN == bDir)
		{
			status = ept->Read(pdat,(LONG&)ncnt);
		}
		else
		{
			status = ept->Write(pdat,(LONG&)ncnt);
		}
	}
	else
	{
		status = FALSE;
	}
	return status;
#else
	if (hDevice==NULL) 
	{
		return FALSE;
	}
	THREAD_CONTROL VendthreadControl;
	VENDOR_OR_CLASS_REQUEST_CONTROL	myRequest;  
	HANDLE VendCompleteEvent=NULL;
	VendCompleteEvent = CreateEvent(0,FALSE,FALSE,NULL);
	myRequest.requestType=0x02; // request type (1=class, 2=vendor)
	myRequest.recepient=0x00; // recipient (0=device,1=interface,2=endpoint,3=other)
	myRequest.request = VendNo;//请求号
	myRequest.value = value;//请求值
	myRequest.index = 0x00;//请求索引
	myRequest.direction =bDir;//0x01;//IN transfer direction (0=host to device, 1=device to host)
	VendthreadControl.hDevice=hDevice;
	VendthreadControl.Ioctl=IOCTL_EZUSB_VENDOR_OR_CLASS_REQUEST;
	VendthreadControl.InBuffer=(PVOID)&myRequest;
	VendthreadControl.InBufferSize=sizeof(myRequest);
	VendthreadControl.OutBuffer=pdat;
	VendthreadControl.OutBufferSize=ncnt;
	VendthreadControl.status=TRUE;
	VendthreadControl.completionEvent=VendCompleteEvent;
	VendthreadControl.BytesReturned = 0;
	// start and wait for transfer threads
/*	CWinThread * TestVend = AfxBeginThread(TransferThread, // thread function 
										   &VendthreadControl); // argument to thread function 
										   */
	DWORD dwThreadId, dwThrdParam = 1; 
    HANDLE hThread;
    hThread = CreateThread(	NULL,                        // no security attributes 
							0,                           // use default stack size  
							TransferThread,                  // thread function 
							&VendthreadControl,                // argument to thread function 
							0,                           // use default creation flags 
							&dwThreadId);                // returns the thread identifier 

	VendthreadControl.hThread = hThread;
    //等待请求回应
	WaitForSingleObject(VendCompleteEvent,INFINITE);
	CloseHandle(VendCompleteEvent);
	CloseHandle(hThread);
/*	if(bDir == DIR_IN)	//del by yt 20100727
	{
		VendthreadControl.status ^= 1;//读取时，返回0表示成功。与写入时正好相反。故在此进行"异或"操作
	}*/	//del by yt 20100727
	if(VendthreadControl.status == 0)//add by yt 20100727
	{
		VendthreadControl.status = 1;	//add by yt 20100727
	}
	return VendthreadControl.status;//0:成功；1：失败；
#endif
}
//打开设备,返回设备句柄
HANDLE OpenDevice(PCHAR devname)
{
#ifdef _ASY
	if(pUSBDevice != NULL && pUSBDevice->Open(atoi(devname)))
	{
		return pUSBDevice->DeviceHandle();
	}
	else
	{
		return NULL;
	}
#else
	char completeDeviceName[MAX_DRIVER_NAME] = "";
	char pcMsg[MAX_DRIVER_NAME] = "";
	HANDLE phDeviceHandle;
	
	//将多个字符串拼接成一个长字符串后返回CreateFile("\\\\.\\ezusb-0",  
	strcat (completeDeviceName,"\\\\.\\" );
	strcat (completeDeviceName,devname);
	
	phDeviceHandle = CreateFile(completeDeviceName,
								GENERIC_WRITE,
								FILE_SHARE_WRITE,
								NULL,
								OPEN_EXISTING,
								0,
								NULL);
	if(phDeviceHandle == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}
	else
	{
		return phDeviceHandle;
	} 
#endif
}
//关闭设备
void CloseDevice(HANDLE hDeviceHandle)
{
#ifdef _ASY
	if(pUSBDevice != NULL)
	{
		pUSBDevice->Close();
	}
#else
	if(hDeviceHandle != NULL)
	{
		CloseHandle(hDeviceHandle);
	}
#endif
}

//根据索引值选择设备
void SelectDeviceIndex(WORD DeviceIndex,PCHAR pcDriverName)
{
#ifdef _ASY
	sprintf(pcDriverName, "%d", DeviceIndex);
#else
	#ifdef _LAUNCH
		sprintf(pcDriverName, "d2250-%d", DeviceIndex);
	#elif defined(DSO3064A)
		sprintf(pcDriverName, "d306A-%d", DeviceIndex);
	#elif defined(DSO3074)
		sprintf(pcDriverName, "d3074-%d", DeviceIndex);
	#elif defined(DSO3074A)
		sprintf(pcDriverName, "d307A-%d", DeviceIndex);
	#else
		sprintf(pcDriverName, "d3064-%d", DeviceIndex);
	#endif
#endif
}

//测试USB接口速度
BOOL PCUSBSpeed(WORD DeviceIndex)
{
	char pcDriverName[MAX_DRIVER_NAME] = "";
	BOOL bResult=FALSE;
	BOOL bDir=DIR_IN;
	UCHAR VendNo=0xB2;
	UCHAR pBuffer[10];
	HANDLE hDevice;
	ULONG ncnt;
	//初始化设备名
	SelectDeviceIndex(DeviceIndex,pcDriverName);
	ncnt=10;
	hDevice = OpenDevice(pcDriverName);
	if(hDevice==NULL)
	{
		return FALSE;
	}
	USHORT value = 0x0000; //读取24LC64的起始地址
 	bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,ncnt);
	CloseDevice(hDevice);
	return pBuffer[0];
}


BOOL ResetDevice(WORD DeviceIndex,WORD control)
{
	char pcDriverName[MAX_DRIVER_NAME] = "";
	BOOL bResult=FALSE;
	BOOL bDir=DIR_OUT;
	UCHAR VendNo=0xb3;
	UCHAR pBuffer[10];
	HANDLE hDevice;
	ULONG ncnt;
	switch (control)//复位设备参数
	{
	case 1:
		pBuffer[0]=0x0f;
		pBuffer[1]=0x03;//0x11,3
		pBuffer[2]=0x03;
		pBuffer[3]=0x03;
		break;
	case 2:
		pBuffer[0]=0x0f;
		pBuffer[1]=0x0a;//1x10,6 trig_ok_check
		pBuffer[2]=0x0a;
		pBuffer[3]=0x0a;
		break;	
	case 3:        //交替或标准
		pBuffer[0]=0x0f;
		pBuffer[1]=0x09;//1x01,5
		pBuffer[2]=0x09;
		pBuffer[3]=0x09;
		break;	
	case 4: //scroll
		pBuffer[0]=0x0f;
		pBuffer[1]=0x01;//1x01,5
		pBuffer[2]=0x01;
		pBuffer[3]=0x01;
		break;
	case 5: //capture_start
		pBuffer[0]=0x0f;
		pBuffer[1]=0x02;//0x10,2
		pBuffer[2]=0x02;
		pBuffer[3]=0x02;
		break;
	case 6: //trig_start
		pBuffer[0]=0x0f;
		pBuffer[1]=0x08;//1x00,4
		pBuffer[2]=0x08;
		pBuffer[3]=0x08;
		break;
	default:
		VendNo=0xb4;
		pBuffer[0]=0x0f;
		pBuffer[1]=0x03;//0x11,3
		pBuffer[2]=0x03;
		pBuffer[3]=0x03;
		break;
	}
	//初始化设备名
	SelectDeviceIndex(DeviceIndex,pcDriverName);
	ncnt=10;
	hDevice = OpenDevice(pcDriverName);
	if(hDevice==NULL)
	{
		return FALSE;
	}
	USHORT value = 0x0000; //读取24LC64的起始地址
	bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,ncnt);
//-------------------------------------------------------------
    bDir=DIR_IN;
	VendNo=0xB2;
	bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,ncnt);

//-------------------------------------------------------------
	CloseDevice(hDevice);
	return pBuffer[0];
}

///////////////////////////////////////////////////////
WORD SetTDCSel(WORD DeviceIndex,WORD TDCSel,WORD nETS)
{                                                                           
	char pcDriverName[MAX_DRIVER_NAME] = "";
	PUCHAR outBuffer = NULL;
	BULK_TRANSFER_CONTROL   outBulkControl;
	HANDLE hOutDevice=NULL; 
	UINT m_nSize=4;

	SelectDeviceIndex(DeviceIndex,pcDriverName);
	if (!ResetDevice(DeviceIndex,1))
	{}

	hOutDevice = OpenDevice(pcDriverName);
	if(hOutDevice == NULL)
	{
		return 0;
	}
	outBuffer=(PUCHAR) malloc(m_nSize);
	outBuffer[0]=0x25;
    outBuffer[1]=0x00;
	outBuffer[2]=( (0x03&TDCSel) | (nETS << 3) );
	outBuffer[3]=0x00;

 	BOOLEAN status=FALSE;
	ULONG BytesReturned=0;
	outBulkControl.pipeNum=0;//端点选择EP2
#ifdef _ASY
	status = TransferDataToDevice(outBuffer,m_nSize);
#else
	status = DeviceIoControl(hOutDevice,
							 IOCTL_EZUSB_BULK_WRITE,
							 (PVOID)&outBulkControl,
							 sizeof(BULK_TRANSFER_CONTROL),
							 outBuffer,//输出缓冲区
							 m_nSize,//字节数，在对话框中可以设置
							 &BytesReturned,//返回字节数据
							 //这里为了测试速度，没有测试返回字节数
							 NULL);	 
#endif
	//关闭设备
	free(outBuffer);
	CloseDevice(hOutDevice);
    return status;
}

WORD SetTDCCal(WORD DeviceIndex,long TDCData)
{
	char pcDriverName[MAX_DRIVER_NAME] = "";
	PUCHAR outBuffer = NULL;
	BULK_TRANSFER_CONTROL   outBulkControl;
	HANDLE hOutDevice=NULL;
    UINT m_nSize=8;

	SelectDeviceIndex(DeviceIndex,pcDriverName);	//初始化设备名
	ResetDevice(DeviceIndex,1);
	hOutDevice = OpenDevice(pcDriverName);
	if(hOutDevice == NULL)
	{
		return 0;
	}
	outBuffer=(PUCHAR) malloc(m_nSize);
	outBuffer[0]=0x16;
    outBuffer[1]=0x00;
	outBuffer[2]=0x01;  //daccs                              
	outBuffer[3]=0x00;  
	outBuffer[4]=0xff&(unsigned char)(TDCData&0xff);
	outBuffer[5]=0xff&(unsigned char)(0x50|((TDCData>>8)&0x0F)) ;
	outBuffer[6]=0x00;
	outBuffer[7]=0x00;	
 	BOOLEAN status=FALSE;
	ULONG BytesReturned=0;
	outBulkControl.pipeNum=0;//端点选择EP2
#ifdef _ASY
	status = TransferDataToDevice(outBuffer,m_nSize);
#else
	status =  DeviceIoControl (hOutDevice,
						 IOCTL_EZUSB_BULK_WRITE,
						 (PVOID)&outBulkControl,
						 sizeof(BULK_TRANSFER_CONTROL),
						 outBuffer,//输出缓冲区
						 m_nSize,//字节数，在对话框中可以设置
						 &BytesReturned,//返回字节数据,这里为了测试速度，没有测试返回字节数
						 NULL);	 
#endif
	//关闭设备
	free(outBuffer);
	CloseDevice(hOutDevice);
    return status;
}

WORD WriteTDCCalData(WORD DeviceIndex,long* TDCCalData)
{
	char pcDriverName[MAX_DRIVER_NAME] = "";
	BOOL bResult=FALSE;
	BOOL bDir=DIR_OUT;
	UCHAR VendNo=0xa2; //读写EEPROM
	UCHAR pBuffer[8*1024];
	HANDLE hDevice;

	ULONG ncnt;
	ncnt= 2;
	USHORT value =0x1B00;//读取24LC64的起始地址
	SelectDeviceIndex(DeviceIndex,pcDriverName);
	hDevice = OpenDevice(pcDriverName);
	if(hDevice==NULL)
	{
		return FALSE;
	}

	pBuffer[0]=(unsigned char) (*TDCCalData&0xFF);//低8位
	pBuffer[1]=(unsigned char) ((*TDCCalData>>8)&0xFF);//高8位

	bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,ncnt);
	
	CloseDevice(hDevice);
	if (bResult)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

WORD ReadTDCCalData(WORD DeviceIndex,long* TDCCalData)
{
	char pcDriverName[MAX_DRIVER_NAME] = "";
	BOOL bResult=FALSE;
	BOOL bDir=DIR_IN;
	UCHAR VendNo=0xA2; //读写EEPROM请求
	UCHAR pBuffer[8*1024];
	HANDLE hDevice;
	ULONG ncnt;

	ncnt=2;
	USHORT value = 0x1B00; //读取24LC64的起始地址

	SelectDeviceIndex(DeviceIndex,pcDriverName);
	hDevice = OpenDevice(pcDriverName);
	if(hDevice==NULL)
	{
		return FALSE;
	}
	bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,ncnt);
	CloseDevice(hDevice);

	*TDCCalData = pBuffer[0]+pBuffer[1]*256;

	if (bResult)
		return 1;
	else
		return 0;
}

long GetTDCData(WORD DeviceIndex)
{
	char pcDriverName[MAX_DRIVER_NAME] = "";
	PUCHAR outBuffer = NULL;
	BULK_TRANSFER_CONTROL   outBulkControl;
	HANDLE hOutDevice=NULL,hInDevice=NULL;
    UINT m_nSize=2;
	long nTCycle = 0;
	BOOL bOK = FALSE;
	USHORT nFlag = 0;

//初始化设备名
	SelectDeviceIndex(DeviceIndex,pcDriverName);
	ResetDevice(DeviceIndex,1);
	hOutDevice = OpenDevice(pcDriverName);
	if(hOutDevice == NULL)
	{
		return 0;
	}
	outBuffer=(PUCHAR) malloc(m_nSize);
	outBuffer[0]=0x26;
    outBuffer[1]=0x00;
 	BOOLEAN status=FALSE;
	ULONG BytesReturned=0;
	outBulkControl.pipeNum=0;//端点选择EP2
#ifdef _ASY
	status = TransferDataToDevice(outBuffer,m_nSize);
#else
	status =  DeviceIoControl (hOutDevice,
							 IOCTL_EZUSB_BULK_WRITE,
							 (PVOID)&outBulkControl,
							 sizeof(BULK_TRANSFER_CONTROL),
							 outBuffer,//输出缓冲区
							 m_nSize,//字节数，在对话框中可以设置
							 &BytesReturned,//返回字节数据
							 //这里为了测试速度，没有测试返回字节数
							 NULL);	
#endif
	//关闭设备
	free(outBuffer);
	CloseDevice(hOutDevice);

    PUCHAR inBuffer = NULL;
	BULK_TRANSFER_CONTROL   inBulkControl;
	BOOL bResult=FALSE;
	//初始化设备名
	//打开设备
	if(PCUSBSpeed(DeviceIndex))
	{
		m_nSize = USB_PACK_SIZE;
	}
	else
	{
		m_nSize = 64;
	}
	hInDevice = OpenDevice(pcDriverName);
	if(hInDevice == NULL)
	{
	   return 0;
	}
	inBuffer=(PUCHAR) malloc(m_nSize);
	memset(inBuffer, 0,m_nSize);
	int nReadTimes = USB_PACK_SIZE / m_nSize;//最少读取512字节
	inBulkControl.pipeNum = 1;
	//起始时间
	while(!bOK)
	{
		for(int i=0;i<nReadTimes;i++)
		{
#ifdef _ASY
			status = TransferDataFromDevice(inBuffer,m_nSize);
#else
			status = DeviceIoControl(hInDevice,
									 IOCTL_EZUSB_BULK_READ,
									 (PVOID)&inBulkControl,
									 sizeof(BULK_TRANSFER_CONTROL),
									 (LPVOID)inBuffer,//输出缓冲区
									 m_nSize,//字节数，在对话框中可以设置
									 &BytesReturned,//返回字节数据
									 //这里为了测试速度，没有测试返回字节数
 									 NULL);
#endif
			if(status == 0)
			{
				break;
			}
			if(i==0)
			{
			//保存数据
				//0x00:复位；0x04：正在记数，未记满；0x07：记数完毕
				if((inBuffer[3] >> 5) == 0x07)//计数完毕
				{
					bOK = TRUE;
					nTCycle = inBuffer[0] + (inBuffer[1]<<8)+(inBuffer[2]<<16) + ((inBuffer[3]&0x1F)<<24);
				}
			}
		}
		nFlag ++;
		if(nFlag > 100)
		{
			break;//超时退出
		}
	}
    //关闭设备
	free(inBuffer);//释放缓冲区
	CloseDevice(hInDevice); 
	
	if(!bOK)
	{
		nTCycle = 0;
	}

	return nTCycle;
}

WORD ETSProc(USHORT* pDataIn,short* pDataOut,ULONG nInDataLen,USHORT nTimeBase,USHORT nHTriggerPos,long nTDC,long* pMaxMinTDC,long* pTDCOffset,USHORT nLeverPos,USHORT nCoupling,short nInvert)
{
	int i = 0;
	float fPointPos = 0.0f;
	SHORT nSection;				//当前屏幕分为的段数
	float fSectionPoints;		//每段包含的点数
	ULONG nTriggerPointPos = 0;	//触发点
	ULONG nLeftPointPos = 0;
	long nMaxTDC = *pMaxMinTDC;
	long nMinTDC = *(pMaxMinTDC+1);
	long nMaxTDCOffset = *pTDCOffset;
	long nMinTDCOffset = *(pTDCOffset+1);

//检查合法性
	if(nTimeBase > MAX_ETS_TIMEDIV)// > ETS 最大时基
	{
		return 0;//出错，返回
	}
	if(nMaxTDC <= (nMinTDC - nMinTDCOffset))//如果最大值 <= 最小值
	{
		return 0;//出错，返回
	}
	if(nTDC > nMaxTDC)
	{
		return 0;//出错，返回
	}
	if(nTDC <= (nMinTDC + nMaxTDCOffset))
	{
		return 0;//出错，返回
	}
	else
	{
		nTDC = nTDC - nMinTDC - nMaxTDCOffset;
	}
//开始计算
	if(nTimeBase == 3)				//50ns --20 000M
	{
		nSection = 50;
		fSectionPoints = 204.8f;
	}
	else if(nTimeBase == 2)			//20ns --50 000M
	{
		nSection = 20;
		fSectionPoints = 512.0f;
	}
	else if(nTimeBase == 1)			//10ns --100 000M
	{
		nSection = 10;
		fSectionPoints = 1024.0f;
	}
	else if(nTimeBase == 0)			//5ns --200 000M
	{
		nSection = 5;
		fSectionPoints = 2048.0f;
	}
//求触发点
	nTriggerPointPos = (ULONG)(nInDataLen * (nHTriggerPos / 100.0));//得到原始触发点
	nLeftPointPos = (ULONG)(nSection * (nHTriggerPos / 100.0));
//求摆点的位置
	fPointPos = (float)(nTDC * 1.0f / (nMaxTDC - nMinTDC - nMinTDCOffset) * (fSectionPoints - 1));//得到位置
	if((fPointPos >= fSectionPoints) || (fPointPos < 0))
	{
		return 0;
	}
	if(nCoupling == GND)
	{
		for(i=0;i < nSection;i++)
		{
			*(pDataOut + ULONG(i * fSectionPoints + fPointPos)) = 0;
		}
	}
	else
	{
		if(nInvert == 0)
		{
			for(i=0;i < nSection;i++)
			{
				*(pDataOut + ULONG(i * fSectionPoints + fPointPos)) = *(pDataIn + nTriggerPointPos - nLeftPointPos + i) - (MAX_DATA - nLeverPos);
			}
		}
		else
		{
			for(i=0;i < nSection;i++)
			{
				*(pDataOut + ULONG(i * fSectionPoints + fPointPos)) = (MAX_DATA - nLeverPos) - *(pDataIn + nTriggerPointPos - nLeftPointPos + i);
			}
		}
	}
//更新
	//....
	return *(pDataIn + nTriggerPointPos);
}


//Function Export...........
////////////////////////

//设置TDCSel
DLL_API WORD WINAPI dsoHTSetTDCSel(WORD nDeviceIndex,WORD TDCSel,WORD nETS)
{
	return SetTDCSel(nDeviceIndex,TDCSel,nETS);
}

//获取TDC数
DLL_API long WINAPI dsoHTGetTDCData(WORD nDeviceIndex)
{
	return GetTDCData(nDeviceIndex);
}

//设置TDC 校对的数到硬件
DLL_API WORD WINAPI dsoHTSetTDCCal(WORD nDeviceIndex,long TDCData)
{
	return SetTDCCal(nDeviceIndex,TDCData);
}

//将校对的TDC数写入到EEPROM中
DLL_API WORD WINAPI dsoHTWriteTDCCalData(WORD nDeviceIndex,long* pTDCCalData)
{
	return WriteTDCCalData(nDeviceIndex,pTDCCalData);
}

//读取EEPROM中的校对数据
DLL_API WORD WINAPI dsoHTReadTDCCalData(WORD nDeviceIndex,long* pTDCCalData)
{
	return ReadTDCCalData(nDeviceIndex,pTDCCalData);
}

//
DLL_API WORD WINAPI dsoETSProc(USHORT* pDataIn,short* pDataOut,ULONG nInDataLen,USHORT nTimeBase,USHORT nHTriggerPos,long nTDC,long* pMaxMinTDC,long* pTDCOffset,USHORT nLeverPos,USHORT nCoupling,short nInvert)
{
	return ETSProc(pDataIn,pDataOut,nInDataLen,nTimeBase,nHTriggerPos,nTDC,pMaxMinTDC,pTDCOffset,nLeverPos,nCoupling,nInvert);
}
