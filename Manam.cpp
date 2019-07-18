
#include "StdAfx.h"
#include "DrawDoc.h"
#include "DrawView.h"
#include "GenEpsInfoDlg.h"
#include "KolDlg.h"
#include "MainFrm.h"
#include "Manam.h"
#include "SplitFrm.h"

#pragma once

extern BOOL drawErrorBoxes;
extern BOOL disableMenu;

/////////////////////////////////////////////////////////////////////////////
// CDrawApp
IMPLEMENT_DYNAMIC(CDrawApp, CWinAppEx)

BEGIN_MESSAGE_MAP(CDrawApp, CWinAppEx)
    //{{AFX_MSG_MAP(CDrawApp)
    ON_COMMAND(ID_APP_ABOUT, &CDrawApp::OnAppAbout)
    ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, &CDrawApp::OnDisableMenu)
    ON_COMMAND(ID_LOGIN, &CDrawApp::OnLogin)
    ON_UPDATE_COMMAND_UI(ID_LOGIN, &CDrawApp::OnUpdateLogin)
    ON_UPDATE_COMMAND_UI(ID_FILE_DBOPEN, &CDrawApp::OnDisableMenuRDBMS)
    ON_UPDATE_COMMAND_UI(IDM_SENDMSG, &CDrawApp::OnDisableMenuRDBMS)
    ON_COMMAND(ID_FILE_DBOPENRO, &CDrawApp::OnFileDBOpenRO)
    ON_COMMAND(ID_FILE_NEW_BATH, &CDrawApp::OnFileNewBath)
    ON_COMMAND(IDM_PASSWD, &CDrawApp::OnPasswd)
    ON_COMMAND(IDM_ACCESS, &CDrawApp::OnAccess)
    ON_UPDATE_COMMAND_UI(IDM_ACCESS, &CDrawApp::OnUpdateAdmin)
    ON_UPDATE_COMMAND_UI(IDM_DAYDIRS, &CDrawApp::OnUpdateStudio)
    ON_COMMAND(IDM_NEWUSER, &CDrawApp::OnNewuser)
    ON_COMMAND(IDM_CAPTIONS, &CDrawApp::OnCaptions)
    ON_UPDATE_COMMAND_UI(IDM_CAPTIONS, &CDrawApp::OnUpdateCaptions)
    ON_UPDATE_COMMAND_UI(IDV_COMBO_HEAD, &CDrawApp::OnUpdateToolBarCombo)
    ON_UPDATE_COMMAND_UI(IDV_COMBO_COLOR, &CDrawApp::OnUpdateToolBarCombo)
    ON_COMMAND(IDM_NEWTITLE, &CDrawApp::OnNewTitle)
    ON_COMMAND(IDM_DAYDIRS, &CDrawApp::OnDaydirs)
    ON_COMMAND(ID_HELP, &CDrawApp::OnHelp)
    ON_COMMAND(IDM_SENDMSG, &CDrawApp::OnSendmsg)
    ON_COMMAND(ID_FILE_REFRESH, &CDrawApp::OnFileRefresh)
    ON_UPDATE_COMMAND_UI(ID_FILE_DBOPENRO, &CDrawApp::OnDisableMenuRDBMS)
    ON_UPDATE_COMMAND_UI(ID_FILE_NEW, &CDrawApp::OnDisableMenu)
    ON_UPDATE_COMMAND_UI(ID_FILE_NEW_BATH, &CDrawApp::OnDisableMenu)
    ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_SETUP, &CDrawApp::OnDisableMenu)
    ON_UPDATE_COMMAND_UI(ID_FILE_REFRESH, &CDrawApp::OnDisableMenu)
    ON_UPDATE_COMMAND_UI(IDM_PASSWD, &CDrawApp::OnDisableMenuRDBMS)
    ON_UPDATE_COMMAND_UI(IDM_NEWUSER, &CDrawApp::OnUpdateAdmin)
    ON_UPDATE_COMMAND_UI(IDM_NEWTITLE, &CDrawApp::OnUpdateAdmin)
    //}}AFX_MSG_MAP
    // Standard file based document commands
    ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, &CDrawApp::OnFileOpen)
    ON_COMMAND(ID_FILE_DBOPEN, &CDrawApp::OnDBOpen)
    // Standard print setup command
    ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// The one and only CDrawApp object

CDrawApp NEAR theApp;

/////////////////////////////////////////////////////////////////////////////
// CDrawApp construction

