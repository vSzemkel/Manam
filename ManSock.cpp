// ManSock.cpp : implementation file
//

#include "stdafx.h"
#include "ManSock.h"
#include "drawadd.h"
#include "drawdoc.h"
#include "mainfrm.h"
#include "manam.h"

static const int MSG_LEN = 1024;

/////////////////////////////////////////////////////////////////////////////
// CManSock

CManSock::CManSock()
{
}

CManSock::~CManSock()
{
}

// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CManSock, CSocket)
    //{{AFX_MSG_MAP(CManSock)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CManSock member functions

void CManSock::OnReceive(int nErrorCode)
{
    TCHAR message[MSG_LEN + 1];
    TCHAR *msg = message;

    int charLen = Receive(msg, MSG_LEN) / sizeof(TCHAR);
    if (msg[0] == 0xfffe) { // java string
        charLen -= 1;
        msg += 1;
        for (int i = 0; i < charLen; ++i)
            msg[i] = ((msg[i] & 0xff00) >> 8) | ((msg[i] & 0xff) << 8);
    }
    msg[charLen] = TCHAR(0);

    CSocket::OnReceive(nErrorCode);

    CWnd *pWnd = AfxGetMainWnd();
    ::ShowWindow(pWnd->m_hWnd, SW_SHOWMINIMIZED);
    ::ShowWindow(pWnd->m_hWnd, SW_SHOWMAXIMIZED);

    FLASHWINFO fi;
    fi.cbSize = sizeof(FLASHWINFO);
    fi.hwnd = pWnd->m_hWnd;
    fi.uCount = 2;
    fi.dwTimeout = 0;
    fi.dwFlags = FLASHW_TRAY;
    ::FlashWindowEx(&fi);

    if (charLen > 9 && !_tcsncmp(msg, _T("DBA: SYS1^"), 10)) {
        if (!theApp.activeDoc) return;
        TCHAR *pos = _tcsstr(&msg[9], _T("^SYS1"));
        if (!pos || pos > msg + MSG_LEN) return;
        *pos = '\0';
        pos += 5;
        ::MessageBox(NULL, pos, APP_NAME, MB_OK | MB_ICONINFORMATION);
        HandleSysMsg1(&msg[10]);
        return;
    }

    CSendDlg dlg;
    dlg.m_rcv = msg;
    if (dlg.DoModal() == IDOK)
        if (theApp.isRDBMS) SendManamMessage(theManODPNET.m_userName + ": " + dlg.m_msg, dlg.m_login, dlg.m_broadcast);
}

void CManSock::SendManamMessage(CString& msg, CString& login, BOOL broadcast)
{
    int num;
    if (broadcast) {
        CManODPNETParms orapar {
            { CManODPNET::DbTypeInt32, CManODPNET::ReturnValue, &num },
            { CManODPNET::DbTypeVarchar2, &msg }
        };
        if (theManODPNET.EI("select space_reservation.manam_msg.send_broadcast(:msg) from dual", orapar)) {
            CString s;
            s.Format(_T("Powiadomienie otrzyma�o %i u�ytkownik�w"), num);
            ::MessageBox(NULL, s, APP_NAME, MB_OK | MB_ICONINFORMATION);
        }
    } else {
        CString ip(' ', 16);
        CManODPNETParms orapar {
            { CManODPNET::DbTypeVarchar2, &login },
            { CManODPNET::DbTypeVarchar2, CManODPNET::ParameterOut, &ip },
            { CManODPNET::DbTypeInt32, CManODPNET::ParameterOut, &num }
        };
        orapar.outParamsCount = 2;
        BOOL doSend = theManODPNET.EI("begin get_manam_socket(:loginname,:ip,:port); end;", orapar);
        if (doSend) SendTo(msg, msg.GetLength() * sizeof(TCHAR), num, ip);
    }
}

void CManSock::HandleSysMsg1(TCHAR *sysmsg) const
{	// asynchroniczna obs�uga spacer.update_reservation
    CDrawAdd *pAdd;
    TCHAR *p, *septok = _T("^");
    int pub_xx = _ttoi(_tcstok(sysmsg, septok));
    auto pMainWnd = reinterpret_cast<CMainFrame*>(theApp.GetMainWnd());
    CMDIChildWnd *pWnd, *pActiveWnd = pMainWnd->MDIGetActive();

    do {
        pAdd = theApp.activeDoc->PubXXExists(pub_xx);
        if (pAdd != nullptr) break;
        pMainWnd->MDINext();
        pWnd = pMainWnd->MDIGetActive();
    } while (pWnd != pActiveWnd);

    if (pAdd == nullptr) {
        ::MessageBox(NULL, _T("Nie odnalezion og�oszenia. Skontaktuj si� ze sprzedawc�, kt�ry wprowadzi� zmian� lub zamknij makiet� bez zapisywania zmian"), APP_NAME, MB_OK | MB_ICONINFORMATION);
        return;
    } else
        pActiveWnd->SetActiveWindow();

    p = _tcstok(NULL, septok);
    int ivar = _ttoi(p);
    UINT arrVal = _ttoi(_tcstok(NULL, septok));
    auto aKolory = &pMainWnd->Spot_ID;
    for (unsigned int i = 0; i < aKolory->size(); ++i)
        if ((*aKolory)[i] == arrVal) {
            arrVal = i;
            break;
        }
    pAdd->kolor = (arrVal << 3) + ivar;
    pAdd->remarks = _tcstok(NULL, septok);
    pAdd->wersja = _tcstok(NULL, septok);
    ivar = _ttol(_tcstok(NULL, septok));
    pAdd->powtorka = ivar ? POWTSEED_1 + ivar * ONEDAY : 0;
    pAdd->oldAdno = _ttol(_tcstok(NULL, septok));

    theApp.activeDoc->SelectAdd(pAdd, FALSE);
}
/////////////////////////////////////////////////////////////////////////////
// CSendDlg dialog


CSendDlg::CSendDlg(CWnd *pParent /*=NULL*/)
    : CDialog(CSendDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CSendDlg)
    m_broadcast = FALSE;
    m_rcv.Empty(); m_msg.Empty(); m_login.Empty();
    //}}AFX_DATA_INIT
}


void CSendDlg::DoDataExchange(CDataExchange *pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSendDlg)
    DDX_Check(pDX, IDC_ALLTYT, m_broadcast);
    DDX_Text(pDX, IDC_LOGINNAME, m_login);
    DDX_Text(pDX, IDC_NAZWA, m_msg);
    DDV_MaxChars(pDX, m_msg, 1023);
    DDX_Text(pDX, IDC_SEKCJA, m_rcv);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSendDlg, CDialog)
    //{{AFX_MSG_MAP(CSendDlg)
    ON_WM_SHOWWINDOW()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CSendDlg::OnInitDialog()
{
    int i;
    if ((i = m_rcv.Find(_T(":"))) > 0)
        m_login = m_rcv.Left(i);
    if (m_login == _T("DBA"))
        GetDlgItem(IDOK)->EnableWindow(FALSE);

    CDialog::OnInitDialog();

    GetDlgItem(IDC_ALLTYT)->EnableWindow(theApp.grupa&R_MAS);

    return TRUE;
}

void CSendDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialog::OnShowWindow(bShow, nStatus);

    if (bShow && !m_rcv.IsEmpty()) GetDlgItem(IDC_NAZWA)->SetFocus();
}
