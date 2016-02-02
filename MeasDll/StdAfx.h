// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__17DBFC49_BC77_4DFC_9CD6_4E3ABA3F4CA7__INCLUDED_)
#define AFX_STDAFX_H__17DBFC49_BC77_4DFC_9CD6_4E3ABA3F4CA7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>

// TODO: reference additional headers your program requires here
#define DLL_API extern "C" _declspec(dllexport)
#include <math.h>
#include "MeasDll.h"
#include "..\\Scope\\DefMacro.h"


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__17DBFC49_BC77_4DFC_9CD6_4E3ABA3F4CA7__INCLUDED_)