CDrawApp::CDrawApp() : m_InternetSession(APP_NAME)
{
    BYTE i, *p = static_cast<BYTE*>(::LockResource(::LoadResource(nullptr, ::FindResource(nullptr, (LPCTSTR)VS_VERSION_INFO, RT_VERSION))));
    for (i = 0, p += 36; *((WORD*)p) == 0 && i < 4; p += 2, i++); // 36 == 3*WORD + sizeof(L"VS_VERSION_INFO")
    auto ffi = reinterpret_cast<VS_FIXEDFILEINFO*>(p);
    m_app_version.Format(_T(" %u.%u.%u.%u"), (ffi->dwFileVersionMS & 0xffff0000) >> 16, (ffi->dwFileVersionMS & 0x0000ffff), (ffi->dwFileVersionLS & 0xffff0000) >> 16, (ffi->dwFileVersionLS & 0x0000ffff));
}

/////////////////////////////////////////////////////////////////////////////
// CDrawApp initialization
BOOL CDrawApp::InitInstance()
{
    CWinAppEx::InitInstance();

    if (!AfxOleInit()) {
        AfxMessageBox(IDP_OLE_INIT_FAILED);
        return FALSE;
    }

    AfxSocketInit();
    AfxEnableControlContainer();
    EnableTaskbarInteraction(FALSE);

    SetRegistryKey(_T("(c) Marcin Buchwald"));

    LoadStdProfileSettings(10);  // Load standard INI file options (including MRU)

    FromIniFile();

    auto pDocTemplate = new CMultiDocTemplate(
        IDR_DRAWCLTYPE,
        RUNTIME_CLASS(CDrawDoc),
        RUNTIME_CLASS(CSplitFrame),
        RUNTIME_CLASS(CDrawView));

    AddDocTemplate(pDocTemplate);

    // create main MDI Frame window
    auto pMainFrame = new CMainFrame();
    if (!pMainFrame->LoadFrame(IDR_MAINFRAME)) {
        AfxMessageBox(_T("Frame window creation failed.\n"));
        disableMenu = FALSE;
        delete pMainFrame;
        return FALSE;
    }
    m_pMainWnd = pMainFrame;

    // enable file manager drag/drop and DDE Execute open
    m_pMainWnd->DragAcceptFiles();
    EnableShellOpen();
    RegisterShellFileTypes();

    // simple command line parsing 
    m_nCmdShow = SW_SHOWMAXIMIZED;
    pMainFrame->ShowWindow(m_nCmdShow);

    // wielki bufor tekstowy 
    bigBuf = static_cast<TCHAR*>(::VirtualAlloc(nullptr, bigSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!bigBuf) {
        AfxMessageBox(_T("Zbyt ma³o pamiêci do uruchomienia aplikacji"), MB_ICONSTOP);
        return FALSE;
    }

    SetScale(CLIENT_SCALE);

    if (!m_sock.Create(m_manam_port = MANAM_DEFAULT_PORT, SOCK_DGRAM))
        if (m_sock.Create(0, SOCK_DGRAM)) {
            CString sAddr;
            m_sock.GetSockName(sAddr, m_manam_port);
        }

    SetRegistryBase(_T("Oracle"));
    if (GetInt(_T("Login"), 1) > 0)
        ConnecttoDB();

    pMainFrame->InsKolorBox();
    InitKratyDrukarnie();

    isOpen = FALSE;
    disableMenu = FALSE;

    if (m_lpCmdLine[0])
        OpenDocumentFile(m_lpCmdLine); 	// open an existing document

    OnIdle(0);  // updates buttons before showing the window
    SetRegistryBase(_T(""));

    return TRUE;
}

int CDrawApp::ExitInstance()
{
    CoUninitialize();
    ::VirtualFree(bigBuf, 0, MEM_RELEASE);
    m_sock.Close();
    m_InternetSession.Close();
    default_mut.TrimLeft();

    SetRegistryBase(_T("General"));
    WriteString(_T("domyslnyTytul"), default_title);
    WriteString(_T("domyslnaMutacja"), default_mut);
    WriteInt(_T("drawErrorBoxes"), drawErrorBoxes);
    WriteInt(_T("exportKratki"), includeKratka);
    WriteInt(_T("podpisGorny"), m_view_top);
    WriteInt(_T("podpisDolny"), m_view_bottom);
    WriteInt(_T("makietujDoKupy"), makietujDoKupy);
    WriteInt(_T("makietujAll"), makietujAll);
    WriteInt(_T("Wydruk"), colsPerPage);
    WriteInt(_T("ShowDeadline"), showDeadline);
    WriteInt(_T("ShowAcDeadline"), showAcDeadline);
    WriteInt(_T("ribbonStyle"), (int)ribbonStyle);
    WriteInt(_T("initZoom"), m_initZoom);

    SetRegistryBase(_T("GenEPS"));
    WriteInt(_T("OpiMode"), (int)isOpiMode);
    WriteInt(_T("ParallelGen"), (int)isParalellGen);

    for (const auto& b : CDrawDoc::brushe)
        delete b;

    return CWinAppEx::ExitInstance();
}

void CDrawApp::AddToRecentFileList(LPCTSTR lpszPathName)
{
    ASSERT_VALID(this);
    ENSURE_ARG(lpszPathName != nullptr);
    ASSERT(AfxIsValidString(lpszPathName));

    if (m_pRecentFileList != nullptr) // do not check if Windows7
        m_pRecentFileList->Add(lpszPathName);
}

void CDrawApp::SetScale(const int scale)
{
    m_vscale = scale;
    m_phight = 78 * m_vscale;
    m_pwidth = 60 * m_vscale;
    m_pmodulx = 12 * m_vscale;
    m_pmoduly = 13 * m_vscale;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// polaczenie z baza /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDrawApp::ConnecttoDB()
{
    /*********** ORACLE *********************/
    CConnDlg connDlg;
    connDlg.m_loginname = GetString(_T("user"), _T(""));
    connDlg.m_passwd = GetString(_T("password"), _T(""));
    if (connDlg.DoModal() != IDOK) return false;

    // odczytaj IP
    char sHostName[256];
    gethostname(sHostName, 255);
    in_addr ia;
    memcpy(&ia, gethostbyname(sHostName)->h_addr_list[0], 4);

    // pobierz MAC adres
    CString mac;
    PMIB_IFROW pifr{nullptr};
    auto pift = (PMIB_IFTABLE)bigBuf;
    ULONG pift_size = bigSize;
    const unsigned int en = GetIfTable(pift, &pift_size, FALSE);
    if (en == NO_ERROR)
        for (unsigned int row = 0; row < pift->dwNumEntries; ++row) {
            pifr = &pift->table[row];
            if (pifr->dwPhysAddrLen == 6 && pifr->dwOperStatus == MIB_IF_OPER_STATUS_OPERATIONAL) {//&& jest polaczona
                mac.Format(_T("%02X%02X%02X%02X%02X%02X"), pifr->bPhysAddr[0], pifr->bPhysAddr[1], pifr->bPhysAddr[2], pifr->bPhysAddr[3], pifr->bPhysAddr[4], pifr->bPhysAddr[5]);
                break;
            }
        }

    // pobierz wersjê programu
    CString sClientInfo;
    m_local_ip = inet_ntoa(ia);
    sClientInfo.Format(_T("%s %u %s%s"), m_local_ip, m_manam_port, APP_NAME, m_app_version);

    int gru_xx;
    CString adapter(en == NO_ERROR && pifr ? reinterpret_cast<char*>(pifr->bDescr) : "VPN?");
    CManODPNETParms orapar {
        { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &gru_xx },
        { CManDbType::DbTypeVarchar2, CManDbDir::ParameterInOut, &sClientInfo },
        { CManDbType::DbTypeVarchar2, &mac },
        { CManDbType::DbTypeVarchar2, &adapter }
    };
    orapar.outParamsCount = 2;

    theManODPNET.EI("begin :gru_xx := init_manam_session(:info,:mac,:adapter); end;", orapar);
    grupa = static_cast<BYTE>(gru_xx);
    if (grupa < 1)
        return false;

#ifndef DEBUG
    if (!connDlg.m_dbtest && !sClientInfo.IsEmpty() && m_app_version.Mid(1).Compare(sClientInfo))
        TryUpgradeImage();
#endif

    CString errinfo;
    const TCHAR* msg{_T("B£¥D")};
    if (connDlg.m_passwd.Find(connDlg.m_loginname) >= 0)
        errinfo = _T("Nie wolno u¿ywaæ has³a domyœlnego");
    else if (connDlg.m_passwd.GetLength() < 4)
        errinfo = _T("Twoje has³o jest za krótkie");
    else {
        if (connDlg.m_dbtest) theManODPNET.m_databaseName += _T("@WORK");
        msg = theManODPNET.m_databaseName.GetBuffer();
    }

    if (!errinfo.IsEmpty()) {
        CManODPNETParms orapar2 { CManDbType::DbTypeVarchar2, &(connDlg.m_loginname + ": " + errinfo) };
        theManODPNET.EI("call dump_error(:msg)", orapar2);
        AfxMessageBox(errinfo + _T(". Zadzwoñ do Centrum i poproœ o jego zmianê lub u¿yj opcji Makieta->Administracja->Zmiana has³a"));
    }

    ((CMainFrame*)m_pMainWnd)->SetLogonStatus(msg);
    return true;
}

void CDrawApp::FromIniFile()
{
    SetRegistryBase(_T("General"));
    m_view_top = GetInt(_T("podpisGorny"), 4);
    m_view_bottom = GetInt(_T("podpisDolny"), 4);
    drawErrorBoxes = (BOOL)GetInt(_T("drawErrorBoxes"), 1);
    includeKratka = (BOOL)GetInt(_T("exportKratki"), 0);
    makietujDoKupy = (BOOL)GetInt(_T("makietujDoKupy"), 1);
    makietujAll = (BOOL)GetInt(_T("makietujAll"), 1);
    colsPerPage = (BOOL)GetInt(_T("Wydruk"), 1);
    showDeadline = (BOOL)GetInt(_T("ShowDeadline"), 0);
    showAcDeadline = (BOOL)GetInt(_T("ShowAcDeadline"), 0);
    default_title = GetString(_T("domyslnyTytul"), _T(""));
    default_mut = GetString(_T("domyslnaMutacja"), _T(""));
    ribbonStyle = GetInt(_T("ribbonStyle"), 0);
    m_initZoom = GetInt(_T("initZoom"), 100);

    SetRegistryBase(_T("GenEPS"));
    isOpiMode = (BOOL)GetInt(_T("OpiMode"), 0);
    isParalellGen = (BOOL)GetInt(_T("ParallelGen"), CGenEpsInfoDlg::GetCpuCnt() > 1 ? 1 : 0);
}

void CDrawApp::InitKratyDrukarnie()
{
    CString bf;
    szpalt_xarr.clear();
    szpalt_yarr.clear();
    kraty.clear();
    if (isRDBMS) {
        int sx, sy;
        theManODPNET.FillArr(&kraty, "select sym from spacer_kratka order by szpalt_x,szpalt_y", CManODPNET::emptyParm);
        for (const auto& k : kraty)
            if (_stscanf_s(k, _T("%ix%i"), &sx, &sy) == 2) {
                szpalt_xarr.push_back(sx);
                szpalt_yarr.push_back(sy);
            }
    } else {
        const int cc = GetProfileInt(_T("Kraty"), _T("Amount"), 0);
        for (int i = 1; i <= cc; ++i) {
            bf.Format(_T("%i"), i);
            kraty.emplace_back(GetProfileString(_T("Kraty"), _T("Krata") + bf, _T("brak")));
            szpalt_xarr.push_back(GetProfileInt(_T("Kraty"), _T("Krata") + bf + _T("x"), -1));
            szpalt_yarr.push_back(GetProfileInt(_T("Kraty"), _T("Krata") + bf + _T("y"), -1));
        }
    }
    // drukarnie
    drukarnie.clear();
    if (isRDBMS) {
        theManODPNET.FillArr(&drukarnie, "select nazwa from spacer_drukarnie where usunieta is null order by xx", CManODPNET::emptyParm);
        // zsylaj¹cy
        zsylajacy.clear();
        theManODPNET.FillArr(&zsylajacy, "select oddzial from space_reservation.zsyla_korekta order by sym", CManODPNET::emptyParm);
        // wydawcy
        wydawcy.clear();
        theManODPNET.FillArr(&wydawcy, "select to_char(xx,'fm000')||sym||' '||nazwa from space_reservation.wydawca where wydaje=1 order by sym", CManODPNET::emptyParm);
    } else {
        const int cc = GetProfileInt(_T("Drukarnie"), _T("Amount"), 0);
        for (int i = 1; i <= cc; ++i) {
            bf.Format(_T("%i"), i);
            drukarnie.emplace_back(GetProfileString(_T("Drukarnie"), _T("Drukarnia") + bf, _T("brak")));
        }
    }
}

void CDrawApp::FillKrataCombo(CComboBox& combo, const int szpalt_x, const int szpalt_y)
{
    for (const auto& k : kraty) {
        const auto ind = combo.AddString(k);
        if (szpalt_xarr[ind] == szpalt_x && szpalt_yarr[ind] == szpalt_y)
            combo.SetCurSel(ind);
    }
}

void CDrawApp::OnDBOpen()
{
    isOpen = TRUE;
    OPENRO = FALSE;
    OnFileNew();
    isOpen = FALSE;
}

// About & Stats
void CDrawApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    if (isRDBMS) {
        CManODPNETParms orapar { CManDbType::DbTypeVarchar2, CManDbDir::ReturnValue, &aboutDlg.m_client };
        theManODPNET.EI("select spacer_user_info() from dual", orapar);
    } else
        aboutDlg.m_client = _T("\nU¿ytkownik nie jest zalogowany do bazy");

    aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CDrawApp commands 

void CDrawApp::OnDisableMenu(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!disableMenu);
}

void CDrawApp::OnDisableMenuRDBMS(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!disableMenu && isRDBMS);
}

