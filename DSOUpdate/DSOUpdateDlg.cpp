// DSOUpdateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DSOUpdate.h"
#include "DSOUpdateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDSOUpdateDlg dialog

CDSOUpdateDlg::CDSOUpdateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDSOUpdateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDSOUpdateDlg)
	m_strPath = _T("C:\\");
	m_strInfo = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDSOUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDSOUpdateDlg)
	DDX_Control(pDX, IDC_PRO, m_pro);
	DDX_Control(pDX, IDC_BTN_UPDATE, m_btnUpdate);
	DDX_Control(pDX, IDC_BTN_READ, m_btnRead);
	DDX_Control(pDX, IDC_BTN_BROWSER, m_btnBrowser);
	DDX_Text(pDX, IDC_EDIT_PATH, m_strPath);
	DDX_Text(pDX, IDC_STC_INFO, m_strInfo);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDSOUpdateDlg, CDialog)
	//{{AFX_MSG_MAP(CDSOUpdateDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_UPDATE, OnBtnUpdate)
	ON_BN_CLICKED(IDC_BTN_READ, OnBtnRead)
	ON_BN_CLICKED(IDC_BTN_BROWSER, OnBtnBrowser)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDSOUpdateDlg message handlers

BOOL CDSOUpdateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_pUpdateData = NULL;
	m_nUpdateFPGA = 0;
	m_strInfo = _T("Connected");
	UpdateData(FALSE);
	m_nCnt = 0;
	m_pro.SetRange(1,130);
	SetTimer(0,500,NULL);
	m_hThreadProc = CreateThread(NULL,0,ThreadCollectData,this,0/*CREATE_SUSPENDED*/,NULL);
	m_hEvent = CreateEvent(NULL,FALSE,TRUE,NULL);
	m_hDrawThreadProc = CreateThread(NULL,0,ThreadDrawData,this,0,NULL);
    
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDSOUpdateDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDSOUpdateDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDSOUpdateDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CDSOUpdateDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CDSOUpdateDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	WaitForSingleObject(m_hEvent,INFINITE);
	ResetEvent(m_hEvent);//获取线程控制权
	SuspendThread(m_hThreadProc);//暂停线程
	SuspendThread(m_hDrawThreadProc);//暂停线程
	SetEvent(m_hEvent);//释放时间信号
	//退出线程
	TerminateThread(m_hThreadProc,0);
	TerminateThread(m_hDrawThreadProc,0);
//关闭线程资源
	CloseHandle(m_hEvent);
	CloseHandle(m_hThreadProc);
	CloseHandle(m_hDrawThreadProc);

	CDialog::OnCancel();
}

DWORD WINAPI CDSOUpdateDlg::ThreadCollectData(LPVOID lParam)
{
	CDSOUpdateDlg* pDlg = (CDSOUpdateDlg*)lParam;
	while(1)
	{
		if(pDlg->m_nUpdateFPGA == 1)//升级
		{
			pDlg->Updating();
			pDlg->m_nUpdateFPGA=0;
		}
		else if(pDlg->m_nUpdateFPGA == 2)//读取
		{
			pDlg->Reading();
		}
		//延迟
		Sleep(5);
	}
	return 1;
}

DWORD WINAPI CDSOUpdateDlg::ThreadDrawData(LPVOID lParam)
{
	CDSOUpdateDlg* pDlg = (CDSOUpdateDlg*)lParam;
	while(1)
	{
		if(pDlg->m_nUpdateFPGA == 1 || pDlg->m_nUpdateFPGA == 2)//升级
		{
			pDlg->m_btnUpdate.EnableWindow(FALSE);
			pDlg->m_btnRead.EnableWindow(FALSE);
		}
		else
		{
			pDlg->m_btnUpdate.EnableWindow();
			pDlg->m_btnRead.EnableWindow();
		}
		Sleep(15);
	}
	return 1;
}

void CDSOUpdateDlg::Updating()
{
		if(dsoWriteFlash(0,m_pUpdateData))
		{
			m_nUpdateFPGA = 0;
			AfxMessageBox(_T("Update sucessfull! Please restart the device!"));//成功 
		}
		else
		{
			m_nUpdateFPGA = 0;
			AfxMessageBox(_T("Error! Please restart the device!"));//失败
		}
		delete m_pUpdateData;
		m_pUpdateData = NULL;
}

