// HTLANDll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "HTLANDll.h"

CHTLAN m_HTLAN[MAX_USB_DEV_NUMBER];

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

CHTLAN::CHTLAN()
{
}

CHTLAN::~CHTLAN()
{
}

BOOL CHTLAN::Init(char* szServer, USHORT port)	//网络初始化
{
	WSADATA wsaData;
	ULONG iMode = 1;
	WORD sockVersion = MAKEWORD(2, 2);
	if(::WSAStartup(sockVersion, &wsaData) != 0)
	{
		return FALSE;
	}
	m_socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(m_socket == INVALID_SOCKET)
	{
		return FALSE;
	}
	
	// 也可以在这里调用bind函数绑定一个本地地址
	// 否则系统将会自动安排
	/*SOCKADDR_IN cliAddr; 
	cliAddr.sin_family = AF_INET;
	cliAddr.sin_port = htons(2002);
	cliAddr.sin_addr.S_un.S_addr = inet_addr("192.168.2.1");
	bind(m_socket, (struct sockaddr*)&cliAddr, sibzeof(cliAddr));*/

	ioctlsocket(m_socket, FIONBIO, (u_long FAR*)&iMode);
		
	// 填写远程地址信息
	m_SrvAddr.sin_family = AF_INET;
	m_SrvAddr.sin_port = htons(port);
	// 注意，这里要填写服务器程序所在机器的IP地址
	// 如果你的计算机没有联网，直接使用127.0.0.1即可
	m_SrvAddr.sin_addr.S_un.S_addr = inet_addr(szServer);

	return TRUE;
}

BOOL CHTLAN::Close()
{
	if (m_socket != NULL)
	{
		::closesocket(m_socket);
		::WSACleanup();	
	}
	m_socket = NULL;
	return TRUE;
}

BOOL CHTLAN::SendPackage(BYTE* data, ULONG len)	//发送一包
{
	int n = ::sendto(m_socket, (char*)data, len, 0, (sockaddr*)&m_SrvAddr, sizeof(m_SrvAddr));
	if (n > 0 && n <= (int)len)//add " && n <= (int)len " by yt 20111121
		return 1;
	else
		return 0;
}

/*
USHORT CHTLAN::LANSend(ULONG cmd, ULONG con, ULONG val)	//向服务端发送控制字
{
	SENDSESSION session;
	session.cmd = cmd;
	session.conreg = con;
	session.value = val;

	return LANSendPackage((BYTE*)&session, sizeof(SENDSESSION));
}*/

USHORT CHTLAN::SendBulk(BYTE* data, ULONG len)	//向服务端发送批量数据包 
{
	ULONG size = 0;
	BYTE* pData = data;
	if(len == 0)
	{
		return 0;
	}
	while(len > 0)
	{
		if (len >= LAN_PACKAGE_SIZE)	//如果当前要放送的数据包比较大
		{
			size = LAN_PACKAGE_SIZE;			
		}
		else
		{
			size = len;
		}
		if (0 == SendPackage(pData, size))
			return 0;
		pData += size;
		len -= size;
	}
	return 1;
}

/*
BOOL CHTLAN::RecvData(BYTE* data, ULONG* len, sockaddr_in* addr)
{
	int nLen = sizeof(sockaddr_in);
	int nRecv = recv(m_socket, (char*)data, LAN_PACKAGE_SIZE, 0);
	if (nRecv > 0)
	{
		*len = nRecv;
		return 1;
	}
	else
	{
		*len = 0;
		return 0;
	}
}
*/

BOOL CHTLAN::RecvPackage(BYTE* data,ULONG* pLen)	//接收一包数据
{
	int nLen = sizeof(sockaddr_in);
	int nRecv = ::recvfrom(m_socket, (char*)data, LAN_PACKAGE_SIZE, 0, (sockaddr*)&m_SrvAddr, &nLen);
	if (nRecv > 0)
	{
		*pLen = nRecv;
		return 1;
	}
	else
	{
		*pLen = 0;
		return 0;
	}
}

BOOL CHTLAN::RecvBulk(BYTE* data, ULONG len)	//接收数据	//改函数目前没有使用--remark by yt 200111121
{
	BYTE* pData = data;
	ULONG size = 0;
	ULONG tn = 0;
	int i = 1000;//add by yt 20111121
	while((size < len) && (i > 0))//add " && (i > 0)" by yt 20111121
	{
		if (1 == RecvPackage(pData, &tn))
		{
			size += tn;
			pData += tn;
			i = 1000;//add by yt 20111121
		}
		i--;
		Sleep(1);
	}
	if(i <= 0)//add by yt 20111121
	{
		return 0;
	}
	else//add by yt 20111121
	{
		return 1;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////

BOOL LANInit(int index, char* szIpV4, USHORT iPort)
{
	return m_HTLAN[index].Init(szIpV4, (USHORT)iPort);
}

BOOL LANClose(int index)
{
	return m_HTLAN[index].Close();
}

BOOL LANRecvDataPackage(ULONG index, BYTE* data)//接收数据包
{
	ULONG nLen = 0;
	int i = 300;//0;

	while(i > 0 && m_HTLAN[index].RecvPackage(data,&nLen) <= 0)
	{
		Sleep(1);
		i--;
	}

	if (i <= 0)
	{
		return 0;
	}

	return 1;
}

BOOL LANSendCtrl(WORD nDeviceIndex,BYTE* pData,ULONG nLen)
{
	return m_HTLAN[nDeviceIndex].SendPackage(pData,nLen);
}

#define ERR_TIME	3
BOOL LANSendBulkData(ULONG DeviceIndex,BYTE* data, ULONG len)
{
	ULONG size = LAN_PACKAGE_SIZE;
	int i;
	i = 0; 
	int err = 0;
	while(len > 0 && err <= ERR_TIME)
	{		
		if (len >= LAN_PACKAGE_SIZE)
		{
			size = LAN_PACKAGE_SIZE;
		}
		else
			size = len;
		
		len -= size;
		if (0 == m_HTLAN[DeviceIndex].SendBulk(data + i * LAN_PACKAGE_SIZE, size))
		{
			err++;
			continue;
		}		
		i ++;

	}
	return (err <= ERR_TIME);	
}

/*
BOOL LANWriteCtrl(ULONG DeviceIndex, ULONG conreg, ULONG value, ULONG* data)
{
	return LANDownloadCtrl(DeviceIndex, CMD_WRITE_CTRL, conreg, value, data);
}

BOOL LANReadCtrl(ULONG DeviceIndex, ULONG conreg, ULONG value, ULONG* data)
{
	return LANDownloadCtrl(DeviceIndex, CMD_READ_CTRL, conreg, value, data);
}
*/

/*
BOOL LANRecvData(ULONG DeviceIndex,BYTE* data, ULONG* rlen)
{
	BYTE byte[1];
	int i = 0;
	int nSec = 0;
	if (*rlen % LAN_PACKAGE_SIZE == 0)
		nSec = *rlen / LAN_PACKAGE_SIZE;
	else
		nSec = *rlen / LAN_PACKAGE_SIZE + 1;
	byte[0] = 0;
	
	for (i = 0; i < nSec; i++)
	{	
		if (0 == m_HTLAN[DeviceIndex].SendBulk(byte, 1))
		{
			return 0;
		}				

		if (0 == LANRecvDataPackage(DeviceIndex, data + i * LAN_PACKAGE_SIZE))
		{
			return 0;
		}
	}
	return 1;
}

BOOL LANUploadData(ULONG DeviceIndex,ULONG len, ULONG type, BYTE* data)	//上传数据
{
	BOOL re = 0;
	ULONG nSec = 0;
	SENDSESSION session;	//发送的数据
	session.cmd = CMD_READ_DATA;
	session.conreg = len;
	session.value = type;	
	re = m_LAN[DeviceIndex].LANSend((BYTE*)&session, sizeof(session));
	if (!re)
	{
		return 0;
	}
	re = LANRecvDataPack(DeviceIndex, data, &type);
	if (!re)
	{
		return 0;
	}
	while(len > 0)
	{
		if (0 == m_HTLAN[DeviceIndex].LANSend(data, 1))
		{
			return 0;
		}		
		nSec = LAN_PACKAGE_SIZE;
		if (0 == LANRecvDataPack(DeviceIndex, data, &nSec))
		{
			return 0;
		}
		data += nSec;
		len -= nSec;
	}
	
	return 1;
}
*/
BOOL ResetDevice(WORD DeviceIndex)
{
	return LANSendCtrl(DeviceIndex,(UCHAR*)(0x27),1);
}

WORD LANSetIP(WORD DeviceIndex,ULONG nIP,ULONG nSubnetMask,ULONG nGateway,USHORT nPort,BYTE* pMac)
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize=22;

	ResetDevice(DeviceIndex);

	outBuffer=(PUCHAR) malloc(m_nSize);
//命令
	outBuffer[0]=0x20;
    outBuffer[1]=0x00;
//IP
	outBuffer[2]=(unsigned char)((nIP >> 16) & 0xFF);                          
	outBuffer[3]=(unsigned char)((nIP>>24) & 0xFF);  
	outBuffer[4]=(unsigned char)(nIP & 0xFF); 
	outBuffer[5]=(unsigned char)((nIP >> 8) & 0xFF);
//网关
	outBuffer[6]=(unsigned char)((nGateway >> 16) & 0xFF);                          
	outBuffer[7]=(unsigned char)((nGateway>>24) & 0xFF);  
	outBuffer[8]=(unsigned char)(nGateway & 0xFF); 
	outBuffer[9]=(unsigned char)((nGateway >> 8) & 0xFF);	
//子网掩码
	outBuffer[10]=(unsigned char)((nSubnetMask >> 16) & 0xFF);                          
	outBuffer[11]=(unsigned char)((nSubnetMask>>24) & 0xFF);  
	outBuffer[12]=(unsigned char)(nSubnetMask & 0xFF); 
	outBuffer[13]=(unsigned char)((nSubnetMask >> 8) & 0xFF);
//端口
	outBuffer[14]=(unsigned char)(nPort&0xff) ;
	outBuffer[15]=(unsigned char)((nPort>>8)&0xFF);
//
	outBuffer[16]=pMac[1];
	outBuffer[17]=pMac[0];
	outBuffer[18]=pMac[3];
	outBuffer[19]=pMac[2];
	outBuffer[20]=pMac[5];
	outBuffer[21]=pMac[4];
	WORD R =  LANSendCtrl(DeviceIndex,outBuffer,m_nSize); 
	//关闭设备
	free(outBuffer);
    return R;
}
///////////////////////////////////////////////////////////////////////////////////////////////
WORD SetOffsetChannel(WORD DeviceIndex,WORD DacData,WORD nCH)
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize=8;
	WORD nCHHigh = 0xFF;
	if(nCH == CH1)
	{
		nCHHigh = 0x10;
	}
	else if(nCH == CH2)
	{
		nCHHigh = 0x50;
	}
	else if(nCH == CH3)
	{
		nCHHigh = 0x90;
	}
	else if(nCH == CH4)
	{
		nCHHigh = 0xD0;
	}
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR)malloc(m_nSize);
	outBuffer[0]=0x16;
    outBuffer[1]=0x00;
	outBuffer[2]=0x00;  //daccs                              
	outBuffer[3]=0x00;  
	outBuffer[4]=0xff&(unsigned char)(DacData&0xff) ;
	outBuffer[5]=0xff&(unsigned char)(nCHHigh|((DacData>>8)&0x0F));
	outBuffer[6]=0x00;
	outBuffer[7]=0x00;
	WORD R =  LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
	return R;
}

