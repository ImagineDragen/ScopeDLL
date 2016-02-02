#ifndef _HT_LAN_CLASS_H_H
#define _HT_LAN_CLASS_H_H

#define MAX_USB_DEV_NUMBER			32						//PC最大设备连接数目
#define LAN_PACKAGE_SIZE			512						//
class CHTLAN
{
public:
	CHTLAN();
	virtual ~CHTLAN();

	//Attributes
public:
	SOCKET m_socket;
	sockaddr_in m_SrvAddr;
protected:

private:

	//Operations
public:
	BOOL Init(char* szServer, USHORT port);		//网络初始化
	BOOL Close();//关闭LAN
//接收/发送1个数据包
	BOOL SendPackage(BYTE* data, ULONG len);		//发送一包数据
	BOOL RecvPackage(BYTE* data,ULONG* pLen);		//接收一包数据
//批量发送/接收数据
	USHORT SendBulk(BYTE* data, ULONG len);		//向服务端发送批量数据包 
	BOOL RecvBulk(BYTE* data, ULONG len);			//接收数据


//	BOOL RecvData(BYTE* data, ULONG* len, sockaddr_in* addr);//接收数据，返回1有数据收到， 返回0无数据收到
//	USHORT LANSend(ULONG cmd, ULONG con, ULONG val);	//向服务端发送控制字	
protected:

private:

};

#endif
