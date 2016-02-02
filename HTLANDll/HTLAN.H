#ifndef _HT_LAN_CLASS_H_H
#define _HT_LAN_CLASS_H_H

#define MAX_USB_DEV_NUMBER			32						//PC����豸������Ŀ
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
	BOOL Init(char* szServer, USHORT port);		//�����ʼ��
	BOOL Close();//�ر�LAN
//����/����1�����ݰ�
	BOOL SendPackage(BYTE* data, ULONG len);		//����һ������
	BOOL RecvPackage(BYTE* data,ULONG* pLen);		//����һ������
//��������/��������
	USHORT SendBulk(BYTE* data, ULONG len);		//�����˷����������ݰ� 
	BOOL RecvBulk(BYTE* data, ULONG len);			//��������


//	BOOL RecvData(BYTE* data, ULONG* len, sockaddr_in* addr);//�������ݣ�����1�������յ��� ����0�������յ�
//	USHORT LANSend(ULONG cmd, ULONG con, ULONG val);	//�����˷��Ϳ�����	
protected:

private:

};

#endif