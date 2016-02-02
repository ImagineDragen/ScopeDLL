// HTNTLVDll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
// DSO3X25Dll.cpp : Defines the entry point for the DLL application.
//
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include "objbase.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <commctrl.h>
#include <tchar.h>
#include <setupapi.h>
#include <initguid.h>
#include <winioctl.h>
#include <process.h>
#include <assert.h>
#include <dbt.h>

#pragma warning(disable:4786)
#define DLL_API extern "C" _declspec(dllexport)
#include "HTNTLVDll.h"
#include "intrface.h"
#include "math.h"
#include <vector>
using namespace std;
#pragma comment(lib, "setupapi.lib")
#pragma pack (1)//让编译器按照1字节对齐


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

//////////////////////////////////////////////////
//////////////////////////////////定义宏
#define DSO_OK			0	//成功
#define DSO_ERROR_WAIT_OUTTIME	1	//超时错误
#define DSO_ERROR_IO			2	//IO错误
#define DSO_ERROR_WRITE			3	//写入错误
#define DSO_ERROR_READ			4	//读出错误
#define DSO_ERROR_OPEN			5	//打开设备出错
#define DSO_ERROR_OTHER			10	//其它错误
//写数据, 返回0表示成功
int dsoWrite(DSO_HANDLE hDSO, unsigned char *InBuffer,DWORD Len);//wnBytes为实际写入的字节数
int dsoRead(DSO_HANDLE hDSO, unsigned char *OutBuffer,DWORD Len);//rnBytes为实际读出的字节数
BOOL dsoOpen(HANDLE* phDSO, int index);
BOOL dsoResetSignal(int index, HANDLE hDSO);	//清除并产生信号

/////////////////////////////////////////////////////////////////
//#define MAX_STRING_LENGTH       500

typedef VOID (*STM32MS_IO_CALLBACK)(PVOID Context);
// Structure used to store I/O information for delayed
// processing of overlapped requests
typedef struct _STM32MS_LIST_ITEM
{
    struct _STM32MS_LIST_ITEM*  Next;
    struct _STM32MS_LIST_ITEM*  Previous;
    OVERLAPPED                  IoOverlapped;
    PCHAR                       InBuffer;
    PCHAR                       OutBuffer;
    ULONG                       InSize;
    ULONG                       OutSize;
    ULONG                       ReturnLength;
    ULONG                       Error;
    STM32MS_IO_CALLBACK         Callback;
} STM32MS_LIST_ITEM, *PSTM32MS_LIST_ITEM;
STM32MS_LIST_ITEM ioItem;

//vector<STM32MS_LIST_ITEM> vecIoItem;
vector<string> vecDevPath;	//驱动路径
vector<BYTE> vecDIO;		//数字输入口


/////////////////////////////////////////////////////
////////////////////////////////变量
#define MAX_FREQ	200000000	//最大刷新时钟
#define MAX_BUF		4096		//最大存储区
#define MAX_NUM		64			//每次发送的数据包的大小
#define MEASURE_FREQ	0x01	//测量频率
#define RESET_COUNTER	0x02	//复位计数器
#define SINGLE_WAVE		0x04	//单次波形
#define EXT_FALL		0x08	//外部下降沿, 0为上升沿
#define EXT_TRIG		0x10	//外部触发
#define CLEAR_SIGNAL	0x20	//触发清除和产生信号（同3005的CONreg(4)）。
//在设置成外触发之前，置1一定时间(100ms)然后清0,再通过CONREG(4)置1启动外触发。
//在设置成内触发后，需要利用将该位置1然后清0来产生一次内触发信号。
//其它位写0
#define DIO_MODE		0x100	//(CONREG(8), 置0为字发生器, 1为数字IO


typedef struct _SENDSESSION
{
	unsigned char CMD;    //命令字节
	unsigned short CONREG;//控制寄存器或者作为EndAddr
	unsigned short EXOUT; //数字输出
	unsigned short DivNum;//分频数
}SENDSESSION;

typedef struct _RECVSESSION
{
    unsigned short EXIN;  //数字输入
	unsigned int FRQ;     //频率或计数器数值
	unsigned int PED;     //周期测量数值
}RECVSESSION;

//SENDSESSION sendSession;
//RECVSESSION recvSession;

vector<SENDSESSION> sendSession;
vector<RECVSESSION> recvSession;

typedef struct _VEC_PARAM
{
	int divNum;
	int nWaveNum;
	int nPeriod;
}VEC_PARAM;

vector<VEC_PARAM> vecParam;
vector<BYTE> vecVersion;	//从硬件读出的版本号
typedef struct _FLASH_WAVE
{
	USHORT nData;
	USHORT divNum;
	USHORT data[4096];
}FLASH_WAVE;
vector<FLASH_WAVE>	g_flashWave;
/////////////////////////////////////////////////////////

DLL_API int WINAPI dsoSearch()
{
	LONG								nDevNum = 0;
	LONG								i = 0;
	LONG                                ii = 0;
    HDEVINFO                            hDeviceInfo;
    DWORD                               bufferSize;
    SP_DEVICE_INTERFACE_DATA            interfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceDetail;
	SENDSESSION							mSendSession;
	RECVSESSION							mRecvSession;
    // Get the handle to the device instance list box
    
	vecDevPath.clear();
	g_flashWave.clear();
	vecVersion.clear();
	vecParam.clear();
	vecDIO.clear();

	string sPath;
    // Find all devices that have our interface
    hDeviceInfo = SetupDiGetClassDevs(
                    (LPGUID)&GUID_DEVINTERFACE_STM32MS,
                    NULL,
                    NULL,
                    DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
                    );
    if (hDeviceInfo == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

	interfaceData.cbSize = sizeof(interfaceData);

    for (ii = 0;
         SetupDiEnumDeviceInterfaces(	//枚举所有的device.
            hDeviceInfo,
            NULL,
            (LPGUID)&GUID_DEVINTERFACE_STM32MS,
            ii,
            &interfaceData);
         ++ii)
    {
        // Found our device instance
        if (!SetupDiGetDeviceInterfaceDetail(
                hDeviceInfo,
                &interfaceData,
                NULL,
                0,
                &bufferSize,
                NULL))
        {
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            {
                continue;
			}
        }

        // Allocate a big enough buffer to get detail data
        deviceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(bufferSize);
        if (deviceDetail == NULL)
        {
            continue;
        }

        // Setup the device interface struct
        deviceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        // Try again to get the device interface detail info
        if (!SetupDiGetDeviceInterfaceDetail(
                hDeviceInfo,
                &interfaceData,
                deviceDetail,
                bufferSize,
                NULL,
                NULL))
        {
            free(deviceDetail);
            continue;
        }

        // Add device instance info to list
		sPath = deviceDetail->DevicePath;

		if(sPath.find(_T("#vid_0783&pid_5725#"),0) == -1)//add by yt 20111121 -- 筛选8ch,与3X25分开
		{
			free(deviceDetail);
            continue;
		}
	//	sPath += "\\PIPE01";
		nDevNum++;
		vecDevPath.push_back(sPath);
		mSendSession.CMD = 0xa0;
		mSendSession.CONREG = 0x00;
		mSendSession.DivNum = 0;
		mSendSession.EXOUT = 0;

		mRecvSession.EXIN = 0;
		mRecvSession.FRQ = 0;
		mRecvSession.PED = 0;
		sendSession.push_back(mSendSession);
		recvSession.push_back(mRecvSession);
		VEC_PARAM param;
		param.divNum = 0;
		param.nPeriod = 1;
		param.nWaveNum = 4096;
		vecParam.push_back(param);

		vecVersion.push_back(0x01);
		FLASH_WAVE wave;
		wave.divNum = 0;
		wave.nData = 4096;
		for (i = 0; i < 4096; i++)
			wave.data[i] = 2048;
		g_flashWave.push_back(wave);

        free(deviceDetail);
    }
    SetupDiDestroyDeviceInfoList(hDeviceInfo);
	

    return nDevNum;		
}

BOOL dsoOpen(DSO_HANDLE* phDSO, int Index)
{

	if (vecDevPath.size() == 0)
		return  DSO_ERROR_OPEN;
	if (Index < 0 || Index >= vecDevPath.size())
		return	DSO_ERROR_OPEN;
	char devPath[MAX_PATH];
	strcpy(devPath, vecDevPath.at(Index).c_str());  
	
	*phDSO = CreateFile(
		(char*)(devPath),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		0
		);	
	if (*phDSO == INVALID_HANDLE_VALUE)
	{
		return DSO_ERROR_OPEN;
	}
	return DSO_OK;
}

int dsoWrite(DSO_HANDLE hDSO,unsigned char *InBuffer,DWORD Len)
{
	DWORD nBytes;
	memset(&(ioItem.IoOverlapped),0,sizeof(OVERLAPPED));
    ioItem.IoOverlapped.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);//创建一事件
    ResetEvent(ioItem.IoOverlapped.hEvent);//复位事件
	if (!WriteFile( hDSO,
                    InBuffer,
                    Len,
                    &nBytes,
                    &(ioItem.IoOverlapped)))
	{
	    
		if(GetLastError()!=ERROR_IO_PENDING)
		{
			CloseHandle(ioItem.IoOverlapped.hEvent);
             return DSO_ERROR_IO;//MessageBox("WriteError");
		}
	}
	switch(WaitForSingleObject(ioItem.IoOverlapped.hEvent,1000))//等待1000ms
	{
            case WAIT_OBJECT_0:
				  
                  if(!::GetOverlappedResult(hDSO,&ioItem.IoOverlapped,&nBytes,TRUE))
				  {
					  CloseHandle(ioItem.IoOverlapped.hEvent);
					  return DSO_ERROR_WRITE;//MessageBox("Write Error");
				  }
                  break;
            case WAIT_TIMEOUT://超时错误
				CloseHandle(ioItem.IoOverlapped.hEvent);
                 return DSO_ERROR_WAIT_OUTTIME;//MessageBox("Write Over Time");
				 break;
            default:
				CloseHandle(ioItem.IoOverlapped.hEvent);
				 return DSO_ERROR_OTHER;//MessageBox("Write Error");
                 break;
    }
	CloseHandle(ioItem.IoOverlapped.hEvent);
	return DSO_OK;  
}
int dsoRead(DSO_HANDLE hDSO,unsigned char *OutBuffer,DWORD Len)
{
	DWORD nBytes;
	memset(&(ioItem.IoOverlapped),0,sizeof(OVERLAPPED));
    ioItem.IoOverlapped.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);//创建一事件
    ResetEvent(ioItem.IoOverlapped.hEvent);//复位事件
    if(!ReadFile(	hDSO,
                    OutBuffer,
                    Len,
                    NULL,
                    &ioItem.IoOverlapped))
	{
		if(GetLastError()!=ERROR_IO_PENDING)
		{
			CloseHandle(ioItem.IoOverlapped.hEvent);
           return DSO_ERROR_IO;//MessageBox("ReadError");
		}
	}
	switch(WaitForSingleObject(ioItem.IoOverlapped.hEvent,1000))//等待1000ms
	{
            case WAIT_OBJECT_0:
                  if(!::GetOverlappedResult(hDSO,&ioItem.IoOverlapped,&nBytes,TRUE))
				  {
					  CloseHandle(ioItem.IoOverlapped.hEvent);
					  return DSO_ERROR_READ;//MessageBox("Read Error");
				  }
                  break;
            case WAIT_TIMEOUT://超时错误
				CloseHandle(ioItem.IoOverlapped.hEvent);
                 return DSO_ERROR_WAIT_OUTTIME;//MessageBox("Read Over Time");
				 break;
            default:
				CloseHandle(ioItem.IoOverlapped.hEvent);
				 return DSO_ERROR_OTHER;//MessageBox("Read Error");
                 break;
    }
	CloseHandle(ioItem.IoOverlapped.hEvent);
	return DSO_OK;
}
/********************************************************
* 函数名:	DSOSetFrequency
* 功  能:	设置频率
* 参  数:	index:			操作的设备号, 
*			frequency:		要设置的频率,
*			wavePointNum:	一次发送的点数
*			TNum:			一次发送的周期数	
* 返回值:	1表示成功, 返回0表示失败. 
*********************************************************/

