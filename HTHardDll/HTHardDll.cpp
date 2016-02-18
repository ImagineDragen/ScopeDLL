// HTHardDll.cpp : Defines the entry point for the DLL application.
//
//�޸���־
/*

---20091226
����dsoHTSetHTriggerLength �����һ������
����dsoHTSetSampleRate �����һ������
����dsoHTSetPeakDetect �޸������һ������,�����һ������
�¼Ӻ��� dsoHTGetHardFC--��ȡӲ��Ƶ�ʼ�/������
�¼Ӻ��� dsoHTSetHardFC--����Ӳ��Ƶ�ʼ�/������



---2010/01/04
�޸�����DLL,��������MFC

���Ӻ��� dsoHTStartRoll

---2010-1-15 ~ 2010-1-19
��������λ�����----��ͣʱ�϶�,��������,XY,����,��ӡ,Rͨ��---�ȹ���.
�޸�����λ�������ȡRoll/Scanģʽ���ݵķ�ʽ.

HardDll��ֻ�޸���GetDataCountinue������. ԭ����GetDataCountinue����ΪSDGetDataCountinue,�Һ������ݲ���,���ڶ��ο���.
������SDK(���ο�����)����
dsoSDHTGetRollData
dsoSDHTGetScanData
HardDll�޸���dsoHTSetPeakDetect����
//
20100227
BOOL dsoGetHardVersion(WORD DeviceIndex);//


*/
#include "stdafx.h"

//////////////////////////////////////////////////////////
//������
#define DIR_IN		1		//device to host
#define DIR_OUT		0		//host to device
//#define HEAD_DATA 0x1704
#ifndef MINISCOPE
//#define _ASY
#endif

#ifdef _ASY
CCyUSBDevice *pUSBDevice = NULL;
#define OUT_ENDPT	0x01	//�˵㣺host to device
#define IN_ENDPT	0x02	//�˵㣺device to host

BOOL TransferDataToDevice(PUCHAR buf,LONG len,ULONG TimeOut = 1000)
{
    BOOL  = FALSE;
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


#endif

#ifndef FILE_TEST
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
#endif


//�շ��̲߳���
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
��д�̺߳������ı���Ϊ��д������ʹ��������
*/

void CloseDevice(HANDLE hDeviceHandle)
{

    if(hDeviceHandle != NULL)
    {
        CloseHandle(hDeviceHandle);
    }

}

BOOL        VendRequest(HANDLE hDevice,UCHAR VendNo,USHORT value,BOOL bDir,UCHAR*pdat,ULONG ncnt)
{

    if (hDevice==NULL)
    {
        return FALSE;
    }
    VENDOR_OR_CLASS_REQUEST_CONTROL	myRequest;
    ULONG BytesReturned=0;
    //HANDLE VendCompleteEvent=NULL;
    WORD status=FALSE;
    //VendCompleteEvent = CreateEvent(0,FALSE,FALSE,NULL);
    myRequest.requestType=0x02; // request type (1=class, 2=vendor)
    myRequest.recepient=0x00; // recipient (0=device,1=interface,2=endpoint,3=other)
    myRequest.request = VendNo;//�����
    myRequest.value = value;//����ֵ
    myRequest.index = 0x00;//��������
    myRequest.direction =bDir;//0x01;//IN transfer direction (0=host to device, 1=device to host)
    // start and wait for transfer threads
    /*	CWinThread * TestVend = AfxBeginThread(TransferThread, // thread function
        &VendthreadControl); // argument to thread function

        */
    status = DeviceIoControl (hDevice,
                              IOCTL_EZUSB_VENDOR_OR_CLASS_REQUEST,
                              (PVOID)&myRequest,
                              sizeof(VENDOR_OR_CLASS_REQUEST_CONTROL),
                              pdat,
                              ncnt,
                              &BytesReturned,
                              NULL);
    return status;
}
//���豸,�����豸���
HANDLE OpenDevice(PCHAR devname)
{
#ifdef _ASY
    if(pUSBDevice != NULL && pUSBDevice->Open(atoi(devname)))
    {
        return INVALID_HANDLE_VALUE;//pUSBDevice->DeviceHandle();
    }
    else
    {
        return NULL;
    }
#else
    char completeDeviceName[MAX_DRIVER_NAME] = "";
    HANDLE phDeviceHandle;

    //������ַ���ƴ�ӳ�һ�����ַ����󷵻�CreateFile("\\\\.\\ezusb-0",
    strcat (completeDeviceName,"\\\\.\\" );
    strcat (completeDeviceName,devname);
    //MessageBox(NULL,LPCTSTR(completeDeviceName),"0C00",MB_OK);
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
//�ر��豸

//��������ֵѡ���豸
void SelectDeviceIndex(WORD DeviceIndex,PCHAR pcDriverName)
{
    sprintf(pcDriverName, "d6CDE-%d", DeviceIndex);


}


BOOL        ResetDevice(WORD DeviceIndex,WORD control)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    //  BOOL bResult=FALSE;
    BOOL bDir=DIR_OUT;
    UCHAR VendNo=0xb3;
    UCHAR pBuffer[10];
    HANDLE hDevice;
    ULONG ncnt;
    switch (control)//��λ�豸����
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
    case 3://������׼
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
    //��ʼ���豸��
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    ncnt=10;
    hDevice = OpenDevice(pcDriverName);
    if(hDevice==NULL)
        return FALSE;
    USHORT value = 0x0000; //��ȡ24LC64����ʼ��ַ
    VendRequest(hDevice,VendNo,value,bDir,pBuffer,ncnt);
    //-------------------------------------------------------------

    bDir=DIR_IN;
    VendNo=0xB2;
    VendRequest(hDevice,VendNo,value,bDir,pBuffer,ncnt);
    CloseDevice(hDevice);
    return pBuffer[0];

}



//����USB�ӿ��ٶ�
BOOL PCUSBSpeed(WORD DeviceIndex)
{
    //#ifndef _LAUNCH_TEST
    char pcDriverName[MAX_DRIVER_NAME] = "";
    UCHAR  bResult;
    BOOL bDir=DIR_IN;
    UCHAR VendNo=0xB2;
    PUCHAR pBuffer=NULL;
    HANDLE hDevice;
    ULONG ncnt=10;
    pBuffer=(PUCHAR)malloc(ncnt);
    memset(pBuffer,0,ncnt);
    //��ʼ���豸��
    SelectDeviceIndex(DeviceIndex,pcDriverName);

    hDevice = OpenDevice(pcDriverName);
    if(hDevice==NULL)
    {
        return FALSE;
    }
    USHORT value = 0x0000; //��ȡ24LC64����ʼ��ַ
    VendRequest(hDevice,VendNo,value,bDir,pBuffer,ncnt);
    CloseDevice(hDevice);
	bResult=pBuffer[0];
    free(pBuffer);
    return bResult;
}

WORD SetLANEnable(WORD DeviceIndex,short nEnable)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    HANDLE hOutDevice=NULL;
    UINT m_nSize=1;
    BOOL bResult=FALSE;
    BOOL bDir=DIR_OUT;
    UCHAR VendNo=0xEC; //��дEEPROM
    USHORT value = 0x1716; //��ȡ24LC64����ʼ��ַ

    SelectDeviceIndex(DeviceIndex,pcDriverName);//��ʼ���豸��
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=(unsigned char)(nEnable);

    bResult=VendRequest(hOutDevice,VendNo,value,bDir,outBuffer,m_nSize);
    CloseDevice(hOutDevice);

    return 1;
}

WORD SetDeviceIP(WORD DeviceIndex,ULONG nIP,ULONG nSubnetMask,ULONG nGateway,USHORT nPort,BYTE* pMac)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    HANDLE hOutDevice=NULL;
    UINT m_nSize=20;
    BOOL bResult=FALSE;
    BOOL bDir=DIR_OUT;
    UCHAR VendNo=0xEC; //��дEEPROM
    USHORT value = 0x1700; //��ȡ24LC64����ʼ��ַ

    SelectDeviceIndex(DeviceIndex,pcDriverName);//��ʼ���豸��
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR) malloc(m_nSize);
    //IP
    outBuffer[0]=(unsigned char)((nIP >> 16) & 0xFF);
    outBuffer[1]=(unsigned char)((nIP>>24) & 0xFF);
    outBuffer[2]=(unsigned char)(nIP & 0xFF);
    outBuffer[3]=(unsigned char)((nIP >> 8) & 0xFF);
    //����
    outBuffer[4]=(unsigned char)((nGateway >> 16) & 0xFF);
    outBuffer[5]=(unsigned char)((nGateway>>24) & 0xFF);
    outBuffer[6]=(unsigned char)(nGateway & 0xFF);
    outBuffer[7]=(unsigned char)((nGateway >> 8) & 0xFF);
    //��������
    outBuffer[8]=(unsigned char)((nSubnetMask >> 16) & 0xFF);
    outBuffer[9]=(unsigned char)((nSubnetMask>>24) & 0xFF);
    outBuffer[10]=(unsigned char)(nSubnetMask & 0xFF);
    outBuffer[11]=(unsigned char)((nSubnetMask >> 8) & 0xFF);
    //�˿�
    outBuffer[12]=(unsigned char)(nPort&0xff) ;
    outBuffer[13]=(unsigned char)((nPort>>8)&0xFF);
    //
    outBuffer[14]=pMac[1];
    outBuffer[15]=pMac[0];
    outBuffer[16]=pMac[3];
    outBuffer[17]=pMac[2];
    outBuffer[18]=pMac[5];
    outBuffer[19]=pMac[4];
    bResult=VendRequest(hOutDevice,VendNo,value,bDir,outBuffer,m_nSize);
    CloseDevice(hOutDevice);

    return 1;
}
//
WORD OpenLan(WORD DeviceIndex,WORD nMode)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    BULK_TRANSFER_CONTROL   outBulkControl;
    HANDLE hOutDevice=NULL;
    UINT m_nSize=2;

    SelectDeviceIndex(DeviceIndex,pcDriverName);	//��ʼ���豸��
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR) malloc(m_nSize);
    if(nMode == NET_LAN)
    {
        outBuffer[0] = 0x21;
    }
    else
    {
        outBuffer[0] = 0x28;
    }
    outBuffer[1]=0x00;
    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status =  DeviceIoControl (hOutDevice,
                               IOCTL_EZUSB_BULK_WRITE,
                               (PVOID)&outBulkControl,
                               sizeof(BULK_TRANSFER_CONTROL),
                               outBuffer,//���������
                               m_nSize,//�ֽ������ڶԻ����п�������
                               &BytesReturned,//�����ֽ�����,����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                               NULL);
#endif
    //�ر��豸
    free(outBuffer);
    CloseDevice(hOutDevice);
    return status;
}

WORD OpenWIFIPower(WORD DeviceIndex)//��WIFI��Դ,��Լ��Ҫ90S
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    BULK_TRANSFER_CONTROL   outBulkControl;
    HANDLE hOutDevice=NULL;
    UINT m_nSize=2;

    SelectDeviceIndex(DeviceIndex,pcDriverName);	//��ʼ���豸��
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0] = 0x29;
    outBuffer[1]=0x00;
    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status =  DeviceIoControl (hOutDevice,
                               IOCTL_EZUSB_BULK_WRITE,
                               (PVOID)&outBulkControl,
                               sizeof(BULK_TRANSFER_CONTROL),
                               outBuffer,//���������
                               m_nSize,//�ֽ������ڶԻ����п�������
                               &BytesReturned,//�����ֽ�����,����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                               NULL);
#endif
    //�ر��豸
    free(outBuffer);
    CloseDevice(hOutDevice);
    return status;
}

WORD ResetWIFI(WORD DeviceIndex)//��λWIFI
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    BULK_TRANSFER_CONTROL   outBulkControl;
    HANDLE hOutDevice=NULL;
    UINT m_nSize=2;

    SelectDeviceIndex(DeviceIndex,pcDriverName);	//��ʼ���豸��
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0] = 0x2A;
    outBuffer[1] = 0x00;
    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status =  DeviceIoControl (hOutDevice,
                               IOCTL_EZUSB_BULK_WRITE,
                               (PVOID)&outBulkControl,
                               sizeof(BULK_TRANSFER_CONTROL),
                               outBuffer,//���������
                               m_nSize,//�ֽ������ڶԻ����п�������
                               &BytesReturned,//�����ֽ�����,����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                               NULL);
#endif
    //�ر��豸
    free(outBuffer);
    CloseDevice(hOutDevice);
    return status;
}
//
WORD GetLANEnable(WORD DeviceIndex,short* pEnable)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_IN;
    UCHAR VendNo=0xED; //��дEEPROM����
    HANDLE hDevice;
    ULONG ncnt;
    PUCHAR inBuffer = NULL;

    ncnt=1;//1024*4;//��ȡ24LC64�ĳ��ȣ����4K
    inBuffer=(PUCHAR) malloc(ncnt);
    USHORT value = 0x1716; //��ȡ24LC64����ʼ��ַ
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice == NULL)
    {
        return 0;
    }
    bResult=VendRequest(hDevice,VendNo,value,bDir,inBuffer,ncnt);
    CloseDevice(hDevice);

    *pEnable = inBuffer[0];
    free(inBuffer);

    return 1;
}

//
WORD GetDeviceIP(WORD DeviceIndex,ULONG* pIP,ULONG* pSubMask,ULONG* pGateway,WORD* pPort,BYTE* pMac)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_IN;
    UCHAR VendNo=0xED; //��дEEPROM����
    HANDLE hDevice;
    ULONG ncnt;
    PUCHAR inBuffer = NULL;

    ncnt=20;//1024*4;//��ȡ24LC64�ĳ��ȣ����4K
    inBuffer=(PUCHAR) malloc(ncnt);
    USHORT value = 0x1700; //��ȡ24LC64����ʼ��ַ
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice == NULL)
    {
        return 0;
    }
    bResult=VendRequest(hDevice,VendNo,value,bDir,inBuffer,ncnt);
    CloseDevice(hDevice);
    *pIP = inBuffer[2] + (inBuffer[3]<<8) + (inBuffer[0]<<16) + (inBuffer[1]<<24);
    *pGateway = inBuffer[6] + (inBuffer[7]<<8) + (inBuffer[4]<<16) + (inBuffer[5]<<24);
    *pSubMask = inBuffer[10] + (inBuffer[11]<<8) + (inBuffer[8]<<16) + (inBuffer[9]<<24);
    *pPort = (inBuffer[13]<<8) + inBuffer[12];
    pMac[1] = inBuffer[14];
    pMac[0] = inBuffer[15];
    pMac[3] = inBuffer[16];
    pMac[2] = inBuffer[17];
    pMac[5] = inBuffer[18];
    pMac[4] = inBuffer[19];
    free(inBuffer);

    return 1;
}


WORD GetUSBModulVersion(WORD DeviceIndex)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_IN;
    UCHAR VendNo=0xF4; //��дEEPROM����
    HANDLE hDevice;
    ULONG ncnt;
    PUCHAR inBuffer = NULL;
    WORD ver = 0;

    ncnt=2;//��ȡ24LC64�ĳ��ȣ����4K
    inBuffer=(PUCHAR) malloc(ncnt);
    USHORT value = 0x1F00; //��ȡ24LC64����ʼ��ַ
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice == NULL)
    {
        return 0;
    }
    bResult=VendRequest(hDevice,VendNo,value,bDir,inBuffer,ncnt);
    CloseDevice(hDevice);
    ver =  inBuffer[1] + (inBuffer[0]<<8);
    free(inBuffer);

    return ver;
}
WORD	HDGetCHMode(WORD nCHset,WORD nTimeDiv)
{
	int nOpenedCH=0;
	for(int i=0;i<MAX_CH_NUM;i++)
	{
		if((nCHset>>i)&0x01)
		{
			nOpenedCH++;
		}	
	}
	if(nOpenedCH>=3)
		return 4;
	else if(nOpenedCH==2)
	{
		return nTimeDiv<=TIMEDIV_500MSA?2:4;
	}
	else if(nOpenedCH==1)
	{
		return nTimeDiv<=TIMEDIV_1GSA?1:nTimeDiv==TIMEDIV_500MSA?2:4;
	}
	return 0;

}
WORD        sendOutBufferNoReSet(WORD DeviceIndex,UINT m_nSize,PUCHAR pBuffer)  //������ ResetDevice(DeviceIndex, 1);
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    WORD status;
    HANDLE hOutDevice=NULL;
    ULONG BytesReturned=0;
    SelectDeviceIndex(DeviceIndex, pcDriverName);//��ʼ���豸��
    hOutDevice = OpenDevice(pcDriverName);
    if (hOutDevice == NULL)
        return 0;
    BULK_TRANSFER_CONTROL   outBulkControl;
    outBulkControl.pipeNum = 0;//�˵�ѡ��EP2
    status = DeviceIoControl(hOutDevice,
                             IOCTL_EZUSB_BULK_WRITE,
                             (PVOID)&outBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             pBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                             NULL);

    CloseDevice(hOutDevice);
    return status;
}
WORD        sendOutBuffer(WORD DeviceIndex,UINT m_nSize,PUCHAR pBuffer)  //ÿ�ε��� ResetDevice(DeviceIndex, 1);
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    WORD status;
    HANDLE hOutDevice=NULL;
    ULONG BytesReturned=0;
    SelectDeviceIndex(DeviceIndex, pcDriverName);//��ʼ���豸��
    ResetDevice(DeviceIndex, 1);
    hOutDevice = OpenDevice(pcDriverName);
    if (hOutDevice == NULL)
        return 0;
    BULK_TRANSFER_CONTROL   outBulkControl;
    outBulkControl.pipeNum = 0;//�˵�ѡ��EP2
    status = DeviceIoControl(hOutDevice,
                             IOCTL_EZUSB_BULK_WRITE,
                             (PVOID)&outBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             pBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                             NULL);

    CloseDevice(hOutDevice);
    return status;
}
WORD        getInBuffer(WORD DeviceIndex,UINT m_nSize,PUCHAR pBuffer)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL status;
    HANDLE hInDevice=NULL;
    ULONG BytesReturned=0;
    BULK_TRANSFER_CONTROL  inBulkControl;
    SelectDeviceIndex(DeviceIndex, pcDriverName);//��ʼ���豸��
    hInDevice = OpenDevice(pcDriverName);
    if(hInDevice == NULL)
    {
        return 0;
    }
    memset(pBuffer, 0,m_nSize);
    inBulkControl.pipeNum = 1;
    //��ʼʱ��
    status = DeviceIoControl(hInDevice,
                             IOCTL_EZUSB_BULK_READ,
                             (PVOID)&inBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             (LPVOID)pBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                             NULL);
    //�ر��豸
    CloseDevice(hInDevice);
    return status;
}
WORD        getInBufferWithoutOpen(HANDLE hInDevice,UINT m_nSize,PUCHAR pBuffer)
{

    if(hInDevice == NULL)
        return 0;
    BOOL status=FALSE;
    ULONG BytesReturned=0;
    BULK_TRANSFER_CONTROL inBulkControl;
    memset(pBuffer, 0,m_nSize);
    inBulkControl.pipeNum = 1;
    status = DeviceIoControl(hInDevice,
                             IOCTL_EZUSB_BULK_READ,
                             (PVOID)&inBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             (LPVOID)pBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                             NULL);
    return status;
}

WORD        GetSPIState(WORD DeviceIndex)
{
    return 0x0F;  //delete
    /*
    PUCHAR outBuffer = NULL;
    UINT m_nSize=4;
    BOOLEAN status=FALSE;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x7F;
    outBuffer[1]=0x01;
    outBuffer[2]=0x0D;
    outBuffer[3]=0x00;

    sendOutBuffer(DeviceIndex,m_nSize,outBuffer);

    free(outBuffer);



    ULONG ReValue;
    PUCHAR inBuffer=NULL;
    m_nSize =PCUSBSpeed(DeviceIndex)?USB_PACK_SIZE:64;
    inBuffer=(PUCHAR)malloc(m_nSize);
    memset(inBuffer, 0,m_nSize);
    if(getInBuffer(DeviceIndex,m_nSize,inBuffer))
        ReValue=inBuffer[0]&0x0F;
    else
        ReValue=0;
    return ReValue;
    */
}
WORD        SetAboutInputAdc_clock(WORD DeviceIndex) //ʱ������by zhang 20150914
{    //	MessageBox(NULL,LPCTSTR(completeDeviceName),"0C00",MB_OK);
    PUCHAR outBuffer = NULL;
    UINT m_nSize=8;
    WORD status=FALSE;
    ULONG nNCounter,nControl,nRCounter;
    //if(nTimeDIV<=5)
    if(0)
    {
	
        nNCounter   =0x013822;
        nControl    =0x4FF12C;        
        nRCounter   =0x3000C9;
		
		/*
		
		nNCounter   =0x01341E;
        nControl    =0x4FF12C;        
        nRCounter   =0x3000C9;
		*/
		

    }
    else
    {/*
        nNCounter   =0x00FA02;
        nControl    =0x4FF10C;
        nRCounter   =0x3000C9;
		*/

		/*
		nNCounter   =0x027102;
        nControl    =0x0FF128;//0x4FF12C;
        nRCounter   =0x2200C9;
		*/
		
		
		nNCounter   =0x013812;
        nControl   =0x0FF128;//0x4FF12C;
        nRCounter   =0x300065;
		
		
    }
    outBuffer=(PUCHAR) malloc(m_nSize);

    outBuffer[0]=0x08;
    outBuffer[1]=0x00;
    outBuffer[2]=0x00;
	outBuffer[6]=0x02;
    outBuffer[7]=0x00;             //PLL


    outBuffer[3]=0xff&(unsigned char)(nRCounter>> 0) ;
    outBuffer[4]=0xff&(unsigned char)(nRCounter>> 8) ;
    outBuffer[5]=0xff&(unsigned char)(nRCounter>>16) ;

    Sleep(2);
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);

    outBuffer[3]=0xff&(unsigned char)(nControl >> 0) ;
    outBuffer[4]=0xff&(unsigned char)(nControl >> 8) ;
    outBuffer[5]=0xff&(unsigned char)(nControl >>16) ;

    Sleep(2);
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);

    outBuffer[3]=0xff&(unsigned char)(nNCounter>> 0) ;
    outBuffer[4]=0xff&(unsigned char)(nNCounter>> 8) ;
    outBuffer[5]=0xff&(unsigned char)(nNCounter>>16) ;

    Sleep(15);
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);   //need modified //Error
    return status;
}
WORD        setInitAdcOnce(WORD nDeviceIndex)
{

    BOOL status = FALSE;
    UINT m_nSize;
    m_nSize = 8;
    PUCHAR outBuffer=NULL;
    outBuffer = (PUCHAR)malloc(m_nSize);
    outBuffer[0]=0x08;
    outBuffer[1]=0x00;
    outBuffer[2]=0x00;
    outBuffer[3]=0xff&(unsigned char)(0x4777>> 0) ;
    outBuffer[4]=0xff&(unsigned char)(0x4777>> 8) ;
    outBuffer[5]=0x12;    //
    outBuffer[6]=0x04;          //ADC
    outBuffer[7]=0x00;
    status=sendOutBuffer(nDeviceIndex,m_nSize,outBuffer);
    Sleep(2);
    outBuffer[0]=0x08;
    outBuffer[1]=0x00;
    outBuffer[2]=0x00;
    outBuffer[3]=0xff&(unsigned char)(0x03>> 0) ;
    outBuffer[4]=0x00 ;
    outBuffer[5]=0x33;    //
    outBuffer[6]=0x04;          //ADC
    outBuffer[7]=0x00;
    status=sendOutBuffer(nDeviceIndex,m_nSize,outBuffer);
	/*Sleep(2);
	outBuffer[0]=0x08;
    outBuffer[1]=0x00;
    outBuffer[2]=0x00;
    outBuffer[3]=0xff&(unsigned char)(0x0444>> 0) ;
    outBuffer[4]=0xff&(unsigned char)(0x0444>> 8) ;
    outBuffer[5]=0x11;    //
    outBuffer[6]=0x04;          //ADC
    outBuffer[7]=0x00;
    status=sendOutBuffer(nDeviceIndex,m_nSize,outBuffer);
	*/


    /*
    outBuffer[0]=0x08;
    outBuffer[1]=0x00;
    outBuffer[2]=0x00;
    outBuffer[3]=0xff&(unsigned char)(0x40) ;  //��ݲ�
    outBuffer[4]=0x00 ;
    outBuffer[5]=0x25;    //
    outBuffer[6]=0x04;          //ADC
    outBuffer[7]=0x00;
    status=sendOutBuffer(nDeviceIndex,m_nSize,outBuffer);
	*/
        
    /*
    outBuffer[0]=0x08;
    outBuffer[1]=0x00;
    outBuffer[2]=0x00;
    outBuffer[3]=0x00;
    outBuffer[4]=0x00 ;
    outBuffer[5]=0x55;    //
    outBuffer[6]=0x04;          //ADC
    outBuffer[7]=0x00;
    status=sendOutBuffer(nDeviceIndex,m_nSize,outBuffer);
	*/

    SetAboutInputAdc_clock(nDeviceIndex);
    free(outBuffer);
    return status;
}

WORD SetOffsetChannel1(WORD DeviceIndex,WORD DacData)
{
    PUCHAR outBuffer = NULL;
    UINT m_nSize=4;
    BOOL status=FALSE;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x00;
    outBuffer[1]=0x00;
    outBuffer[2]=0xff&(DacData>>0);
    outBuffer[3]=0xff&(DacData>>8);
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
    return status;
}
WORD SetOffsetChannel2(WORD DeviceIndex,WORD DacData)
{
    PUCHAR outBuffer = NULL;
    UINT m_nSize=4;
    BOOL status=FALSE;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x01;
    outBuffer[1]=0x00;
    outBuffer[2]=0xff&(DacData>>0);
    outBuffer[3]=0xff&(DacData>>8);
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
    return status;
}
WORD SetOffsetChannel3(WORD DeviceIndex,WORD DacData)
{
    PUCHAR outBuffer = NULL;
    UINT m_nSize=4;
    BOOL status=FALSE;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x02;
    outBuffer[1]=0x00;
    outBuffer[2]=0xff&(DacData>>0);
    outBuffer[3]=0xff&(DacData>>8);
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
    return status;
}
WORD SetOffsetChannel4(WORD DeviceIndex,WORD DacData)
{
    PUCHAR outBuffer = NULL;
    UINT m_nSize=4;
    BOOL status=FALSE;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x04;
    outBuffer[1]=0x00;
    outBuffer[2]=0xff&(DacData>>0);
    outBuffer[3]=0xff&(DacData>>8);
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
    return status;
}

WORD		SetTrigLevel(WORD DeviceIndex,WORD nLevel_256,WORD nSetTrigLevel)
{
    PUCHAR outBuffer = NULL;
    UINT m_nSize=18;
    //compute channel address
    unsigned int nFactor=nSetTrigLevel;
    BOOL status=FALSE;
    //compute channel address over
    short nBigger;
    short nSmaller;
	//nLevel_256=WORD(100.0+(nLevel_256-128)*199.0/255.0);
	//nLevel_256=WORD(nLevel_256*199.0/255.0+28+0.5);
   // nLevel_256=nLevel_256<0     ?0x00:nLevel_256;
   // nLevel_256=nLevel_256>227  ?227:nLevel_256;
	nLevel_256=WORD((nLevel_256)*200/256.0+28.5);
    nBigger=nLevel_256+nFactor;
    nSmaller=nLevel_256-nFactor;
    nBigger=nBigger<0     ?0x00:nBigger;
    //nBigger=nBigger>200   ?200:nBigger;
    nSmaller=nSmaller<0     ?0x00:nSmaller;
    //nSmaller=nSmaller>200   ?0x200:nSmaller;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x07;
    outBuffer[1]=0x00;
    outBuffer[2]=0xFF&(unsigned short)(nBigger);
    outBuffer[3]=0xFF&(unsigned short)(nBigger);
    outBuffer[4]=0xff&(unsigned short)(nSmaller);
    outBuffer[5]=0xff&(unsigned short)(nSmaller);
    outBuffer[6]=0xFF&(unsigned short)(nBigger);
    outBuffer[7]=0xFF&(unsigned short)(nBigger);
    outBuffer[8]=0xff&(unsigned short)(nSmaller);
    outBuffer[9]=0xff&(unsigned short)(nSmaller);
    outBuffer[10]=0xFF&(unsigned short)(nBigger);
    outBuffer[11]=0xFF&(unsigned short)(nBigger);
    outBuffer[12]=0xff&(unsigned short)(nSmaller);
    outBuffer[13]=0xff&(unsigned short)(nSmaller);
    outBuffer[14]=0xFF&(unsigned short)(nBigger);
    outBuffer[15]=0xFF&(unsigned short)(nBigger);
    outBuffer[16]=0xff&(unsigned short)(nSmaller);
    outBuffer[17]=0xff&(unsigned short)(nSmaller);
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
    return status;
}
/*
WORD SetTriggerLength(WORD DeviceIndex,ULONG nBufferLen,WORD nHTriggerPos,WORD nTimeDIV,WORD nCHSet)//���ô�������
{
    PUCHAR outBuffer = NULL;
    UINT m_nSize=14;
   // long trig_lenth,pre_trig_lenth;
    WORD status=FALSE;
	ULONG nPre,nTrg;
    __int64 nPreL,nTrgL;//nPreUp,nPreLow,nTrgUp,nTrgLow;
    ULONG nMaxLength = 0x10000;//64K
	int nOpenCHNum=0;
	double nDivNum=0;
	double dBufferLen=0;
	double dPreLen,dAfterLen;
	for(int i=0;i<MAX_CH_NUM;i++)
	{
		if((nCHSet>>i)&0x01)
		{
			nOpenCHNum++; 
		}
	}
	
	if(nOpenCHNum==1)
	{
		nDivNum=nTimeDIV<=5?0.125:(nTimeDIV==6?0.25:(nTimeDIV==7?0.5:1));
	}
	else if(nOpenCHNum==2)
	{
		nDivNum=nTimeDIV<=6?0.25:(nTimeDIV==7?0.5:1);
	}
	else if(nOpenCHNum==3||nOpenCHNum==4)
	{
		nDivNum=nTimeDIV<=7?0.5:1;
	}
	if (nDivNum==1)
	{
		switch (nTimeDIV) {
			case 8://2uS
			nDivNum=1;
			break;
			case 9://5uS
			nDivNum=2.5;
			break;
			case 10://10uS
			nDivNum=5;
			break;
			case 11://20uS
			nDivNum=10;
			break;
			case 12://50uS
			nDivNum=25;
			break;
			case 13://100uS
			nDivNum=50;
			break;
			case 14://200uS
			nDivNum=100;
			break;
			case 15://500uS
			nDivNum=250;
			break;
			case 16://1mS
			nDivNum=500;
			break;
			case 17://2mS
			nDivNum=1e3;
			break;
			case 18://5mS
			nDivNum=2.5e3;
			break;
			case 19://10mS
			nDivNum=5e3;
			break;
			case 20://20mS
			nDivNum=1e4;
			break;
			case 21://50mS
			nDivNum=2.5e4;
			break;
			case 22://100mS
			nDivNum=5e4;
			break;
			case 23://200mS
			nDivNum=1e5;
			break;
			case 24://500mS
			nDivNum=2.5e5;
			break;
			case 25://1S
			nDivNum=5e5;
			break;
			case 26://2S
			nDivNum=1e6;
			break;
			case 27://5S
			nDivNum=2.5e6;
			break;
			case 28://10S
			nDivNum=5e6;
			break;
			case 29://20S
			nDivNum=1e7;
			break;
			case 30://50S
			nDivNum=2.5e7;
			break;
			case 31://100S
			nDivNum=5e7;
			break;
			case 32://200S
			nDivNum=1e8;
			break;
			case 33://500S
			nDivNum=2.5e8;
			break;
			case 34://1000S
			nDivNum=5e8;
			break;
			default:
			nDivNum=1;
			break;	
			
		}
	}
	nBufferLen=nBufferLen>nMaxLength?nMaxLength:nBufferLen;

	if(nDivNum==1&&nTimeDIV>8)
	{
		nTrg = ULONG((100 - nHTriggerPos) * nBufferLen / 100.0);
		nPre = ULONG(nHTriggerPos * nBufferLen / 100.0);
		dPreLen=(nPre+100)*nDivNum/8;
		dAfterLen=nTrg *nDivNum/8;
		nPreL=(__int64)dPreLen;		
		nTrgL=(__int64)dAfterLen;
		
	}
	else
	{
		nBufferLen=ULONG(nBufferLen*nDivNum);
		nTrg = ULONG((100 - nHTriggerPos) * nBufferLen / 100.0);
		nPre = ULONG(nHTriggerPos * nBufferLen / 100.0);
		dPreLen=(nPre+100);
		dAfterLen=nTrg ;
		nPreL=(__int64)dPreLen;		
		nTrgL=(__int64)dAfterLen;
	}
    

    //trig_lenth=(long)(nMaxLength - nTrg);
    //pre_trig_lenth=(long)(nMaxLength - nPre);

	outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x10;
    outBuffer[1]=0x00;
    outBuffer[2]=0xff&(unsigned char)(nPreL>>0) ;
    outBuffer[3]=0xff&(unsigned char)(nPreL>>8) ;
    outBuffer[4]=0xff&(unsigned char)(nPreL>>16) ;
    outBuffer[5]=0xff&(unsigned char)(nPreL>>24) ;
	outBuffer[6]=0xff&(unsigned char)(nPreL>>32) ;
    outBuffer[7]=0xff&(unsigned char)(nPreL>>40) ;
    outBuffer[8]=0xff&(unsigned char)(nTrgL>>0) ;
    outBuffer[9]=0xff&(unsigned char)(nTrgL>>8) ;
    outBuffer[10]=0xff&(unsigned char)(nTrgL>>16) ;
    outBuffer[11]=0xff&(unsigned char)(nTrgL>>24) ;
	outBuffer[12]=0xff&(unsigned char)(nTrgL>>32) ;
    outBuffer[13]=0xff&(unsigned char)(nTrgL>>40) ;
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);



    free(outBuffer);   //need modified //Error

    return status;
}
*/
WORD SetTriggerLength(WORD DeviceIndex,ULONG nBufferLen,WORD nHTriggerPos,WORD nTimeDIV,WORD nCHMod)//���ô�������
{
//	nHTriggerPos=50;
    PUCHAR outBuffer = NULL;
    UINT m_nSize=14;
    WORD status=FALSE;
	ULONG nPre,nTrg;
	UINT nOffsetTime=0;
    __int64 nPreL,nTrgL;//nPreUp,nPreLow,nTrgUp,nTrgLow;
    ULONG nMaxLength = 0x10000;//64K
	//int nOpenCHNum=0;
	double dDivNum=0;
/*
	for(int i=0;i<MAX_CH_NUM;i++)
	{
		if((nCHSet>>i)&0x01)
		{
			nOpenCHNum++; 
		}
	}*/
	if(nTimeDIV<=TIMEDIV_500MSA)
	{
		dDivNum=nCHMod;

	}
	else
	{
		switch (nTimeDIV) {
			case TIMEDIV_250MSA ://1uS
			dDivNum=4;
			break;			
			case TIMEDIV_250MSA+1://2uS
			dDivNum=8;
			break;
			case TIMEDIV_250MSA+2://5uS
			dDivNum=2e1;
			break;
			case TIMEDIV_250MSA+3://10uS
			dDivNum=4e1;
			break;
			case TIMEDIV_250MSA+4://20uS
			dDivNum=8e1;
			break;
			case TIMEDIV_250MSA+5://50uS
			dDivNum=2e2;
			break;
			case TIMEDIV_250MSA+6://100uS
			dDivNum=4e2;
			break;
			case TIMEDIV_250MSA+7://200uS
			dDivNum=8e2;
			break;
			case TIMEDIV_250MSA+8://500uS
			dDivNum=2e3;
			break;
			case TIMEDIV_250MSA+9://1mS
			dDivNum=4e3;
			break;
			case TIMEDIV_250MSA+10://2mS
			dDivNum=8e3;
			break;
			case TIMEDIV_250MSA+11://5mS
			dDivNum=2e4;
			break;
			case TIMEDIV_250MSA+12://10mS
			dDivNum=4e4;
			break;
			case TIMEDIV_250MSA+13://20mS
			dDivNum=8e4;
			break;
			case TIMEDIV_250MSA+14://50mS
			dDivNum=2e5;
			break;
			case TIMEDIV_250MSA+15://100mS
			dDivNum=4e5;
			break;
			case TIMEDIV_250MSA+16://200mS
			dDivNum=8e5;
			break;
			case TIMEDIV_250MSA+17://500mS
			dDivNum=2e6;
			break;
			case TIMEDIV_250MSA+18://1S
			dDivNum=4e6;
			break;
			case TIMEDIV_250MSA+19://2S
			dDivNum=8e6;
			break;
			case TIMEDIV_250MSA+20://5S
			dDivNum=2e7;
			break;
			case TIMEDIV_250MSA+21://10S
			dDivNum=4e7;
			break;
			case TIMEDIV_250MSA+22://20S
			dDivNum=8e7;
			break;
			case TIMEDIV_250MSA+23://50S
			dDivNum=2e8;
			break;
			case TIMEDIV_250MSA+24://100S
			dDivNum=4e8;
			break;
			case TIMEDIV_250MSA+25://200S
			dDivNum=8e8;
			break;
			case TIMEDIV_250MSA+26://500S
			dDivNum=2e9;
			break;
			case TIMEDIV_250MSA+27://1000S
			dDivNum=4e9;
			break;
			default:
			dDivNum=1;
			break;
			
		}
	}
	nBufferLen=nBufferLen>nMaxLength?nMaxLength:nBufferLen;
	
	nPre = ULONG(nHTriggerPos * nBufferLen / 100.0);
	nTrg = nBufferLen-nPre+8;	
	nPre=nPre+100;
	nPreL=__int64((nPre*dDivNum)/8);	
	nTrgL=__int64(nTrg*dDivNum/8);

	
	
	if(dDivNum<200)
	{
		if(nPre*dDivNum/8-int(nPre*dDivNum/8)!=0)
			nPreL++;	
		if(nTrg*dDivNum/8-int(nTrg*dDivNum/8)!=0)
			nTrgL++;
	}

	outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x10;
    outBuffer[1]=0x00;
    outBuffer[2]=0xff&(unsigned char)(nPreL>>0) ;
    outBuffer[3]=0xff&(unsigned char)(nPreL>>8) ;
    outBuffer[4]=0xff&(unsigned char)(nPreL>>16) ;
    outBuffer[5]=0xff&(unsigned char)(nPreL>>24) ;
	outBuffer[6]=0xff&(unsigned char)(nPreL>>32) ;
    outBuffer[7]=0xff&(unsigned char)(nPreL>>40) ;
	
    outBuffer[8]=0xff&(unsigned char)(nTrgL>>0) ;
    outBuffer[9]=0xff&(unsigned char)(nTrgL>>8) ;
    outBuffer[10]=0xff&(unsigned char)(nTrgL>>16) ;
    outBuffer[11]=0xff&(unsigned char)(nTrgL>>24) ;
	outBuffer[12]=0xff&(unsigned char)(nTrgL>>32) ;
    outBuffer[13]=0xff&(unsigned char)(nTrgL>>40) ;
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);   //need modified //Error
    return status;
}
WORD SetRamLength(WORD DeviceIndex,WORD nBufferSize)//
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    BULK_TRANSFER_CONTROL   outBulkControl;
    HANDLE hOutDevice=NULL;
    UINT m_nSize=4;

    SelectDeviceIndex(DeviceIndex,pcDriverName);	//��ʼ���豸��
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x0D;
    outBuffer[1]=0x0F;//NOUSE 16bit usb bus
    outBuffer[2]=0xff&(nBufferSize&0x07);
    outBuffer[3]=0x00;
    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status =  DeviceIoControl (hOutDevice,
                               IOCTL_EZUSB_BULK_WRITE,
                               (PVOID)&outBulkControl,
                               sizeof(BULK_TRANSFER_CONTROL),
                               outBuffer,//���������
                               m_nSize,//�ֽ������ڶԻ����п�������
                               &BytesReturned,//�����ֽ�����
                               NULL);
#endif
    free(outBuffer);	//�ر��豸
    CloseDevice(hOutDevice);

    return status;
}

WORD SetSampleRateBak(WORD DeviceIndex,WORD nTimeDIV,WORD nYTFormat)   //0912
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    BULK_TRANSFER_CONTROL   outBulkControl;
    HANDLE hOutDevice=NULL;
    SelectDeviceIndex(DeviceIndex,pcDriverName);//��ʼ���豸��
    UINT m_nSize=12;
    long Fenpin;
    WORD M_50M,M_125M,Clk_Alt;

    switch (nTimeDIV)
    {
    case 0:		//5ns -- 100MSa/s
        M_50M=0;
        M_125M=0;
        Clk_Alt=1;
        Fenpin=0xFFFFFFFF;// �����壬�����븳��һ�����������򻻵�ʱ���ٶȱ���
        break;
    case 1:		//10ns -- 100MSa/s
        M_50M=0;
        M_125M=0;
        Clk_Alt=1;
        Fenpin=0xFFFFFFFF;// �����壬�����븳��һ������
        break;
    case 2:		//20ns -- 100MSa/s
        M_50M=0;
        M_125M=0;
        Clk_Alt=1;
        Fenpin=0xFFFFFFFF;// �����壬�����븳��һ������
        break;
    case 3:		//50ns -- 100MSa/s
        M_50M=0;
        M_125M=0;
        Clk_Alt=1;
        Fenpin=0xFFFFFFFF;// �����壬�����븳��һ������
        break;
    case 4:		//100ns -- 100MSa/s
        M_50M=0;
        M_125M=0;
        Clk_Alt=1;
        Fenpin=0xFFFFFFFF;// �����壬�����븳��һ������
        break;
    case 5:		//200ns -- 100MSa/s
        M_50M=0;
        M_125M=0;
        Clk_Alt=1;
        Fenpin=0xFFFFFFFF;// �����壬�����븳��һ������
        break;
    case 6:		//500ns -- 100MSa/s
        M_50M=0;
        M_125M=0;
        Clk_Alt=1;
        Fenpin=0xFFFFFFFF;// �����壬�����븳��һ������
        break;
    case 7:		//1us -- 100MSa/s
        M_50M=0;
        M_125M=0;
        Clk_Alt=1;
        Fenpin=0xFFFFFFFF;// �����壬�����븳��һ������
        break;
    case 8:		//2us -- 100MSa/s
        M_50M=0;
        M_125M=0;
        Clk_Alt=1;
        Fenpin=0xFFFFFFFF;// �����壬�����븳��һ������
        break;
    case 9:		//5us -- 100MSa/s
        M_50M=0;
        M_125M=0;
        Clk_Alt=1;
        Fenpin=0xFFFFFFFF;// �����壬�����븳��һ������
        break;
    case 10:	//10us -- 100MSa/s
        M_50M=0;
        M_125M=0;
        Clk_Alt=1;
        Fenpin=0xFFFFFFFF;// �����壬�����븳��һ������
        break;
    case 11:	//20us -- 50MSa/s.....OK yt_20091215
        Fenpin=0xFFFFFFFF;
        M_50M=0;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 12:	//50us -- 20MSa/s.....ok yt_20091215
        Fenpin=0xFFFFFFFC;
        M_50M=1;
        M_125M=0;
        Clk_Alt=1;
        break;
    case 13:	//100us -- 10MSa/s.....ok yt_20091215
        Fenpin=0xFFFFFFF7;
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 14:	//200us -- 5MSa/s.....ok yt_20091215
        Fenpin=0xFFFFFFED;
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 15:	//500us -- 2MSa/s.....ok yt_20091215
        Fenpin=0xFFFFFFCF;
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 16:	//1ms -- 1MSa/s.....ok yt_20091215
        Fenpin=0xFFFFFF9D;
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 17:	//2ms -- 500kSa/s.....ok yt_20091215
        Fenpin=0xFFFFFF39;
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 18:	//5ms -- 200kSa/s.....ok yt_20091215
        Fenpin=0xFFFFFE0D;
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 19:	//10ms -- 100kSa/s.....ok yt_20091215
        Fenpin=0xFFFFFC19;
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 20:	//20ms -- 50kSa/s.....ok yt_20091215
        Fenpin=0xFFFFF831;
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 21:	//50ms -- 20kSa/s.....ok yt_20091215
        Fenpin=0xFFFFEC79;
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 22:	//100ms -- 10kSa/s.....ok yt_20091215 ---Scan
        Fenpin= 0xFFFFD8F1;	//10kSa/s
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 23:	//200ms -- 5kSa/s.....ok yt_20091215
        Fenpin= 0xFFFFB1E1;	//5kSa/s
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 24:	//500ms -- 2kSa/s.....ok yt_20091215
        if(nYTFormat == YT_NORMAL)
        {
            Fenpin= 0xFFFF3CB1;	//2kSa/s
        }
        else
        {
            Fenpin= 0xFFFFCF2D;	//8kSa/s	//��ʼ(���������4����ÿ�ɼ�128����������32������(ÿ8������1�����ֵ��1����Сֵ))
        }
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 25:	//1s -- 1kSa/s.....ok yt_20091215
        if(nYTFormat == YT_NORMAL)
        {
            Fenpin= 0xFFFE7962;	//1kSa/s
        }
        else
        {
            Fenpin= 0xFFFF9E59;	//4kSa/s
        }
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 26:	//2s -- 500Sa/s.....ok yt_20091215
        if(nYTFormat == YT_NORMAL)
        {
            Fenpin= 0xFFFCF2BF;	//500Sa/s
        }
        else
        {
            Fenpin= 0xFFFF3CB1;	//2kSa/s
        }
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 27:	//5s -- 200Sa/s.....ok yt_20091215
        if(nYTFormat == YT_NORMAL)
        {
            Fenpin= 0xFFF85EDF;	//200Sa/s
        }
        else
        {
            Fenpin= 0xFFFE17BA;	// 800Sa/s
        }
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 28:	//10s -- 100Sa/s.....ok yt_20091215
        if(nYTFormat == YT_NORMAL)
        {
            Fenpin= 0xFFF0BDBA;	//100Sa/s
        }
        else
        {
            Fenpin= 0xFFFC2F6F;	//400Sa/s
        }
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 29:	//20s -- 50Sa/s.....ok yt_20091215
        if(nYTFormat == YT_NORMAL)
        {
            Fenpin = 0xFFE17B70;	//50Sa/s
        }
        else
        {
            Fenpin = 0xFFF85EDF;//200Sa/s
        }
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 30:	//50s -- 20Sa/s.....ok yt_20091215
        if(nYTFormat == YT_NORMAL)
        {
            Fenpin= 0xFFB3B49C;	//20Sa/s
        }
        else
        {
            Fenpin= 0xFFECED3E;	//80Sa/s
        }
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 31:	//100s -- 10Sa/s.....ok yt_20091215
        if(nYTFormat == YT_NORMAL)
        {
            Fenpin= 0xFF678490;	//10Sa/s
        }
        else
        {
            Fenpin= 0xFFD9DA53;	//40Sa/s
        }
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 32:	//200s -- 5Sa/s.....ok yt_20091215
        if(nYTFormat == YT_NORMAL)
        {
            Fenpin= 0xFEA00000;	//5Sa/s--δȷ��
        }
        else
        {
            Fenpin= 0xFF678490;	//20Sa/s
        }
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 33:	//500s -- 2Sa/s.....ok yt_20091215
        if(nYTFormat == YT_NORMAL)
        {
            Fenpin= 0xFD07A4A0;	//2Sa/s
        }
        else
        {
            Fenpin= 0xFF1FEFBE;	//8Sa/s
        }
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 34:	//1000s -- 1Sa/s.....ok yt_20091215
        if(nYTFormat == YT_NORMAL)
        {
            Fenpin= 0xFEA00000;	//1Sa/s--δȷ��
        }
        else
        {
            Fenpin= 0xFEA00000;	//4Sa/s--δȷ��
        }
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 35:
        Fenpin=0xFD07A4A0;//
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 36:
        Fenpin=0xFD07A4A0;//
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    case 37:
        Fenpin=0xFD07A4A0;//
        M_50M=1;
        M_125M=1;
        Clk_Alt=1;
        break;
    default:
        break;
    }
    //
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x0E;
    outBuffer[1]=0x00;
    outBuffer[2]=0xff&((M_125M&0x01)|((M_50M&0x01)<<1));
    outBuffer[3]=0x00;
    outBuffer[4]=0xff&(unsigned char)Fenpin;
    outBuffer[5]=0xff&(unsigned char)(Fenpin>>8);
    outBuffer[6]=0xff&(unsigned char)(Fenpin>>16);
    outBuffer[7]=0xff&(unsigned char)(Fenpin>>24);
    outBuffer[8]=0x00;
    outBuffer[9]=0x00;
    outBuffer[10]=0x00;
    outBuffer[11]=0x00;

    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status = DeviceIoControl(hOutDevice,
                             IOCTL_EZUSB_BULK_WRITE,
                             (PVOID)&outBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             outBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                             NULL);
#endif

    //	if(nTimeDIV>24)
    //	{
    //	   	ResetDevice(DeviceIndex,4);
    //  }
    //�ر��豸
    free(outBuffer);
    CloseDevice(hOutDevice);
    return status;
}
WORD        SetSampleRate_DSO6104(WORD DeviceIndex, WORD nTimeDIV,WORD nYTFormat)//����ʾ�����Ĳ�����
{
    long Fenpin=1;
	if(nTimeDIV>TIMEDIV_250MSA+1)
	{
		switch (nTimeDIV)
		{

		case TIMEDIV_250MSA+2:		//5us ��ͨ��˫ͨ��4ͨ�� 20MSa/s
			Fenpin = 2;
			break;
		case TIMEDIV_250MSA+3:	//10us ��ͨ��˫ͨ��4ͨ�� 10MSa/s
			Fenpin = 5;
			break;
		case TIMEDIV_250MSA+4:	//20us ��ͨ��˫ͨ��4ͨ�� 5MSa/s
			Fenpin = 10;
			break;
		case TIMEDIV_250MSA+5:	//50us ��ͨ��˫ͨ��4ͨ�� 2MSa/s
			Fenpin = 25;
			break;
		case TIMEDIV_250MSA+6:	//100us ��ͨ��˫ͨ��4ͨ�� 1MSa/s
			Fenpin = 50;
			break;
		case TIMEDIV_250MSA+7:	//200us ��ͨ��˫ͨ��4ͨ�� 500KSa/s
			Fenpin = 100;
			break;
		case TIMEDIV_250MSA+8:	//500us ��ͨ��˫ͨ��4ͨ�� 200KSa/s
			Fenpin = 250;
			break;
		case TIMEDIV_250MSA+9:	//1ms ��ͨ��˫ͨ��4ͨ�� 100KSa/s
			Fenpin = 500;
			break;
		case TIMEDIV_250MSA+10:	//2ms ��ͨ��˫ͨ��4ͨ�� 50KSa/s
			Fenpin = 1000;
			break;
		case TIMEDIV_250MSA+11:	//5ms ��ͨ��˫ͨ��4ͨ�� 20KSa/s
			Fenpin = 2500;
			break;
		case TIMEDIV_250MSA+12:	//10ms ��ͨ��˫ͨ��4ͨ�� 10KSa/s
			Fenpin = 5000;
			break;
		case TIMEDIV_250MSA+13:	//20ms ��ͨ��˫ͨ��4ͨ�� 5KSa/s
			Fenpin = 10000;
			break;
		case TIMEDIV_250MSA+14:	//50ms ��ͨ��˫ͨ��4ͨ�� 2KSa/s
			Fenpin = 25000;
			break;
		case TIMEDIV_250MSA+15:	//100ms ��ͨ��˫ͨ��4ͨ�� 1KSa/s
			Fenpin = 50000;
			break;
		case TIMEDIV_250MSA+16:	//200ms ��ͨ��˫ͨ��4ͨ�� 500Sa/s
			Fenpin = 100000;
			break;
		case TIMEDIV_250MSA+17:	//500ms ��ͨ��˫ͨ��4ͨ�� 200Sa/s
			Fenpin = 250000;
			break;
		case TIMEDIV_250MSA+18:	//1s ��ͨ��˫ͨ��4ͨ�� 100Sa/s
			Fenpin = 500000;
			break;
		case TIMEDIV_250MSA+19:	//2s ��ͨ��˫ͨ��4ͨ�� 50Sa/s
			Fenpin = 1000000;
			break;
		case TIMEDIV_250MSA+20:	//5s ��ͨ��˫ͨ��4ͨ�� 20Sa/s
			Fenpin = 2500000;
			break;
		case TIMEDIV_250MSA+21:	//10s ��ͨ��˫ͨ��4ͨ�� 10Sa/s
			Fenpin = 5000000;
			break;
		case TIMEDIV_250MSA+22:	//20s ��ͨ��˫ͨ��4ͨ�� 5Sa/s
			Fenpin = 10000000;
			break;
		case TIMEDIV_250MSA+23:	//50s ��ͨ��˫ͨ��4ͨ�� 2Sa/s
			Fenpin = 25000000;
			break;
		case TIMEDIV_250MSA+24:	//100s ��ͨ��˫ͨ��4ͨ�� 1Sa/s
			Fenpin = 50000000;
			break;
		case TIMEDIV_250MSA+25:	//200s ��ͨ��˫ͨ��4ͨ�� 0.5Sa/s
			Fenpin = 100000000;
			break;
		case TIMEDIV_250MSA+26:	//500s ��ͨ��˫ͨ��4ͨ�� 0.2Sa/s
			Fenpin = 250000000;
			break;
		case TIMEDIV_250MSA+27:	//1000s ��ͨ��˫ͨ��4ͨ�� 0.1Sa/s
			Fenpin = 500000000;
		case TIMEDIV_250MSA+28:	//2000s ��ͨ��˫ͨ��4ͨ�� 0.1Sa/s
			Fenpin = 500000000;
		default:
			break;
		}
	}
    //
	Fenpin--;
    PUCHAR outBuffer = NULL;
    UINT m_nSize = 6;
    BOOL status=FALSE;
    outBuffer = (PUCHAR)malloc(m_nSize);
    outBuffer[0] = 0x0F;
    outBuffer[1] = 0x00;
    outBuffer[2] = 0xff & (unsigned char)Fenpin;
    outBuffer[3] = 0xff & (unsigned char)(Fenpin>>8);
    outBuffer[4] = 0xff & (unsigned char)(Fenpin>>16);
    outBuffer[5] = 0xff & (unsigned char)(Fenpin >>24);
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
    return status;
}


WORD SetFireSampleRate40ms(WORD DeviceIndex,WORD nTimeDIV,WORD nFireMode)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    BULK_TRANSFER_CONTROL   outBulkControl;
    HANDLE hOutDevice=NULL;
    UINT m_nSize=12;
    long Fenpin;
    WORD M_50M,M_125M,Clk_Alt;

    if(nFireMode != 2)
    {
        return 0;
    }
    Fenpin=0xFFFFFFFF;//20MSa/s
    M_50M=1;
    M_125M=0;
    Clk_Alt=1;

    SelectDeviceIndex(DeviceIndex,pcDriverName);//��ʼ���豸��
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x0E;
    outBuffer[1]=0x00;
    outBuffer[2]=0xff&((M_125M&0x01)|((M_50M&0x01)<<1));
    outBuffer[3]=0x00;
    outBuffer[4]=0xff&(unsigned char)Fenpin;
    outBuffer[5]=0xff&(unsigned char)(Fenpin>>8);
    outBuffer[6]=0xff&(unsigned char)(Fenpin>>16);
    outBuffer[7]=0xff&(unsigned char)(Fenpin>>24);
    outBuffer[8]=0x00;
    outBuffer[9]=0x00;
    outBuffer[10]=0x00;
    outBuffer[11]=0x00;

    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status = DeviceIoControl(hOutDevice,
                             IOCTL_EZUSB_BULK_WRITE,
                             (PVOID)&outBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             outBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                             NULL);
#endif

    //	if(nTimeDIV>24)
    //	{
    //	   	ResetDevice(DeviceIndex,4);
    //   }
    //�ر��豸
    free(outBuffer);
    CloseDevice(hOutDevice);
    return status;
}

WORD        setAmpCalibrate(WORD nDeviceIndex,WORD nCHSet,WORD nTimeDIV,WORD *pLevel,WORD *nVoltDiv,WORD *pCHPos)
{
    WORD nCHmod;
	nCHmod=HDGetCHMode(nCHSet,nTimeDIV);
	if(nCHmod==0)
		return 0;
    WORD nCH[MAX_CH_NUM];
    WORD nVoltDiv_Temp[MAX_CH_NUM];
	WORD nCHPosTemp[MAX_CH_NUM];
	ULONG nCHOffset[MAX_CH_NUM];
    memset(nCH,0,MAX_CH_NUM*sizeof(WORD));
    int i;
#ifdef DSO_6000_12DIV
    for(i=0;i<MAX_CH_NUM;i++)
    {
		nVoltDiv_Temp[i]=nVoltDiv[i];
		switch (nVoltDiv[i]){
		case 0: //2mV
		case 1: //5mV
		case 2: //10mV
		case 3: //20mV
		case 4: //50mV
			nVoltDiv_Temp[i]=4;
			break;
		case 5: //100mV
		case 6: //200mV
			nVoltDiv_Temp[i]=6;
			break;
		case 7: //500mV
		case 8: //1V
		case 9: //2V
			nVoltDiv_Temp[i]=9;
			break;
		case 10: //5V
		case 11: //10V
			nVoltDiv_Temp[i]=11;
			break;
		default:
			break;
		}
        
    }
#elif defined D10_100DELAY
	for(i=0;i<MAX_CH_NUM;i++)
    {
		nVoltDiv_Temp[i]=nVoltDiv[i];
		switch (nVoltDiv[i]){
		case 0: //2mV
		case 1: //5mV
		case 2: //10mV
		case 3: //20mV
		case 4: //50mV
		case 5: //100mV
			nVoltDiv_Temp[i]=5;
			break;
	
		case 6: //200mV
			
		case 7: //500mV
		case 8: //1V
			nVoltDiv_Temp[i]=7;
			break;
		case 9: //2V
		case 10: //5V
		case 11: //10V
			nVoltDiv_Temp[i]=9;
			break;
		default:
			break;
		}
        
    }
#endif
    switch (nCHmod) {
    case 4:
        for(i=0;i<MAX_CH_NUM;i++)
        {
            nCH[i]=pLevel[i*AMPCALI_PER_CH_Len+nVoltDiv_Temp[i]*AMPCALI_PER_VOLT_Len+2];
			nCHPosTemp[i]=pCHPos[i];

        }
		/*
		#ifdef D10_100DELAY
			for(i=0;i<MAX_CH_NUM;i++){
				if(nVoltDiv[i]==0){
					nCH[i]=nCH[i]*2;
				}
				else if(nVoltDiv[i]==1){
					nCH[i]=WORD(nCH[i]*1.25+0.5);
				}
			}
		#endif	
		*/
        break;
    case 2:
        for(i=0;i<MAX_CH_NUM;i++)
        {
            if((nCHSet>>i)&0x01)
            {
                nCH[0]=pLevel[i*AMPCALI_PER_CH_Len+nVoltDiv_Temp[i]*AMPCALI_PER_VOLT_Len+1];
                nCH[1]=pLevel[i*AMPCALI_PER_CH_Len+nVoltDiv_Temp[i]*AMPCALI_PER_VOLT_Len+1];
                nCH[2]=pLevel[i*AMPCALI_PER_CH_Len+nVoltDiv_Temp[i]*AMPCALI_PER_VOLT_Len+1];
                nCH[3]=pLevel[i*AMPCALI_PER_CH_Len+nVoltDiv_Temp[i]*AMPCALI_PER_VOLT_Len+1];
				nCHPosTemp[0]=pCHPos[i];
				nCHPosTemp[1]=pCHPos[i];
				nCHPosTemp[2]=pCHPos[i];
				nCHPosTemp[3]=pCHPos[i];
				i++;
                break;
            }
        }
		 for(;i<MAX_CH_NUM;i++)
        {
            if((nCHSet>>i)&0x01)
            {
				nCHPosTemp[2]=pCHPos[i];
				nCHPosTemp[3]=pCHPos[i];
                break;
            }
        }
        break;
    case 1:
        for(i=0;i<MAX_CH_NUM;i++)
        {
            if((nCHSet>>i)&0x01)
            {
                nCH[0]=pLevel[i*AMPCALI_PER_CH_Len+nVoltDiv_Temp[i]*AMPCALI_PER_VOLT_Len];
                nCH[1]=pLevel[i*AMPCALI_PER_CH_Len+nVoltDiv_Temp[i]*AMPCALI_PER_VOLT_Len];
                nCH[2]=pLevel[i*AMPCALI_PER_CH_Len+nVoltDiv_Temp[i]*AMPCALI_PER_VOLT_Len];
                nCH[3]=pLevel[i*AMPCALI_PER_CH_Len+nVoltDiv_Temp[i]*AMPCALI_PER_VOLT_Len];
				nCHPosTemp[0]=pCHPos[i];
				nCHPosTemp[1]=pCHPos[i];
				nCHPosTemp[2]=pCHPos[i];
				nCHPosTemp[3]=pCHPos[i];
                break;
            }
        }
        break;
    default:
        break;
    }
	for(i=0;i<MAX_CH_NUM;i++){
		nCHPosTemp[i]=WORD((nCHPosTemp[i]-128)*200.0/256.0+128.5);
	}
    PUCHAR outBuffer = NULL;
    UINT m_nSize=26;
    BOOL status=FALSE;
    outBuffer=(PUCHAR) malloc(m_nSize);
    memset(outBuffer,0,m_nSize);
    outBuffer[0]=0x1E;
    outBuffer[1]=0x00;
    outBuffer[2]=0xff&(unsigned char)(0xFF&nCH[0]);
    outBuffer[3]=0xff&(unsigned char)(0xFF&(nCH[0]>>8)) ;
    outBuffer[4]=0xff&(unsigned char)(0xFF&nCH[1]);
    outBuffer[5]=0xff&(unsigned char)(0xFF&(nCH[1]>>8)) ;
    outBuffer[6]=0xff&(unsigned char)(0xFF&nCH[2]);
    outBuffer[7]=0xff&(unsigned char)(0xFF&(nCH[2]>>8)) ;
    outBuffer[8]=0xff&(unsigned char)(0xFF&nCH[3]);
    outBuffer[9]=0xff&(unsigned char)(0xFF&(nCH[3]>>8)) ;
	for(i=0;i<MAX_CH_NUM;i++)
	{
		if(nCH[i]==AMPCALI_DEGREE)
			 continue;		
		int x=int((1-nCH[i]*1.0/AMPCALI_DEGREE)*nCHPosTemp[i]*AMPCALI_DEGREE*1.0+0.5);
		nCHOffset[i]=x<0?x+0x2000000:x;
		//nCHOffset[i]=168;
		outBuffer[9+4*i+1]=0xff&(unsigned char)(0xFF&nCHOffset[i]);
		outBuffer[9+4*i+2]=0xff&(unsigned char)(0xFF&(nCHOffset[i]>>8)) ;
		outBuffer[9+4*i+3]=0xff&(unsigned char)(0xFF&(nCHOffset[i]>>16));
		outBuffer[9+4*i+4]=0xff&(unsigned char)(0xFF&(nCHOffset[i]>>24)) ;
	}
    status=sendOutBuffer(nDeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
    return status;
}
WORD        SetAboutInputAdc_Only_DSO6104(WORD DeviceIndex, PRELAYCONTROL RelayControl, WORD nTimeDIV)
{
    BOOL OnOffCh[4];
    unsigned char OnOffDso;//ʾ�����ܵ�ͨ��״̬
    unsigned char GainCh[4];//ʾ��������ͨ������������Ŀ���
    unsigned char VoltsCh[4];//ʾ�����ĵ��۵�λ
    short activateCH_Sum;
    BOOL use_4,use_2,use_1;
    short nChannelMode;
    long AdcSleep;//ADC1511�����߿��ƼĴ���  0x0F
    long AdcGain4;//4ͨ��ģʽ������Ŀ���     //0x2A
    //long AdcGain2AndGain1;//2ͨ��ģʽ�͵�ͨ��ģʽͨ��ģʽ������Ŀ���  //2B
    long AdcInpSel1;//�����ģ�⿪�ص�ѡ��  //3A
    long AdcInpSel2;//�����ģ�⿪�ص�ѡ��        //3B
    long AdcChaNumClkDiv;//ADC��ͨ��ģʽ�ͷ�Ƶ��������   //31
    unsigned short FpgaWrRamModel;//FPGAдRAM����16��64��
    unsigned short FpgaWrTrigModel;//100M  FPGA������ģʽ
    //unsigned short FpgaWrRamControl;
    //unsigned short FpgaWrTrigControl;

    use_4=FALSE;use_2=FALSE;use_1=FALSE;
    for(int j=0;j<4;j++)
    {
        VoltsCh[j] = (unsigned char)RelayControl->nCHVoltDIV[j];
        OnOffCh[j]=RelayControl->bCHEnable[j]?0x01:0x00;//���ø���ͨ���˷ŵ�ʹ��
    }

    /****/
    PUCHAR outBuffer = NULL;
    nChannelMode=0;
    activateCH_Sum=0;
    OnOffDso = 0x0F & (
                ((OnOffCh[3] & 0x01) << 3) |//CH4
            ((OnOffCh[2] & 0x01) << 2) |//CH3
            ((OnOffCh[1] & 0x01) << 1) |//CH2
            ((OnOffCh[0] & 0x01) << 0)  //CH1
            );


    /*****************************************Start***************************************************/
    //���ø���ͨ���̵���˥���Ŀ���ADC1511��������ı��

    for(int i=0;i<4;i++)
    {
        // swich ���������ø���ͨ���̵���˥���Ŀ���ADC1511��������ı��
        //        cgain_cfg    cgain*<3:0>    Implemented Gain Factor(x)
        //            1              0               1
        //            1              1              1.25
        //            1              10              2
        //            1              11             2.5
        //            1             100              4
        //            1             101              5
        //            1             110              8
        //            1             111              10
        //            1             1000            12.5
        //            1             1001             16
        //            1             1010             20
        //            1             1011             25
        //            1             1100             32
        //            1             1101             50
        //            1             1110           Not used
        //            1             1111           Not used

 switch (VoltsCh[i])//5nV ,10mV,20mV,50mV ��˥��
        {
#ifdef DSO_6000_12DIV

        case 0://2mV 50
            GainCh[i] = 13;
            break;
        case 1://5mV 20
            GainCh[i] = 10;
            break;
        case 2://10mV 10
            GainCh[i] = 7;
            break;
        case 3://20mV 5
            GainCh[i] = 5;
            break;
        case 4://50mV 2
            GainCh[i] = 2;
            break;
        case 5://100mV 5
            GainCh[i] = 5;
            break;
        case 6://200mV 2.5
            GainCh[i] = 3; 
            break;
        case 7://500mV 8  
            GainCh[i] = 6;
            break;
        case 8://1V 4
            GainCh[i] = 4;
            break;
        case 9://2V 2  
            GainCh[i] = 2;
            break;
        case 10://5V 4  
            GainCh[i] = 4;
            break;
        case 11://10V 2  
            GainCh[i] = 2;
            break;
#elif defined D10_100DELAY

        case 0://2mV 50
            GainCh[i] = 13;
            break;
        case 1://5mV 20
            GainCh[i] = 10;
            break;
        case 2://10mV 10
            GainCh[i] = 7;
            break;
        case 3://20mV 5
            GainCh[i] = 5;
            break;
        case 4://50mV 2
            GainCh[i] = 2;
            break;
        case 5://100mV 1
            GainCh[i] = 0;
            break;
        case 6://200mV 5
            GainCh[i] = 5; 
            break;
        case 7://500mV 2  
            GainCh[i] = 2;
            break;
        case 8://1V 1
            GainCh[i] = 0;
            break;
        case 9://2V 5  
            GainCh[i] = 5;
            break;
        case 10://5V 2  
            GainCh[i] = 2;
            break;
        case 11://10V 1  
            GainCh[i] = 0;
            break;

#else
        case 0://5mV 20
            GainCh[i] = 10;
            break;
        case 1://10mV 10
            GainCh[i] = 7;
            break;
        case 2://20mV 5
            GainCh[i] = 5;
            break;
        case 3://50mV 2
            GainCh[i] = 2;
            break;
        case 4://100mV 1  //��˥����
            GainCh[i] = 0;
            break;
        case 5://200mV 25
            GainCh[i] = 11;
            break;
        case 6://500mV 10
            GainCh[i] = 7;
            break;
        case 7://1V 5
            GainCh[i] = 5;
            break;
        case 8://2V 2.5
            GainCh[i] = 3;
            break;
        case 9://5V 1
            GainCh[i] = 0;
            break;
#endif

        default:
            GainCh[i] = 0;
            break;
        }
        if(OnOffCh[i])        //�����ܹ�ǰ�˼����ͨ����Ŀ
            activateCH_Sum++;
    }
    //����Ĭ��ֵ
    {
        AdcSleep = 0x00;//ADC1511�����߿��ƼĴ���
        //AdcGain2AndGain1 = 0;//2ͨ��ģʽ�͵�ͨ��ģʽͨ��ģʽ������Ŀ���
        AdcInpSel1 = 0x0000;//ģ��ͨ��CH1 CH1
        AdcInpSel2 = 0x0000;//ģ��ͨ��CH1 CH1
        AdcChaNumClkDiv = 0x0000;//��ͨ��ͨ��ģʽ 4��Ƶ
        AdcGain4 = 0xFFFF ;//4ͨ��ģʽ������Ŀ���
        FpgaWrRamModel=0;
        FpgaWrTrigModel=0;
    }
    //��switch�����и�ֵ
    switch (activateCH_Sum) {
    case 0:  //û�д��κ�ͨ��
        break;
    case 1:  //ǰ��ֻ��һ��ͨ��
        if(nTimeDIV<=TIMEDIV_1GSA) //1GSa
        {
            use_4=TRUE;
            nChannelMode=1;
            AdcSleep = 0x0000;//ADC1511�����߿��ƼĴ���
            AdcChaNumClkDiv = 0x0001;//��ͨ��ͨ��ģʽ
            for(int i=0;i<4;i++)
            {
                if(OnOffCh[i])
                {
                    AdcGain4 = 0xFFFF & (
                                ((GainCh[i] & 0x0F) << 12) |
                                ((GainCh[i] & 0x0F) << 8 ) |
                                ((GainCh[i] & 0x0F) << 4 ) |
                                ((GainCh[i] & 0x0F) << 0));//4ͨ��ģʽ������Ŀ���
                    break;
                }
            }
            //qDebug()<<OnOffDso;
            switch (OnOffDso) {
            case 1:  //ͨ��1��
                AdcInpSel1 = 0x1010;//ģ��ͨ��CH1 CH1
                break;
            case 2://ͨ��2��
                AdcInpSel1 = 0x0808;//ģ��ͨ��CH1 CH1
                break;
            case 4://ͨ��3��
                AdcInpSel1 = 0x0404;//ģ��ͨ��CH1 CH1
                break;
            case 8://ͨ��4��
                AdcInpSel1 = 0x0202;//ģ��ͨ��CH1 CH1
                break;
            default:
                break;
            }
            AdcInpSel2=AdcInpSel1;
        }
        else if(nTimeDIV==TIMEDIV_500MSA)  //500MSa
        {
            use_2=TRUE;
            nChannelMode=2;
            AdcChaNumClkDiv = 0x0102;//��ͨ��ͨ��ģʽ 4��Ƶ
            for(int i=0;i<4;i++)
            {
                if(OnOffCh[i])//��Ȼ��˫ͨ�����Ǹ�ֵֻ����һ��ͨ��
                {
                    AdcGain4 = 0xFFFF & (
                                ((GainCh[i] & 0x0F) << 12) |
                                ((GainCh[i] & 0x0F) << 8 ) |
                                ((GainCh[i] & 0x0F) << 4 ) |
                                ((GainCh[i] & 0x0F) << 0));//4ͨ��ģʽ������Ŀ���
                    break;
                }
            }
            switch (OnOffDso) {
            case 1:
                AdcSleep = 0x0020;  //ADC1511�����߿��ƼĴ���
                AdcInpSel1 = 0x1010;//ģ��ͨ��CH1 CH1
                AdcInpSel2 = 0x0808;//ģ��ͨ��CH1 CH1
                break;
            case 2:
                AdcSleep = 0x0010;  //ADC1511�����߿��ƼĴ���
                AdcInpSel1 = 0x0404;//ģ��ͨ��CH1 CH1
                AdcInpSel2 = 0x0808;//ģ��ͨ��CH1 CH1

                break;
            case 4:
                AdcSleep = 0x0020;  //ADC1511�����߿��ƼĴ���
                /*AdcInpSel1 = 0x0202;//ģ��ͨ��CH1 CH1
                    AdcInpSel2 = 0x0404;//ģ��ͨ��CH1 CH1
                    */
                AdcInpSel1 = 0x0404;//ģ��ͨ��CH1 CH1
                AdcInpSel2 = 0x0202;//ģ��ͨ��CH1 CH1

                break;
            case 8:
                AdcSleep = 0x0010;  //ADC1511�����߿��ƼĴ���
                AdcInpSel1 = 0x1010;//ģ��ͨ��CH1 CH1
                AdcInpSel2 = 0x0202;//ģ��ͨ��CH1 CH1
                // AdcInpSel2 = 0x0202;//ģ��ͨ��CH1 CH1
                break;
            default:
                break;
            }
			 //AdcInpSel2=AdcInpSel1;
        }
        else//250Msa
        {
            AdcGain4 = 0xFFFF & (
                        ((GainCh[3] & 0x0F) << 12) |
                    ((GainCh[2] & 0x0F) << 8 ) |
                    ((GainCh[1] & 0x0F) << 4 ) |
                    ((GainCh[0] & 0x0F) << 0));//4ͨ��ģʽ������Ŀ���
            use_1=TRUE;
            nChannelMode=4;
            AdcChaNumClkDiv = 0x0204;//��ͨ��ͨ��ģʽ 4��Ƶ
            AdcInpSel1 = 0x0810;//ģ��ͨ��CH1 CH1
            AdcInpSel2 = 0x0204;//ģ��ͨ��CH1 CH1
            AdcSleep=0x0F-(OnOffDso&0x0F);
        }
        //FpgaWrRamModel;FPGAдRAM����16��64���� FpgaWrTrigModel;100M  FPGA������ģʽ
        if(nTimeDIV<=TIMEDIV_1GSA )  //1GSa
            FpgaWrRamModel=0x00;
        else if(nTimeDIV==TIMEDIV_500MSA)//500MSa
            FpgaWrRamModel=0x01;
        else if(nTimeDIV==TIMEDIV_250MSA )//250MSa
            FpgaWrRamModel=0x02;
        else
            FpgaWrRamModel=0x03;
        FpgaWrTrigModel=0x03-FpgaWrRamModel;
        break;
    case 2://ǰ�˴�2��ͨ��
        if(nTimeDIV<=TIMEDIV_500MSA)  //500MSa
        {
            use_4=TRUE;
            nChannelMode=2;
            AdcChaNumClkDiv=0x0102;
            AdcSleep=0x00;
            int i=0;
            AdcGain4 =0;
            for(;i<4;i++)
            {
                if(OnOffCh[i])
                {
                    AdcGain4 = 0x00FF & (
                                ((GainCh[i] & 0x0F) << 4) |
                                ((GainCh[i] & 0x0F) << 0));
                    break;
                }
            }
            for(;i<4;i++)
            {
                if(OnOffCh[i])
                {
                    AdcGain4 = 0xFFFF & (
                                ((GainCh[i] & 0x0F) << 12) |
                                ((GainCh[i] & 0x0F) << 8 ) |
                                ((AdcGain4 & 0xFF) << 0));//2ͨ��ģʽ������Ŀ���
                }
            }
            switch (OnOffDso) {
            case 3://1  2
                AdcInpSel1=0x1010;
                AdcInpSel2=0x0808;
                break;
            case 5://1 3
                AdcInpSel1=0x1010;
                AdcInpSel2=0x0404;
                break;
            case 9:// 1 4
                AdcInpSel1=0x1010;
                AdcInpSel2=0x0202;
                break;
            case 6:// 2 3
                AdcInpSel1=0x0808;
                AdcInpSel2=0x0404;

                break;
            case 10://
                AdcInpSel1=0x0808;
                AdcInpSel2=0x0202;
                break;
            case 12:
                AdcInpSel1=0x0404;
                AdcInpSel2=0x0202;
                break;
            default:
                break;
            }
        }
        else//250Msa
        {

            use_2=TRUE;
            nChannelMode=4;
            AdcChaNumClkDiv=0x0204;
            AdcGain4 = 0xFFFF & (
                        ((GainCh[3] & 0x0F) << 12) |
                    ((GainCh[2] & 0x0F) << 8 ) |
                    ((GainCh[1] & 0x0F) << 4 ) |
                    ((GainCh[0] & 0x0F) << 0));//4ͨ��ģʽ������Ŀ���
            AdcInpSel1=0x0810;
            AdcInpSel2=0x0204;
            AdcSleep=0x0F-(OnOffDso&0x0F);
        }
        //FpgaWrRamModel;FPGAдRAM����16��64���� FpgaWrTrigModel;100M  FPGA������ģʽ
        if(nTimeDIV<=TIMEDIV_500MSA)
            FpgaWrRamModel=0x00;
        else if(nTimeDIV==TIMEDIV_250MSA )
            FpgaWrRamModel=0x01;
        else
            FpgaWrRamModel=0x02;
        FpgaWrTrigModel=0x02-FpgaWrRamModel;
        break;
    case 3://ǰ�˴�3��ͨ��
    case 4://ǰ�˴�4��ͨ��
        use_4=TRUE;
        nChannelMode=4;
        AdcChaNumClkDiv=0x0204;
        AdcSleep=0x0F-(OnOffDso&0x0F);
        AdcInpSel1=0x0810;
        AdcInpSel2=0x0204;
        AdcGain4 = 0xFFFF & (
                    ((GainCh[3] & 0x0F) << 12) |
                ((GainCh[2] & 0x0F) << 8 ) |
                ((GainCh[1] & 0x0F) << 4 ) |
                ((GainCh[0] & 0x0F) << 0));//4ͨ��ģʽ������Ŀ���
        //FpgaWrRamModel;FPGAдRAM����16��64���� FpgaWrTrigModel;100M  FPGA������ģʽ

        if(nTimeDIV<=TIMEDIV_250MSA )
            FpgaWrRamModel=0x00;
        else
            FpgaWrRamModel=0x01;
        FpgaWrTrigModel=0x01-FpgaWrRamModel;
        break;
    default:
        break;
    }

    if(nChannelMode==0)
        return 0;
    WORD status=FALSE;

    UINT m_nSize;

    m_nSize = 8;
    outBuffer = (PUCHAR)malloc(m_nSize);
    outBuffer[0]=0x08;
    outBuffer[1]=0x00;
    outBuffer[2]=0x00;
    outBuffer[6]=0x04;          //ADC
    outBuffer[7]=0x00;


    outBuffer[3]=0xff&(unsigned char)(AdcInpSel1>> 0) ;
    outBuffer[4]=0xff&(unsigned char)(AdcInpSel1>> 8) ;
    outBuffer[5]=0x3A;          //3A

    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    Sleep(3);
    outBuffer[3]=0xff&(unsigned char)(AdcInpSel2>> 0) ;
    outBuffer[4]=0xff&(unsigned char)(AdcInpSel2>> 8) ;
    outBuffer[5]=0x3B;          //3B

    status= sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    Sleep(3);
    AdcSleep=0x00;
    outBuffer[3]=0xff&(unsigned char)(AdcSleep>> 0) ;
    outBuffer[4]=0xff&(unsigned char)(AdcSleep>> 8) ;
    outBuffer[5]=0x0F;          //AdcSleep

    status= sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    Sleep(3);
    outBuffer[3]=0xff&(unsigned char)(AdcChaNumClkDiv>> 0) ;
    outBuffer[4]=0xff&(unsigned char)(AdcChaNumClkDiv>> 8) ;
    outBuffer[5]=0x31;          //AdcChaNumClkDivͨ��ģʽ
    status= sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    Sleep(3);
    if(nChannelMode==4)
    {
        outBuffer[3]=0xff&(unsigned char)(AdcGain4>> 0) ;
        outBuffer[4]=0xff&(unsigned char)(AdcGain4>> 8) ;
        outBuffer[5]=0x2A;          //AdcGain4
        status= sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    }
    else
    {
        outBuffer[3]=0xff&(unsigned char)((AdcGain4&0x0F)|
                                          ((AdcGain4>>8)&0xF0)) ;  //˫ͨ��
        // outBuffer[3]=0xff&(unsigned char)((AdcGain4)&0xFF);//+((AdcGain4>>16)&0xF0)) ;
        outBuffer[4]=0xff&(unsigned char)(AdcGain4&0x0F) ;//��ͨ��ģʽ
        outBuffer[5]=0x2B;          //AdcGain2A1
        status= sendOutBuffer(DeviceIndex,m_nSize,outBuffer);

    }

    //Sleep(1);
    //realloc(outBuffer,4);
    /*
        outBuffer[0]=0x12;
        outBuffer[1]=0x00;
        outBuffer[2]=0xff&(unsigned char)(FpgaWrRamControl>> 0) ;
        outBuffer[3]=0xff&(unsigned char)(FpgaWrRamControl>> 8) ;

        status= sendOutBuffer(DeviceIndex,4,outBuffer);
        */
    free(outBuffer);
    return status;//DelayCurrentState;
}

/*
WORD        SetADC(WORD DeviceIndex, PRELAYCONTROL RelayControl, WORD nTimeDIV)
{

    BOOL OnOffCh[4];
    unsigned char OnOffDso;//ʾ�����ܵ�ͨ��״̬
    unsigned char GainCh[4];//ʾ��������ͨ������������Ŀ���
    unsigned char VoltsCh[4];//ʾ�����ĵ��۵�λ
    short activateCH_Sum;
    short nChannelMode;
    long AdcSleep;//ADC1511�����߿��ƼĴ���  0x0F
    long AdcGain4;//4ͨ��ģʽ������Ŀ���     //0x2A
    //long AdcGain2AndGain1;//2ͨ��ģʽ�͵�ͨ��ģʽͨ��ģʽ������Ŀ���  //2B
    long AdcInpSel1;//�����ģ�⿪�ص�ѡ��  //3A
    long AdcInpSel2;//�����ģ�⿪�ص�ѡ��        //3B
    long AdcChaNumClkDiv;//ADC��ͨ��ģʽ�ͷ�Ƶ��������   //31
    for(int j=0;j<4;j++)
    {
        VoltsCh[j] = (unsigned char)RelayControl->nCHVoltDIV[j];
        OnOffCh[j]=RelayControl->bCHEnable[j]?0x01:0x00;//���ø���ͨ���˷ŵ�ʹ��
    }

    
    PUCHAR outBuffer = NULL;
    nChannelMode=0;
    activateCH_Sum=0;
    OnOffDso = 0x0F & (
            ((OnOffCh[3] & 0x01) << 3) |//CH4
            ((OnOffCh[2] & 0x01) << 2) |//CH3
            ((OnOffCh[1] & 0x01) << 1) |//CH2
            ((OnOffCh[0] & 0x01) << 0)  //CH1
            );




    for(int i=0;i<4;i++)
    {
        // swich ���������ø���ͨ���̵���˥���Ŀ���ADC1511��������ı��
        //        cgain_cfg    cgain*<3:0>    Implemented Gain Factor(x)
        //            1              0               1
        //            1              1              1.25
        //            1              10              2
        //            1              11             2.5
        //            1             100              4
        //            1             101              5
        //            1             110              8
        //            1             111              10
        //            1             1000            12.5
        //            1             1001             16
        //            1             1010             20
        //            1             1011             25
        //            1             1100             32
        //            1             1101             50
        //            1             1110           Not used
        //            1             1111           Not used

        switch (VoltsCh[i])//5nV ,10mV,20mV,50mV ��˥��
        {
        case 0://5mV 20
            GainCh[i] = 10;
            break;
        case 1://10mV 10
            GainCh[i] = 7;
            break;
        case 2://20mV 5
            GainCh[i] = 5;
            break;
        case 3://50mV 2
            GainCh[i] = 2;
            break;
        case 4://100mV 1  //��˥����
            GainCh[i] = 0;
            break;
        case 5://200mV 25
            GainCh[i] = 11;
            break;
        case 6://500mV 10
            GainCh[i] = 7;
            break;
        case 7://1V 5
            GainCh[i] = 5;
            break;
        case 8://2V 2.5
            GainCh[i] = 3;
            break;
        case 9://5V 1
            GainCh[i] = 0;
            break;
        default:
            GainCh[i] = 0;
            break;
        }

        if(OnOffCh[i])        //�����ܹ�ǰ�˼����ͨ����Ŀ
            activateCH_Sum++;
    }
    //����Ĭ��ֵ
    {
        AdcSleep = 0x00;//ADC1511�����߿��ƼĴ���
        //AdcGain2AndGain1 = 0;//2ͨ��ģʽ�͵�ͨ��ģʽͨ��ģʽ������Ŀ���
        AdcInpSel1 = 0x0000;//ģ��ͨ��CH1 CH1
        AdcInpSel2 = 0x0000;//ģ��ͨ��CH1 CH1
        AdcChaNumClkDiv = 0x0000;//��ͨ��ͨ��ģʽ 4��Ƶ
        AdcGain4 = 0xFFFF ;//4ͨ��ģʽ������Ŀ���


    }
    //��switch�����и�ֵ
    switch (activateCH_Sum) {
    case 0:  //û�д��κ�ͨ��
        break;
    case 1:  //ǰ��ֻ��һ��ͨ��
        if(nTimeDIV<6) //500ns  500MSa
        {
            nChannelMode=1;
            AdcSleep = 0x0000;//ADC1511�����߿��ƼĴ���
            AdcChaNumClkDiv = 0x0001;//��ͨ��ͨ��ģʽ
            for(int i=0;i<4;i++)
            {
                if(OnOffCh[i])
                {
                    AdcGain4 = 0xFFFF & (
                                ((GainCh[i] & 0x0F) << 12) |
                                ((GainCh[i] & 0x0F) << 8 ) |
                                ((GainCh[i] & 0x0F) << 4 ) |
                                ((GainCh[i] & 0x0F) << 0));//4ͨ��ģʽ������Ŀ���
                    break;
                }
            }
            //qDebug()<<OnOffDso;
            switch (OnOffDso) {
            case 1:  //ͨ��1��
                AdcInpSel1 = 0x1010;//ģ��ͨ��CH1 CH1
                break;
            case 2://ͨ��2��
                AdcInpSel1 = 0x0808;//ģ��ͨ��CH1 CH1
                break;
            case 4://ͨ��3��
                AdcInpSel1 = 0x0404;//ģ��ͨ��CH1 CH1
                break;
            case 8://ͨ��4��
                AdcInpSel1 = 0x0202;//ģ��ͨ��CH1 CH1
                break;
            default:
                break;
            }
            AdcInpSel2=AdcInpSel1;
        }
        else if(nTimeDIV==6)  //500nS
        {
            nChannelMode=2;
            AdcChaNumClkDiv = 0x0102;//��ͨ��ͨ��ģʽ 4��Ƶ
            for(int i=0;i<4;i++)
            {
                if(OnOffCh[i])//��Ȼ��˫ͨ�����Ǹ�ֵֻ����һ��ͨ��
                {
                    AdcGain4 = 0xFFFF & (
                                ((GainCh[i] & 0x0F) << 12) |
                                ((GainCh[i] & 0x0F) << 8 ) |
                                ((GainCh[i] & 0x0F) << 4 ) |
                                ((GainCh[i] & 0x0F) << 0));//4ͨ��ģʽ������Ŀ���
                    break;
                }
            }
            switch (OnOffDso) {
            case 1:
                AdcSleep = 0x0020;  //ADC1511�����߿��ƼĴ���
                AdcInpSel1 = 0x1010;//ģ��ͨ��CH1 CH1
                AdcInpSel2 = 0x1010;//ģ��ͨ��CH1 CH1
                break;
            case 2:
                AdcSleep = 0x0010;  //ADC1511�����߿��ƼĴ���
                AdcInpSel1 = 0x0808;//ģ��ͨ��CH1 CH1
                AdcInpSel2 = 0x0808;//ģ��ͨ��CH1 CH1

                break;
            case 4:
                AdcSleep = 0x0020;  //ADC1511�����߿��ƼĴ���

                AdcInpSel1 = 0x0404;//ģ��ͨ��CH1 CH1
                AdcInpSel2 = 0x0404;//ģ��ͨ��CH1 CH1

                break;
            case 8:
                AdcSleep = 0x0010;  //ADC1511�����߿��ƼĴ���
                AdcInpSel1 = 0x0202;//ģ��ͨ��CH1 CH1
                AdcInpSel2 = 0x0202;//ģ��ͨ��CH1 CH1
               // AdcInpSel2 = 0x0202;//ģ��ͨ��CH1 CH1
                break;
            default:
                break;
            }
        }
        else
        {
            AdcGain4 = 0xFFFF & (
                     ((GainCh[3] & 0x0F) << 12) |
                    ((GainCh[2] & 0x0F) << 8 ) |
                    ((GainCh[1] & 0x0F) << 4 ) |
                    ((GainCh[0] & 0x0F) << 0));//4ͨ��ģʽ������Ŀ���
            nChannelMode=4;
            AdcChaNumClkDiv = 0x0204;//��ͨ��ͨ��ģʽ 4��Ƶ
            AdcInpSel1 = 0x0810;//ģ��ͨ��CH1 CH1
            AdcInpSel2 = 0x0204;//ģ��ͨ��CH1 CH1
            AdcSleep=0x0F-(OnOffDso&0x0F);
        }
        break;
    case 2://ǰ�˴�2��ͨ��
        if(nTimeDIV<7)  //1uS  250Ms
        {
            nChannelMode=2;
            AdcChaNumClkDiv=0x0102;
            AdcSleep=0x00;
            int i=0;
            AdcGain4 =0;
            for(;i<4;i++)
            {
                if(OnOffCh[i])
                {
                    AdcGain4 = 0x00FF & (
                                ((GainCh[i] & 0x0F) << 4) |
                                ((GainCh[i] & 0x0F) << 0));
                    break;
                }
            }
            for(;i<4;i++)
            {
                if(OnOffCh[i])
                {
                    AdcGain4 = 0xFFFF & (
                                ((GainCh[i] & 0x0F) << 12) |
                                ((GainCh[i] & 0x0F) << 8 ) |
                                ((AdcGain4 & 0xFF) << 0));//2ͨ��ģʽ������Ŀ���
                }
            }
            switch (OnOffDso) {
            case 3://1  2
                AdcInpSel1=0x1010;
                AdcInpSel2=0x0808;
                break;
            case 5://1 3
                AdcInpSel1=0x1010;
                AdcInpSel2=0x0404;
                break;
            case 9:// 1 4
                AdcInpSel1=0x1010;
                AdcInpSel2=0x0202;
                break;
            case 6:// 2 3
                AdcInpSel1=0x0808;
                AdcInpSel2=0x0404;

                break;
            case 10://
                AdcInpSel1=0x0808;
                AdcInpSel2=0x0202;
                break;
            case 12:
                AdcInpSel1=0x0404;
                AdcInpSel2=0x0202;
                break;
            default:
                break;
            }
        }
        else
        {
            nChannelMode=4;
            AdcChaNumClkDiv=0x0204;
            AdcGain4 = 0xFFFF & (
                    ((GainCh[3] & 0x0F) << 12) |
                    ((GainCh[2] & 0x0F) << 8 ) |
                    ((GainCh[1] & 0x0F) << 4 ) |
                    ((GainCh[0] & 0x0F) << 0));//4ͨ��ģʽ������Ŀ���
            AdcInpSel1=0x0810;
            AdcInpSel2=0x0204;
            AdcSleep=0x0F-(OnOffDso&0x0F);
        }

        break;
    case 3://ǰ�˴�3��ͨ��
    case 4://ǰ�˴�4��ͨ��
        nChannelMode=4;
        AdcChaNumClkDiv=0x0204;
        AdcSleep=0x0F-(OnOffDso&0x0F);
        AdcInpSel1=0x0810;
        AdcInpSel2=0x0204;
        AdcGain4 = 0xFFFF & (
                ((GainCh[3] & 0x0F) << 12) |
                ((GainCh[2] & 0x0F) << 8 ) |
                ((GainCh[1] & 0x0F) << 4 ) |
                ((GainCh[0] & 0x0F) << 0));//4ͨ��ģʽ������Ŀ���

        break;
    default:
        break;
    }

    if(nChannelMode==0)
        return 0;
    WORD status=FALSE;

    UINT m_nSize;

    m_nSize = 8;
    outBuffer = (PUCHAR)malloc(m_nSize);
    outBuffer[0]=0x08;
    outBuffer[1]=0x00;
    outBuffer[2]=0x00;
    outBuffer[6]=0x04;          //ADC
    outBuffer[7]=0x00;


    outBuffer[3]=0xff&(unsigned char)(AdcInpSel1>> 0) ;
    outBuffer[4]=0xff&(unsigned char)(AdcInpSel1>> 8) ;
    outBuffer[5]=0x3A;          //3A

    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    Sleep(3);
    outBuffer[3]=0xff&(unsigned char)(AdcInpSel2>> 0) ;
    outBuffer[4]=0xff&(unsigned char)(AdcInpSel2>> 8) ;
    outBuffer[5]=0x3B;          //3B

    status= sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    Sleep(3);
   // AdcSleep=0x00;
    outBuffer[3]=0xff&(unsigned char)(AdcSleep>> 0) ;
    outBuffer[4]=0xff&(unsigned char)(AdcSleep>> 8) ;
    outBuffer[5]=0x0F;          //AdcSleep

    status= sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    Sleep(3);
    outBuffer[3]=0xff&(unsigned char)(AdcChaNumClkDiv>> 0) ;
    outBuffer[4]=0xff&(unsigned char)(AdcChaNumClkDiv>> 8) ;
    outBuffer[5]=0x31;          //AdcChaNumClkDivͨ��ģʽ
    status= sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    Sleep(3);
    if(nChannelMode==4)
    {
        outBuffer[3]=0xff&(unsigned char)(AdcGain4>> 0) ;
        outBuffer[4]=0xff&(unsigned char)(AdcGain4>> 8) ;
        outBuffer[5]=0x2A;          //AdcGain4
        status= sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    }
    else
    {
        outBuffer[3]=0xff&(unsigned char)((AdcGain4&0x0F)|
                                          ((AdcGain4>>8)&0xF0)) ;  //˫ͨ��
        // outBuffer[3]=0xff&(unsigned char)((AdcGain4)&0xFF);//+((AdcGain4>>16)&0xF0)) ;
        outBuffer[4]=0xff&(unsigned char)(AdcGain4&0x0F) ;//��ͨ��ģʽ
        outBuffer[5]=0x2B;          //AdcGain2A1
        status= sendOutBuffer(DeviceIndex,m_nSize,outBuffer);

    }


    free(outBuffer);
    return status;


}*/
DLL_API WORD WINAPI dsoHTSetADC(WORD DeviceIndex, PRELAYCONTROL RelayControl, WORD nTimeDIV)
{
	return 1;
//	return SetADC(DeviceIndex, RelayControl,nTimeDIV);
}


WORD        SetAboutInputChannelAndTrigger_6104(WORD DeviceIndex,PRELAYCONTROL RelayControl)
{
    unsigned char Ch_ADC[4];    //4��ͨ����AD/DC����,����Ǵ�RelayControl��ȡ��ԭʼ����
    unsigned char FiltCh[4];    //4��ͨ���Ĵ�������
    unsigned char OpEnCh[4];    //4��ͨ�����˷�ʹ��

	unsigned char ACDC_Ch[4];   //4��ͨ����AD/DC����

    unsigned char H595_Ch[4];
    PUCHAR outBuffer = NULL;
	unsigned char nReset;

    /*****************************************Start***************************************************/ //yt20090927
#ifdef DSO_6000_12DIV
	nReset=0x86;
	unsigned char Ch_d25[4];        //4��ͨ��˥������
    unsigned char Ch_d25Delay[4];  //4��ͨ��˥������    
	unsigned char Ch_d10[4];        //4��ͨ��˥������
    unsigned char Ch_d10Delay[4];  //4��ͨ��˥������
    for(int i=0;i<4;i++)
    {
        FiltCh[i]       =RelayControl->bCHBWLimit[i];

        Ch_ADC[i]       =(RelayControl->nCHCoupling[i])&0xFF;
       // OpEnCh[i]       =RelayControl->bCHEnable[i]?0x01:0x00;  //���ø���ͨ���˷ŵ�ʹ��
        OpEnCh[i]       =0x01;  //���ø���ͨ���˷ŵ�ʹ��
        ACDC_Ch[i]      =Ch_ADC[i] ?0x00:0x01;           //���ø���ͨ����AD/DC���� //ԭ����DC 0 AC 1 GND 2
       // ACDC_ChDelay[i] =Ch_ADC[i] ?0x01:0x00;
        Ch_d25[i]       = RelayControl->nCHVoltDIV[i] <= 6?0x00:0x01;
        Ch_d25Delay[i]  = Ch_d25[i]==0?0x01:0x00;

        Ch_d10[i]       = RelayControl->nCHVoltDIV[i] <= 4
                ||RelayControl->nCHVoltDIV[i]==7
                ||RelayControl->nCHVoltDIV[i]==8
				||RelayControl->nCHVoltDIV[i]==9?0x00:0x01;//���ø���ͨ���̵���˥���Ŀ���//5mV ,10mV,20mV,50mV,100mV ��˥��

        Ch_d10Delay[i]  = Ch_d10[i]==0?0x01:0x00;
    }

    //����ƴ��H595�Ĵ����ı�ʶ
    for(int k=0;k<4;k++)
    {
        H595_Ch[k] = 0xFF & (
                    ((FiltCh[k] & 0x01)       << 7) |//��������				1
                    ((Ch_d25[k] & 0x01)       << 6) |//25��˥�����ƶ�2
                    ((Ch_d25Delay[k] & 0x01)  << 5) |//25��˥�����ƶ�1 ?
                    ((Ch_d10[k] & 0x01 & 0x01)<< 4) |//10��˥�����ƶ�2
                    ((Ch_d10Delay[k] & 0x01)  << 3) |//10��˥�����ƶ�1?
                    ((ACDC_Ch[k] & 0x01)      << 2) |//AC/DC�Ĺ���1
                    ((OpEnCh[k] & 0x01)       << 1) //�˷�ʹ��                                                    
                    );
    }
#elif defined D10_100DELAY
	nReset=0x86;
	unsigned char Ch_d25[4];        //4��ͨ��˥������
    unsigned char Ch_d25Delay[4];  //4��ͨ��˥������    
	unsigned char Ch_d10[4];        //4��ͨ��˥������
    unsigned char Ch_d10Delay[4];  //4��ͨ��˥������
    for(int i=0;i<4;i++)
    {
        FiltCh[i]       =RelayControl->bCHBWLimit[i];

        Ch_ADC[i]       =(RelayControl->nCHCoupling[i])&0xFF;
       // OpEnCh[i]       =RelayControl->bCHEnable[i]?0x01:0x00;  //���ø���ͨ���˷ŵ�ʹ��
        OpEnCh[i]       =0x01;  //���ø���ͨ���˷ŵ�ʹ��
        ACDC_Ch[i]      =Ch_ADC[i] ?0x00:0x01;           //���ø���ͨ����AD/DC���� //ԭ����DC 0 AC 1 GND 2
       // ACDC_ChDelay[i] =Ch_ADC[i] ?0x01:0x00;
        Ch_d25[i]       = RelayControl->nCHVoltDIV[i] <= 8?0x00:0x01;
        
/*
        Ch_d10[i]       = RelayControl->nCHVoltDIV[i] == 6
                ||RelayControl->nCHVoltDIV[i]==7
                ||RelayControl->nCHVoltDIV[i]==8?0x01:0x00;//���ø���ͨ���̵���˥���Ŀ���
				*/
		
		Ch_d10[i]       = RelayControl->nCHVoltDIV[i]<=5?0x00:0x01;

		Ch_d25Delay[i]  = Ch_d25[i]==0?0x01:0x00;
        Ch_d10Delay[i]  = Ch_d10[i]==0?0x01:0x00;
    }

    /*****************************************end***************************************************/ //yt20090927
    //����ƴ��H595�Ĵ����ı�ʶ
    for(int k=0;k<4;k++)
    {
        H595_Ch[k] = 0xFF & (
                    ((FiltCh[k] & 0x01)       << 7) |//��������				1
                    ((Ch_d25[k] & 0x01)       << 6) |//25��˥�����ƶ�2
                    ((Ch_d25Delay[k] & 0x01)  << 5) |//25��˥�����ƶ�1 ?
                    ((Ch_d10[k] & 0x01 & 0x01)<< 4) |//10��˥�����ƶ�2
                    ((Ch_d10Delay[k] & 0x01)  << 3) |//10��˥�����ƶ�1?
                    ((ACDC_Ch[k] & 0x01)      << 2) |//AC/DC�Ĺ���1
                    ((OpEnCh[k] & 0x01)       << 1) //�˷�ʹ��                                                    
                    );
    }

#else
	 nReset=0x87;
    //unsigned char ACDC_ChDelay[4];  //4��ͨ����AD/DC����
	unsigned char Ch_d50[4];        //4��ͨ��˥������
    unsigned char Ch_d50Delay[4];  //4��ͨ��˥������
	for(int i=0;i<4;i++)
    {
        FiltCh[i]       =RelayControl->bCHBWLimit[i];
		//FiltCh[i]       =0;
        Ch_ADC[i]       =(RelayControl->nCHCoupling[i])&0xFF;
       // OpEnCh[i]       =RelayControl->bCHEnable[i]?0x01:0x00;  //���ø���ͨ���˷ŵ�ʹ��
		OpEnCh[i]       =0x01;  //���ø���ͨ���˷ŵ�ʹ��
        ACDC_Ch[i]      =Ch_ADC[i] ?0x00:0x01;           //���ø���ͨ����AD/DC���� //ԭ����DC 0 AC 1 GND 2
        ACDC_ChDelay[i] =Ch_ADC[i] ?0x01:0x00;
        Ch_d50[i]       = RelayControl->nCHVoltDIV[i] <= 4?0x00:0x01;//���ø���ͨ���̵���˥���Ŀ���//5mV ,10mV,20mV,50mV,100mV ��˥��
        Ch_d50Delay[i]  = RelayControl->nCHVoltDIV[i] <= 4?0x01:0x00;
    }

    /*****************************************end***************************************************/ //yt20090927
    //����ƴ��H595�Ĵ����ı�ʶ
    for(int k=0;k<4;k++)
    {
        H595_Ch[k] = 0xFC & (
                    ((FiltCh[k] & 0x01)       << 7) |//��������
                    ((Ch_d50[k] & 0x01)       << 6) |//50��˥�����ƶ�2
                    ((Ch_d50Delay[k] & 0x01)  << 5) |//50��˥�����ƶ�1
                    ((ACDC_Ch[k] & 0x01)      << 4) |//AC/DC�Ĺ���2
                    ((ACDC_ChDelay[k] & 0x01) << 3) |//AC/DC�Ĺ���1
                    ((OpEnCh[k] & 0x01)       << 2)  //�˷�ʹ��
                    );
    }

#endif
    UINT m_nSize=8;
    BOOL status=FALSE;
    outBuffer = (PUCHAR)malloc(m_nSize);

    outBuffer[0] = 0x08;
    outBuffer[1] = 0x00;
    outBuffer[2] = H595_Ch[0]&0xFF;
    outBuffer[3] = H595_Ch[1]&0xFF;
    outBuffer[4] = H595_Ch[2]&0xFF;
    outBuffer[5] = H595_Ch[3]&0xFF;
    outBuffer[6] = 0x01; 
    outBuffer[7] = 0x00;        //�̵�������
    while(!GetSPIState(DeviceIndex)&0x01)
        Sleep(1);
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);

    //�̵�����λ   // 20150912 add by zhang

    Sleep(4);

    outBuffer[0] = 0x08;
    outBuffer[1] = 0x00;
    outBuffer[2] = outBuffer[2]&nReset;
    outBuffer[3] = outBuffer[3]&nReset;
    outBuffer[4] = outBuffer[4]&nReset;
    outBuffer[5] = outBuffer[5]&nReset;
    outBuffer[7] = 0x01;
    while(!GetSPIState(DeviceIndex)&0x01)
        Sleep(1);
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
	Sleep(50);
    free(outBuffer);
    return status;
}
WORD SetAboutInputChannelAndTriggerbak(WORD DeviceIndex,PRELAYCONTROL RelayControl)//�������ô���Դ�͸�Ƶ����   //ԭ�����ü̵�������by zhang 20150912
{
    int DelayCurrentState;
    unsigned char Ch1_1S1,Ch1_1S2,Ch2_1S1,Ch2_1S2,Ch3_1S1,Ch3_1S2,Ch4_1S1,Ch4_1S2;//
    unsigned char Trigsel1,Trigsel2,Trigsel3,Trigsel4,FiltCh1,FiltCh2,FiltCh3,FiltCh4;
    unsigned char Ch1_2S1,Ch1_2S2,Ch2_2S1,Ch2_2S2,Ch3_2S1,Ch3_2S2,Ch4_2S1,Ch4_2S2;
    unsigned char Ch1_1d100,Ch1_1d10,Ch2_1d100,Ch2_1d10,Ch3_1d100,Ch3_1d10;
    unsigned char Ch3_ACDC,Ch4_1d100,Ch4_1d10,Ch4_ACDC,ExtTrigControl;//,Relayn;
    unsigned char Ch1_ACDC,Ch2_ACDC;
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    BULK_TRANSFER_CONTROL   outBulkControl;
    HANDLE hOutDevice=NULL,hInDevice=NULL;

    //Chx_1S1,Chx_1S2(AD Sourse):00,ch1;01,ch2;10,ch3;11,ch4
    //Trigsel1,Trigsel2,Trigsel3(Triger Sourse):000,ch2;001,ch3;010,ch4;011,ch1;100,EXT TRIG
    //Trigsel4(Triger FILT)
    //CHANNEL FILT:FiltCh1,FiltCh2,FiltCh3,FiltCh4
    //Chx_2S1,Chx_2S2(ATT 1,2,5):00,5V;01,2V;10,1V;11,NC

    /*****************************************Start***************************************************/ //yt20090927
    Trigsel4 = RelayControl->bTrigFilt;
    FiltCh1 = RelayControl->bCHBWLimit[0];
    FiltCh2 = RelayControl->bCHBWLimit[1];
    FiltCh3 = RelayControl->bCHBWLimit[2];
    FiltCh4 = RelayControl->bCHBWLimit[3];
    Ch1_ACDC = (unsigned char)RelayControl->nCHCoupling[0];
    Ch2_ACDC = (unsigned char)RelayControl->nCHCoupling[1];
    Ch3_ACDC = (unsigned char)RelayControl->nCHCoupling[2];
    Ch4_ACDC = (unsigned char)RelayControl->nCHCoupling[3];
    ExtTrigControl = 0x00;
    //CH1+CH2

    if(RelayControl->nALT == 1)//����
    {
        if(RelayControl->nTrigSource == 0)//CH1
        {
            Ch1_1S1 = 0x00;
            Ch1_1S2 = 0x00;
            Ch2_1S1 = 0x00;
            Ch2_1S2 = 0x00;
        }
        else if(RelayControl->nTrigSource == 1)//CH2
        {
            Ch1_1S1 = 0x01;
            Ch1_1S2 = 0x00;
            Ch2_1S1 = 0x01;
            Ch2_1S2 = 0x00;
        }
        else if(RelayControl->nTrigSource == 2)//CH3
        {
            Ch3_1S1 = 0x00;
            Ch3_1S2 = 0x01;
            Ch4_1S1 = 0x00;
            Ch4_1S2 = 0x01;
        }
        else if(RelayControl->nTrigSource == 3)//CH4
        {
            Ch3_1S1 = 0x01;
            Ch3_1S2 = 0x01;
            Ch4_1S1 = 0x01;
            Ch4_1S2 = 0x01;
        }
    }
    else
    {
        if(RelayControl->bCHEnable[0] == TRUE && RelayControl->bCHEnable[1] == FALSE && RelayControl->bCHEnable[2] == FALSE && RelayControl->bCHEnable[3] == FALSE)
        {
            //CH1 ��ͨ��
            Ch1_1S1 = 0x00;
            Ch1_1S2 = 0x00;
            Ch2_1S1 = 0x00;
            Ch2_1S2 = 0x00;
            Ch3_1S1 = 0x00;
            Ch3_1S2 = 0x00;
            Ch4_1S1 = 0x00;
            Ch4_1S2 = 0x00;
        }
        else if(RelayControl->bCHEnable[0] == FALSE && RelayControl->bCHEnable[1] == TRUE && RelayControl->bCHEnable[2] == FALSE && RelayControl->bCHEnable[3] == FALSE)
        {
            //CH2 ��ͨ��
            Ch1_1S1 = 0x01;
            Ch1_1S2 = 0x00;
            Ch2_1S1 = 0x01;
            Ch2_1S2 = 0x00;
            Ch3_1S1 = 0x01;
            Ch3_1S2 = 0x00;
            Ch4_1S1 = 0x01;
            Ch4_1S2 = 0x00;
        }
        else if(RelayControl->bCHEnable[0] == FALSE && RelayControl->bCHEnable[1] == FALSE && RelayControl->bCHEnable[2] == TRUE && RelayControl->bCHEnable[3] == FALSE)
        {
            //CH3 ��ͨ��
            Ch1_1S1 = 0x00;
            Ch1_1S2 = 0x01;
            Ch2_1S1 = 0x00;
            Ch2_1S2 = 0x01;
            Ch3_1S1 = 0x00;
            Ch3_1S2 = 0x01;
            Ch4_1S1 = 0x00;
            Ch4_1S2 = 0x01;
        }
        else if(RelayControl->bCHEnable[0] == FALSE && RelayControl->bCHEnable[1] == FALSE && RelayControl->bCHEnable[2] == FALSE && RelayControl->bCHEnable[3] == TRUE)
        {
            //CH4 ��ͨ��
            Ch1_1S1 = 0x01;
            Ch1_1S2 = 0x01;
            Ch2_1S1 = 0x01;
            Ch2_1S2 = 0x01;
            Ch3_1S1 = 0x01;
            Ch3_1S2 = 0x01;
            Ch4_1S1 = 0x01;
            Ch4_1S2 = 0x01;
        }
        else
        {
            if(RelayControl->bCHEnable[0] == TRUE)//���CH1��
            {
                Ch1_1S1 = 0x00;
                Ch1_1S2 = 0x00;
                if(RelayControl->bCHEnable[1] == FALSE)//���CH2�رգ���ϲ���CH1
                {
                    Ch2_1S1 = 0x00;
                    Ch2_1S2 = 0x00;
                }
                else
                {
                    Ch2_1S1 = 0x01;
                    Ch2_1S2 = 0x00;
                }
            }
            else
            {
                if(RelayControl->bCHEnable[1] == TRUE)//���CH1�رգ�CH2�򿪣���CH1�ϲ���CH2
                {
                    Ch1_1S1 = 0x01;
                    Ch1_1S2 = 0x00;
                    Ch2_1S1 = 0x01;
                    Ch2_1S2 = 0x00;
                }
                else
                {
                    Ch1_1S1 = 0x00;
                    Ch1_1S2 = 0x00;
                    Ch2_1S1 = 0x01;
                    Ch2_1S2 = 0x00;
                }
            }
            //CH3+CH4
            if(RelayControl->bCHEnable[2] == TRUE)//���CH3��
            {
                Ch3_1S1 = 0x00;
                Ch3_1S2 = 0x01;
                if(RelayControl->bCHEnable[3] == FALSE)//���CH4�رգ���ϲ���CH3
                {
                    Ch4_1S1 = 0x00;
                    Ch4_1S2 = 0x01;
                }
                else
                {
                    Ch4_1S1 = 0x01;
                    Ch4_1S2 = 0x01;
                }
            }
            else
            {
                if(RelayControl->bCHEnable[3] == TRUE)//���CH3�رգ�CH4�򿪣���CH3�ϲ���CH4
                {
                    Ch3_1S1 = 0x01;
                    Ch3_1S2 = 0x01;
                    Ch4_1S1 = 0x01;
                    Ch4_1S2 = 0x01;
                }
                else
                {
                    Ch3_1S1 = 0x00;
                    Ch3_1S2 = 0x01;
                    Ch4_1S1 = 0x01;
                    Ch4_1S2 = 0x01;
                }
            }
        }
    }
    //Trigger Source ����4051 10.12.10 zht
    if(RelayControl->nTrigSource == 0)//CH1
    {
        Trigsel1 = 0x01;
        Trigsel2 = 0x01;
        Trigsel3 = 0x00;
        ExtTrigControl = 0x01;//����EXT�̵���
    }
    else if(RelayControl->nTrigSource == 1)//CH2
    {
        Trigsel1 = 0x00;
        Trigsel2 = 0x00;
        Trigsel3 = 0x00;
        ExtTrigControl = 0x01;//����EXT�̵���
    }
    else if(RelayControl->nTrigSource == 2)//CH3
    {
        Trigsel1 = 0x01;
        Trigsel2 = 0x00;
        Trigsel3 = 0x00;
        ExtTrigControl = 0x01;//����EXT�̵���
    }
    else if(RelayControl->nTrigSource == 3)//CH4
    {
        Trigsel1 = 0x00;
        Trigsel2 = 0x01;
        Trigsel3 = 0x00;
        ExtTrigControl = 0x01;//����EXT�̵���
    }
    else// if(RelayControl->nTrigSource == 4)//EXT(EXT/10)
    {
        Trigsel1 = 0x00;
        Trigsel2 = 0x00;
        Trigsel3 = 0x01;
        ExtTrigControl = 0;//����EXT�̵���
    }
    //VoltDIV
    /*Chx_2S1,Chx_2S2(ATT 1,2,5):00,5V;
                                                                 01,2V;
                                                                 10,1V;
                                                                 11,NC
                                                                                */
    if(RelayControl->bCHEnable[0] == TRUE)
    {
        if(RelayControl->nCHVoltDIV[0] == 0 || RelayControl->nCHVoltDIV[0] == 3 || RelayControl->nCHVoltDIV[0] == 6 ||RelayControl->nCHVoltDIV[0] == 9)//10mv,100mv,1v,10v
        {
            Ch1_2S1 = 0x00;
            Ch1_2S2 = 0x01;
        }
        else if(RelayControl->nCHVoltDIV[0] == 1 || RelayControl->nCHVoltDIV[0] == 4 || RelayControl->nCHVoltDIV[0] == 7||RelayControl->nCHVoltDIV[0] == 10)//20mv,200mv,2v,20v
        {
            Ch1_2S1 = 0x01;
            Ch1_2S2 = 0x00;
        }
        else if(RelayControl->nCHVoltDIV[0] == 2 || RelayControl->nCHVoltDIV[0] == 5 || RelayControl->nCHVoltDIV[0] == 8)//50mv,500mv,5v
        {
            Ch1_2S1 = 0x00;
            Ch1_2S2 = 0x00;
        }
        else
        {
            Ch1_2S1 = 0x01;
            Ch1_2S2 = 0x01;
        }
    }
    if(RelayControl->bCHEnable[1] == TRUE)
    {
        if(RelayControl->nCHVoltDIV[1] == 0 || RelayControl->nCHVoltDIV[1] == 3 || RelayControl->nCHVoltDIV[1] == 6 || RelayControl->nCHVoltDIV[1] == 9)//10mv,100mv,1v,10v
        {
            Ch2_2S1 = 0x00;
            Ch2_2S2 = 0x01;
        }
        else if(RelayControl->nCHVoltDIV[1] == 1 || RelayControl->nCHVoltDIV[1] == 4 || RelayControl->nCHVoltDIV[1] == 7 || RelayControl->nCHVoltDIV[1] == 10)//20mv,200mv,2v,20v
        {
            Ch2_2S1 = 0x01;
            Ch2_2S2 = 0x00;
        }
        else if(RelayControl->nCHVoltDIV[1] == 2 || RelayControl->nCHVoltDIV[1] == 5 || RelayControl->nCHVoltDIV[1] == 8)//50mv,500mv,5v
        {
            Ch2_2S1 = 0x00;
            Ch2_2S2 = 0x00;
        }
        else
        {
            Ch2_2S1 = 0x01;
            Ch2_2S2 = 0x01;
        }
    }
    if(RelayControl->bCHEnable[2] == TRUE)
    {
        if(RelayControl->nCHVoltDIV[2] == 0 || RelayControl->nCHVoltDIV[2] == 3 || RelayControl->nCHVoltDIV[2] == 6 || RelayControl->nCHVoltDIV[2] == 9)//10mv,100mv,1v,10v
        {
            Ch3_2S1 = 0x00;
            Ch3_2S2 = 0x01;
        }
        else if(RelayControl->nCHVoltDIV[2] == 1 || RelayControl->nCHVoltDIV[2] == 4 || RelayControl->nCHVoltDIV[2] == 7 || RelayControl->nCHVoltDIV[2] == 10)//20mv,200mv,2v,20v
        {
            Ch3_2S1 = 0x01;
            Ch3_2S2 = 0x00;
        }
        else if(RelayControl->nCHVoltDIV[2] == 2 || RelayControl->nCHVoltDIV[2] == 5 || RelayControl->nCHVoltDIV[2] == 8)//50mv,500mv,5v
        {
            Ch3_2S1 = 0x00;
            Ch3_2S2 = 0x00;
        }
        else
        {
            Ch3_2S1 = 0x01;
            Ch3_2S2 = 0x01;
        }
    }
    if(RelayControl->bCHEnable[3] == TRUE)
    {
        if(RelayControl->nCHVoltDIV[3] == 0 || RelayControl->nCHVoltDIV[3] == 3 || RelayControl->nCHVoltDIV[3] == 6 || RelayControl->nCHVoltDIV[3] == 9)//10mv,100mv,1v,10v
        {
            Ch4_2S1 = 0x00;
            Ch4_2S2 = 0x01;
        }
        else if(RelayControl->nCHVoltDIV[3] == 1 || RelayControl->nCHVoltDIV[3] == 4 || RelayControl->nCHVoltDIV[3] == 7 || RelayControl->nCHVoltDIV[3] == 10)//20mv,200mv,2v,20v
        {
            Ch4_2S1 = 0x01;
            Ch4_2S2 = 0x00;
        }
        else if(RelayControl->nCHVoltDIV[3] == 2 || RelayControl->nCHVoltDIV[3] == 5 || RelayControl->nCHVoltDIV[3] == 8)//50mv,500mv,5v
        {
            Ch4_2S1 = 0x00;
            Ch4_2S2 = 0x00;
        }
        else
        {
            Ch4_2S1 = 0x01;
            Ch4_2S2 = 0x01;
        }
    }
    //x100,10x
    // �̵���˥������ 09.12.14 zht
    if(RelayControl->bCHEnable[0] == TRUE)
    {
        if(RelayControl->nCHVoltDIV[0]<3)// AC/DC
        {
            Ch1_1d100=0x00;
            Ch1_1d10=0x00;
        }
        else if(RelayControl->nCHVoltDIV[0]<6)// 100mv/50mvģʽ
        {
            Ch1_1d100=0x00;
            Ch1_1d10=0x01;
        }
        else// if(RelayControl->nCHVoltDIV[0]<12)// 1v/500mvģʽ
        {
            Ch1_1d100=0x01;
            Ch1_1d10=0x01;
        }
    }

    if(RelayControl->bCHEnable[1] == TRUE)
    {
        if(RelayControl->nCHVoltDIV[1]<3)
        {
            Ch2_1d100=0x00;
            Ch2_1d10=0x00;
        }
        else if(RelayControl->nCHVoltDIV[1]<6)
        {
            Ch2_1d100=0x00;
            Ch2_1d10=0x01;
        }/*
                else if(RelayControl->nCHVoltDIV[1]<9)
                {
                        Ch2_1d100=0x01;
                        Ch2_1d10=0x00;
                }*/
        else// if(RelayControl->nCHVoltDIV[1]<12)
        {
            Ch2_1d100=0x01;
            Ch2_1d10=0x01;
        }
    }

    if(RelayControl->bCHEnable[2] == TRUE)
    {
        if(RelayControl->nCHVoltDIV[2]<3)
        {
            Ch3_1d100=0x00;
            Ch3_1d10=0x00;
        }
        else if(RelayControl->nCHVoltDIV[2]<6)
        {
            Ch3_1d100=0x00;
            Ch3_1d10=0x01;
        }/*
                else if(RelayControl->nCHVoltDIV[2]<9)
                {
                        Ch3_1d100=0x01;
                        Ch3_1d10=0x00;
                }*/
        else// if(RelayControl->nCHVoltDIV[2]<12)
        {
            Ch3_1d100=0x01;
            Ch3_1d10=0x01;
        }
    }

    if(RelayControl->bCHEnable[3] == TRUE)
    {
        if(RelayControl->nCHVoltDIV[3]<3)
        {
            Ch4_1d100=0x00;
            Ch4_1d10=0x00;
        }
        else if(RelayControl->nCHVoltDIV[3]<6)
        {
            Ch4_1d100=0x00;
            Ch4_1d10=0x01;
        }/*
                else if(RelayControl->nCHVoltDIV[3]<9)
                {
                        Ch4_1d100=0x01;
                        Ch4_1d10=0x00;
                }*/
        else// if(RelayControl->nCHVoltDIV[3]<12)
        {
            Ch4_1d100=0x01;
            Ch4_1d10=0x01;
        }
    }
    /*****************************************end***************************************************/ //yt20090927


    //��ʼ���豸��
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    UINT m_nSize=2;
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x15;
    outBuffer[1]=0x00;
    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status =  DeviceIoControl (hOutDevice,
                               IOCTL_EZUSB_BULK_WRITE,
                               (PVOID)&outBulkControl,
                               sizeof(BULK_TRANSFER_CONTROL),
                               outBuffer,//���������
                               m_nSize,//�ֽ������ڶԻ����п�������
                               &BytesReturned,//�����ֽ�����
                               //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                               NULL);
#endif

    //�ر��豸
    free(outBuffer);
    CloseDevice(hOutDevice);

    /////
    PUCHAR inBuffer = NULL;
    BULK_TRANSFER_CONTROL   inBulkControl;
    BOOL bResult=FALSE;

    if (PCUSBSpeed(DeviceIndex))
    {
        m_nSize = 512;
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
    inBulkControl.pipeNum = 1;
#ifdef _ASY
    status = TransferDataFromDevice(inBuffer,m_nSize);
#else
    //��ʼʱ��
    status = DeviceIoControl(hInDevice,
                             IOCTL_EZUSB_BULK_READ,
                             (PVOID)&inBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             (LPVOID)inBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                             NULL);
#endif
    if(status != 0)
    {
        DelayCurrentState=inBuffer[0];
        DelayCurrentState=inBuffer[1]*256+DelayCurrentState;
    }
    else
    {
        DelayCurrentState = 0;
    }
    //�ر��豸
    free(inBuffer);//�ͷŻ�����
    CloseDevice(hInDevice);
    /*
     //=========================test1=============================================================//
Ch1_1S1=0x00;Ch1_1S2=0x00;Ch2_1S1=0x01;Ch2_1S2=0x00;Ch3_1S1=0x00;Ch3_1S2=0x01;Ch4_1S1=0x01;Ch4_1S2=0x01;
Trigsel1=0x01;Trigsel2=0x01;Trigsel3=0x00;Trigsel4=0x01;FiltCh1=0x00;FiltCh2=0x00;FiltCh3=0x00;FiltCh4=0x01;
Ch1_2S1=0x01;Ch1_2S2=0x01;Ch2_2S1=0x00;Ch2_2S2=0x00;Ch3_2S1=0x01;Ch3_2S2=0x01;Ch4_2S1=0x00;Ch4_2S2=0x01;
//=========================test1=============================================================//
*/
    /*
     //=========================test2=============================================================//
Ch1_1d100=0x00;Ch1_1d10=0x00;Ch1_ACDC=0x00;Ch2_1d100=0x00;Ch2_1d10=0x00;Ch2_ACDC=0x00;Ch3_1d100=0x00;Ch3_1d10=0x00;
Ch3_ACDC=0x00;Ch4_1d100=0x00;Ch4_1d10=0x00;Ch4_ACDC=0x00;ExtTrigControl=0x00;

*/

    /*
FILE *fp;
if((fp=fopen("f:\\cmd.txt","a+"))!=NULL)
{
        fprintf(fp,"%d     ",DelayCurrentState);
        fclose(fp);
}

*/

    //=========================test1=============================================================//

    long OtherControlData=0;
    int DelayNewControlData,DelayDownloadData;//  10011111000000 0010 0101
    OtherControlData=0xffffff&(((Ch1_1S1&0x01)<<23)|
                               ((Ch1_1S2&0x01)<<22)|
                               ((Ch2_1S1&0x01)<<21)|
                               ((Ch2_1S2&0x01)<<20)|
                               ((Ch3_1S1&0x01)<<19)|
                               ((Ch3_1S2&0x01)<<18)|
                               ((Ch4_1S1&0x01)<<17)|
                               ((Ch4_1S2&0x01)<<16)|
                               ((Trigsel1&0x01)<<15)|
                               ((Trigsel2&0x01)<<14)|
                               ((Trigsel3&0x01)<<13)|
                               ((Trigsel4&0x01)<<12)|
                               ((FiltCh1&0x01)<<11)|
                               ((FiltCh2&0x01)<<10)|
                               ((FiltCh3&0x01)<<9)|
                               ((FiltCh4&0x01)<<8)|
                               ((Ch1_2S1&0x01)<<7)|
                               ((Ch1_2S2&0x01)<<6)|
                               ((Ch2_2S1&0x01)<<5)|
                               ((Ch2_2S2&0x01)<<4)|
                               ((Ch3_2S1&0x01)<<3)|
                               ((Ch3_2S2&0x01)<<2)|
                               ((Ch4_2S1&0x01)<<1)|
                               ((Ch4_2S2&0x01)<<0)
                               );

#ifdef _LAUNCH
    // �������С��
    DelayNewControlData=0xffff&(((Ch1_1d100&0x01)<<15)|
                                ((Ch1_1d10&0x01)<<14)|
                                ((Ch1_ACDC&0x01)<<13)| // ���Ƶ�һͨ���̵���
                                ((Ch2_1d100&0x01)<<12)
                                |((Ch2_1d10&0x01)<<11)|
                                ((Ch2_ACDC&0x01)<<10)|	// ���Ƶڶ�ͨ���̵���
                                ((0x01&0x01)<<9)|
                                ((0x01&0x01)<<8)|
                                // ����һ��ͨ���̵���������
                                ((Ch3_1d100&0x01)<<7)|
                                ((Ch3_1d10&0x01)<<6)|
                                ((Ch3_ACDC&0x01)<<5)|	// ���Ƶ����̵���
                                ((Ch4_1d100&0x01)<<4)|
                                ((Ch4_1d10&0x01)<<3)|
                                ((Ch4_ACDC&0x01)<<2)|	// ���Ƶ��ļ̵���
                                ((ExtTrigControl&0x01)<<1)|
                                ((0X01&0x01)<<0)					   		// ���ƴ����̵����͹�����
                                );
#else
    //OtherControlData=0xffffff&0xCD;
    DelayNewControlData=0x3fff&(((Ch1_1d100&0x01)<<13)|((Ch1_1d10&0x01)<<12)
                                |((Ch1_ACDC&0x01)<<11)|((Ch2_1d100&0x01)<<10)|((Ch2_1d10&0x01)<<9)|((Ch2_ACDC&0x01)<<8)
                                |((Ch3_1d100&0x01)<<7)|((Ch3_1d10&0x01)<<6)|((Ch3_ACDC&0x01)<<5)|((Ch4_1d100&0x01)<<4)
                                |((Ch4_1d10&0x01)<<3)|((Ch4_ACDC&0x01)<<2)|((ExtTrigControl&0x01)<<1)|((0X01&0x01)<<0)
                                );

#endif
    //WORD DatTemp,
    WORD t;										// �̵������ݴ�����ʱ����
    WORD Runtemp1,Runtemp2;								// �������м̵���״̬��ʱֵ
    unsigned char i=0;												// �ֲ���ʱ���������ڼ�¼ѭ������
    //unsigned char flag;												// �ֲ���ʱ���������ڱ�Ǽ̵��������Ƿ��������
    unsigned char num=16;// С��num=16,���num=13; zht 09/10/29 13:50

#ifdef _LAUNCH
    //С��
    Runtemp1 = DelayCurrentState;							// ��ֵ
    Runtemp2 = DelayNewControlData;
#else
    //���
    Runtemp1 = DelayCurrentState>>1;							// ��ֵ 09/10/19 13:51 zht
    Runtemp2 = DelayNewControlData>>1;
#endif
    t=0x0001;											// ͨ�����ж���ζԼ̵�������
    SelectDeviceIndex(DeviceIndex,pcDriverName);//��ʼ���豸��
    m_nSize=10;
    while( i < num )									//
    {
        ResetDevice(DeviceIndex,1);
        hOutDevice = OpenDevice(pcDriverName);
        if(hOutDevice == NULL)
        {
            return 0;
        }
        outBuffer=(PUCHAR) malloc(m_nSize);
        outBulkControl.pipeNum=0;//�˵�ѡ��EP2
        // ��λ�Ƚϲ������жϺ��ֵ���͵��̵�����
        // (DelayCurrentState & 0x0100 ) == 0 ) ��С����ж�����
#ifdef _LAUNCH														// (DelayCurrentState & 0x0001 ) == 0 ) �Ǵ����ж����� 09/10/19 13:52 zht
        //С��
        if(((DelayCurrentState & 0x0100 ) == 0 )|| (( Runtemp1 & 0x0001 ) != ( Runtemp2 & 0x0001 ) ))
        {
            if( ( Runtemp2 & t ) == 0x0001 )
            {
                DelayDownloadData = t<<(i);
            }
            else
            {
                DelayDownloadData =~(t<<(i));
            }
#else
        //���
        if(((DelayCurrentState & 0x0001 ) == 0 )|| (( Runtemp1 & 0x0001 ) != ( Runtemp2 & 0x0001 ) ))
        {
            if( ( Runtemp2 & t ) == 0x0001 )
            {
                DelayDownloadData = t<<(3+i);
            }
            else
            {
                DelayDownloadData =~(t<<(3+i));
            }
#endif
            //==================================�������====================================================//
            /*	SelectDeviceIndex(DeviceIndex,pcDriverName);//��ʼ���豸��
                                m_nSize=10;
                                ResetDevice(DeviceIndex,1);
                                hOutDevice = OpenDevice(pcDriverName);
                                if(hOutDevice == NULL)
                                {
                                        return 0;
                                }
                                outBuffer=(PUCHAR) malloc(m_nSize);
                                */
            outBuffer[0]=0x12;
            outBuffer[1]=0x00;
            outBuffer[2]=0xff&(unsigned char)DelayDownloadData;
            outBuffer[3]=0xff&(unsigned char)(DelayDownloadData>>8);
            outBuffer[4]=0xff&(unsigned char)(OtherControlData) ;
            outBuffer[5]=0xff&(unsigned char)(OtherControlData>>8) ;
            outBuffer[6]=0xff&(unsigned char)(OtherControlData>>16) ;
            outBuffer[7]= 0x00;
            outBuffer[8]=0xff&(unsigned char)(DelayNewControlData) ;
            outBuffer[9]=0xff&(unsigned char)(DelayNewControlData>>8) ;
            status=FALSE;
            BytesReturned=0;
            //	outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
            status = TransferDataToDevice(outBuffer,m_nSize);
            if(!status)
            {
                i = num;
            }
#else

            status = DeviceIoControl(hOutDevice,
                                     IOCTL_EZUSB_BULK_WRITE,
                                     (PVOID)&outBulkControl,
                                     sizeof(BULK_TRANSFER_CONTROL),
                                     outBuffer,//���������
                                     m_nSize,//�ֽ������ڶԻ����п�������
                                     &BytesReturned,//�����ֽ�����
                                     //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                                     NULL);
#endif
            //	free(outBuffer);
            //	CloseDevice(hOutDevice);	//�ر��豸
            //==================================�������====================================================//

            Sleep(20);								// ϵͳ��ʱ20ms,����FPGA���Ƽ̵�������
        }
        Runtemp1=Runtemp1 >>1;
        Runtemp2=Runtemp2 >>1;
        i++;
        free(outBuffer);
        CloseDevice(hOutDevice);	//�ر��豸
    } // while
    /*
if((fp=fopen("f:\\cmd.txt","a+"))!=NULL)
{
        fprintf(fp,"%d     %d\n",DelayNewControlData,OtherControlData);
        fclose(fp);
}
*/
    //==================================�������====================================================//

    SelectDeviceIndex(DeviceIndex,pcDriverName);//��ʼ���豸��
    m_nSize=10;

    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }

    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x12;
    outBuffer[1]=0x00;
    outBuffer[2]=0xff&(unsigned char)0x00;
    outBuffer[3]=0xff&(unsigned char)(0x00>>8);
    outBuffer[4]=0xff&(unsigned char)(OtherControlData) ;
    outBuffer[5]=0xff&(unsigned char)(OtherControlData>>8) ;
    outBuffer[6]=0xff&(unsigned char)(OtherControlData>>16) ;
    outBuffer[7]= 0x00;
    outBuffer[8]=0xff&(unsigned char)(DelayNewControlData) ;
    outBuffer[9]=0xff&(unsigned char)(DelayNewControlData>>8) ;
    status=FALSE;
    BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status = DeviceIoControl(hOutDevice,
                             IOCTL_EZUSB_BULK_WRITE,
                             (PVOID)&outBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             outBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                             NULL);
#endif
    free(outBuffer);
    CloseDevice(hOutDevice);//�ر��豸
    //==================================�������====================================================//

    return status;//DelayCurrentState;
}


WORD SetAboutInputChannelAndTrigger_Only_DSO3074(WORD DeviceIndex,PRELAYCONTROL RelayControl,USHORT nDirect)//�������ô���Դ�͸�Ƶ����
{
    unsigned char Ch1_1S1,Ch1_1S2,Ch2_1S1,Ch2_1S2,Ch3_1S1,Ch3_1S2,Ch4_1S1,Ch4_1S2;//
    unsigned char Trigsel1,Trigsel2,Trigsel3,Trigsel4,FiltCh1,FiltCh2,FiltCh3,FiltCh4;
    unsigned char Ch1_2S1,Ch1_2S2,Ch2_2S1,Ch2_2S2,Ch3_2S1,Ch3_2S2,Ch4_2S1,Ch4_2S2;
    unsigned char Ch1_1d100,Ch1_1d10,Ch2_1d100,Ch2_1d10,Ch3_1d100,Ch3_1d10;
    unsigned char Ch3_ACDC,Ch4_1d100,Ch4_1d10,Ch4_ACDC,ExtTrigControl;//,Relayn;
    unsigned char Ch1_ACDC,Ch2_ACDC;
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    BULK_TRANSFER_CONTROL   outBulkControl;
    HANDLE hOutDevice=NULL,hInDevice=NULL;

    //Chx_1S1,Chx_1S2(AD Sourse):00,ch1;01,ch2;10,ch3;11,ch4
    //Trigsel1,Trigsel2,Trigsel3(Triger Sourse):000,ch2;001,ch3;010,ch4;011,ch1;100,EXT TRIG
    //Trigsel4(Triger FILT)
    //CHANNEL FILT:FiltCh1,FiltCh2,FiltCh3,FiltCh4
    //Chx_2S1,Chx_2S2(ATT 1,2,5):00,5V;01,2V;10,1V;11,NC

    /*****************************************Start***************************************************/ //yt20090927
    Trigsel4 = RelayControl->bTrigFilt;
    FiltCh1 = RelayControl->bCHBWLimit[0];
    FiltCh2 = RelayControl->bCHBWLimit[1];
    FiltCh3 = RelayControl->bCHBWLimit[2];
    FiltCh4 = RelayControl->bCHBWLimit[3];
    Ch1_ACDC = (unsigned char)RelayControl->nCHCoupling[0];
    Ch2_ACDC = (unsigned char)RelayControl->nCHCoupling[1];
    Ch3_ACDC = (unsigned char)RelayControl->nCHCoupling[2];
    Ch4_ACDC = (unsigned char)RelayControl->nCHCoupling[3];
    ExtTrigControl = 0x00;
    //CH1+CH2

    if(RelayControl->nALT == 1)//����
    {
        if(RelayControl->nTrigSource == 0)//CH1
        {
            Ch1_1S1 = 0x00;
            Ch1_1S2 = 0x00;
            Ch2_1S1 = 0x00;
            Ch2_1S2 = 0x00;
        }
        else if(RelayControl->nTrigSource == 1)//CH2
        {
            Ch1_1S1 = 0x01;
            Ch1_1S2 = 0x00;
            Ch2_1S1 = 0x01;
            Ch2_1S2 = 0x00;
        }
        else if(RelayControl->nTrigSource == 2)//CH3
        {
            Ch3_1S1 = 0x00;
            Ch3_1S2 = 0x01;
            Ch4_1S1 = 0x00;
            Ch4_1S2 = 0x01;
        }
        else if(RelayControl->nTrigSource == 3)//CH4
        {
            Ch3_1S1 = 0x01;
            Ch3_1S2 = 0x01;
            Ch4_1S1 = 0x01;
            Ch4_1S2 = 0x01;
        }
    }
    else
    {
        if(RelayControl->bCHEnable[0] == TRUE && RelayControl->bCHEnable[1] == FALSE && RelayControl->bCHEnable[2] == FALSE && RelayControl->bCHEnable[3] == FALSE)
        {
            //CH1 ��ͨ��
            Ch1_1S1 = 0x00;
            Ch1_1S2 = 0x00;
            Ch2_1S1 = 0x00;
            Ch2_1S2 = 0x00;
            Ch3_1S1 = 0x00;
            Ch3_1S2 = 0x00;
            Ch4_1S1 = 0x00;
            Ch4_1S2 = 0x00;
        }
        else if(RelayControl->bCHEnable[0] == FALSE && RelayControl->bCHEnable[1] == TRUE && RelayControl->bCHEnable[2] == FALSE && RelayControl->bCHEnable[3] == FALSE)
        {
            //CH2 ��ͨ��
            Ch1_1S1 = 0x01;
            Ch1_1S2 = 0x00;
            Ch2_1S1 = 0x01;
            Ch2_1S2 = 0x00;
            Ch3_1S1 = 0x01;
            Ch3_1S2 = 0x00;
            Ch4_1S1 = 0x01;
            Ch4_1S2 = 0x00;
        }
        else if(RelayControl->bCHEnable[0] == FALSE && RelayControl->bCHEnable[1] == FALSE && RelayControl->bCHEnable[2] == TRUE && RelayControl->bCHEnable[3] == FALSE)
        {
            //CH3 ��ͨ��
            Ch1_1S1 = 0x00;
            Ch1_1S2 = 0x01;
            Ch2_1S1 = 0x00;
            Ch2_1S2 = 0x01;
            Ch3_1S1 = 0x00;
            Ch3_1S2 = 0x01;
            Ch4_1S1 = 0x00;
            Ch4_1S2 = 0x01;
        }
        else if(RelayControl->bCHEnable[0] == FALSE && RelayControl->bCHEnable[1] == FALSE && RelayControl->bCHEnable[2] == FALSE && RelayControl->bCHEnable[3] == TRUE)
        {
            //CH4 ��ͨ��
            Ch1_1S1 = 0x01;
            Ch1_1S2 = 0x01;
            Ch2_1S1 = 0x01;
            Ch2_1S2 = 0x01;
            Ch3_1S1 = 0x01;
            Ch3_1S2 = 0x01;
            Ch4_1S1 = 0x01;
            Ch4_1S2 = 0x01;
        }
        else
        {
            if(RelayControl->bCHEnable[0] == TRUE)//���CH1��
            {
                Ch1_1S1 = 0x00;
                Ch1_1S2 = 0x00;
                if(RelayControl->bCHEnable[1] == FALSE)//���CH2�رգ���ϲ���CH1
                {
                    Ch2_1S1 = 0x00;
                    Ch2_1S2 = 0x00;
                }
                else
                {
                    Ch2_1S1 = 0x01;
                    Ch2_1S2 = 0x00;
                }
            }
            else
            {
                if(RelayControl->bCHEnable[1] == TRUE)//���CH1�رգ�CH2�򿪣���CH1�ϲ���CH2
                {
                    Ch1_1S1 = 0x01;
                    Ch1_1S2 = 0x00;
                    Ch2_1S1 = 0x01;
                    Ch2_1S2 = 0x00;
                }
                else
                {
                    Ch1_1S1 = 0x00;
                    Ch1_1S2 = 0x00;
                    Ch2_1S1 = 0x01;
                    Ch2_1S2 = 0x00;
                }
            }
            //CH3+CH4
            if(RelayControl->bCHEnable[2] == TRUE)//���CH3��
            {
                Ch3_1S1 = 0x00;
                Ch3_1S2 = 0x01;
                if(RelayControl->bCHEnable[3] == FALSE)//���CH4�رգ���ϲ���CH3
                {
                    Ch4_1S1 = 0x00;
                    Ch4_1S2 = 0x01;
                }
                else
                {
                    Ch4_1S1 = 0x01;
                    Ch4_1S2 = 0x01;
                }
            }
            else
            {
                if(RelayControl->bCHEnable[3] == TRUE)//���CH3�رգ�CH4�򿪣���CH3�ϲ���CH4
                {
                    Ch3_1S1 = 0x01;
                    Ch3_1S2 = 0x01;
                    Ch4_1S1 = 0x01;
                    Ch4_1S2 = 0x01;
                }
                else
                {
                    Ch3_1S1 = 0x00;
                    Ch3_1S2 = 0x01;
                    Ch4_1S1 = 0x01;
                    Ch4_1S2 = 0x01;
                }
            }
        }
    }
    //Trigger Source ����4051 10.12.10 zht
    if(RelayControl->nTrigSource == 0)//CH1
    {
        Trigsel1 = 0x01;
        Trigsel2 = 0x01;
        Trigsel3 = 0x00;
        ExtTrigControl = 0x01;//����EXT�̵���
    }
    else if(RelayControl->nTrigSource == 1)//CH2
    {
        Trigsel1 = 0x00;
        Trigsel2 = 0x00;
        Trigsel3 = 0x00;
        ExtTrigControl = 0x01;//����EXT�̵���
    }
    else if(RelayControl->nTrigSource == 2)//CH3
    {
        Trigsel1 = 0x01;
        Trigsel2 = 0x00;
        Trigsel3 = 0x00;
        ExtTrigControl = 0x01;//����EXT�̵���
    }
    else if(RelayControl->nTrigSource == 3)//CH4
    {
        Trigsel1 = 0x00;
        Trigsel2 = 0x01;
        Trigsel3 = 0x00;
        ExtTrigControl = 0x01;//����EXT�̵���
    }
    else// if(RelayControl->nTrigSource == 4)//EXT(EXT/10)
    {
        Trigsel1 = 0x00;
        Trigsel2 = 0x00;
        Trigsel3 = 0x01;
        ExtTrigControl = 0;//����EXT�̵���
    }
    //VoltDIV
    /*Chx_2S1,Chx_2S2(ATT 1,2,5):00,5V;
                                                                 01,2V;
                                                                 10,1V;
                                                                 11,NC
                                                                                */
    if(RelayControl->bCHEnable[0] == TRUE)
    {
        if(RelayControl->nCHVoltDIV[0] == 0 || RelayControl->nCHVoltDIV[0] == 3 || RelayControl->nCHVoltDIV[0] == 6 ||RelayControl->nCHVoltDIV[0] == 9)//10mv,100mv,1v,10v
        {
            Ch1_2S1 = 0x00;
            Ch1_2S2 = 0x01;
        }
        else if(RelayControl->nCHVoltDIV[0] == 1 || RelayControl->nCHVoltDIV[0] == 4 || RelayControl->nCHVoltDIV[0] == 7||RelayControl->nCHVoltDIV[0] == 10)//20mv,200mv,2v,20v
        {
            Ch1_2S1 = 0x01;
            Ch1_2S2 = 0x00;
        }
        else if(RelayControl->nCHVoltDIV[0] == 2 || RelayControl->nCHVoltDIV[0] == 5 || RelayControl->nCHVoltDIV[0] == 8)//50mv,500mv,5v
        {
            Ch1_2S1 = 0x00;
            Ch1_2S2 = 0x00;
        }
        else
        {
            Ch1_2S1 = 0x01;
            Ch1_2S2 = 0x01;
        }
    }
    if(RelayControl->bCHEnable[1] == TRUE)
    {
        if(RelayControl->nCHVoltDIV[1] == 0 || RelayControl->nCHVoltDIV[1] == 3 || RelayControl->nCHVoltDIV[1] == 6 || RelayControl->nCHVoltDIV[1] == 9)//10mv,100mv,1v,10v
        {
            Ch2_2S1 = 0x00;
            Ch2_2S2 = 0x01;
        }
        else if(RelayControl->nCHVoltDIV[1] == 1 || RelayControl->nCHVoltDIV[1] == 4 || RelayControl->nCHVoltDIV[1] == 7 || RelayControl->nCHVoltDIV[1] == 10)//20mv,200mv,2v,20v
        {
            Ch2_2S1 = 0x01;
            Ch2_2S2 = 0x00;
        }
        else if(RelayControl->nCHVoltDIV[1] == 2 || RelayControl->nCHVoltDIV[1] == 5 || RelayControl->nCHVoltDIV[1] == 8)//50mv,500mv,5v
        {
            Ch2_2S1 = 0x00;
            Ch2_2S2 = 0x00;
        }
        else
        {
            Ch2_2S1 = 0x01;
            Ch2_2S2 = 0x01;
        }
    }
    if(RelayControl->bCHEnable[2] == TRUE)
    {
        if(RelayControl->nCHVoltDIV[2] == 0 || RelayControl->nCHVoltDIV[2] == 3 || RelayControl->nCHVoltDIV[2] == 6 || RelayControl->nCHVoltDIV[2] == 9)//10mv,100mv,1v,10v
        {
            Ch3_2S1 = 0x00;
            Ch3_2S2 = 0x01;
        }
        else if(RelayControl->nCHVoltDIV[2] == 1 || RelayControl->nCHVoltDIV[2] == 4 || RelayControl->nCHVoltDIV[2] == 7 || RelayControl->nCHVoltDIV[2] == 10)//20mv,200mv,2v,20v
        {
            Ch3_2S1 = 0x01;
            Ch3_2S2 = 0x00;
        }
        else if(RelayControl->nCHVoltDIV[2] == 2 || RelayControl->nCHVoltDIV[2] == 5 || RelayControl->nCHVoltDIV[2] == 8)//50mv,500mv,5v
        {
            Ch3_2S1 = 0x00;
            Ch3_2S2 = 0x00;
        }
        else
        {
            Ch3_2S1 = 0x01;
            Ch3_2S2 = 0x01;
        }
    }
    if(RelayControl->bCHEnable[3] == TRUE)
    {
        if(RelayControl->nCHVoltDIV[3] == 0 || RelayControl->nCHVoltDIV[3] == 3 || RelayControl->nCHVoltDIV[3] == 6 || RelayControl->nCHVoltDIV[3] == 9)//10mv,100mv,1v,10v
        {
            Ch4_2S1 = 0x00;
            Ch4_2S2 = 0x01;
        }
        else if(RelayControl->nCHVoltDIV[3] == 1 || RelayControl->nCHVoltDIV[3] == 4 || RelayControl->nCHVoltDIV[3] == 7 || RelayControl->nCHVoltDIV[3] == 10)//20mv,200mv,2v,20v
        {
            Ch4_2S1 = 0x01;
            Ch4_2S2 = 0x00;
        }
        else if(RelayControl->nCHVoltDIV[3] == 2 || RelayControl->nCHVoltDIV[3] == 5 || RelayControl->nCHVoltDIV[3] == 8)//50mv,500mv,5v
        {
            Ch4_2S1 = 0x00;
            Ch4_2S2 = 0x00;
        }
        else
        {
            Ch4_2S1 = 0x01;
            Ch4_2S2 = 0x01;
        }
    }
    //x100,10x
    // �̵���˥������ 09.12.14 zht
    if(RelayControl->bCHEnable[0] == TRUE)
    {
        if(RelayControl->nCHVoltDIV[0]<3)// AC/DC
        {
            Ch1_1d100=0x00;
            Ch1_1d10=0x00;
        }
        else if(RelayControl->nCHVoltDIV[0]<6)// 100mv/50mvģʽ
        {
            Ch1_1d100=0x00;
            Ch1_1d10=0x01;
        }
        else// if(RelayControl->nCHVoltDIV[0]<12)// 1v/500mvģʽ
        {
            Ch1_1d100=0x01;
            Ch1_1d10=0x01;
        }
    }

    if(RelayControl->bCHEnable[1] == TRUE)
    {
        if(RelayControl->nCHVoltDIV[1]<3)
        {
            Ch2_1d100=0x00;
            Ch2_1d10=0x00;
        }
        else if(RelayControl->nCHVoltDIV[1]<6)
        {
            Ch2_1d100=0x00;
            Ch2_1d10=0x01;
        }/*
                else if(RelayControl->nCHVoltDIV[1]<9)
                {
                        Ch2_1d100=0x01;
                        Ch2_1d10=0x00;
                }*/
        else// if(RelayControl->nCHVoltDIV[1]<12)
        {
            Ch2_1d100=0x01;
            Ch2_1d10=0x01;
        }
    }

    if(RelayControl->bCHEnable[2] == TRUE)
    {
        if(RelayControl->nCHVoltDIV[2]<3)
        {
            Ch3_1d100=0x00;
            Ch3_1d10=0x00;
        }
        else if(RelayControl->nCHVoltDIV[2]<6)
        {
            Ch3_1d100=0x00;
            Ch3_1d10=0x01;
        }/*
                else if(RelayControl->nCHVoltDIV[2]<9)
                {
                        Ch3_1d100=0x01;
                        Ch3_1d10=0x00;
                }*/
        else// if(RelayControl->nCHVoltDIV[2]<12)
        {
            Ch3_1d100=0x01;
            Ch3_1d10=0x01;
        }
    }

    if(RelayControl->bCHEnable[3] == TRUE)
    {
        if(RelayControl->nCHVoltDIV[3]<3)
        {
            Ch4_1d100=0x00;
            Ch4_1d10=0x00;
        }
        else if(RelayControl->nCHVoltDIV[3]<6)
        {
            Ch4_1d100=0x00;
            Ch4_1d10=0x01;
        }/*
                else if(RelayControl->nCHVoltDIV[3]<9)
                {
                        Ch4_1d100=0x01;
                        Ch4_1d10=0x00;
                }*/
        else// if(RelayControl->nCHVoltDIV[3]<12)
        {
            Ch4_1d100=0x01;
            Ch4_1d10=0x01;
        }
    }
    /*****************************************end***************************************************/ //yt20090927

    //����ƴ��H595�Ĵ����ı�ʶ
    unsigned char H595_U54,H595_U55,H595_U56,H595_U57,H595_U58;
    //H595_U54:CH1,  H595_U55:CH2,  H595_U56:CH3,  H595_U57:CH4  H595_U58:Ƭѡ

    H595_U54 = 0xFF & ( ((Trigsel1&0x01)<<7)|
                        ((FiltCh1&0x01)<<6)|
                        ((Ch1_2S2&0x01)<<5)|
                        ((Ch1_2S1&0x01)<<4)|
                        ((Ch1_ACDC&0x01)<<2)| // ���Ƶ�һͨ���̵���
                        ((Ch1_1d10&0x01)<<1)|
                        ((Ch1_1d100&0x01)<<0)
                        );
    H595_U55 = 0xFF & ( ((Trigsel2&0x01)<<7)|
                        ((FiltCh2&0x01)<<6)|
                        ((Ch2_2S2&0x01)<<5)|
                        ((Ch2_2S1&0x01)<<4)|
                        ((Ch2_ACDC&0x01)<<2)| // ���Ƶڶ�ͨ���̵���
                        ((Ch2_1d10&0x01)<<1)|
                        ((Ch2_1d100&0x01)<<0)
                        );
    H595_U56 = 0xFF & ( ((Trigsel3&0x01)<<7)|
                        ((FiltCh3&0x01)<<6)|
                        ((Ch3_2S2&0x01)<<5)|
                        ((Ch3_2S1&0x01)<<4)|
                        ((Ch3_ACDC&0x01)<<2)| // ���Ƶ���ͨ���̵���
                        ((Ch3_1d10&0x01)<<1)|
                        ((Ch3_1d100&0x01)<<0)
                        );
    H595_U57 = 0xFF & ( ((Trigsel4&0x01)<<7)|
                        ((FiltCh4&0x01)<<6)|
                        ((Ch4_2S2&0x01)<<5)|
                        ((Ch4_2S1&0x01)<<4)|
                        ((ExtTrigControl&0x01)<<3)|
                        ((Ch4_ACDC&0x01)<<2)| // ���Ƶ���ͨ���̵���
                        ((Ch4_1d10&0x01)<<1)|
                        ((Ch4_1d100&0x01)<<0)
                        );
    H595_U58 = 0xFF & ( ((Ch1_1S1&0x01)<<7)|
                        ((Ch1_1S2&0x01)<<6)|
                        ((Ch2_1S1&0x01)<<5)|
                        ((Ch2_1S2&0x01)<<4)|
                        ((Ch3_1S1&0x01)<<3)|
                        ((Ch3_1S2&0x01)<<2)|
                        ((Ch4_1S1&0x01)<<1)|
                        ((Ch4_1S2&0x01)<<0)
                        );

    USHORT m_nSize=0;
    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;

    //================================�µļ̵�������=====================================//
    //����˳��RL11��RL12��RL13��RL21��RL22��RL23��RL31��RL32��RL33��RL41��RL42��RL43��RL44���ɵ͵��ߣ���13λ
    USHORT DelayNewState = 0x1FFF & ( ((Ch1_1d100&0x01)<<0) |
                                      ((Ch1_1d10&0x01)<<1) |
                                      ((Ch1_ACDC&0x01)<<2) |
                                      ((Ch2_1d100&0x01)<<3) |
                                      ((Ch2_1d10&0x01)<<4) |
                                      ((Ch2_ACDC&0x01)<<5) |
                                      ((Ch3_1d100&0x01)<<6) |
                                      ((Ch3_1d10&0x01)<<7) |
                                      ((Ch3_ACDC&0x01)<<8) |
                                      ((Ch4_1d100&0x01)<<9) |
                                      ((Ch4_1d10&0x01)<<10) |
                                      ((Ch4_ACDC&0x01)<<11) |
                                      ((ExtTrigControl&0x01)<<12)
                                      );

    //=================================��ȡ�豸��ǰ�̵�����״̬==================================================//

    USHORT DelayCurrentState = 0;		//13���̵�����״̬

    //��ʼ���豸��
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    m_nSize=2;
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x15;
    outBuffer[1]=0x00;
    status=FALSE;
    BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status =  DeviceIoControl (hOutDevice,
                               IOCTL_EZUSB_BULK_WRITE,
                               (PVOID)&outBulkControl,
                               sizeof(BULK_TRANSFER_CONTROL),
                               outBuffer,//���������
                               m_nSize,//�ֽ������ڶԻ����п�������
                               &BytesReturned,//�����ֽ�����
                               //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                               NULL);
#endif

    //�ر��豸
    free(outBuffer);
    CloseDevice(hOutDevice);

    /////
    PUCHAR inBuffer = NULL;
    BULK_TRANSFER_CONTROL   inBulkControl;
    BOOL bResult=FALSE;

    if (PCUSBSpeed(DeviceIndex))
    {
        m_nSize = 512;
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
    inBulkControl.pipeNum = 1;
#ifdef _ASY
    status = TransferDataFromDevice(inBuffer,m_nSize);
#else
    //��ʼʱ��
    status = DeviceIoControl(hInDevice,
                             IOCTL_EZUSB_BULK_READ,
                             (PVOID)&inBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             (LPVOID)inBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                             NULL);
#endif
    if(status != 0)
    {
        DelayCurrentState=inBuffer[0];
        DelayCurrentState=inBuffer[1]*256+DelayCurrentState;
    }
    else
    {
        DelayCurrentState = 0;
    }
    //�ر��豸
    free(inBuffer);//�ͷŻ�����
    CloseDevice(hInDevice);

    //=================================ѭ�������л��̵�������===================================================//

    m_nSize=10;
    unsigned char H595_CH1,H595_CH2,H595_CH3,H595_CH4;
    for(USHORT i=0;i<13;i++)
    {
        if(0 == nDirect)
        {
            //�����̵�����ӦΪ���䣬�Ͳ���Ҫ�·���
            if(((DelayNewState>>i)&0x01) == ((DelayCurrentState>>i)&0x01))
            {
                continue;
            }
        }

        SelectDeviceIndex(DeviceIndex,pcDriverName);//��ʼ���豸��
        ResetDevice(DeviceIndex,1);
        hOutDevice = OpenDevice(pcDriverName);
        if(hOutDevice == NULL)
        {
            return 0;
        }
        outBuffer=(PUCHAR) malloc(m_nSize);
        outBulkControl.pipeNum=0;//�˵�ѡ��EP2

        H595_CH1=H595_U54;
        H595_CH2=H595_U55;
        H595_CH3=H595_U56;
        H595_CH4=H595_U57;
        // ��λ�Ƚϲ������жϺ��ֵ���͵��̵�����
        switch(i)
        {
        case 0:
        {
            if(0x01 == (0x01&Ch1_1d100))		//RL11
            {
                H595_CH1 = 0x01;	H595_CH2 = 0x00;	H595_CH3 = 0x00;	H595_CH4 = 0x00;
            }
            else
            {
                H595_CH1 = 0xFE;	H595_CH2 = 0xFF;	H595_CH3 = 0xFF;	H595_CH4 = 0xFF;
            }
        }
            break;
        case 1:
        {
            if(0x01 == (0x01&Ch1_1d10))			//RL12
            {
                H595_CH1 = 0x02;	H595_CH2 = 0x00;	H595_CH3 = 0x00;	H595_CH4 = 0x00;
            }
            else
            {
                H595_CH1 = 0xFD;	H595_CH2 = 0xFF;	H595_CH3 = 0xFF;	H595_CH4 = 0xFF;
            }
        }
            break;
        case 2:
            if(0x01 == (0x01&Ch1_ACDC))			//RL13
            {
                H595_CH1 = 0x04;	H595_CH2 = 0x00;	H595_CH3 = 0x00;	H595_CH4 = 0x00;
            }
            else
            {
                H595_CH1 = 0xFB;	H595_CH2 = 0xFF;	H595_CH3 = 0xFF;	H595_CH4 = 0xFF;
            }
            break;
        case 3:
        {
            if(0x01 == (0x01&Ch2_1d100))		//RL21
            {
                H595_CH1 = 0x00;	H595_CH2 = 0x01;	H595_CH3 = 0x00;	H595_CH4 = 0x00;
            }
            else
            {
                H595_CH1 = 0xFF;	H595_CH2 = 0xFE;	H595_CH3 = 0xFF;	H595_CH4 = 0xFF;
            }
        }
            break;
        case 4:
        {
            if(0x01 == (0x01&Ch2_1d10))			//RL22
            {
                H595_CH1 = 0x00;	H595_CH2 = 0x02;	H595_CH3 = 0x00;	H595_CH4 = 0x00;
            }
            else
            {
                H595_CH1 = 0xFF;	H595_CH2 = 0xFD;	H595_CH3 = 0xFF;	H595_CH4 = 0xFF;
            }
        }
            break;
        case 5:
        {
            if(0x01 == (0x01&Ch2_ACDC))			//RL23
            {
                H595_CH1 = 0x00;	H595_CH2 = 0x04;	H595_CH3 = 0x00;	H595_CH4 = 0x00;
            }
            else
            {
                H595_CH1 = 0xFF;	H595_CH2 = 0xFB;	H595_CH3 = 0xFF;	H595_CH4 = 0xFF;
            }
        }
            break;
        case 6:
        {
            if(0x01 == (0x01&Ch3_1d100))		//RL31
            {
                H595_CH1 = 0x00;	H595_CH2 = 0x00;	H595_CH3 = 0x01;	H595_CH4 = 0x00;
            }
            else
            {
                H595_CH1 = 0xFF;	H595_CH2 = 0xFF;	H595_CH3 = 0xFE;	H595_CH4 = 0xFF;
            }
        }
            break;
        case 7:
        {
            if(0x01 == (0x01&Ch3_1d10))			//RL32
            {
                H595_CH1 = 0x00;	H595_CH2 = 0x00;	H595_CH3 = 0x02;	H595_CH4 = 0x00;
            }
            else
            {
                H595_CH1 = 0xFF;	H595_CH2 = 0xFF;	H595_CH3 = 0xFD;	H595_CH4 = 0xFF;
            }
        }
            break;
        case 8:
        {
            if(0x01 == (0x01&Ch3_ACDC))			//RL33
            {
                H595_CH1 = 0x00;	H595_CH2 = 0x00;	H595_CH3 = 0x04;	H595_CH4 = 0x00;
            }
            else
            {
                H595_CH1 = 0xFF;	H595_CH2 = 0xFF;	H595_CH3 = 0xFB;	H595_CH4 = 0xFF;
            }
        }
            break;
        case 9:
        {
            if(0x01 == (0x01&Ch4_1d100))		//RL41
            {
                H595_CH1 = 0x00;	H595_CH2 = 0x00;	H595_CH3 = 0x00;	H595_CH4 = 0x01;
            }
            else
            {
                H595_CH1 = 0xFF;	H595_CH2 = 0xFF;	H595_CH3 = 0xFF;	H595_CH4 = 0xFE;
            }
        }
            break;
        case 10:
        {
            if(0x01 == (0x01&Ch4_1d10))			//RL42
            {
                H595_CH1 = 0x00;	H595_CH2 = 0x00;	H595_CH3 = 0x00;	H595_CH4 = 0x02;
            }
            else
            {
                H595_CH1 = 0xFF;	H595_CH2 = 0xFF;	H595_CH3 = 0xFF;	H595_CH4 = 0xFD;
            }
        }
            break;
        case 11:
        {
            if(0x01 == (0x01&Ch4_ACDC))			//RL43
            {
                H595_CH1 = 0x00;	H595_CH2 = 0x00;	H595_CH3 = 0x00;	H595_CH4 = 0x04;
            }
            else
            {
                H595_CH1 = 0xFF;	H595_CH2 = 0xFF;	H595_CH3 = 0xFF;	H595_CH4 = 0xFB;
            }
        }
            break;
        case 12:
        {

            if(0x01 == (0x01&ExtTrigControl))			//RL44
            {
                H595_CH1 = 0x00;	H595_CH2 = 0x00;	H595_CH3 = 0x00;	H595_CH4 = 0x08;
            }
            else
            {
                H595_CH1 = 0xFF;	H595_CH2 = 0xFF;	H595_CH3 = 0xFF;	H595_CH4 = 0xF7;
            }
        }
            break;
        }

        outBuffer[0]=0x12;
        outBuffer[1]=0x00;
        outBuffer[2]=H595_U58;
        outBuffer[3]=(H595_U57&0xF0) | (H595_CH4&0x0F);
        outBuffer[4]=(H595_U56&0xF0) | (H595_CH3&0x0F);
        outBuffer[5]=(H595_U55&0xF0) | (H595_CH2&0x0F);
        outBuffer[6]=(H595_U54&0xF0) | (H595_CH1&0x0F);
        outBuffer[7]=0x00;
        outBuffer[8]=0x00;
        outBuffer[9]=0x00;
        status=FALSE;
        BytesReturned=0;
        //	outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
        status = TransferDataToDevice(outBuffer,m_nSize);
        if(!status)
        {
            i = num;
        }
#else

        status = DeviceIoControl(hOutDevice,
                                 IOCTL_EZUSB_BULK_WRITE,
                                 (PVOID)&outBulkControl,
                                 sizeof(BULK_TRANSFER_CONTROL),
                                 outBuffer,//���������
                                 m_nSize,//�ֽ������ڶԻ����п�������
                                 &BytesReturned,//�����ֽ�����
                                 //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                                 NULL);
#endif
        //	free(outBuffer);
        //	CloseDevice(hOutDevice);	//�ر��豸
        //==================================�������====================================================//

        Sleep(20);								// ϵͳ��ʱ20ms,����FPGA���Ƽ̵�������

        free(outBuffer);
        CloseDevice(hOutDevice);	//�ر��豸
    }



    //==================================�������(����״̬)====================================================//

    SelectDeviceIndex(DeviceIndex,pcDriverName);//��ʼ���豸��
    m_nSize=10;
    status=FALSE;
    BytesReturned=0;

    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }

    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x12;
    outBuffer[1]=0x00;
    outBuffer[2]=H595_U58;
    outBuffer[3]=H595_U57;
    outBuffer[4]=H595_U56;
    outBuffer[5]=H595_U55;
    outBuffer[6]=H595_U54;
    outBuffer[7]=0x00;
    outBuffer[8]=0xFF&DelayNewState;
    outBuffer[9]=0xFF&(DelayNewState>>8);
    status=FALSE;
    BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status = DeviceIoControl(hOutDevice,
                             IOCTL_EZUSB_BULK_WRITE,
                             (PVOID)&outBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             outBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                             NULL);
#endif
    free(outBuffer);
    CloseDevice(hOutDevice);//�ر��豸
    //==================================�������====================================================//

    return status;//DelayCurrentState;

    return 0;
}

WORD        CaptureStart(WORD DeviceIndex,WORD nStartControl)
{
    PUCHAR outBuffer = NULL;
    UINT m_nSize=4;
    BOOL status=FALSE;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x03;
    outBuffer[1]=0x00;
    outBuffer[2]=0xFF&(nStartControl&0x07);   //Outbuffer[2]��0λ     nor_auto//0     1 Ϊ�Զ�ģʽ    0 Ϊ����ģʽ    Outbuffer[2]��1λ     scan_sel,//1     1 ����ģʽ    0�ǹ���ģʽ
    outBuffer[3]=0x00;
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
    /*  ���˵������
    //                                                      ��С��Ҫ���ϴ�����

    PUCHAR inBuffer = NULL;
    m_nSize = PCUSBSpeed(DeviceIndex)?USB_PACK_SIZE:64;
    inBuffer=(PUCHAR) malloc(m_nSize);
    memset(inBuffer, 0,m_nSize);
    status=getInBuffer(DeviceIndex,m_nSize,inBuffer);
    free(inBuffer);//�ͷŻ�����
    */
    return status;
}

WORD InitSDRam(WORD DeviceIndex)
{

    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    BULK_TRANSFER_CONTROL outBulkControl;
    HANDLE hOutDevice=NULL;
    UINT m_nSize=2;

    //��ʼ���豸��
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR)malloc(m_nSize);
    outBuffer[0]=0x18;
    outBuffer[1]=0x00;
    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status = DeviceIoControl(hOutDevice,
                             IOCTL_EZUSB_BULK_WRITE,
                             (PVOID)&outBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             outBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             NULL);	//����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
#endif
    //�ر��豸
    free(outBuffer);
    CloseDevice(hOutDevice);
    return status;
}

WORD TriggerEnabled(WORD DeviceIndex)
{
    return TRUE;
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    BULK_TRANSFER_CONTROL outBulkControl;
    HANDLE hOutDevice=NULL;
    UINT m_nSize=2;

    //��ʼ���豸��
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x04;
    outBuffer[1]=0x00;
    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status = DeviceIoControl(hOutDevice,
                             IOCTL_EZUSB_BULK_WRITE,
                             (PVOID)&outBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             outBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                             NULL);
#endif
    //�ر��豸
    free(outBuffer);
    CloseDevice(hOutDevice);
    return status;
}

BOOL ForceTrigger(WORD DeviceIndex)
{
    return TRUE;
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    BULK_TRANSFER_CONTROL outBulkControl;
    HANDLE hOutDevice=NULL;
    UINT m_nSize=2;

    TriggerEnabled(DeviceIndex);
    //��ʼ���豸��
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x02;
    outBuffer[1]=0x00;
    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status =  DeviceIoControl(	hOutDevice,
                                IOCTL_EZUSB_BULK_WRITE,
                                (PVOID)&outBulkControl,
                                sizeof(BULK_TRANSFER_CONTROL),
                                outBuffer,//���������
                                m_nSize,//�ֽ������ڶԻ����п�������
                                &BytesReturned,//�����ֽ�����
                                NULL);//����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
#endif
    //�ر��豸
    free(outBuffer);
    CloseDevice(hOutDevice);
    return status;
}

WORD GetCalData(WORD DeviceIndex,WORD* calData)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_IN;
    UCHAR VendNo=0xA2; //��дEEPROM����
    UCHAR pBuffer[8*1024];
    HANDLE hDevice;
    ULONG ncnt;
    ncnt=1;//length;//1024*4;//��ȡ24LC64�ĳ��ȣ����4K

    USHORT value = 0x1600; //��ȡ24LC64����ʼ��ַ
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice == NULL)
    {
        return 0;
    }
    bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,ncnt);
    CloseDevice(hDevice);
    unsigned long i;
    for(i=0;i<ncnt;i++)
    {
        *calData = pBuffer[i];
    }
    return 1;
}

__int64		GetStartReadAddress(WORD DeviceIndex)  //��ʵ����Ǵ�����ַ
{
    PUCHAR outBuffer = NULL;
    UINT m_nSize=2;
    //BOOLEAN status=FALSE;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x0D;
    outBuffer[1]=0x00;
    sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
    m_nSize=PCUSBSpeed(DeviceIndex)?USB_PACK_SIZE:64;
    PUCHAR inBuffer=NULL;
    __int64 temp=0;
    inBuffer=(PUCHAR) malloc(m_nSize);
    memset(inBuffer, 0,m_nSize);
    getInBuffer(DeviceIndex,m_nSize,inBuffer);
	__int64  temp2=UCHAR(inBuffer[4])*0x100000000;
    temp+=((ULONG)(inBuffer[0]))
		+((ULONG)(inBuffer[1]))*0x100
		+((ULONG)(inBuffer[2]))*0x10000
		+((ULONG)(inBuffer[3]))*0x1000000;//��λΪ�ɼ�jieshu ��λΪ
	temp=temp+temp2;
	//temp=inBuffer[2]+(((ULONG)(inBuffer[3]))<<8);
    //��ʼʱ��
    free(inBuffer);//�ͷŻ�����
    return temp;
}

ULONG GetEndReadAddress(WORD DeviceIndex)//��ʵ�����End��ַ
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BULK_TRANSFER_CONTROL   outBulkControl;
    BULK_TRANSFER_CONTROL   inBulkControl;
    HANDLE hOutDevice=NULL,hInDevice=NULL;
    PUCHAR outBuffer = NULL;
    PUCHAR inBuffer = NULL;
    UINT m_nSize;
    ULONG SdramAddrTriged = 0;
    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;

    //��ʼ���豸��
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    //��ʼ��ȡ SdramAddrTriged ֵ.................................................
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    m_nSize=4;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x7F;//READ SdramAddrTriged
    outBuffer[1]=0x01;
    outBuffer[2]=0x04;//READ SdramAddrTriged
    outBuffer[3]=0x00;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status = DeviceIoControl(hOutDevice,
                             IOCTL_EZUSB_BULK_WRITE,
                             (PVOID)&outBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             outBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                             NULL);
#endif
    free(outBuffer);
    CloseDevice(hOutDevice);//�ر��豸

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
    inBuffer=(PUCHAR)malloc(m_nSize);
    memset(inBuffer,0,m_nSize);
    int nReadTimes = USB_PACK_SIZE / m_nSize;
    inBulkControl.pipeNum = 1;
    for(int i=0;i<nReadTimes;i++)
    {
#ifdef _ASY
        status = TransferDataFromDevice(inBuffer,m_nSize);
#else
        status = DeviceIoControl(hInDevice,
                                 IOCTL_EZUSB_BULK_READ,
                                 (PVOID)&inBulkControl,
                                 sizeof(BULK_TRANSFER_CONTROL),
                                 (LPVOID)inBuffer,//���������
                                 m_nSize,//�ֽ������ڶԻ����п�������
                                 &BytesReturned,//�����ֽ�����
                                 //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                                 NULL);
#endif
        if(status==0)
        {
            break;
        }
        if(i==0)
        {
            //��������(������SdramAddrTriged)
            //SdramAddrTriged=inBuffer[2];
            //SdramAddrTriged=SdramAddrTriged*256*256;
            //SdramAddrTriged=SdramAddrTriged+inBuffer[1]*256+inBuffer[0];
            SdramAddrTriged=inBuffer[0];
        }
    }

    free(inBuffer);//�ͷŻ�����
    CloseDevice(hInDevice); //�ر��豸
    //������ȡ SdramAddrTriged ֵ.................................................
    return SdramAddrTriged;  //need modified //Error
}

/*
int SetReadAddressAndLength(WORD DeviceIndex,ULONG RamReadStartAddr,ULONG RamReadLength)
{
        char pcDriverName[MAX_DRIVER_NAME] = "";
        BULK_TRANSFER_CONTROL   outBulkControl;
        HANDLE hOutDevice=NULL;
        PUCHAR outBuffer = NULL;
    UINT m_nSize;
        int status;
        ULONG BytesReturned=0;
//д����ʼ��ַ�����ݳ���.......................................................
        //��ʼ���豸��
        SelectDeviceIndex(DeviceIndex,pcDriverName);
        m_nSize=14;
        ResetDevice(DeviceIndex,1);
        hOutDevice = OpenDevice(pcDriverName);
        if(hOutDevice == NULL)
        {
                return 0;
        }
        outBuffer=(PUCHAR)malloc(m_nSize);
        outBuffer[0]=0x13;
        outBuffer[1]=0x00;
        //��ȡ������ʼ��ַ
        RamReadStartAddr = RamReadStartAddr - 1;
        outBuffer[2]=0xff&(unsigned char)RamReadStartAddr;
        outBuffer[3]=0xff&(unsigned char)(RamReadStartAddr>>8);
        outBuffer[4]=0xff&(unsigned char)(RamReadStartAddr>>16);
        outBuffer[5]=0xff&(unsigned char)(RamReadStartAddr>>24);
        outBuffer[6]=(0xff&(unsigned char)(RamReadStartAddr>>32));//SdramRdBegin
        outBuffer[7]=0x80;
        //��ȡ���ݳ���
        outBuffer[8]=0xff&(unsigned char)RamReadLength;
        outBuffer[9]=0xff&(unsigned char)(RamReadLength>>8);
        outBuffer[10]=0xff&(unsigned char)(RamReadLength>>16);
        outBuffer[11]=0xff&(unsigned char)(RamReadLength>>24);
        outBuffer[12]=0xff&(unsigned char)(RamReadLength>>32);
        outBuffer[13]=0x00;
        outBulkControl.pipeNum=0;//�˵�ѡ��EP2
        status = DeviceIoControl(hOutDevice,
                                                        IOCTL_EZUSB_BULK_WRITE,
                                                        (PVOID)&outBulkControl,
                                                        sizeof(BULK_TRANSFER_CONTROL),
                                                        outBuffer,//���������
                                                        m_nSize,//�ֽ������ڶԻ����п�������
                                                        &BytesReturned,//�����ֽ�����
                                                        //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                                                        NULL);

        //�ر��豸
        free(outBuffer);
        CloseDevice(hOutDevice);
//����д��.........................................................................
        return status;
}
*/

int SetReadAddress(WORD DeviceIndex,ULONG RamReadStartAddr)
{
    PUCHAR outBuffer = NULL;
    UINT m_nSize=4;
    WORD status=FALSE;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x0E;
    outBuffer[1]=0x00;
    outBuffer[2]=0xFF&(unsigned char)(RamReadStartAddr&0xFF);
    outBuffer[3]=0xFF&(unsigned char)((RamReadStartAddr>>8));
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
    return status;
}

int SetReadLength(WORD DeviceIndex,ULONG RamReadLength)
{
    return 1;
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BULK_TRANSFER_CONTROL   outBulkControl;
    HANDLE hOutDevice=NULL;
    PUCHAR outBuffer = NULL;
    UINT m_nSize;
    int status;
    ULONG BytesReturned=0;

    //��ʼ���豸��
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    m_nSize=8;
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR)malloc(m_nSize);
    outBuffer[0]=0x1B;
    outBuffer[1]=0x00;
    //�������ݳ���
    outBuffer[2]=0xff&(unsigned char)RamReadLength;
    outBuffer[3]=0xff&(unsigned char)(RamReadLength>>8);
    outBuffer[4]=0xff&(unsigned char)(RamReadLength>>16);
    outBuffer[5]=0xff&(unsigned char)(RamReadLength>>24);
    outBuffer[6]=0x00;//0xff&(unsigned char)(RamReadLength>>32);
    outBuffer[7]=0x00;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status = DeviceIoControl(hOutDevice,
                             IOCTL_EZUSB_BULK_WRITE,
                             (PVOID)&outBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             outBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                             NULL);
#endif

    //�ر��豸
    free(outBuffer);
    CloseDevice(hOutDevice);
    return status;
}

WORD        ReadHardData_6104(WORD DeviceIndex,PWORD *CHData,int nCHNum,ULONG nReadLen)
{
    if(nCHNum*nReadLen==0)
        return 0;
    ULONG i,j;
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR inBuffer = NULL,outBuffer;
    HANDLE  hInDevice=NULL;
    WORD status=FALSE;
    UINT m_nSize;
    ULONG nReadTimes = 0;
	
/*	if(nReadLen%512)
	{
		nReadLen=((ULONG(nReadLen/512)+1))*512;
	}
	*/
	ULONG  nSend=nReadLen/2;
    //׼����ȡ����
    //��ʼ���豸��
    m_nSize=4;
    outBuffer=(PUCHAR)malloc(m_nSize);
    outBuffer[0]=0x05;
    outBuffer[1]=0x00;
    outBuffer[2]=unsigned char(0xFF&(nSend));
    outBuffer[3]=unsigned char(0xFF&(nSend>>8));
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);	//�ر��豸
    //��ʼ��ȡ����
    m_nSize=PCUSBSpeed(DeviceIndex)?USB_PACK_SIZE:64;
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    hInDevice = OpenDevice(pcDriverName);
    if(hInDevice == NULL)
    {
        return 0;
    }
    inBuffer=(PUCHAR)malloc(m_nSize);
    memset(inBuffer, 0,m_nSize);
	UINT nOffset=0;
	nOffset=nReadLen % m_nSize;	
    nReadTimes = nReadLen / m_nSize+(nOffset?1:0);
    USHORT nStep = (m_nSize / nCHNum);
    double temp=0;
    for(i=0;i<nReadTimes;i++) //���ѭ������
    {
        status=getInBufferWithoutOpen(hInDevice,m_nSize,inBuffer);
        if(!status)
            break;
        for (j=0;j<nStep;j++)
        {
            for(int l=0;l<nCHNum;l++)
            {
                temp=inBuffer[nCHNum*j+l];
                //temp=255.0*temp/199.0-35.8793+0.5;
				temp=256.0*(temp-28)/200.0+0.5;
                if(temp<0)
                    temp=0;
                if(temp>255)
                    temp=255;
                *(CHData[l]+(i*nStep+j))=(unsigned char)temp;
            }
        }
    }
    free(inBuffer);//�ͷŻ�����
    CloseDevice(hInDevice);//�ر��豸
    return status;
}

WORD ReadHardData(WORD DeviceIndex,WORD* CH1Data, WORD* CH2Data,WORD* CH3Data, WORD* CH4Data,ULONG nReadLen)
{
    ULONG i,j;
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR inBuffer = NULL,outBuffer;
    HANDLE  hInDevice=NULL;
    WORD status=FALSE;
    UINT m_nSize;
    ULONG nReadTimes = 0;

    //׼����ȡ����
    //��ʼ���豸��
    m_nSize=2;
    outBuffer=(PUCHAR)malloc(m_nSize);
    outBuffer[0]=0x05;
    outBuffer[1]=0x00;
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);	//�ر��豸


    //��ʼ��ȡ����
    m_nSize=PCUSBSpeed(DeviceIndex)?USB_PACK_SIZE:64;
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    hInDevice = OpenDevice(pcDriverName);
    if(hInDevice == NULL)
    {
        return 0;
    }
    inBuffer=(PUCHAR)malloc(m_nSize);
    memset(inBuffer, 0,m_nSize);
    nReadTimes = nReadLen / m_nSize;
    USHORT nStep = (m_nSize / MAX_CH_NUM);


    for(i=0;i<nReadTimes;i++) //���ѭ������
    {

        status=getInBufferWithoutOpen(hInDevice,m_nSize,inBuffer);

        if(!status)
            break;
        for (j=0;j<nStep;j++)
        {
            *(CH1Data+i*nStep+j) = inBuffer[MAX_CH_NUM*j+0];
            *(CH2Data+i*nStep+j) = inBuffer[MAX_CH_NUM*j+1];
            *(CH3Data+i*nStep+j) = inBuffer[MAX_CH_NUM*j+2];
            *(CH4Data+i*nStep+j) = inBuffer[MAX_CH_NUM*j+3];
        }

    }
    free(inBuffer);//�ͷŻ�����
    CloseDevice(hInDevice);//�ر��豸
    return status;
}
WORD ReadHardDataToOneBuffer(WORD DeviceIndex,WORD* AllCHData,ULONG nReadLen)   //add by zhang 20150822  AllCHData�����������ʼ����
{
#ifdef _LAUNCH_TEST
    BOOL bReading = TRUE;
#endif

    ULONG i;
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL,inBuffer = NULL;
    BULK_TRANSFER_CONTROL   outBulkControl;
    BULK_TRANSFER_CONTROL   inBulkControl;
    HANDLE hOutDevice=NULL, hInDevice=NULL;
    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    UINT m_nSize;
    ULONG nReadTimes = 0;

    //׼����ȡ����
    //��ʼ���豸��
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    m_nSize=2;
    outBuffer=(PUCHAR)malloc(m_nSize);
    outBuffer[0]=0x05;
    outBuffer[1]=0x00;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status = DeviceIoControl(hOutDevice,
                             IOCTL_EZUSB_BULK_WRITE,
                             (PVOID)&outBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             outBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                             NULL);
#endif

    free(outBuffer);	//�ر��豸
    CloseDevice(hOutDevice);
    //��ʼ��ȡ����
    if(PCUSBSpeed(DeviceIndex))
    {
        m_nSize = USB_PACK_SIZE;
    }
    else
    {
        m_nSize = 64;
    }
    //���豸
    hInDevice = OpenDevice(pcDriverName);
    if(hInDevice == NULL)
    {
        return 0;
    }
    // inBuffer=(PUCHAR)malloc(m_nSize);
    // memset(inBuffer, 0,m_nSize);
    nReadTimes = nReadLen / m_nSize;
    //USHORT nStep = (m_nSize / MAX_CH_NUM);
    inBulkControl.pipeNum = 1;//EP6IN

    for(i=0;i<nReadTimes;i++) //���ѭ������
    {
#ifdef _ASY
        //status = TransferDataFromDevice(inBuffer,m_nSize);
#else
        status = DeviceIoControl(hInDevice,
                                 IOCTL_EZUSB_BULK_READ,
                                 (PVOID)&inBulkControl,
                                 sizeof(BULK_TRANSFER_CONTROL),
                                 (LPVOID)(AllCHData+m_nSize*i),//���������
                                 m_nSize,//�ֽ������ڶԻ����п�������
                                 &BytesReturned,//�����ֽ�����
                                 NULL);//����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
#endif
#ifdef _LAUNCH_TEST
        if(BytesReturned != m_nSize)
        {
            bReading = FALSE;
            break;
        }
#endif
        if(status==0)
        {
            break;
        }
        /*
                for (j=0;j<nStep;j++)
                {
                        *(CH1Data+i*nStep+j) = inBuffer[MAX_CH_NUM*j+0];
                        *(CH2Data+i*nStep+j) = inBuffer[MAX_CH_NUM*j+1];
                        *(CH3Data+i*nStep+j) = inBuffer[MAX_CH_NUM*j+2];
                        *(CH4Data+i*nStep+j) = inBuffer[MAX_CH_NUM*j+3];
                }
                */

    }
    // free(inBuffer);//�ͷŻ�����
    CloseDevice(hInDevice);//�ر��豸

#ifdef _LAUNCH_TEST
    if(bReading)
    {
        return status;
    }
    else
    {
        return 3064;
    }
#else
    return status;
#endif
}

WORD		GetCaptureState(WORD DeviceIndex)
{
    PUCHAR outBuffer = NULL;
    UINT m_nSize=2;
    //BOOLEAN status=FALSE;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x06;
    outBuffer[1]=0x00;
    sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);

    m_nSize=PCUSBSpeed(DeviceIndex)?USB_PACK_SIZE:64;
    PUCHAR inBuffer=NULL;
    unsigned short temp=0;
    inBuffer=(PUCHAR) malloc(m_nSize);
    memset(inBuffer, 0,m_nSize);
    getInBuffer(DeviceIndex,m_nSize,inBuffer);
    temp=(inBuffer[0]&0xFF)|
		((inBuffer[1]&0xFF)<<8);
    //��ʼʱ��
    free(inBuffer);//�ͷŻ�����
    return temp;
}
/*
WORD GetLANEnable(WORD DeviceIndex)//�ж��Ƿ�������
{
        char pcDriverName[MAX_DRIVER_NAME] = "";
        PUCHAR outBuffer = NULL;
        BULK_TRANSFER_CONTROL   outBulkControl;
        HANDLE hOutDevice=NULL,hInDevice=NULL;
        //��ʼ���豸��
    UINT m_nSize=2;

        SelectDeviceIndex(DeviceIndex,pcDriverName);
        ResetDevice(DeviceIndex,1);
        hOutDevice = OpenDevice(pcDriverName);
        if(hOutDevice == NULL)
        {
                return 0;
        }
        outBuffer=(PUCHAR) malloc(m_nSize);
        outBuffer[0]=0x27;
    outBuffer[1]=0x00;
        BOOLEAN status=FALSE;
        ULONG BytesReturned=0;
        outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
        status = TransferDataToDevice(outBuffer,m_nSize);
#else
        status =  DeviceIoControl (hOutDevice,
                                                         IOCTL_EZUSB_BULK_WRITE,
                                                         (PVOID)&outBulkControl,
                                                         sizeof(BULK_TRANSFER_CONTROL),
                                                         outBuffer,//���������
                                                         m_nSize,//�ֽ������ڶԻ����п�������
                                                         &BytesReturned,//�����ֽ�����
                                                         //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                                                         NULL);
#endif
        //�ر��豸
        free(outBuffer);
        CloseDevice(hOutDevice);


    WORD LINS = 0;
    PUCHAR inBuffer = NULL;
        BULK_TRANSFER_CONTROL   inBulkControl;
        BOOL bResult=FALSE;
        //��ʼ���豸��
        //���豸
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
        int nReadTimes = USB_PACK_SIZE / m_nSize;//���ٶ�ȡ512�ֽ�
        inBulkControl.pipeNum = 1;
        //��ʼʱ��
        for(int i=0;i<nReadTimes;i++)
        {
#ifdef _ASY
                status = TransferDataFromDevice(inBuffer,m_nSize);
#else
                status = DeviceIoControl(hInDevice,
                                                                 IOCTL_EZUSB_BULK_READ,
                                                                 (PVOID)&inBulkControl,
                                                                 sizeof(BULK_TRANSFER_CONTROL),
                                                                 (LPVOID)inBuffer,//���������
                                                                 m_nSize,//�ֽ������ڶԻ����п�������
                                                                 &BytesReturned,//�����ֽ�����
                                                                 //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                                                                 NULL);
#endif
                if(status==0)
                {
                        break;
                }
                if(i==0)
                {
                //��������
                        LINS=inBuffer[1];//��2���ֽڣ���������1��û��������2
                }
        }
    //�ر��豸
        free(inBuffer);//�ͷŻ�����
        CloseDevice(hInDevice);

        return LINS;
}
*/
WORD GetFPGAVersion(WORD DeviceIndex)//
{
    PUCHAR outBuffer = NULL;

    //BOOLEAN status=FALSE;
    //��ʼ���豸��
    UINT m_nSize=2;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x0C;
    outBuffer[1]=0x00;
    sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);

    PUCHAR inBuffer = NULL;
    int temp=0;
    m_nSize=PCUSBSpeed(DeviceIndex)?USB_PACK_SIZE:64;
    inBuffer=(PUCHAR) malloc(m_nSize);
    memset(inBuffer, 0,m_nSize);
    getInBuffer(DeviceIndex,m_nSize,inBuffer);
    temp=inBuffer[0]+(inBuffer[1]<<8);
    free(inBuffer);//�ͷŻ�����
    return temp;
}


ULONG GetHardVersion(WORD DeviceIndex)
{
    PUCHAR outBuffer = NULL;


    UINT m_nSize=2;
    ULONG nVersion = 0;
    //��ʼ���豸��
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x09;
    outBuffer[1]=0x00;
    WORD status=FALSE;
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);


    PUCHAR inBuffer = NULL;

    m_nSize=PCUSBSpeed(DeviceIndex)?USB_PACK_SIZE:64;

    inBuffer=(PUCHAR) malloc(m_nSize);
    memset(inBuffer, 0,m_nSize);
    status=getInBuffer(DeviceIndex,m_nSize,inBuffer);
    nVersion=inBuffer[0]+(inBuffer[1]<<8)+(inBuffer[2]<<16)+(inBuffer[3]<<24);
    free(inBuffer);//�ͷŻ�����

    return nVersion;
}

ULONG       GetHardFandC(WORD DeviceIndex)//Ƶ�ʼ�/������
{
    PUCHAR outBuffer = NULL;
    UINT m_nSize=2;
    ULONG nFC_H = 0;
    ULONG nFC_L = 0;
    BOOL status=FALSE;
    //��ʼ���豸��
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x0A;
    outBuffer[1]=0x00;
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);

    PUCHAR inBuffer = NULL;
    m_nSize=PCUSBSpeed(DeviceIndex)?USB_PACK_SIZE:64;
    inBuffer=(PUCHAR) malloc(m_nSize);
    memset(inBuffer, 0,m_nSize);
    status=getInBuffer(DeviceIndex,m_nSize,inBuffer);
    nFC_H=inBuffer[0]|(inBuffer[1]<<8)|(inBuffer[2]<<16)|(inBuffer[3]<<24);//��ƵƵ�ʼ�����
    nFC_L=inBuffer[4]|(inBuffer[5]<<8)|(inBuffer[6]<<16)|(inBuffer[7]<<24);//��ƵƵ�ʼ�����
    free(inBuffer);//�ͷŻ�����
	//return nFC_L;
	return nFC_H<1000000?nFC_L:nFC_H;
    
    //ע�⣡nFC_L��û���ϴ�
}

WORD        SetHardFandC(WORD DeviceIndex,ULONG nTime)
{    
    PUCHAR outBuffer = NULL;
    UINT m_nSize = 6;
    BOOL status=FALSE;
    outBuffer = (PUCHAR)malloc(m_nSize);
    outBuffer[0] = 0x0B;
    outBuffer[1] = 0x00;
    outBuffer[2] = 0xff & (unsigned char)nTime;
    outBuffer[3] = 0xff & (unsigned char)(nTime>>8);
    outBuffer[4] = 0xff & (unsigned char)(nTime>>16);
    outBuffer[5] = 0xff & (unsigned char)(nTime>>24);
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
    return status;
}

WORD ResetCounter(WORD DeviceIndex,WORD nValue)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    BULK_TRANSFER_CONTROL   outBulkControl;
    HANDLE hOutDevice=NULL;
    UINT m_nSize=4;

    SelectDeviceIndex(DeviceIndex,pcDriverName);	//��ʼ���豸��
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x10;
    outBuffer[1]=0x00;
    outBuffer[2]=(nValue & 0xFF);//��λ
    outBuffer[3]=0x00;
    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status =  DeviceIoControl (hOutDevice,
                               IOCTL_EZUSB_BULK_WRITE,
                               (PVOID)&outBulkControl,
                               sizeof(BULK_TRANSFER_CONTROL),
                               outBuffer,//���������
                               m_nSize,//�ֽ������ڶԻ����п�������
                               &BytesReturned,//�����ֽ�����,����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                               NULL);
#endif
    //�ر��豸
    free(outBuffer);
    CloseDevice(hOutDevice);
    return status;
}

WORD GetChannelLevel(WORD DeviceIndex,WORD* level,WORD length)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_IN;
    UCHAR VendNo=0xA2; //��дEEPROM����
    UCHAR pBuffer[8*1024];
    HANDLE hDevice;
    ULONG ncnt,i;

    USHORT value = 0x1400+EE_OFFSET; //��ȡ24LC64����ʼ��ַ

    ncnt=length*2;//��ȡ24LC64�ĳ��ȣ����4K

    SelectDeviceIndex(DeviceIndex,pcDriverName);
    ResetDevice(DeviceIndex,1);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice == NULL)
    {
        return 0;
    }
    bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,ncnt);
    CloseDevice(hDevice);
    for(i=0;i<length;i++)
    {
        level[i]=pBuffer[2*i]+(pBuffer[2*i+1]<<8);
    }
    return 1;
}

WORD SetChannelLevel(WORD DeviceIndex,WORD* level,WORD length)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_OUT;
    UCHAR VendNo=0xa2; //��дEEPROM
    UCHAR pBuffer[8*1024];
    HANDLE hDevice = NULL;
    WORD i;
    USHORT value = 0x1400+EE_OFFSET; //��ȡ24LC64����ʼ��ַ
    for(i=0;i<length;i++)
    {
		if(i!=ZEROCALI_LEN)
		{
			pBuffer[2*i]=(level[i]+ZEROCALI_FIX_OFFSET) & 0xFF;//��8bit
			pBuffer[2*i+1]=((level[i]+ZEROCALI_FIX_OFFSET)>>8) & 0xFF;//��8bit
		}else{
			pBuffer[2*i]=(level[i]) & 0xFF;//��8bit
			pBuffer[2*i+1]=((level[i])>>8) & 0xFF;//��8bit
		}
    }

    SelectDeviceIndex(DeviceIndex,pcDriverName);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice == NULL)
    {
        return 0;
    }
    bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,length*2);
    CloseDevice(hDevice);
    return 1;
}

WORD SetRollCollect(WORD DeviceIndex,WORD nEnable)//����Roll�ɼ�ģʽ
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    BULK_TRANSFER_CONTROL   outBulkControl;
    HANDLE hOutDevice=NULL;
    UINT m_nSize=4;

    SelectDeviceIndex(DeviceIndex,pcDriverName);	//��ʼ���豸��
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR)malloc(m_nSize);
    outBuffer[0]=0x19;
    outBuffer[1]=0x00;
    outBuffer[2]=(nEnable & 0x01);
    outBuffer[3]=0x00;
    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status =  DeviceIoControl (hOutDevice,
                               IOCTL_EZUSB_BULK_WRITE,
                               (PVOID)&outBulkControl,
                               sizeof(BULK_TRANSFER_CONTROL),
                               outBuffer,//���������
                               m_nSize,//�ֽ������ڶԻ����п�������
                               &BytesReturned,//�����ֽ�����
                               NULL);
#endif
    free(outBuffer);	//�ر��豸
    CloseDevice(hOutDevice);

    return status;
}

WORD SetPeakMode(WORD DeviceIndex,WORD nOpen,ULONG nPeak)//
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    BULK_TRANSFER_CONTROL   outBulkControl;
    HANDLE hOutDevice=NULL;
    UINT m_nSize = 10;

    SelectDeviceIndex(DeviceIndex,pcDriverName);	//��ʼ���豸��
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }

    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]= 0x1A;
    outBuffer[1]= 0x00;
    outBuffer[2]= 0xff&(unsigned char)(nPeak);//��8λ
    outBuffer[3]= 0xff&(unsigned char)(nPeak>>8);//
    outBuffer[4]= 0xff&(unsigned char)(nPeak>>16);//
    outBuffer[5]= 0xff&(unsigned char)(nPeak>>24);//
    outBuffer[6]= (nOpen & 0x01);
    outBuffer[7]= 0x00;
    outBuffer[8]= 0x00;
    outBuffer[9]= 0x00;

    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status =  DeviceIoControl (hOutDevice,
                               IOCTL_EZUSB_BULK_WRITE,
                               (PVOID)&outBulkControl,
                               sizeof(BULK_TRANSFER_CONTROL),
                               outBuffer,//���������
                               m_nSize,//�ֽ������ڶԻ����п�������
                               &BytesReturned,//�����ֽ�����
                               NULL);
#endif
    free(outBuffer);
    CloseDevice(hOutDevice);//�ر��豸

    return status;
}

WORD        SetTriggerAndSyncOutput(WORD DeviceIndex,PCONTROLDATA pControl,WORD nTriggerMode, WORD nTriggerSlope,WORD nPWCondition,ULONG nPW,
                                    USHORT nVideoStandard,USHORT nVedioSyncSelect,USHORT nVideoHsyncNumOption,
                                    WORD nSync)//���ô���
{
   
   /* int timeDiv=pControl->nTimeDIV;
    unsigned short nCHEnable=pControl->nCHSet;
    unsigned short nSampelRate=0;
    unsigned short nNumen=0;
    for(int i=0;i<4;i++)
    {
        if((nCHEnable>>i))
            nNumen++;
    }
    if(timeDiv<=6)
    {
        if(nNumen==1)
        {
            nSampelRate=timeDiv<=5?0x10:01;
        }
        else if(nNumen==2)
        {
           nSampelRate=timeDiv<=6?0x01:00; 
        }

    }
*/
    WORD m_nSize=6;
    PUCHAR outBuffer=NULL;
    BOOL status=FALSE;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x11;
    outBuffer[1]=0x00;
    outBuffer[2]=0xFF&(unsigned short)(nTriggerMode);
    outBuffer[3]=0xFF&(unsigned short)(pControl->nTriggerSource);
    outBuffer[4]=0xFF&(unsigned short)(0x01);//zhang
    outBuffer[5]=0x00;
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
    return status;
}
//��ȡɨ��ģʽ��Ԥ������������
WORD GetPreTrigData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl/*,USHORT nFlag*/)
{
    WORD nRE = 0;
    WORD* pCHData[MAX_CH_NUM];
    ULONG RamReadStartAddr;//ȷ��������ʼ��ַ
    ULONG i = 0;
	WORD nStartOffset = 0;
    ULONG nPreDataLen = 0,nPriTriggerLen;//Ԥ��������ǰ���ǵ���ͨ���������ܵĳ���
	unsigned int nTriggerAddress;
    unsigned int nEndAddress;
	int nEvenOffset=0;
	nPreDataLen = ULONG(pControl->nHTriggerPos * pControl->nReadDataLen/100.0);//Ԥ��������
    //��ȡ������ַ
    __int64  nAddress = GetStartReadAddress(nDeviceIndex);
	nTriggerAddress	=WORD(nAddress&0xFFFF);               //������ַ
    nEndAddress		=WORD((nAddress>>16)&0xFFFF);             //�ɼ�������ַ
	nStartOffset	=WORD((nAddress>>32)&0xFFFF);
	PWORD ppData[4];
    int nActivateCHNum=0;//������������ͨ����Ŀ    
    {//�����nActivateCHNum����ppData[4]ָ����������Ĳ���        
        pCHData[0] = pCH1Data;
        pCHData[1] = pCH2Data;
        pCHData[2] = pCH3Data;
        pCHData[3] = pCH4Data;
        for(int m=0;m<4;m++)
        {
            if(((pControl->nCHSet)>>m)&0x01)
            {
                ppData[nActivateCHNum++]=pCHData[m];
            }
        }
        if(nActivateCHNum==3)
        {
            for(int k=0;k<4;k++)
            {
                ppData[k]=pCHData[k];
            }
            nActivateCHNum=4;
        }
    }
	
	switch (nActivateCHNum){
	case 4:
		nStartOffset=nStartOffset&0x01+6;
		break;
	case 2:
		nStartOffset=nStartOffset&0x03+4;
		break;
	case 1:
		nStartOffset=nStartOffset&0x07;
		break;
	default :
		nStartOffset=0;
		break;
	}
	nPreDataLen=nPreDataLen>=nStartOffset?nPreDataLen-nStartOffset:0;
	nStartOffset=nStartOffset*nActivateCHNum;

	nPriTriggerLen = nActivateCHNum*nPreDataLen;//Ԥ��������   �㷨����ǰ��һ������Ҫ����ͨ����//by zhang
	nEvenOffset=nPriTriggerLen&0x01;//�������ȱ�����ż��

	if((!nPreDataLen)||nPreDataLen==1)
	{
		pControl->nAlreadyReadLen = 0;
		pControl->nLastAddress=nTriggerAddress-nEvenOffset*nActivateCHNum;
		return 1;
	}
    //���ݴ�����ַ������ʼ������ַ
    RamReadStartAddr=nTriggerAddress >= nPriTriggerLen?nTriggerAddress- nPriTriggerLen:0x10000 - (nPriTriggerLen - nTriggerAddress);
	RamReadStartAddr+=nStartOffset;//ȥ����ƫ��
    SetReadAddress(nDeviceIndex,RamReadStartAddr);//�ڶ��� ���ö�ȡ��ַ

    //���ö�ȡ���Ȳ���ʼ����
    nRE  = ReadHardData_6104(nDeviceIndex,ppData,nActivateCHNum,nPriTriggerLen-nEvenOffset*nActivateCHNum);//������ ��ʼ����
    if(nRE != 0)
    {
        pControl->nAlreadyReadLen = nPreDataLen-nEvenOffset;
		pControl->nLastAddress=nTriggerAddress-nEvenOffset*nActivateCHNum;
	
    }
    return nRE;   //need modified  //Error
}
#define CONTINUE_LEN_128		128
#define DATA_STEP_32			32

WORD GetDataContinue(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl,WORD nScanRoll)
{
    ULONG i=0;
	ULONG nEndress,nReadLenScan;
	ULONG nLastAddress=pControl->nLastAddress;
	ULONG nDataLen=pControl->nReadDataLen;
    WORD nRE = 0;
    ULONG nPreLen = pControl->nAlreadyReadLen;
	int nEvenOffset=0;
	if(nPreLen==1&&nScanRoll== YT_ROLL)
		nPreLen=0;
    PWORD pCHData[MAX_CH_NUM];
    __int64  nAddress = GetStartReadAddress(nDeviceIndex);	
    nEndress=WORD((nAddress>>16)&0xFFFF);             //�ɼ�������ַ
	//ULONG nStartOffset=WORD((nAddress>>32)&0xFFFF);

	nReadLenScan=(nEndress>=pControl->nLastAddress)?(nEndress-pControl->nLastAddress):(nEndress+0x10000-pControl->nLastAddress);
	if(nReadLenScan>16096){
	//	nReadLenScan=0;//add to ROLl diyige dian
	}
	
	PWORD ppData[4];
    int nActivateCHNum=0;//������������ͨ����Ŀ    
    {//�����nActivateCHNum����ppData[4]ָ����������Ĳ���        
        pCHData[0] = pCH1Data;
        pCHData[1] = pCH2Data;
        pCHData[2] = pCH3Data;
        pCHData[3] = pCH4Data;
        for(int m=0;m<4;m++)
        {
            if(((pControl->nCHSet)>>m)&0x01)
            {
                ppData[nActivateCHNum++]=pCHData[m];
            }
        }
        if(nActivateCHNum==3)
        {
            for(int k=0;k<4;k++)
            {
                ppData[k]=pCHData[k];
            }
            nActivateCHNum=4;
        }
    }
	nReadLenScan-=nReadLenScan%nActivateCHNum; //�����Ǽ���ͨ����Ŀ��������
	
	if((!nReadLenScan)||nReadLenScan==1)
	{
		return 0;
	}
	if(nScanRoll== YT_SCAN)//SCAN ģʽ�������Ķ�ȡ���ȼ����Ѿ����ĳ��ȴ����ܵĳ��Ȳ� Ӧ���ʵ���С
	{
		nReadLenScan=(nReadLenScan/nActivateCHNum+nPreLen)>nDataLen?(nDataLen-nPreLen)*nActivateCHNum:nReadLenScan;

	}
	else
	{
		if(nReadLenScan>nDataLen)//YT-Rollģʽ���ֵ�ܶ�ȡ4K������
		{
			nReadLenScan=(nReadLenScan>nDataLen*nActivateCHNum)?nDataLen*nActivateCHNum:nReadLenScan;
			pControl->nLastAddress=(nEndress>=nDataLen*nActivateCHNum)?(nEndress-nDataLen*nActivateCHNum):(nEndress+0xFFFF-nDataLen*nActivateCHNum);//��Ӧ�Ľ���յ�ַҲӦ������һ��			
		}
	}
	if(nReadLenScan>1500){
		int xxxxxxx=0;
	}
	
	nEvenOffset=nReadLenScan%2;
    if(nScanRoll == YT_ROLL && pControl->nAlreadyReadLen == 0)//Roll��1�ζ���������������
    {
		pControl->nLastAddress=nEndress;//add zhang  ROLL��һ����
        //nRE = ReadHardData(nDeviceIndex,pCHData1[0],pCHData1[1],pCHData1[2],pCHData1[3],4*1024);
        pControl->nAlreadyReadLen = 1;//�ڴ� = 1�����������Ƿ��ǵ�1��Roll��ȡ����
        return 0;
    }
    else//���ö�������
    {     
		if(nScanRoll==YT_ROLL)
		{
			SetReadAddress(nDeviceIndex,pControl->nLastAddress+8);//add by zhang  ���ROLLģʽ��һ�����������
		}
		else{
			SetReadAddress(nDeviceIndex,pControl->nLastAddress);
		}
        nRE  = ReadHardData_6104(nDeviceIndex,ppData,nActivateCHNum,nReadLenScan-nEvenOffset*nActivateCHNum);//������ ��ʼ����
		if(pControl->nLastAddress>0xFFFF)
			pControl->nLastAddress-=0xFFFF;	
		pControl->nLastAddress=nEndress-nEvenOffset;           
		pControl->nAlreadyReadLen=nPreLen+nReadLenScan/nActivateCHNum-nEvenOffset;			
    }
    return nRE;
	
}
WORD SDGetDataContinue(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl,WORD nScanRoll)
{
    ULONG i=0;
    WORD nRE = 0;
    ULONG nPreLen = pControl->nAlreadyReadLen;
    WORD pCHData[MAX_CH_NUM][CONTINUE_LEN_128];

    if(nScanRoll == 1 && pControl->nAlreadyReadLen == 0)//Roll��1�ζ���������������
    {
        WORD pCHData1[MAX_CH_NUM][1024];
        nRE = ReadHardData(nDeviceIndex,pCHData1[0],pCHData1[1],pCHData1[2],pCHData1[3],4*1024);
        pControl->nAlreadyReadLen = 1;//�ڴ� = 1�����������Ƿ��ǵ�1��Roll��ȡ����
        return 0;
    }
    else
    {
        //���ö�������
        SetReadLength(nDeviceIndex,256);
        nRE = ReadHardData(nDeviceIndex,pCHData[0],pCHData[1],pCHData[2],pCHData[3],512);
    }
    if(nRE != 0)
    {
        if(nScanRoll == 0)//Scan
        {
            if(pControl->nTimeDIV < MIN_ROLL_TIMEDIV)
            {
                for(i=0;i<CONTINUE_LEN_128;i++)
                {
                    if((nPreLen+i) >= pControl->nReadDataLen)//����
                    {
                        pControl->nAlreadyReadLen = pControl->nReadDataLen;
                        return nRE;//
                    }
                    else
                    {
                        *(pCH1Data+nPreLen+i) = pCHData[0][i];
                        *(pCH2Data+nPreLen+i) = pCHData[1][i];
                        *(pCH3Data+nPreLen+i) = pCHData[2][i];
                        *(pCH4Data+nPreLen+i) = pCHData[3][i];
                    }
                }
                pControl->nAlreadyReadLen += CONTINUE_LEN_128;
            }
            else
            {
                WORD* pData[MAX_CH_NUM];
                pData[0] = pCH1Data+nPreLen;
                pData[1] = pCH2Data+nPreLen;
                pData[2] = pCH3Data+nPreLen;
                pData[3] = pCH4Data+nPreLen;
                if((nPreLen+DATA_STEP_32) >= pControl->nReadDataLen)//����
                {
                    for(i=0;i<MAX_CH_NUM;i++)
                    {
                        dsoSFChooseData(pCHData[i],CONTINUE_LEN_128,8,pData[i],(pControl->nReadDataLen-nPreLen));
                    }
                    pControl->nAlreadyReadLen = pControl->nReadDataLen;
                }
                else
                {
                    for(i=0;i<MAX_CH_NUM;i++)
                    {
                        dsoSFChooseData(pCHData[i],CONTINUE_LEN_128,8,pData[i],DATA_STEP_32);
                    }
                    pControl->nAlreadyReadLen += DATA_STEP_32;
                }
            }
        }
        else//Roll
        {
            ULONG i=0,j=0;
            WORD* pData[MAX_CH_NUM];
            WORD nData[MAX_CH_NUM][DATA_STEP_32];
            ULONG nDataLen = 0;

            pData[0] = pCH1Data;
            pData[1] = pCH2Data;
            pData[2] = pCH3Data;
            pData[3] = pCH4Data;

            for(i=0;i<MAX_CH_NUM;i++)
            {
                dsoSFChooseData(pCHData[i],CONTINUE_LEN_128,8,nData[i],DATA_STEP_32);
            }
            if(pControl->nAlreadyReadLen == 1)//֮ǰ�Ѿ����һ������
            {
                pControl->nAlreadyReadLen = 0;
            }
            nDataLen = pControl->nAlreadyReadLen + DATA_STEP_32;
            if(nDataLen > DEF_READ_DATA_LEN)
            {
                ULONG nOffset = nDataLen - DEF_READ_DATA_LEN;
                for(i=0;i<MAX_CH_NUM;i++)
                {
                    for(j=0;j<DEF_READ_DATA_LEN;j++)
                    {
                        if(j+nOffset < DEF_READ_DATA_LEN)
                        {
                            pData[i][j] = pData[i][j+nOffset];
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                for(i=0;i<MAX_CH_NUM;i++)
                {
                    for(j=0;j<DATA_STEP_32;j++)
                    {
                        pData[i][(DEF_READ_DATA_LEN-DATA_STEP_32)+j] = nData[i][j];
                    }
                }
                nDataLen = DEF_READ_DATA_LEN;
            }
            else
            {
                for(i=0;i<MAX_CH_NUM;i++)
                {
                    for(j=0;j<DATA_STEP_32;j++)
                    {
                        pData[i][pControl->nAlreadyReadLen+j] = nData[i][j];
                    }
                }
            }
            pControl->nAlreadyReadLen = nDataLen;
        }
    }
    return nRE;
}
///////////////////////////////////////////////ETS///////////////////////////////////////////////////
WORD ProcETSData(WORD nTimeDIV,
                 WORD * gra1_data,
                 WORD * gra2_data,
                 WORD * gra_data,
                 WORD * gengxintm,
                 WORD * TDCCalValue,
                 WORD * MaxMinValue,
                 WORD flag,
                 WORD* IsUpdateNow//�Ƿ���������
                 )
{
    int MAX_TIMES;
    int i,addr_stop,b,c;
    long L,L0;
    int j,m;
    int a=512;
    WORD MaxCalData;
    WORD MinCalData;

    WORD MaxTDCData;
    WORD MinTDCData;

    MaxTDCData = MaxMinValue[0];
    MinTDCData = MaxMinValue[1];

    MaxCalData = TDCCalValue[0];
    MinCalData = TDCCalValue[1];

    if(nTimeDIV < 2)
        MAX_TIMES=15;//2ns,4ns
    else
        MAX_TIMES=10;


    //0.2us
    if(nTimeDIV==6)
    {
        for (i=0;i<MAX_TIMES;i++)
        {/*
                        K=(*(gra_data+i*(3*a+64)+1547))*256;//��λ
                        L0=K+(*(gra_data+i*(3*a+64)+1546));//���ϵ�λ*/
            L0=*(gra_data+i*(3*a+64)+1546)+*(gra_data+i*(3*a+64)+1547)*256+*(gra_data+i*(3*a+64)+1548)*256*256+*(gra_data+i*(3*a+64)+1549)*256*256*256;

            addr_stop=(*(gra_data+i*(3*a+64)+1537))*256;
            addr_stop= addr_stop+(*(gra_data+i*(3*a+64)+1536));
            if(addr_stop>511 || addr_stop<0) L0=0;//ȥ��Ram_Addr�������
            //			Value[i]=L0;
            if(L0>MaxTDCData) L0=0;
            L0=L0-MinTDCData-MaxCalData;

            if (L0>0)
            {
                if ((MaxTDCData-MinTDCData-MaxCalData)<=0) return 0;
                L=(int)(L0/((MaxTDCData-MinTDCData-MaxCalData)/5.0));//564;//���յ�ʱ��
                /*
                                if(L>0)
                                        L=L-1;
                                else
                                        L=L+4;*/
                //	Value[i]=L;
                b=101;
                c=511;

                if(L<0) return 1;
                if(L>4) return 1;

                for (;addr_stop>=0;addr_stop=addr_stop-1)
                {
                    if (b>=0)
                    {/*
                                                if (L==0)
                                                {
                                                        *(gra1_data+5*b+0)=(*(gra1_data+5*b+1))*2-(*(gra1_data+5*b+2));
                                                        *(gra2_data+5*b+0)=(*(gra2_data+5*b+1))*2-(*(gra2_data+5*b+2));
                                                }
                                                else if(L==4)
                                                {
                                                        *(gra1_data+5*b+4)=(*(gra1_data+5*b+3))*2-(*(gra1_data+5*b+2));
                                                        *(gra2_data+5*b+4)=(*(gra2_data+5*b+3))*2-(*(gra2_data+5*b+2));
                                                }
                                                else
                                                {*/
                        *(gra1_data+5*b+L)=*(gra_data+i*(3*a+64)+addr_stop);
                        *(gra2_data+5*b+L)=*(gra_data+i*(3*a+64)+512+addr_stop);
                        //	}
                        b=b-1;
                    }
                }

                for (;b>=0;b=b-1)
                {/*
                                        if (L==0)
                                        {
                                                *(gra1_data+5*b+0)=(*(gra1_data+5*b+1))*2-(*(gra1_data+5*b+2));
                                                *(gra2_data+5*b+0)=(*(gra2_data+5*b+1))*2-(*(gra2_data+5*b+2));
                                        }
                                        else if(L==4)
                                        {
                                                *(gra1_data+5*b+4)=(*(gra1_data+5*b+3))*2-(*(gra1_data+5*b+2));
                                                *(gra2_data+5*b+4)=(*(gra2_data+5*b+3))*2-(*(gra2_data+5*b+2));
                                        }
                                        else
                                        {*/
                    *(gra1_data+5*b+L)=*(gra_data+i*(3*a+64)+c);
                    *(gra2_data+5*b+L)=*(gra_data+i*(3*a+64)+512+c);
                    //	}
                    c=c-1;
                }

                if(!flag) return 1;//������
                *(gengxintm+L)=50;
                if(*IsUpdateNow)
                {
                    for(j=0;j<5;j++)
                    {
                        *(gengxintm+j)=50;
                    }
                    for(j=0;j<5;j++)
                    {
                        for(m=0;m<100;m++)
                        {
                            *(gra1_data+5*m+j)=*(gra1_data+5*m+L);
                            *(gra2_data+5*m+j)=*(gra2_data+5*m+L);
                        }
                    }
                    *IsUpdateNow=0;
                }
                else
                {
                    for (j=0;j<5;j++)
                    {
                        gengxintm[j]--;

                        if(gengxintm[j]<6)
                        {
                            gengxintm[j] = 50;

                            for (m=0;m<100;m++)
                            {
                                if ((j==0)&&(m==0))
                                {
                                    gra1_data[0] = gra1_data[1];
                                    gra2_data[0] = gra2_data[1];
                                }
                                else if ((j==4)&&(m==99))
                                {
                                    gra1_data[499] = gra1_data[498];
                                    gra2_data[499] = gra2_data[498];
                                }
                                else
                                {
                                    gra1_data[5*m+j] = (int)((gra1_data[5*m+j-1]+gra1_data[5*m+j+1])/2);
                                    gra2_data[5*m+j] = (int)((gra2_data[5*m+j-1]+gra2_data[5*m+j+1])/2);

                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //0.1us
    if(nTimeDIV==5)
    {
        for (i=0;i<MAX_TIMES;i++)
        {
            L0=*(gra_data+i*(3*a+64)+1546)+*(gra_data+i*(3*a+64)+1547)*256+*(gra_data+i*(3*a+64)+1548)*256*256+*(gra_data+i*(3*a+64)+1549)*256*256*256;

            addr_stop=(*(gra_data+i*(3*a+64)+1537))*256;
            addr_stop=addr_stop+(*(gra_data+i*(3*a+64)+1536));
            if(addr_stop>511 || addr_stop<0) L0=0;//ȥ��Ram_Addr�������

            if (L0>MaxTDCData) L0=0;//ȥ��ʱ��������
            L0=L0-MinTDCData-MaxCalData;//���յ�ʱ��

            if (L0>0)
            {
                if((MaxTDCData-MinTDCData-MaxCalData)<=0) return 0;
                L=(int)(L0/((MaxTDCData-MinTDCData-MaxCalData)/10.0));

                b=51;
                c=511;
                /*
                                if (L>0)
                                        L=L-1;
                                else
                                {
                                        L=L+9;
                                }*/
                if(L<0) return 1;
                if(L>9) return 1;
                for (;addr_stop>=0;addr_stop--)
                {
                    if (b>=0)
                    {/*
                                                if (L==0)
                                                {
                                                        *(gra1_data+10*b+0)=(*(gra1_data+10*b+1))*2-(*(gra1_data+10*b+2));
                                                        *(gra2_data+10*b+0)=(*(gra2_data+10*b+1))*2-(*(gra2_data+10*b+2));
                                                }
                                                else if(L==9)
                                                {
                                                        *(gra1_data+10*b+9)=(*(gra1_data+10*b+8))*2-(*(gra1_data+10*b+7));
                                                        *(gra2_data+10*b+9)=(*(gra2_data+10*b+8))*2-(*(gra2_data+10*b+7));
                                                }
                                                else
                                                {*/
                        *(gra1_data+10*b+L)=*(gra_data+i*(3*512+64)+addr_stop);
                        *(gra2_data+10*b+L)=*(gra_data+i*(3*512+64)+512+addr_stop);	//ch2
                        //	}
                        b=b-1;
                    }
                }

                for (;b>0;b=b-1)
                {/*
                                        if (L==0)
                                        {
                                                *(gra1_data+10*b+0)=(*(gra1_data+10*b+1))*2-(*(gra1_data+10*b+2));
                                                *(gra2_data+10*b+0)=(*(gra2_data+10*b+1))*2-(*(gra2_data+10*b+2));
                                        }
                                        else if(L==9)
                                        {
                                                *(gra1_data+10*b+9)=(*(gra1_data+10*b+8))*2-(*(gra1_data+10*b+7));
                                                *(gra2_data+10*b+9)=(*(gra2_data+10*b+8))*2-(*(gra2_data+10*b+7));
                                        }
                                        else
                                        {*/
                    *(gra1_data+10*b+L)=*(gra_data+i*(3*512+64)+c);
                    *(gra2_data+10*b+L)=*(gra_data+i*(3*512+64)+512+c);	//ch2
                    //	}
                    c=c-1;
                }
                if(!flag) return 1;//������
                *(gengxintm+L)=50;
                if(*IsUpdateNow)
                {
                    for(j=0;j<10;j++)
                    {
                        *(gengxintm+j)=50;
                    }
                    for(j=0;j<10;j++)
                    {
                        for(m=0;m<50;m++)
                        {
                            *(gra1_data+10*m+j)=*(gra1_data+10*m+L);
                            *(gra2_data+10*m+j)=*(gra2_data+10*m+L);
                        }
                    }
                    *IsUpdateNow=0;
                }
                else
                {
                    for (j=0;j<10;j++)
                    {
                        gengxintm[j]--;

                        if(gengxintm[j]<6)
                        {
                            gengxintm[j] = 50;
                            for (m=0;m<50;m++)
                            {
                                if ((j==0)&&(m==0))
                                {
                                    gra1_data[0] = gra1_data[1];
                                    gra2_data[0] = gra2_data[1];
                                }
                                else if ((j==9)&&(m==49))
                                {
                                    gra1_data[499] = gra1_data[498];
                                    gra2_data[499] = gra2_data[498];
                                }
                                else
                                {
                                    gra1_data[10*m+j] = (int)((gra1_data[10*m+j-1]+gra1_data[10*m+j+1])/2);
                                    gra2_data[10*m+j] = (int)((gra2_data[10*m+j-1]+gra2_data[10*m+j+1])/2);

                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //40nsec
    if(nTimeDIV==4)
    {
        for (i=0;i<MAX_TIMES;i++)
        {
            L0=*(gra_data+i*(3*a+64)+1546)+*(gra_data+i*(3*a+64)+1547)*256+*(gra_data+i*(3*a+64)+1548)*256*256+*(gra_data+i*(3*a+64)+1549)*256*256*256;

            addr_stop=(*(gra_data+i*(3*a+64)+1537))*256;
            addr_stop= addr_stop+(*(gra_data+i*(3*a+64)+1536));
            if(addr_stop>511 || addr_stop<0) L0=0;//ȥ��Ram_Addr�������

            if (L0>MaxTDCData) L0=0;//ȥ��ʱ��������
            L0=L0-MinTDCData-MaxCalData;//���յ�ʱ��

            if (L0>0)
            {
                if((MaxTDCData-MinTDCData-MaxCalData)<0) return 0;
                L=(int)(L0/((MaxTDCData-MinTDCData-MaxCalData)/25.0));

                //	Value[i]=L;//for test
                b=20;
                c=511;

                if(L<0) return 1;
                if(L>24) return 1;

                for (;addr_stop>=0;addr_stop=addr_stop-1)
                {
                    if (b>=0)
                    {/*
                                                if (L==0)
                                                {
                                                        *(gra1_data+25*b+L)= (*(gra1_data+25*b+L+1))*2-(*(gra1_data+25*b+L+2));
                                                        *(gra2_data+25*b+L)=(*(gra2_data+25*b+L+1))*2-(*(gra2_data+25*b+L+2));
                                                }
                                                else if(L==24)
                                                {
                                                        *(gra1_data+25*b+24)=(*(gra1_data+25*b+23))*2-(*(gra1_data+25*b+22));
                                                        *(gra2_data+25*b+24)=(*(gra2_data+25*b+23))*2-(*(gra2_data+25*b+22));
                                                }
                                                else
                                                {*/
                        *(gra1_data+25*b+L)=*(gra_data+i*(3*a+64)+addr_stop);
                        *(gra2_data+25*b+L)=*(gra_data+i*(3*a+64)+512+addr_stop);
                        //	}
                        b=b-1;
                    }
                }
                for (;b>=0;b=b-1)
                {/*
                                        if (L==0)
                                        {
                                                *(gra1_data+25*b+L)=(*(gra1_data+25*b+L+1))*2-(*(gra1_data+25*b+L+2));
                                                *(gra2_data+25*b+L)=(*(gra2_data+25*b+L+1))*2-(*(gra2_data+25*b+L+2));
                                        }
                                        else if(L==24)
                                        {
                                                *(gra1_data+25*b+24)=(*(gra1_data+25*b+23))*2-(*(gra1_data+25*b+22));
                                                *(gra2_data+25*b+24)=(*(gra2_data+25*b+23))*2-(*(gra2_data+25*b+22));
                                        }
                                        else
                                        {*/
                    *(gra1_data+25*b+L)=*(gra_data+i*(3*a+64)+c);
                    *(gra2_data+25*b+L)=*(gra_data+i*(3*a+64)+512+c);
                    //	}
                    c=c-1;
                }
                if(!flag) return 1;//������
                *(gengxintm+L)=50;
                if(*IsUpdateNow)
                {
                    for(j=0;j<25;j++)
                    {
                        *(gengxintm+j)=50;
                    }
                    for(j=0;j<25;j++)
                    {
                        for(m=0;m<20;m++)
                        {
                            *(gra1_data+25*m+j)=*(gra1_data+25*m+L);
                            *(gra2_data+25*m+j)=*(gra2_data+25*m+L);
                        }
                    }
                    *IsUpdateNow=0;
                }
                else
                {
                    for (j=0;j<25;j++)
                    {
                        gengxintm[j]--;
                        if(gengxintm[j]<6)
                        {
                            gengxintm[j] = 50;
                            for (m=0;m<20;m++)
                            {
                                if ((j==0)&&(m==0))
                                {
                                    gra1_data[0] = gra1_data[1];
                                    gra2_data[0] = gra2_data[1];
                                }
                                else if ((j==24)&&(m==19))
                                {
                                    gra1_data[499] = gra1_data[498];
                                    gra2_data[499] = gra2_data[498];
                                }
                                else
                                {
                                    gra1_data[25*m+j] = (int)((gra1_data[25*m+j-1]+gra1_data[25*m+j+1])/2);
                                    gra2_data[25*m+j] = (int)((gra2_data[25*m+j-1]+gra2_data[25*m+j+1])/2);

                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //20nsec
    if(nTimeDIV==3)
    {
        for (i=0;i<MAX_TIMES;i++)
        {
            L0=*(gra_data+i*(3*a+64)+1546)+*(gra_data+i*(3*a+64)+1547)*256+*(gra_data+i*(3*a+64)+1548)*256*256+*(gra_data+i*(3*a+64)+1549)*256*256*256;

            addr_stop=(*(gra_data+i*(3*a+64)+1537))*256;
            addr_stop= addr_stop+(*(gra_data+i*(3*a+64)+1536));
            if(addr_stop>511 || addr_stop<0) L0=0;//ȥ��Ram_Addr�������

            L0=L0-MinTDCData-MaxCalData;//250;//���յ�ʱ��

            if(L0>(MaxTDCData+MaxCalData)) L0 =0;

            if (L0>0)
            {
                if((MaxTDCData-MinTDCData-MinCalData)<=0) return 0;
                L=(int)(L0/((MaxTDCData-MinTDCData-MinCalData)/50.0));

                b=11;
                c=511;

                if(L<0) return 1;
                if(L>49) return 1;

                for (;addr_stop>=0;addr_stop=addr_stop-1)
                {
                    if (b>=0)
                    {/*
                                                if (L==0)
                                                {
                                                        *(gra1_data+50*b+L)=(*(gra1_data+50*b+L+1))*2-(*(gra1_data+50*b+L+2));
                                                        *(gra2_data+50*b+L)=(*(gra2_data+50*b+L+1))*2-(*(gra2_data+50*b+L+2));
                                                }
                                                else if(L==49)
                                                {
                                                        *(gra1_data+50*b+49)=(*(gra1_data+50*b+48))*2-(*(gra1_data+50*b+47));
                                                        *(gra2_data+50*b+49)=(*(gra2_data+50*b+48))*2-(*(gra2_data+50*b+47));
                                                }
                                                else
                                                {*/
                        *(gra1_data+50*b+L)=*(gra_data+i*(3*a+64)+addr_stop);
                        *(gra2_data+50*b+L)=*(gra_data+i*(3*a+64)+512+addr_stop);
                        //	}
                        b=b-1;
                    }
                }
                for (;b>=0;b=b-1)
                {/*
                                        if (L==0)
                                        {
                                                *(gra1_data+50*b+L)=(*(gra1_data+50*b+L+1))*2-(*(gra1_data+50*b+L+2));
                                                *(gra2_data+50*b+L)=(*(gra2_data+50*b+L+1))*2-(*(gra2_data+50*b+L+2));
                                        }
                                        else if(L==49)
                                        {
                                                *(gra1_data+50*b+49)=(*(gra1_data+50*b+48))*2-(*(gra1_data+50*b+47));
                                                *(gra2_data+50*b+49)=(*(gra2_data+50*b+48))*2-(*(gra2_data+50*b+47));
                                        }
                                        else
                                        {*/
                    *(gra1_data+50*b+L)=*(gra_data+i*(3*a+64)+c);
                    *(gra2_data+50*b+L)=*(gra_data+i*(3*a+64)+512+c);
                    //	}
                    c=c-1;
                }
                if(!flag) return 1;//������
                *(gengxintm+L)=100;
                if(*IsUpdateNow)
                {
                    for(j=0;j<50;j++)
                    {
                        *(gengxintm+j)=100;
                    }
                    for(j=0;j<50;j++)
                    {
                        for(m=0;m<10;m++)
                        {
                            *(gra1_data+50*m+j)=*(gra1_data+50*m+L);
                            *(gra2_data+50*m+j)=*(gra2_data+50*m+L);
                        }
                    }
                    *IsUpdateNow=0;
                }
                else
                {
                    for (j=0;j<50;j++)
                    {
                        gengxintm[j]--;
                        if(gengxintm[j]<6)
                        {
                            gengxintm[j] = 100;
                            for (m=0;m<10;m++)
                            {
                                if ((j==0)&&(m==0))
                                {
                                    gra1_data[0] = gra1_data[1];
                                    gra2_data[0] = gra2_data[1];
                                }
                                else if ((j==49)&&(m==9))
                                {
                                    gra1_data[499] = gra1_data[498];
                                    gra2_data[499] = gra2_data[498];
                                }
                                else
                                {
                                    gra1_data[50*m+j] = (int)((gra1_data[50*m+j-1]+gra1_data[50*m+j+1])/2);
                                    gra2_data[50*m+j] = (int)((gra2_data[50*m+j-1]+gra2_data[50*m+j+1])/2);

                                }
                            }
                        }
                    }
                }
            }
        }
    }

    //10ns
    if(nTimeDIV==2)
    {
        for (i=0;i<MAX_TIMES;i++)
        {
            /*
                        K=(*(gra_data+i*(3*a+64)+1547))*256;
                        L0=K+(*(gra_data+i*(3*a+64)+1546));*/
            L0=*(gra_data+i*(3*a+64)+1546)+*(gra_data+i*(3*a+64)+1547)*256+*(gra_data+i*(3*a+64)+1548)*256*256+*(gra_data+i*(3*a+64)+1549)*256*256*256;
            //		Value[i]=L0;

            addr_stop=(*(gra_data+i*(3*a+64)+1537))*256;
            addr_stop= addr_stop+(*(gra_data+i*(3*a+64)+1536));

            if(addr_stop>510 || addr_stop<1)
            {
                return 0;//ȥ��Ram_Addr�������
            }
            L0=L0-MinTDCData-MaxCalData;//250;//���յ�ʱ��

            if(L0>(MaxTDCData+MaxCalData)) L0 = 0;

            if (L0>0)
            {
                if((MaxTDCData-MinTDCData-MinCalData)<=0) return 0;
                L=(int)(L0/((MaxTDCData-MinTDCData-MinCalData)/100.0));

                b=6;
                c=511;


                /*	if (L>10)
                                        L=L-11;*/

                if(L<0) return 1;
                if(L>99) return 1;

                for (;addr_stop>=0;addr_stop=addr_stop-1)
                {
                    if (b>=0)
                    {
                        if (L==0)
                        {
                            *(gra1_data+100*b+L)=*(gra1_data+100*b+L+1)*2-*(gra1_data+100*b+L+2);// (*(gra1_data+100*b+1))*2-(*(gra1_data+100*b+2));
                            *(gra2_data+100*b+L)=*(gra2_data+100*b+L+1)*2-*(gra2_data+100*b+L+2);// (*(gra2_data+100*b+1))*2-(*(gra2_data+100*b+2));
                        }
                        else if(L==99)
                        {
                            *(gra1_data+100*b+99)=(*(gra1_data+100*b+98)+(*(gra1_data+100*b+97)))/2;//(*(gra1_data+100*b+98))*2-(*(gra1_data+100*b+97));
                            *(gra2_data+100*b+99)=(*(gra2_data+100*b+98)+(*(gra2_data+100*b+97)))/2;//(*(gra2_data+100*b+98))*2-(*(gra2_data+100*b+97));
                        }
                        else
                        {
                            *(gra1_data+100*b+L)=*(gra_data+i*(3*a+64)+addr_stop);
                            *(gra2_data+100*b+L)=*(gra_data+i*(3*a+64)+512+addr_stop);
                        }
                        b=b-1;
                    }
                }

                for (;b>=0;b=b-1)
                {
                    if (L==0)
                    {
                        *(gra1_data+100*b+0)=(*(gra1_data+100*b+1)+(*(gra1_data+100*b+2)))/2;// (*(gra1_data+100*b+1))*2-(*(gra1_data+100*b+2));
                        *(gra2_data+100*b+0)=(*(gra2_data+100*b+1)+(*(gra2_data+100*b+2)))/2;// (*(gra2_data+100*b+1))*2-(*(gra2_data+100*b+2));
                    }
                    else if(L==99)
                    {
                        *(gra1_data+100*b+99)=(*(gra1_data+100*b+98)+(*(gra1_data+100*b+97)))/2;//(*(gra1_data+100*b+98))*2-(*(gra1_data+100*b+97));
                        *(gra2_data+100*b+99)=(*(gra2_data+100*b+98)+(*(gra2_data+100*b+97)))/2;//(*(gra2_data+100*b+98))*2-(*(gra2_data+100*b+97));
                    }
                    else
                    {
                        *(gra1_data+100*b+L)=*(gra_data+i*(3*a+64)+c);
                        *(gra2_data+100*b+L)=*(gra_data+i*(3*a+64)+512+c);
                    }
                    c=c-1;

                }
                if(!flag) return 1;//������
                *(gengxintm+L)=120;
                if(*IsUpdateNow)
                {
                    for(j=0;j<100;j++)
                    {
                        *(gengxintm+j)=120;
                    }
                    for(j=0;j<100;j++)
                    {
                        for(m=0;m<5;m++)
                        {
                            *(gra1_data+100*m+j)=*(gra1_data+100*m+L);
                            *(gra2_data+100*m+j)=*(gra2_data+100*m+L);
                        }
                    }
                    *IsUpdateNow=0;
                }
                else
                {
                    for (j=0;j<100;j++)
                    {
                        gengxintm[j]--;
                        if(gengxintm[j]<6)
                        {
                            gengxintm[j] = 120;

                            for (m=0;m<5;m++)
                            {
                                if ((j==0)&&(m==0))
                                {
                                    gra1_data[0] = gra1_data[1];
                                    gra2_data[0] = gra2_data[1];
                                }
                                else if ((j==99)&&(m==4))
                                {
                                    gra1_data[499] = gra1_data[498];
                                    gra2_data[499] = gra2_data[498];
                                }
                                else
                                {
                                    gra1_data[100*m+j] = (int)((gra1_data[100*m+j-1]+gra1_data[100*m+j+1])/2);
                                    gra2_data[100*m+j] = (int)((gra2_data[100*m+j-1]+gra2_data[100*m+j+1])/2);

                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //4ns
    if(nTimeDIV==1)
    {
        for (i=0;i<MAX_TIMES;i++)
        {
            L0=*(gra_data+i*(3*a+64)+1546)+*(gra_data+i*(3*a+64)+1547)*256+*(gra_data+i*(3*a+64)+1548)*256*256+*(gra_data+i*(3*a+64)+1549)*256*256*256;

            addr_stop=(*(gra_data+i*(3*a+64)+1537))*256;
            addr_stop= addr_stop+(*(gra_data+i*(3*a+64)+1536));


            if(addr_stop>511 || addr_stop<0) L0=0;//ȥ��Ram_Addr�������

            L0=L0-MinTDCData-MaxCalData;//250;//���յ�ʱ��

            //	if(L0>(MaxTDCData+MaxCalData)) L0=0;

            if (L0>0)
            {
                if((MaxTDCData-MinTDCData-MinCalData)<=0) return 0;
                L=(int)(L0/((MaxTDCData-MinTDCData-MinCalData)/250.0));


                //		L = (1.0+0.07*LL/250.0)*LL*(242.0/((1.0+(0.07*249.0)/250.0)*249.0));


                //	Value[i]=L;//for test
                b=2;
                c=511;
                /*
                                if (L>10)
                                        L=L-11;*/

                if(L<0) return 1;
                if(L>249) return 1;
                for (;addr_stop>=0;addr_stop=addr_stop-1)
                {
                    if (b>=0)
                    {/*
                                                if (L==0)
                                                {
                                                        *(gra1_data+250*b+L)= (*(gra1_data+250*b+L+1))*2-(*(gra1_data+250*b+L+2));
                                                        *(gra2_data+250*b+L)=(*(gra2_data+250*b+L+1))*2-(*(gra2_data+250*b+L+2));
                                                }
                                                else if(L==249)
                                                {
                                                        *(gra1_data+250*b+249)=(*(gra1_data+250*b+248))*2-(*(gra1_data+250*b+247));
                                                        *(gra2_data+250*b+249)=(*(gra2_data+250*b+248))*2-(*(gra2_data+250*b+247));
                                                }
                                                else
                                                {*/
                        *(gra1_data+250*b+L)=*(gra_data+i*(3*a+64)+addr_stop);
                        *(gra2_data+250*b+L)=*(gra_data+i*(3*a+64)+512+addr_stop);
                        //	}
                        b=b-1;
                    }
                }
                for (;b>=0;b=b-1)
                {/*
                                        if (L==0)
                                        {
                                                *(gra1_data+250*b+L)= (*(gra1_data+250*b+L+1))*2-(*(gra1_data+250*b+L+2));
                                                *(gra2_data+250*b+L)=(*(gra2_data+250*b+L+1))*2-(*(gra2_data+250*b+L+2));
                                        }
                                        else if(L==249)
                                        {
                                                *(gra1_data+250*b+249)=(*(gra1_data+250*b+248))*2-(*(gra1_data+250*b+247));
                                                *(gra2_data+250*b+249)=(*(gra2_data+250*b+248))*2-(*(gra2_data+250*b+247));
                                        }
                                        else
                                        {*/
                    *(gra1_data+250*b+L)=*(gra_data+i*(3*a+64)+c);
                    *(gra2_data+250*b+L)=*(gra_data+i*(3*a+64)+512+c);
                    //	}
                    c=c-1;
                }

                if(!flag) return 1;//������
                *(gengxintm+L)=140;
                if(*IsUpdateNow)
                {
                    for(j=0;j<250;j++)
                    {
                        *(gengxintm+j)=140;
                    }
                    for(j=0;j<250;j++)
                    {
                        for(m=0;m<2;m++)
                        {
                            *(gra1_data+250*m+j)=*(gra1_data+250*m+L);
                            *(gra2_data+250*m+j)=*(gra2_data+250*m+L);
                        }
                    }
                    *IsUpdateNow=0;
                }
                else
                {
                    for (j=0;j<250;j++)
                    {
                        gengxintm[j]--;
                        if(gengxintm[j]<6)
                        {
                            gengxintm[j] = 140;

                            for (m=0;m<2;m++)
                            {
                                if ((j==0)&&(m==0))
                                {
                                    gra1_data[0] = gra1_data[1];
                                    gra2_data[0] = gra2_data[1];
                                }
                                else if ((j==249)&&(m==1))
                                {
                                    gra1_data[499] = gra1_data[498];
                                    gra2_data[499] = gra2_data[498];
                                }
                                else
                                {
                                    gra1_data[250*m+j] = (int)((gra1_data[250*m+j-1]+gra1_data[250*m+j+1])/2);
                                    gra2_data[250*m+j] = (int)((gra2_data[250*m+j-1]+gra2_data[250*m+j+1])/2);

                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //2ns
    if(nTimeDIV==0)
    {
        for (i=0;i<MAX_TIMES;i++)
        {
            L0=*(gra_data+i*(3*a+64)+1546)+*(gra_data+i*(3*a+64)+1547)*256+*(gra_data+i*(3*a+64)+1548)*256*256+*(gra_data+i*(3*a+64)+1549)*256*256*256;

            addr_stop=(*(gra_data+i*(3*a+64)+1537))*256;
            addr_stop= addr_stop+(*(gra_data+i*(3*a+64)+1536));
            if(addr_stop>511 || addr_stop<0) L0=0;//ȥ��Ram_Addr�������

            L0=L0-MinTDCData-MaxCalData;//250;//���յ�ʱ��

            if(L0>(MaxTDCData+MaxCalData)) L0 =0;

            if (L0>0)
            {
                if((MaxTDCData-MinTDCData-MinCalData)<=0) return 0;
                L=(int)(L0/((MaxTDCData-MinTDCData-MinCalData)/500.0));

                //	Value[i]=L;//for test
                b=1;
                c=511;

                /*
                                if (L>42)
                                        L=L-43;*/

                if(L<0) return 1;
                if(L>499) return 1;

                for (;addr_stop>=0;addr_stop=addr_stop-1)
                {
                    if (b>=0)
                    {/*
                                                if (L==0)
                                                {
                                                        *(gra1_data+500*b+L)= (*(gra1_data+500*b+L+1))*2-(*(gra1_data+500*b+L+2));
                                                        *(gra2_data+500*b+L)=(*(gra2_data+500*b+L+1))*2-(*(gra2_data+500*b+L+2));
                                                }
                                                else if(L==99)
                                                {
                                                        *(gra1_data+500*b+499)=(*(gra1_data+500*b+498))*2-(*(gra1_data+500*b+497));
                                                        *(gra2_data+500*b+499)=(*(gra2_data+500*b+498))*2-(*(gra2_data+500*b+497));
                                                }
                                                else
                                                {*/
                        *(gra1_data+500*b+L)=*(gra_data+i*(3*a+64)+addr_stop);
                        *(gra2_data+500*b+L)=*(gra_data+i*(3*a+64)+512+addr_stop);
                        //	}
                        b=b-1;
                    }
                }
                for (;b>=0;b=b-1)
                {/*
                                        if (L==0)
                                        {
                                                *(gra1_data+500*b+0)=(*(gra1_data+500*b+L+1))*2-(*(gra1_data+500*b+L+2));
                                                *(gra2_data+500*b+0)=(*(gra2_data+500*b+L+1))*2-(*(gra2_data+500*b+L+2));
                                        }
                                        else if(L==99)
                                        {
                                                *(gra1_data+500*b+499)=(*(gra1_data+500*b+498))*2-(*(gra1_data+500*b+497));
                                                *(gra2_data+500*b+499)=(*(gra2_data+500*b+498))*2-(*(gra2_data+500*b+497));
                                        }
                                        else
                                        {*/
                    *(gra1_data+500*b+L)=*(gra_data+i*(3*a+64)+c);
                    *(gra2_data+500*b+L)=*(gra_data+i*(3*a+64)+512+c);
                    //	}
                    c=c-1;
                }
                if(!flag) return 1;//������
                *(gengxintm+L)=150;
                if(*IsUpdateNow)
                {
                    for(j=0;j<500;j++)
                    {
                        *(gengxintm+j)=150;
                    }
                    for(j=0;j<500;j++)
                    {
                        for(m=0;m<1;m++)
                        {
                            *(gra1_data+500*m+j)=*(gra1_data+500*m+L);
                            *(gra2_data+500*m+j)=*(gra2_data+500*m+L);
                        }
                    }
                    *IsUpdateNow=0;
                }
                else
                {
                    for (j=0;j<500;j++)
                    {
                        gengxintm[j]--;
                        if(gengxintm[j]<10)
                        {
                            gengxintm[j] = 150;

                            for (m=0;m<1;m++)
                            {
                                if ((j==0)&&(m==0))
                                {
                                    gra1_data[0] = gra1_data[1];
                                    gra2_data[0] = gra2_data[1];
                                }
                                else if ((j==499)&&(m==0))
                                {
                                    gra1_data[499] = gra1_data[498];
                                    gra2_data[499] = gra2_data[498];
                                }
                                else
                                {
                                    gra1_data[500*m+j] = (int)((gra1_data[500*m+j-1]+gra1_data[500*m+j+1])/2);
                                    gra2_data[500*m+j] = (int)((gra2_data[500*m+j-1]+gra2_data[500*m+j+1])/2);

                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return 1;
}

int ETSGetData(WORD DeviceIndex,
               WORD Ch1_Att,
               WORD Ch2_Att,
               WORD* ch1_data,
               WORD nTimeDIV,
               WORD TriggerModel)
{
    int MAX_TIMES,mid8[65408],k,*midBuffer,cnt=0;
    WORD h,result;
    double mid;
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL,inBuffer = NULL;
    BULK_TRANSFER_CONTROL outBulkControl,inBulkControl;
    HANDLE HDevice=NULL;
    UINT m_nSize;
    long outLen = 2;
    long a=0,i,j,Data_L;
    ULONG bufferSize = 0,BytesReturned=0;
    BOOL bResult=FALSE,status=FALSE;

    midBuffer = (int*)malloc(76800 *sizeof(int));
    BytesReturned=0;
    outBuffer=(PUCHAR) malloc(outLen);
    outBuffer[0]=0x05;
    outBuffer[1]=0x00;
    outBulkControl.pipeNum=0;

    if(nTimeDIV < 2)
    {
        MAX_TIMES=15;
    }
    else
    {
        MAX_TIMES=10;
    }
    if (PCUSBSpeed(DeviceIndex))
    {
        m_nSize = USB_PACK_SIZE;
    }
    else
    {
        m_nSize = 64;//USB1.1
    }
    inBuffer=(PUCHAR) malloc(m_nSize);
    memset(inBuffer, 0,m_nSize);
    inBulkControl.pipeNum = 1;
    WORD Data_Length;

    Data_Length = 3;

    switch (Data_Length)
    {
    case 3:
        Data_L=5*512/m_nSize;
        a = 512;
        break;
    default:
        Data_L=5*512/m_nSize;
        a = 512;
        break;
    }

    SelectDeviceIndex(DeviceIndex,pcDriverName);//��ʼ���豸��

    result = CaptureStart(DeviceIndex,1);//��ʼ����  zhang

    //���豸
    HDevice = OpenDevice(pcDriverName);
    if(HDevice==NULL)
    {
        return 0;
    }
    //��ʼѭ������
    for (j=0;j<MAX_TIMES;j++)
    {

        result = GetCaptureState(DeviceIndex);
        while(result <=3)
        {
            cnt++;
            if(cnt>350) return 0;//���3�����û�д��������ж�Ϊû���豸
            if(cnt>250)
            {
                if(TriggerModel==0)//Auto
                {
                    result = ForceTrigger(DeviceIndex);
                }
                else if (TriggerModel==1)//Normal
                {
                    return 2;
                }
            }
            result = GetCaptureState(DeviceIndex);
        }

        ResetDevice(DeviceIndex,1);//�豸��λ
#ifdef _ASY
        status = TransferDataToDevice(outBuffer,outLen);
#else
        //֪ͨ����
        status = DeviceIoControl(HDevice,
                                 IOCTL_EZUSB_BULK_WRITE,
                                 (PVOID)&outBulkControl,
                                 sizeof(BULK_TRANSFER_CONTROL),
                                 outBuffer,//���������
                                 (UINT)outLen,//�ֽ������ڶԻ����п�������
                                 &BytesReturned,//�����ֽ�����
                                 NULL);//����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
#endif

        //��ʼ����
        for(i=0;i<Data_L;i++)
        {
#ifdef _ASY
            status = TransferDataFromDevice(inBuffer,m_nSize);
#else
            status = DeviceIoControl(HDevice,
                                     IOCTL_EZUSB_BULK_READ,
                                     (PVOID)&inBulkControl,
                                     sizeof(BULK_TRANSFER_CONTROL),
                                     (LPVOID)inBuffer,//���������
                                     m_nSize,//�ֽ������ڶԻ����п�������
                                     &BytesReturned,//�����ֽ�����
                                     NULL);//����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
#endif
            if(status == 0)
            {
                break;
            }

            if(i<(long)(4*512/m_nSize))//ԭ����4��512
            {
                if(i<Data_L/2)//ǰ1K����
                {
                    for(k=0;k<(int)(m_nSize/2);k++)
                    {
                        midBuffer[i*(m_nSize/2)+k+j* 5*512] = inBuffer[2*k+1];
                        midBuffer[i*(m_nSize/2)+512+k+j* 5*512] = inBuffer[2*k];
                    }
                }
                else//��1K����
                {
                    for(k=0;k<(int)(m_nSize/2);k++)
                    {
                        midBuffer[j*5*512+2*512+(i-Data_L/2)*(m_nSize/2)+k] = inBuffer[2*k+1];
                        midBuffer[j*5*512+2*512+(i-Data_L/2)*(m_nSize/2)+512+k] = inBuffer[2*k];
                    }
                }
            }
            else//TDC��512
            {
                //����TDC
                for(k=0;k<(int)m_nSize;k++)
                {
                    if(k==3)
                    {
                        inBuffer[k]=inBuffer[k] & 0x0f;
                    }
                    midBuffer[j* 5*512+ 4*512+k] = inBuffer[k];
                }
            }
        }
        //����Ram_Addr
        //		midBuffer[j*5*512+4*m_nSize+4] = Ram_Addr&0xFF;//Ram_Addr��λ
        //	midBuffer[j*5*512+4*m_nSize+5] = (Ram_Addr>>8)&0xff;//Ram_Addr��λ
        //	::Sleep(2*j);
    }
    //30�ζ�������
    free(inBuffer);//�ͷ��ڴ�
    free(outBuffer);//�ͷ��ڴ�
    CloseDevice(HDevice);//�ر��豸

    /*9BIT */
    a = 512;
    m_nSize=512;
    switch (Ch1_Att)
    {
    case 0:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+i])|(midBuffer[h*(5*a)+3*a+i]&0x03)<<8);
                mid=mid*1.4;
                mid8[h*(3*a+64)+i]=(int)mid;
            }
        }
        break;
    case 1:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+i])|(midBuffer[h*(5*a)+3*a+i]&0x03)<<8);
                mid=mid*0.675;
                mid8[h*(3*a+64)+i]=(int)mid;
            }
        }
        break;
    case 2:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+i])|(midBuffer[h*(5*a)+3*a+i]&0x03)<<8);
                mid=mid*0.565;
                mid8[h*(3*a+64)+i]=(int)mid;
            }
        }
        break;
    case 3:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+i])|(midBuffer[h*(5*a)+3*a+i]&0x03)<<8);
                mid=mid*0.28;
                mid8[h*(3*a+64)+i]=(int)mid;
            }
        }
        break;
    case 4:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+i])|(midBuffer[h*(5*a)+3*a+i]&0x03)<<8);
                mid=mid*0.71;
                mid8[h*(3*a+64)+i]=(int)mid;
            }
        }
        break;
    case 5:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+i])|(midBuffer[h*(5*a)+3*a+i]&0x03)<<8);
                mid=mid*0.56;//0.58;
                mid8[h*(3*a+64)+i]=(int)mid;
            }
        }
        break;
    case 6:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+i])|(midBuffer[h*(5*a)+3*a+i]&0x03)<<8);
                mid=mid*0.28;
                mid8[h*(3*a+64)+i]=(int)mid;
            }
        }
        break;
    case 7:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+i])|(midBuffer[h*(5*a)+3*a+i]&0x03)<<8);
                mid=mid*0.72;//0.685;
                mid8[h*(3*a+64)+i]=(int)mid;
            }
        }
        break;
    case 8:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+i])|(midBuffer[h*(5*a)+3*a+i]&0x03)<<8);
                mid=mid*0.585;
                mid8[h*(3*a+64)+i]=(int)mid;
            }
        }
        break;
    case 9:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+i])|(midBuffer[h*(5*a)+3*a+i]&0x03)<<8);
                mid=mid*0.29;
                mid8[h*(3*a+64)+i]=(int)mid;
            }
        }
        break;
    default:
        break;
    }
    switch (Ch2_Att)
    {
    case 0:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+a+i])|((midBuffer[h*(5*a)+3*a+i]>>2)&0x03)<<8);
                mid=mid*1.4;
                mid8[h*(3*a+64)+a+i]=(int)mid;
            }
        }
        break;
    case 1:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+a+i])|((midBuffer[h*(5*a)+3*a+i]>>2)&0x03)<<8);
                mid=mid*0.675;
                mid8[h*(3*a+64)+a+i]=(int)mid;
            }
        }
        break;
    case 2:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+a+i])|((midBuffer[h*(5*a)+3*a+i]>>2)&0x03)<<8);
                mid=mid*0.565;
                mid8[h*(3*a+64)+a+i]=(int)mid;
            }
        }
        break;
    case 3:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+a+i])|((midBuffer[h*(5*a)+3*a+i]>>2)&0x03)<<8);
                mid=mid*0.28;
                mid8[h*(3*a+64)+a+i]=(int)mid;
            }
        }
        break;
    case 4:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+a+i])|((midBuffer[h*(5*a)+3*a+i]>>2)&0x03)<<8);
                mid=mid*0.71;
                mid8[h*(3*a+64)+a+i]=(int)mid;
            }
        }
        break;
    case 5:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+a+i])|((midBuffer[h*(5*a)+3*a+i]>>2)&0x03)<<8);
                mid=mid*0.565;//0.58;
                mid8[h*(3*a+64)+a+i]=(int)mid;
            }
        }
        break;
    case 6:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+a+i])|((midBuffer[h*(5*a)+3*a+i]>>2)&0x03)<<8);
                mid=mid*0.28;
                mid8[h*(3*a+64)+a+i]=(int)mid;
            }
        }
        break;
    case 7:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+a+i])|((midBuffer[h*(5*a)+3*a+i]>>2)&0x03)<<8);
                mid=mid*0.72;//0.685;
                mid8[h*(3*a+64)+a+i]=(int)mid;
            }
        }
        break;
    case 8:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+a+i])|((midBuffer[h*(5*a)+3*a+i]>>2)&0x03)<<8);
                mid=mid*0.585;
                mid8[h*(3*a+64)+a+i]=(int)mid;
            }
        }
        break;
    case 9:
        for (h=0;h<MAX_TIMES;h++)
        {
            for (i=0;i<a;i++)
            {
                mid=((midBuffer[h*(5*a)+a+i])|((midBuffer[h*(5*a)+3*a+i]>>2)&0x03)<<8);
                mid=mid*0.29;
                mid8[h*(3*a+64)+a+i]=(int)mid;
            }
        }
        break;
    default:
        break;
    }

    for(h=0;h<MAX_TIMES;h++)
    {
        for(i=0;i<4;i++)
        {
            //����TDC����**************************************************
            mid8[h*(3*a+64)+3*a+10+i]=midBuffer[h*(5*a)+4*a+i];//����
            if(i<2)//����Ram_Addr
            {
                mid8[h*(3*a+64)+3*a+i]=midBuffer[h*(5*a)+4*a+i+4];
            }
        }
    }
    free(midBuffer);//�ͷ�midBuffer�ڴ�ռ�
    for (k=0;k<(3*a+64)*MAX_TIMES;k=k+1)
    {
        *(ch1_data+k)=mid8[k];
    }

    return 1;
}
WORD        RDADCCali(WORD DeviceIndex,WORD* level,WORD length)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_IN;
    UCHAR VendNo=0xA2; //��дEEPROM����
    UCHAR pBuffer[8*1024];
    HANDLE hDevice;
    ULONG ncnt,i;
    USHORT value = 0x1B80+EE_OFFSET;// add by yt 20100626
    ncnt=length*2;//��ȡ24LC64�ĳ��ȣ����4K
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    ResetDevice(DeviceIndex,1);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice == NULL)
    {
        return 0;
    }
    bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,ncnt);
    CloseDevice(hDevice);
    for(i=0;i<length;i++)
    {
        level[i]=pBuffer[2*i]+(pBuffer[2*i+1]<<8);
    }
    return bResult;
} 

DLL_API WORD WINAPI dsoHTRDADCCali(WORD nDeviceIndex,WORD* pLevel,WORD nLen)
{
    return RDADCCali(nDeviceIndex,pLevel,nLen);
}

WORD        WRADCCali(WORD DeviceIndex,WORD* level,WORD length)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_OUT;
    UCHAR VendNo=0xa2; //��дEEPROM
    PUCHAR pBuffer=(PUCHAR)malloc(2*sizeof(UCHAR)*length);
    memset(pBuffer,0,2*sizeof(UCHAR)*length);
    HANDLE hDevice = NULL;
    USHORT value = 0x1B80+EE_OFFSET;// add by yt 20100626
    for(int i=0;i<length;i++)
    {
        pBuffer[2*i]=level[i] & 0xFF;//��8bit
        pBuffer[2*i+1]=(level[i]>>8) & 0xFF;//��8bit
    }
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice == NULL)
        return 0;
    bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,length*2);
    free(pBuffer);
    CloseDevice(hDevice);
    return bResult;
}



WORD        WRAmpCali(WORD DeviceIndex,WORD* level,WORD length)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_OUT;
    UCHAR VendNo=0xa2; //��дEEPROM
    PUCHAR pBuffer=(PUCHAR)malloc(2*sizeof(UCHAR)*length);
    memset(pBuffer,0,2*sizeof(UCHAR)*length);
    HANDLE hDevice = NULL;
    USHORT value = 0x1900+EE_OFFSET;
    for(int i=0;i<length;i++)
    {
        pBuffer[2*i]=level[i] & 0xFF;//��8bit
        pBuffer[2*i+1]=(level[i]>>8) & 0xFF;//��8bit
    }
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice == NULL)
        return 0;
    bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,length*2);
    free(pBuffer);
    CloseDevice(hDevice);
    return bResult;
}



WORD        RDAmpCali(WORD DeviceIndex,WORD* level,WORD length)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_IN;
    UCHAR VendNo=0xA2; //��дEEPROM����
    UCHAR pBuffer[8*1024];
    HANDLE hDevice;
    ULONG ncnt,i;
    USHORT value = 0x1900+EE_OFFSET;// add by yt 20100626
    ncnt=length*2;//��ȡ24LC64�ĳ��ȣ����4K
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    ResetDevice(DeviceIndex,1);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice == NULL)
    {
        return 0;
    }
    bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,ncnt);
    CloseDevice(hDevice);
    for(i=0;i<length;i++)
    {
        level[i]=pBuffer[2*i]+(pBuffer[2*i+1]<<8);
    }
    return bResult;
}




//Export--Hard Functions////////////////////////////////////////////////////////////////////////////////////////

//��ȡУ�Ե�ƽ����
DLL_API WORD WINAPI  dsoHTWRADCCali(WORD nDeviceIndex,WORD* pLevel,WORD nLen)
{
    return WRADCCali(nDeviceIndex,pLevel,nLen);

}
DLL_API WORD WINAPI  dsoHTWRAmpCali(WORD nDeviceIndex,WORD* pLevel,WORD nLen)
{
    return WRAmpCali(nDeviceIndex,pLevel,nLen);

}
DLL_API WORD WINAPI  dsoHTRDAmpCali(WORD nDeviceIndex,WORD* pLevel,WORD nLen)
{
    return RDAmpCali(nDeviceIndex,pLevel,nLen);
}
DLL_API WORD WINAPI dsoHTReadCalibrationData(WORD nDeviceIndex,WORD* pLevel,WORD nLen)
{
    return GetChannelLevel(nDeviceIndex,pLevel,nLen);
}

//д��У�Ե�ƽ����
DLL_API WORD WINAPI dsoHTWriteCalibrationData(WORD nDeviceIndex,WORD* pLevel,WORD nLen)
{
    return SetChannelLevel(nDeviceIndex,pLevel,nLen);
}

//�����豸�����������豸��
DLL_API WORD WINAPI dsoHTSearchDevice(short* pDevInfo)
{
    HANDLE hDevice = NULL;
    char pcDriverName[MAX_DRIVER_NAME] = "";
    WORD nDevNum = 0;
#ifdef _ASY
    /*	GetDevices(0x3064);//�����豸
        for(int i=0;i<MAX_USB_DEV_NUMBER;i++)
        {
                if(g_DevIndex[i] >= 0 && pUSBDevice->Open(g_DevIndex[i]))
                {
                        pUSBDevice->Close();
                        pDevInfo[i] = 0;//���豸
                        nDevNum++;
                        if(nDevNum >= g_DevNum)
                        {
                                break;
                        }
                }
                else
                {
                        pDevInfo[i] = -1;//���豸
                }
        }*/
#else
    for(int i =0; i<MAX_USB_DEV_NUMBER; i++)
    {
        SelectDeviceIndex(i,pcDriverName);
        hDevice = OpenDevice(pcDriverName);
        if (hDevice != NULL)
        {
            pDevInfo[i] = 1;//���豸
            CloseDevice(hDevice);
            nDevNum++;
        }
        else
        {
            pDevInfo[i] = 0;//���豸
        }
    }
#endif
    return nDevNum;

}

DLL_API WORD WINAPI dsoHTDeviceConnect(WORD nDeviceIndex)
{
    HANDLE hDevice = NULL;

    char pcDriverName[MAX_DRIVER_NAME] = "";

    SelectDeviceIndex(nDeviceIndex,pcDriverName);

    hDevice = OpenDevice(pcDriverName);
    if(hDevice != NULL)
    {
        CloseDevice(hDevice);
        return 1;//���豸
    }
    else
    {
        return 0;//���豸
    }

}

//����CH Lever Pos
WORD        SetChannelPosDrect(WORD DeviceIndex,WORD CHIndex,WORD DacData)
{
    PUCHAR outBuffer = NULL;
    UINT m_nSize=4;
    BOOL status=FALSE;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=CHIndex==0?0x00:(0x01<<(CHIndex-1));  //ע�ⲻ�ܶ���128ͨ��
    outBuffer[1]=0x00;
    outBuffer[2]=0xff&(DacData>>0);
    outBuffer[3]=0xff&(DacData>>8);
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
	Sleep(10);
    return status;
}
WORD        SetChannelPosDrectNoS(WORD DeviceIndex,WORD CHIndex,WORD DacData)
{
	    PUCHAR outBuffer = NULL;
    UINT m_nSize=4;
    BOOL status=FALSE;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=CHIndex==0?0x00:(0x01<<(CHIndex-1));  //ע�ⲻ�ܶ���128ͨ��
    outBuffer[1]=0x00;
    outBuffer[2]=0xff&(DacData>>0);
    outBuffer[3]=0xff&(DacData>>8);
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
	Sleep(20);
    free(outBuffer);

    return status;
}
DLL_API WORD WINAPI  dsoHTSetCHPos(WORD nDeviceIndex,WORD* pLevel,WORD nVoltDIV,WORD nPos,WORD nCH,WORD nCHMode)
{
	//nPos=255-nPos;
    int storeIndex=0;
    if(nCHMode==4)  
        storeIndex=4;
    if(nCHMode==2)
        storeIndex=2;
    if(nCHMode==1)
        storeIndex=0;
	WORD nUp,nDown;
	WORD nMid=0;
	WORD nOffset=0;
	bool bNeedOffset=true;
#ifdef DSO_6000_12DIV
	if(nVoltDIV==10)//5V
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+1)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+1)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/2.0+0.5);
	}
	else if(nVoltDIV==7) //500mV
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+2)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+2)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/4.0+0.5);
	}
	else if(nVoltDIV==8)//1V
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+1)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+1)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/2.0+0.5);
	}
	else if(nVoltDIV==5)//100mV
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+1)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+1)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/2.0+0.5);
	}
	else if(nVoltDIV==3)//20mV
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+1)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+1)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/2.5+0.5);
	}
	else if(nVoltDIV==2)//10mV
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+2)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+2)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/5.0+0.5);
	}
	else if(nVoltDIV==1)//5mV
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+3)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+3)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/10.0+0.5);
	}
	else if(nVoltDIV==0)//2mV
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+4)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+4)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/25.0+0.5);
	}
	else
	{
	//int x1=nCH*(ZEROCALI_PER_CH_LEN)+nVoltDIV*ZEROCALI_PER_VOLT_LEN+storeIndex+1;
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+nVoltDIV*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+nVoltDIV*ZEROCALI_PER_VOLT_LEN+storeIndex];
		bNeedOffset=false;
	}
	if(bNeedOffset)
	{
		nUp=nMid+nOffset;
		nDown=nMid-nOffset;
	}
    nPos = WORD((nUp - nDown)*1.0/MAX_DATA*nPos + nDown);
#elif defined D10_100DELAY
	if(nVoltDIV==10)//5V
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+1)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+1)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/2.0+0.5);
	}
	else if(nVoltDIV==9) //2V
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+2)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+2)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/5.0+0.5);
	}

	else if(nVoltDIV==7) //500mV
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+1)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+1)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/2.0+0.5);
	}
	else if(nVoltDIV==6)//200mV
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+2)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+2)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/5.0+0.5);
	}

	else if(nVoltDIV==4)//50mV
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+1)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+1)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/2.0+0.5);
	}
	else if(nVoltDIV==3)//20mV
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+2)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+2)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/5.0+0.5);
	}
	else if(nVoltDIV==2)//10mV
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+3)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+3)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/10.0+0.5);
	}
	else if(nVoltDIV==1)//5mV
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+4)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+4)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/20.0+0.5);
	}
	else if(nVoltDIV==0)//2mV
	{
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+5)*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+(nVoltDIV+5)*ZEROCALI_PER_VOLT_LEN+storeIndex];
		nMid=WORD((nDown+nUp)/2.0+0.5);
		nOffset=WORD((nUp-nMid)/50.0+0.5);
	}
	else
	{
	//int x1=nCH*(ZEROCALI_PER_CH_LEN)+nVoltDIV*ZEROCALI_PER_VOLT_LEN+storeIndex+1;
		nUp=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+nVoltDIV*ZEROCALI_PER_VOLT_LEN+storeIndex+1];
		nDown=pLevel[nCH*(ZEROCALI_PER_CH_LEN)+nVoltDIV*ZEROCALI_PER_VOLT_LEN+storeIndex];
		bNeedOffset=false;
	}
	if(bNeedOffset)
	{
		nUp=nMid+nOffset;
		nDown=nMid-nOffset;
	}
    nPos = WORD((nUp - nDown)*1.0/MAX_DATA*nPos + nDown);

#endif
    return SetChannelPosDrect(nDeviceIndex,nCH,nPos);
   
}

//ֱ�ӿ��ƻ�׼��ƽ��Ϊ9��У׼���
DLL_API WORD WINAPI dsoHTSetCHDirectLeverPos(WORD nDeviceIndex,WORD nPos,WORD nCH)
{
	return SetChannelPosDrectNoS(nDeviceIndex,nCH,nPos);
}


//����Trigger Level
WORD        setEdgeTriggerDirect(WORD DeviceIndex,WORD nLevel_256,WORD nCHIndex,bool bBiggerOne)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    BULK_TRANSFER_CONTROL   outBulkControl;
    HANDLE hOutDevice=NULL;
    UINT m_nSize=6;

    SelectDeviceIndex(DeviceIndex,pcDriverName);	//��ʼ���豸��
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    //compute channel address
    unsigned int nCHAddress=0x8011;
    nCHAddress+=2*nCHIndex+bBiggerOne?0x00:0x01;
    //compute channel address over
    nLevel_256=nLevel_256<0     ?0x00:nLevel_256;
    nLevel_256=nLevel_256>255   ?0xFF:nLevel_256;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x7F;
    outBuffer[1]=0x01;
    outBuffer[2]=0xFF&(nCHAddress);  //daccs
    outBuffer[3]=0xFF&(nCHAddress>>8);
    outBuffer[4]=0xff&(nLevel_256);
    outBuffer[5]=0xff&(nLevel_256>>8) ;
    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status =  DeviceIoControl (hOutDevice,
                               IOCTL_EZUSB_BULK_WRITE,
                               (PVOID)&outBulkControl,
                               sizeof(BULK_TRANSFER_CONTROL),
                               outBuffer,//���������
                               m_nSize,//�ֽ������ڶԻ����п�������
                               &BytesReturned,//�����ֽ�����,����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                               NULL);
#endif
    //�ر��豸
    free(outBuffer);
    CloseDevice(hOutDevice);
    return status;
}

DLL_API WORD WINAPI dsoHTSetVTriggerLevel(WORD nDeviceIndex,WORD* pLevel,WORD nPos,WORD nSensitivity)
{

    return SetTrigLevel(nDeviceIndex,nPos,nSensitivity);
}

//���ô������Ⱥ�Ԥ��������(����Triggerˮƽλ��)-->����nBufferLen ������512����������,�Ҳ���>16M
DLL_API WORD WINAPI dsoHTSetHTriggerLength(WORD nDeviceIndex,ULONG nBufferLen,WORD HTriggerPos,WORD nTimeDIV,WORD nCHMod)
{
    return SetTriggerLength(nDeviceIndex,nBufferLen,HTriggerPos,nTimeDIV,nCHMod);
}

//�˺���Ŀǰ��Ч���������
DLL_API WORD WINAPI dsoHTSetBufferSize(WORD nDeviceIndex,WORD nBufferSize)
{
    return SetRamLength(nDeviceIndex,nBufferSize);
}

//����ǿ��ˢ��13·�̵�������
DLL_API WORD WINAPI dsoHTSetCHAndTriggerDirect(WORD nDeviceIndex,PRELAYCONTROL pRelayControl,USHORT nDirect,WORD nDriverCode)
{
    if(DRIVER_CODE_SECOND == nDriverCode)
    {
        return SetAboutInputChannelAndTrigger_Only_DSO3074(nDeviceIndex,pRelayControl,nDirect);
    }
    else //if(DRIVER_CODE_FIRST == nDriverCode)
    {
        return 0;
    }
}

//����CH��Trigger Source and Trigger Filt
DLL_API WORD WINAPI dsoHTSetCHAndTrigger(WORD nDeviceIndex,PRELAYCONTROL pRelayControl,WORD nDriverCode,PCONTROLDATA pControl)
{
    BOOL status=FALSE;
	//SetTriggerLength(nDeviceIndex,pControl->nBufferLen,pControl->nHTriggerPos,pControl->nTimeDIV,pControl->nCHSet);//
    //Sleep(1);
    status=SetAboutInputChannelAndTrigger_6104(nDeviceIndex,pRelayControl);
    //Sleep(1);
    //status=setAmpCalibrate(nDeviceIndex,pControl,);
   
    status=SetAboutInputAdc_Only_DSO6104(nDeviceIndex,pRelayControl,pControl->nTimeDIV);
   // Sleep(1);
    return status;

}

DLL_API WORD WINAPI dsoHTSetCHAndTriggerVB(WORD nDeviceIndex,WORD* pCHEnable,WORD* pCHVoltDIV,WORD* pCHCoupling,WORD* pCHBWLimit,WORD nTriggerSource,WORD nTriggerFilt,WORD nALT,WORD nDriverCode)
{
    RELAYCONTROL RelayControl;
    for(int i = 0;i<MAX_CH_NUM;i++)
    {
        RelayControl.bCHEnable[i] = pCHEnable[i];
        RelayControl.nCHVoltDIV[i] = pCHVoltDIV[i];
        RelayControl.nCHCoupling[i] = pCHCoupling[i];
        RelayControl.bCHBWLimit[i] = pCHBWLimit[i];
        RelayControl.nTrigSource = nTriggerSource;
        RelayControl.bTrigFilt = nTriggerFilt;
        RelayControl.nALT = nALT;
    }
	return 0;
    //return dsoHTSetCHAndTrigger(nDeviceIndex,&RelayControl,nDriverCode,6);
}

//����Trigger��ͬ�����



DLL_API WORD WINAPI dsoHTSetTriggerAndSyncOutput(WORD nDeviceIndex,PCONTROLDATA pControl,WORD nTriggerMode,WORD nTriggerSlope,WORD nPWCondition,ULONG nPW,USHORT nVideoStandard,USHORT nVedioSyncSelect,USHORT nVideoHsyncNumOption,WORD nSync)
{
    return SetTriggerAndSyncOutput(nDeviceIndex,pControl,nTriggerMode,nTriggerSlope,nPWCondition,nPW,nVideoStandard,nVedioSyncSelect,nVideoHsyncNumOption,nSync);
}



//���ò�����
DLL_API WORD WINAPI dsoHTSetSampleRate(WORD nDeviceIndex,WORD *pAmpLevel,WORD nYTFormat,PRELAYCONTROL pRelayControl,PCONTROLDATA pControl)
{
    WORD status=NULL;
	
    //status=SetAboutInputAdc_clock(nDeviceIndex);
//	setAmpCalibrate(nDeviceIndex,pControl,pAmpLevel,pRelayControl->nCHVoltDIV);
	
    SetAboutInputAdc_Only_DSO6104(nDeviceIndex,pRelayControl,pControl->nTimeDIV);
    status=SetSampleRate_DSO6104(nDeviceIndex,pControl->nTimeDIV,nYTFormat);
	SetTriggerLength(nDeviceIndex,pControl->nBufferLen,pControl->nHTriggerPos,pControl->nTimeDIV,HDGetCHMode(pControl->nCHSet,pControl->nTimeDIV));
    return status;
}

//��ʼ��SDRam
DLL_API WORD WINAPI dsoHTSetAmpCalibrate(WORD nDeviceIndex,WORD nCHSet,WORD nTimeDIV,WORD *pLevel,WORD *nVoltDiv,WORD *pCHPos)
{
    return setAmpCalibrate(nDeviceIndex,nCHSet,nTimeDIV,pLevel,nVoltDiv,pCHPos);
}
DLL_API WORD WINAPI dsoHTInitSDRam(WORD nDeviceIndex)
{
    return InitSDRam(nDeviceIndex);
}

//�����ɼ�
DLL_API WORD WINAPI dsoHTStartCollectData(WORD nDeviceIndex,WORD nStartControl)
{
    return CaptureStart(nDeviceIndex,nStartControl);
}

//��������
DLL_API WORD WINAPI dsoHTStartTrigger(WORD nDeviceIndex)
{
    return TriggerEnabled(nDeviceIndex);
}

//ǿ�ƴ���
DLL_API WORD WINAPI dsoHTForceTrigger(WORD nDeviceIndex)
{
    return ForceTrigger(nDeviceIndex);
}

//��ȡ��ַ
DLL_API WORD WINAPI dsoHTGetState(WORD nDeviceIndex)
{
    //return 0x07;
    WORD nState = GetCaptureState(nDeviceIndex);
    //nState &= 0x03;//������2λ    //Error?
    return nState;
}
//��ȡ�Ƿ��Ѳɼ���512Bytes
DLL_API WORD WINAPI dsoHTGetPackState(WORD nDeviceIndex)
{
    WORD nState = GetCaptureState(nDeviceIndex);
    nState = (nState>>3) & 0x01;//ȡ����3λ//�Ƿ���512Byte��־.0:δ��,1:����
    return nState;
}

//�ж�SDRam�Ƿ��ʼ���ɹ�
DLL_API WORD WINAPI dsoHTGetSDRamInit(WORD nDeviceIndex)
{
    WORD nState = GetCaptureState(nDeviceIndex);
    nState = (nState>>4) & 0x01;//ȡ����4λ//�Ƿ��ʼ���ɹ�.0:ʧ��,1:�ɹ�
    return nState;
}

//��ȡYT-Scan����
DLL_API WORD WINAPI dsoHTGetScanData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl)
{
    WORD nRe = 0;
    if(pControl->nLastAddress == 0)//��֤��nHTriggerPos==0shi����
    {
        nRe = GetPreTrigData(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,pControl);//0:Scan
    }
    else
    {
        nRe = GetDataContinue(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,pControl,YT_SCAN);//������ȡ
    }
    return nRe;
}

//��ȡRoll����
DLL_API WORD WINAPI dsoHTGetRollData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl)
{
    WORD nRe = 0;

    nRe = GetDataContinue(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,pControl,YT_ROLL);//������ȡ

    return nRe;
}

DLL_API WORD WINAPI dsoHTStartRoll(WORD nDeviceIndex)
{/*
    ULONG RamReadLength,SDRamAddrTriggerd,RamReadStartAddr,nTemp;
    ULONG nStartOffset = 0;

    //��ȡ������ַ
    //SDRamAddrTriggerd = GetStartReadAddress(nDeviceIndex);
    //���ݴ�����ַ������ʼ������ַ
    nTemp = ULONG(SDRamAddrTriggerd);
    nStartOffset = ULONG(nTemp % USB_PACK_SIZE);
    nTemp = nTemp - nStartOffset;
    if(nTemp < 0)
    {
        RamReadStartAddr = 16777216 + nTemp;
    }
    else
    {
        RamReadStartAddr = nTemp;
    }
    //���ö�����ַ
    SetReadAddress(nDeviceIndex,RamReadStartAddr);
#ifdef TRIGGER_POS_OFFSET
    RamReadLength =  (USB_PACK_SIZE + 512);
#else
    RamReadLength =  USB_PACK_SIZE;
#endif
    //���ö�������
    int nRe = SetReadLength(nDeviceIndex,2*RamReadLength);
    return nRe;
	*/
	return 0;
}

//��ȡYT-Normal����
/*DLL_API WORD WINAPI dsoHTGetData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl)
{
//pControl->nETSOpen;
    WORD status = 0;
    WORD* pCHData[MAX_CH_NUM];
    ULONG RamReadStartAddr;//ULONG nTemp; // del by yt 20101009
    ULONG nPriTriggerLen;	//Ԥ����������ͨ�������
    ULONG SDRamAddrTriggerd;
    unsigned int nTriggerAddress;
    unsigned int nEndAddress;
    ULONG nDataLen = pControl->nReadDataLen;
    //��ȡ������ַ�Ͳɼ�������ַ
    SDRamAddrTriggerd = GetStartReadAddress(nDeviceIndex);  //��һ�� ��ȡ������ַ
    nTriggerAddress=SDRamAddrTriggerd&0xFFFF;               //������ַ
    nEndAddress=(SDRamAddrTriggerd>>16)&0xFFFF;             //�ɼ�������ַ
    PWORD ppData[4];
    //qDebug()<<"nTriggerAddress:"<<nTriggerAddress<<"nEndAddress:"<<nEndAddress<<"nEndAddress-nTriggerAddress"<<nEndAddress-nTriggerAddress;
    // qDebug()<<nEndAddress-nTriggerAddress;//4��ָ��ָ��4����������
    int nActivateCHNum=0;//������������ͨ����Ŀ
    
    {//�����nActivateCHNum����ppData[4]ָ����������Ĳ���
        
        pCHData[0] = pCH1Data;
        pCHData[1] = pCH2Data;
        pCHData[2] = pCH3Data;
        pCHData[3] = pCH4Data;
        for(int m=0;m<4;m++)
        {
            if(((pControl->nCHSet)>>m)&0x01)
            {
                ppData[nActivateCHNum++]=pCHData[m];
            }
        }
        if(nActivateCHNum==3)
        {
            for(int k=0;k<4;k++)
            {
                ppData[k]=pCHData[k];
            }
            nActivateCHNum=4;
        }
    }
    //	*p1 = SDRamAddrTriggerd;
    //����Ԥ��������=�ٷֱ�*ͨ����*nBuffer
    nPriTriggerLen = pControl->nHTriggerPos * nActivateCHNum*nDataLen/100;//Ԥ��������   �㷨����ǰ��һ������Ҫ����ͨ����//byzhang
    //���ݴ�����ַ������ʼ������ַ
    //RamReadStartAddr=nTriggerAddress >= nPriTriggerLen?nTriggerAddress- nPriTriggerLen:0x10000 - (nPriTriggerLen - nTriggerAddress);
    RamReadStartAddr=nEndAddress-nDataLen*nActivateCHNum;
    //���ö�����ַ
    SetReadAddress(nDeviceIndex,RamReadStartAddr);//�ڶ��� ���ö�ȡ��ַ
    //���ö�ȡ���Ȳ���ʼ����
    status  = ReadHardData_6104(nDeviceIndex,ppData,nActivateCHNum,nActivateCHNum*nDataLen);//������ ��ʼ����
    return status ;
}
*/
DLL_API WORD WINAPI dsoHTGetData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl)
{
    WORD status = 0;
    WORD* pCHData[MAX_CH_NUM];
    LONG RamReadStartAddr;//ULONG nTemp; // del by yt 20101009
	unsigned int nOffSetall=0;
    unsigned int nTriggerAddress;
    unsigned int nEndAddress;
	double p=1.0*pControl->nHTriggerPos/100;
    ULONG nReadLen = pControl->nReadDataLen;
	ULONG nDataLen = pControl->nBufferLen;
    //��ȡ������ַ�Ͳɼ�������ַ
    __int64  nAddress = GetStartReadAddress(nDeviceIndex);
	nTriggerAddress=WORD(nAddress&0xFFFF);               //������ַ
    nEndAddress=WORD((nAddress>>16)&0xFFFF);             //�ɼ�������ַ
	ULONG nStartOffset=WORD((nAddress>>32)&0xFFFF);

    PWORD ppData[4];
    int nActivateCHNum=0;//������������ͨ����Ŀ
    
    {//�����nActivateCHNum����ppData[4]ָ����������Ĳ���        
        pCHData[0] = pCH1Data;
        pCHData[1] = pCH2Data;
        pCHData[2] = pCH3Data;
        pCHData[3] = pCH4Data;
        for(int m=0;m<4;m++)
        {
            if(((pControl->nCHSet)>>m)&0x01)
            {
                ppData[nActivateCHNum++]=pCHData[m];
            }
        }
        if(nActivateCHNum==3)
        {
            for(int k=0;k<4;k++)
            {
                ppData[k]=pCHData[k];
            }
            nActivateCHNum=4;
        }
    }
	//����Ԥ��������=�ٷֱ�*ͨ����*nBuffer
	//nPriTriggerLen = pControl->nHTriggerPos * nActivateCHNum*pControl->nBufferLen/100;//Ԥ��������   �㷨����ǰ��һ������Ҫ����ͨ����//byzhang
    //���ݴ�����ַ������ʼ������ַ
    //RamReadStartAddr=nTriggerAddress >= nPriTriggerLen?nTriggerAddress- nPriTriggerLen:0x10000 - (nPriTriggerLen - nTriggerAddress);
   // RamReadStartAddr=nEndAddress-nReadLen*nActivateCHNum+8;
	RamReadStartAddr=nEndAddress-LONG(nActivateCHNum*((1-p)*nDataLen+p*nReadLen));
	if(RamReadStartAddr<0)
		RamReadStartAddr+=0x10000;
    //���ö�����ַ
/*
	if(RamReadStartAddr%8!=0)
	{
		if(RamReadStartAddr%8<=3)
			RamReadStartAddr-=RamReadStartAddr%8;
		else
			RamReadStartAddr+=8-RamReadStartAddr%8;
	}
	*/
	nStartOffset=7-nStartOffset&0x07;
	switch (nActivateCHNum){
	case 4:
		nStartOffset=nStartOffset&0x01+6;
		break;
	case 2:
		nStartOffset=nStartOffset&0x03+4;
		break;
	case 1:
		nStartOffset=nStartOffset&0x07;
		break;
	default :
		nStartOffset=0;
		break;
	}
	//nStartOffset=0;
	nStartOffset=nStartOffset*nActivateCHNum;
    SetReadAddress(nDeviceIndex,RamReadStartAddr+nStartOffset);//�ڶ��� ���ö�ȡ��ַ
    //���ö�ȡ���Ȳ���ʼ����
    status  = ReadHardData_6104(nDeviceIndex,ppData,nActivateCHNum,nActivateCHNum*nReadLen);//������ ��ʼ����
    return status ;
}
DLL_API WORD WINAPI dsoHTOpenRollMode(WORD nDeviceIndex)
{
    return SetRollCollect(nDeviceIndex,0x01);
}

DLL_API WORD WINAPI dsoHTCloseRollMode(WORD nDeviceIndex)
{
    return SetRollCollect(nDeviceIndex,0x00);
}

#define MAX_PEAK	0x0FFFFFFF
DLL_API WORD WINAPI dsoHTSetPeakDetect(WORD nDeviceIndex,WORD nTimeDIV,WORD nYTFormat)
{
	return 0;
    ULONG i = 0;
    ULONG nPeakValue = 0;
    ULONG nPeak[MAX_TIMEDIV_NUM];
    for(i=0;i<PEAK_START_TIMEDIV;i++)
    {
        nPeak[i] = MAX_PEAK;//
    }
    nPeak[i++] = MAX_PEAK - 18;	//10M
    nPeak[i++] = MAX_PEAK - 38;//5M
    nPeak[i++] = MAX_PEAK - 98;//2M
    nPeak[i++] = MAX_PEAK - 198;//1M
    nPeak[i++] = MAX_PEAK - 398;//500K
    nPeak[i++] = MAX_PEAK - 998;//200k
    nPeak[i++] = MAX_PEAK - 1998;//100k
    nPeak[i++] = MAX_PEAK - 3998;//50k
    nPeak[i++] = MAX_PEAK - 9998;//20k -- 50mS/div
    nPeak[i++] = MAX_PEAK - 19998;//10k
    nPeak[i++] = MAX_PEAK - 39998;//5k
    if(nTimeDIV >= MIN_ROLL_TIMEDIV)//���������4��
    {
        nPeak[i++] = MAX_PEAK - 24998;//8k	--500mS/DIV
        nPeak[i++] = MAX_PEAK - 49998;//4k -- 1.0S/DIV
        nPeak[i++] = MAX_PEAK - 99998;//2k
        nPeak[i++] = MAX_PEAK - 249998;//800
        nPeak[i++] = MAX_PEAK - 499998;//400
        nPeak[i++] = MAX_PEAK - 999998;//200
        nPeak[i++] = MAX_PEAK - 2499998;//80
        nPeak[i++] = MAX_PEAK - 4999998;//40
        nPeak[i++] = MAX_PEAK - 9999998;//20
        nPeak[i++] = MAX_PEAK - 24999998;//8
        nPeak[i++] = MAX_PEAK - 49999998;//4 -- 1000s/div--1Hz
    }
    else
    {
        nPeak[i++] = MAX_PEAK - 99998;//2k	--500mS/DIV
        nPeak[i++] = MAX_PEAK - 199998;//1k -- 1.0S/DIV
        nPeak[i++] = MAX_PEAK - 399998;//500
        nPeak[i++] = MAX_PEAK - 999998;//200
        nPeak[i++] = MAX_PEAK - 1999998;//100
        nPeak[i++] = MAX_PEAK - 3999998;//50
        nPeak[i++] = MAX_PEAK - 9999998;//20
        nPeak[i++] = MAX_PEAK - 19999998;//10
        nPeak[i++] = MAX_PEAK - 39999998;//5
        nPeak[i++] = MAX_PEAK - 99999998;//2
        nPeak[i++] = MAX_PEAK - 199999998;//1 -- 1000s/div--1Hz
    }
    nPeakValue = nPeak[nTimeDIV];
    return SetPeakMode(nDeviceIndex,0x01,nPeakValue);
}

DLL_API WORD WINAPI dsoHTClosePeakDetect(WORD nDeviceIndex)
{
	return 0;
    return SetPeakMode(nDeviceIndex,0x00,0x00);
}

/*Soft Functions**************************************************************************************************************************/
//SDK��ȡ����
WORD SDGetData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl,WORD nInsertMode)
{
    WORD nRe = 0;
    if(pControl->nTimeDIV > MAX_INSERT_TIMEDIV)//����Ҫ��ֵ
    {
        nRe = dsoHTGetData(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,pControl);
        return nRe;
    }
    else//��Ҫ��ֵ�ĵ�λ
    {
        int i = 0;
        ULONG nState[MAX_CH_NUM];
        WORD* CHData[MAX_CH_NUM];
        USHORT* nData[MAX_CH_NUM];

        CHData[0] = pCH1Data;
        CHData[1] = pCH2Data;
        CHData[2] = pCH3Data;
        CHData[3] = pCH4Data;
        memset(nState,0,sizeof(nState));
        for(i=0;i<MAX_CH_NUM;i++)
        {
            nData[i] = new USHORT[pControl->nReadDataLen - 10240 + 40960];
        }
        //����
        nRe = dsoHTGetData(nDeviceIndex,nData[0],nData[1],nData[2],nData[3],pControl);

        //		for(i=0;i<MAX_CH_NUM;i++)
        //		{
        //			delete nData[i];
        //			nData[i] = NULL;
        //		}
        //		return nRe;
        //��ֵ
        //Ӧ4250����Ҫ�����в�ֵ�������������Ҵ���
        if(pControl->nALT == 1)
        {
            for(i=0;i<MAX_CH_NUM;i++)
            {
                if(pControl->nTriggerSource == i)
                {
                    if(((pControl->nCHSet >> i) & 0x01) == 1)
                    {
                        dsoSFInsert(nData[i],CHData[i],pControl,nInsertMode,i,nState);
                    }
                }
            }
        }
        else
        {
            if(pControl->nTriggerSource < MAX_CH_NUM)
            {
#ifdef FILE_TEST
                {
                    CStdioFile file;
                    CString str;
                    if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
                    {
                        str=_T("Cann't save Test file");
                        //					AfxMessageBox(str);
                        return 0;
                    }

                    file.SeekToEnd();

                    {
                        SYSTEMTIME st;
                        GetLocalTime(&st);
                        str.Format(_T("dsoSFInsert_Trig_In::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
                    }

                    file.WriteString(str);

                    file.Close();
                    Sleep(5);
                }
#endif

                dsoSFInsert(nData[pControl->nTriggerSource],CHData[pControl->nTriggerSource],pControl,nInsertMode,pControl->nTriggerSource,nState);

#ifdef FILE_TEST
                {
                    CStdioFile file;
                    CString str;
                    if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
                    {
                        str=_T("Cann't save Test file");
                        //					AfxMessageBox(str);
                        return 0;
                    }

                    file.SeekToEnd();

                    {
                        SYSTEMTIME st;
                        GetLocalTime(&st);
                        str.Format(_T("dsoSFInsert_Trig_Out::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
                    }

                    file.WriteString(str);

                    file.Close();
                    Sleep(5);
                }
#endif

                for(i=0;i<MAX_CH_NUM;i++)
                {
                    if(pControl->nTriggerSource == i)
                    {
                        continue;
                    }
                    if(((pControl->nCHSet >> i) & 0x01) == 1)
                    {
#ifdef FILE_TEST
                        {
                            CStdioFile file;
                            CString str;
                            if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
                            {
                                str=_T("Cann't save Test file");
                                //					AfxMessageBox(str);
                                return 0;
                            }

                            file.SeekToEnd();

                            {
                                SYSTEMTIME st;
                                GetLocalTime(&st);
                                str.Format(_T("dsoSFInsert_Soft_In::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
                            }

                            file.WriteString(str);

                            file.Close();
                            Sleep(5);
                        }
#endif

                        dsoSFInsert(nData[i],CHData[i],pControl,nInsertMode,i,nState);

#ifdef FILE_TEST
                        {
                            CStdioFile file;
                            CString str;
                            if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
                            {
                                str=_T("Cann't save Test file");
                                //					AfxMessageBox(str);
                                return 0;
                            }

                            file.SeekToEnd();

                            {
                                SYSTEMTIME st;
                                GetLocalTime(&st);
                                str.Format(_T("dsoSFInsert_Soft_Out::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
                            }

                            file.WriteString(str);

                            file.Close();
                            Sleep(5);
                        }
#endif
                    }
                }
            }
            else//EXT
            {
                for(i=0;i<MAX_CH_NUM;i++)
                {
                    if(((pControl->nCHSet >> i) & 0x01) == 1)
                    {
                        dsoSFInsert(nData[i],CHData[i],pControl,nInsertMode,i,nState);
                    }
                }
            }
        }
        //
        for(i=0;i<MAX_CH_NUM;i++)
        {
            delete nData[i];
        }
    }
    return nRe;
}

DLL_API ULONG WINAPI dsoHTGetHardFC(WORD nDeviceIndex)
{
    return GetHardFandC(nDeviceIndex);
}

DLL_API WORD WINAPI dsoHTSetHardFC(WORD nDeviceIndex,ULONG nTime)
{
    return SetHardFandC(nDeviceIndex,nTime);
}

DLL_API WORD WINAPI dsoHTResetCnter(WORD nDeviceIndex)
{
    ResetCounter(nDeviceIndex,0x03);
    return ResetCounter(nDeviceIndex,0x01);
}

/*
{
                WORD nCHSet;
        WORD nTimeDIV;
        WORD nTriggerSource;
        WORD nHTriggerPos;
                WORD nVTriggerPos;
                WORD nTriggerSlope;
                ULONG nBufferLen;//��Ӧ��10K��1M��2M.....16M
                ULONG nReadDataLen;//��¼���ν�Ҫ��Ӳ����ȡ�������ܹ�����
                ULONG nAlreadyReadLen;//��¼�����Ѿ���ȡ�����ݳ��ȣ���ɨ��/����ģʽ����Ч,��NORMALģʽ����Ч
        WORD nALT;
                WORD nETSOpen;

}CONTROLDATA,*PCONTROLDATA;
*/

//���º���SDKҲ������
DLL_API WORD WINAPI dsoSDGetData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl,WORD nInsertMode)
{
    CONTROLDATA Control;

    Control.nCHSet = pControl->nCHSet;

    Control.nTimeDIV = pControl->nTimeDIV;

    Control.nTriggerSource = pControl->nTriggerSource;

    Control.nHTriggerPos = pControl->nHTriggerPos;

    Control.nVTriggerPos = pControl->nVTriggerPos;

    Control.nTriggerSlope = pControl->nTriggerSlope;

    Control.nBufferLen = pControl->nBufferLen;

    Control.nReadDataLen = pControl->nReadDataLen;

    Control.nAlreadyReadLen = pControl->nAlreadyReadLen;

    Control.nALT = pControl->nALT;

    Control.nETSOpen = pControl->nETSOpen;

    return SDGetData(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,&Control,nInsertMode);
}

//��ȡYT-Scan����
DLL_API WORD WINAPI dsoSDHTGetScanData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl)
{
    CONTROLDATA Control;

    Control.nCHSet = pControl->nCHSet;

    Control.nTimeDIV = pControl->nTimeDIV;

    Control.nTriggerSource = pControl->nTriggerSource;

    Control.nHTriggerPos = pControl->nHTriggerPos;

    Control.nVTriggerPos = pControl->nVTriggerPos;

    Control.nTriggerSlope = pControl->nTriggerSlope;

    Control.nBufferLen = pControl->nBufferLen;

    Control.nReadDataLen = pControl->nReadDataLen;

    Control.nAlreadyReadLen = pControl->nAlreadyReadLen;

    Control.nALT = pControl->nALT;

    Control.nETSOpen = pControl->nETSOpen;

    WORD nRe = 0;
    if(pControl->nAlreadyReadLen == 0)//��1�ζ�ȡ
    {
        nRe = GetPreTrigData(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,&Control/*,0*/);//0:Scan
    }
    else
    {
        nRe = SDGetDataContinue(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,&Control,YT_SCAN);//������ȡ
    }
    return nRe;
}

//��ȡRoll����
DLL_API WORD WINAPI dsoSDHTGetRollData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl)
{
    CONTROLDATA Control;

    Control.nCHSet = pControl->nCHSet;

    Control.nTimeDIV = pControl->nTimeDIV;

    Control.nTriggerSource = pControl->nTriggerSource;

    Control.nHTriggerPos = pControl->nHTriggerPos;

    Control.nVTriggerPos = pControl->nVTriggerPos;

    Control.nTriggerSlope = pControl->nTriggerSlope;

    Control.nBufferLen = pControl->nBufferLen;

    Control.nReadDataLen = pControl->nReadDataLen;

    Control.nAlreadyReadLen = pControl->nAlreadyReadLen;

    Control.nALT = pControl->nALT;

    Control.nETSOpen = pControl->nETSOpen;

    WORD nRe = 0;
    nRe = SDGetDataContinue(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,&Control,YT_ROLL);//������ȡ
    return nRe;
}

//��ȡӲ���汾
DLL_API ULONG WINAPI dsoGetHardVersion(WORD nDeviceIndex)
{
    return GetHardVersion(nDeviceIndex);
}

DLL_API BOOL WINAPI dsoSetUSBBus(WORD DeviceIndex)//for usb
{

    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_OUT;
    UCHAR VendNo=0xEA;
    UCHAR pBuffer[10];
    HANDLE hDevice=NULL;
    ULONG ncnt = 10;
    //��ʼ���豸��
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice==NULL)
    {
        return FALSE;
    }
    USHORT value = 0x0000; //��ȡ24LC64����ʼ��ַ
    bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,ncnt);
    CloseDevice(hDevice);


    return bResult;

}

DLL_API BOOL WINAPI dsoSetSPIBus(WORD DeviceIndex)//for SPI
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_OUT;
    UCHAR VendNo=0xE5;
    UCHAR pBuffer[10];
    HANDLE hDevice;
    ULONG ncnt;
    //��ʼ���豸��
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    ncnt=10;
    hDevice = OpenDevice(pcDriverName);
    if(hDevice==NULL)
    {
        return FALSE;
    }
    USHORT value = 0x0000; //��ȡ24LC64����ʼ��ַ
    bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,ncnt);
    CloseDevice(hDevice);
    return TRUE;
}

DLL_API BOOL WINAPI dsoSetHardInfo(WORD DeviceIndex,UCHAR* pBuffer)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_OUT;
    UCHAR VendNo=0xa2; //��дEEPROM
    HANDLE hDevice = NULL;


    USHORT value = 0x1380+EE_OFFSET; //��ȡ24LC64����ʼ��ַ

    SelectDeviceIndex(DeviceIndex,pcDriverName);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice == NULL)
    {
        return FALSE;
    }
    bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,DEV_INFO_LEN+1);
    CloseDevice(hDevice);
    return TRUE;
}

DLL_API BOOL WINAPI dsoGetHardInfo(WORD DeviceIndex,UCHAR* pBuffer)
{
    //	return 0;
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_IN;
    UCHAR VendNo=0xa2; //��дEEPROM����
    HANDLE hDevice=NULL;
    ULONG ncnt;
    ncnt=DEV_INFO_LEN+1;//��ȡ24LC64�ĳ��ȣ����4K

    USHORT value = 0x1380+EE_OFFSET; //��ȡ24LC64����ʼ��ַ

    SelectDeviceIndex(DeviceIndex,pcDriverName);
    hDevice = OpenDevice(pcDriverName);

    if(hDevice == NULL)
    {
        return FALSE;
    }
    bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,ncnt);
    CloseDevice(hDevice);

    return TRUE;
}
DLL_API BOOL WINAPI dsoGetDDSCali(WORD DeviceIndex,WORD* level,WORD length)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_IN;
    UCHAR VendNo=0xA2; //��дEEPROM����
    UCHAR pBuffer[512];
    HANDLE hDevice;
    ULONG ncnt,i;

    USHORT value = 0x13E0+EE_OFFSET; //��ȡ24LC64����ʼ��ַ

    ncnt=length*2;//��ȡ24LC64�ĳ��ȣ����4K

    SelectDeviceIndex(DeviceIndex,pcDriverName);
    ResetDevice(DeviceIndex,1);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice == NULL)
    {
        return 0;
    }
    bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,ncnt);
    CloseDevice(hDevice);
    for(i=0;i<length;i++)
    {
        level[i]=pBuffer[2*i]+(pBuffer[2*i+1]<<8);
    }
    return 1;
}
DLL_API BOOL WINAPI dsoSetDDSCali(WORD DeviceIndex,WORD* level,WORD length)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_OUT;
    UCHAR VendNo=0xa2; //��дEEPROM
    UCHAR pBuffer[512];
    HANDLE hDevice = NULL;
    WORD i;
    USHORT value = 0x13E0+EE_OFFSET; //��ȡ24LC64����ʼ��ַ
    for(i=0;i<length;i++)
    {
        pBuffer[2*i]=(level[i]) & 0xFF;//��8bit
        pBuffer[2*i+1]=((level[i])>>8) & 0xFF;//��8bit
    }

    SelectDeviceIndex(DeviceIndex,pcDriverName);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice == NULL)
    {
        return 0;
    }
    bResult=VendRequest(hDevice,VendNo,value,bDir,pBuffer,length*2);
    CloseDevice(hDevice);
    return 1;
}

DLL_API WORD WINAPI dsoGetDriverVersion(WORD DeviceIndex,UCHAR* pBuffer)
{
	//��ȡ�̶�2Bit��4��CHAR  by zhang 20160128
	char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_IN;
    UCHAR VendNo=0xE3; //��дEEPROM����
    HANDLE hDevice=NULL;
    ULONG ncnt;
    ncnt=(DRIVERVERLEN/2);//��ȡ24LC64�ĳ��ȣ����4K
	UCHAR pTemp[DRIVERVERLEN/2];
    USHORT value = 0x13F0+EE_OFFSET; //��ȡ24LC64����ʼ��ַ
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice == NULL)
    {
        return FALSE;
    }
    bResult=VendRequest(hDevice,VendNo,value,bDir,pTemp,ncnt);
	for(int i=0;i<DRIVERVERLEN;i++){
		int flag=i%2;
		if(flag==0){
			pBuffer[i]=(pTemp[i/2]>>4)+'0';
		}
		else{
			pBuffer[i]=(pTemp[(i-1)/2]&0x0F)+'0';
		}		
	}
    CloseDevice(hDevice);
	return bResult;    
}

DLL_API BOOL WINAPI dsoGetDeviceSN(WORD DeviceIndex,UCHAR* pBuffer)
{
    int i=0;
    int nLen = 0;
    UCHAR chInfo[DEV_INFO_LEN];
    memset(chInfo,0,sizeof(chInfo));
    dsoGetHardInfo(DeviceIndex,chInfo);
    if(chInfo[0] == 0xFF)
    {
        return FALSE;
    }
    else
    {
        //��Ʒ����--
        nLen += TYPE_LEN;

        //��Ʒ����--
        nLen += NAME_LEN;

        //PCB�汾��
        nLen += PCB_LEN;

        //�����汾��
        nLen += DRIVER_LEN;

        //����С��
        nLen += PRODUCTOR_LEN;

        //��װС��
        nLen += PACKAGER_LEN;

        //��Ʒ���
        for(i=0;i<SN_LEN;i++)
        {
            if(chInfo[nLen+i] == 0xFF)
            {
                continue;
            }
            pBuffer[i] = chInfo[nLen+i];
        }
        /*
//��������
                i = nLen;
                nLen += PRODUCE_LEN;

//��������
                i = nLen;
                nLen += TESTTIME_LEN;

//����Ա
                i = nLen;
                nLen += TESTSN_LEN;

//FPGA
                i = nLen;
                nLen += FPGA_LEN;
                */
        return TRUE;
    }
}

DLL_API WORD WINAPI dsoUSBModeSetIPAddr(WORD DeviceIndex,ULONG nIP,ULONG nSubnetMask,ULONG nGateway,USHORT nPort,BYTE* pMac)
{
    return SetDeviceIP(DeviceIndex,nIP,nSubnetMask,nGateway,nPort,pMac);
}

DLL_API WORD WINAPI dsoUSBModeGetIPAddr(WORD DeviceIndex,ULONG* pIP,ULONG* pSubMask,ULONG* pGateway,WORD* pPort,BYTE* pMac)
{
    return GetDeviceIP(DeviceIndex,pIP,pSubMask,pGateway,pPort,pMac);
}

DLL_API WORD WINAPI dsoSetLANEnable(WORD DeviceIndex,short nEnable)
{
    return SetLANEnable(DeviceIndex,nEnable);
}

DLL_API WORD WINAPI dsoGetLANEnable(WORD DeviceIndex,short* pEnable)
{
#ifdef _LAUNCH
    return 0x04;
#else
    return GetLANEnable(DeviceIndex,pEnable);
#endif
}

DLL_API WORD WINAPI dsoOpenLan(WORD DeviceIndex,WORD nMode)
{
    return OpenLan(DeviceIndex,nMode);
}


DLL_API BOOL WINAPI dsoWriteFlash(WORD DeviceIndex,UCHAR* pBuffer)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_OUT;
    UCHAR VendNo=0xF0; //��дEEPROM
    HANDLE hDevice = NULL;
    USHORT value = 0x1E00;//

    SelectDeviceIndex(DeviceIndex,pcDriverName);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice == NULL)
    {
        return FALSE;
    }
    for(int i=0;i<8192;i++)//512K���ֽ�.ÿ��д64���ֽ�
    {
        bResult=VendRequest(hDevice,VendNo,value,bDir,(pBuffer+i*64),64);
    }
    CloseDevice(hDevice);
    return TRUE;
}

DLL_API BOOL WINAPI dsoReadFlash(WORD DeviceIndex,UCHAR* pBuffer)
{
    char pcDriverName[MAX_DRIVER_NAME] = "";
    BOOL bResult=FALSE;
    BOOL bDir=DIR_IN;
    UCHAR VendNo=0xF1; //��дEEPROM����
    HANDLE hDevice;
    ULONG ncnt;
    USHORT value = 0x1E00;//

    ncnt=64;

    SelectDeviceIndex(DeviceIndex,pcDriverName);
    hDevice = OpenDevice(pcDriverName);
    if(hDevice == NULL)
    {
        return FALSE;
    }
    for(int i=0;i<8192;i++)//512K���ֽ�.ÿ��д64���ֽ�
    {
        bResult=VendRequest(hDevice,VendNo,value,bDir,(pBuffer+i*ncnt),ncnt);
    }
    CloseDevice(hDevice);

    return TRUE;
}
/*
DLL_API WORD WINAPI dsoGetLANEnable(WORD DeviceIndex)
{
        return GetLANEnable(DeviceIndex);
}
*/

DLL_API WORD WINAPI dsoOpenWIFIPower(WORD DeviceIndex)
{
    return OpenWIFIPower(DeviceIndex);
}

DLL_API WORD WINAPI dsoResetWIFI(WORD DeviceIndex)
{
    return ResetWIFI(DeviceIndex);
}

DLL_API WORD WINAPI dsoGetFPGAVersion(WORD DeviceIndex)  //ADC Init by zhang
{
    BOOL status=FALSE;

     //
    //Sleep(1);
    return GetFPGAVersion(DeviceIndex);
}
DLL_API WORD WINAPI dsoInitADCOnce(WORD DeviceIndex)
{
	return setInitAdcOnce(DeviceIndex);
}
DLL_API WORD WINAPI dsoGetUSBModulVersion(WORD DeviceIndex)
{
    return GetUSBModulVersion(DeviceIndex);
}
//=================DDS-----------------------
DLL_API ULONG WINAPI ddsSetOnOff(WORD DeviceIndex,short nOnOff)
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize=4;
    WORD status=FALSE;
	UCHAR cOnOff=nOnOff?0x00:0x01;//ע��01�ǹر���ԭ���෴
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x30;
    outBuffer[1]=0x00;
    outBuffer[2]=0xFF&(cOnOff&0x01);
	outBuffer[3]=0x00;
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);   //need modified //Error
    return status;
}

DLL_API ULONG WINAPI ddsSetFrequency(WORD DeviceIndex,double dbFre,WORD* pWaveNum,WORD* pPeriodNum)
{
	ddsSetOnOff(DeviceIndex,0x00);
    int iNumber = 0;
    double dT;
	double dF;
    float flNum;
    USHORT DivNum = 0;//��Ƶ����
    int i;
	dF=fabs(dbFre - 0) < 1?1:dbFre;
    dT = 1.0  / dF ;

    if (dF <= 200000 )	//��Ƶ��С��200K���ǰ���ǰ�ķ���
    {
        DivNum  = int(MAX_CLOCK*dT/MAX_BUFFER);	//�����Ƶ��
        if (DivNum == 0)
            iNumber = (int)(dT * MAX_CLOCK);
        else
            iNumber = (int)(dT * 200000000 / DivNum);
        *pPeriodNum = 1;//buffer�洢һ������
    }
    else
    {
        DivNum = 0;
        flNum = (float)(MAX_CLOCK / dF);
        i = 1;
        while(1)
        {
            if (flNum * (i + 1) > MAX_BUFFER)
                break;
            i++;
        }
        iNumber = (int)(flNum*i);
        *pPeriodNum = i;
    }
    if (iNumber > MAX_BUFFER)
    {
        iNumber = MAX_BUFFER;
    }
    *pWaveNum = iNumber;
    //
	PUCHAR outBuffer;
	BOOL status;
    UINT m_nSize = 4;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]= 0x31;//���÷�Ƶ��
    outBuffer[1]= 0x00;
    outBuffer[2]= 0xFF & DivNum;
    outBuffer[3]= 0xFF & (DivNum>>8);
	status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);   //need modified //Error
	ddsSetOnOff(DeviceIndex,0x01);
    return status;
}

DLL_API ULONG WINAPI ddsDownload(WORD DeviceIndex,WORD iWaveNum, WORD* pData)
{
    PUCHAR outWaveBuffer = NULL;
    BOOL status=FALSE;
    UINT m_nSize=4;
	UINT m_nNumEach=0;//ÿ��ʵ�����·���������
    ULONG nWriteTimes = 0;	
	m_nSize=4;
	PUCHAR outBuffer = NULL;
	outBuffer=(PUCHAR)malloc(m_nSize*sizeof(UCHAR));
	memset(outBuffer,0,m_nSize*sizeof(UCHAR));
    outBuffer[0]=0x33;//1:������ַ
    outBuffer[1]=0x00;
    outBuffer[2]=0xFF&(iWaveNum-1);
    outBuffer[3]=0xFF&((iWaveNum-1) >> 8);//������ַ����λ�ں�
	status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
    m_nSize = 512;//һ���·�����
	m_nNumEach=(m_nSize-2)/2;//ÿ��ʵ�����·��Ĳ������ݵĸ��� ע������size�Ǳ�����ϵ
	m_nNumEach=m_nNumEach>250?250:m_nNumEach;
    outWaveBuffer=(PUCHAR)malloc(m_nSize);
	int nOffSet=iWaveNum%(m_nNumEach);
	nWriteTimes=(iWaveNum-nOffSet)/(m_nNumEach);
	nWriteTimes+=nOffSet?1:0;
	outWaveBuffer[0]=0x32;
	//outWaveBuffer[1]=0x00;//ǡ����255����������
	UINT m_nNumCopy=0;//16bit ���ݸ���
	ULONG xxxxx=0;
    for(UINT i=0;i<nWriteTimes;i++) //���ѭ����������
    {
        //2:����42����+�������ݰ�������
        memset(outWaveBuffer+2,0,(m_nSize-2));
		m_nNumCopy=(i+1)*m_nNumEach<=iWaveNum?m_nNumEach:(iWaveNum%m_nNumEach);
		for(UINT j=0;j<m_nNumCopy;j++)
		{
			/*if(m_nNumCopy!=m_nNumEach)
			{
				realloc(outWaveBuffer,m_nNumCopy*2+2);
				
			}
			*/
			outWaveBuffer[1]=0xFF&(m_nNumCopy);
			*(outWaveBuffer+2+2*j)=(*(pData+i*m_nNumEach+j))&0xFF;
			*(outWaveBuffer+2+2*j+1)=((*(pData+i*m_nNumEach+j))>>8)&0xFF;
			//xxxxx++;
			
		}
		//memcpy(outWaveBuffer+2,pData+i*m_nNumEach,m_nNumCopy*sizeof(UCHAR));
		status=sendOutBuffer(DeviceIndex,m_nNumCopy*2+2,outWaveBuffer);
		xxxxx+=m_nNumCopy*2;
    }
    free(outWaveBuffer);//�ͷŻ�����
    return status;
}
DLL_API ULONG WINAPI ddsEmitSingle(WORD DeviceIndex)
{

    BOOL status=FALSE;
    UINT m_nSize=4;
	PUCHAR outBuffer = NULL;
	outBuffer=(PUCHAR)malloc(m_nSize*sizeof(UCHAR));
	memset(outBuffer,0,m_nSize*sizeof(UCHAR));
    outBuffer[0]=0x34;//1:������ַ
    outBuffer[1]=0x00;
    outBuffer[2]=0x44;
    outBuffer[3]=0x00;//������ַ����λ�ں�
	status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
	ddsSetCmd(DeviceIndex,1);
    return status;
}
DLL_API ULONG WINAPI ddsSetSyncOut(WORD DeviceIndex, BOOL bEnable)
{
	return 1;//DSO6104sɾ���������
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL;
    BULK_TRANSFER_CONTROL   outBulkControl;
    HANDLE hOutDevice=NULL;
    UINT m_nSize = 2;

    SelectDeviceIndex(DeviceIndex,pcDriverName);	//��ʼ���豸��
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }

    outBuffer=(PUCHAR) malloc(m_nSize);
    if(bEnable == 0)
    {
        outBuffer[0]= 0x3A;//�ر�
        outBuffer[1]= 0x00;
    }
    else
    {
        outBuffer[0]= 0x39;//��
        outBuffer[1]= 0x00;
    }


    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status =  DeviceIoControl (hOutDevice,
                               IOCTL_EZUSB_BULK_WRITE,
                               (PVOID)&outBulkControl,
                               sizeof(BULK_TRANSFER_CONTROL),
                               outBuffer,//���������
                               m_nSize,//�ֽ������ڶԻ����п�������
                               &BytesReturned,//�����ֽ�����
                               NULL);
#endif
    free(outBuffer);
    CloseDevice(hOutDevice);//�ر��豸

    return status;
}

/*
���п��ƼĴ���CONREG�Ĺ������£�
CONREG(1)Ϊ0ʱΪNomalģʽ Ϊ1ʱ��λ������

CONREG(2)Ϊ0ʱ������1Ϊ���Σ�ͬ3005��CONreg(0)��

CONREG(3)Ϊ1�ⲿ�����أ�0Ϊ�ⲿ�½��أ�ͬ3005��CONreg(1)��
CONREG(4)Ϊ0ʱΪ�ڴ���ģʽ Ϊ1�ⴥ��ģʽ��ͬ3005��CONreg(2)��
CONREG(5)Ϊ��������Ͳ����źţ�ͬ3005��CONreg(4)����
�����ó��ⴥ��֮ǰ����1һ��ʱ��(100ms)Ȼ����0,��ͨ��CONREG(4)��1�����ⴥ���������ó��ڴ�������Ҫ���ý���λ��1Ȼ����0������һ���ڴ����źš�
CONREG����λдΪ0

*/
DLL_API ULONG WINAPI ddsSetCmd(WORD DeviceIndex, USHORT nSingle)
{
    PUCHAR outBuffer = NULL;
    UINT m_nSize = 4;
	BOOL status;
    outBuffer=(PUCHAR)malloc(m_nSize);
    memset(outBuffer,0,m_nSize);
    outBuffer[0]= 0x34;//
    outBuffer[1]= 0x00;
    outBuffer[2]= 0xFF & ((nSingle & 0x01) << 2);
    outBuffer[3]= 0x00;//��λ
	status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
    return status;
}

//----------------�����жϵ�������ȡ------------2015.02.03-------------------


WORD ReadHardData_Suspend(WORD DeviceIndex,WORD* CH1Data, WORD* CH2Data,WORD* CH3Data, WORD* CH4Data,ULONG nReadLen,USHORT* nSuspend)
{
#ifdef _LAUNCH_TEST
    BOOL bReading = TRUE;
#endif

    ULONG i,j;
    char pcDriverName[MAX_DRIVER_NAME] = "";
    PUCHAR outBuffer = NULL,inBuffer = NULL;
    BULK_TRANSFER_CONTROL   outBulkControl;
    BULK_TRANSFER_CONTROL   inBulkControl;
    HANDLE hOutDevice=NULL, hInDevice=NULL;
    BOOLEAN status=FALSE;
    ULONG BytesReturned=0;
    UINT m_nSize;
    ULONG nReadTimes = 0;

    //׼����ȡ����
    //��ʼ���豸��
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    m_nSize=2;
    outBuffer=(PUCHAR)malloc(m_nSize);
    outBuffer[0]=0x05;
    outBuffer[1]=0x00;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status = DeviceIoControl(hOutDevice,
                             IOCTL_EZUSB_BULK_WRITE,
                             (PVOID)&outBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             outBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                             NULL);
#endif

    free(outBuffer);	//�ر��豸
    CloseDevice(hOutDevice);
    //��ʼ��ȡ����
    if(PCUSBSpeed(DeviceIndex))
    {
        m_nSize = USB_PACK_SIZE;
    }
    else
    {
        m_nSize = 64;
    }
    //���豸
    hInDevice = OpenDevice(pcDriverName);
    if(hInDevice == NULL)
    {
        return 0;
    }
    inBuffer=(PUCHAR)malloc(m_nSize);
    memset(inBuffer, 0,m_nSize);
    nReadTimes = nReadLen / m_nSize;
    USHORT nStep = (m_nSize / MAX_CH_NUM);
    inBulkControl.pipeNum = 1;//EP6IN

    for(i=0;i<nReadTimes;i++) //���ѭ������
    {
        if(0 != *nSuspend)
        {
            status = 0;
            break;
        }
#ifdef _ASY
        status = TransferDataFromDevice(inBuffer,m_nSize);
#else
        status = DeviceIoControl(hInDevice,
                                 IOCTL_EZUSB_BULK_READ,
                                 (PVOID)&inBulkControl,
                                 sizeof(BULK_TRANSFER_CONTROL),
                                 (LPVOID)inBuffer,//���������
                                 m_nSize,//�ֽ������ڶԻ����п�������
                                 &BytesReturned,//�����ֽ�����
                                 NULL);//����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
#endif
#ifdef _LAUNCH_TEST
        if(BytesReturned != m_nSize)
        {
            bReading = FALSE;
            break;
        }
#endif
        if(status==0)
        {
            break;
        }
        for (j=0;j<nStep;j++)
        {
            *(CH1Data+i*nStep+j) = inBuffer[MAX_CH_NUM*j+0];
            *(CH2Data+i*nStep+j) = inBuffer[MAX_CH_NUM*j+1];
            *(CH3Data+i*nStep+j) = inBuffer[MAX_CH_NUM*j+2];
            *(CH4Data+i*nStep+j) = inBuffer[MAX_CH_NUM*j+3];
        }

    }
    free(inBuffer);//�ͷŻ�����
    CloseDevice(hInDevice);//�ر��豸

#ifdef _LAUNCH_TEST
    if(bReading)
    {
        return status;
    }
    else
    {
        return 3064;
    }
#else
    return status;
#endif
}

//��ȡYT-Normal����
WORD dsoHTGetData_Suspend(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl,USHORT* nSuspend)
{/*
    WORD nRE = 0;
    WORD* pCHData[MAX_CH_NUM];
    ULONG RamReadStartAddr;//ULONG nTemp; // del by yt 20101009
    double dbTemp;	//add by yt 20101009
    ULONG RamReadLength,SDRamAddrTriggerd;
    ULONG i = 0,nStartOffset = 0;
    ULONG nDataLen = pControl->nReadDataLen;

    //��ȡ������ַ
    //SDRamAddrTriggerd = GetStartReadAddress(nDeviceIndex);
    //	*p1 = SDRamAddrTriggerd;
    //���ݴ�����ַ������ʼ������ַ
    dbTemp = pControl->nHTriggerPos * nDataLen/100.0;//add by yt 20101009 --- start ---
    if(SDRamAddrTriggerd >= ULONG(dbTemp))
    {
        RamReadStartAddr = ULONG(SDRamAddrTriggerd - dbTemp);//�����ȡ���ݵ���ʼ��ַ
    }
    else//ע��: ���õ��Ĵ�����ַ < Ԥ�������� ʱ����Ҫ�����´���
    {
        RamReadStartAddr = ULONG(16777216 - (dbTemp - SDRamAddrTriggerd));//�����ȡ���ݵ���ʼ��ַ
    }
    nStartOffset = RamReadStartAddr % USB_PACK_SIZE;//������ȡ��ַ��ʹ�� = 512��������
    RamReadStartAddr = RamReadStartAddr - nStartOffset;//add by yt 20101009 --- end ---

																			//del by yt 20101009

    //���ö�����ַ
    SetReadAddress(nDeviceIndex,RamReadStartAddr);
    //	*p2 = RamReadStartAddr;
#ifdef TRIGGER_POS_OFFSET
    RamReadLength = 2 * (nDataLen + USB_PACK_SIZE*2);
#else
    RamReadLength = 2 * (nDataLen + USB_PACK_SIZE);
#endif
    //���ö�������
    SetReadLength(nDeviceIndex,RamReadLength);
    //	*p3 = RamReadLength;
    //������
    for(i=0;i<MAX_CH_NUM;i++)
    {
#ifdef TRIGGER_POS_OFFSET
        pCHData[i] = new WORD[nDataLen+USB_PACK_SIZE*2];
#else
        pCHData[i] = new WORD[nDataLen+USB_PACK_SIZE];
#endif
    }
    nRE = ReadHardData_Suspend(nDeviceIndex,pCHData[0],pCHData[1],pCHData[2],pCHData[3],2*RamReadLength,nSuspend);
    //�������
#ifdef TRIGGER_POS_OFFSET
    nStartOffset = USB_PACK_SIZE + nStartOffset;//����������λ��
#endif
    for(i=0;i<nDataLen;i++)
    {
        *(pCHData[0]+i) = *(pCHData[0]+nStartOffset+i);
        *(pCHData[1]+i) = *(pCHData[1]+nStartOffset+i);
        *(pCHData[2]+i) = *(pCHData[2]+nStartOffset+i-1);
        *(pCHData[3]+i) = *(pCHData[3]+nStartOffset+i-1);
        //��λ�������CH1��CH3���1���㣬CH2��CH4���һ���㡣 CH1/CH3 �� CH2/CH4���0.5����			//add by yt 20110927
        //����ֻ����CH1��CH3֮�䡢CH2��CH4֮�����λ�CH1/CH3 �� CH2/CH4����λ������ﴦ��			//add by yt 20110927
    }
    if(pControl->nETSOpen == 0)
    {
        if(pControl->nALT == 1)//����
        {
            if(pControl->nTriggerSource == CH1)
            {
                if(pControl->nTimeDIV > MAX_INSERT_TIMEDIV)
                {
                    for(i=0;i<nDataLen;i++)
                    {
                        *(pCH1Data+i) = *(pCHData[0]+i);
                    }
                }
                else
                {
                    //#ifdef _LAUNCH
                    //				dsoSFProcessALTData(pCH1Data,pCHData[1],pCHData[0],nDataLen,pControl->nHTriggerPos,0);
                    //#else
                    dsoSFProcessALTData(pCH1Data,pCHData[0],pCHData[1],nDataLen,pControl->nHTriggerPos,0);
                    //#endif
                }
            }
            else if(pControl->nTriggerSource == CH2)
            {
                if(pControl->nTimeDIV > MAX_INSERT_TIMEDIV)
                {
                    for(i=0;i<nDataLen;i++)
                    {
                        *(pCH2Data+i) = *(pCHData[1]+i);
                    }
                }
                else
                {
                    //#ifdef _LAUNCH
                    //				dsoSFProcessALTData(pCH2Data,pCHData[1],pCHData[0],nDataLen,pControl->nHTriggerPos,0);
                    //#else
                    dsoSFProcessALTData(pCH2Data,pCHData[0],pCHData[1],nDataLen,pControl->nHTriggerPos,0);
                    //#endif
                }
            }
            else if(pControl->nTriggerSource == CH3)
            {
                if(pControl->nTimeDIV > MAX_INSERT_TIMEDIV)
                {
                    for(i=0;i<nDataLen;i++)
                    {
                        *(pCH3Data+i) = *(pCHData[2]+i);
                    }
                }
                else
                {
                    //#ifdef _LAUNCH
                    //				dsoSFProcessALTData(pCH3Data,pCHData[3],pCHData[2],nDataLen,pControl->nHTriggerPos,0);
                    //#else
                    dsoSFProcessALTData(pCH3Data,pCHData[2],pCHData[3],nDataLen,pControl->nHTriggerPos,0);
                    //#endif
                }
            }
            else if(pControl->nTriggerSource == CH4)
            {
                if(pControl->nTimeDIV > MAX_INSERT_TIMEDIV)
                {
                    for(i=0;i<nDataLen;i++)
                    {
                        *(pCH4Data+i) = *(pCHData[3]+i);
                    }
                }
                else
                {
                    //#ifdef _LAUNCH
                    //						dsoSFProcessALTData(pCH4Data,pCHData[3],pCHData[2],nDataLen,pControl->nHTriggerPos,0);
                    //#else
                    dsoSFProcessALTData(pCH4Data,pCHData[2],pCHData[3],nDataLen,pControl->nHTriggerPos,0);
                    //#endif
                }
            }
        }
        else//�ǽ���
        {
 
            {
                //CH1+CH2
                if((pControl->nCHSet & 0x03) != 0x03)//��ͨ��
                {
                    if(((pControl->nCHSet >> 0) & 0x01) == 0x01)//CH1��,CH2�ر�
                    {
                        if(pControl->nTimeDIV > MAX_INSERT_TIMEDIV)
                        {
                            for(i=0;i<nDataLen;i++)
                            {
                                *(pCH1Data+i) = *(pCHData[0]+i);
                            }
                        }
                        else
                        {
                            //#ifdef _LAUNCH
                            //						dsoSFProcessALTData(pCH1Data,pCHData[1],pCHData[0],nDataLen,pControl->nHTriggerPos,0);
                            //#else
                            dsoSFProcessALTData(pCH1Data,pCHData[0],pCHData[1],nDataLen,pControl->nHTriggerPos,0);
                            //#endif
                        }
                    }
                    else//CH2��,CH1�ر�
                    {
                        if(pControl->nTimeDIV > MAX_INSERT_TIMEDIV)
                        {
                            for(i=0;i<nDataLen;i++)
                            {
                                *(pCH2Data+i) = *(pCHData[1]+i);
                            }
                        }
                        else
                        {
                            //#ifdef _LAUNCH
                            //						dsoSFProcessALTData(pCH2Data,pCHData[1],pCHData[0],nDataLen,pControl->nHTriggerPos,0);
                            //#else
                            dsoSFProcessALTData(pCH2Data,pCHData[0],pCHData[1],nDataLen,pControl->nHTriggerPos,0);
                            //#endif
                        }
                    }
                }
                else//˫ͨ��
                {
                    for(i=0;i<nDataLen;i++)
                    {
                        *(pCH1Data+i) = *(pCHData[0]+i);
                        *(pCH2Data+i) = *(pCHData[1]+i);
                    }
                }
                //CH3+CH4
                if(((pControl->nCHSet>>2) & 0x03) != 0x03)//��ͨ��
                {
                    if(((pControl->nCHSet >> 2) & 0x01) == 0x01)//CH3��,CH4�ر�
                    {
                        if(pControl->nTimeDIV > MAX_INSERT_TIMEDIV)
                        {
                            for(i=0;i<nDataLen;i++)
                            {
                                *(pCH3Data+i) = *(pCHData[2]+i);
                            }
                        }
                        else
                        {
                            //#ifdef _LAUNCH
                            //						dsoSFProcessALTData(pCH3Data,pCHData[3],pCHData[2],nDataLen,pControl->nHTriggerPos,0);
                            //#else
                            dsoSFProcessALTData(pCH3Data,pCHData[2],pCHData[3],nDataLen,pControl->nHTriggerPos,0);
                            //#endif
                        }
                    }
                    else//CH4��,CH3�ر�
                    {
                        if(pControl->nTimeDIV > MAX_INSERT_TIMEDIV)
                        {
                            for(i=0;i<nDataLen;i++)
                            {
                                *(pCH4Data+i) = *(pCHData[3]+i);
                            }
                        }
                        else
                        {
                            //#ifdef _LAUNCH
                            //						dsoSFProcessALTData(pCH4Data,pCHData[3],pCHData[2],nDataLen,pControl->nHTriggerPos,0);
                            //#else
                            dsoSFProcessALTData(pCH4Data,pCHData[2],pCHData[3],nDataLen,pControl->nHTriggerPos,0);
                            //#endif
                        }
                    }
                }
                else//˫ͨ��
                {
                    for(i=0;i<nDataLen;i++)
                    {
                        *(pCH3Data+i) = *(pCHData[2]+i);
                        *(pCH4Data+i) = *(pCHData[3]+i);
                    }
                }
            }
        }
    }
    else//ETS
    {
        for(i=0;i<nDataLen;i++)
        {
            *(pCH1Data+i) = *(pCHData[0]+i);
            *(pCH2Data+i) = *(pCHData[1]+i);
            *(pCH3Data+i) = *(pCHData[2]+i);
            *(pCH4Data+i) = *(pCHData[3]+i);
        }
    }

    for(i=0;i<MAX_CH_NUM;i++)
    {
        delete pCHData[i];
    }
    return nRE;
	*/
return 0;
}

//SDK��ȡ����
WORD SDGetData_Suspend(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl,WORD nInsertMode,USHORT* nSuspend)
{
    WORD nRe = 0;
    if(pControl->nTimeDIV > MAX_INSERT_TIMEDIV)//����Ҫ��ֵ
    {
        nRe = dsoHTGetData_Suspend(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,pControl,nSuspend);
        return nRe;
    }
    else//��Ҫ��ֵ�ĵ�λ
    {
        int i = 0;
        ULONG nState[MAX_CH_NUM];
        WORD* CHData[MAX_CH_NUM];
        USHORT* nData[MAX_CH_NUM];

        CHData[0] = pCH1Data;
        CHData[1] = pCH2Data;
        CHData[2] = pCH3Data;
        CHData[3] = pCH4Data;
        memset(nState,0,sizeof(nState));
        for(i=0;i<MAX_CH_NUM;i++)
        {
            nData[i] = new USHORT[pControl->nReadDataLen - 10240 + 40960];
        }
        //����
        nRe = dsoHTGetData_Suspend(nDeviceIndex,nData[0],nData[1],nData[2],nData[3],pControl,nSuspend);

        //		for(i=0;i<MAX_CH_NUM;i++)
        //		{
        //			delete nData[i];
        //			nData[i] = NULL;
        //		}
        //		return nRe;
        //��ֵ
        //Ӧ4250����Ҫ�����в�ֵ�������������Ҵ���
        if(pControl->nALT == 1)
        {
            for(i=0;i<MAX_CH_NUM;i++)
            {
                if(pControl->nTriggerSource == i)
                {
                    if(((pControl->nCHSet >> i) & 0x01) == 1)
                    {
                        dsoSFInsert(nData[i],CHData[i],pControl,nInsertMode,i,nState);
                    }
                }
            }
        }
        else
        {
            if(pControl->nTriggerSource < MAX_CH_NUM)
            {
#ifdef FILE_TEST
                {
                    CStdioFile file;
                    CString str;
                    if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
                    {
                        str=_T("Cann't save Test file");
                        //					AfxMessageBox(str);
                        return 0;
                    }

                    file.SeekToEnd();

                    {
                        SYSTEMTIME st;
                        GetLocalTime(&st);
                        str.Format(_T("dsoSFInsert_Trig_In::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
                    }

                    file.WriteString(str);

                    file.Close();
                    Sleep(5);
                }
#endif

                dsoSFInsert(nData[pControl->nTriggerSource],CHData[pControl->nTriggerSource],pControl,nInsertMode,pControl->nTriggerSource,nState);

#ifdef FILE_TEST
                {
                    CStdioFile file;
                    CString str;
                    if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
                    {
                        str=_T("Cann't save Test file");
                        //					AfxMessageBox(str);
                        return 0;
                    }

                    file.SeekToEnd();

                    {
                        SYSTEMTIME st;
                        GetLocalTime(&st);
                        str.Format(_T("dsoSFInsert_Trig_Out::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
                    }

                    file.WriteString(str);

                    file.Close();
                    Sleep(5);
                }
#endif

                for(i=0;i<MAX_CH_NUM;i++)
                {
                    if(pControl->nTriggerSource == i)
                    {
                        continue;
                    }
                    if(((pControl->nCHSet >> i) & 0x01) == 1)
                    {
#ifdef FILE_TEST
                        {
                            CStdioFile file;
                            CString str;
                            if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
                            {
                                str=_T("Cann't save Test file");
                                //					AfxMessageBox(str);
                                return 0;
                            }

                            file.SeekToEnd();

                            {
                                SYSTEMTIME st;
                                GetLocalTime(&st);
                                str.Format(_T("dsoSFInsert_Soft_In::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
                            }

                            file.WriteString(str);

                            file.Close();
                            Sleep(5);
                        }
#endif

                        dsoSFInsert(nData[i],CHData[i],pControl,nInsertMode,i,nState);

#ifdef FILE_TEST
                        {
                            CStdioFile file;
                            CString str;
                            if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
                            {
                                str=_T("Cann't save Test file");
                                //					AfxMessageBox(str);
                                return 0;
                            }

                            file.SeekToEnd();

                            {
                                SYSTEMTIME st;
                                GetLocalTime(&st);
                                str.Format(_T("dsoSFInsert_Soft_Out::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
                            }

                            file.WriteString(str);

                            file.Close();
                            Sleep(5);
                        }
#endif
                    }
                }
            }
            else//EXT
            {
                for(i=0;i<MAX_CH_NUM;i++)
                {
                    if(((pControl->nCHSet >> i) & 0x01) == 1)
                    {
                        dsoSFInsert(nData[i],CHData[i],pControl,nInsertMode,i,nState);
                    }
                }
            }
        }
        //
        for(i=0;i<MAX_CH_NUM;i++)
        {
            delete nData[i];
        }
    }
    return nRe;
}


//���º���SDKҲ������
DLL_API WORD WINAPI dsoSDGetData_Suspend(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl,WORD nInsertMode,USHORT* nSuspend)
{
    CONTROLDATA Control;

    Control.nCHSet = pControl->nCHSet;

    Control.nTimeDIV = pControl->nTimeDIV;

    Control.nTriggerSource = pControl->nTriggerSource;

    Control.nHTriggerPos = pControl->nHTriggerPos;

    Control.nVTriggerPos = pControl->nVTriggerPos;

    Control.nTriggerSlope = pControl->nTriggerSlope;

    Control.nBufferLen = pControl->nBufferLen;

    Control.nReadDataLen = pControl->nReadDataLen;

    Control.nAlreadyReadLen = pControl->nAlreadyReadLen;

    Control.nALT = pControl->nALT;

    Control.nETSOpen = pControl->nETSOpen;

    return SDGetData_Suspend(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,&Control,nInsertMode,nSuspend);
}




//=================DDS-----------------------

/*
2. ������ƽ[0~255]
3. ���������ȡ�0~255���������滻Ϊ��������Ϸ�ʽ�����ݲ�ͬ��Ϸ�ʽ����ͬ����������
4. ʾ��������ͨ����
5. ʾ������ʱ��
6. ���ش�������
*/
// new functions add by zhang to cope with DSO6104   20150916
WORD        setEdgeTrigger(WORD DeviceIndex,PCONTROLDATA pControlData,WORD nTriggerCouple)//set Trigger of Edge
{
    short nChannelMode;//ͨ��ģʽ
    short nSampleRate_Control;
    short nTrigerSourceControl;
    short nTrigerSlopeControl;
    short nDisableTriggerControl;
    short nDisableFandControl;
    short nSelControl;

    //����ͨ��ģʽ 1 or 2 or 4
    short nOPench=0;
    for(int i=0;i<MAX_CH_NUM;i++)
    {
        if((pControlData->nCHSet>>i)&0x01)
            nOPench++;
    }
    if(nOPench>2)
        nChannelMode=4;
    else if(nOPench==2)
        nChannelMode=pControlData->nTimeDIV<6?2:4;
    else if(nOPench==1)
        nChannelMode=pControlData->nTimeDIV<5?1:(pControlData->nTimeDIV==5?2:4);
    else
        nChannelMode=0;
    //����ͨ��ģʽ���
    //nSampleRate_Control start
    if(pControlData->nTimeDIV<6)
        nSampleRate_Control=nChannelMode==4?0x01:(nChannelMode==2?0x10:(nChannelMode==1?0x11:0x00));
    else if(pControlData->nTimeDIV==6)
        nSampleRate_Control=0x01;  //Error?
    else
        nSampleRate_Control=0x00;
    //nSampleRate_Control over
    if(pControlData->nTriggerSource>=0&&pControlData->nTriggerSource<4)
        nTrigerSourceControl=pControlData->nTriggerSource;
    else
        nTrigerSourceControl=0xFF;
    if(pControlData->nTriggerSlope>=0&&pControlData->nTriggerSlope<4)
        nTrigerSlopeControl=pControlData->nTriggerSlope;
    else
        nTrigerSlopeControl=0xFF;

    //���δ���
    if( ((pControlData->nCHSet)>>(pControlData->nTriggerSource))&0x01)
        nDisableTriggerControl=0x00;
    else
        nDisableTriggerControl=0x01;
    //Ƶ�ʼƹر�ʹ��1 for disable 0 for enable
    if( ((pControlData->nCHSet)>>(pControlData->nTriggerSource))&0x01)
        nDisableFandControl=0x00;
    else
        nDisableFandControl=0x01;
    nSelControl=pControlData->nTriggerSource;
    long nCOntrol=((0x03&nSampleRate_Control)<<0)|   //[1:0]���ִ���ͨ�������ʵ�ѡ��
            ((0x03&nTrigerSourceControl)<<2)|   //[3:2]ʾ��������Դ��ѡ��
            ((0x03&nTrigerSlopeControl)<<4)|    //[5:4]  �������ص�ѡ��
            ((0x01&nDisableTriggerControl)<<6)| //[6]  ��������λ
            ((0x01&nDisableFandControl)<<7)|    //[7]  Ƶ�ʼƼ���ʹ��
            ((0x03&nSelControl)<<10)|           //[11:10]��Ƶ Ƶ�ʼ�Դ��ѡ��
            ((0x03&nSelControl)<<12);           //[13:12]��Ƶ Ƶ�ʼ�Դ��ѡ��



    // PUCHAR sss=(CHAR)malloc(20);
    //sss[0]=


    char pcDriverName[MAX_DRIVER_NAME] = "";
    BULK_TRANSFER_CONTROL   outBulkControl;
    HANDLE hOutDevice=NULL;
    PUCHAR outBuffer = NULL;
    UINT m_nSize;
    int status;
    ULONG BytesReturned=0;

    //��ʼ���豸��
    SelectDeviceIndex(DeviceIndex,pcDriverName);
    m_nSize=8;
    ResetDevice(DeviceIndex,1);
    hOutDevice = OpenDevice(pcDriverName);
    if(hOutDevice == NULL)
    {
        return 0;
    }
    outBuffer=(PUCHAR)malloc(m_nSize);
    outBuffer[0]=0x13;
    outBuffer[1]=0x00;
    //�������ݶ�ȡ��ʼ��ַ
    outBuffer[2]=0xff;
    outBuffer[3]=0xff;
    outBuffer[4]=0xff;
    outBuffer[5]=0xff;
    outBuffer[6]=0x00;//(0xff&(unsigned char)(RamReadStartAddr>>32));//SdramRdBegin
    outBuffer[7]=0x80;
    outBulkControl.pipeNum=0;//�˵�ѡ��EP2
#ifdef _ASY
    status = TransferDataToDevice(outBuffer,m_nSize);
#else
    status = DeviceIoControl(hOutDevice,
                             IOCTL_EZUSB_BULK_WRITE,
                             (PVOID)&outBulkControl,
                             sizeof(BULK_TRANSFER_CONTROL),
                             outBuffer,//���������
                             m_nSize,//�ֽ������ڶԻ����п�������
                             &BytesReturned,//�����ֽ�����
                             //����Ϊ�˲����ٶȣ�û�в��Է����ֽ���
                             NULL);
#endif

    //�ر��豸
    free(outBuffer);
    CloseDevice(hOutDevice);
    return status;



}
DLL_API WORD WINAPI dsoHTADCCHModGain(WORD DeviceIndex,WORD nCHMod)
{
/*
	111111 0x3F	9.70%
	111110 0x3E	9.40%
	100001 0x21 0.30%
	100000 0x20 0%
	11111  0x1F -0.3%
	1		-9.7%
	0		-10%
	*/
	BOOL status = FALSE;
    UINT m_nSize;
    m_nSize = 8;
    PUCHAR outBuffer=(PUCHAR)malloc(m_nSize*sizeof(UCHAR));    
    outBuffer[0]=0x08;
    outBuffer[1]=0x00;

    outBuffer[2]=0x00;
    //outBuffer[3]=0xFF&(nCHMod==4?0x3F:nCHMod==2?42:5);//0x3F:0x01);
	outBuffer[3]=0xFF&(nCHMod==4?0x3F:nCHMod==2?0x30:0x19);

    outBuffer[4]=0x00;

    outBuffer[5]=0x55;    //
    outBuffer[6]=0x04;    //ADC
    outBuffer[7]=0x00;
    status=sendOutBuffer(DeviceIndex,m_nSize,outBuffer);


    free(outBuffer);
    return status;
}

WORD        setRamAndTrigerControl(WORD DeviceIndex,WORD nTimeDiv,WORD nCHset,WORD nTrigerSource,WORD nPeak)
{
    PUCHAR outBuffer = NULL;
    UINT m_nSize=6;
    int nOpenCH=0;
    int nRamControl=0;
    int nTrigerControl=0;
    int nDisableTri=0;
    int nLogicTriggerSource=0;
    for(int i=0;i<MAX_CH_NUM;i++)
    {
        if((nCHset>>i)&0x01)
            nOpenCH++;
    }
    switch (nOpenCH) {
    case 4:
    case 3:
        nRamControl=nTimeDiv<=TIMEDIV_250MSA?0x00:0x01;//7 1uS
        nTrigerControl=0x01-nRamControl;
        break;
    case 2:
        nRamControl=nTimeDiv<=TIMEDIV_500MSA?0x00:(nTimeDiv==TIMEDIV_250MSA?0x01:0x02);
        nTrigerControl=0x02-nRamControl;
        break;
    case 1:
        nRamControl=nTimeDiv<=TIMEDIV_1GSA?0x00:(nTimeDiv==TIMEDIV_500MSA?0x01:(nTimeDiv==TIMEDIV_250MSA?0x02:0x03));
        nTrigerControl=0x03-nRamControl;
        break;
    default:
        break;
    }
    nDisableTri=((nCHset>>nTrigerSource)&0x01)?0x00:0x01;

    switch (HDGetCHMode(nCHset,nTimeDiv)) {
    case 4:
    case 3:
        nLogicTriggerSource=nTrigerSource;
        break;
    case 2:
        if(nOpenCH==1)
            nLogicTriggerSource=0x00;
        else
            nLogicTriggerSource=(nCHset>>nTrigerSource)==0x01?0x01:0x00;
        break;
    case 1:
        nLogicTriggerSource=0x00;
        break;
    default:
        break;
    } 
	WORD FpgaWrRamControl = 0x3F & (
                ((nCHset & 0x0F) << 2) |
                ((nRamControl & 0x03) << 0)
                );
    WORD FpgaWrTrigControl = 0x3F & (
                ((nTrigerControl & 0x03) << 0)
                );
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x12;
    outBuffer[1]=0x00;
    outBuffer[2]=0xFF&(FpgaWrRamControl|((nPeak&0x01)<<6));
	//outBuffer[2]=0xFF&(FpgaWrRamControl);
    outBuffer[3]=0xFF&(FpgaWrRamControl>>8);
    outBuffer[4]=0xFF&(FpgaWrTrigControl);
    outBuffer[5]=0xFF&((nDisableTri<<2)|nLogicTriggerSource);
    return sendOutBuffer(DeviceIndex,m_nSize,outBuffer);
}
DLL_API WORD WINAPI dsoHTSetRamAndTrigerControl(WORD DeviceIndex,WORD nTimeDiv,WORD nCHset,WORD nTrigerSource,WORD nPeak)
{
    return setRamAndTrigerControl(DeviceIndex, nTimeDiv,nCHset,nTrigerSource,nPeak);
}
DLL_API WORD WINAPI dsoHTSetTrigerMode(WORD m_nDeviceIndex,WORD nTriggerMode,WORD nTriggerSlop,WORD nTriggerCouple)
{
    WORD m_nSize=6;
    PUCHAR outBuffer=NULL;
    BOOL status=FALSE;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x11;
    outBuffer[1]=0x00;
    outBuffer[2]=0xFF&(unsigned short)(nTriggerMode);
    outBuffer[3]=0xFF&(unsigned short)(nTriggerSlop);
    outBuffer[4]=0xFF&(unsigned short)(nTriggerCouple);//zhang
    outBuffer[5]=0x00;
    status=sendOutBuffer(m_nDeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
    return status;
}
DLL_API WORD WINAPI dsoHTSetVideoTriger(WORD m_nDeviceIndex,USHORT nStand,USHORT nVedioSyncSelect,USHORT nVideoHsyncNumOption,USHORT nVideoPositive,WORD nLevel,WORD nLogicTriggerSource)
{
    WORD m_nSize=16;
    UINT t1=29400/8,t2=27100/8,t3=4700/8,t4=2400/8;
    PUCHAR outBuffer=NULL;
    BOOL status=FALSE;
    nLevel=WORD((nLevel)*200/256.0+28.5);
    if(nStand==PALSECAM)
    {
        if(nVideoHsyncNumOption<=5)
            nVideoHsyncNumOption=(nVideoHsyncNumOption-1)*2+310;
        else if(nVideoHsyncNumOption<=310)
            nVideoHsyncNumOption=nVideoHsyncNumOption+318;
        else if(nVideoHsyncNumOption<=318)
            nVideoHsyncNumOption=(nVideoHsyncNumOption-311)*2+329;
        else if(nVideoHsyncNumOption<=622)
            nVideoHsyncNumOption=nVideoHsyncNumOption-319;
        else if(nVideoHsyncNumOption<=625)
            nVideoHsyncNumOption=(nVideoHsyncNumOption-623)*2+304;
    }
    else
    {
        if(nVideoHsyncNumOption<=9)
            nVideoHsyncNumOption=(nVideoHsyncNumOption-1)*2+254;
        else if(nVideoHsyncNumOption<=263)
            nVideoHsyncNumOption=nVideoHsyncNumOption+262;
        else if(nVideoHsyncNumOption<=271)
            nVideoHsyncNumOption=(nVideoHsyncNumOption-263)*2+525;
        else if(nVideoHsyncNumOption<=525)
            nVideoHsyncNumOption=nVideoHsyncNumOption-272;
    } 
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x16;
    outBuffer[1]=0x00;
    outBuffer[2]=0xFF&(unsigned short)(nVideoHsyncNumOption);
    outBuffer[3]=0xFF&(unsigned short)(((nVideoHsyncNumOption>>8)&0x03)|
                                       ((nVedioSyncSelect<<3)&0x38)|
                                       (((nVideoPositive?0:1)<<6)&0x40));
    outBuffer[4]=0xFF&(unsigned short)(nLevel+2);//zhang
    outBuffer[5]=0xFF&(unsigned short)(nLevel-2);//zha
    outBuffer[6]=0xFF&(unsigned short)(t1);//zhang
    outBuffer[7]=0xFF&(unsigned short)(t1>>8);//zha
    outBuffer[8]=0xFF&(unsigned short)(t2);//zhang
    outBuffer[9]=0xFF&(unsigned short)(t2>>8);//zha
    outBuffer[10]=0xFF&(unsigned short)(t3);//zhang
    outBuffer[11]=0xFF&(unsigned short)(t3>>8);//zha
    outBuffer[12]=0xFF&(unsigned short)(t4);//zhang
    outBuffer[13]=0xFF&(unsigned short)(t4>>8);//zha
    outBuffer[14]=0xFF&(unsigned short)(nLogicTriggerSource&0x03);//zhang
    outBuffer[15]=0xFF&(unsigned short)(50);//zha
    status=sendOutBuffer(m_nDeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
    return status;
}
DLL_API WORD WINAPI dsoHTSetPulseTriger(WORD m_nDeviceIndex,ULONG nPW,WORD nPWCondition)
{
    WORD m_nSize=16;
    PUCHAR outBuffer=NULL;
    ULONG nUpperPW=ULONG(nPW*1.05);
    ULONG nDownPW=ULONG(nPW*0.95);
	BOOL status;
    outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x13;
    outBuffer[1]=0x00;
    outBuffer[2]=0xFF&(unsigned short)(nPW);
    outBuffer[3]=0xFF&(unsigned short)(nPW>>8);
    outBuffer[4]=0xFF&(unsigned short)(nPW>>16);
    outBuffer[5]=0xFF&(unsigned short)(nPW>>24);
    outBuffer[6]=0xFF&(unsigned short)(nUpperPW);//zhang
    outBuffer[7]=0xFF&(unsigned short)(nUpperPW>>8);//zha
    outBuffer[8]=0xFF&(unsigned short)(nUpperPW>>16);//zhang
    outBuffer[9]=0xFF&(unsigned short)(nUpperPW>>24);//zha
    outBuffer[10]=0xFF&(unsigned short)(nDownPW);//zhang
    outBuffer[11]=0xFF&(unsigned short)(nDownPW>>8);//zha
    outBuffer[12]=0xFF&(unsigned short)(nDownPW>>16);//zhang
    outBuffer[13]=0xFF&(unsigned short)(nDownPW>>24);//zha
    outBuffer[14]=0xFF&(unsigned short)(nPWCondition);//zha
	outBuffer[15]=0x00;//zha
    status=sendOutBuffer(m_nDeviceIndex,m_nSize,outBuffer);
    free(outBuffer);
    return status;
}