WORD SetTrigLevel(WORD DeviceIndex,WORD DacData)
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize=8;

	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR) malloc(m_nSize);
	outBuffer[0]=0x16;
    outBuffer[1]=0x00;
	outBuffer[2]=0x01;  //daccs                              
	outBuffer[3]=0x00;  
	outBuffer[4]=0xff&(unsigned char)(DacData&0xff);
	outBuffer[5]=0xff&(unsigned char)(0x10|((DacData>>8)&0x0F)) ;
	outBuffer[6]=0x00;
	outBuffer[7]=0x00;
	WORD R = LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
	return R;
}


WORD SetTriggerLength(WORD DeviceIndex,ULONG nBufferLen,WORD nHTriggerPos,WORD nTimeDIV,WORD nYTFormat)//设置触发长度
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize=12;
	long trig_lenth,pre_trig_lenth;
	long nOffset = 0;
	ULONG nPre,nTrg;
	ULONG BytesReturned=0;
	ULONG nMaxLength = 16777215;

#ifdef TRIGGER_POS_OFFSET
	nMaxLength = 16777215 - LAN_PACKAGE_SIZE*2;
#endif

	if(nYTFormat == YT_SCAN)
	{
		if(nTimeDIV >= MIN_ROLL_TIMEDIV)
		{
			nBufferLen = nBufferLen * 4;
		}
	}
	else if(nYTFormat == YT_ROLL)
	{
		nHTriggerPos = 0;//Roll 水平触发点为0,没有预触发
	}

	if(nBufferLen > nMaxLength)
	{
		nBufferLen = nMaxLength;
	}

	if(nHTriggerPos == 100)
	{
		trig_lenth = nMaxLength;
		pre_trig_lenth = nMaxLength - nBufferLen;
	}
	else
	{
		nTrg = ULONG((100 - nHTriggerPos) * nBufferLen / 100.0);
		nPre = ULONG(nHTriggerPos * nBufferLen / 100.0);

		trig_lenth=(long)(nMaxLength - nTrg);
		pre_trig_lenth=(long)(nMaxLength - nPre);
	}

	if(trig_lenth < 0)
	{
		trig_lenth = 0;
	}
	if(pre_trig_lenth < 0)
	{
		pre_trig_lenth = 0;
	}
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR) malloc(m_nSize);
	outBuffer[0]=0x0F;
    outBuffer[1]=0x00;
	outBuffer[2]=0xff&(unsigned char)pre_trig_lenth;                                
	outBuffer[3]=0xff&(unsigned char)(pre_trig_lenth>>8);  
	outBuffer[4]=0xff&(unsigned char)(pre_trig_lenth>>16) ;                                   
	outBuffer[5]=0x00;
	outBuffer[6]=0xff&(unsigned char)trig_lenth;
	outBuffer[7]=0xff&(unsigned char)(trig_lenth>>8);																			
    outBuffer[8]=0xff&(unsigned char)(trig_lenth>>16) ;
	outBuffer[9]= 0x00;
	outBuffer[10]= 0x00;
	outBuffer[11]= 0x00;
    WORD R =  LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
	return R;
}

