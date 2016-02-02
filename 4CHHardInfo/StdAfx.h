// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__E3D4B337_1962_46F0_817B_5C99033CA57A__INCLUDED_)
#define AFX_STDAFX_H__E3D4B337_1962_46F0_817B_5C99033CA57A__INCLUDED_

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

#include "..\\Scope\\Defmacro.h"
#include "..\\HTSoftDll\\HTSoftDll.h"
#include "..\\HTHardDll\\HTHardDll.h"

#ifdef DSO3064A

#pragma comment(lib,"..\\HTHardDll\\HTHardDll___Win32_DSO3064ARelease\\HTHardDll.lib")

#else

#pragma comment(lib,"..\\HTHardDll\\Debug\\HTHardDll.lib")

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__E3D4B337_1962_46F0_817B_5C99033CA57A__INCLUDED_)
