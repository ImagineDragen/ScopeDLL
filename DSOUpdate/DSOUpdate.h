// DSOUpdate.h : main header file for the DSOUPDATE application
//

#if !defined(AFX_DSOUPDATE_H__4A355432_FE96_49F8_A403_AF7C0B6DD490__INCLUDED_)
#define AFX_DSOUPDATE_H__4A355432_FE96_49F8_A403_AF7C0B6DD490__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDSOUpdateApp:
// See DSOUpdate.cpp for the implementation of this class
//

class CDSOUpdateApp : public CWinApp
{
public:
	CDSOUpdateApp();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDSOUpdateApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDSOUpdateApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSOUPDATE_H__4A355432_FE96_49F8_A403_AF7C0B6DD490__INCLUDED_)