void CDSOUpdateDlg::Reading()
{
	if(dsoReadFlash(0,m_pUpdateData))
	{
		SaveUpdateFile();
		m_nUpdateFPGA = 0;
		AfxMessageBox(_T("Read sucessfull!"));
	}
	else
	{
		m_nUpdateFPGA = 0;
		AfxMessageBox(_T("Error! Please check the device!"));
	}
}
BOOL CDSOUpdateDlg::LoadUpdateFile(CString strFileName)
{
	CFile file;
	CFileException ex;
	TCHAR szByte[7];
	CString str;
	//如果后缀名不为".rfc"
//	strFileName = _T("D:\\DSOUpdate\\Release\\fgdata.ufg");
	if(strFileName.Right(4).CompareNoCase(_T(".ufg")) && strFileName.Right(4).CompareNoCase(_T(".bin")))
	{
		AfxMessageBox(_T("This is not a ufg or bin file!"));
		return FALSE;
	}

	if (!file.Open(strFileName, CFile::modeRead, &ex))//打开文件
	{
		TCHAR szError[1024];
		ex.GetErrorMessage(szError, 1024);
		str.Format(_T("Can not open file: %s"), szError);
		AfxMessageBox(str);		
		return FALSE;		
	}
	
//开始读取
 	file.SeekToBegin();
    if(!strFileName.Right(4).CompareNoCase(_T(".ufg")))
	{
	//文件头部
	//保存HANTEK-DSO,14个字节
		file.Read(szByte,sizeof(szByte));
	#ifdef _UNICODE
		if(wcsncmp(szByte,_T("HTK-DSO"),7) != 0)
		{
			file.Close();
			AfxMessageBox(_T("Invalid file"));
			return FALSE;
		}
	#else
		if(strncmp(szByte,_T("HTK-DSO"),7) != 0)
		{
			file.Close();
			AfxMessageBox(_T("Invalid file"));
			return FALSE;
		}
	#endif
	}
	if(m_pUpdateData == NULL)
	{
		m_pUpdateData = new UCHAR[UPDATE_FIALE_LEN];
	}
	memset(m_pUpdateData,0,sizeof(UCHAR)*UPDATE_FIALE_LEN);
	file.Read(m_pUpdateData,sizeof(UCHAR)*UPDATE_FIALE_LEN);
	file.Close();
	return TRUE;
}

BOOL CDSOUpdateDlg::SaveUpdateFile()
{
	CFile file;
	CFileException ex;
	ULONG i=0;
	CString str;
	TCHAR szByte[7];
	CString strFileName;

	if(m_pUpdateData == NULL)
	{
		str.Format(_T("Cann't save file."));
		AfxMessageBox(str);
		return FALSE;
	}
	CString appPath,strPath;
	int at;
	TCHAR tempPath[_MAX_PATH];
//获取路径
	::GetModuleFileName(NULL,tempPath,_MAX_PATH);
	appPath=tempPath;
	at=appPath.ReverseFind('\\');
	appPath=appPath.Left(at);

	strFileName.Format(_T("%s\\%s"), appPath,_T("fgdata.ufg"));

//开始保存数据
	if(!file.Open(strFileName,CFile::modeWrite | CFile::modeCreate,&ex))
	{
		TCHAR szError[1024];

		ex.GetErrorMessage(szError, 1024);
		str.Format(_T("Cann't save file: %s"), szError);
		AfxMessageBox(str);
		return FALSE;
	}
//保存HANTEK-DSO,14个字节
#ifdef _UNICODE
	wcscpy(szByte,_T("HTK-DSO"));
#else
	strcpy(szByte,_T("HTK-DSO"));
#endif
	if(m_pUpdateData != NULL)
	{
		file.Write(szByte,sizeof(szByte));
		file.Write(m_pUpdateData,sizeof(UCHAR)*UPDATE_FIALE_LEN);
	}
	else
	{
		AfxMessageBox(_T("No update data"));
	}
	file.Close();
	return TRUE;
}
extern int nTotal;
void CDSOUpdateDlg::OnBtnUpdate() 
{
	// TODO: Add your control notification handler code here
	nTotal=0;
	UpdateData(TRUE);
	if(LoadUpdateFile(m_strPath))
	{
		if(m_pUpdateData != NULL)
		{
			m_nUpdateFPGA = 1;
			m_strInfo = _T("Updating");
			UpdateData(FALSE);
		}
	}
}

void CDSOUpdateDlg::OnBtnRead() 
{
	// TODO: Add your control notification handler code here
	if(m_pUpdateData == NULL)
	{
		m_pUpdateData = new UCHAR[UPDATE_FIALE_LEN];
	}
	memset(m_pUpdateData,0,sizeof(UCHAR)*UPDATE_FIALE_LEN);
	m_nUpdateFPGA = 2;
	m_strInfo = _T("Read");
	UpdateData(FALSE);
}

void CDSOUpdateDlg::OnBtnBrowser() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CFileDialog dlg(TRUE,_T(".bin"),NULL,OFN_OVERWRITEPROMPT,_T("Bin File(*.bin)|*.bin||"));
	dlg.m_ofn.lpstrTitle=_T("Open File");
	if (dlg.DoModal() == IDOK)
	{
		m_strPath = dlg.GetPathName();
		UpdateData(FALSE);
	}
}
int nTotal=0;
void CDSOUpdateDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_nUpdateFPGA == 1 || m_nUpdateFPGA == 2)
	{
		if(m_nCnt == 0)
		{
			m_strPoint = _T("...");
		}
		else if(m_nCnt == 1)
		{
			m_strPoint = _T("");
		}
		else if(m_nCnt == 2)
		{
			m_strPoint = _T(".");
		}
		else if(m_nCnt == 3)
		{
			m_strPoint = _T("..");
		}
		m_nCnt++;
		if(m_nCnt > 3)
		{
			m_nCnt = 0;
		}
		if(m_nUpdateFPGA == 1)
		{
			m_strInfo = _T("Updating") + m_strPoint;
		}
		else if(m_nUpdateFPGA == 2)
		{
			m_strInfo = _T("Read") + m_strPoint;
		}
	}
	else
	{
		m_strInfo = _T("Connected");
	}
	UpdateData(FALSE);
	if(m_nUpdateFPGA==1)
	{
	 m_pro.SetPos(nTotal);
	nTotal++;
	}
	CDialog::OnTimer(nIDEvent);
}
