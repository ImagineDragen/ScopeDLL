// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__E8515053_87A3_4815_A887_85364D366A20__INCLUDED_)
#define AFX_STDAFX_H__E8515053_87A3_4815_A887_85364D366A20__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

//#define FILE_TEST  //��д�ļ�

#ifdef FILE_TEST
#include "afx.h"
#else
#include <windows.h>
#endif

// TODO: reference additional headers your program requires here
#define DLL_API extern "C" __declspec(dllexport)
#include <math.h>
#include "..\\Scope\\DefMacro.h"
#include "HTSoftDll.h"

#ifdef FILE_TEST
#include <windows.h>
#endif

//��4250ʹ��
#define _LAUNCH


/*

typedef struct _HT_CONTROL_DATA
{
		WORD nCHSet;//CH����//Value://��0λ����ʾCH1�����߹�. 0:�أ�1��
									//��1λ����ʾCH2�����߹�. 0:�أ�1��
									//��2λ����ʾCH3�����߹�. 0:�أ�1��
									//��3λ����ʾCH4�����߹�. 0:�أ�1��
        WORD nTimeDIV;//ʱ��
        WORD nTriggerSource;//����Դ
        WORD nHTriggerPos;//ˮƽ����λ��
		WORD nVTriggerPos;//��ֱ����λ��
		WORD nTriggerSlope;//���ش���������
		ULONG nBufferLen;//�ڴ泤��
		ULONG nReadDataLen;//��ȡ���ݳ���
		ULONG nAlreadyReadLen;//�Ѿ���ȡ�ĳ���
    	WORD nALT;//�Ƿ���
		WORD nETSOpen;//�Ƿ�ETS�ɼ�
		
}CONTROLDATA,*PCONTROLDATA;


*/
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__E8515053_87A3_4815_A887_85364D366A20__INCLUDED_)
