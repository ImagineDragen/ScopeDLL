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

//#define FILE_TEST  //读写文件

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

//仅4250使用
#define _LAUNCH


/*

typedef struct _HT_CONTROL_DATA
{
		WORD nCHSet;//CH开关//Value://第0位：表示CH1开或者关. 0:关，1开
									//第1位：表示CH2开或者关. 0:关，1开
									//第2位：表示CH3开或者关. 0:关，1开
									//第3位：表示CH4开或者关. 0:关，1开
        WORD nTimeDIV;//时基
        WORD nTriggerSource;//触发源
        WORD nHTriggerPos;//水平触发位置
		WORD nVTriggerPos;//垂直触发位置
		WORD nTriggerSlope;//边沿触发触发沿
		ULONG nBufferLen;//内存长度
		ULONG nReadDataLen;//读取数据长度
		ULONG nAlreadyReadLen;//已经读取的长度
    	WORD nALT;//是否交替
		WORD nETSOpen;//是否ETS采集
		
}CONTROLDATA,*PCONTROLDATA;


*/
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__E8515053_87A3_4815_A887_85364D366A20__INCLUDED_)