WORD SetAboutInputChannelAndTrigger(WORD DeviceIndex,PRELAYCONTROL RelayControl)//包括设置触发源和高频抑制
{
	int DelayCurrentState;
    unsigned char Ch1_1S1,Ch1_1S2,Ch2_1S1,Ch2_1S2,Ch3_1S1,Ch3_1S2,Ch4_1S1,Ch4_1S2;//
    unsigned char Trigsel1,Trigsel2,Trigsel3,Trigsel4,FiltCh1,FiltCh2,FiltCh3,FiltCh4;
    unsigned char Ch1_2S1,Ch1_2S2,Ch2_2S1,Ch2_2S2,Ch3_2S1,Ch3_2S2,Ch4_2S1,Ch4_2S2;
    unsigned char Ch1_1d100,Ch1_1d10,Ch2_1d100,Ch2_1d10,Ch3_1d100,Ch3_1d10;
    unsigned char Ch3_ACDC,Ch4_1d100,Ch4_1d10,Ch4_ACDC,ExtTrigControl;//,Relayn;
	unsigned char Ch1_ACDC,Ch2_ACDC;
	PUCHAR outBuffer = NULL;
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

	if(RelayControl->nALT == 1)//交替
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
			//CH1 单通道
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
			//CH2 单通道
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
			//CH3 单通道
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
			//CH4 单通道
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
			if(RelayControl->bCHEnable[0] == TRUE)//如果CH1打开
			{
				Ch1_1S1 = 0x00;
				Ch1_1S2 = 0x00;
				if(RelayControl->bCHEnable[1] == FALSE)//如果CH2关闭，则合并到CH1
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
				if(RelayControl->bCHEnable[1] == TRUE)//如果CH1关闭，CH2打开，则CH1合并到CH2
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
			if(RelayControl->bCHEnable[2] == TRUE)//如果CH3打开
			{
				Ch3_1S1 = 0x00;
				Ch3_1S2 = 0x01;
				if(RelayControl->bCHEnable[3] == FALSE)//如果CH4关闭，则合并到CH3
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
				if(RelayControl->bCHEnable[3] == TRUE)//如果CH3关闭，CH4打开，则CH3合并到CH4
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
//Trigger Source
	if(RelayControl->nTrigSource == 0)//CH1
	{
		Trigsel1 = 0x01;
		Trigsel2 = 0x01;
		Trigsel3 = 0x00;
	}
	else if(RelayControl->nTrigSource == 1)//CH2
	{
		Trigsel1 = 0x00;
		Trigsel2 = 0x00;
		Trigsel3 = 0x00;
	}
	else if(RelayControl->nTrigSource == 2)//CH3
	{
		Trigsel1 = 0x01;
		Trigsel2 = 0x00;
		Trigsel3 = 0x00;
	}
	else if(RelayControl->nTrigSource == 3)//CH4
	{
		Trigsel1 = 0x00;
		Trigsel2 = 0x01;
		Trigsel3 = 0x00;
	}
	else// if(RelayControl->nTrigSource == 4)//EXT(EXT/10)
	{
		Trigsel1 = 0x00;
		Trigsel2 = 0x00;
		Trigsel3 = 0x01;
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
	// 继电器衰减控制 09.12.14 zht
	if(RelayControl->bCHEnable[0] == TRUE)
	{
		if(RelayControl->nCHVoltDIV[0]<3)// AC/DC
		{
			Ch1_1d100=0x00;
			Ch1_1d10=0x00;
		}
		else if(RelayControl->nCHVoltDIV[0]<6)// 100mv/50mv模式
		{
			Ch1_1d100=0x00;
			Ch1_1d10=0x01;
		}
		else// if(RelayControl->nCHVoltDIV[0]<12)// 1v/500mv模式 
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
	//初始化设备名
    UINT m_nSize=2;
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR) malloc(m_nSize);
	outBuffer[0]=0x15;
    outBuffer[1]=0x00;
	LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
/////
    PUCHAR inBuffer = NULL;
	inBuffer=(PUCHAR) malloc(LAN_PACKAGE_SIZE);
	memset(inBuffer, 0,LAN_PACKAGE_SIZE);
	LANRecvDataPackage(DeviceIndex,inBuffer);
    DelayCurrentState=inBuffer[0];
	DelayCurrentState=inBuffer[1]*256+DelayCurrentState;
	free(inBuffer);

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

//=========================test1=============================================================// 

  
long OtherControlData=0;
int DelayNewControlData,DelayDownloadData;
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
	// 这里控制小板	
	DelayNewControlData=0xffff&(((Ch1_1d100&0x01)<<15)|
							((Ch1_1d10&0x01)<<14)|
							((Ch1_ACDC&0x01)<<13)| // 控制第一通道继电器
							((Ch2_1d100&0x01)<<12)
							|((Ch2_1d10&0x01)<<11)|
							((Ch2_ACDC&0x01)<<10)|	// 控制第二通道继电器
							((0x01&0x01)<<9)|
							((0x01&0x01)<<8)|
							// 控制一二通道继电器公共端
							((Ch3_1d100&0x01)<<7)|
							((Ch3_1d10&0x01)<<6)|
							((Ch3_ACDC&0x01)<<5)|	// 控制第三继电器
							((Ch4_1d100&0x01)<<4)|
							((Ch4_1d10&0x01)<<3)|
							((Ch4_ACDC&0x01)<<2)|	// 控制第四继电器
							((ExtTrigControl&0x01)<<1)|
							((0X01&0x01)<<0)					   		// 控制触发继电器和公共端
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
WORD t;										// 继电器数据处理临时变量
WORD Runtemp1,Runtemp2;								// 正常运行继电器状态临时值
unsigned char i=0;												// 局部临时变量，用于记录循环次数
//unsigned char flag;												// 局部临时变量，用于标记继电器数据是否下载完成
unsigned char num=16;// 小板num=16,大板num=13; zht 09/10/29 13:50	
				
#ifdef _LAUNCH	
		//小板
		Runtemp1 = DelayCurrentState;							// 赋值
		Runtemp2 = DelayNewControlData;	
#else
		//大板
		Runtemp1 = DelayCurrentState>>1;							// 赋值 09/10/19 13:51 zht
		Runtemp2 = DelayNewControlData>>1;	
#endif
		t=0x0001;											// 通过其判断如何对继电器操作
								// 逐位比较并根据判断后的值发送到继电器端	
		while( i < num )									//
		{													// (DelayCurrentState & 0x0100 ) == 0 ) 是小板的判断条件
#ifdef _LAUNCH														// (DelayCurrentState & 0x0001 ) == 0 ) 是大板的判断条件 09/10/19 13:52 zht
			//小板
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
			//大板
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
				//==================================数据输出====================================================//
				m_nSize=10; 
				ResetDevice(DeviceIndex);
				outBuffer=(PUCHAR) malloc(m_nSize);
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
				LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
				free(outBuffer);
				//==================================数据输出====================================================//
				
				Sleep(20);								// 系统延时20ms,用于FPGA控制继电器动作
			}
			Runtemp1=Runtemp1 >>1;
			Runtemp2=Runtemp2 >>1;
			i++;
		} // while

//==================================数据输出====================================================//


    m_nSize=10; 
	ResetDevice(DeviceIndex);
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
    WORD R =  LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
	return R;
//==================================数据输出====================================================//	
}

WORD SetTriggerAndSyncOutput(WORD DeviceIndex,WORD nTriggerMode,
							 WORD nTriggerSlope,WORD nPWCondition,ULONG nPW,
							 USHORT nVideoStandard,USHORT nVedioSyncSelect,USHORT nVideoHsyncNumOption,
							 WORD nSync)//设置触发
{                                                                        
	PUCHAR outBuffer = NULL;
    UINT m_nSize=10;

	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR) malloc(m_nSize);
	outBuffer[0]=0x0c;
    outBuffer[1]=0x00;
	/*-------------del by yt 20101023
	outBuffer[2]=0xff&(unsigned char)(((nSync&0x01)<<7)|
									((nPWCondition&0x03)<<5)|
									((nTriggerMode&0x01)<<4)|
									((0x00)<<3)|
									((nTriggerSlope&0x01)<<2)|
									((0x00)<<1)|
									((0x00)<<0));                                 
                               
	outBuffer[3]= 0x00;
	outBuffer[4]= 0xff&(unsigned char)nPW;
	outBuffer[5]= 0xff&(unsigned char)(nPW>>8);																			
    outBuffer[6]= 0x07&(unsigned char)(nPW>>16) ;
    outBuffer[7]= 0x07&(unsigned char)(nPW>>24) ;
	outBuffer[8]= 0x00;
	outBuffer[9]= 0x00;
	*///-----------del by yt 20101023 
	////add by yt 20101023 -- start --
	outBuffer[2]=0xff&(unsigned char)(((nSync&0x01)<<7)|
									((nPWCondition&0x03)<<5)|
									((nTriggerMode&0x03)<<3)|	//00:Edge;01:Pulse;10:Video;11:NULL	// modify by yt 20101023
									((nTriggerSlope&0x01)<<2)|
									((0x00)<<1)|
									((0x00)<<0));
    outBuffer[3]= 0xff&(unsigned char)(((0x00&0x01)<<7)|
									((0x00&0x03)<<5)|
									((0x00&0x01)<<4)|
									((nVedioSyncSelect&0x07)<<1)|	//000:所有行;001:所有场;010:奇数场;011:偶数场;100:指定行;
									((nVideoStandard&0x01)<<0));    //选择工作模式--0:Pal/Secam;1:Ntsc; 

	outBuffer[4]= 0xff&(unsigned char)nPW;
	outBuffer[5]= 0xff&(unsigned char)(nPW>>8);																			
    outBuffer[6]= 0xff&(unsigned char)(nPW>>16);//0x07&(unsigned char)(nPW>>16);	//0x07修改为0xff --- modify by yt 20101023
    outBuffer[7]= 0x07&(unsigned char)(nPW>>24);
	outBuffer[8]= 0xff&(unsigned char)nVideoHsyncNumOption;	//(10 downto 0);--VideoHsyncNumOption
	outBuffer[9]= 0x03&(unsigned char)(nVideoHsyncNumOption>>8);						//(10 downto 0);--VideoHsyncNumOption
	////add by yt 20101023 -- end ---
 	WORD R =  LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
	return R;
}

WORD SetSampleRate(WORD DeviceIndex,WORD nTimeDIV,WORD nYTFormat)
{                                                                
	PUCHAR outBuffer = NULL;
    UINT m_nSize=12;
	long Fenpin;
	WORD M_50M,M_125M,Clk_Alt;

	switch (nTimeDIV)
	{
		case 0:		//5ns -- 100MSa/s
			M_50M=0;
			M_125M=0;
			Clk_Alt=1;
			Fenpin=0xFFFFFFFF;// 无意义，但必须赋给一个大数，否则换档时，速度变慢
			break;
		case 1:		//10ns -- 100MSa/s
			M_50M=0;
			M_125M=0;
			Clk_Alt=1;
			Fenpin=0xFFFFFFFF;// 无意义，但必须赋给一个大数
			break;
		case 2:		//20ns -- 100MSa/s
			M_50M=0;
			M_125M=0;
			Clk_Alt=1;
			Fenpin=0xFFFFFFFF;// 无意义，但必须赋给一个大数
			break;
		case 3:		//50ns -- 100MSa/s
			M_50M=0;
			M_125M=0;
			Clk_Alt=1;
			Fenpin=0xFFFFFFFF;// 无意义，但必须赋给一个大数
			break;
		case 4:		//100ns -- 100MSa/s
			M_50M=0;
			M_125M=0;
			Clk_Alt=1;
			Fenpin=0xFFFFFFFF;// 无意义，但必须赋给一个大数
			break;
		case 5:		//200ns -- 100MSa/s
			M_50M=0;
			M_125M=0;
			Clk_Alt=1;
			Fenpin=0xFFFFFFFF;// 无意义，但必须赋给一个大数
			break;
		case 6:		//500ns -- 100MSa/s
			M_50M=0;
			M_125M=0;
			Clk_Alt=1;
			Fenpin=0xFFFFFFFF;// 无意义，但必须赋给一个大数
			break;
		case 7:		//1us -- 100MSa/s
			M_50M=0;
			M_125M=0;
			Clk_Alt=1;
			Fenpin=0xFFFFFFFF;// 无意义，但必须赋给一个大数
			break;
		case 8:		//2us -- 100MSa/s
			M_50M=0;
			M_125M=0;
			Clk_Alt=1;
			Fenpin=0xFFFFFFFF;// 无意义，但必须赋给一个大数
			break;
		case 9:		//5us -- 100MSa/s
			M_50M=0;
			M_125M=0;
			Clk_Alt=1;
			Fenpin=0xFFFFFFFF;// 无意义，但必须赋给一个大数
			break;
		case 10:	//10us -- 100MSa/s
			M_50M=0;
			M_125M=0;
			Clk_Alt=1;	
			Fenpin=0xFFFFFFFF;// 无意义，但必须赋给一个大数
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
				Fenpin= 0xFFFFCF2D;	//8kSa/s	//开始(采样率提高4倍，每采集128个数据挑出32个数据(每8个挑出1个最大值和1个最小值))
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
				Fenpin= 0xFEA00000;	//5Sa/s--未确定
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
				Fenpin= 0xFEA00000;	//1Sa/s--未确定
			}
			else
			{
				Fenpin= 0xFEA00000;	//4Sa/s--未确定
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
	ResetDevice(DeviceIndex);
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
	WORD R =  LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
	return R;
}

//////

WORD CaptureStart(WORD DeviceIndex)
{  
	PUCHAR outBuffer = NULL;
	UINT m_nSize=2;
	//初始化设备名
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR) malloc(m_nSize);
    outBuffer[0]=0x03;
   	outBuffer[1]=0x00;
 	WORD R =  LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
Sleep(5);
	return R;
}

WORD TriggerEnabled(WORD DeviceIndex)
{
	PUCHAR outBuffer = NULL;
	UINT m_nSize=2;
	//初始化设备名
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR)malloc(m_nSize);
    outBuffer[0]=0x04;
   	outBuffer[1]=0x00;
 	WORD R =  LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
Sleep(5);
	return R;
}

BOOL ForceTrigger(WORD DeviceIndex)
{
	PUCHAR outBuffer = NULL;
	UINT m_nSize=2;

	TriggerEnabled(DeviceIndex);
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR)malloc(m_nSize);
    outBuffer[0]=0x02;
   	outBuffer[1]=0x00;
 	WORD R =  LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
Sleep(5);
	return R;
}

WORD GetCaptureState(WORD DeviceIndex)
{
	PUCHAR outBuffer = NULL;
	PUCHAR inBuffer = NULL;
    UINT m_nSize=2;
	WORD Result;

	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR)malloc(m_nSize);
	outBuffer[0]=0x06;
    outBuffer[1]=0x00;
 	LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
	Sleep(5);
	m_nSize = 512;
	inBuffer=(PUCHAR) malloc(m_nSize);
	memset(inBuffer,0,m_nSize);
	LANRecvDataPackage(DeviceIndex,inBuffer);
	Result = inBuffer[0];
	free(inBuffer);
	Sleep(5);
	return Result;
}
/*
BOOL GetHardInfo(WORD DeviceIndex,UCHAR* pBuffer)
{
	char pcDriverName[MAX_DRIVER_NAME] = "";
	BOOL bResult=FALSE;
	BOOL bDir=DIR_IN;
	UCHAR VendNo=0xE3; //读写EEPROM请求
	HANDLE hDevice;
	ULONG ncnt;
	ncnt=DEV_INFO_LEN;//读取24LC64的长度，最大4K
	USHORT value = 0x1800;// 

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
*/
WORD GetChannelLevel(WORD DeviceIndex,WORD* level,WORD length)
{
	BOOL bResult=FALSE;
	PUCHAR outBuffer = NULL;
	UCHAR pBuffer[8*1024];
	ULONG ncnt,i;
	USHORT value = 0x1400; //读取24LC64的起始地址
	USHORT m_nSize = 2;//6;
	ncnt=length*2;//读取24LC64的长度，最大4K

	outBuffer=(PUCHAR)malloc(m_nSize);
	outBuffer[0]=0x1C;//
    outBuffer[1]=0x00;
/*	outBuffer[2]=0x00;//
    outBuffer[3]=0x14;
	outBuffer[4]=(UCHAR)(ncnt & 0xFF);//
    outBuffer[5]=(UCHAR)((ncnt>>8) & 0xFF);*/
	LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
	//
	bResult = LANRecvDataPackage(DeviceIndex,pBuffer);
	if(bResult==1)
	{
		for(i=0;i<length;i++)
		{
			level[i]=pBuffer[2*i]+(pBuffer[2*i+1]<<8);
		}
	}
	Sleep(5);
	return bResult;
}

