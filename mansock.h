#pragma once

/////////////////////////////////////////////////////////////////////////////
// CManSock command target

class CManSock : public CSocket
{
// Attributes
public:

// Operations
public:
	CManSock();
	virtual ~CManSock();

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CManSock)
	public:
	virtual void OnReceive(int nErrorCode);
	//}}AFX_VIRTUAL

	void SendManamMessage(CString &msg, CString &login, BOOL broadcast);
	
	// Generated message map functions
	//{{AFX_MSG(CManSock)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
private:
	void HandleSysMsg1(TCHAR* sysmsg) const;
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CSendDlg dialog

class CSendDlg : public CDialog
{
// Construction
public:
	CSendDlg(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSendDlg)
	enum { IDD = IDD_MANCHAT };
	BOOL	m_broadcast;
	CString	m_login;
	CString	m_msg;
	CString	m_rcv;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSendDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
//{{AFX_INSERT_LOCATION}}