DLL_API BOOL WINAPI dsoSetFrequency(int index, double frequency, int* wavePointNum, int* TNum)		//设置频率
{	
	DSO_HANDLE hDSO;
	if (DSO_OK != dsoOpen(&hDSO, index))
		return 0;
	int iNumber = 0;
	double period;
	float flNum;
	int i;
	if (fabs(frequency - 0) < 1)
	{
		frequency = 1;
	}
	period = 1.0  / frequency ;		
	
	if (frequency <= 100000 )	//当频率小于100K还是按以前的方法
	{
		sendSession.at(index).DivNum  = int(MAX_FREQ*period/MAX_BUF);	//计算分频数
		if (sendSession.at(index).DivNum == 0)
			iNumber = period * MAX_FREQ;
		else
			iNumber = period * 100000000 / sendSession.at(index).DivNum;
		*TNum = 1;
	}
	else
	{
		sendSession.at(index).DivNum = 0;
		flNum = MAX_FREQ / frequency;		
		i = 1;
		while(1)
		{
			if (flNum * (i + 1) > MAX_BUF)
				break;
			i++;
		}
		iNumber = (int)(flNum*i);
		*TNum = i;
	}
	if (iNumber > MAX_BUF)
	{		
		iNumber = MAX_BUF;
	}
	*wavePointNum = iNumber;
	vecParam.at(index).nPeriod = *TNum;
	vecParam.at(index).nWaveNum = *wavePointNum;
	sendSession.at(index).CMD = 0xa0;
	vecParam.at(index).divNum = sendSession.at(index).DivNum;
	g_flashWave.at(index).divNum = sendSession.at(index).DivNum;
	////////////
	dsoWrite(hDSO, (unsigned char*)(&(sendSession.at(index))), 7);
	if (dsoRead(hDSO, (unsigned char*)(&(recvSession.at(index))), 10) != 0)
	{
		CloseHandle(hDSO);
		return 0;
	}
	CloseHandle(hDSO);
	return 1;
}
DLL_API int WINAPI dsoGetDivNum(int index)
{
	return vecParam.at(index).divNum;
}
DLL_API BOOL WINAPI dsoSetDivNum(int index, int num)			//设置分频数
{
	DSO_HANDLE hDSO;
	if (DSO_OK != dsoOpen(&hDSO, index))
		return 0;

	sendSession.at(index).CMD = 0xa0;
	sendSession.at(index).DivNum = num;
		
	dsoWrite(hDSO, (unsigned char*)(&(sendSession.at(index))), 7);
	if (dsoRead(hDSO, (unsigned char*)(&(recvSession.at(index))), 10) != 0)
		return 0;
	CloseHandle(hDSO);
	return 1;
}

DLL_API BOOL WINAPI dsoGetMeasure(int index, BOOL bFreq, double* measure)	//bFreq: 0:测量频率, 1:测量计数器
{		
	DSO_HANDLE hDSO;
	if (DSO_OK != dsoOpen(&hDSO, index))
		return 0;
	unsigned char temp = 0;
	if (bFreq == 1)
		sendSession.at(index).CONREG |= MEASURE_FREQ;	
	else
		sendSession.at(index).CONREG &= ~MEASURE_FREQ;//频率计

	sendSession.at(index).CMD = 0xa0;
	dsoWrite(hDSO, (unsigned char*)(&(sendSession.at(index))), 7);
	dsoRead(hDSO, (unsigned char*)(&(recvSession.at(index))), 10);
	
	if (bFreq == 0)	//如果是测频率
	{		
		if (recvSession.at(index).FRQ  == 0)
		{
			*measure = 0;
		}
		else if (recvSession.at(index).FRQ <= 30000)	//如果频率小于5K
		{
		//	*measure = 1.0 / (recvSession.PED * 8) *1000000000;
			*measure = MAX_FREQ * 1.0 / recvSession.at(index).PED;
		}
		else
		{
			*measure = recvSession.at(index).FRQ * 2;
		}
	}
	else
	{
		*measure = recvSession.at(index).FRQ;		
	}
	CloseHandle(hDSO);
	return 1;
}
DLL_API BOOL WINAPI dsoSetSingleWave(int index, BOOL bSingle)				//b: 1:单次, 0:连续
{
	DSO_HANDLE hDSO;
	if (DSO_OK != dsoOpen(&hDSO, index))
		return 0;


	sendSession.at(index).CMD = 0xa0;
	if (bSingle)
		sendSession.at(index).CONREG |= SINGLE_WAVE;	//单次
	else
		sendSession.at(index).CONREG &= ~SINGLE_WAVE;	//连续

	dsoWrite(hDSO, (unsigned char*)(&(sendSession.at(index))), 7);
	if (dsoRead(hDSO, (unsigned char*)(&(recvSession.at(index))), 10) != 0)
		return 0;

	CloseHandle(hDSO);
	return 1;
}
DLL_API BOOL WINAPI dsoResetCounter(int index)						//复位计数器
{
	DSO_HANDLE hDSO;
	if (DSO_OK != dsoOpen(&hDSO, index))
		return 0;
	
	
	sendSession.at(index).CMD = 0xa0;	
	sendSession.at(index).CONREG |= RESET_COUNTER;	//复位	

	dsoWrite(hDSO, (unsigned char*)(&(sendSession.at(index))), 7);
	if (dsoRead(hDSO, (unsigned char*)(&(recvSession.at(index))), 10) != 0)
		return 0;

//	Sleep(100);
	sendSession.at(index).CMD = 0xa0;	
	sendSession.at(index).CONREG &= ~RESET_COUNTER;	//恢复	

	dsoWrite(hDSO, (unsigned char*)(&(sendSession.at(index))), 7);
	if (dsoRead(hDSO, (unsigned char*)(&(recvSession.at(index))), 10) != 0)
	{
		CloseHandle(hDSO);
		return 0;
	}

	CloseHandle(hDSO);
	return 1;
}
//bExt = 1:外触发, 0:内触发. bEdge(当是外触发时有效) 0:上升沿,1:下降沿.
DLL_API BOOL WINAPI dsoSetTrigger(int index, BOOL bExt, BOOL bEdge)	
{
	DSO_HANDLE hDSO;
	if (DSO_OK != dsoOpen(&hDSO, index))
		return 0;
	

	dsoResetSignal(index, hDSO);

	sendSession.at(index).CMD = 0xa0;
	if (bExt)
	{
		sendSession.at(index).CONREG |= EXT_TRIG;	//外部触发
		if (!bEdge)
		{
			sendSession.at(index).CONREG &= ~EXT_FALL;
		}
		else
		{
			sendSession.at(index).CONREG |= EXT_FALL;
		}
	}
	else
		sendSession.at(index).CONREG &= ~EXT_TRIG;	//内触发

	dsoWrite(hDSO, (unsigned char*)(&(sendSession.at(index))), 7);
	if (dsoRead(hDSO, (unsigned char*)(&(recvSession.at(index))), 10) != 0)
	{
		CloseHandle(hDSO);
		return 0;
	}

	CloseHandle(hDSO);
	return 1;
}
DLL_API BOOL WINAPI dsoGetDigitalIn(int index, unsigned short* In)		//读取数字输入IO口			
{
	DSO_HANDLE hDSO;
	if (DSO_OK != dsoOpen(&hDSO, index))
		return 0;
	
	sendSession.at(index).CMD = 0xa0;
	dsoWrite(hDSO, (unsigned char*)(&(sendSession.at(index))), 7);
	dsoRead(hDSO, (unsigned char*)(&(recvSession.at(index))), 10);

	*In = recvSession.at(index).EXIN;

	CloseHandle(hDSO);
	return 1;
}
DLL_API BOOL WINAPI dsoSetDigitalOut(int index, unsigned short Out)	//发送数字输出IO口
{
	DSO_HANDLE hDSO;
	if (DSO_OK != dsoOpen(&hDSO, index))
		return 0;

	sendSession.at(index).CMD = 0xa0;
	sendSession.at(index).EXOUT = Out;
	dsoWrite(hDSO, (unsigned char*)(&(sendSession.at(index))), 7);
	dsoRead(hDSO, (unsigned char*)(&(recvSession.at(index))), 10);

	CloseHandle(hDSO);
	return 1;
}
DLL_API BOOL WINAPI dsoDownload(int index, unsigned short* buffer, int num)//下载波形数据到DSO中
{
	DSO_HANDLE hDSO;
	if (DSO_OK != dsoOpen(&hDSO, index))
		return 0;
	
	unsigned short reg = sendSession.at(index).CONREG;	//保存控制寄存器映像
	unsigned short buf[MAX_BUF];
	int number, i, tt, nPer, nBan;
	unsigned char temp;
	

	if (num % MAX_NUM != 0)
	{
		number = (num / MAX_NUM + 1) * MAX_NUM;
	}
	else
	{
		number = num;
	}
	nPer = vecParam.at(index).nWaveNum / vecParam.at(index).nPeriod;
	nBan = int(nPer * 1.0 / 2 + 0.5);
	for (i = 0; i < number; i++)
	{
		if (i < num)
		{
			buf[i] = buffer[i];
		}
		else
			buf[i] = buffer[num - 1];
		tt = i % nPer;
		buf[i] &= 0x0FFF;
		if (tt < nBan)
			buf[i] |= 0x1000;
		if (i < num - 1)
			buf[i] |= 0x2000;
		g_flashWave.at(index).data[i] = buf[i];

	}
//	return 1;

	g_flashWave.at(index).nData = number;


	sendSession.at(index).CMD=0xa1;
	sendSession.at(index).CONREG = num  - 1;

	sendSession.at(index).CONREG |= 0x8000;
    dsoWrite(hDSO, (unsigned char *)(&(sendSession.at(index))), 3);
	i=dsoRead(hDSO, &temp, 1);
	
    for(i=0;i<(number*2/MAX_NUM);i++)
    {
       dsoWrite(hDSO, ((unsigned char *)buf)+i*MAX_NUM, MAX_NUM);
	   if (0 != dsoRead(hDSO, &temp, 1))
	   {
		   sendSession.at(index).CONREG = reg;
		   CloseHandle(hDSO);
		   return 0;
	   }
	}
	sendSession.at(index).CMD=0xa1;
	sendSession.at(index).CONREG = num  - 1;

	sendSession.at(index).CONREG &= ~0x8000;

    dsoWrite(hDSO, (unsigned char *)(&(sendSession.at(index))), 3);
	sendSession.at(index).CONREG = reg;
	CloseHandle(hDSO);
	return 1;
}
BOOL dsoResetSignal(int index, HANDLE hDSO)	//清除并产生信号
{
	sendSession.at(index).CMD = 0xa0;
	sendSession.at(index).CONREG |= CLEAR_SIGNAL;	//首先置1清除
	dsoWrite(hDSO, (unsigned char *)(&(sendSession.at(index))), 7);
	if (dsoRead(hDSO, (unsigned char*)(&(recvSession.at(index))), 10) != 0)
	{
		CloseHandle(hDSO);
		return 0;
	}

	Sleep(100);

	sendSession.at(index).CONREG &= ~CLEAR_SIGNAL; //置0产生信号
	dsoWrite(hDSO, (unsigned char *)(&(sendSession.at(index))), 7);
	if (dsoRead(hDSO, (unsigned char*)(&(recvSession.at(index))), 10) != 0)
	{
		CloseHandle(hDSO);
		return 0;
	}
	return 1;
}
DLL_API BOOL WINAPI dsoSetDIOMode(int index, BOOL mode)
{
	DSO_HANDLE hDSO;
	if (DSO_OK != dsoOpen(&hDSO, index))
		return 0;

	sendSession.at(index).CMD = 0xa0;
	if (mode)	//数字IO
		sendSession.at(index).CONREG |= DIO_MODE;
	else
		sendSession.at(index).CONREG &= ~DIO_MODE;
	dsoWrite(hDSO, (unsigned char*)(&(sendSession.at(index))), 7);
	dsoRead(hDSO, (unsigned char*)(&(recvSession.at(index))), 10);

	CloseHandle(hDSO);
	return 1;
}
DLL_API BOOL WINAPI dsoCheck(int index)
{
	DSO_HANDLE hDSO;
	if (DSO_OK != dsoOpen(&hDSO, index))
		return 0;
	CloseHandle(hDSO);
	return 1;
}
DLL_API BOOL WINAPI dsoSetPowerOnOutput(int index, BOOL bOpen)	//是否上电输出波形
{
	unsigned char temp, version = 0xFF;
	DSO_HANDLE hDSO;
	USHORT nData = 0;
	int i;
	if (DSO_OK != dsoOpen(&hDSO, index))
		return 0;
	sendSession.at(index).CMD = 0xa3;
	dsoWrite(hDSO, (unsigned char *)(&(sendSession.at(index))),1);
	dsoRead(hDSO, &version,1);
	if (bOpen == 1)
	{
		sendSession.at(index).CMD = 0xa2;
		dsoWrite(hDSO, (unsigned char*)(&(sendSession.at(index))), 7);
		dsoRead(hDSO, &temp,1);
		if (version != 0xFF)
		{
	//		nData = g_flashWave.at(index).nData + 1;
			for(i=0;i<(g_flashWave.at(index).nData*2/MAX_NUM);i++)
			{
			   dsoWrite(hDSO, ((unsigned char *)g_flashWave.at(index).data)+i*MAX_NUM, MAX_NUM);
			   if (0 != dsoRead(hDSO, &temp, 1))
			   {
				   CloseHandle(hDSO);
				   return 0;
			   }
			}
		}
	}
	else
	{
		sendSession.at(index).CMD=0xa1;
		sendSession.at(index).CONREG=0xffff;//将末地址设为FFFFH就关闭了输出功能
		dsoWrite(hDSO, (unsigned char *)(&(sendSession.at(index))),3);	
		dsoRead(hDSO, &temp,1);
		sendSession.at(index).CMD=0xa2;     //保存到FLASH
		dsoWrite(hDSO, (unsigned char *)(&(sendSession.at(index))),1);	
		dsoRead(hDSO, &temp,1);
		sendSession.at(index).CMD=0xa1;
		sendSession.at(index).CONREG=vecParam.at(index).nWaveNum;//将末地址恢复为有效的数值
		dsoWrite(hDSO,(unsigned char *)(&(sendSession.at(index))),3);
		dsoRead(hDSO, &temp,1);
	}
	
	
	CloseHandle(hDSO);
	return 1;
}