void CDrawApp::OnLogin()
{
    /* Pozwala na zalogowanie siê innemu u¿ytkownikowi podczas pracy programu. 	vu */
    if (!SaveAllModified()) return; // zapisz zmiany

    disableMenu = TRUE;
    CMDIChildWnd *vWnd; // pozamykaj okna
    auto mainFrame = reinterpret_cast<CMainFrame*>(m_pMainWnd);
    while (vWnd = mainFrame->MDIGetActive())
        vWnd->DestroyWindow();

    mainFrame->SetLogonStatus(_T("OFFLINE"));

    if (ConnecttoDB())
        mainFrame->SetStatusBarInfo((LPCTSTR)_T("Logowanie zakoñczy³o siê pomyœlnie."));

    InitKratyDrukarnie();
    mainFrame->InsKolorBox();
    disableMenu = FALSE;
}

void CDrawApp::OnUpdateLogin(CCmdUI* pCmdUI)
{
    pCmdUI->SetText((isRDBMS ? (LPCTSTR)_T("&Przelogowanie") : (LPCTSTR)_T("&Logowanie")));
    pCmdUI->Enable(!disableMenu);
}

void CDrawApp::OnFileRefresh()
{
    FileRefresh(nullptr);
}

void CDrawApp::FileRefresh(CDrawDoc* refreshDoc)
{
    CMDIChildWnd *vWnd;	// pozamykaj okna
    CByteArray aIsRO, aSwCZV;
    std::vector<CString> doce, tytuly;

    // zapisz zmiany
    if (refreshDoc ? !refreshDoc->SaveModified() : !SaveAllModified()) return;
    // zapamiêtaj nazwy i zamknij
    while (vWnd = ((CMDIFrameWnd*)m_pMainWnd)->MDIGetActive()) {
        auto vDoc = (CDrawDoc*)((CMDIFrameWnd*)vWnd)->GetActiveDocument();
        if (refreshDoc && refreshDoc != vDoc) {
            ((CMDIFrameWnd*)m_pMainWnd)->MDINext();
            continue;
        }
        if (vDoc->m_mak_xx > 0 || !vDoc->GetPathName().IsEmpty()) {
            aIsRO.Add((BOOL)vDoc->isRO);
            aSwCZV.Add((BYTE)vDoc->swCZV);
            CString d{vDoc->data};
            if (vDoc->iDocType != DocType::makieta_lib) {
                d.SetAt(2, '-'); d.SetAt(5, '-');
            }
            if (vDoc->m_mak_xx < 0 && vDoc->iDocType != DocType::grzbiet_drukowany) // offline
                doce.emplace_back(vDoc->GetPathName());
            else // online 
                doce.emplace_back(vDoc->gazeta + _T("_") + d + CDrawDoc::asDocTypeExt[static_cast<int8_t>(vDoc->iDocType)]);

            tytuly.emplace_back(vDoc->GetTitle());
        }
        vWnd->DestroyWindow();
        if (refreshDoc) break;
    }
    // otwórz ponownie w odwrotnej kolejnoœci
    for (int i = (int)doce.size() - 1; i >= 0; i--) {
        OPENRO = aIsRO[i];
        initCZV = (ToolbarMode)aSwCZV[i];
        if (OpenDocumentFile(doce[i]))
            theApp.activeDoc->SetTitle(tytuly[i]);
    }
    OPENRO = FALSE;
    initCZV = ToolbarMode::normal;
    (grupa&UserRole::dea) ? ((CMDIFrameWnd*)m_pMainWnd)->MDICascade() : ((CMDIFrameWnd*)m_pMainWnd)->MDITile(MDITILE_HORIZONTAL);
}