WORD SetChannelLevel(WORD DeviceIndex,WORD* level,WORD length)
{
	BOOL bResult=FALSE;
	WORD i;
	USHORT value = 0x1400; //读取24LC64的起始地址
	PUCHAR outBuffer = NULL;
	USHORT m_nSize;
	//初始化设备名
//	m_nSize=6+length*2; //del by yt 20100810
	m_nSize = 256;//add by yt 20100810
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR)malloc(m_nSize);
	memset(outBuffer,0xFF,m_nSize);
	outBuffer[0]=0x1D;
	outBuffer[1]=0x00;
	outBuffer[2]=0x00;
	outBuffer[3]=0x14;
	outBuffer[4]=(UCHAR)(length & 0xFF);
	outBuffer[5]=(UCHAR)(length>>8 & 0xFF);
	for(i=0;i<length;i++)
	{
	   outBuffer[2*i+6]=level[i] & 0xFF;//低8bit
	   outBuffer[2*i+1+6]=(level[i]>>8) & 0xFF;//高8bit
	}
	bResult = LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
Sleep(5);
	return bResult;
}

ULONG GetStartReadAddress(WORD DeviceIndex)
{
	PUCHAR outBuffer = NULL;
	PUCHAR inBuffer = NULL;
    UINT m_nSize;
	ULONG SdramAddrTriged = 0;
	BOOLEAN status=FALSE;
	ULONG BytesReturned=0;

//开始读取 SdramAddrTriged 值.................................................
	ResetDevice(DeviceIndex);
	m_nSize=2;
	outBuffer=(PUCHAR)malloc(m_nSize);
	outBuffer[0]=0x17;//READ SdramAddrTriged
    outBuffer[1]=0x00;
	LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
    Sleep(5);
	
	inBuffer=(PUCHAR)malloc(LAN_PACKAGE_SIZE);
	memset(inBuffer,0,m_nSize);
	LANRecvDataPackage(DeviceIndex,inBuffer);
	//保存数据(触发点SdramAddrTriged)
	SdramAddrTriged=inBuffer[2];
	SdramAddrTriged=SdramAddrTriged*256*256;
	SdramAddrTriged=SdramAddrTriged+inBuffer[1]*256+inBuffer[0];
	free(inBuffer);//释放缓冲区
Sleep(5);
//结束读取 SdramAddrTriged 值.................................................
	return SdramAddrTriged;
}

int SetReadAddress(WORD DeviceIndex,ULONG RamReadStartAddr)
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize;
	int status;
	ULONG BytesReturned=0;

	//初始化设备名
	m_nSize=8;
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR)malloc(m_nSize);
	outBuffer[0]=0x13;
	outBuffer[1]=0x00;
	//设置数据读取起始地址
	RamReadStartAddr = RamReadStartAddr - 1;
	outBuffer[2]=0xff&(unsigned char)RamReadStartAddr;                         
	outBuffer[3]=0xff&(unsigned char)(RamReadStartAddr>>8);
	outBuffer[4]=0xff&(unsigned char)(RamReadStartAddr>>16);                           
	outBuffer[5]=0xff&(unsigned char)(RamReadStartAddr>>24);
	outBuffer[6]=(0xff&(unsigned char)(RamReadStartAddr>>32));//SdramRdBegin
	outBuffer[7]=0x80;
	status = LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
Sleep(5);
	return status;
}

int SetReadLength(WORD DeviceIndex,ULONG RamReadLength)
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize;
	int status;
	ULONG BytesReturned=0;
	//初始化设备名
	m_nSize=8;
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR)malloc(m_nSize);
	outBuffer[0]=0x1B;
	outBuffer[1]=0x00;
	//设置数据长度
	outBuffer[2]=0xff&(unsigned char)RamReadLength;                         
	outBuffer[3]=0xff&(unsigned char)(RamReadLength>>8);   
	outBuffer[4]=0xff&(unsigned char)(RamReadLength>>16);                           
	outBuffer[5]=0xff&(unsigned char)(RamReadLength>>24);
	outBuffer[6]=0xff&(unsigned char)(RamReadLength>>32);
	outBuffer[7]=0x00;
	status = LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
Sleep(5);
	return status;
}

int OpenConnetMode(WORD DeviceIndex,WORD nMode)
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize;
	int status;
	ULONG BytesReturned=0;
	//初始化设备名
	m_nSize=2;
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR)malloc(m_nSize);
	if(nMode == USB)
	{
		outBuffer[0]=0x22;
	}
	else if(nMode == NET_LAN)
	{
		outBuffer[0]=0x21;
	}
	else if(nMode == NET_WIFI)
	{
		outBuffer[0]=0x28;
	}
	else
	{
		outBuffer[0]=0x22;
	}
	outBuffer[1]=0x00;
	status = LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
	return status;
}

int ResetWIFI(WORD DeviceIndex)//只有在USB/WIFI下才能复位WIFI
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize;
	int status;
	ULONG BytesReturned=0;
	//初始化设备名
	m_nSize=2;
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR)malloc(m_nSize);
	outBuffer[0]=0x2A;
	outBuffer[1]=0x00;
	status = LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
	return status;
}

WORD ReadHardData(WORD DeviceIndex,WORD* CH1Data, WORD* CH2Data,WORD* CH3Data, WORD* CH4Data,ULONG nReadLen)
{
	ULONG i,j;
	PUCHAR outBuffer = NULL,inBuffer = NULL;
	BOOLEAN status=FALSE;
	ULONG BytesReturned=0;
    UINT m_nSize;
	ULONG nReadTimes = 0;

//准备读取数据
	//初始化设备名
	ResetDevice(DeviceIndex);
	m_nSize=2;
	outBuffer=(PUCHAR)malloc(m_nSize);
	outBuffer[0]=0x05; 
	outBuffer[1]=0x00;
	LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
Sleep(5);
	free(outBuffer);

//开始读取数据
	m_nSize = LAN_PACKAGE_SIZE;
	inBuffer=(PUCHAR)malloc(m_nSize);
	memset(inBuffer, 0,m_nSize);
	nReadTimes = nReadLen / m_nSize;
	USHORT nStep = (m_nSize / MAX_CH_NUM);
	
	for(i=0;i<nReadTimes;i++) //多次循环读数
	{
		status = LANRecvDataPackage(DeviceIndex,inBuffer);
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
	free(inBuffer);//释放缓冲区
	
	return status;
}
//读取扫描模式下预触发长度数据
WORD GetPreTrigData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl/*,USHORT nFlag*/)
{
	WORD nRE = 0;
	WORD* pCHData[MAX_CH_NUM];
	ULONG RamReadStartAddr, nTemp;
	ULONG RamReadLength,SDRamAddrTriggerd;
	ULONG i = 0,nStartOffset = 0;
	ULONG nDataLen = 0;
	if(pControl->nTimeDIV >= MIN_ROLL_TIMEDIV)
	{
		nDataLen = ULONG(pControl->nHTriggerPos * pControl->nReadDataLen/100.0 * 4);
	}
	else
	{
		nDataLen = ULONG(pControl->nHTriggerPos * pControl->nReadDataLen/100.0);
	}
//获取触发地址
	SDRamAddrTriggerd = GetStartReadAddress(nDeviceIndex);
//根据触发地址计算起始读数地址
	nTemp = ULONG(SDRamAddrTriggerd - nDataLen);
	nStartOffset = ULONG(nTemp % LAN_PACKAGE_SIZE);
	nTemp = nTemp - nStartOffset;
	if(nTemp < 0)
	{
		RamReadStartAddr = 16777216 + nTemp;
	}
	else
	{
		RamReadStartAddr = nTemp;
	}
//设置读数地址
	SetReadAddress(nDeviceIndex,RamReadStartAddr);
#ifdef TRIGGER_POS_OFFSET
	RamReadLength =  (nDataLen + LAN_PACKAGE_SIZE + 512);
#else
	RamReadLength =  (nDataLen + nStartOffset);
#endif
//设置读数长度
	if(RamReadLength > LAN_PACKAGE_SIZE)
	{
		RamReadLength = RamReadLength - (RamReadLength % LAN_PACKAGE_SIZE);
	}
	else
	{
		RamReadLength = LAN_PACKAGE_SIZE;
	}
	SetReadLength(nDeviceIndex,2*RamReadLength);
//读数据
	for(i=0;i<MAX_CH_NUM;i++)
	{
		pCHData[i] = new WORD[RamReadLength];
	}
	nRE = ReadHardData(nDeviceIndex,pCHData[0],pCHData[1],pCHData[2],pCHData[3],4*RamReadLength);
	if(nRE != 0)
	{
	//输出数据
	#ifdef TRIGGER_POS_OFFSET
		nStartOffset = 512 + nStartOffset;//纠正触发点位置
	#endif
		pControl->nAlreadyReadLen = RamReadLength - nStartOffset;
		if(pControl->nTimeDIV >= MIN_ROLL_TIMEDIV)
		{
			for(i=0;i<pControl->nAlreadyReadLen;i++)
			{
				*(pCHData[0]+i) = *(pCHData[0]+nStartOffset+i);
				*(pCHData[1]+i) = *(pCHData[1]+nStartOffset+i);
				*(pCHData[2]+i) = *(pCHData[2]+nStartOffset+i);
				*(pCHData[3]+i) = *(pCHData[3]+nStartOffset+i);
			}
			nDataLen = pControl->nAlreadyReadLen;
			pControl->nAlreadyReadLen /= 4;//之前采样率提高了4倍，所以长度缩小4倍
			if(pControl->nAlreadyReadLen > pControl->nReadDataLen)
			{
				pControl->nAlreadyReadLen = pControl->nReadDataLen;
			}
			dsoSFChooseData(pCHData[0],nDataLen,8,pCH1Data,pControl->nAlreadyReadLen);
			dsoSFChooseData(pCHData[1],nDataLen,8,pCH2Data,pControl->nAlreadyReadLen);
			dsoSFChooseData(pCHData[2],nDataLen,8,pCH3Data,pControl->nAlreadyReadLen);
			dsoSFChooseData(pCHData[3],nDataLen,8,pCH4Data,pControl->nAlreadyReadLen);
		}
		else
		{
			if(pControl->nAlreadyReadLen > pControl->nReadDataLen)
			{
				pControl->nAlreadyReadLen = pControl->nReadDataLen;
			}
			for(i=0;i<pControl->nAlreadyReadLen;i++)
			{
				*(pCH1Data+i) = *(pCHData[0]+nStartOffset+i);
				*(pCH2Data+i) = *(pCHData[1]+nStartOffset+i);
				*(pCH3Data+i) = *(pCHData[2]+nStartOffset+i);
				*(pCH4Data+i) = *(pCHData[3]+nStartOffset+i);
			}

		}
	}
	for(i=0;i<MAX_CH_NUM;i++)
	{
		delete pCHData[i];
	}
	return nRE;
}