/*****************************************************************************************/

USHORT GetDataNum(USHORT nTimeDIV,USHORT nCHNum)
{
	int i = 0;
	USHORT nDataNum[MAX_TIMEDIV_NUM];

	if(nTimeDIV > MAX_TIMEDIV_NUM)
	{
		return 0;
	}
	if(nCHNum == 2)
	{
		if(nTimeDIV < 7)
		{
			nTimeDIV = 7;
		}
	}
	else if(nCHNum == 3 || nCHNum == 4 || nCHNum == 5 || nCHNum == 6)
	{
		if(nTimeDIV < 8)
		{
			nTimeDIV = 8;
		}
	}
	else if(nCHNum == 7 || nCHNum == 8)
	{
		if(nTimeDIV < 9)
		{
			nTimeDIV = 9;
		}
	}

/*
	nDataNum[i++] = 2;//1.2;//0--1ns---2.4M
	nDataNum[i++] = 2;//1.2;//1--2ns---2.4M
	nDataNum[i++] = 2;//1.2;//2--5ns---2.4M
	nDataNum[i++] = 2;//1.2;//3--10ns---2.4M
	nDataNum[i++] = 2;//1.2;//4--20ns---2.4M
	nDataNum[i++] = 2;//1.2;//5--50ns---2.4M
	nDataNum[i++] = 2;//6-- 100ns---2.4M---->1CH 当打开1CH个时，这是最小时基
	nDataNum[i++] = 5;//7-- 200ns---2.4M---->2CH 当打开2CH个时，这是最小时基
	nDataNum[i++] = 12;//8--500ns-- ---2.4M--->3,4,5,6CH 当打开3,4,5，6个CH时，这是最小时基

	nDataNum[i++] = 24;//9--1us ---2.4M--->7,8CH 当打开7或8个CH时，这是最小时基

	nDataNum[i++] = 48;//10--2us ---2.4M
	nDataNum[i++] = 120;//11--5us ---2.4M
	nDataNum[i++] = 240;//12--10us---2.4M
	nDataNum[i++] = 480;//13--20us---2.4M
	nDataNum[i++] = 1200;//14--50us---2.4M
	nDataNum[i++] = 2400;//15--100us---2.4M
*/
	nDataNum[i++] = 4000;//1.2;//0--1ns---2.4M
	nDataNum[i++] = 4000;//1.2;//1--2ns---2.4M
	nDataNum[i++] = 4000;//1.2;//2--5ns---2.4M
	nDataNum[i++] = 4000;//1.2;//3--10ns---2.4M
	nDataNum[i++] = 4000;//1.2;//4--20ns---2.4M
	nDataNum[i++] = 4000;//1.2;//5--50ns---2.4M
	nDataNum[i++] = 4000;//6-- 100ns---2.4M---->1CH 当打开1CH个时，这是最小时基
	nDataNum[i++] = 4000;//7-- 200ns---2.4M---->2CH 当打开2CH个时，这是最小时基
	nDataNum[i++] = 4000;//8--500ns-- ---2.4M--->3,4,5,6CH 当打开3,4,5，6个CH时，这是最小时基

	nDataNum[i++] = 4000;//9--1us ---2.4M--->7,8CH 当打开7或8个CH时，这是最小时基

	nDataNum[i++] = 4000;//10--2us ---2.4M
	nDataNum[i++] = 4000;//11--5us ---2.4M
	nDataNum[i++] = 4000;//12--10us---2.4M
	nDataNum[i++] = 4000;//13--20us---2.4M
	nDataNum[i++] = 4000;//14--50us---2.4M
	nDataNum[i++] = 4000;//15--100us---2.4M
/////////////
	nDataNum[i++] = 1600;//2400;//16--200us---1.17M;//1.2M	//modify by yt 20120428 由1.2M 改为 1.17M

	nDataNum[i++] = 4000;//17--500us
	nDataNum[i++] = 4000;//18--1ms
	nDataNum[i++] = 4000;//19--2ms

	nDataNum[i++] = 3750;//20--5ms
	nDataNum[i++] = 3703;//21--10ms
	nDataNum[i++] = 3571;//22--20ms

	nDataNum[i++] = 4000;//23--50ms
	nDataNum[i++] = 4000;//24--100ms
	nDataNum[i++] = 4000;//25
	nDataNum[i++] = 4000;//26
	nDataNum[i++] = 4000;//27
	nDataNum[i++] = 4000;//28
	nDataNum[i++] = 4000;//29
	nDataNum[i++] = 4000;//30
	nDataNum[i++] = 4000;//31
	nDataNum[i++] = 4000;//32
	nDataNum[i++] = 4000;//33
	nDataNum[i++] = 4000;//34
	nDataNum[i++] = 4000;//35
	nDataNum[i++] = 4000;//36
	nDataNum[i++] = 4000;//37
	nDataNum[i++] = 4000;//38
	nDataNum[i++] = 4000;//39
	nDataNum[i++] = 4000;//40

	return nDataNum[nTimeDIV];
}

DLL_API float WINAPI dsoGetVoltZoom(USHORT nVoltDIV)
{
#ifdef NO_1_2_5MV
	nVoltDIV += 3;
#endif
	if(nVoltDIV == 0)//1mV--
	{
		return 5.0f;
	}
	else if(nVoltDIV == 1)//2mV
	{
		return 2.5f;
	}
	else if(nVoltDIV == 2)//5mV
	{
		return 10.0f;
	}
	else if(nVoltDIV == 3)//10mV
	{
		return 5.0f;
	}
	else if(nVoltDIV == 4)//20mV
	{
		return 2.5f;
	}	
	else if(nVoltDIV == 5)//50mV
	{
		return 10.0f;
	}	
	else if(nVoltDIV == 6)//100mV
	{
		return 5.0f;
	}	
	else if(nVoltDIV == 7)//200mV
	{
		return 2.5f;
	}	
	else if(nVoltDIV == 8)//500mV
	{
		return 10.0f;
	}	
	else if(nVoltDIV == 9)//1V
	{
		return 5.0f;
	}
	else if(nVoltDIV == 10)//2V
	{
		return 2.5f;
	}
	else if(nVoltDIV == 11)//5V
	{
		return 1.0f;
	}
	return 1.0f;
}


DLL_API USHORT WINAPI dsoWriteCalData(int index,USHORT* pCalData,ULONG nDataLen)
{
	ULONG i = 0,j = 0;
	UCHAR* pBuf;
	UCHAR pTmp[33];
	DSO_HANDLE hDSO;
	BYTE cmd[4] = {0xB1,0xB2,0xB3,0xB4};
	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	nDataLen *= 2;//
	pBuf = (UCHAR*)malloc(nDataLen*sizeof(UCHAR));
	for(i=0;i<nDataLen/2;i++)
	{
		pBuf[i*2] = pCalData[i] & 0xFF;
		pBuf[i*2+1] = (pCalData[i]>>8) & 0xFF;
	}
	for(j=0;j<4;j++)
	{
		pTmp[0] = cmd[j];
		for(i=0;i<32;i++)
		{
			pTmp[i+1] = pBuf[j*32+i];
		}
		//写入
		dsoWrite(hDSO, pTmp, 33);
		//读取
		memset(pTmp,0,33*sizeof(UCHAR));
		dsoRead(hDSO, pTmp,1);

		if(pTmp[0] != cmd[j])
		{
			free(pBuf);
			CloseHandle(hDSO);
			return 0;
		}
	}
	free(pBuf);
	CloseHandle(hDSO);
	return 1;
}

DLL_API USHORT WINAPI dsoReadCalData(int index,USHORT* pCalData,ULONG nDataLen)
{
	ULONG i = 0, j = 0;
	UCHAR* pBuf;
	DSO_HANDLE hDSO;
	BYTE cmd[2] = {0xB5,0xB6};
	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	pBuf = (UCHAR*)malloc(64*sizeof(UCHAR));
	for(j=0;j<2;j++)
	{
		//CMD+value
		pBuf[0] = cmd[j];
		//写入
		dsoWrite(hDSO, pBuf, 1);
		//读取
		memset(pBuf,0,64*sizeof(UCHAR));
		dsoRead(hDSO, pBuf,64);
		for(i=0;i<32;i++)
		{
			pCalData[j*32+i] = (pBuf[i*2+1] << 8) + pBuf[i*2];
		}
	}
	free(pBuf);
	CloseHandle(hDSO);
	return 1;
}

/*
0.	void Set_CHI_Param() 
设置通道个数

上位机发送2个字节：
FLAG，value1。
FLAG：  0xA0
value1：CH_Num，指当前要打开的通道个数，范围0x00~0x08；
下位机回复0xA0
*/

DLL_API USHORT WINAPI dsoSetCHNum(int index,USHORT nCHNum)
{
	UCHAR* pBuf;
	DWORD nLen = 2;
	DSO_HANDLE hDSO;

	//打开设备
	if (DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xA0;
	pBuf[1] = nCHNum;
	;//a0 00 00 00 - 00 30 00
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);
//
	if(pBuf[0] != 0xA0)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}

/*
1.	设置通道打开或关闭

上位机发送9个字节：
FLAG，value1，value2，value3，value4，value5，value6，value7，value8。
FLAG：  0xAA
value1~ value8：对应通道打开或关闭; value8代表第8个通道。 范围0x00~0x01；
含义如下：
0x01表示打开，
0x00表示关闭。

*/

