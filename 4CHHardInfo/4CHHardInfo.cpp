// 4CHHardInfo.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "4CHHardInfo.h"
#include "4CHHardInfoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMy4CHHardInfoApp

BEGIN_MESSAGE_MAP(CMy4CHHardInfoApp, CWinApp)
	//{{AFX_MSG_MAP(CMy4CHHardInfoApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMy4CHHardInfoApp construction

CMy4CHHardInfoApp::CMy4CHHardInfoApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMy4CHHardInfoApp object

CMy4CHHardInfoApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMy4CHHardInfoApp initialization

BOOL CMy4CHHardInfoApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CMy4CHHardInfoDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

CString CMy4CHHardInfoApp::GetAppPath()
{
	CString appPath,strPath;
	int at;
	TCHAR tempPath[_MAX_PATH];

	::GetModuleFileName(NULL,tempPath,_MAX_PATH);
	appPath=tempPath;
	at=appPath.ReverseFind('\\');
	appPath=appPath.Left(at);
	return appPath;
}