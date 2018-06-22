
#include "StdAfx.h"
#include "GenEpsInfoDlg.h"

IMPLEMENT_DYNAMIC(CGenEpsInfoDlg, CDialog)

WORD CGenEpsInfoDlg::iCpuCnt;
CGenEpsInfoDlg CGenEpsInfoDlg::m_instance;

CGenEpsInfoDlg::CGenEpsInfoDlg()
{
    cancelGenEPS = FALSE;
    hCreationEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (hCreationEvent == nullptr)
        AfxMessageBox(_T("B³¹d inicjalizacji CGenEpsInfoDlg"), MB_ICONERROR);
}

CGenEpsInfoDlg::~CGenEpsInfoDlg()
{
    ::CloseHandle(hCreationEvent);
}

DWORD WINAPI CGenEpsInfoDlg::CreateGenEPSDialog(PMESPUMPDLGARG pArg)
{
    HANDLE hEvent = pArg->hEvent;
    HWND hWnd = ::CreateDialog(nullptr, MAKEINTRESOURCE(CGenEpsInfoDlg::IDD), nullptr, CGenEpsInfoDlg::DialogProc);

    if (hWnd != nullptr)
        ::ShowWindow(hWnd, SW_SHOW);

    pArg->pDlg->Attach(hWnd);
    ::SetEvent(hEvent); //okno gotowe do u¿ycia, pArg zaraz zostanie usuniête ze stosu

    MSG msg;
    BOOL bRet;

    while ((bRet = ::GetMessage(&msg, hWnd, 0, 0)) != 0) {
        if (bRet == -1)
            m_instance.StrInfo(0, CString("Powa¿ny b³¹d"));
        else {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    ::SetEvent(hEvent);
    return 0;
}

INT_PTR CALLBACK CGenEpsInfoDlg::DialogProc(HWND /*unused*/, UINT iMsg, WPARAM wParam, LPARAM /*unused*/)
{
    if (iMsg == WM_COMMAND && wParam == IDCANCEL) {
        m_instance.OnInterruptProcessing();
        return TRUE;
    }

    return FALSE;
}

CGenEpsInfoDlg* CGenEpsInfoDlg::GetGenEpsInfoDlg(BOOL bIsGen)
{
    ASSERT(CGenEpsInfoDlg::m_instance.m_hWnd == nullptr);
    MESPUMPDLGARG dlgArg;
    dlgArg.pDlg = &CGenEpsInfoDlg::m_instance;
    dlgArg.hEvent = CGenEpsInfoDlg::m_instance.hCreationEvent;
    dlgArg.pDlg->cancelGenEPS = FALSE;
    CGenEpsInfoDlg::m_instance.hWorkingThread = ::CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)CGenEpsInfoDlg::CreateGenEPSDialog, &dlgArg, 0, nullptr);
    WaitForSingleObject(CGenEpsInfoDlg::m_instance.hCreationEvent, INFINITE);

    m_instance.m_bIsGen = bIsGen;
    if (!bIsGen)
        CGenEpsInfoDlg::m_instance.SetWindowText(_T("Manam - sprawdzanie materia³ów graficznych"));

    return &CGenEpsInfoDlg::m_instance;
}

void CGenEpsInfoDlg::ReleaseGenEpsInfoDlg(CGenEpsInfoDlg *pDlg)
{
    ::PostMessage(pDlg->m_hWnd, WM_QUIT, 0, 0);
    ::WaitForSingleObject(CGenEpsInfoDlg::m_instance.hCreationEvent, INFINITE);
    ::CloseHandle(pDlg->hWorkingThread);
    CGenEpsInfoDlg::m_instance.Detach();
}

WORD CGenEpsInfoDlg::GetCpuCnt()
{
    if (iCpuCnt > 0) return iCpuCnt;

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return iCpuCnt = (WORD)si.dwNumberOfProcessors;
}

void CGenEpsInfoDlg::SetChannelCount(int iChannelCnt)
{
    CRect dwr, cwr, dtwr;
    CWnd *pChildCtrl;
    CGenEpsInfoDlg::m_instance.GetWindowRect(&dwr);
    ::GetWindowRect(::GetDesktopWindow(), &dtwr);

    if (iChannelCnt > ciMaxChannels)
        iChannelCnt = ciMaxChannels;

    switch (iChannelCnt) {
        case 1: // defined in Manam.rc
            break;
        case 2:
            pChildCtrl = CGenEpsInfoDlg::m_instance.GetDlgItem(IDC_SPA);
            pChildCtrl->GetWindowRect(&cwr);
            ::MapWindowPoints(nullptr, CGenEpsInfoDlg::m_instance.m_hWnd, (LPPOINT)&cwr, 2);
            pChildCtrl->SetWindowPos(&CWnd::wndTop, cwr.left, cwr.top - 45, cwr.Width(), cwr.Height(), SWP_SHOWWINDOW);

            pChildCtrl = CGenEpsInfoDlg::m_instance.GetDlgItem(IDC_A3_PAGE);
            pChildCtrl->GetWindowRect(&cwr);
            ::MapWindowPoints(nullptr, CGenEpsInfoDlg::m_instance.m_hWnd, (LPPOINT)&cwr, 2);
            pChildCtrl->SetWindowPos(&CWnd::wndTop, cwr.left, cwr.top - 45, cwr.Width(), cwr.Height(), SWP_SHOWWINDOW);

            pChildCtrl = CGenEpsInfoDlg::m_instance.GetDlgItem(IDC_OBJETOSC);
            pChildCtrl->GetWindowRect(&cwr);
            ::MapWindowPoints(nullptr, CGenEpsInfoDlg::m_instance.m_hWnd, (LPPOINT)&cwr, 2);
            pChildCtrl->SetWindowPos(&CWnd::wndTop, cwr.left, cwr.top - 45, cwr.Width(), cwr.Height(), SWP_SHOWWINDOW);

            pChildCtrl = CGenEpsInfoDlg::m_instance.GetDlgItem(IDCANCEL);
            pChildCtrl->GetWindowRect(&cwr);
            ::MapWindowPoints(nullptr, CGenEpsInfoDlg::m_instance.m_hWnd, (LPPOINT)&cwr, 2);
            pChildCtrl->SetWindowPos(&CWnd::wndTop, cwr.left, cwr.top + 110, cwr.Width(), cwr.Height(), SWP_SHOWWINDOW);

            CGenEpsInfoDlg::m_instance.SetWindowPos(&CWnd::wndTop, (dtwr.right - dwr.Width()) / 2, (dtwr.bottom - dwr.Height()) / 3, dwr.Width(), 295, SWP_SHOWWINDOW);
    }
    ShowWindow(SW_SHOW);
}

void CGenEpsInfoDlg::OglInfo(int iChannel, const CString& s)
{
    switch (iChannel % ciMaxChannels) {
        case 0:
            SetDlgItemText(IDC_OGLSTR, s); break;
        case 1:
            SetDlgItemText(IDC_OBJETOSC, s); break;
    }
    ShowWindow(SW_SHOW);
}

void CGenEpsInfoDlg::StrInfo(int iChannel, const CString& s)
{
    switch (iChannel % ciMaxChannels) {
        case 1:
            SetDlgItemText(IDC_A3_PAGE, s); break;
        default:
            SetDlgItemText(IDC_EPSSTR, s); break;
    }
    ShowWindow(SW_SHOW);
}

void CGenEpsInfoDlg::OnInterruptProcessing()
{
    cancelGenEPS = TRUE;
    CString s("Praca przerwana");
    StrInfo(0, s);
    StrInfo(1, s);
    ::Sleep(500);
    OnCancel();
}