#define CONTINUE_LEN_128		128
#define DATA_STEP_32			32
WORD GetDataContinue(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl,WORD nScanRoll)
{
	ULONG i=0;
	WORD nRE = 0;
	ULONG nPreLen = pControl->nAlreadyReadLen;
	WORD pCHData[MAX_CH_NUM][CONTINUE_LEN_128];
	
	if(nScanRoll == 1 && pControl->nAlreadyReadLen == 0)//Roll第1次读，本次数据作废
	{
		WORD pCHData1[MAX_CH_NUM][1024];
		nRE = ReadHardData(nDeviceIndex,pCHData1[0],pCHData1[1],pCHData1[2],pCHData1[3],4*1024);
		pControl->nAlreadyReadLen = 1;//在此 = 1，用以区别是否是第1次Roll读取数据
		return 0;
	}
	else
	{
	//设置读数长度
		SetReadLength(nDeviceIndex,256);
		nRE = ReadHardData(nDeviceIndex,pCHData[0],pCHData[1],pCHData[2],pCHData[3],512);
	}
	if(nRE != 0)
	{
		if(nScanRoll == 0)//Scan
		{
			if(pControl->nTimeDIV < MIN_ROLL_TIMEDIV)
			{
				if((nPreLen + CONTINUE_LEN_128) > pControl->nReadDataLen)
				{
					pControl->nAlreadyReadLen = pControl->nReadDataLen;
					for(i=0;i<pControl->nAlreadyReadLen-nPreLen;i++)
					{
						*(pCH1Data+i) = pCHData[0][i];
						*(pCH2Data+i) = pCHData[1][i];
						*(pCH3Data+i) = pCHData[2][i];
						*(pCH4Data+i) = pCHData[3][i];
					}
					return nRE;
				}
				else
				{
					pControl->nAlreadyReadLen += CONTINUE_LEN_128;
				}
				for(i=0;i<CONTINUE_LEN_128;i++)
				{
					*(pCH1Data+i) = pCHData[0][i];
					*(pCH2Data+i) = pCHData[1][i];
					*(pCH3Data+i) = pCHData[2][i];
					*(pCH4Data+i) = pCHData[3][i];
				}
			}
			else
			{
				WORD* pData[MAX_CH_NUM];
				pData[0] = pCH1Data;
				pData[1] = pCH2Data;
				pData[2] = pCH3Data;
				pData[3] = pCH4Data;
				if((nPreLen+DATA_STEP_32) >= pControl->nReadDataLen)//已满
				{
					for(i=0;i<MAX_CH_NUM;i++)
					{
						dsoSFChooseData(pCHData[i], pControl->nReadDataLen-nPreLen,8,pData[i],(pControl->nReadDataLen-nPreLen));
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
			ULONG i=0;
			WORD* pData[MAX_CH_NUM];

			pData[0] = pCH1Data;
			pData[1] = pCH2Data;
			pData[2] = pCH3Data;
			pData[3] = pCH4Data;

			for(i=0;i<MAX_CH_NUM;i++)
			{
				dsoSFChooseData(pCHData[i],CONTINUE_LEN_128,8,pData[i],DATA_STEP_32);
			}
		}
	}
	return nRE;
}

WORD SetRollCollect(WORD DeviceIndex,WORD nEnable)//设置Roll采集模式						    
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize=4;

	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR)malloc(m_nSize);
	outBuffer[0]=0x19;
   	outBuffer[1]=0x00;
    outBuffer[2]=(nEnable & 0x01); 
	outBuffer[3]=0x00; 
 	
	int nR = LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);	//关闭设备

    return nR;
}

WORD SetPeakMode(WORD DeviceIndex,WORD nOpen,ULONG nPeak)//				    
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize = 10;
	
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR)malloc(m_nSize);
	outBuffer[0]= 0x1A;
   	outBuffer[1]= 0x00;
	outBuffer[2]= 0xff&(unsigned char)(nPeak);//低8位
   	outBuffer[3]= 0xff&(unsigned char)(nPeak>>8);//
	outBuffer[4]= 0xff&(unsigned char)(nPeak>>16);//
	outBuffer[5]= 0xff&(unsigned char)(nPeak>>24);//
    outBuffer[6]= (nOpen & 0x01); 
	outBuffer[7]= 0x00;
	outBuffer[8]= 0x00;
	outBuffer[9]= 0x00;
	int nR = LANSendCtrl(DeviceIndex,outBuffer,m_nSize);  
	free(outBuffer);	
    return nR;
}

WORD GetRestartLANStatus(WORD DeviceIndex)//获取设备LAN是否启动完毕
{
    UINT m_nSize;
	PUCHAR inBuffer = NULL;
	PUCHAR outBuffer = NULL;
	UCHAR nRe = 2;
//	FILE *fp;

	ResetDevice(DeviceIndex);
	m_nSize=2;
	outBuffer=(PUCHAR)malloc(m_nSize);
	outBuffer[0]=0x24;
	outBuffer[1]=0x00;
	LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
//
	m_nSize = LAN_PACKAGE_SIZE;
	inBuffer=(PUCHAR) malloc(m_nSize);
	memset(inBuffer, 0,m_nSize);
	int nReadTimes = LAN_PACKAGE_SIZE / m_nSize;//最少读取512字节
	int status = 0;

	for(int i=0;i<nReadTimes;i++)
	{
		status = LANRecvDataPackage(DeviceIndex,inBuffer);
		if(status == 0)
		{
			nRe = 0;//add by yt 20111121
			break;
		}
		if(i == 0)
		{
			nRe = inBuffer[1];// 0:失败；1：成功
		}
	}
	free(inBuffer);//释放缓冲区
	
	return nRe;
}

ULONG GetHardFandC(WORD DeviceIndex)//频率计/计数器
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize=2;
	ULONG nFreq = 0,nTCycle = 0;

	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR) malloc(m_nSize);
	outBuffer[0]=0x0A;
    outBuffer[1]=0x00;
 	LANSendCtrl(DeviceIndex,outBuffer,m_nSize);

	free(outBuffer);
	m_nSize = LAN_PACKAGE_SIZE;
    PUCHAR inBuffer = NULL;
	inBuffer=(PUCHAR) malloc(m_nSize);
	memset(inBuffer, 0,m_nSize);
	int nReadTimes = LAN_PACKAGE_SIZE / m_nSize;//最少读取512字节
	int status = 0;
	//起始时间
	for(int i=0;i<nReadTimes;i++)
	{
		status = LANRecvDataPackage(DeviceIndex,inBuffer);
		if(status == 0)
		{
			break;
		}
		if(i==0)
		{
		//保存数据
			nFreq = inBuffer[0] + (inBuffer[1]<<8) + (inBuffer[2]<<16) + (inBuffer[3]<<24);
		//	nTCycle = inBuffer[4] + (inBuffer[5]<<8)+(inBuffer[6]<<16) + (inBuffer[7]<<24);
		}
	}
    //关闭设备
	free(inBuffer);//释放缓冲区
	
	return nFreq*2;
}

WORD GetFPGAVersion(WORD DeviceIndex)//
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize=2;
	WORD nVersion = 0;

	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR) malloc(m_nSize);
	outBuffer[0]=0x2B;
    outBuffer[1]=0x00;
 	LANSendCtrl(DeviceIndex,outBuffer,m_nSize);

	free(outBuffer);
	m_nSize = LAN_PACKAGE_SIZE;
    PUCHAR inBuffer = NULL;
	inBuffer=(PUCHAR) malloc(m_nSize);
	memset(inBuffer, 0,m_nSize);
	int nReadTimes = LAN_PACKAGE_SIZE / m_nSize;//最少读取512字节
	int status = 0;
	//起始时间
	for(int i=0;i<nReadTimes;i++)
	{
		status = LANRecvDataPackage(DeviceIndex,inBuffer);
		if(status == 0)
		{
			break;
		}
		if(i==0)
		{
		//保存数据
			nVersion = inBuffer[62] + inBuffer[63]*256;
		}
	}
    //关闭设备
	free(inBuffer);//释放缓冲区
	
	return nVersion;
}

WORD SetHardFandC(WORD DeviceIndex,WORD nType)
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize=4;

	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR)malloc(m_nSize);
	outBuffer[0]=0x10;
    outBuffer[1]=0x00;
	outBuffer[2]=(nType & 0xFF);                      
	outBuffer[3]=0x00;  
 	int nRe = LANSendCtrl(DeviceIndex,outBuffer,m_nSize); 
	free(outBuffer);
    return nRe;
}


WORD ResetCounter(WORD DeviceIndex,WORD nValue)
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize=4;

	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR) malloc(m_nSize);
	outBuffer[0]=0x10;
    outBuffer[1]=0x00;
	outBuffer[2]=(nValue & 0xFF);//复位                      
	outBuffer[3]=0x00;  
 	int nRe = LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
    return nRe;
}

ULONG GetHardVersion(WORD DeviceIndex)
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize=2;
	ULONG nVersion = 0;