DLL_API USHORT WINAPI dsoSetCHOpen(int index,BOOL* pEnable)
{
	UCHAR* pBuf;
	DWORD nLen = 9;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xAA;
	pBuf[1] = 0x01 & pEnable[0];
	pBuf[2] = 0x01 & pEnable[1];
	pBuf[3] = 0x01 & pEnable[2];
	pBuf[4] = 0x01 & pEnable[3];
	pBuf[5] = 0x01 & pEnable[4];
	pBuf[6] = 0x01 & pEnable[5];
	pBuf[7] = 0x01 & pEnable[6];
	pBuf[8] = 0x01 & pEnable[7];

	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	if(pBuf[0] != 0xAA)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}

/*
void Set_Sample_Num()
 设置总采样点数

上位机发送3个字节：
FLAG，value1，value2。
FLAG：  0xA1
value1*256+ value2：总采样点数；

下位机回复一个字节0xA1。

*/

DLL_API USHORT WINAPI dsoSetCHSample(int index,ULONG nSampleLen)
{
	UCHAR* pBuf;
	DWORD nLen = 3;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	nSampleLen *= 2;
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xA1;
	pBuf[1] = 0xFF & (nSampleLen>>8);
	pBuf[2] = 0xFF & (nSampleLen);
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	if(pBuf[0] != 0xA1)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}
/*
3.	void Set_Volt_Range() 
设置电压档位

上位机发送9个字节：
FLAG，value1，value2，value3，value4，value5，value6，value7，value8。
FLAG：  0xA2
value1~ value8：CHI_VoltRange，各通道电压档位。范围0x00~0x03；
含义如下：
0x00 ±5mV 量程
2mV--1
1mV--0
0x01 ±50mV 量程
20mV--4
10mV--3
5mV--2
0x02 ±500mV 量程
200mV--7
100mV--6
50mV--5
0x03 ±5V 量程
5V--11
2V--10
1V--9
500mV--8
下位机回复一个字节0xA2。

*/
DLL_API float WINAPI dsoGetVoltDIVFactor(USHORT nVoltDIV)//获取放大倍数
{
/*	//备份原来的 20131105
#ifdef NO_1_2_5MV
	nVoltDIV += 3;
#endif
	if(nVoltDIV < 2)
	{
		return 825.0f;
	}
	else if(nVoltDIV < 5)
	{
		return 82.5f;
	}
	else if(nVoltDIV < 8)
	{
		return 8.25f;
	}
	else if(nVoltDIV < 12)
	{
		return 1.0f;
	}
	else
	{
		return 1.0f;
	}
*/

#ifdef NO_1_2_5MV
	nVoltDIV += 3;
#endif

	double fValule = 0.0;
	switch(nVoltDIV)
	{
	case 3:		//10mV
		fValule = 414.4;
		break;	
	case 4:		//20mV
		fValule = 207.2;
		break;
	case 5:		//50mV
		fValule = 82.98;
		break;
	case 6:		//100mV
		fValule = 41.44;
		break;
	case 7:		//200mV
		fValule = 20.72;
		break;
	case 8:		//500mV
		fValule = 8.298;
		break;
	case 9:		//1.00V
		fValule = 4.144;
		break;
	case 10:	//2.00V
		fValule = 2.072;
		break;
	case 11:	//5.00V
		fValule = 0.8298;
		break;
	default:
		fValule = 1.0;
		break;
	}
	return (float)(fValule);
}

DLL_API USHORT WINAPI dsoGetRealVoltDIV(USHORT nVoltDIV)
{
#ifdef NO_1_2_5MV
	nVoltDIV += 3;
#endif
	if(nVoltDIV < 2)
	{
		return 0x00;
	}
	else if(nVoltDIV < 5)
	{
		return 0x01;
	}
	else if(nVoltDIV < 8)
	{
		return 0x02;
	}
	else if(nVoltDIV < 12)
	{
		return 0x03;
	}
	else
	{
		return 0x00;
	}
}

DLL_API USHORT WINAPI dsoSetCHVolt(int index,USHORT* pVolt,USHORT nVoltLen)
{
	UCHAR* pBuf;
	DWORD nLen = 9;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xA2;
/*	pBuf[1] = 0x03 & pVolt[0];
	pBuf[2] = 0x03 & pVolt[1];
	pBuf[3] = 0x03 & pVolt[2];
	pBuf[4] = 0x03 & pVolt[3];
	pBuf[5] = 0x03 & pVolt[4];
	pBuf[6] = 0x03 & pVolt[5];
	pBuf[7] = 0x03 & pVolt[6];
	pBuf[8] = 0x03 & pVolt[7];*/
	if(nVoltLen > MAX_CH_NUM)
		nVoltLen = MAX_CH_NUM;
	for(int i=0;i<nVoltLen;i++)
	{
		pBuf[i+1] = dsoGetRealVoltDIV(pVolt[i]);
	/*	if((pVolt[i]) < 2)
		{
			pBuf[i+1] = 0x00;//0x00 ±5mV 量程
		}
		else if((pVolt[i]) < 5)
		{
			pBuf[i+1] = 0x01;//0x01 ±50mV 量程
		}
		else if((pVolt[i]) < 8)
		{
			pBuf[i+1] = 0x02;//0x02 ±500mV 量程
		}
		else// < 12
		{
			pBuf[i+1] = 0x03;//0x03 ±5V 量程
		}*/
	}
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	if(pBuf[0] != 0xA2)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}

/*
4.	void Set_Clk() 
设置采样速度

上位机发送2个字节：
FLAG，value1。
FLAG：  0xA3
value1当前采样率。范围0x00~0x0F

*/

DLL_API USHORT WINAPI dsoSetSampleRate(int index,USHORT nSample)
{
	UCHAR* pBuf;
	DWORD nLen = 2;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xA3;
	pBuf[1] = nSample;
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	if(pBuf[0] != 0xA3)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}

/*
void BeginSaveData(unsigned char Continue) 
开始采集数据

上位机发送2个字节：
FLAG，value1。
FLAG：  0xA4
value1：continue。范围0x00~0x01；
0x01：  表示非连续采集模式，
0x02：  表示连续采集模式。
下位机回复一个字节0xA4。

*/

DLL_API USHORT WINAPI dsoStartCaptureData(int index,USHORT nMode)
{
	UCHAR* pBuf;
	DWORD nLen = 2;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xA4;
	if(nMode == YT_ROLL)
	{
		pBuf[1] = 0x02;
	}
	else
	{
		pBuf[1] = 0x01;//if(nMode == YT_NORMAL || nMode == YT_SCAN)
	}
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	if(pBuf[0] != 0xA4)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}

DLL_API USHORT WINAPI dsoPengLeiTest(int index)
{
	UCHAR* pBuf;
	DWORD nLen = 16;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xF0;
	for(int i=1;i<16;i++)
	{
		pBuf[i] = i+10;
	}
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	if(pBuf[0] != 0xF0)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
		
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}

/*
BOOL IsFinished()
数据采集是否已经结束

上位机发送2个字节：
FLAG，value1。
FLAG：  0xA5
value1：0x5A。
下位机回复两个字节：
FLAG，value1。
FLAG：  0xA5

value1：0x00或0x01或0x02。
0x00：   表示数据采集尚未触发，
0x01：   表示数据采集已触发采集未结束。
0x02：   表示数据采集已触发采集已经结束。

问题：这样处理有可能使软件误把采样数据作为应答处理吗

*/

DLL_API USHORT WINAPI dsoCaptureFinish(int index)
{
	UCHAR* pBuf;
	DWORD nLen = 2;
	DSO_HANDLE hDSO;

	
	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xA5;
	pBuf[1] = 0x5A;
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,2);
#ifdef _DEBUG
	dsoTest(hDSO);
#endif
	CloseHandle(hDSO);
	USHORT nRe = pBuf[1];
	free(pBuf);
//dsoPengLeiTest(index);
	return nRe;
}

/*
BOOL ReadBuffer(unsigned short *buffer,unsigned short Num)
读取缓冲区数据

上位机发送2个字节：
FLAG，value1。
FLAG：  0xA6
value1：要读取数据个数。范围待定；
下位机回复AD data


*/

DLL_API USHORT WINAPI dsoReadNormalData(int index,USHORT* pSrcData,ULONG nSrcLen)
{
	int i = 0;
	ULONG j = 0;
	UCHAR* pBuf;
	DWORD nCmdLen = 2;
	DSO_HANDLE hDSO;
	USHORT nReadTimes = 0;
	UCHAR* pTmpBuf = NULL;
	ULONG nReadCnt = 0;
	short k = 1;
	ULONG nDataLen = 0;
	ULONG nReadDataTotal = 0;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	pTmpBuf = (UCHAR*)malloc(64*sizeof(UCHAR));
	pBuf = (UCHAR*)malloc(nCmdLen*sizeof(UCHAR));
	//CMD+value
	for(k=2;k<4;k++)
	{
		i=0;
		nReadTimes = 0;
		nReadCnt = 0;
		memset(pBuf,0,sizeof(UCHAR)*nCmdLen);
		pBuf[0] = 0xC6;
		pBuf[1] = k;
		//写入
		dsoWrite(hDSO, pBuf, nCmdLen);
		//读取
		memset(pTmpBuf,0,sizeof(UCHAR)*64);
		dsoRead(hDSO, pTmpBuf,nCmdLen);
		//nDataLen是每个CH的数据长度
		nDataLen  = pTmpBuf[0]*256+pTmpBuf[1];//这里得到是字节数，是数据点数的2倍，所以2段数据的长度和是8000.
		if(nDataLen % 64 == 0)
		{
			nReadTimes = nDataLen/64;
		}
		else
		{
			nReadTimes = nDataLen/64 + 1;
		}
#ifdef _DEBUG
		dsoTest(hDSO);
#endif
		for(i=0;i<nReadTimes;i++)
		{
			pBuf[0] = 0xA6;
			pBuf[1] = k;
			//写入
			dsoWrite(hDSO, pBuf, nCmdLen);
			//读取
			memset(pTmpBuf,0,sizeof(UCHAR)*64);
			dsoRead(hDSO, pTmpBuf,64);
			for(j=0;j<64/2;j++)
			{
				*pSrcData = pTmpBuf[j*2+0] + pTmpBuf[j*2+1]*256;
				nReadCnt++;
				nReadDataTotal++;
				if(nReadDataTotal < nSrcLen)
				{
					pSrcData++;
				}
				if(nReadCnt >= nDataLen/2)
				{
					break;//已读取完毕
				}
			}
		}
	}
	free(pBuf);
	free(pTmpBuf);
	CloseHandle(hDSO);
	return 1;
}