void CDrawApp::OnFileDBOpenRO()
{
    isOpen = OPENRO = TRUE;
    OnFileNew();
    isOpen = OPENRO = FALSE;
}

void CDrawApp::OnFileNewBath()
{
    if (!theApp.isRDBMS) return;

    CFileDialog dlg(TRUE, _T("txt"), _T(""), OFN_HIDEREADONLY, _T("Definicje edycji makiet (*.txt) |*.txt| Wszystkie pliki (*.*)|*.*||"), nullptr);
    if (dlg.DoModal() != IDOK) return;

    CStdioFile f;
    TCHAR tytul[5], mutacja[5], data[11], wersja[15];

    if (!f.Open(dlg.m_ofn.lpstrFile, CFile::modeRead | CFile::typeText)) return;

    int i = 0;
    BeginWaitCursor();
    while (f.ReadString(bigBuf, bigSize)) {
        if (bigBuf[0] == '#' || bigBuf[0] == '\n') continue;
        if (_stscanf_s(bigBuf, _T("%s\t%s\t%s\t%s"), data, 11, tytul, 5, mutacja, 5, wersja, 15) == 4) {
            CManODPNETParms orapar {
                { CManDbType::DbTypeVarchar2, &data },
                { CManDbType::DbTypeVarchar2, &tytul },
                { CManDbType::DbTypeVarchar2, &mutacja },
                { CManDbType::DbTypeVarchar2, &wersja }
            };
            if (theManODPNET.EI("begin space_reservation.wyprzedzeniowe.create_makieta_offline(:kiedy,:tytul,:mutacja,:wersja); end;", orapar))
                i++;
        }
    }
    EndWaitCursor();
    if (i) {
        ::StringCchPrintf(bigBuf, n_size, _T("%i"), i);
        AfxMessageBox(_T("Utworzono ") + CString(bigBuf) + _T(" makiet."));
    }
}

