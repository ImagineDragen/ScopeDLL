// DSOUpdateDlg.h : header file
//

#if !defined(AFX_DSOUPDATEDLG_H__895B5982_B48A_4D42_82E7_8DA4DBA187D5__INCLUDED_)
#define AFX_DSOUPDATEDLG_H__895B5982_B48A_4D42_82E7_8DA4DBA187D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDSOUpdateDlg dialog

class CDSOUpdateDlg : public CDialog
{
// Construction
public:
	CDSOUpdateDlg(CWnd* pParent = NULL);	// standard constructor

//Attributes
public:
	//Update
	int m_nUpdateFPGA; //0：没有升级；1：正在升级；2：正在读取
	UCHAR* m_pUpdateData;
	int m_nCnt;
	CString m_strPoint;
protected:
	HANDLE m_hEvent;//线程信号
	HANDLE m_hThreadProc;//线程
	HANDLE m_hDrawThreadProc;//
private:


//Operates
public:
	BOOL LoadUpdateFile(CString strFileName);
	BOOL SaveUpdateFile();
	void Updating();
	void Reading();
protected:
	static DWORD WINAPI ThreadCollectData(LPVOID lParam);//线程函数
	static DWORD WINAPI ThreadDrawData(LPVOID lParam);//线程函数
private:

public:
// Dialog Data
	//{{AFX_DATA(CDSOUpdateDlg)
	enum { IDD = IDD_DSOUPDATE_DIALOG };
	CProgressCtrl	m_pro;
	CButton	m_btnUpdate;
	CButton	m_btnRead;
	CButton	m_btnBrowser;
	CString	m_strPath;
	CString	m_strInfo;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDSOUpdateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CDSOUpdateDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBtnUpdate();
	afx_msg void OnBtnRead();
	afx_msg void OnBtnBrowser();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSOUPDATEDLG_H__895B5982_B48A_4D42_82E7_8DA4DBA187D5__INCLUDED_)