DLL_API USHORT WINAPI dsoReadScanRollData(int index,USHORT* pSrcData,ULONG* pLen,USHORT nMode)
{
	int i = 0;
	ULONG j = 0;
	UCHAR* pBuf;
	DWORD nCmdLen = 2;
	DSO_HANDLE hDSO;
	USHORT nReadTimes = 0;
	UCHAR* pTmpBuf = NULL;
//	ULONG nReadCnt = 0;
	ULONG nDataLen = 0;
	ULONG nLen = 0;
	//打开设备

	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	pTmpBuf = (UCHAR*)malloc(64*sizeof(UCHAR));
	pBuf = (UCHAR*)malloc(nCmdLen*sizeof(UCHAR));
	//CMD+value
	memset(pBuf,0,sizeof(UCHAR)*nCmdLen);
	if(nMode == YT_SCAN)
	{
		pBuf[0] = 0xC9;
	}
	else if(nMode == YT_ROLL)
	{
		pBuf[0] = 0xC7;
	}
	//写入
	dsoWrite(hDSO, pBuf, 1);
	//读取
	memset(pTmpBuf,0,sizeof(UCHAR)*64);
	if(DSO_OK != dsoRead(hDSO, pTmpBuf,nCmdLen))
	{
		*pLen = 0;
		free(pBuf);
		free(pTmpBuf);
		CloseHandle(hDSO);
		return 0;
	}
	//nDataLen是每个CH的数据长度
	nDataLen  = pTmpBuf[0]*256+pTmpBuf[1];//这里得到是字节数，是数据点数的2倍，所以2段数据的长度和是8000.

	if(nDataLen > 8000)
	{
		nDataLen = 8000;
	}

	if(nDataLen % 64 == 0)
	{
		nReadTimes = nDataLen/64;
	}
	else
	{
		nReadTimes = nDataLen/64 + 1;
	}


	if(nMode == YT_SCAN)
	{
	//	Sleep(100);
		pBuf[0] = 0xCA;
	}
	else if(nMode == YT_ROLL)
	{
		pBuf[0] = 0xC8;
	}
	for(i=0;i<nReadTimes;i++)
	{
		//写入
		dsoWrite(hDSO, pBuf, 1);
		//读取
		memset(pTmpBuf,0,sizeof(UCHAR)*64);
		dsoRead(hDSO, pTmpBuf,64);
		
		for(j=0;j<64/2;j++)
		{
			*pSrcData = pTmpBuf[j*2+0] + pTmpBuf[j*2+1]*256;
			nLen++;
			if(nLen >= nDataLen/2)
			{
				break;//已读取完毕
			}
			pSrcData++;
		}
	}
	*pLen = nLen;
	free(pBuf);
	free(pTmpBuf);
	CloseHandle(hDSO);
	return 1;
}

/*
unsigned char I_O(unsigned char Out) 
   访问8bit双向IO

上位机发送3个字节：
FLAG，value1，value2。
FLAG：   0xA7
value1： 每个位代表对应IO当前设置为输入还是输出。该位为1表示输出，为0表示输入。字节最高位代表第8个IO。范围0x00~0x255；
value2： 对于value2每一位，若对应IO设置为输入，该位为0；若对应IO设置为输出，该位为1表示输出1，为0表示输出0。字节最高位代表第8个IO。范围0x00~0x255；

下位机回复2个字节：
FLAG，value1。
FLAG：   0Xa7
value1：IOvalue。

*/

DLL_API USHORT WINAPI dsoSetIOCtrl(int index,USHORT nIO1,USHORT nIO2)
{
	UCHAR* pBuf;
	DWORD nLen = 3;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xA7;
	pBuf[1] = nIO1;
	pBuf[2] = nIO2;
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	memset(pBuf,0,nLen*sizeof(UCHAR));
	//读取
	dsoRead(hDSO, pBuf,2);

	if(pBuf[0] != 0xA7)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
		
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}

/*
触发电平：

上位机发送2个字节：
FLAG，value1。
FLAG：   0xAB
value1： 范围待定；
下位机回复一个字节0XAB.

*/

DLL_API USHORT WINAPI dsoSetTrigLevel(int index,int nLevel,USHORT nVoltDIV,USHORT nCalData,float fAmpCalData)
{
	UCHAR* pBuf;
	DWORD nLen = 3;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
//	nLevel = MID_DATA + nLevel / dsoGetVoltZoom(nVoltDIV);	---del by yt 20110810 ---old
	nLevel = nCalData + nLevel / dsoGetVoltZoom(nVoltDIV) / fAmpCalData;
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xAB;
	pBuf[1] = (nLevel >> 8) & 0x0F;//高4位
	pBuf[2] = (nLevel) & 0xFF;//低八位
	
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	if(pBuf[0] != 0xAB)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
		
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}

/*
触发位置：

上位机发送2个字节：
FLAG，value1。
FLAG：   0XAC
value1： 范围0x00~0x64；
含义如下：
10*value1为触发点之后的点数。

下位机回复一个字节0XAC

*/

DLL_API USHORT WINAPI dsoSetHTrigPos(int index,USHORT nTimeDIV,USHORT nOpenNum,USHORT nHTriggerPos)
{
	UCHAR* pBuf;
	DWORD nLen = 9;
	USHORT nPos = 0;
	DSO_HANDLE hDSO;
	ULONG nPreTime=0,nAfterTime=0;
	ULONG nTotleDataNum = 0;
	double dbN = 0.0;
	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	
	if(nOpenNum > 0)
	{
		if(nTimeDIV < 24) // < 100ms	//add by yt 20110924
		{
			if(nOpenNum == 3 || nOpenNum == 5 || nOpenNum == 7)
			{
				nOpenNum++;
			}
			if(nTimeDIV <= 15) //<= 100us 按照4000点,50%算触发
			{
				nHTriggerPos = 50;
			}
		}
		nTotleDataNum = GetDataNum(nTimeDIV,nOpenNum);
		nPos = nTotleDataNum/nOpenNum * nHTriggerPos / 100 * 2;// *2 是因为要下字节数，不是数据点数
	}
	else
	{
		nTotleDataNum = 0;
		nPos = 0;
		CloseHandle(hDSO);
		return 1;
	}
//时间t=（1/s） *  n //单位us

	if(nTimeDIV <= 15)//100us -- 2.4M
	{
		nPreTime = 1000000.0 / 2400000 * (nTotleDataNum * nHTriggerPos / 100.0);
		nAfterTime = 1000000.0 / 2400000 * (nTotleDataNum * (100 - nHTriggerPos) / 100.0);
	}
	else if(nTimeDIV == 16)//200us -- 1.14M/1.2M//modify by yt 20120410 由1.2M 改为 1.14M
	{
	//	nPreTime = 1000000.0 / 1200000 * (nTotleDataNum * nHTriggerPos / 100.0);
	//	nAfterTime = 1000000.0 / 1200000 * (nTotleDataNum * (100 - nHTriggerPos) / 100.0);
		nPreTime = 1000000.0 / 1142857 * (nTotleDataNum * nHTriggerPos / 100.0);			//modify by yt 20120410 由1.2M 改为 1.14M
		nAfterTime = 1000000.0 / 1142857 * (nTotleDataNum * (100 - nHTriggerPos) / 100.0);	//modify by yt 20120410 由1.2M 改为 1.14M
	}
	else if(nTimeDIV == 17)//500us -- 800K
	{
		nPreTime = 1000000.0 / 800000 * (nTotleDataNum * nHTriggerPos / 100.0);
		nAfterTime = 1000000.0 / 800000 * (nTotleDataNum * (100 - nHTriggerPos) / 100.0);
	}
	else if(nTimeDIV == 18)//1ms -- 400K
	{
		nPreTime = 1000000.0 / 400000 * (nTotleDataNum * nHTriggerPos / 100.0);
		nAfterTime = 1000000.0 / 400000 * (nTotleDataNum * (100 - nHTriggerPos) / 100.0);
	}
	else if(nTimeDIV == 19)//2ms -- 200K
	{
		nPreTime = 1000000.0 / 200000 * (nTotleDataNum * nHTriggerPos / 100.0);
		nAfterTime = 1000000.0 / 200000 * (nTotleDataNum * (100 - nHTriggerPos) / 100.0);
	}
	else if(nTimeDIV == 20)//5ms -- 75k
	{
		nPreTime = 1000000.0 / 75000 * (nTotleDataNum * nHTriggerPos / 100.0);
		nAfterTime = 1000000.0 / 75000 * (nTotleDataNum * (100 - nHTriggerPos) / 100.0);
	}
	else if(nTimeDIV == 21)//10ms -- 37.037K
	{
		nPreTime = 1000000.0 / 37037 * (nTotleDataNum * nHTriggerPos / 100.0);
		nAfterTime = 1000000.0 / 37037 * (nTotleDataNum * (100 - nHTriggerPos) / 100.0);
	}
	else if(nTimeDIV == 22)//20ms -- 17.875K
	{
		nPreTime = 1000000.0 / 17875 * (nTotleDataNum * nHTriggerPos / 100.0);
		nAfterTime = 1000000.0 / 17875 * (nTotleDataNum * (100 - nHTriggerPos) / 100.0);
	}
	else if(nTimeDIV == 23)//50ms -- 7.937K
	{
		nPreTime = 1000000.0 / 7937 * (nTotleDataNum * nHTriggerPos / 100.0);
		nAfterTime = 1000000.0 / 7937 * (nTotleDataNum * (100 - nHTriggerPos) / 100.0);
	}
	else
	{
		nPreTime = 0;//1.0 / 37037 * (nTotleDataNum * nHTriggerPos / 100.0);
		nAfterTime = 0;//1.0 / 37037 * (nTotleDataNum * (100 - nHTriggerPos) / 100.0);
	}
	nPreTime += 1;
	nAfterTime += 1;

	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xAC;
	pBuf[1] = (nPos >> 8) & 0xFF;//高4位
	pBuf[2] = (nPos) & 0xFF;//低八位
	pBuf[3] = (nPreTime>>16) & 0xFF;
	pBuf[4] = (nPreTime>>8) & 0xFF;
	pBuf[5] = nPreTime & 0xFF;
	pBuf[6] = (nAfterTime>>16) & 0xFF;
	pBuf[7] = (nAfterTime>>8) & 0xFF;
	pBuf[8] = nAfterTime & 0xFF;
	/*
	T1=value3*256+ value4。
T2=value5*256+ value6。
T1, 预触发点数的时间.
T2, 后触发点数的时间。

  */
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	if(pBuf[0] != 0xAC)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
		
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}

/*
设置初始化：
上位机发送1个字节：
FLAG。
FLAG：   0XB0
当改变通道设置时，先发送设置初始化命令，再发送通道设置命令。
下位机回复0XB0.：

通道设置命令包括：
1.设置通道个数
2.	设置通道打开或关闭
3.	设置每个通道的采样点数
4.	设置电压档位

改变其中一个1234都要重新设置一遍

*/
DLL_API USHORT WINAPI dsoInitDevice(int index)
{
	UCHAR* pBuf;
	DWORD nLen = 1;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xB0;
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	if(pBuf[0] != 0xB0)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
		
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}
/*
21. 触发使能：
上位机发送：0xC0；
下位机回复：0xC0
*/

DLL_API USHORT WINAPI dsoTriggerEnable(int index)
{
	UCHAR* pBuf;
	DWORD nLen = 1;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xC0;
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	if(pBuf[0] != 0xC0)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
		
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}

/*
触发通道：
上位机发送：0xC1，Value1,Value2.
Value1:    0~7;
Value2:    0：下降沿；  1：上升沿。
*/

DLL_API USHORT WINAPI dsoSetTriggerInfo(int index,USHORT nTrigSrc,USHORT nSlope)
{
	UCHAR* pBuf;
	DWORD nLen = 3;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xC1;
	pBuf[1] = nTrigSrc;
	pBuf[2] = nSlope;
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	if(pBuf[0] != 0xC1)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
		
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}

/*
强制触发
上位机发送：0xC2；
下位机回复：0xC2；
*/

DLL_API USHORT WINAPI dsoForceTrigger(int index)
{
	UCHAR* pBuf;
	DWORD nLen = 1;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xC2;
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	if(pBuf[0] != 0xC2)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
		
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}

DLL_API BOOL WINAPI dsoGetDevEnable(int index)
{
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK == dsoOpen(&hDSO, index))
	{
		CloseHandle(hDSO);
		return TRUE;//
	}
	else
	{
//		CloseHandle(hDSO);
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////////////
//for test
DLL_API USHORT WINAPI dsoTest(DSO_HANDLE hDSO)
{
	UCHAR* pBuf;
	DWORD nLen = 2;
//	DSO_HANDLE hDSO;

	//打开设备
//	if(DSO_OK != dsoOpen(&hDSO, index))
//	{
//		return 0;
//	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xD0;
	//写入
	dsoWrite(hDSO, pBuf, 1);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,2);

	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xD1;
	//写入
	dsoWrite(hDSO, pBuf, 1);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,2);

	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xD2;
	//写入
	dsoWrite(hDSO, pBuf, 1);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,2);

	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xD3;
	//写入
	dsoWrite(hDSO, pBuf, 1);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,2);