void CDrawApp::OnPasswd()
{
    if (!isRDBMS) return;

    CPassDlg dlg;
    if (dlg.DoModal() == IDOK) {
        CManODPNETParms orapar { CManDbType::DbTypeVarchar2, &dlg.m_newpass };
        if (theManODPNET.EI("begin set_pass(:pass); end;", orapar))
            AfxMessageBox(_T("Has³o zosta³o zmienione"), MB_ICONINFORMATION);
    }
}

void CDrawApp::OnAccess()
{
    CAccDlg dlg;
yesNext:
    if (dlg.DoModal() == IDOK) {
        BOOL isOK;
        CManODPNETParm komuPar { CManDbType::DbTypeVarchar2, &dlg.m_loginname };
        if (dlg.m_senior.IsEmpty()) {
            CString acc;
            if (dlg.m_racc) acc.AppendChar('R');
            if (dlg.m_sacc) acc.AppendChar('S');
            if (dlg.m_wacc) acc.AppendChar('W');
            if (dlg.m_dacc) acc.AppendChar('D');
            if (dlg.m_pacc) acc.AppendChar('P');
            if (dlg.m_gacc) acc.AppendChar('G');
            if (dlg.m_alltyt)
                dlg.m_tytul = dlg.m_mutacja = _T("%");
            CManODPNETParms orapar {
                komuPar,
                { CManDbType::DbTypeVarchar2, &acc },
                { CManDbType::DbTypeVarchar2, &dlg.m_tytul },
                { CManDbType::DbTypeVarchar2, &dlg.m_mutacja }
            };
            isOK = theManODPNET.EI("begin spacer_grant(:komu,:co,:tytul,:mutacja); end;", orapar);
        } else {
            CManODPNETParms orapar {
                komuPar,
                { CManDbType::DbTypeVarchar2, &dlg.m_senior }
            };
            isOK = theManODPNET.EI("begin spacer_grant_like(:komu,:czyje); end;", orapar);
        }
        if (isOK)
            AfxMessageBox(_T("Uprawnienia zmienione"));
        if (dlg.yesNext)
            goto yesNext;
    }
}

