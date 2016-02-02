// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__D7484867_A963_4235_8FC1_7ECAD89C851D__INCLUDED_)
#define AFX_STDAFX_H__D7484867_A963_4235_8FC1_7ECAD89C851D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
//#include "DSO_DLL.h"
//#include "D:\\Project\\ScopeCode\\HTSoftDll\\HTSoftDll.h"
//#include "D:\\Project\\ScopeCode\\HTHardDll\\HTHardDll.h"
//#include "D:\\Project\\ScopeCode\\Scope\\DefMacro.h"
#include "..\\HTHardDll\\HTHardDll.h"
//Update
#define UPDATE_FIALE_LEN	524288	//bytes
#ifdef _DEBUG
		#pragma comment(lib,"..\\HTHardDll\\Debug\\HTHardDll.lib")
#else
		#pragma comment(lib,"..\\HTHardDll\\Release\\HTHardDll.lib")
#endif
//导入操作硬件DLL
/*#ifdef _LAUNCH
	#ifdef _DEBUG
		#pragma comment(lib,"D:\\Project\\ScopeCode\\HTHardDll\\HTHardDll___Win32__LAUNCH_Debug\\HTHardDll.lib")
	#else
		#pragma comment(lib,"D:\\Project\\ScopeCode\\HTHardDll\\HTHardDll___Win32__LAUNCH_Release\\HTHardDll.lib")
	#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib,"D:\\Project\\ScopeCode\\HTHardDll\\Debug\\HTHardDll.lib")
	#else
		#pragma comment(lib,"D:\\Project\\ScopeCode\\HTHardDll\\Release\\HTHardDll.lib")
	#endif
#endif*/

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__D7484867_A963_4235_8FC1_7ECAD89C851D__INCLUDED_)