//	CloseHandle(hDSO);
	free(pBuf);
	return 1;
}

void AmpCalc(MINICH* pMiniCH)//幅度休正
{
	ULONG j = 0,i = 0;
	double tmp;
	USHORT data = 0;

	for(i=0;i<MAX_CH_NUM;i++)
	{	
		if(pMiniCH->bEnable[i] && dsoGetRealVoltDIV(pMiniCH->nVoltDIV[i]) == 0x03)//电压档位为500mV,1V,2V,5V
		{
			for(j = 0;j < pMiniCH->nSrcDataLen;j++)
			{
				data = pMiniCH->pData[i][j];
				if(data >= 0x0be1 && data <= 0x0c5d)//0x0be1 ~ 0x0c5d
				{
					tmp =((1.13-1.04) * data + 1.04*0x0c5d - 1.13*0x0be1) / (0x0c5d-0x0be1);
				}
				else if(data >= 0x0c5e && data <= 0x0cd9)//0x0c5e ~ 0x0cd9
				{
					tmp = ((1.20-1.13) * data + 1.13*0x0cd9 -1.20*0x0c5d ) / (0x0cd9-0x0c5d);
				}
				else if(data >= 0x0cda && data <= 0x0d55)//0x0cda ~ 0x0d55
				{
					tmp = ((1.23-1.20) * data + 1.20*0x0d55 -1.23*0x0cd9 ) / (0x0d55 -0x0cd9);
				}
				else if(data >= 0x0d56 && data <= 0x0dd1)//0x0d56 ~ 0x0dd1
				{
					tmp = ((1.28-1.23) * data +1.23*0x0dd1 -1.28*0x0d55 ) / (0x0dd1 -0x0d55);
				}
				else if(data >= 0x0dd2 && data <= 0x0e4e)//0x0dd2 ~ 0x0e4e
				{
					tmp = ((1.30-1.28) * data + 1.28*0x0e4e -1.30*0x0dd1 ) / (0x0e4e -0x0dd1);
				}
				else if(data >= 0x0e4f && data <= 0x0eca)//0x0e4f ~ 0x0eca
				{
					tmp = ((1.32-1.30) * data +1.30*0x0eca -1.32*0x0e4e ) / (0x0eca -0x0e4e);
				}
				else if(data >= 0x0ecb && data <= 0x0f46)//0x0ecb ~ 0x0f46
				{
					tmp = ((1.34-1.32) * data +1.32*0x0f46 -1.34*0x0eca ) / (0x0f46 -0x0eca);
				}
				else
				{
					tmp = 1.0;
				}
				if(tmp != 0 || tmp != 1.0)
				{
					pMiniCH->pData[i][j] = data / tmp;
				}
			}
		}
	}
}

DLL_API USHORT WINAPI dsoSoftCopy(short* pSrcData,ULONG nLen,USHORT nStartPt)
{
	ULONG i = 0;
	short* pData = NULL;

	pData = new short[nLen];
	for(i = 0; i < nLen; i++)
	{
		pData[i] = pSrcData[nStartPt + i];
	}
	for(i = 0; i < nLen; i++)
	{
		pSrcData[i] = pData[i];
	}
	delete pData;
	return 1;
}

DLL_API USHORT WINAPI dsoSoftTrigger(short* pSrcData,MINICH* pMiniCH,USHORT nCH,short nTrigLevel,USHORT nSlope,USHORT nHTrigPos)
{
	int i = 0;
	USHORT nDataNum[MAX_TIMEDIV_NUM];
	USHORT nSrcDataLen = 0;
	short SrcData[BUF_4K_LEN];
	USHORT nTimeDIV = pMiniCH->nTimeDIV;
	USHORT nCHNum = pMiniCH->nOpenCHNum;
	USHORT nDataLen = pMiniCH->nSrcDataLen;
	short ntmp = 0;
	USHORT nFlag = 100;
	USHORT nDataTrig = nDataLen * 50 / 100;

	if(nCHNum == 0)
	{
		return 0;
	}
	else if(nCHNum == 2)
	{
		if(nTimeDIV < 7)
		{
			nTimeDIV = 7;
		}
	}
	else if(nCHNum == 3 || nCHNum == 4 || nCHNum == 5 || nCHNum == 6)
	{
		if(nTimeDIV < 8)
		{
			nTimeDIV = 8;
		}
	}
	else if(nCHNum == 7 || nCHNum == 8)
	{
		if(nTimeDIV < 9)
		{
			nTimeDIV = 9;
		}
	}

	nDataNum[i++] = 2;//1.2;//0--1ns---2.4M
	nDataNum[i++] = 2;//1.2;//1--2ns---2.4M
	nDataNum[i++] = 2;//1.2;//2--5ns---2.4M
	nDataNum[i++] = 2;//1.2;//3--10ns---2.4M
	nDataNum[i++] = 2;//1.2;//4--20ns---2.4M
	nDataNum[i++] = 2;//1.2;//5--50ns---2.4M
	nDataNum[i++] = 2;//6-- 100ns---2.4M---->1CH 当打开1CH个时，这是最小时基
	nDataNum[i++] = 5;//7-- 200ns---2.4M---->2CH 当打开2CH个时，这是最小时基
	nDataNum[i++] = 12;//8--500ns-- ---2.4M--->3,4,5,6CH 当打开3,4,5，6个CH时，这是最小时基

	nDataNum[i++] = 24;//9--1us ---2.4M--->7,8CH 当打开7或8个CH时，这是最小时基

	nDataNum[i++] = 48;//10--2us ---2.4M
	nDataNum[i++] = 120;//11--5us ---2.4M
	nDataNum[i++] = 240;//12--10us---2.4M
	nDataNum[i++] = 480;//13--20us---2.4M
	nDataNum[i++] = 1200;//14--50us---2.4M
	nDataNum[i++] = 2400;//15--100us---2.4M

//按CH分离数据
//	nCHNum = pMiniCH->nOpenCHNum;
	if(pMiniCH->nTimeDIV < 24)//小于24(100ms)的时基
	{
		if(nCHNum == 3 || nCHNum == 5 || nCHNum == 7)//小于24的时基如果打开奇数个CH,则按偶数通道个数计算数据点数并分配数据
		{
			nCHNum++;
		}
	}
	nSrcDataLen = nDataNum[nTimeDIV] / nCHNum;

	USHORT nPreTrigLen = nSrcDataLen * nHTrigPos / 100;
	USHORT nAfterTrigLen = nSrcDataLen - nPreTrigLen;
	

	

	for(i = 0; i < nDataLen; i++)
	{
		SrcData[i] = pSrcData[i];
	}

	for(i = nPreTrigLen; i < (nDataLen - nAfterTrigLen);i++)
	{
		ntmp = (short)(SrcData[i] - nTrigLevel);
		if(abs(ntmp) == 0)
		{
			if(i < (nDataLen - 3))
			{
				if(nSlope == RISE)
				{
					if(SrcData[i] <= SrcData[i + 1] && SrcData[i] < SrcData[i + 2] && SrcData[i] < SrcData[i + 3])
					{
						nDataTrig = i;
						break;
					}
				}
				else
				{
					if(SrcData[i] >= (SrcData[i + 1]) && SrcData[i] > (SrcData[i + 2]) && SrcData[i] > (SrcData[i + 3]))
					{
						nDataTrig = i;
						break;
					}
				}
			}
			else
			{
				if(nSlope == RISE)
				{
					if(SrcData[i] >= SrcData[i - 1] && SrcData[i] > SrcData[i - 2] && SrcData[i] > SrcData[i - 3])
					{
						nDataTrig = i;
						break;
					}
				}
				else
				{
					if(SrcData[i] <= SrcData[i - 1] && SrcData[i] < SrcData[i - 2] && SrcData[i] < SrcData[i - 3])
					{
						nDataTrig = i;
						break;
					}
				}
			}
		}
		else if(abs(ntmp) < nFlag)
		{
			if(i < (nDataLen - 3))
			{
				if(nSlope == RISE)
				{
					if(SrcData[i] <= SrcData[i + 1] && SrcData[i] < SrcData[i + 2] && SrcData[i] < SrcData[i + 3])
					{
						nFlag = abs(ntmp);
						nDataTrig = i;
					}
				}
				else
				{
					if(SrcData[i] >= SrcData[i + 1] && SrcData[i] > SrcData[i + 2] && SrcData[i] > SrcData[i + 3])
					{
						nFlag = abs(ntmp);
						nDataTrig = i;
					}
				}
			}
			else
			{
				if(nSlope == RISE)
				{
					if(SrcData[i] >= SrcData[i - 1] && SrcData[i] > SrcData[i - 2] && SrcData[i] > SrcData[i - 3])
					{
						nFlag = abs(ntmp);
						nDataTrig = i;
					}
				}
				else
				{
					if(SrcData[i] <= SrcData[i - 1] && SrcData[i] < SrcData[i - 2] && SrcData[i] < SrcData[i - 3])
					{
						nFlag = abs(ntmp);
						nDataTrig = i;
					}
				}
			}
		}
	}

	nDataTrig = nDataTrig - nPreTrigLen;
	for(i = 0; i < nSrcDataLen; i++)
	{
		pSrcData[i] = SrcData[nDataTrig + i];
	}
	pMiniCH->nSrcDataLen = nSrcDataLen;

	return nDataTrig;
}

DLL_API USHORT WINAPI dsoHTMiniReadNormalData(int index,MINICH* pMiniCH)
{
	ULONG j = 0, m = 0;
	int i=0;
	BOOL bFirst = FALSE;
	USHORT nData[BUF_4K_LEN];
	USHORT nCHNum;
//	float fTmp = 0.0f;

	if(pMiniCH->nOpenCHNum > 0)
	{
		//读取4K数据
		if(dsoReadNormalData(index,nData,BUF_4K_LEN) == 1)
		{
			//按CH分离数据
			nCHNum = pMiniCH->nOpenCHNum;
			if(pMiniCH->nTimeDIV < 24)//小于24(100ms)的时基
			{
				if(nCHNum == 3 || nCHNum == 5 || nCHNum == 7)//小于24的时基如果打开奇数个CH,则按偶数通道个数计算数据点数并分配数据
				{
					nCHNum++;
				}
			}
			pMiniCH->nSrcDataLen = GetDataNum(pMiniCH->nTimeDIV,nCHNum)/nCHNum;
			for(i=0;i<MAX_CH_NUM;i++)
			{	
				if(pMiniCH->bEnable[i])
				{
					bFirst = TRUE;
					for(j = 0;j < pMiniCH->nSrcDataLen;j++)
					{
						pMiniCH->pData[i][j] = nData[m + j * nCHNum];
						if(bFirst && j >= 1 && pMiniCH->pData[i][j] == 0)
						{
							pMiniCH->pData[i][j] = pMiniCH->pData[i][j-1];
							bFirst = FALSE;
						}
					}
					m++;
					if(pMiniCH->nOpenCHNum == 1 && pMiniCH->nTimeDIV <= 23)//只有1个CH,奇偶项调换一下,当时基<=23时(50ms),硬件采用了2个AD采集数据,所以根据实际情况要调换一下.
					{
						USHORT tmp;
						for(j = 0;j < pMiniCH->nSrcDataLen;j+=2)
						{
							tmp = pMiniCH->pData[i][j];
							pMiniCH->pData[i][j] = pMiniCH->pData[i][j+1];
							pMiniCH->pData[i][j+1]= tmp;
						}
						break;
					}
				}
			}
		}
		else
		{
			return 0;
		}
	}
//	AmpCalc(pMiniCH);
#ifndef NO_1_2_5MV//如果有1mv,2mv,5mv
	for(i=0;i<MAX_CH_NUM;i++)
	{
		if(pMiniCH->nVoltDIV[i] < 2)//1mV,2mV
		{
			for(j = 0;j < pMiniCH->nSrcDataLen;j++)
			{
				pMiniCH->pData[i][j] *= 4.75;
			}
		}
	}
#endif
	return 1;
}