void CDrawApp::OnNewuser()
{
    CUserDlg dlg;
yesNext:
    if (dlg.DoModal() == IDOK) {
        CManODPNETParms orapar {
            { CManDbType::DbTypeVarchar2, &dlg.m_loginname },
            { CManDbType::DbTypeVarchar2, &dlg.m_login_nds },
            { CManDbType::DbTypeVarchar2, &dlg.m_pass },
            { CManDbType::DbTypeVarchar2, &dlg.m_grupa },
            { CManDbType::DbTypeVarchar2, &dlg.m_imie },
            { CManDbType::DbTypeVarchar2, &dlg.m_nazwisko },
            { CManDbType::DbTypeVarchar2, &dlg.m_telefon },
            { CManDbType::DbTypeVarchar2, &dlg.m_uwagi }
        };
        if (theManODPNET.EI("begin spacer_adm.add_spacer_user(:ln,:ln_ds,:pass,:grupa,:imie,:nazw,:tel,:uwagi); end;", orapar))
            AfxMessageBox(_T("Konto utworzone"));
        if (dlg.yesNext) goto yesNext;
    }
}

void CDrawApp::OnUpdateAdmin(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(isRDBMS && (grupa & (UserRole::adm | UserRole::mas)));
}

void CDrawApp::OnUpdateStudio(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(isRDBMS && (grupa & (UserRole::stu | UserRole::adm | UserRole::mas)));
}