//初始化设备名
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR) malloc(m_nSize);
	outBuffer[0]=0x09;
    outBuffer[1]=0x00;
 	LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);

    PUCHAR inBuffer = NULL;
	m_nSize = LAN_PACKAGE_SIZE;
	inBuffer=(PUCHAR) malloc(m_nSize);
	memset(inBuffer, 0,m_nSize);
	int nReadTimes = LAN_PACKAGE_SIZE / m_nSize;//最少读取512字节
	int nRe = 0;
	//起始时间
	for(int i=0;i<nReadTimes;i++)
	{
		nRe = LANRecvDataPackage(DeviceIndex,inBuffer);
		if(nRe == 0)
		{
			break;
		}
		if(i==0)
		{
			nVersion = inBuffer[2] + (inBuffer[3]<<8);
		}
	}
    //关闭设备
	free(inBuffer);//释放缓冲区
	
	return nVersion;
}

WORD LANGetLanEnable(WORD DeviceIndex,short* pEnable)
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize=1;
	ULONG nVersion = 0;

//初始化设备名
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR) malloc(m_nSize);
	outBuffer[0]=0x23;
    outBuffer[1]=0x00;
 	LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);

    PUCHAR inBuffer = NULL;
	m_nSize = LAN_PACKAGE_SIZE;
	inBuffer=(PUCHAR) malloc(m_nSize);
	memset(inBuffer, 0,m_nSize);
	int nReadTimes = LAN_PACKAGE_SIZE / m_nSize;//最少读取512字节
	int nRe = 0;
	//起始时间
	for(int i=0;i<nReadTimes;i++)
	{
		nRe = LANRecvDataPackage(DeviceIndex,inBuffer);
		if(nRe == 0)
		{
			break;
		}
		if(i==0)
		{
			*pEnable = inBuffer[0];
		}
	}
    //关闭设备
	free(inBuffer);//释放缓冲区
	
	return nRe;
}

WORD LANGetIP(WORD DeviceIndex,ULONG* pIP,ULONG* pSubMask,ULONG* pGateway,USHORT* pPort,BYTE* pMac)
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize=2;
	ULONG nVersion = 0;

//初始化设备名
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR) malloc(m_nSize);
	outBuffer[0]=0x23;
    outBuffer[1]=0x00;
 	LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);

    PUCHAR inBuffer = NULL;
	m_nSize = LAN_PACKAGE_SIZE;
	inBuffer=(PUCHAR) malloc(m_nSize);
	memset(inBuffer, 0,m_nSize);
	int nReadTimes = LAN_PACKAGE_SIZE / m_nSize;//最少读取512字节
	int nRe = 0;
	//起始时间
	for(int i=0;i<nReadTimes;i++)
	{
		nRe = LANRecvDataPackage(DeviceIndex,inBuffer);
		if(nRe == 0)
		{
			break;
		}
		if(i==0)
		{
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

		}
	}
    //关闭设备
	free(inBuffer);//释放缓冲区
	
	return nRe;
}

WORD SetRamLength(WORD DeviceIndex,WORD nBufferSize)//						    
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize=4;

	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR) malloc(m_nSize);
	outBuffer[0]=0x0D;
   	outBuffer[1]=0x0F;//NOUSE 16bit usb bus
    outBuffer[2]=0xff&(nBufferSize&0x07); 
	outBuffer[3]=0x00;
	int nRe = LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);	//关闭设备

    return nRe;
}

WORD InitSDRam(WORD DeviceIndex)
{  
	PUCHAR outBuffer = NULL;
	UINT m_nSize=2;
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR)malloc(m_nSize);
    outBuffer[0]=0x18;
   	outBuffer[1]=0x00;
 	int nRe = LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);
	return nRe;
}


/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
//Export...

//设置CH Lever Pos
DLL_API WORD WINAPI dsoLANSetCHPos(WORD nDeviceIndex,WORD* pLevel,WORD nVoltDIV,WORD nPos,WORD nCH)
{
	nPos = WORD((pLevel[nCH*CAL_VOLTDIV_NUM + 2*nVoltDIV+1] - pLevel[nCH*CAL_VOLTDIV_NUM + 2*nVoltDIV])*1.0/MAX_DATA*nPos + pLevel[nCH*CAL_VOLTDIV_NUM + 2*nVoltDIV]);
	return SetOffsetChannel(nDeviceIndex,nPos,nCH);
}

//设置Trigger Level
DLL_API WORD WINAPI dsoLANSetVTriggerLevel(WORD nDeviceIndex,WORD* pLevel,WORD nPos)
{
	nPos = USHORT(1012.0/MAX_DATA*nPos);
	return SetTrigLevel(nDeviceIndex,nPos);
}

//设置触发长度和预触发长度(包括Trigger水平位置)-->参数nBufferLen 必须是512的整数倍数,且不能>16M
DLL_API WORD WINAPI dsoLANSetHTriggerLength(WORD nDeviceIndex,ULONG nBufferLen,WORD HTriggerPos,WORD nTimeDIV,WORD nYTFormat)
{
	return SetTriggerLength(nDeviceIndex,nBufferLen,HTriggerPos,nTimeDIV,nYTFormat);
}

//设置CH和Trigger Source and Trigger Filt
DLL_API WORD WINAPI dsoLANSetCHAndTrigger(WORD nDeviceIndex,RELAYCONTROL RelayControl)
{
	return SetAboutInputChannelAndTrigger(nDeviceIndex,&RelayControl);
}
DLL_API WORD WINAPI dsoLANSetCHAndTriggerVB(WORD nDeviceIndex,WORD* pCHEnable,WORD* pCHVoltDIV,WORD* pCHCoupling,WORD* pCHBWLimit,WORD nTriggerSource,WORD nTriggerFilt,WORD nALT)
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
	return dsoLANSetCHAndTrigger(nDeviceIndex,RelayControl);
}

//设置Trigger和同步输出
DLL_API WORD WINAPI dsoLANSetTriggerAndSyncOutput(WORD nDeviceIndex,WORD nTriggerMode,WORD nTriggerSlope,WORD nPWCondition,ULONG nPW,USHORT nVideoStandard,USHORT nVedioSyncSelect,USHORT nVideoHsyncNumOption,WORD nSync)
{
	return SetTriggerAndSyncOutput(nDeviceIndex,nTriggerMode,nTriggerSlope,nPWCondition,nPW,nVideoStandard,nVedioSyncSelect,nVideoHsyncNumOption,nSync);
}

//设置采样率
DLL_API WORD WINAPI dsoLANSetSampleRate(WORD nDeviceIndex,WORD nTimeDIV,WORD nYTFormat)
{
	return SetSampleRate(nDeviceIndex,nTimeDIV,nYTFormat);
}

//启动采集
DLL_API WORD WINAPI dsoLANStartCollectData(WORD nDeviceIndex)
{
	return CaptureStart(nDeviceIndex);
}

//启动触发
DLL_API WORD WINAPI dsoLANStartTrigger(WORD nDeviceIndex)
{
	return TriggerEnabled(nDeviceIndex);
}

//强制触发
DLL_API WORD WINAPI dsoLANForceTrigger(WORD nDeviceIndex)
{
	return ForceTrigger(nDeviceIndex);
}

//获取地址
DLL_API WORD WINAPI dsoLANGetState(WORD nDeviceIndex)
{
	WORD nState = GetCaptureState(nDeviceIndex);
	nState &= 0x07;//保留低3位
	return nState;
}

//获取是否已采集满512Bytes
DLL_API WORD WINAPI dsoLANGetPackState(WORD nDeviceIndex)
{
	WORD nState = GetCaptureState(nDeviceIndex);
	nState = (nState>>3) & 0x01;//取出第3位//是否满512Byte标志.0:未满,1:已满
	return nState;
}


//读取校对电平数据
DLL_API WORD WINAPI dsoLANReadCalibrationData(WORD nDeviceIndex,WORD* pLevel,WORD nLen)
{
	return GetChannelLevel(nDeviceIndex,pLevel,nLen);
}

//写入校对电平数据
DLL_API WORD WINAPI dsoLANWriteCalibrationData(WORD nDeviceIndex,WORD* pLevel,WORD nLen)
{
	return SetChannelLevel(nDeviceIndex,pLevel,nLen);
}