DLL_API USHORT WINAPI dsoHTMiniReadAD(int index,MINICH* pMiniCH)//此函数暂时无用, remark by yt 20110924
{
	ULONG j = 0,m=0;
	int i=0;
	USHORT nData[BUF_4K_LEN];
	USHORT nCHNum;

	if(pMiniCH->nOpenCHNum > 0)
	{
		//读取4K数据
		if(dsoReadNormalData(index,nData,BUF_4K_LEN) == 1)
		{
			//按CH分离数据
			nCHNum = pMiniCH->nOpenCHNum;
			if(nCHNum == 3 || nCHNum == 5 || nCHNum == 7)
			{
				nCHNum++;
			}
			pMiniCH->nSrcDataLen = GetDataNum(pMiniCH->nTimeDIV,nCHNum)/nCHNum;
			for(i=0;i<MAX_CH_NUM;i++)
			{
				if(pMiniCH->bEnable[i])
				{
					for(j = 0;j < pMiniCH->nSrcDataLen;j++)
					{
						pMiniCH->pData[i][j] = (nData[m + j * nCHNum]);
					}
					m++;
				}
			}
		}
		else
		{
			return 0;
		}
	}
//	AmpCalc(pMiniCH);
#ifndef NO_1_2_5MV
	for(i=0;i<MAX_CH_NUM;i++)
	{
		if(pMiniCH->nVoltDIV[i] < 2)//1mV,2mV
		{
			for(j = 0;j < pMiniCH->nSrcDataLen;j++)
			{
				pMiniCH->pData[i][j] *= 4.75;
			}
		}
	}
#endif
	return 1;
}

DLL_API USHORT WINAPI dsoHTMiniReadScanData(int index,MINICH* pMiniCH)
{
	ULONG j = 0, m = 0, nLenOffset = 0;
	int i=0;
	USHORT nData[DEF_READ_DATA_LEN];
	USHORT nCHNum;
	ULONG nDataLen = 0, nTmpLen = 0;
//	float fTmp = 0.0f;


	if(pMiniCH->nOpenCHNum > 0)
	{
		//读取4K数据
		if(dsoReadScanRollData(index,nData,&nLenOffset,YT_SCAN) == 1)
		{
			nCHNum = pMiniCH->nOpenCHNum;
			pMiniCH->nSrcDataLen = GetDataNum(pMiniCH->nTimeDIV,nCHNum)/nCHNum;
			nDataLen = nLenOffset/nCHNum;
			if((pMiniCH->nAlreadyReadLen + nDataLen) > pMiniCH->nSrcDataLen)//已满
			{
				pMiniCH->nAlreadyReadLen = pMiniCH->nSrcDataLen;
			}
			else
			{
				pMiniCH->nAlreadyReadLen += nDataLen;
			}
			//按CH分离数据
			for(i=0;i<MAX_CH_NUM;i++)
			{	
				if(pMiniCH->bEnable[i])
				{
					for(j = 0;j < nDataLen;j++)
					{
						pMiniCH->pData[i][j] = nData[m + j * nCHNum];
					}
					m++;
					/*		---- add by yt 20110809 滚动下没有2AD的情况,所以不需要这个部分
					if(pMiniCH->nOpenCHNum == 1)//只有1个CH,奇偶项调换一下
					{
						USHORT tmp;
						for(j = 0;j < pMiniCH->nSrcDataLen;j+=2)
						{
							tmp = pMiniCH->pData[i][j];
							pMiniCH->pData[i][j] = pMiniCH->pData[i][j+1];
							pMiniCH->pData[i][j+1]= tmp;
						}
						break;
					}
					*/
				}
			}
		}
		else
		{
			return 0;
		}
	}
//	AmpCalc(pMiniCH);
#ifndef NO_1_2_5MV
	for(i=0;i<MAX_CH_NUM;i++)
	{
		if(pMiniCH->nVoltDIV[i] < 2)//1mV,2mV
		{
			for(j = 0;j < pMiniCH->nSrcDataLen;j++)
			{
				pMiniCH->pData[i][j] *= 4.75;
			}
		}
	}
#endif
	return 1;
}

DLL_API USHORT WINAPI dsoHTMiniReadRollData(int index,MINICH* pMiniCH,double* pdbT)
{
	ULONG j = 0, m = 0, nLenOffset = 0;
	int i=0;
	USHORT nData[DEF_READ_DATA_LEN];
	USHORT nCHNum;
	ULONG nDataLen = 0, nTmpLen = 0, nSumT = 0;

//	float fTmp = 0.0f;

	if(pMiniCH->nOpenCHNum > 0)
	{
		//读取4K数据
		if(dsoReadScanRollData(index,nData,&nLenOffset,YT_ROLL) == 1)
		{
			if(nLenOffset > DEF_READ_DATA_LEN)
			{
				nLenOffset = DEF_READ_DATA_LEN;
			}
			nCHNum = pMiniCH->nOpenCHNum + 1;//隐藏一个采集温度的CH
			pMiniCH->nSrcDataLen = GetDataNum(pMiniCH->nTimeDIV,nCHNum)/nCHNum;
			nDataLen = nLenOffset/nCHNum;
		/*	if((pMiniCH->nAlreadyReadLen + nDataLen) > pMiniCH->nSrcDataLen)//已满
			{
				pMiniCH->nAlreadyReadLen = pMiniCH->nSrcDataLen;
			}
			else
			{*/
				pMiniCH->nAlreadyReadLen += nDataLen;
		//	}
			//按CH分离数据
			for(i=0;i<MAX_CH_NUM;i++)
			{	
				if(pMiniCH->bEnable[i])
				{
					for(j = 0;j < nDataLen;j++)
					{
						pMiniCH->pData[i][j] = nData[m + j * nCHNum];
					}
					m++;
					/*		---- add by yt 20110809 滚动下没有2AD的情况,所以不需要这个部分
					if(pMiniCH->nOpenCHNum == 1 && nCHNum == 1)//只有1个CH,奇偶项调换一下
					{
						USHORT tmp;
						for(j = 0;j < pMiniCH->nSrcDataLen;j+=2)
						{
							tmp = pMiniCH->pData[i][j];
							pMiniCH->pData[i][j] = pMiniCH->pData[i][j+1];
							pMiniCH->pData[i][j+1]= tmp;
						}
						break;
					}*/
				}
			}
			//求温度
			nSumT = 0;
			for(i=0;i<nDataLen;i++)
			{
				nSumT += nData[pMiniCH->nOpenCHNum + i * nCHNum];
			}
			*pdbT = (1.42-(nSumT*1.0 / nDataLen)*3.3/4095)/0.00435+15;//计算实际温度
		}
		else
		{
			return 0;
		}
	}
#ifndef NO_1_2_5MV
	for(i=0;i<MAX_CH_NUM;i++)
	{
		if(pMiniCH->nVoltDIV[i] < 2)//1mV,2mV
		{
			for(j = 0;j < pMiniCH->nSrcDataLen;j++)
			{
				pMiniCH->pData[i][j] *= 4.75;
			}
		}
	}
#endif
	return 1;
}

DLL_API USHORT WINAPI dsoHTMiniFlash(int index,USHORT* pData,ULONG* pDataLen,short nPos,USHORT nType)
{
	int i=0,j=0;
	UCHAR* pBuf;
	DWORD nLen = 2;
	DSO_HANDLE hDSO;
	USHORT nRe = 0;
	ULONG nCnt = 0;

	if(nPos < 0 || nPos >= MAX_MINIFLASH_LEN)
	{
		return nRe;
	}
	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return nRe;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(64*sizeof(UCHAR));
	pBuf[0] = 0xB7;
	pBuf[1] = nPos;
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,64*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	if(pBuf[0] == 0xB7)
	{
		memset(pBuf,0,64*sizeof(UCHAR));
		if(nType == 0)//Load--读
		{
			nCnt = 0;
			pBuf[0] = 0xBA;
			//写入
			dsoWrite(hDSO, pBuf, 1);
			//读取
			memset(pBuf,0,64*sizeof(UCHAR));
			dsoRead(hDSO, pBuf,64);
			*pDataLen = pBuf[0] + (pBuf[1] << 8);
			for(j=0;j<62;j++)
			{
				pData[j] = pBuf[j+2];
				nCnt++;
				if(nCnt >= *pDataLen)
				{
					break;
				}
			}
			//是否继续读取数据
			while(nCnt < *pDataLen)
			{
				pBuf[0] = 0xBA;
				//写入
				dsoWrite(hDSO, pBuf, 1);
				//读取
				memset(pBuf,0,64*sizeof(UCHAR));
				dsoRead(hDSO, pBuf,64);
				for(j=0;j<64;j++)
				{
					pData[nCnt] = pBuf[j];
					nCnt++;
					if(nCnt >= *pDataLen)
					{
						break;
					}
				}
			}
			nRe = 1;
		}
		else if(nType == 1)//Download--写
		{
			nRe = 1;
			memset(pBuf,0,64*sizeof(UCHAR));
			pBuf[0] = 0xBF;
			pBuf[1] = *pDataLen & 0xFF;
			pBuf[2] = (*pDataLen >> 8) & 0xFF;
			//写入
			dsoWrite(hDSO, pBuf, 3);
			//读取
			memset(pBuf,0,64*sizeof(UCHAR));
			dsoRead(hDSO, pBuf,1);
			if(pBuf[0] == 0xBF)
			{
				ULONG nPackage = 0;
				memset(pBuf,0,64*sizeof(UCHAR));
				pBuf[0] = 0xB8;
				if(*pDataLen % 62 != 0)
				{
					nPackage = *pDataLen / 62 + 1;
				}
				else
				{
					nPackage = *pDataLen / 62;
				}
				nLen = 0;
				for(i=1;i<=nPackage;i++)
				{
					pBuf[1] = i;
					//写入
					for(j=0;j<62;j++)
					{
						pBuf[j+2] = pData[(i-1)*62+j];
						nLen++;
						if(nLen >= *pDataLen)
						{
							break;
						}
					}
					dsoWrite(hDSO, pBuf,64);
					//读取
					nCnt = 0;
					memset(pBuf,0,64*sizeof(UCHAR));
					while(pBuf[0] != 0xB8)
					{
						dsoRead(hDSO, pBuf,1);
						nCnt++;
						if(nCnt > 1000)
						{
							break;
						}
					}
					if(nCnt > 1000)
					{
						nRe = 0;
						break;
					}
				}
			}
			else
			{
				nRe = 0;
			}
		}
		else
		{
			nRe = 0;
		}
	}
	CloseHandle(hDSO);
	free(pBuf);
	return nRe;
}

DLL_API USHORT WINAPI dsoSetGeneratorTimeUS(int index,double dbTime,USHORT nWaveNum)
{
	UCHAR* pBuf;
	DWORD nLen = 6;
	DSO_HANDLE hDSO;
	long nTime = 0;

	//打开设备
	if (DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xB9;
	if(dbTime < 20.0)
	{
		if(nWaveNum == 3)
		{
			nTime = 12.0 * dbTime - 14.68;
		}
		else if(nWaveNum == 5)
		{
			nTime = 12.0 * dbTime - 22;
		}
		else
		{
			nTime = 12.0 * dbTime - 32.98;
		}
		if(nTime < 0)
		{
			nTime = 0;
		}
		pBuf[1] = 2; //高速模式
	}
	else
	{
		nTime = dbTime * 24;
		pBuf[1] = 1; //普通模式
	}
	pBuf[2] = (nTime) & 0xFF;
	pBuf[3] = (nTime>>8) & 0xFF;
	pBuf[4] = (nTime>>16) & 0xFF;
	pBuf[5] = (nTime>>24) & 0xFF;
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);
//
	if(pBuf[0] != 0xB9)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}

