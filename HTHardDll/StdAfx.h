// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__C28DDF53_ABC2_466E_9237_246054D684BF__INCLUDED_)
#define AFX_STDAFX_H__C28DDF53_ABC2_466E_9237_246054D684BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

//#include <Afxwin.h>
//#define FILE_TEST
//#define FILE_TEST//add by zhang
#ifdef FILE_TEST
	#include "afx.h"
#else
	#include <windows.h>
#endif
// TODO: reference additional headers your program requires here

#define DLL_API						extern "C" __declspec(dllexport)
#define MAX_DRIVER_NAME				64		//最大设备名称长度
#define TRIGGER_POS_OFFSET					//触发点偏移
#define USB_PACK_SIZE				512		//USB上传数据包大小

#include <conio.h>
#include <math.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include ".\\inc\\CyAPI.h"
#include "c:\\cypress\\usb\\drivers\\ezusbdrv\\ezusbsys.h"
#include "..\\Scope\\DefMacro.h"
#include "..\\HTSoftDll\\HTSoftDll.h"

#ifdef FILE_TEST
#include <windows.h>
#endif

#ifdef MINISCOPE
	#ifdef _DEBUG
		#pragma comment(lib,"..\\HTSoftDll\\HTSoftDll___Win32_MiniScopeDebug\\HTSoftDll.lib")
	#else
		#pragma comment(lib,"..\\HTSoftDll\\HTSoftDll___Win32_MiniScopeRelease\\HTSoftDll.lib")
	#endif
		
#else
	#ifdef _DEBUG
		#pragma comment(lib,"..\\HTSoftDll\\Debug\\HTSoftDll.lib")
	#else
		#pragma comment(lib,"..\\HTSoftDll\\Release\\HTSoftDll.lib")
	#endif

#endif

#include "HTHardDll.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C28DDF53_ABC2_466E_9237_246054D684BF__INCLUDED_)