void CDrawApp::OnCaptions()
{
    const int lastSet = GetProfileInt(_T("General"), _T("Captions"), 1);
    WriteProfileInt(_T("General"), _T("Captions"), (1 - lastSet) % 2);
    ((CMainFrame*)m_pMainWnd)->IniCaptionBox(0, INT_MAX);
}

void CDrawApp::OnUpdateCaptions(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!disableMenu);
    pCmdUI->SetCheck((BOOL)GetProfileInt(_T("General"), _T("Captions"), 1));
}

void CDrawApp::OnUpdateToolBarCombo(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!disableMenu && theApp.activeDoc != nullptr);
}

void CDrawApp::OnNewTitle()
{
    CNewTitleDlg dlg;
    if (dlg.DoModal() != IDOK) return;

    ::StringCchPrintf(theApp.bigBuf, n_size, _T("call spacer_adm.register_title('%s','%s','%s','%s','%s','%s')"), (LPCTSTR)dlg.m_tytul, dlg.m_mutacja.IsEmpty() ? _T(" ") : (LPCTSTR)dlg.m_mutacja, (LPCTSTR)dlg.m_opis, (LPCTSTR)dlg.m_do_kiedy.Format(c_ctimeData), (LPCTSTR)dlg.m_tytul_upraw, dlg.m_mutacja_upraw.IsEmpty() ? _T(" ") : (LPCTSTR)dlg.m_mutacja_upraw);
    if (!theManODPNET.EI(CStringA(theApp.bigBuf))) {
        AfxMessageBox(_T("Nie uda³o siê zarejestrowaæ tytu³u"), MB_ICONSTOP);
        return;
    }
    if (dlg.m_strona_x != pszpalt_x || dlg.m_strona_y != pszpalt_y) {
        ::StringCchPrintf(theApp.bigBuf, n_size, _T("call spacer_adm.define_rozm_kraty('%s','%s',%i,%i,%i0,%i0,%i0,%i0)"), (LPCTSTR)dlg.m_tytul, dlg.m_mutacja.IsEmpty() ? _T(" ") : (LPCTSTR)dlg.m_mutacja, dlg.m_kra_x, dlg.m_kra_y, dlg.m_strona_x, dlg.m_strona_y, dlg.m_sw_w, dlg.m_sw_h);
        if (!theManODPNET.EI(CStringA(theApp.bigBuf))) {
            AfxMessageBox(_T("Nie uda³o siê zarejestrowaæ kraty"), MB_ICONSTOP);
            return;
        }
    }
    AfxMessageBox(_T("Tytul zarejestrowany"));
}

void CDrawApp::OnDaydirs()
{
    CDirDaysDlg dlg;
    dlg.m_path = GetProfileString(_T("GenEPS"), _T("DayDirsRoot"));
    dlg.DoModal();
}

bool CDrawApp::OpenWebBrowser(const TCHAR* const sUrl)
{
    const auto ret = (size_t)::ShellExecute(::GetDesktopWindow(), _T("Open"), sUrl, nullptr, nullptr, SW_SHOWNORMAL);
    return ret > 32;
}

void CDrawApp::OpenWebBrowser(const size_t service, const TCHAR* const sUrl)
{
    auto pFile = theApp.OpenURL(service, sUrl);
    if (pFile) {
        auto url = pFile->GetFileURL();
        pFile->Close();
        OpenWebBrowser(url);
    }
}

void CDrawApp::SetErrorMessage(LPTSTR lpBuffer)
{
    const auto dw = ::GetLastError();
    ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, dw, LANG_USER_DEFAULT, lpBuffer, n_size, nullptr);
}