//读取YT-Normal数据
DLL_API WORD WINAPI dsoLANGetData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl/*,ULONG* p1,ULONG* p2,ULONG* p3*/)
{
	WORD nRE = 0;
	WORD* pCHData[MAX_CH_NUM];
	ULONG RamReadStartAddr, nTemp;
	ULONG RamReadLength,SDRamAddrTriggerd;
	ULONG i = 0,nStartOffset = 0;
	ULONG nDataLen = pControl->nReadDataLen;

//获取触发地址
	SDRamAddrTriggerd = GetStartReadAddress(nDeviceIndex);
//	*p1 = SDRamAddrTriggerd;
//根据触发地址计算起始读数地址
	nTemp = ULONG(SDRamAddrTriggerd - (pControl->nHTriggerPos * nDataLen/100.0));
	nStartOffset = ULONG(nTemp % LAN_PACKAGE_SIZE);
	nTemp = nTemp - nStartOffset;
	if(nTemp < 0)
	{
		RamReadStartAddr = 16777216 + nTemp;
	}
	else
	{
		RamReadStartAddr = nTemp;
	}
//设置读数地址
	SetReadAddress(nDeviceIndex,RamReadStartAddr);
//	*p2 = RamReadStartAddr;
#ifdef TRIGGER_POS_OFFSET
	RamReadLength = 2 * (nDataLen + LAN_PACKAGE_SIZE*2);
#else
	RamReadLength = 2 * (nDataLen + LAN_PACKAGE_SIZE);
#endif
//设置读数长度
	SetReadLength(nDeviceIndex,RamReadLength);
//	*p3 = RamReadLength;
//读数据
	for(i=0;i<MAX_CH_NUM;i++)
	{
#ifdef TRIGGER_POS_OFFSET
		pCHData[i] = new WORD[nDataLen+LAN_PACKAGE_SIZE*2];
#else
		pCHData[i] = new WORD[nDataLen+LAN_PACKAGE_SIZE];
#endif
	}
	nRE = ReadHardData(nDeviceIndex,pCHData[0],pCHData[1],pCHData[2],pCHData[3],2*RamReadLength);
//输出数据
#ifdef TRIGGER_POS_OFFSET
	nStartOffset = LAN_PACKAGE_SIZE + nStartOffset;//纠正触发点位置
#endif
	for(i=0;i<nDataLen;i++)
	{
		*(pCHData[0]+i) = *(pCHData[0]+nStartOffset+i);
		*(pCHData[1]+i) = *(pCHData[1]+nStartOffset+i);
		*(pCHData[2]+i) = *(pCHData[2]+nStartOffset+i);
		*(pCHData[3]+i) = *(pCHData[3]+nStartOffset+i);
	}
	if(pControl->nALT == 1)//交替
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
//				dsoSFProcessALTData(pCH4Data,pCHData[3],pCHData[2],nDataLen,pControl->nHTriggerPos,0);
//#else
				dsoSFProcessALTData(pCH4Data,pCHData[2],pCHData[3],nDataLen,pControl->nHTriggerPos,0);
//#endif	
			}
		}
	}
	else//非交替
	{
		/*************************此段暂时无用
		//只有1个CH打开时
		if(pControl->nCHSet == 0x01 && pControl->nTimeDIV <= MAX_INSERT_TIMEDIV)
		{
			dsoSFProcessALTData4CH(pCH1Data,pCHData[0],pCHData[2],pCHData[1],pCHData[3],nDataLen,pControl->nHTriggerPos,0);
		}
		else if(pControl->nCHSet == 0x02 && pControl->nTimeDIV <= MAX_INSERT_TIMEDIV)
		{
			dsoSFProcessALTData4CH(pCH2Data,pCHData[0],pCHData[1],pCHData[2],pCHData[3],nDataLen,pControl->nHTriggerPos,0);
		}
		else if(pControl->nCHSet == 0x04 && pControl->nTimeDIV <= MAX_INSERT_TIMEDIV)
		{
			dsoSFProcessALTData4CH(pCH3Data,pCHData[0],pCHData[1],pCHData[2],pCHData[3],nDataLen,pControl->nHTriggerPos,0);
		}
		else if(pControl->nCHSet == 0x08 && pControl->nTimeDIV <= MAX_INSERT_TIMEDIV)
		{
			dsoSFProcessALTData4CH(pCH4Data,pCHData[0],pCHData[1],pCHData[2],pCHData[3],nDataLen,pControl->nHTriggerPos,0);
		}
		else
		此段暂时无用*************************/
		{
		//CH1+CH2
			if((pControl->nCHSet & 0x03) != 0x03)//单通道
			{
				if(((pControl->nCHSet >> 0) & 0x01) == 0x01)//CH1打开,CH2关闭
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
				else//CH2打开,CH1关闭
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
			else//双通道
			{
				for(i=0;i<nDataLen;i++)
				{
					*(pCH1Data+i) = *(pCHData[0]+i);
					*(pCH2Data+i) = *(pCHData[1]+i);
				}
			}
		//CH3+CH4
			if(((pControl->nCHSet>>2) & 0x03) != 0x03)//单通道
			{
				if(((pControl->nCHSet >> 2) & 0x01) == 0x01)//CH3打开,CH4关闭
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
				else//CH4打开,CH3关闭
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
			else//双通道
			{
				for(i=0;i<nDataLen;i++)
				{
					*(pCH3Data+i) = *(pCHData[2]+i);
					*(pCH4Data+i) = *(pCHData[3]+i);
				}
			}
		}
	}
	for(i=0;i<MAX_CH_NUM;i++)
	{
		delete pCHData[i];
	}
	return nRE;
}

//读取YT-Scan数据
DLL_API WORD WINAPI dsoLANGetScanData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl)
{
	WORD nRe = 0;
	if(pControl->nAlreadyReadLen == 0)//第1次读取
	{
		nRe = GetPreTrigData(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,pControl/*,0*/);//0:Scan
	}
	else
	{
		nRe = GetDataContinue(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,pControl,0);//连续读取
	}
	return nRe;
}

//读取Roll数据
DLL_API WORD WINAPI dsoLANGetRollData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl)
{
	WORD nRe = 0;
//	if(pControl->nAlreadyReadLen == 0)//第1次读取
//	{
//		nRe = GetPreTrigData(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,pControl/*,1*/);//1:Roll
//	}
//	else
//	{
		nRe = GetDataContinue(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,pControl,1);//连续读取
//	}
	return nRe;
}


DLL_API WORD WINAPI dsoLANOpenRollMode(WORD nDeviceIndex)
{
	return SetRollCollect(nDeviceIndex,0x01);
}

DLL_API WORD WINAPI dsoLANCloseRollMode(WORD nDeviceIndex)
{
	return SetRollCollect(nDeviceIndex,0x00);
}

