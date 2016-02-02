// 4CHHardInfoDlg.cpp : implementation file
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
// CMy4CHHardInfoDlg dialog

CMy4CHHardInfoDlg::CMy4CHHardInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMy4CHHardInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMy4CHHardInfoDlg)
	m_strInfo = _T("");
	m_strName = _T("");
	m_strPCBVersion = _T("");
	m_strDriverVersion = _T("");
	m_strProductor = _T("");
	m_strPackger = _T("");
	m_strSN = _T("");
	m_strProduceTime = _T("");
	m_strTestTime = _T("");
	m_strTester = _T("");
	m_strFPGAVersion = _T("");
	m_strType = _T("");
	m_nPort = 50000;
	m_nMac0 = 0;
	m_nMac1 = 0;
	m_nMac2 = 0;
	m_nMac3 = 0;
	m_nMac4 = 0;
	m_nMac5 = 0;
	m_stcMac = _T("");
	m_strPW = _T("");
	m_bDDS = FALSE;
	m_bAuto = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	

}

void CMy4CHHardInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMy4CHHardInfoDlg)
	DDX_Control(pDX, IDC_EDIT7, m_ctrSN);
	DDX_Control(pDX, IDC_CHECK_AUTO, m_bAutoC);
	DDX_Control(pDX, IDC_COMBO_BW, m_comboBW);
	DDX_Control(pDX, IDC_CHECK_DDS, m_bDDSC);
	DDX_Control(pDX, IDC_COMBO1, m_comboSubVersion);
	DDX_Control(pDX, IDC_EDIT_PORT, m_editPort);
	DDX_Control(pDX, IDC_EDIT_MAC5, m_editMac5);
	DDX_Control(pDX, IDC_EDIT_MAC4, m_editMac4);
	DDX_Control(pDX, IDC_EDIT_MAC3, m_editMac3);
	DDX_Control(pDX, IDC_EDIT_MAC2, m_editMac2);
	DDX_Control(pDX, IDC_EDIT_MAC1, m_editMac1);
	DDX_Control(pDX, IDC_EDIT_MAC0, m_editMac0);
	DDX_Control(pDX, IDC_IPADDRESS_SUBMASK, m_ctlSubmask);
	DDX_Control(pDX, IDC_IPADDRESS_IP, m_ctlIP);
	DDX_Control(pDX, IDC_IPADDRESS_GATEWAY, m_ctlGateway);
	DDX_Control(pDX, IDC_CHK_WIFI, m_chkWIFI);
	DDX_Control(pDX, IDC_CHK_LAN, m_chkLAN);
	DDX_Text(pDX, IDC_EDIT_HARDINFO, m_strInfo);
	DDX_Text(pDX, IDC_EDIT2, m_strName);
	DDX_Text(pDX, IDC_EDIT3, m_strPCBVersion);
	DDX_Text(pDX, IDC_EDIT4, m_strDriverVersion);
	DDX_Text(pDX, IDC_EDIT5, m_strProductor);
	DDX_Text(pDX, IDC_EDIT6, m_strPackger);
	DDX_Text(pDX, IDC_EDIT7, m_strSN);
	DDX_Text(pDX, IDC_EDIT8, m_strProduceTime);
	DDX_Text(pDX, IDC_EDIT9, m_strTestTime);
	DDX_Text(pDX, IDC_EDIT10, m_strTester);
	DDX_Text(pDX, IDC_EDIT11, m_strFPGAVersion);
	DDX_Text(pDX, IDC_EDIT1, m_strType);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_Text(pDX, IDC_EDIT_MAC0, m_nMac0);
	DDX_Text(pDX, IDC_EDIT_MAC1, m_nMac1);
	DDX_Text(pDX, IDC_EDIT_MAC2, m_nMac2);
	DDX_Text(pDX, IDC_EDIT_MAC3, m_nMac3);
	DDX_Text(pDX, IDC_EDIT_MAC4, m_nMac4);
	DDX_Text(pDX, IDC_EDIT_MAC5, m_nMac5);
	DDX_Text(pDX, IDC_STATIC_MAC, m_stcMac);
	DDX_Text(pDX, IDC_EDIT12, m_strPW);
	DDX_Check(pDX, IDC_CHECK_DDS, m_bDDS);
	DDX_Check(pDX, IDC_CHECK_AUTO, m_bAuto);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMy4CHHardInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CMy4CHHardInfoDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_WIRTE, OnBtnWirte)
	ON_BN_CLICKED(IDC_BTN_READ, OnBtnRead)
	ON_BN_CLICKED(IDC_CHK_LAN, OnChkLan)
	ON_BN_CLICKED(IDC_CHK_WIFI, OnChkWifi)
	ON_BN_CLICKED(IDC_BTN_READLAN, OnBtnReadlan)
	ON_BN_CLICKED(IDC_BTN_WRITELAN, OnBtnWritelan)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
	ON_BN_CLICKED(IDC_CHECK_DDS, OnCheckDds)
	ON_BN_CLICKED(IDC_CHECK_AUTO, OnCheckAuto)
	ON_CBN_SELCHANGE(IDC_COMBO_BW, OnSelchangeComboBw)
	ON_CBN_EDITCHANGE(IDC_COMBO1, OnEditchangeCombo1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMy4CHHardInfoDlg message handlers

BOOL CMy4CHHardInfoDlg::OnInitDialog()
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
	m_nDeviceIndex = 0;
	short nDevInfo[32];
	m_nLAN = 0;
	m_nWIFI = 0;
	m_nDevIP = 0xC0A80114;//192.168.1.20
	m_nDevPort = 50000;
	m_nSubMask = 0xFFFFFF00;//255.255.255.0
	m_nGateway = 0xC0A80101;//192.168.1.1
	memset(m_MAC,0,sizeof(m_MAC));
	m_editPort.SetLimitText(5);
	m_editMac0.SetLimitText(3);
	m_editMac1.SetLimitText(3);
	m_editMac2.SetLimitText(3);
	m_editMac3.SetLimitText(3);
	m_editMac4.SetLimitText(3);
	m_editMac5.SetLimitText(3);
	m_comboSubVersion.AddString("Hantek6074BC");
	m_comboSubVersion.AddString("Hantek6104BC");
	m_comboSubVersion.AddString("Hantek6204BC");
	m_comboSubVersion.AddString("Hantek6254BC");
	m_comboSubVersion.AddString("Hantek6074BD");
	m_comboSubVersion.AddString("Hantek6104BD");
	m_comboSubVersion.AddString("Hantek6204BD");
	m_comboSubVersion.AddString("Hantek6254BD");
	m_comboSubVersion.AddString("Hantek6074BE");
	m_comboSubVersion.AddString("Hantek6104BE");
	m_comboSubVersion.AddString("Hantek6204BE");
	m_comboSubVersion.AddString("Hantek6254BE");
	m_comboBW.AddString("70MHz");
	m_comboBW.AddString("100MHz");
	m_comboBW.AddString("200MHz");
	m_comboBW.AddString("250MHz");
	
	memset(nDevInfo,-1,sizeof(nDevInfo));
	if(dsoHTSearchDevice(nDevInfo) == 0)
	{
#ifndef _ENGLISH
		AfxMessageBox(_T("没有发现设备"));
#else
		AfxMessageBox(_T("No device was found!"));
#endif
		//USB
		if(ReadCfg() == 1)
		{
			ShowHardInfo();
		}
		else
		{
#ifndef _ENGLISH
			AfxMessageBox(_T("读取上一次USB配置文件失败"));
#else

#endif
		}
		//LAN
		if(ReadCfg() == 1)
		{
			m_nLAN = 0;
			m_nWIFI = 0;
			ShowLANInfo();
		}
		else
		{
#ifndef _ENGLISH
			AfxMessageBox(_T("读取上一次LAN配置文件失败"));
#else

#endif
		}
	}
	else//有设备 
	{
		//USB
		memset(m_chInfo,0,sizeof(m_chInfo));
		dsoGetHardInfo(m_nDeviceIndex,m_chInfo);
		if(m_chInfo[0] != 'D'||m_chInfo[1] != 'S'||m_chInfo[2] != 'O')
		{
#ifndef _ENGLISH
			AfxMessageBox(_T("未写入USB设备信息"));
#else
			AfxMessageBox(_T("Error: no info...!"));
#endif
			if(ReadCfg() == 1)
			{
				ShowHardInfo();
			}
			else
			{
#ifndef _ENGLISH
				AfxMessageBox(_T("读取上一次USB配置文件失败"));
#else
				AfxMessageBox(_T("Error: no USB info...!"));
#endif
			}
		}
		else
		{
			ShowHardInfo();
		}
		//LAN
		short nMode = 0;
		dsoGetLANEnable(m_nDeviceIndex,&nMode);
		if(nMode == 0xFF)
		{
#ifndef _ENGLISH
			AfxMessageBox(_T("未写入LAN设备信息"));
#else
			AfxMessageBox(_T("Error: no LAN info...!"));
#endif
			if(ReadLANCfg() == 1)
			{
				ShowLANInfo();
			}
			else
			{
#ifndef _ENGLISH
				AfxMessageBox(_T("读取上一次LAN配置文件失败"));
#else
				AfxMessageBox(_T("Error: no LAN info...!"));
#endif
			}
		}
		else
		{
			m_nLAN = nMode & 0x01;
			m_nWIFI = (nMode>>1) & 0x01;
			if(m_nLAN == 1)//有LAN
			{
				dsoUSBModeGetIPAddr(m_nDeviceIndex,&m_nDevIP,&m_nSubMask,&m_nGateway,&m_nDevPort,m_MAC);//读取IP Addr;
			}
			ShowLANInfo();
		}
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMy4CHHardInfoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMy4CHHardInfoDlg::OnPaint() 
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
HCURSOR CMy4CHHardInfoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CMy4CHHardInfoDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CMy4CHHardInfoDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

#define TYPE_LEN		6		//产品类型
#define NAME_LEN		8		//产品名称
#define PCB_LEN			6		//PCB版本
#define DRIVER_LEN		6		//驱动版本号
#define PRODUCTOR_LEN	4		//生产小组
#define PACKGER_LEN		4		//包装小组
#define SN_LEN			8		//产品编号
#define PRODUCE_LEN		8		//生产日期
#define TESTTIME_LEN	8		//测试日期
#define TESTSN_LEN		4		//测试人员
#define FPGA_LEN		6		//FPGA版本号

void CMy4CHHardInfoDlg::OnBtnWirte() 
{
	// TODO: Add your control notification handler code here
	if(UpdateData())
	{
		int i;
		int nLen = 0;
		
		m_strType=_T("DSO");
		m_strName=_T("6000");
		memset(m_chInfo,0xFF,sizeof(m_chInfo));
		for(i=0;i<m_strType.GetLength();i++)
		{
			
			m_chInfo[nLen + i] = m_strType.GetAt(i);
		}
		nLen += TYPE_LEN;
		for(i=0;i<m_strName.GetLength();i++)
		{
			
			m_chInfo[nLen + i] = m_strName.GetAt(i);
		}
		nLen += NAME_LEN;
		for(i=0;i<m_strPCBVersion.GetLength();i++)
		{
			m_chInfo[nLen + i] = m_strPCBVersion.GetAt(i);
		}
		nLen += PCB_LEN;
		for(i=0;i<m_strDriverVersion.GetLength();i++)
		{
			m_chInfo[nLen + i] = m_strDriverVersion.GetAt(i);
		}
		nLen += DRIVER_LEN;
		for(i=0;i<m_strProductor.GetLength();i++)
		{
			m_chInfo[nLen + i] = m_strProductor.GetAt(i);
		}
		nLen += PRODUCTOR_LEN;
		for(i=0;i<m_strPackger.GetLength();i++)
		{
			m_chInfo[nLen + i] = m_strPackger.GetAt(i);
		}
		nLen += PACKGER_LEN;
		for(i=0;i<m_strSN.GetLength();i++)
		{
			m_chInfo[nLen + i] = m_strSN.GetAt(i);
		}
		nLen += SN_LEN;
		for(i=0;i<m_strProduceTime.GetLength();i++)
		{
			m_chInfo[nLen + i] = m_strProduceTime.GetAt(i);
		}
		nLen += PRODUCE_LEN;
		for(i=0;i<m_strTestTime.GetLength();i++)
		{
			m_chInfo[nLen + i] = m_strTestTime.GetAt(i);
		}
		nLen += TESTTIME_LEN;
		for(i=0;i<m_strTester.GetLength();i++)
		{
			m_chInfo[nLen + i] = m_strTester.GetAt(i);
		}
		nLen += TESTSN_LEN;
		for(i=0;i<m_strFPGAVersion.GetLength();i++)
		{
			m_chInfo[nLen + i] = m_strFPGAVersion.GetAt(i);
		}
/*		nLen += FPGA_LEN;
		m_chInfo[nLen] = m_nLAN;
		nLen += LAN_ENABLE;
		m_chInfo[nLen] = m_nWIFI;*/
		//
		nLen += FPGA_LEN;

		if(GetDlgItem(IDC_EDIT7)->IsWindowEnabled())
		{
			BOOL isddsCheck=m_bDDSC.GetCheck();
			if(!isddsCheck){
				m_nSubVersion=3;
			}else{
				m_nSubVersion=7;
			}
			m_comboBW.SetCurSel(m_nSubVersion);
			ShowHardInfo();
		}

		
		m_chInfo[nLen ] = m_nSubVersion;
		m_chInfo[nLen +1] = 0xFF;
		m_chInfo[nLen +2] = 0xAC;
		UpdateData(FALSE);
		if(dsoSetHardInfo(m_nDeviceIndex,m_chInfo) == 1)
		{
#ifndef _ENGLISH
			AfxMessageBox(_T("USB信息写入完毕"));
#else
			AfxMessageBox(_T("USB Info write OK!"));
#endif
			WriteCfg();
		}
		else
		{
#ifndef _ENGLISH
			AfxMessageBox(_T("USB信息写入失败,检测设备是否连接"));
#else
			AfxMessageBox(_T("USB Info write Error!"));
#endif
		}
	}
}

void CMy4CHHardInfoDlg::OnBtnRead() 
{
	// TODO: Add your control notification handler code here
	memset(m_chInfo,0,sizeof(m_chInfo));
	m_strSubVersion="123456654321";
	unsigned char pTemp[70];
	if(dsoGetHardInfo(m_nDeviceIndex,pTemp) == 1)
	{
		if(pTemp[0] != 'D'||pTemp[1] != 'S'||pTemp[2] != 'O'){
		
			AfxMessageBox(_T("未写入设备信息"));
			return ;
		}
		for(int i=0;i<70;i++){
			m_chInfo[i] =pTemp[i];
		
		}
		ShowHardInfo();
#ifndef _ENGLISH
		AfxMessageBox(_T("已读取USB设备信息"));
#else
		AfxMessageBox(_T("Read USB info OK!"));
#endif
	}
	else
	{
#ifndef _ENGLISH
		AfxMessageBox(_T("无法读取USB设备信息"));
#else
		AfxMessageBox(_T("Read USB info Error!"));
#endif
	}
}

void CMy4CHHardInfoDlg::ShowHardInfo()
{
	//return ;
	int i=0;
	CString str;
	int nLen = 0;
//产品类型--
	nLen += TYPE_LEN;
	m_strType = _T("");
	for(i=0;i<TYPE_LEN;i++)
	{
		if(m_chInfo[i] == 0xFF)
		{
			continue;
		}
		str.Format(_T("%c"),m_chInfo[i]);
		m_strType += str;
	}
	
//产品名称--
	nLen += NAME_LEN;
	m_strName = _T("");
	for(;i<nLen;i++)
	{
		if(m_chInfo[i] == 0xFF)
		{
			continue;
		}
		str.Format(_T("%c"),m_chInfo[i]);
		m_strName += str;
	}
	
//PCB版本号
	nLen += PCB_LEN;
	m_strPCBVersion = _T("");
	for(;i<nLen;i++)
	{
		if(m_chInfo[i] == 0xFF)
		{
			continue;
		}
		str.Format(_T("%c"),m_chInfo[i]);
		m_strPCBVersion += str;
	}
//驱动版本号
	nLen += DRIVER_LEN;
	m_strDriverVersion = _T("");
	for(;i<nLen;i++)
	{
		if(m_chInfo[i] == 0xFF)
		{
			continue;
		}
		str.Format(_T("%c"),m_chInfo[i]);
		m_strDriverVersion += str;
	}
//生产小组
	nLen += PRODUCTOR_LEN;
	m_strProductor = _T("");
	for(;i<nLen;i++)
	{
		if(m_chInfo[i] == 0xFF)
		{
			continue;
		}
		str.Format(_T("%c"),m_chInfo[i]);
		m_strProductor += str;
	}
//包装小组
	nLen += PACKGER_LEN;
	m_strPackger = _T("");
	for(;i<nLen;i++)
	{
		if(m_chInfo[i] == 0xFF)
		{
			continue;
		}
		str.Format(_T("%c"),m_chInfo[i]);
		m_strPackger += str;
	}
//产品编号
	nLen += SN_LEN;
	m_strSN = _T("");
	for(;i<nLen;i++)
	{
		if(m_chInfo[i] == 0xFF)
		{
			continue;
		}
		str.Format(_T("%c"),m_chInfo[i]);
		m_strSN += str;
	}
//生产日期
	nLen += PRODUCE_LEN;
	m_strProduceTime = _T("");
	for(;i<nLen;i++)
	{
		if(m_chInfo[i] == 0xFF)
		{
			continue;
		}
		str.Format(_T("%c"),m_chInfo[i]);
		m_strProduceTime += str;
	}
//测试日期
	nLen += TESTTIME_LEN;
	m_strTestTime = _T("");
	for(;i<nLen;i++)
	{
		if(m_chInfo[i] == 0xFF)
		{
			continue;
		}
		str.Format(_T("%c"),m_chInfo[i]);
		m_strTestTime += str;
	}
//测试员
	nLen += TESTSN_LEN;
	m_strTester = _T("");
	for(;i<nLen;i++)
	{
		if(m_chInfo[i] == 0xFF)
		{
			continue;
		}
		str.Format(_T("%c"),m_chInfo[i]);
		m_strTester += str;
	}
//FPGA
	nLen += FPGA_LEN;
	m_strFPGAVersion = _T("");
	for(;i<nLen;i++)
	{
		if(m_chInfo[i] == 0xFF)
		{
			continue;
		}
		str.Format(_T("%c"),m_chInfo[i]);
		m_strFPGAVersion += str;
	}
//子版本

	nLen +=SUBVERSION_LEN;
	//m_strFPGAVersion = _T("");
	for(;i<nLen;i++)
	{
		if(m_chInfo[i] == 0xFF)
		{
			continue;
		}
		m_nSubVersion=m_chInfo[i];
		if(m_nSubVersion>20)
			m_nSubVersion=-1;

		m_comboSubVersion.SetCurSel(m_nSubVersion);
	//	m_comboSubVersion.SetCurSel(-1);
		//m_comboSubVersion.GetCurSel();
		//if(m_nSubVersion)
		m_comboSubVersion.GetLBText(m_nSubVersion,m_strSubVersion);
		upDateTheCtrs(m_nSubVersion);

		
	}
	
	UpdateData(FALSE);
}

USHORT CMy4CHHardInfoDlg::ReadCfg()
{
	CString strFileName;
	CFile file;
	CFileException ex;

	strFileName=((CMy4CHHardInfoApp*)AfxGetApp())->GetAppPath();
	strFileName = strFileName + _T("\\Default.fig");
	//如果后缀名不为".set"
	if(strFileName.Right(4).CompareNoCase(_T(".fig")))
	{
		return 0;
	}

	if (!file.Open(strFileName, CFile::modeRead, &ex))//打开文件
	{	
		return 0;		
	}

//开始读取
	file.SeekToBegin();
	file.Read(m_chInfo,sizeof(m_chInfo));
	file.Close();
	return 1;
}

USHORT CMy4CHHardInfoDlg::WriteCfg()
{
	CString strFileName;
	CFile file;
	CFileException ex;

	strFileName=((CMy4CHHardInfoApp*)AfxGetApp())->GetAppPath();
	strFileName = strFileName + _T("\\Default.fig");
	//如果后缀名不为".set"
	if(strFileName.Right(4).CompareNoCase(_T(".fig")))
	{
		return 0;
	}

	if(!file.Open(strFileName,CFile::modeWrite | CFile::modeCreate,&ex))
	{	
		return 0;		
	}

//开始读取
	file.SeekToBegin();
	file.Write(m_chInfo,sizeof(m_chInfo));
	file.Close();
	return 1;
}

void CMy4CHHardInfoDlg::ShowLANInfo()
{
	CString str = _T("");
	BYTE ip[4];

//
	//网口
	if(m_nLAN == 0xFF || m_nLAN == 0x01)
	{
		m_chkLAN.SetCheck(1);
		m_chkWIFI.EnableWindow(TRUE);
		if(m_nWIFI == 0xFF || m_nWIFI == 0x01)
		{
			m_chkWIFI.SetCheck(1);
		}
		else
		{
			m_chkWIFI.SetCheck(0);
		}
	}
	else
	{
		m_chkLAN.SetCheck(0);
		m_chkWIFI.EnableWindow(FALSE);
		m_chkWIFI.SetCheck(0);
		m_nWIFI = 0;

	}

	GetValue(m_nDevIP,ip);
	m_ctlIP.SetAddress(ip[0],ip[1],ip[2],ip[3]);
	GetValue(m_nSubMask,ip);
	m_ctlSubmask.SetAddress(ip[0],ip[1],ip[2],ip[3]);
	GetValue(m_nGateway,ip);
	m_ctlGateway.SetAddress(ip[0],ip[1],ip[2],ip[3]);
	m_nPort = m_nDevPort;

	m_nMac0 = m_MAC[0];
	m_nMac1 = m_MAC[1];
	m_nMac2 = m_MAC[2];
	m_nMac3 = m_MAC[3];
	m_nMac4 = m_MAC[4];
	m_nMac5 = m_MAC[5];

	m_stcMac = _T("");
	for(int i=0;i<6;i++)
	{
		str.Format(_T(" %02X  "),m_MAC[i]);
		if(i == 5)
		{
			;
		}
		else
		{
			str += _T("  -  ");
		}
		m_stcMac += str;
	}
	UpdateData(FALSE);
}

USHORT CMy4CHHardInfoDlg::ReadLANCfg()
{
	return 0;
	CString strFileName;
	CFile file;
	CFileException ex;

	strFileName=((CMy4CHHardInfoApp*)AfxGetApp())->GetAppPath();
	strFileName = strFileName + _T("\\LANDefault.fig");
	//如果后缀名不为".set"
	if(strFileName.Right(4).CompareNoCase(_T(".fig")))
	{
		return 0;
	}

	if (!file.Open(strFileName, CFile::modeRead, &ex))//打开文件
	{	
		return 0;		
	}

//开始读取
	file.SeekToBegin();
	file.Read(&m_nDevIP,sizeof(m_nDevIP));
	file.Read(&m_nDevPort,sizeof(m_nDevPort));
	file.Read(&m_nSubMask,sizeof(m_nSubMask));
	file.Read(&m_nGateway,sizeof(m_nGateway));
	file.Read(m_MAC,sizeof(m_MAC));
	file.Close();
	return 1;
}

USHORT CMy4CHHardInfoDlg::WriteLANCfg()
{
	CString strFileName;
	CFile file;
	CFileException ex;

	strFileName=((CMy4CHHardInfoApp*)AfxGetApp())->GetAppPath();
	strFileName = strFileName + _T("\\LANDefault.fig");
	//如果后缀名不为".set"
	if(strFileName.Right(4).CompareNoCase(_T(".fig")))
	{
		return 0;
	}

	if(!file.Open(strFileName,CFile::modeWrite | CFile::modeCreate,&ex))
	{	
		return 0;		
	}

//开始读取
	file.SeekToBegin();
	file.Write(&m_nDevIP,sizeof(m_nDevIP));
	file.Write(&m_nDevPort,sizeof(m_nDevPort));
	file.Write(&m_nSubMask,sizeof(m_nSubMask));
	file.Write(&m_nGateway,sizeof(m_nGateway));
	file.Write(m_MAC,sizeof(m_MAC));
	file.Close();
	return 1;
}

void CMy4CHHardInfoDlg::OnChkLan() 
{
	// TODO: Add your control notification handler code here
	return ;//zhang
	m_nLAN = m_chkLAN.GetCheck();
	if(m_nLAN == 0)
	{
		m_chkWIFI.SetCheck(0);
		m_chkWIFI.EnableWindow(FALSE);
		m_nWIFI = 0;
	}
	else
	{
		m_chkWIFI.EnableWindow(TRUE);
		m_chkWIFI.SetCheck(m_nWIFI);
	}
}

void CMy4CHHardInfoDlg::OnChkWifi() 
{
	// TODO: Add your control notification handler code here
	m_nWIFI = m_chkWIFI.GetCheck();
}

void CMy4CHHardInfoDlg::OnBtnReadlan() 
{
	// TODO: Add your control notification handler code here
	short nMode = 0;
	if(dsoGetLANEnable(m_nDeviceIndex,&nMode) == 1)
	{
		m_nLAN = nMode & 0x01;
		m_nWIFI = (nMode>>1) & 0x01;
		dsoUSBModeGetIPAddr(m_nDeviceIndex,&m_nDevIP,&m_nSubMask,&m_nGateway,&m_nDevPort,m_MAC);//读取IP Addr
		ShowLANInfo();
#ifndef _ENGLISH
		AfxMessageBox(_T("已读取LAN设备信息"));
#else
		AfxMessageBox(_T("Read LAN info OK!"));
#endif
	}
	else
	{
#ifndef _ENGLISH
		AfxMessageBox(_T("无法读取LAN设备信息"));
#else
		AfxMessageBox(_T("Read LAN info Error!"));
#endif
	}
}

void CMy4CHHardInfoDlg::OnBtnWritelan() 
{
	// TODO: Add your control notification handler code here
	BOOL bSet = TRUE;
	short nMode = 0xFF;
	BYTE a,b,c,d;
	ULONG tmp = 0;
	nMode = (m_nLAN & 0x01);
	nMode = nMode | ((m_nWIFI & 0x01)<<1);

	m_ctlIP.GetAddress(a,b,c,d);
	tmp = (a<<24) + (b<<16) + (c<<8) + d;
	if(tmp != m_nDevIP)
	{
		m_nDevIP = tmp;
		bSet = TRUE;
	}
	m_ctlSubmask.GetAddress(a,b,c,d);
	tmp = (a<<24) + (b<<16) + (c<<8) + d;
	if(tmp != m_nSubMask)
	{
		m_nSubMask = tmp;
		bSet = TRUE;
	}
	m_ctlGateway.GetAddress(a,b,c,d);
	tmp = (a<<24) + (b<<16) + (c<<8) + d;
	if(tmp != m_nGateway)
	{
		m_nGateway = tmp;
		bSet = TRUE;
	}

	if(UpdateData())
	{
		//Port
		if(m_nPort<1 || m_nPort > 65535)
		{
			m_nPort = m_nDevPort;
		}
		else
		{
			if(m_nPort != m_nDevPort)
			{
				m_nDevPort = m_nPort;
				bSet = TRUE;
			}
		}
		//Mac
		if(m_nMac0 < 0 || m_nMac0 > 255)
		{
			m_nMac0 = m_MAC[0];
		}
		else
		{
			if(m_nMac0 != m_MAC[0])
			{
				m_MAC[0] = m_nMac0;
				bSet = TRUE;
			}
		}
		if(m_nMac1 < 0 || m_nMac1 > 255)
		{
			m_nMac1 = m_MAC[1];
		}
		else
		{
			if(m_nMac1 != m_MAC[1])
			{
				m_MAC[1] = m_nMac1;
				bSet = TRUE;
			}
		}

		if(m_nMac2 < 0 || m_nMac2 > 255)
		{
			m_nMac2 = m_MAC[2];
		}
		else
		{
			if(m_nMac2 != m_MAC[2])
			{
				m_MAC[2] = m_nMac2;
				bSet = TRUE;
			}
		}
		
		if(m_nMac3 < 0 || m_nMac3 > 255)
		{
			m_nMac3 = m_MAC[3];
		}
		else
		{
			if(m_nMac3 != m_MAC[3])
			{
				m_MAC[3] = m_nMac3;
				bSet = TRUE;
			}
		}

		if(m_nMac4 < 0 || m_nMac4 > 255)
		{
			m_nMac4 = m_MAC[4];
		}
		else
		{
			if(m_nMac4 != m_MAC[4])
			{
				m_MAC[4] = m_nMac4;
				bSet = TRUE;
			}
		}
		
		if(m_nMac5 < 0 || m_nMac5 > 255)
		{
			m_nMac5 = m_MAC[5];
		}
		else
		{
			if(m_nMac5 != m_MAC[5])
			{
				m_MAC[5] = m_nMac5;
				bSet = TRUE;
			}
		}

		UpdateData(FALSE);
	}
	if(dsoSetLANEnable(m_nDeviceIndex,nMode) == 1)
	{
		if(bSet)
		{
			dsoUSBModeSetIPAddr(m_nDeviceIndex,m_nDevIP,m_nSubMask,m_nGateway,m_nDevPort,m_MAC);
		}
#ifndef _ENGLISH
		AfxMessageBox(_T("LAN信息写入完毕"));
#else
		AfxMessageBox(_T("LAN Info write OK!"));
#endif
		WriteLANCfg();
	}
	else
	{
#ifndef _ENGLISH
		AfxMessageBox(_T("LAN信息写入失败,检测设备是否连接"));
#else
		AfxMessageBox(_T("LAN Info write Error!"));
#endif
	}
}
void CMy4CHHardInfoDlg::GetValue(ULONG nType,BYTE* pValue)
{
	pValue[3] = (BYTE)(nType & 0xFF);
	pValue[2] = (BYTE)((nType>>8) & 0xFF);
	pValue[1] = (BYTE)((nType>>16) & 0xFF);
	pValue[0] = (BYTE)((nType>>24) & 0xFF);
}

void CMy4CHHardInfoDlg::OnSelchangeCombo1() 
{
	
	// TODO: Add your control notification handler code here
	int nPreBW=0xFF;
	if(m_strSubVersion.GetLength()>=6)
		nPreBW=_ttoi(m_strSubVersion.Right(5).Left(2));
	if(m_strPW=="iamadmin")
	{
		m_nSubVersion=m_comboSubVersion.GetCurSel();
		m_comboSubVersion.GetLBText(m_comboSubVersion.GetCurSel(),m_strSubVersion);
		upDateTheCtrs(m_nSubVersion);
		return;
	}
	CString str="123456654321";
	m_comboSubVersion.GetLBText(m_comboSubVersion.GetCurSel(),str.GetBuffer(0));
	if(str.GetLength()<6)
	{
		m_comboSubVersion.SetCurSel(m_nSubVersion);
		m_comboSubVersion.GetLBText(m_comboSubVersion.GetCurSel(),m_strSubVersion);
		upDateTheCtrs(m_nSubVersion);
		return;
	}
	else
	{
		int nBW=_ttoi(str.Right(5).Left(2));
		if(nBW>nPreBW)
		{
			m_comboSubVersion.SetCurSel(m_nSubVersion);
			m_comboSubVersion.GetLBText(m_comboSubVersion.GetCurSel(),m_strSubVersion.GetBuffer(0));
			AfxMessageBox(_T("带宽不能增大"));
			upDateTheCtrs(m_nSubVersion);
			return ;
		}
		if(str.Right(1)=="C")
		{
			if(m_strSubVersion.Right(1)=="D")
			{
				m_comboSubVersion.SetCurSel(m_nSubVersion);
				m_comboSubVersion.GetLBText(m_comboSubVersion.GetCurSel(),m_strSubVersion.GetBuffer(0));
				AfxMessageBox(_T("不能将有DDS设备设置成无DDS设备"));
				upDateTheCtrs(m_nSubVersion);
				return ;
			}
		}
		else if(str.Right(1)=="D")
		{
			if(m_strSubVersion.Right(1)=="C")
			{
				m_comboSubVersion.SetCurSel(m_nSubVersion);
				m_comboSubVersion.GetLBText(m_comboSubVersion.GetCurSel(),m_strSubVersion.GetBuffer(0));
				AfxMessageBox(_T("不能将无DDS设备设置成有DDS设备"));
				upDateTheCtrs(m_nSubVersion);
				return ;
			}
			else if(m_strSubVersion.Right(1)=="E"){				
				m_comboSubVersion.SetCurSel(m_nSubVersion);
				m_comboSubVersion.GetLBText(m_comboSubVersion.GetCurSel(),m_strSubVersion.GetBuffer(0));
				AfxMessageBox(_T("不能将无DDS设备设置成有DDS设备"));
				upDateTheCtrs(m_nSubVersion);
				return ;

			}
		}
		else if(str.Right(1)=="E"){
			if(m_strSubVersion.Right(1)=="D")
			{
				m_comboSubVersion.SetCurSel(m_nSubVersion);
				m_comboSubVersion.GetLBText(m_comboSubVersion.GetCurSel(),m_strSubVersion.GetBuffer(0));
				AfxMessageBox(_T("不能将有DDS设备设置成无DDS设备"));
				upDateTheCtrs(m_nSubVersion);
				return ;
			}

		}

	}

	m_nSubVersion=m_comboSubVersion.GetCurSel();
	m_comboSubVersion.GetLBText(m_comboSubVersion.GetCurSel(),m_strSubVersion);
	upDateTheCtrs(m_nSubVersion);
	
}
void CMy4CHHardInfoDlg::upDateTheCtrs(WORD nIndex)
{
	if(nIndex>=0&&nIndex<4){
		m_bDDS=false;
		m_bAuto=false;

	}
	else if(nIndex>=4&&nIndex<8){
		m_bDDS=true;
		m_bAuto=false;
	}
	else if(nIndex>=8&&nIndex<12){
		m_bDDS=false;
		m_bAuto=true;
	}
	else
	{
		m_bDDS=false;
		m_bAuto=false;
	}
	m_nBWIndex=nIndex%4;
	m_bDDSC.SetCheck(m_bDDS);
	m_bAutoC.SetCheck(m_bAuto);
	m_comboBW.SetCurSel(m_nBWIndex);
}

void CMy4CHHardInfoDlg::OnCheckDds() 

{
	// TODO: Add your control notification handler code here
	int bAuto=m_bAutoC.GetCheck();
	int bDDS=m_bDDSC.GetCheck();
	if(bAuto&&bDDS)
		m_bAutoC.SetCheck(false);
	changgeSel();
	
}

void CMy4CHHardInfoDlg::OnCheckAuto() 
{
	// TODO: Add your control notification handler code here
	int bAuto=m_bAutoC.GetCheck();
	int bDDS=m_bDDSC.GetCheck();
	if(bAuto&&bDDS)
	{
		//m_bDDSC.SetCheck(false);
		m_bAutoC.SetCheck(false);
		AfxMessageBox(_T("不能将有DDS设备用作汽车测试设备"));
		
	}
	changgeSel();
	
}

void CMy4CHHardInfoDlg::OnSelchangeComboBw() 
{
	// TODO: Add your control notification handler code here
	changgeSel();
	
}
void CMy4CHHardInfoDlg::changgeSel()
{
	// TODO: Add your control notification handler code here
	int bDDS=m_bDDSC.GetCheck();
	int bAuto=m_bAutoC.GetCheck();
	int x;
	if(bDDS){
		x=4;
	}
	else
	{
		x=bAuto?8:0;
	}
	int temp=m_comboBW.GetCurSel();
	if(temp<0)
	{
		temp=3;
		m_comboBW.SetCurSel(3);
	}
	x+=temp;
	m_nSubVersion=x;
	m_comboSubVersion.GetLBText(x,m_strSubVersion);
	m_comboSubVersion.SetCurSel(x);
	//upDateTheCtrs(m_nSubVersion);
	
}

void CMy4CHHardInfoDlg::OnEditchangeCombo1() 
{
	// TODO: Add your control notification handler code here
	//upDateTheCtrs(m_comboSubVersion.GetCurSel());
	
}