DLL_API USHORT WINAPI dsoSetGeneratorOutput(int index,USHORT nPos, USHORT nOnOff,USHORT nWaveNum)
{
	UCHAR* pBuf;
	DSO_HANDLE hDSO;

	//打开设备
	if (DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	pBuf = (UCHAR*)malloc(3*sizeof(UCHAR));
	pBuf[0] = 0xB7;
	pBuf[1] = nPos;
	//写入
	dsoWrite(hDSO, pBuf, 2);
	memset(pBuf,0,3*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);
	//CMD+value
	memset(pBuf,0,3*sizeof(UCHAR));
	pBuf[0] = 0xBB;
	pBuf[1] = nWaveNum;
	pBuf[2] = nOnOff;
	//写入
	dsoWrite(hDSO, pBuf, 3);
	//读取
	memset(pBuf,0,3*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);
//
	if(pBuf[0] != 0xBB)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}

DLL_API USHORT WINAPI dsoSetIOEnableProgram(int index,USHORT nEnable)//是IO口可编程
{
	UCHAR* pBuf;
	DWORD nLen = 2;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xBC;
	pBuf[1] = nEnable & 0x01;

	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	if(pBuf[0] != 0xBC)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
		
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}
//获取当前温度 
DLL_API USHORT WINAPI dsoGetNowT(int index,double* pT)
{
	UCHAR* pBuf;
	DWORD nLen = 64;
	DSO_HANDLE hDSO;
	USHORT T = 0;
	ULONG nSum = 0;
	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xE4;
	pBuf[1] = 0x01;
	//写入
	dsoWrite(hDSO, pBuf, 2);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xE6;
	pBuf[1] = 0x01;
	//写入
	dsoWrite(hDSO, pBuf, 2);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,nLen);

	for(int i=0;i<10;i+=2)
	{
		T = pBuf[i] + (pBuf[i+1]<<8);
		nSum += T;
	}
	T = USHORT(nSum / 5);
	free(pBuf);
	CloseHandle(hDSO);
	
/*	USHORT nARMVersion = 0;
	dsoGetARMVersion(index,&nARMVersion);
	if(nARMVersion >= 0x207)
	{
		*pT = (1.42-T*3.3/4095)/0.00435 + 25;//+15;//计算实际温度
	}
	else//老版本
	{*/
		*pT = (1.42-T*3.3/4095)/0.00435 + 15;//计算实际温度
//	}

	return 1;
}
//获取校对时温度
DLL_API USHORT WINAPI dsoGetCalT(int index,double* pT)
{
	UCHAR* pBuf;
	DWORD nLen = 2;
	DSO_HANDLE hDSO;
	USHORT T = 0;
	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xE5;
	//写入
	dsoWrite(hDSO, pBuf, 1);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,2);
	T = pBuf[0] + (pBuf[1]<<8);
	free(pBuf);
	CloseHandle(hDSO);
/*
	USHORT nARMVersion = 0;
	dsoGetARMVersion(index,&nARMVersion);
	if(nARMVersion >= 0x207)
	{
		*pT = (1.42-T*3.3/4095)/0.00435 + 25;//+15;//计算实际温度
	}
	else//老板本
	{*/
		*pT = (1.42-T*3.3/4095)/0.00435 + 15;//计算实际温度
//	}
	return 1;
}




DLL_API WORD WINAPI dsoSetLanguage(int index,WORD nSet)//add by yt 20110729
{
	UCHAR* pBuf;
	DWORD nLen = 3;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xE7;
	pBuf[1] = nSet & 0xFF;
	pBuf[2] = (nSet>>8) & 0xFF;
	

	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	if(pBuf[0] != 0xE7)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}

DLL_API WORD WINAPI dsoGetLanguage(int index,WORD* pSet)//add by yt 20110729
{
	UCHAR* pBuf;
	DWORD nLen = 2;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xE8;
	//写入
	dsoWrite(hDSO, pBuf, 1);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,2);

	*pSet = pBuf[0] + (pBuf[1]<<8);
	free(pBuf);
	CloseHandle(hDSO);

	return 1;
}

DLL_API WORD WINAPI dsoGetARMVersion(int index,WORD* pVersion)//add by yt 20110903
{
	UCHAR* pBuf;
	DWORD nLen = 2;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xE9;
	//写入
	dsoWrite(hDSO, pBuf, 1);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,2);

	*pVersion = pBuf[0] + (pBuf[1]<<8);
	free(pBuf);
	CloseHandle(hDSO);

	return 1;
}

DLL_API float WINAPI dsoGetAmpCalData(USHORT* pCalLevel,USHORT nCH,USHORT nVoltDIV)
{
	USHORT n = dsoGetRealVoltDIV(nVoltDIV);
	int a = pCalLevel[CAL_LEVEL_LEN /2 + n * MAX_CH_NUM + nCH] - pCalLevel[n * MAX_CH_NUM + nCH];

	if(a != 0)
	{
		if(n == 0)
		{
			return 1028.0f / a;
		}
		else
		{
			return 513.0f / a;
		}
	}
	else
	{
		return 1.0f;
	}
}

DLL_API USHORT WINAPI dsoSDReadNormalData(int index,MINICH* pMiniCH,short** pSrcData,USHORT* pCalLevel)
{
	USHORT nReadOK = 0;
	ULONG i = 0, j = 0;
	USHORT nCalLevel = 0;
	float fAmpCalData = 0.0f;
	float fVoltZoom = 0.0f;

	nReadOK = dsoHTMiniReadNormalData(index,pMiniCH);

	if(nReadOK == 1)
	{
		for(j=0;j<MAX_CH_NUM;j++)
		{
			if(pMiniCH->bEnable[j] == 1)
			{
				nCalLevel = pCalLevel[dsoGetRealVoltDIV(pMiniCH->nVoltDIV[j])*MAX_CH_NUM+j];
				fAmpCalData = dsoGetAmpCalData(pCalLevel,j,pMiniCH->nVoltDIV[j]);
				fVoltZoom = dsoGetVoltZoom(pMiniCH->nVoltDIV[j]);

				for(i=0;i<pMiniCH->nSrcDataLen;i++)
				{
					*(pSrcData[j]+i) = short(( *(pMiniCH->pData[j]+i) - nCalLevel) * fAmpCalData * fVoltZoom);
				}
			}
		}
		return nReadOK;
	}
	else
	{
		return 0;
	}
}

DLL_API USHORT WINAPI dsoSaveToDevice(int index)
{
	UCHAR* pBuf;
	DWORD nLen = 1;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	pBuf = (UCHAR*)malloc(nLen*sizeof(UCHAR));
	pBuf[0] = 0xea;
	//写入
	dsoWrite(hDSO, pBuf, nLen);
	//读取
	memset(pBuf,0,nLen*sizeof(UCHAR));
	dsoRead(hDSO, pBuf,1);

	if(pBuf[0] != 0xea)
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 0;
		
	}
	else
	{
		free(pBuf);
		CloseHandle(hDSO);
		return 1;
	}
}

DLL_API USHORT WINAPI dsoWriteAmpCalData(int index,short* pCalData)
{

	ULONG i = 0,j = 0;
	UCHAR Buf[184];
	UCHAR SendBuf[64];
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	
	for(i=0;i<92;i++)
	{
		Buf[i*2] = pCalData[i] & 0xFF;
		Buf[i*2+1] = (pCalData[i]>>8) & 0xFF;
	}
	//CMD+value
	SendBuf[0] = 0xF1;
	for(i=0;i<63;i++)
	{
		SendBuf[i+1] = Buf[i];
	}
	//写入
	dsoWrite(hDSO, SendBuf,64);
	//读取
	SendBuf[0] = 0;
	dsoRead(hDSO, SendBuf,1);

	if(SendBuf[0] != 0xF1)
	{
		CloseHandle(hDSO);
		return 0;
	}
	
	//CMD+value
	SendBuf[0] = 0xF2;
	for(i=0;i<63;i++)
	{
		SendBuf[i+1] = Buf[63+i];
	}
	//写入
	dsoWrite(hDSO, SendBuf,64);
	//读取
	SendBuf[0] = 0;
	dsoRead(hDSO, SendBuf,1);

	if(SendBuf[0] != 0xF2)
	{
		CloseHandle(hDSO);
		return 0;
	}
	//CMD+value
	SendBuf[0] = 0xF9;
	for(i=0;i<58;i++)
	{
		SendBuf[i+1] = Buf[126 + i];
	}
	//写入
	dsoWrite(hDSO, SendBuf,59);
	//读取
	SendBuf[0] = 0;
	dsoRead(hDSO, SendBuf,1);

	if(SendBuf[0] != 0xF1)
	{
		CloseHandle(hDSO);
		return 0;
	}

	CloseHandle(hDSO);
	return 1;

}

DLL_API USHORT WINAPI dsoReadAmpCalData(int index,short* pCalData)
{
	ULONG i = 0,j = 0;
	UCHAR Buf[64];
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	memset(Buf,0,sizeof(Buf));
	Buf[0] = 0xF7;
	//写入
	dsoWrite(hDSO, Buf,1);
	//读取
	Buf[0] = 0;
	dsoRead(hDSO, Buf,64);
	for(i=0;i<32;i++)
	{
		pCalData[i] = short(Buf[i*2] + (Buf[i*2+1] << 8));
	}
	//CMD+value
	memset(Buf,0,sizeof(Buf));
	Buf[0] = 0xF8;
	//写入
	dsoWrite(hDSO, Buf,1);
	//读取
	Buf[0] = 0;
	dsoRead(hDSO, Buf,64);
	for(i=0;i<32;i++)
	{
		pCalData[32 + i] = short(Buf[i*2] + (Buf[i*2+1] << 8));
	}
	
	//CMD+value
	memset(Buf,0,sizeof(Buf));
	Buf[0] = 0xFA;
	//写入
	dsoWrite(hDSO, Buf,1);
	//读取
	Buf[0] = 0;
	dsoRead(hDSO, Buf,56);
	for(i=0;i<28;i++)
	{
		pCalData[64 + i] = short(Buf[i*2] + (Buf[i*2+1] << 8));
	}

	CloseHandle(hDSO);
	return 1;
}

DLL_API USHORT WINAPI dsoStartWatch(int index)
{
	ULONG i = 0,j = 0;
	UCHAR Buf;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	Buf = 0xF3;
	//写入
	dsoWrite(hDSO,&Buf,1);
	//读取
	Buf = 0;
	dsoRead(hDSO,&Buf,1);

	if(Buf != 0xF3)
	{
		CloseHandle(hDSO);
		return 0;
	}
	CloseHandle(hDSO);
	return 1;
}

DLL_API USHORT WINAPI dsoEndWatch(int index)
{
	ULONG i = 0,j = 0;
	UCHAR Buf;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	Buf = 0xF4;
	//写入
	dsoWrite(hDSO,&Buf,1);
	//读取
	Buf = 0;
	dsoRead(hDSO,&Buf,1);
	if(Buf != 0xF4)
	{
		CloseHandle(hDSO);
		return 0;
	}
	CloseHandle(hDSO);
	return 1;
}

DLL_API USHORT WINAPI dsoSetGND(int index)
{
	ULONG i = 0,j = 0;
	UCHAR Buf;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	Buf = 0xF5;
	//写入
	dsoWrite(hDSO,&Buf,1);
	//读取
	Buf = 0;
	dsoRead(hDSO,&Buf,1);
	if(Buf != 0xF5)
	{
		CloseHandle(hDSO);
		return 0;
	}
	CloseHandle(hDSO);
	return 1;
}

DLL_API USHORT WINAPI dsoExitGND(int index)
{
	ULONG i = 0,j = 0;
	UCHAR Buf;
	DSO_HANDLE hDSO;

	//打开设备
	if(DSO_OK != dsoOpen(&hDSO, index))
	{
		return 0;
	}
	//CMD+value
	Buf = 0xF6;
	//写入
	dsoWrite(hDSO,&Buf,1);
	//读取
	Buf = 0;
	dsoRead(hDSO,&Buf,1);
	if(Buf != 0xF6)
	{
		CloseHandle(hDSO);
		return 0;
	}
	CloseHandle(hDSO);
	return 1;
}