#define MAX_PEAK	0x0FFFFFFF
DLL_API WORD WINAPI dsoLANSetPeakDetect(WORD nDeviceIndex,WORD nTimeDIV,WORD nYTFormat)
{
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
	if(nTimeDIV >= MIN_ROLL_TIMEDIV)//采样率提高4倍
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

DLL_API WORD WINAPI dsoLANClosePeakDetect(WORD nDeviceIndex)
{
	return SetPeakMode(nDeviceIndex,0x00,0x00);
}

DLL_API ULONG WINAPI dsoLANGetHardFC(WORD nDeviceIndex)
{
	return GetHardFandC(nDeviceIndex);
}

DLL_API WORD WINAPI dsoLANSetHardFC(WORD nDeviceIndex,WORD nType)
{
	return SetHardFandC(nDeviceIndex,nType);
}

DLL_API WORD WINAPI dsoLANResetCnter(WORD nDeviceIndex)
{
	ResetCounter(nDeviceIndex,0x03);
	return ResetCounter(nDeviceIndex,0x01);
}

DLL_API WORD WINAPI dsoLANStartRoll(WORD nDeviceIndex)
{
	ULONG RamReadLength,SDRamAddrTriggerd,RamReadStartAddr,nTemp;
	ULONG nStartOffset = 0;

//获取触发地址
	SDRamAddrTriggerd = GetStartReadAddress(nDeviceIndex);
//根据触发地址计算起始读数地址
	nTemp = ULONG(SDRamAddrTriggerd);
	nStartOffset = ULONG(nTemp % LAN_PACKAGE_SIZE);
	nTemp = nTemp - nStartOffset;
	if(nTemp < 0)
	{
		RamReadStartAddr = 16777216 + nTemp;
	}
	else
	{
		RamReadStartAddr = nTemp;
	}
//设置读数地址
	SetReadAddress(nDeviceIndex,RamReadStartAddr);
#ifdef TRIGGER_POS_OFFSET
	RamReadLength =  (LAN_PACKAGE_SIZE + 512);
#else
	RamReadLength =  LAN_PACKAGE_SIZE;
#endif
//设置读数长度
	int nRe = SetReadLength(nDeviceIndex,2*RamReadLength);
	return nRe;
}

//读取硬件版本
DLL_API ULONG WINAPI dsoLANGetHardVersion(WORD nDeviceIndex)
{
	return GetHardVersion(nDeviceIndex);
}

//此函数目前无效，无需调用
DLL_API WORD WINAPI dsoLANSetBufferSize(WORD nDeviceIndex,WORD nBufferSize)
{
	return SetRamLength(nDeviceIndex,nBufferSize);
}

//初始化SDRam
DLL_API WORD WINAPI dsoLANInitSDRam(WORD nDeviceIndex)
{
	return InitSDRam(nDeviceIndex);
}

//判断SDRam是否初始化成功
DLL_API WORD WINAPI dsoLANGetSDRamInit(WORD nDeviceIndex)
{
	WORD nState = GetCaptureState(nDeviceIndex);
	nState = (nState>>4) & 0x01;//取出第4位//是否初始化成功.0:失败,1:成功
	return nState;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*Soft Functions**************************************************************************************************************************/
//SDK读取数据
WORD SDGetData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl,WORD nInsertMode)
{
	WORD nRe = 0;
	if(pControl->nTimeDIV > MAX_INSERT_TIMEDIV)//不需要差值
	{
		nRe = dsoLANGetData(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,pControl);
	}
	else//需要差值的档位
	{
		int i = 0;
		ULONG nState[MAX_CH_NUM];
		WORD* CHData[MAX_CH_NUM];
		USHORT* nData[MAX_CH_NUM];
		CHData[0] = pCH1Data;
		CHData[1] = pCH2Data;
		CHData[2] = pCH3Data;
		CHData[3] = pCH4Data;
		for(i=0;i<MAX_CH_NUM;i++)
		{
			nData[i] = new USHORT[pControl->nReadDataLen - 10240 + 40960];
		}
		//读数
		nRe = dsoLANGetData(nDeviceIndex,nData[0],nData[1],nData[2],nData[3],pControl);
		//差值
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
				dsoSFInsert(nData[pControl->nTriggerSource],CHData[pControl->nTriggerSource],pControl,nInsertMode,pControl->nTriggerSource,nState);
				for(i=0;i<MAX_CH_NUM;i++)
				{
					if(pControl->nTriggerSource == i)
					{
						continue;
					}
					if(((pControl->nCHSet >> i) & 0x01) == 1)
					{
						dsoSFInsert(nData[i],CHData[i],pControl,nInsertMode,i,nState);
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

WORD SDGetDataContinue(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl,WORD nScanRoll)
{
	ULONG i=0;
	WORD nRE = 0;
	ULONG nPreLen = pControl->nAlreadyReadLen;
	WORD pCHData[MAX_CH_NUM][CONTINUE_LEN_128];
	
	if(nScanRoll == 1 && pControl->nAlreadyReadLen == 0)//Roll第1次读，本次数据作废
	{
		WORD pCHData1[MAX_CH_NUM][1024];
		nRE = ReadHardData(nDeviceIndex,pCHData1[0],pCHData1[1],pCHData1[2],pCHData1[3],4*1024);
		pControl->nAlreadyReadLen = 1;//在此 = 1，用以区别是否是第1次Roll读取数据
		return 0;
	}
	else
	{
	//设置读数长度
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
					if((nPreLen+i) >= pControl->nReadDataLen)//已满
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
				if((nPreLen+DATA_STEP_32) >= pControl->nReadDataLen)//已满
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
			if(pControl->nAlreadyReadLen == 1)//之前已经清空一次数据
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

//////////////////////////////////////////////////////////////////////////////////////////////////////


//以下函数SDK也可以用
DLL_API WORD WINAPI dsoSDLANGetData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl,WORD nInsertMode)
{
	return SDGetData(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,pControl,nInsertMode);
}

//读取Roll数据
DLL_API WORD WINAPI dsoSDLANGetRollData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl)
{
	WORD nRe = 0;
	nRe = SDGetDataContinue(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,pControl,1);//连续读取
	return nRe;
}

//读取YT-Scan数据
DLL_API WORD WINAPI dsoSDLANGetScanData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl)
{
	WORD nRe = 0;
	if(pControl->nAlreadyReadLen == 0)//第1次读取
	{
		nRe = GetPreTrigData(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,pControl/*,0*/);//0:Scan
	}
	else
	{
		nRe = SDGetDataContinue(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,pControl,0);//连续读取
	}
	return nRe;
}

DLL_API WORD WINAPI dsoLANInit(WORD nDeviceIndex,char* szlpV4,USHORT iPort)
{
	return LANInit(nDeviceIndex,szlpV4,iPort);
}

DLL_API WORD WINAPI dsoLANInitSocket(WORD nDeviceIndex,USHORT* pIP,USHORT iPort)
{
	char chTargetIP[15];
	sprintf(chTargetIP,"%d.%d.%d.%d", pIP[0],pIP[1],pIP[2],pIP[3]);
	return LANInit(nDeviceIndex,chTargetIP,iPort);
}


DLL_API WORD WINAPI dsoLANClose(WORD nDeviceIndex)//关闭LAN口Socket
{
	return LANClose(nDeviceIndex);
}

DLL_API WORD WINAPI dsoLANOpenConnet(WORD DeviceIndex,WORD nMode)//关闭网口，打开USB
{
	return OpenConnetMode(DeviceIndex,nMode);
}

DLL_API WORD WINAPI dsoLANGetRestartLANStatus(WORD DeviceIndex)//获取设备LAN是否启动完毕
{
	return GetRestartLANStatus(DeviceIndex);
}

DLL_API WORD WINAPI dsoLANModeSetIPAddr(WORD DeviceIndex,ULONG nIP,ULONG nSubnetMask,ULONG nGateway,USHORT nPort,BYTE* pMac)
{
	return LANSetIP(DeviceIndex,nIP,nSubnetMask,nGateway,nPort,pMac);
}

DLL_API WORD WINAPI dsoLANModeGetIPAddr(WORD DeviceIndex,ULONG* pIP,ULONG* pSubnetMask,ULONG* pGateway,USHORT* pPort,BYTE* pMac)
{
	return LANGetIP(DeviceIndex,pIP,pSubnetMask,pGateway,pPort,pMac);
}

DLL_API WORD WINAPI  dsoLANGetLanEnable(WORD DeviceIndex,short* pEnable)
{
	return LANGetLanEnable(DeviceIndex,pEnable);
}

DLL_API WORD WINAPI dsoLANResetWIFI(WORD DeviceIndex)
{
	return ResetWIFI(DeviceIndex);
}

DLL_API WORD WINAPI dsoLANGetFPGAVersion(WORD DeviceIndex)
{
	return GetFPGAVersion(DeviceIndex);
}

//SDK读取数据
WORD SDLANGetData(WORD nDeviceIndex,WORD* pCH1Data,WORD* pCH2Data,WORD* pCH3Data,WORD* pCH4Data,PCONTROLDATA pControl,WORD nInsertMode)
{
	WORD nRe = 0;
	if(pControl->nTimeDIV > MAX_INSERT_TIMEDIV)//不需要差值
	{
		nRe = dsoLANGetData(nDeviceIndex,pCH1Data,pCH2Data,pCH3Data,pCH4Data,pControl);
	}
	else//需要差值的档位
	{
		int i = 0;
		ULONG nState[MAX_CH_NUM];
		WORD* CHData[MAX_CH_NUM];
		USHORT* nData[MAX_CH_NUM];
		CHData[0] = pCH1Data;
		CHData[1] = pCH2Data;
		CHData[2] = pCH3Data;
		CHData[3] = pCH4Data;
		for(i=0;i<MAX_CH_NUM;i++)
		{
			nData[i] = new USHORT[pControl->nReadDataLen - 10240 + 40960];
		}
		//读数
		nRe = dsoLANGetData(nDeviceIndex,nData[0],nData[1],nData[2],nData[3],pControl);
		//差值
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
				dsoSFInsert(nData[pControl->nTriggerSource],CHData[pControl->nTriggerSource],pControl,nInsertMode,pControl->nTriggerSource,nState);
				for(i=0;i<MAX_CH_NUM;i++)
				{
					if(pControl->nTriggerSource == i)
					{
						continue;
					}
					if(((pControl->nCHSet >> i) & 0x01) == 1)
					{
						dsoSFInsert(nData[i],CHData[i],pControl,nInsertMode,i,nState);
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

/*************************DDS Lan**********************/
DLL_API ULONG WINAPI ddsLANSetOnOff(WORD DeviceIndex,short nOnOff)
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize = 2;
	int nRe = 0;

	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR) malloc(m_nSize);
	if(nOnOff == 0)
	{
		outBuffer[0]= 0x31;//关闭
   		outBuffer[1]= 0x00;
	}
	else
	{
		outBuffer[0]= 0x30;//打开
   		outBuffer[1]= 0x00;
	}
 	nRe = LANSendCtrl(DeviceIndex,outBuffer,m_nSize); 	  
	free(outBuffer);	
    return nRe;
}

DLL_API ULONG WINAPI ddsLANDownload(WORD DeviceIndex,WORD iWaveNum, WORD* pData)
{
	ULONG i,j;
	PUCHAR outBuffer = NULL,inBuffer = NULL;

	BOOLEAN status=FALSE;
	ULONG BytesReturned=0;
    UINT m_nSize;
	ULONG nReadTimes = 0;
	USHORT nPackSize = 0;
	WORD nDataLen = 0;

	outBuffer=(PUCHAR)malloc(4);
	memset(outBuffer,0,4);
//准备读取数据
	//初始化设备名
	ResetDevice(DeviceIndex);
	m_nSize = 64;//下载数据包大小设置为64

//打开设备
	inBuffer=(PUCHAR)malloc(m_nSize);
	memset(inBuffer,0,m_nSize);
	if(iWaveNum / (m_nSize/2 - 2) != 0)
	{
		nReadTimes = iWaveNum / (m_nSize/2 - 2) + 1;
	}
	else
	{
		nReadTimes = iWaveNum / (m_nSize/2 - 2);
	}
	nPackSize = m_nSize / 2;//16bit算
	for(i=0;i<nReadTimes;i++) //多次循环下载数据
	{
		//1:先设置37命令 +数据包大小
		outBuffer[0]=0x37;
		outBuffer[1]=0x00;
		outBuffer[2]=(0xFF & nPackSize);
		outBuffer[3]=(nPackSize >> 8);//数据包长度，高位在后
 		LANSendCtrl(DeviceIndex,outBuffer,4); 

		//2:设置36命令+发送数据包的内容
		memset(inBuffer, 0,m_nSize);
		inBuffer[0]=0x36;
		inBuffer[1]=0x00;
		for(j=2;j<m_nSize;j=j+2)
		{
			*(inBuffer+j) = 0xFF & *(pData+nDataLen);
			*(inBuffer+j+1) = 0xFF & (*(pData+nDataLen)>>8);//高位在后
			nDataLen++;
			if(nDataLen >= iWaveNum)
			{
				break;
			}
		}
		LANSendCtrl(DeviceIndex,inBuffer,m_nSize); 
	}
	free(inBuffer);//释放缓冲区
//	4. 数据下载完毕后设置35命令发送数据大小[11 : 0] 作为EndAddr
	nPackSize = iWaveNum;
	outBuffer[0]=0x35;
	outBuffer[1]=0x00;
	outBuffer[2]=(0xFF & nPackSize);
	outBuffer[3]=(nPackSize >> 8);//数据包长度，高位在后
	LANSendCtrl(DeviceIndex,outBuffer,4); 
	free(outBuffer);
	return status;
}

DLL_API ULONG WINAPI ddsLANSetSyncOut(WORD DeviceIndex, BOOL bEnable)
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize = 2;
	int nRe = 0;

	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR) malloc(m_nSize);
	if(bEnable == 0)
	{
		outBuffer[0]= 0x3A;//关闭
   		outBuffer[1]= 0x00;
	}
	else
	{
		outBuffer[0]= 0x39;//打开
   		outBuffer[1]= 0x00;
	}

 	nRe = LANSendCtrl(DeviceIndex,outBuffer,m_nSize); 
	free(outBuffer);	
    return nRe;
}

DLL_API ULONG WINAPI ddsLANSetCmd(WORD DeviceIndex, USHORT nSingle)
{
	PUCHAR outBuffer = NULL;
    UINT m_nSize = 4;
	int nRe = 0;
	
	outBuffer=(PUCHAR)malloc(m_nSize);
	memset(outBuffer,0,m_nSize);
	outBuffer[0]= 0x33;//
   	outBuffer[1]= 0x00;
	outBuffer[2]= 0xFF & ((nSingle & 0x01) << 2);
	outBuffer[3]= 0x00;//高位
	ResetDevice(DeviceIndex);
	nRe = LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);	

    return nRe;
}

DLL_API ULONG WINAPI ddsLANSetFrequency(WORD DeviceIndex,double dbFre,WORD* pWaveNum,WORD* pPeriodNum)
{
	int iNumber = 0;
	double period;
	float flNum;
	USHORT DivNum = 0;
	int i,nRe = 0;

	if (fabs(dbFre - 0) < 1)
	{
		dbFre = 1;
	}
	period = 1.0  / dbFre ;		
	
	if (dbFre <= 100000 )	//当频率小于100K还是按以前的方法
	{
		DivNum  = int(MAX_CLOCK*period/MAX_BUFFER);	//计算分频数
		if (DivNum == 0)
			iNumber = (int)(period * MAX_CLOCK);
		else
			iNumber = (int)(period * 100000000 / DivNum);
		*pPeriodNum = 1;
	}
	else
	{
		DivNum = 0;
		flNum = (float)(MAX_CLOCK / dbFre);		
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
	PUCHAR outBuffer = NULL;
    UINT m_nSize = 4;
	ResetDevice(DeviceIndex);
	outBuffer=(PUCHAR) malloc(m_nSize);
	outBuffer[0]= 0x34;//设置分频数
   	outBuffer[1]= 0x00;
	outBuffer[2]= 0xFF & DivNum;
	outBuffer[3]= 0xFF & (DivNum>>8);
	nRe = LANSendCtrl(DeviceIndex,outBuffer,m_nSize);
	free(outBuffer);	

	return nRe;
}
/*************************DDS Lan**********************/
