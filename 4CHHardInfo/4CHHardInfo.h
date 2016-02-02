// 4CHHardInfo.h : main header file for the 4CHHARDINFO application
//

#if !defined(AFX_4CHHARDINFO_H__D02EF7AF_E4F2_4C2E_B7AA_9036CBB8E019__INCLUDED_)
#define AFX_4CHHARDINFO_H__D02EF7AF_E4F2_4C2E_B7AA_9036CBB8E019__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMy4CHHardInfoApp:
// See 4CHHardInfo.cpp for the implementation of this class
//

class CMy4CHHardInfoApp : public CWinApp
{
public:
	CMy4CHHardInfoApp();
public:
	CString GetAppPath();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMy4CHHardInfoApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMy4CHHardInfoApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_4CHHARDINFO_H__D02EF7AF_E4F2_4C2E_B7AA_9036CBB8E019__INCLUDED_)