std::unique_ptr<CHttpFile> CDrawApp::OpenURL(const size_t service, const CString& sUrl)
{
    if (!isRDBMS) {
        ::MessageBox(nullptr, _T("Wymagane po³¹czenie do bazy danych"), APP_NAME, MB_OK | MB_ICONERROR);
        return nullptr;
    }

    if (m_uriDict.empty())
        theManODPNET.FillArr(&m_uriDict, "select uri from manam_uri order by xx", CManODPNET::emptyParm);

    ASSERT(service < m_uriDict.size());
    BeginWaitCursor();
    std::unique_ptr<CHttpFile> f(reinterpret_cast<CHttpFile*>(this->m_InternetSession.OpenURL(m_uriDict[service] + sUrl)));
    EndWaitCursor();

    if (!f)
        ::MessageBox(nullptr, _T("Nieprawid³owy URL: ") + sUrl, APP_NAME, MB_OK | MB_ICONERROR);

    return f;
}

CTime CDrawApp::ShortDateToCTime(const CString& sData)
{
    // _ttoi stops reading the input string at the first character that it cannot recognize as part of a number
    const auto buf = (LPCTSTR)sData;
    return {_ttoi(buf + 6), _ttoi(buf + 3), _ttoi(buf), 0, 0, 0};
}

void CDrawApp::CTimeToShortDate(const CTime& tData, CString& sData)
{
    sData.Format(c_formatDaty, tData.GetDay(), tData.GetMonth(), tData.GetYear());
}

void CDrawApp::OnHelp()
{
    CDrawApp::OpenWebBrowser(_T("http://gwspacer.agora.pl"));
}

void CDrawApp::OnSendmsg()
{
    CSendDlg dlg;
    if (dlg.DoModal() == IDOK) {
        CString msg{theManODPNET.m_userName + _T(": ") + dlg.m_msg};
        m_sock.SendManamMessage(msg, dlg.m_login, dlg.m_broadcast);
    }
}

bool CDrawApp::TryUpgradeImage() const
{
    CString cmdLine(::GetCommandLine()), sBackupImage;
    cmdLine = cmdLine.Mid(1, cmdLine.Find(_T(".exe")) + 3);
    sBackupImage.Format(_T("%s%s.exe"), cmdLine.Left(cmdLine.GetLength() - 4), m_app_version);
    // nie wiadomo, czy mamy prawo do systemu plików
    if (!::MoveFileEx(cmdLine, sBackupImage, MOVEFILE_REPLACE_EXISTING))
        return false;

    const auto ret = theManODPNET.Deploy(cmdLine);
    if (!ret) ::MoveFile(sBackupImage, cmdLine);

    return ret;
}

/*BOOL CDrawApp::GetIePath(CString& sPath)
{
    HKEY hKey;
    if (RegOpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE"), &hKey) == ERROR_SUCCESS) {
        TCHAR name[255];
        DWORD wNameLen, wValueLen, wType, wInd = 0L;
        do {
            wNameLen = 255; wValueLen = bigSize;
            if (RegEnumValue(hKey, wInd++, name, &wNameLen, NULL, &wType, (unsigned char*)theApp.bigBuf, &wValueLen) != ERROR_SUCCESS)
                break;
        } while (_tcsicmp(name, _T("path")));
        RegCloseKey(hKey);
        wValueLen -= 2;
        if (theApp.bigBuf[wValueLen] == ';')
            theApp.bigBuf[wValueLen] = '\0';
        sPath = theApp.bigBuf;
        return TRUE;
    } else {
        sPath = _T("c:\\program files\\internet explorer");
        return FALSE;
    }
}*/

/*void CDrawApp::ImportManamEps() {
    GetCurrentDirectory(bigSize, theApp.bigBuf);
    CFileDialog fd(TRUE, NULL, NULL, 0, "Eps Files (*.eps)|*.eps|");
    if (fd.DoModal() != IDOK)
        return;
    SetCurrentDirectory(theApp.bigBuf);

    OClob oclob(theApp.m_ODB);
    oclob.CopyFromFile(fd.GetPathName());

    OParameterCollection orapar = theApp.m_ODB.GetParameters();
    orapar.Add("datafile", oclob, OPARAMETER_INVAR, OTYPE_CLOB);
    theManODPNET.EI("update space_reservation.manam_clob set kiedy=sysdate,datafile=:datafile where nazwa='manam.eps'");
    orapar.Remove("datafile");
    oclob.Close();
}*/
