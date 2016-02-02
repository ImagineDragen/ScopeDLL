// 4CHHardInfoDlg.h : header file
//

#if !defined(AFX_4CHHARDINFODLG_H__91C625DF_5A05_420D_BB3F_34C57B061368__INCLUDED_)
#define AFX_4CHHARDINFODLG_H__91C625DF_5A05_420D_BB3F_34C57B061368__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMy4CHHardInfoDlg dialog

class CMy4CHHardInfoDlg : public CDialog
{
// Construction
public:
	CMy4CHHardInfoDlg(CWnd* pParent = NULL);	// standard constructor
	void upDateTheCtrs(WORD nIndex);
	int m_nBWIndex;
	void changgeSel();

// Dialog Data
	//{{AFX_DATA(CMy4CHHardInfoDlg)
	enum { IDD = IDD_MY4CHHARDINFO_DIALOG };
	CEdit	m_ctrSN;
	CButton	m_bAutoC;
	CComboBox	m_comboBW;
	CButton	m_bDDSC;
	CComboBox	m_comboSubVersion;
	CEdit	m_editPort;
	CEdit	m_editMac5;
	CEdit	m_editMac4;
	CEdit	m_editMac3;
	CEdit	m_editMac2;
	CEdit	m_editMac1;
	CEdit	m_editMac0;
	CIPAddressCtrl	m_ctlSubmask;
	CIPAddressCtrl	m_ctlIP;
	CIPAddressCtrl	m_ctlGateway;
	CButton	m_chkWIFI;
	CButton	m_chkLAN;
	CString	m_strInfo;
	CString	m_strName;
	CString	m_strPCBVersion;
	CString	m_strDriverVersion;
	CString	m_strProductor;
	CString	m_strPackger;
	CString	m_strSN;
	CString	m_strProduceTime;
	CString	m_strTestTime;
	CString	m_strTester;
	CString	m_strFPGAVersion;
	CString	m_strType;
	UINT	m_nPort;
	BYTE	m_nMac0;
	BYTE	m_nMac1;
	BYTE	m_nMac2;
	BYTE	m_nMac3;
	BYTE	m_nMac4;
	BYTE	m_nMac5;
	CString	m_stcMac;
	CString	m_strPW;
	BOOL	m_bDDS;
	BOOL	m_bAuto;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMy4CHHardInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
public:
	USHORT m_nDeviceIndex;
	UCHAR m_chInfo[DEV_INFO_LEN+8];
	int m_nLAN;
	int m_nWIFI;
	int m_nSubVersion;
	CString m_strSubVersion;
	ULONG m_nDevIP;
	USHORT m_nDevPort;
	ULONG m_nSubMask;
	ULONG m_nGateway;
	BYTE m_MAC[6];
public:
	USHORT ReadCfg();
	USHORT WriteCfg();
	USHORT ReadLANCfg();
	USHORT WriteLANCfg();
	void ShowHardInfo();
	void ShowLANInfo();
	void GetValue(ULONG nType,BYTE* pValue);
	//
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMy4CHHardInfoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBtnWirte();
	afx_msg void OnBtnRead();
	afx_msg void OnChkLan();
	afx_msg void OnChkWifi();
	afx_msg void OnBtnReadlan();
	afx_msg void OnBtnWritelan();
	afx_msg void OnSelchangeCombo1();
	afx_msg void OnCheckDds();
	afx_msg void OnCheckAuto();
	afx_msg void OnSelchangeComboBw();
	afx_msg void OnEditchangeCombo1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_4CHHARDINFODLG_H__91C625DF_5A05_420D_BB3F_34C57B061368__INCLUDED_)
