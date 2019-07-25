#pragma once

/////////////////////////////////////////////////////////////////////////////
// CManSock command target

class CManSock : public CSocket
{
    // Operations
  public:
    CManSock() = default;
    ~CManSock() override = default;

    void OnReceive(int nErrorCode) override;
    void SendManamMessage(CString& msg, CString& login, BOOL broadcast);

  private:
    static constexpr int MSG_LEN = 1024;
    void HandleSysMsg1(TCHAR* sysmsg) const;
};

class CSendDlg : public CDialog
{
  public:
    CSendDlg(CWnd* pParent = nullptr); // standard constructor

    //{{AFX_DATA(CSendDlg)
    enum { IDD = IDD_MANCHAT };
    CString m_login;
    CString m_msg;
    CString m_rcv;
    BOOL m_broadcast;
    //}}AFX_DATA

  protected:
    DECLARE_MESSAGE_MAP()

    BOOL OnInitDialog() override;
    void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

    //{{AFX_MSG(CSendDlg)
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    //}}AFX_MSG
};
