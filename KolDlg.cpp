
#include "StdAfx.h"
#include "DrawAdd.h"
#include "DrawDoc.h"
#include "DrawPage.h"
#include "GenEpsInfoDlg.h"
#include "KolDlg.h"
#include "MainFrm.h"
#include "Manam.h"
#include "QueView.h"

/////////////////////////////////////////////////////////////////////////////
// CConnDlg dialog

CConnDlg::CConnDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CConnDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CConnDlg)
    m_dbtest = FALSE;
    //}}AFX_DATA_INIT
}

void CConnDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CConnDlg)
    DDX_Text(pDX, IDC_LOGINNAME, m_loginname);
    DDX_Text(pDX, IDC_PASSWD, m_passwd);
    DDX_Check(pDX, IDC_DBTEST, m_dbtest);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CConnDlg, CDialog)
//{{AFX_MSG_MAP(CConnDlg)
ON_EN_CHANGE(IDC_PASSWD, &CConnDlg::OnChangePasswd)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnDlg message handlers

BOOL CConnDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    GetDlgItem(IDOK)->EnableWindow(!m_passwd.IsEmpty());
    return TRUE;
}

void CConnDlg::OnChangePasswd()
{
    GetDlgItem(IDOK)->EnableWindow(TRUE);
}

void CConnDlg::OnOK()
{
    CDialog::OnOK();
    CString& tns = theApp.m_tnsname;

    TCHAR manamPath[MAX_PATH];
    ::GetModuleFileName(nullptr, manamPath, MAX_PATH);
    auto pos = _tcsrchr(manamPath, _T('\\'));
    ::StringCchCopy(pos + 1, 29, _T("Oracle.ManagedDataAccess.dll"));
    if (::PathFileExistsW(manamPath) == FALSE) {
        AfxMessageBox(_T("Nie odnaleziono klienta Oracle"), MB_ICONSTOP);
        EndDialog(IDABORT);
        return;
    };

    if (m_dbtest) {
        AfxMessageBox(_T("Logujesz siê do bazy testowej. Efekty Twojej pracy zostan¹ utracone"), MB_ICONINFORMATION);
        tns = theApp.GetString(_T("dbtp"));
        if (tns.IsEmpty()) tns.LoadString(IDS_DB_TP);
    } else {
        tns = theApp.GetString(_T("dbpd"));
        if (tns.IsEmpty()) tns.LoadString(IDS_DB_PD);
    }

    theApp.BeginWaitCursor();
    if (!theManODPNET.PrepareConnectionString(m_loginname, m_passwd, tns)) {
        AfxMessageBox(theManODPNET.m_lastErrorMsg, MB_OK | MB_ICONINFORMATION);
        EndDialog(IDABORT);
    } else
        theApp.m_login = m_loginname;

    theApp.WriteString(_T("user"), m_loginname);
    theApp.EndWaitCursor();
}
/////////////////////////////////////////////////////////////////////////////
// CKolDlg dialog

CKolDlg::CKolDlg(CWnd* pParent) : CDialog(CKolDlg::IDD, pParent), m_ile_kolumn(_T("4"))
{
}

void CKolDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CKolDlg)
    DDX_Control(pDX, IDC_ILEKOLUMN, m_combo_box);
    DDX_CBString(pDX, IDC_ILEKOLUMN, m_ile_kolumn);
    DDV_MaxChars(pDX, m_ile_kolumn, 3);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CKolDlg, CDialog)
//{{AFX_MSG_MAP(CKolDlg)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKolDlg message handlers

BOOL CKolDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_combo_box.SelectString(-1, m_ile_kolumn);

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CPageDlg dialog

CPageDlg::CPageDlg(CWnd* pParent /*=NULL*/) : CDialog(CPageDlg::IDD, pParent),
                                              m_topage(1), m_kolor(ColorId::brak), m_deadline(0), m_typ_pary(0), m_drukarnie(0), m_ile_spotow(0), m_nr(0),
                                              m_dervlvl(DervType::none), m_rzymska(FALSE), m_iscaption(FALSE), m_sztywna_kratka(FALSE), m_niemakietuj(FALSE)
{
}

void CPageDlg::OnOK()
{
    if (theApp.swCZV == ToolbarMode::tryb_studia && m_prn_mak.GetCount()) {
        int prn_flag = 0L;
        long lBit = 1L;
        for (int i = 0; i < m_prn_fun.GetCount(); ++i, lBit <<= 1)
            if (m_prn_fun.GetSel(i))
                prn_flag |= lBit;
        CDialog::OnOK();

        CManODPNETParms orapar {
            { CManDbType::DbTypeInt32, &m_mak_xx },
            { CManDbType::DbTypeInt32, &m_id_str },
            { CManDbType::DbTypeInt32, &m_prn_mak_xx },
            { CManDbType::DbTypeInt32, &prn_flag },
            { CManDbType::DbTypeInt32, &m_topage }
        };
        theManODPNET.EI("begin pagina.set_config(:mak_xx,:str_xx,:prn_mak_xx,:prn_flag,:cnt); end;", orapar);
    } else {
        GetDlgItem(m_iscaption ? IDC_CAPTIONCOMBO : IDC_NAMECOMBO)->GetWindowText(m_caption);
        if (m_caption.GetLength() > 32) {
            AfxMessageBox(_T("Nag³ówek nie mo¿e mieæ wiêcej ni¿ 32 znaki"));
            return;
        }
        SetDlgItemText(m_iscaption ? IDC_CAPTION : IDC_NAME, m_caption);
        CDialog::OnOK();
    }

    // kolor
    CString rs;
    m_kolorcombo.GetLBText(m_kolorcombo.GetCurSel(), rs);
    if (rs.CompareNoCase(FULL) == 0)
        m_kolor = ColorId::full;
    else if (rs.CompareNoCase(BRAK) == 0)
        m_kolor = ColorId::brak;
    else
        m_kolor = m_kolorcombo.GetCurSel() * 8 + ColorId::spot;
    // mutred
    m_mutred.Empty();
    int i, rc = m_mutredcombo.GetCount();
    for (i = 0; i < rc; ++i)
        if (m_mutredcombo.GetSel(i)) {
            m_mutredcombo.GetText(i, rs);
            m_mutred += rs;
        }
    // wydawcy
    if ((i = m_wydawcycombo.GetCurSel()) != CB_ERR)
        m_wyd_xx = (int)m_wydawcycombo.GetItemData(i);
    // kratka
    if (!m_sztywna_kratka && (i = m_kratkacombo.GetCurSel()) != CB_ERR) {
        m_szpalt_x = (int)theApp.szpalt_xarr[i];
        m_szpalt_y = (int)theApp.szpalt_yarr[i];
    }

    unsigned long mask = 1L;
    m_drukarnie = 0L;
    rc = m_drukarniecombo.GetCount();
    for (i = 0; i < rc; ++i, mask <<= 1)
        if (m_drukarniecombo.GetSel(i))
            m_drukarnie |= mask;

    m_deadline = CTime(m_deadlineday.GetYear(), m_deadlineday.GetMonth(), m_deadlineday.GetDay(), m_deadline.GetHour(), m_deadline.GetMinute(), 0);
}

void CPageDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPageDlg)
    DDX_Control(pDX, IDC_MUTRED, m_mutredcombo);
    DDX_Control(pDX, IDC_PRINTHOUSE, m_drukarniecombo);
    DDX_Control(pDX, IDC_PRN_FUN, m_prn_fun);
    DDX_Control(pDX, IDC_PRN_MAK, m_prn_mak);
    DDX_Control(pDX, IDC_KRATKA, m_kratkacombo);
    DDX_Control(pDX, IDC_KOLOR, m_kolorcombo);
    DDX_Control(pDX, IDC_NAMECOMBO, m_namecombo);
    DDX_Control(pDX, IDC_CAPTIONCOMBO, m_captioncombo);
    DDX_Text(pDX, IDC_CAPTION, m_caption);
    DDX_Check(pDX, IDC_RZYMSKA, m_rzymska);
    DDX_Text(pDX, IDC_NAME, m_name);
    DDX_Text(pDX, IDC_NR, m_nr);
    DDV_MinMaxInt(pDX, m_nr, 1, 9999);
    DDX_Check(pDX, IDC_NIEMAKIETUJ, m_niemakietuj);
    DDX_Text(pDX, IDC_TOPAGE, m_topage);
    DDV_MinMaxInt(pDX, m_topage, 1, 9999);
    DDX_DateTimeCtrl(pDX, IDC_DEADLINE, m_deadline);
    DDX_Text(pDX, IDC_DERVINFO, m_dervinfo);
    DDX_DateTimeCtrl(pDX, IDC_DEADLINEDAY, m_deadlineday);
    DDX_CBIndex(pDX, IDC_ROZKLAD, m_typ_pary);
    DDX_Control(pDX, IDC_WYDAWCA, m_wydawcycombo);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPageDlg, CDialog)
//{{AFX_MSG_MAP(CPageDlg)
ON_CBN_SELCHANGE(IDC_NAMECOMBO, &CPageDlg::GiveOutStrLog)
ON_CBN_SELCHANGE(IDC_PRN_MAK, &CPageDlg::OnSelchangePrnMak)
ON_BN_CLICKED(IDC_ACDEAD, &CPageDlg::OnBnClickedAcdead)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPageDlg::GiveOutStrLog()
{
    if (theApp.swCZV == ToolbarMode::tryb_studia) return;
    m_namecombo.GetLBText(m_namecombo.GetCurSel(), m_name);
    m_caption = m_name.Right(m_name.GetLength() - m_name.ReverseFind(_T('/')) - 1);
    SetDlgItemText(IDC_CAPTION, m_caption);
}

void CPageDlg::SetFunListBox(const bool setDefaults)
{
    m_prn_fun.ResetContent();

    CManODPNETParms orapar { CManDbType::DbTypeInt32, &m_prn_mak_xx };
    theManODPNET.FillList(&m_prn_fun, "select opis,deflag from makfun where mak_xx=:mak_xx", orapar, 1);

    if (setDefaults) m_prn_flag.SetZero();
    const int rc = m_prn_fun.GetCount();
    for (int i = 0; i < rc; ++i)
        if (setDefaults && m_prn_fun.GetItemData(i)) {
            m_prn_fun.SetSel(i);
            m_prn_flag.SetBit(i);
        } else if (m_prn_flag[i])
            m_prn_fun.SetSel(i);
}

void CPageDlg::OnSelchangePrnMak()
{
    const int ind = m_prn_mak.GetCurSel();
    m_prn_mak_xx = ind != CB_ERR ? (int)m_prn_mak.GetItemData(ind) : 0;
    SetFunListBox(true);
}

void CPageDlg::OnBnClickedAcdead()
{
    CAcDeadDlg dlg;
    if (m_red > 0) {
        dlg.m_red = m_red;
        dlg.m_fot = m_fot;
        dlg.m_kol = m_kol;
    } else
        dlg.m_red = dlg.m_fot = dlg.m_kol = CTime::GetCurrentTime();

    if (dlg.DoModal() == IDOK) {
        m_red = dlg.m_red;
        m_fot = dlg.m_fot;
        m_kol = dlg.m_kol;

        CString sDataRed, sDataFot, sDataKol;
        CDrawApp::CTimeToShortDate(m_red, sDataRed);
        CDrawApp::CTimeToShortDate(m_fot, sDataFot);
        CDrawApp::CTimeToShortDate(m_kol, sDataKol);

        CManODPNETParms orapar {
            { CManDbType::DbTypeInt32, &m_mak_xx },
            { CManDbType::DbTypeInt32, &m_id_str },
            { CManDbType::DbTypeVarchar2, &sDataRed },
            { CManDbType::DbTypeVarchar2, &sDataFot },
            { CManDbType::DbTypeVarchar2, &sDataKol }
        };
        theManODPNET.EI("begin save_ac_deadlines(:mak_xx,:str_xx,:data_red,:data_fot,:data_kol); end;", orapar);
    }
}

BOOL CPageDlg::OnInitDialog()
{
    int i, j;
    m_deadlineday = m_deadline;
    CDialog::OnInitDialog();
    for (i = 1; i < (int)m_ile_spotow + 1; ++i)
        m_kolorcombo.AddString(_T("SPOT ") + CDrawPage::Rzymska(i));
    m_kolorcombo.AddString(BRAK);
    m_kolorcombo.AddString(FULL);

    if (m_kolor == ColorId::brak)
        m_kolorcombo.SetCurSel(m_ile_spotow);
    else if (m_kolor == ColorId::full)
        m_kolorcombo.SetCurSel(m_ile_spotow + 1);
    else
        m_kolorcombo.SetCurSel(m_kolor >> 3);

    // captions
    CComboBox* vCB = m_iscaption ? &m_captioncombo : &m_namecombo;
    if (theApp.swCZV != ToolbarMode::tryb_studia && theApp.activeDoc->isRED == (BOOL)(theApp.GetProfileInt(_T("General"), _T("Captions"), 1)))
        for (j = 0; j < ((CMainFrame*)AfxGetMainWnd())->GetCaptionBoxSize(); ++j) {
            vCB->AddString(((CMainFrame*)AfxGetMainWnd())->GetCaption(j));
            vCB->SetItemData(j, ((CMainFrame*)AfxGetMainWnd())->GetCaptionDataItem(j));
        }

    if (m_iscaption) {
        auto dlgItem = GetDlgItem(IDC_NAMECOMBO);
        dlgItem->EnableWindow(FALSE);
        dlgItem->ShowWindow(SW_HIDE);
        dlgItem = GetDlgItem(IDC_NAME);
        dlgItem->EnableWindow(TRUE);
        dlgItem->ShowWindow(SW_SHOW);
        dlgItem = GetDlgItem(IDC_CAPTIONCOMBO);
        dlgItem->EnableWindow(TRUE);
        dlgItem->ShowWindow(SW_SHOW);
        dlgItem = GetDlgItem(IDC_CAPTION);
        dlgItem->EnableWindow(FALSE);
        dlgItem->ShowWindow(SW_HIDE);

        if (vCB->SelectString(0, m_caption) == CB_ERR) {
            vCB->AddString(m_caption);
            vCB->SelectString(0, m_caption);
        }
    } else
        vCB->SelectString(0, m_name);

    // mutred
    j = m_docmutred.GetLength() / 2;
    for (i = 0; i < j; ++i) {
        CString m = m_docmutred.Mid(2 * i, 2);
        int p = 0, ind = m_mutredcombo.AddString(m);
        while (p >= 0) {
            p = m_mutred.Find(m, p);
            if (p >= 0 && (p | 1) != p) {
                m_mutredcombo.SetSel(ind);
                break;
            }
            if (p > 0) p++;
        }
    }

    // wydawcy
    for (const auto& w : theApp.wydawcy) {
        int iWydXX = _ttoi(w.Left(3));
        j = m_wydawcycombo.AddString(w.Mid(3));
        m_wydawcycombo.SetItemData(j, iWydXX);
        if (iWydXX == m_wyd_xx)
            m_wydawcycombo.SetCurSel(j);
    }

    if (theApp.swCZV == ToolbarMode::tryb_studia && theApp.isRDBMS && (theApp.grupa & (UserRole::stu | UserRole::mas))) {
        // odczytaj ustawienia strony
        int prn_flag;
        CManODPNETParms orapar {
            { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &prn_flag },
            { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &m_prn_mak_xx },
            { CManDbType::DbTypeInt32, &m_mak_xx },
            { CManDbType::DbTypeInt32, &m_id_str }
        };
        orapar.outParamsCount = 2;
        theManODPNET.EI("begin select nvl(prn_flag,0),nvl(prn_mak_xx,0) into :flag,:prn_mak_xx from spacer_strona where mak_xx=:mak_xx and str_xx=:str_xx; end;", orapar);
        m_prn_flag |= prn_flag;

        // inicjalizacja listy dostêpnych pagin
        CManODPNETParms orapar2 {
            { CManDbType::DbTypeInt32, &m_mak_xx },
            { CManDbType::DbTypeInt32, &m_id_str }
        };
        m_prn_mak.SetItemData(m_prn_mak.AddString(_T("Bez paginy")), 0);
        if (!theManODPNET.FillCombo(&m_prn_mak, "select xx,nazwa from makprn where mak_xx=:mak_xx and str_xx=:str_xx", orapar2, 0)) return FALSE;

        // ustaw wybór w combo i liste funkcji
        if (m_prn_mak_xx > 0) {
            const int cnt = m_prn_mak.GetCount();
            for (i = 0; i < cnt; ++i)
                if ((int)m_prn_mak.GetItemData(i) == m_prn_mak_xx) {
                    m_prn_mak.SetCurSel(i);
                    break;
                }
            SetFunListBox(false);
        }
    } else {
        GetDlgItem(IDC_UWAGI)->SetWindowText(_T("Taki sam wydawca dla kolejnych"));
        SetWindowPos(&wndTop, 0, 0, 525, 210, SWP_NOMOVE);
    }

    // ustaw kratki
    theApp.FillKrataCombo(m_kratkacombo, m_szpalt_x, m_szpalt_y);
    if (m_sztywna_kratka)
        GetDlgItem(IDC_KRATKA)->EnableWindow(FALSE);

    // drukarnie
    if (theApp.activeDoc->iDocType != DocType::makieta_lib) {
        for (const auto& d : theApp.drukarnie)
            m_drukarniecombo.AddString(d);
        unsigned long mask = 1L;
        for (i = 0; i < m_drukarniecombo.GetCount(); ++i, mask <<= 1)
            if (m_drukarnie & mask)
                m_drukarniecombo.SetSel(i);
    }

    if (m_dervlvl == DervType::fixd || theApp.activeDoc->iDocType == DocType::grzbiet_drukowany) {
        GetDlgItem(IDOK)->EnableWindow(FALSE);
        GetDlgItem(IDC_PRINTHOUSE)->EnableWindow(FALSE);
    }

    // deadliny dla czasopism tylko dla zachowanych stron
    if (m_id_str == -1) GetDlgItem(IDC_ACDEAD)->ShowWindow(SW_HIDE);

    return TRUE;
} //OnInitDialog

BOOL CAddDlg::matchingOldAdnoUpdate = FALSE;
// CAddDlg dialog
CAddDlg::CAddDlg(CWnd* pParent) : CDialog(CAddDlg::IDD, pParent), vActiveDoc(theApp.activeDoc), m_kolor(ColorId::brak),
                                  m_add_xx(0), m_nag_xx(0), m_spad(0), m_fizpage(0), m_oldadno(0), m_txtposx(0), m_txtposy(0), m_posx(0), m_posy(0),
                                  m_sizex(0), m_sizey(0), m_powt(0), m_studio(1), m_spad_top(FALSE), m_spad_bottom(FALSE), m_spad_left(FALSE),
                                  m_spad_right(FALSE), m_locked(FALSE), m_flaga_rezerw(FALSE), m_rzymnum(FALSE), m_epsok(FALSE), m_always(FALSE),
                                  m_zagroz(FALSE), m_fullView(FALSE), c_height(470), c_narrow(340), c_wide(500)
{
    m_godz_czob = m_data_czob = CTime::GetCurrentTime();
}

void CAddDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    const int base = m_fizpage ? 1 : 0;
    const int ax = m_szpalt_x - m_sizex + 1;
    const int ay = m_szpalt_y - m_sizey + 1;
    //{{AFX_DATA_MAP(CAddDlg)
    DDX_Control(pDX, IDC_POWT, m_powtctrl);
    DDX_Control(pDX, IDC_EMISJE, m_emisjelist);
    DDX_Control(pDX, IDC_TYP_OGL, m_typ_ogl_combo);
    DDX_Control(pDX, IDC_KRATOWE, m_kratowe);
    DDX_Control(pDX, IDC_KRATKA, m_kratkacombo);
    DDX_Control(pDX, IDC_ZAJAWKA, m_zajawkacombo);
    DDX_Control(pDX, IDC_POSY, m_posyedit);
    DDX_Control(pDX, IDC_POSX, m_posxedit);
    DDX_Control(pDX, IDC_FIZPAGE, m_fizpageedit);
    DDX_Text(pDX, IDC_WERSJA, m_wersja);
    DDV_MaxChars(pDX, m_wersja, 5);
    DDX_Control(pDX, IDC_LOCKED, m_checkbox);
    DDX_Control(pDX, IDC_KOLOR, m_kolorcombo);
    DDX_Text(pDX, IDC_LOGPAGE, m_logpage);
    DDV_MaxChars(pDX, m_logpage, 15);
    DDX_Text(pDX, IDC_FIZPAGE, m_fizpage);
    DDV_MinMaxInt(pDX, m_fizpage, -256, 256);
    DDX_Text(pDX, IDC_NAZWA, m_nazwa);
    DDV_MaxChars(pDX, m_nazwa, 150);
    DDX_Text(pDX, IDC_SIZEX, m_sizex);
    DDV_MinMaxInt(pDX, m_sizex, 1, m_szpalt_x);
    DDX_Text(pDX, IDC_SIZEY, m_sizey);
    DDV_MinMaxInt(pDX, m_sizey, 1, m_szpalt_y);
    DDX_Text(pDX, ID_NREPS, m_nreps);
    DDV_MaxChars(pDX, m_nreps, 9);
    DDX_Text(pDX, IDC_POSX, m_posx);
    DDV_MinMaxInt(pDX, m_posx, base, ax);
    DDX_Text(pDX, IDC_POSY, m_posy);
    DDV_MinMaxInt(pDX, m_posy, base, ay);
    DDX_Text(pDX, IDC_TXTPOSX, m_txtposx);
    DDV_MinMaxInt(pDX, m_txtposx, -20, 20);
    DDX_Text(pDX, IDC_TXTPOSY, m_txtposy);
    DDV_MinMaxInt(pDX, m_txtposy, -20, 20);
    DDX_Text(pDX, IDC_REMARKS, m_remarks);
    DDV_MaxChars(pDX, m_remarks, 255);
    DDX_Check(pDX, IDC_LOCKED, m_locked);
    DDX_Check(pDX, IDC_RZYMSKA, m_rzymnum);
    DDX_Check(pDX, IDC_FLAGA_REZERW, m_flaga_rezerw);
    DDX_Text(pDX, ID_SPACER_IDA, m_add_xx);
    DDX_Check(pDX, IDC_ZAGROZONE, m_zagroz);
    DDX_Check(pDX, IDC_GBRAK, m_digital);
    DDX_Check(pDX, IDC_ALWAYS, m_always);
    DDX_Text(pDX, IDC_SPRZEDAL, m_sprzedal);
    DDX_DateTimeCtrl(pDX, IDC_DATA_CZOB, m_data_czob);
    DDX_DateTimeCtrl(pDX, IDC_GODZ_CZOB, m_godz_czob);
    DDX_Check(pDX, IDC_EPSOK, m_epsok);
    DDX_Text(pDX, ID_OLDADNO, m_oldadno);
    DDX_CBIndex(pDX, IDC_STUDIO, m_studio);
    DDX_Text(pDX, IDC_ALIKE, m_uwagi_atex);
    DDX_Check(pDX, IDC_TOP, m_spad_top);
    DDX_Check(pDX, IDC_BOTTOM, m_spad_bottom);
    DDX_Check(pDX, IDC_LEFT, m_spad_left);
    DDX_Check(pDX, IDC_RIGHT, m_spad_right);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddDlg, CDialog)
//{{AFX_MSG_MAP(CAddDlg)
ON_WM_KEYDOWN()
ON_BN_CLICKED(IDC_LOCKED, &CAddDlg::OnLocked)
ON_BN_CLICKED(IDC_KRATOWE, &CAddDlg::OnKratowe)
ON_BN_CLICKED(IDC_ALWAYS, &CAddDlg::OnAlways)
ON_BN_CLICKED(IDEMISJE, &CAddDlg::OnEmisje)
ON_BN_CLICKED(IDDELSEL, &CAddDlg::OnDelsel)
ON_BN_CLICKED(IDDELALL, &CAddDlg::OnDelall)
ON_BN_CLICKED(IDATEX, &CAddDlg::OnAtex)
ON_BN_CLICKED(IDC_CBPOWT, &CAddDlg::OnCbpowt)
ON_BN_CLICKED(IDC_ODBLOKUJ, &CAddDlg::OnBnClickedOdblokuj)
ON_NOTIFY(DTN_DATETIMECHANGE, IDC_POWT, &CAddDlg::OnDtnDatetimechangePowt)
ON_EN_UPDATE(ID_OLDADNO, &CAddDlg::OnEnUpdateOldadno)
ON_EN_CHANGE(IDC_WERSJA, &CAddDlg::OnEnChangeWersja)
ON_CBN_SELCHANGE(IDC_KRATKA, &CAddDlg::OnSelchangeKratka)
ON_CBN_SELCHANGE(IDC_ZAJAWKA, &CAddDlg::OnCbnSelchangeZajawka)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddDlg message handlers
BOOL CAddDlg::OnInitDialog()
{
    m_initAdno = m_nreps;
    m_txtposx -= TXTSHIFT;
    m_txtposy -= TXTSHIFT;
    //spad
    m_spad_right = (m_spad & 1) > 0;
    m_spad_top = (m_spad & 2) > 0;
    m_spad_left = (m_spad & 4) > 0;
    m_spad_bottom = (m_spad & 8) > 0;

    CDialog::OnInitDialog();
    for (const auto& k : CDrawDoc::kolory)
        m_kolorcombo.AddString(k);

    // TODO: Add extra initialization here
    switch (m_kolor) {
        case ColorId::brak:
            m_kolorcombo.SetCurSel(0);
            break;
        case ColorId::full:
            m_kolorcombo.SetCurSel(1);
            break;
        default:
            m_kolorcombo.SetCurSel(m_kolor >> 3);
    }

    if (m_locked && m_fizpage != 0) {
        m_posxedit.SetReadOnly(TRUE);
        m_posyedit.SetReadOnly(TRUE);
        m_fizpageedit.SetReadOnly(TRUE);
    } else {
        m_posxedit.SetReadOnly(FALSE);
        m_posyedit.SetReadOnly(FALSE);
        m_fizpageedit.SetReadOnly(FALSE);
    }

    // ustaw kratki
    theApp.FillKrataCombo(m_kratkacombo, m_szpalt_x, m_szpalt_y);

    // z boku
    if (m_fizpage)
        GetDlgItem(IDC_KRATKA)->EnableWindow(FALSE);
    else {
        GetDlgItem(IDC_POSX)->EnableWindow(FALSE);
        GetDlgItem(IDC_POSY)->EnableWindow(FALSE);
    }

    // dialog otwierany z widoku kolejki ma mieæ nieaktywne wiele opcji
    if (m_fromQue) {
        GetDlgItem(IDC_POSX)->EnableWindow(FALSE);
        GetDlgItem(IDC_POSY)->EnableWindow(FALSE);
        GetDlgItem(IDC_WERSJA)->EnableWindow(FALSE);
        GetDlgItem(IDC_LOCKED)->EnableWindow(FALSE);
        GetDlgItem(IDC_FLAGA_REZERW)->EnableWindow(FALSE);
    }

    // po zaporze wyszarzyæ czasy
    if (m_always) {
        GetDlgItem(IDC_DATA_CZOB)->EnableWindow(FALSE);
        GetDlgItem(IDC_GODZ_CZOB)->EnableWindow(FALSE);
    }
    // blokuj opcje dla og³oszeñ niespacerowanych
    if (m_add_xx < 0 && (theApp.grupa & UserRole::dea))
        GetDlgItem(IDOK)->EnableWindow(FALSE);
    const BOOL enable = m_add_xx > 0 && (theApp.grupa & (UserRole::mas | UserRole::kie | UserRole::dea));
    GetDlgItem(IDEMISJE)->EnableWindow(enable);
    GetDlgItem(IDATEX)->EnableWindow(enable);
    GetDlgItem(IDC_ODBLOKUJ)->EnableWindow(theApp.grupa & (UserRole::mas | UserRole::kie));
    // dla zajawek pobierz listê dopuszczalnych nazw
    OnEnChangeWersja();

    m_wymiarowe = m_typ_xx == 0;
    if (!m_wymiarowe)
        OnSelchangeKratka();

    // inicjuj date powtorki
    int d, m, r;
    if (m_powt == 0 && _stscanf_s(theApp.activeDoc->data, c_formatDaty, &d, &m, &r) == 3) {
        COleDateTime kon(r, m, d, 23, 59, 59);
        kon -= COleDateTimeSpan(1, 0, 0, 0);
        m_powtctrl.SetRange(nullptr, &kon);
    }

    if (m_powt != 0) {
        m_powtctrl.SetRange(&m_powt, &m_powt);
        CDrawApp::CTimeToShortDate(m_powt, m_candidateAdnoDate);
        CheckDlgButton(IDC_CBPOWT, BST_CHECKED);
        GetDlgItem(IDC_POWT)->EnableWindow(TRUE);
        GetDlgItem(ID_OLDADNO)->EnableWindow(TRUE);
        GetDlgItem(IDC_WERSJA)->EnableWindow(FALSE);
    }

    // jesli jest na stale, to tak ma zostac
    if (IsDlgButtonChecked(IDC_ALWAYS))
        GetDlgItem(IDC_ALWAYS)->EnableWindow(FALSE);

    // set broad
    SetWindowPos(&wndTop, 20, 20, c_narrow, c_height, SWP_NOMOVE);

    if (m_derived)
        GetDlgItem(IDOK)->EnableWindow(FALSE);

    static_cast<CComboBox*>(GetDlgItem(IDC_NAGL_RED))->SetCurSel(m_nag_xx - 1);

    return TRUE; // return TRUE  unless you set the focus to a control
}

void CAddDlg::OnEnChangeWersja()
{
    CString sWersja;
    CWnd* wWer = GetDlgItem(IDC_WERSJA);
    wWer->GetWindowText(sWersja);
    const auto isZajwMode = sWersja.Find(_T("z")) >= 0 && m_add_xx == -1;
    m_zajawkacombo.ShowWindow(isZajwMode ? SW_SHOW : SW_HIDE);
    GetDlgItem(IDC_NAZWA)->ShowWindow(isZajwMode ? SW_HIDE : SW_SHOW);
    if (isZajwMode && m_zajawkacombo.GetCount() == 0) {
        GetDlgItem(IDC_KRATKA)->GetWindowText(sWersja);
        CManODPNETParms orapar {
            { CManDbType::DbTypeInt32, &theApp.activeDoc->m_mak_xx },
            { CManDbType::DbTypeVarchar2, &sWersja },
            { CManDbType::DbTypeInt32, &m_sizex },
            { CManDbType::DbTypeInt32, &m_sizey },
            { CManDbType::DbTypeRefCursor, CManDbDir::ParameterOut, nullptr }
        };
        theManODPNET.FillCombo(&m_zajawkacombo, "begin zajawki4mak(:mak_xx,:kra_sym,:sizex,:sizey,:retCur); end;", orapar, 1);

        if (m_zajawkacombo.SelectString(-1, m_nazwa) == LB_ERR) { // nazwa zajawki wprowadzana rêcznie
            m_zajawkacombo.SetItemData(m_zajawkacombo.AddString(m_nazwa), (DWORD)-1L);
            m_zajawkacombo.SetCurSel(m_zajawkacombo.GetCount() - 1);
        }
    }
}

void CAddDlg::OnCbnSelchangeZajawka()
{
    const int cs = m_zajawkacombo.GetCurSel();
    auto zaj_xx = (int)m_zajawkacombo.GetItemData(cs);
    if (zaj_xx < 0) return; // nazwa zajawki wprowadzana rêcznie
    int iPowt = theApp.activeDoc->m_mak_xx;

    CString sAdno = theManODPNET.AdnoDlaZajawki(&zaj_xx, &iPowt);
    if (!sAdno.IsEmpty()) {
        SetDlgItemText(ID_NREPS, sAdno);
        // zaznacz powtorke
        if (iPowt > 0) {
            auto dlgItem = GetDlgItem(ID_OLDADNO);
            dlgItem->EnableWindow();
            matchingOldAdnoUpdate = TRUE;
            SetDlgItemInt(ID_OLDADNO, zaj_xx, FALSE);
            dlgItem->UpdateData();
            matchingOldAdnoUpdate = FALSE;
            CheckDlgButton(IDC_CBPOWT, BST_CHECKED);
            CTime ct{POWTSEED_1 + iPowt * ONEDAY};
            m_powtctrl.SetTime(&ct);
            m_powtctrl.EnableWindow();
        }
    }

    m_zajawkacombo.GetLBText(cs, m_nazwa);
    SetDlgItemText(IDC_NAZWA, m_nazwa);
    m_kolorcombo.SelectString(0, _T("ZIELONY"));
}

void CAddDlg::OnEmisje()
{
    if (!m_fullView) {
        m_fullView = TRUE;
        int add_xx = GetDlgItemInt(ID_SPACER_IDA);
        SetWindowPos(&wndTop, 20, 20, c_wide, c_height, SWP_NOMOVE);
        SetDlgItemText(IDEMISJE, _T("&Powrót"));
        m_emisjelist.ResetContent();

        CManODPNETParms orapar { CManDbType::DbTypeInt32, &add_xx };
        theManODPNET.FillList(&m_emisjelist, "select pub_xx,emisja from spapub where add_xx=:add_xx", orapar, 0);
    } else {
        m_fullView = FALSE;
        SetWindowPos(&wndTop, 20, 20, c_narrow, c_height, SWP_NOMOVE);
        SetDlgItemText(IDEMISJE, _T("&Emisje"));
    }
}

void CAddDlg::OnDelsel()
{
    if (!m_fullView || !m_emisjelist.GetSelCount()) return;
    BOOL refresh = FALSE;
    if (IDYES == AfxMessageBox(_T("Czy usun¹æ zaznaczone emisje tego og³oszenia"), MB_YESNO)) {
        CString s;
        int i, pub_xx, rc = m_emisjelist.GetCount();
        CManODPNETParms orapar { CManDbType::DbTypeInt32, &pub_xx };
        for (i = 0; i < rc; ++i)
            if (m_emisjelist.GetSel(i)) {
                m_emisjelist.GetText(i, s);
                if (s.Find(theApp.activeDoc->data) >= 0) refresh = TRUE;
                pub_xx = (int)m_emisjelist.GetItemData(i);
                theManODPNET.EI("begin spacer.del_pub(:xx); end;", orapar);
            }
        OnCancel();
        if (refresh) theApp.FileRefresh();
    }
}

void CAddDlg::OnDelall()
{
    if (!m_fullView) return;
    if (IDYES == AfxMessageBox(_T("Czy usun¹æ wszystkie emisje tego og³oszenia"), MB_YESNO)) {
        int add_xx = GetDlgItemInt(ID_SPACER_IDA);
        CManODPNETParms orapar { CManDbType::DbTypeInt32, &add_xx };
        theManODPNET.EI("begin spacer.del_all(:xx); end;", orapar);
        OnCancel();
        if (!m_fromQue)
            theApp.FileRefresh();
        else {
            vActiveDoc->m_addsque.erase(std::find(vActiveDoc->m_addsque.begin(), vActiveDoc->m_addsque.end(), CQueView::selected_add));
            vActiveDoc->SetModifiedFlag();
            vActiveDoc->ArrangeQue();
        };
    }
}

void CAddDlg::OnAlways()
{
    const BOOL isClear = !IsDlgButtonChecked(IDC_ALWAYS);
    GetDlgItem(IDC_DATA_CZOB)->EnableWindow(isClear);
    GetDlgItem(IDC_GODZ_CZOB)->EnableWindow(isClear);
    if (isClear) GetDlgItem(IDC_DATA_CZOB)->SetFocus();
}

void CAddDlg::OnKratowe()
{
    m_wymiarowe = !m_wymiarowe;
    if (!m_wymiarowe && !m_typ_ogl_arr.GetSize()) {
        RefreshNiekrat();
        if (!m_typ_ogl_arr.GetSize()) {
            CheckDlgButton(IDC_KRATOWE, BST_UNCHECKED);
            GetDlgItem(IDC_KRATOWE)->EnableWindow(FALSE);
            return;
        }
        m_wymiarowe = FALSE;
    }
    GetDlgItem(IDC_TYP_OGL)->ShowWindow(m_wymiarowe ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_SIZEX)->ShowWindow(m_wymiarowe ? SW_SHOW : SW_HIDE);
    GetDlgItem(IDC_SIZEY)->ShowWindow(m_wymiarowe ? SW_SHOW : SW_HIDE);
    if (m_wymiarowe) m_typ_xx = 0;
}

void CAddDlg::RefreshNiekrat()
{   // ustaw og³oszenia niestandardowe
    if (theApp.isRDBMS)
        theManODPNET.FillNiekratowe(this);
}

void CAddDlg::OnSelchangeKratka()
{
    int i = m_kratkacombo.GetCurSel();
    if (i != CB_ERR) {
        m_szpalt_x = (int)theApp.szpalt_xarr[i];
        m_szpalt_y = (int)theApp.szpalt_yarr[i];
    }
    RefreshNiekrat();
    m_wymiarowe = !m_typ_xx || !m_typ_ogl_arr.GetSize();
    GetDlgItem(IDC_TYP_OGL)->ShowWindow(m_wymiarowe ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_SIZEX)->ShowWindow(m_wymiarowe ? SW_SHOW : SW_HIDE);
    GetDlgItem(IDC_SIZEY)->ShowWindow(m_wymiarowe ? SW_SHOW : SW_HIDE);
    CheckDlgButton(IDC_KRATOWE, !m_wymiarowe);
    GetDlgItem(IDC_KRATOWE)->EnableWindow(m_typ_ogl_arr.GetSize() > 0);
}

void CAddDlg::OnLocked()
{
    m_locked = !m_locked;
    m_checkbox.SetCheck(m_locked);
    if (m_locked && m_fizpage != 0) {
        m_posxedit.SetReadOnly(TRUE);
        m_posyedit.SetReadOnly(TRUE);
        m_fizpageedit.SetReadOnly(TRUE);
    } else {
        m_posxedit.SetReadOnly(FALSE);
        m_posyedit.SetReadOnly(FALSE);
        m_fizpageedit.SetReadOnly(FALSE);
    }
}

void CAddDlg::OnAtex()
{
    CString newAdno;
    GetDlgItemText(ID_NREPS, newAdno);
    if (newAdno.GetLength() < 7)
        GetDlgItem(ID_NREPS)->SetFocus();
    else
        AtexVerify(newAdno);
}

BOOL CAddDlg::UniqueAdno(LPCTSTR adno)
{
    //sprawdz czy ju¿ jest na makiecie - nie krzyczy jesli nowe jest ze spacera, a stare nie
    dupNoSpacer = vActiveDoc->AddExists(_ttol(adno));
    if (dupNoSpacer != nullptr && !(this->m_add_xx > 0 && dupNoSpacer->m_add_xx == -1)) {
        AfxMessageBox(CString("Og³oszenie o numerze ") + adno + " ju¿ jest na tej makiecie", MB_ICONERROR);
        GetDlgItem(ID_NREPS)->SetFocus();
        dupNoSpacer = nullptr;
        return FALSE;
    }
    return TRUE;
}

BOOL CAddDlg::AtexVerify(LPCTSTR adno)
{
    if (!UniqueAdno(adno) || !theManODPNET.CkAccess(vActiveDoc->gazeta, _T("S"))) FALSE;

    CString sAddXX, sURL, sMsg;
    if (m_fullView && m_emisjelist.GetSelCount()) {
        const int rc = m_emisjelist.GetCount();
        CFlag f(0, 0, 1, rc);
        for (int i = 0; i < rc; ++i)
            if (m_emisjelist.GetSel(i))
                f.SetBit(i);
        sURL = f.Print();
        sURL.MakeReverse();
        sURL = "&ordKiedy=" + sURL;
    }
    GetDlgItemText(ID_SPACER_IDA, sAddXX);

    CString sNowy(AfxMessageBox(_T("Nowy materia³ na ka¿d¹ emisjê?"), MB_YESNO | MB_DEFBUTTON2) == IDYES ? "1" : "0");
    if (sNowy == "1") SetDlgItemText(IDC_WERSJA, _T(".n"));
    sURL = "add_xx=" + sAddXX + "&adno=" + adno + "&nowy_mat=" + sNowy + sURL;
    auto pFile = theApp.OpenURL(6, sURL);
    if (!pFile)
        return FALSE;

    auto buf = reinterpret_cast<char*>(theApp.bigBuf);
    while (pFile->ReadString(theApp.bigBuf, bigSize))
        if (!strncmp(buf, "&atex begin", 11)) break;
    while (pFile->ReadString(theApp.bigBuf, bigSize)) {
        if (!strncmp(buf, "&atex end", 9)) {
            if (buf[9] > 10) SetDlgItemText(IDC_ALIKE, CString(buf + 9)); // pobiera uwagi z atexa
            break;
        }
        sMsg += CString(buf);
    }
    pFile->Close();

    if (!sMsg.IsEmpty()) {
        AfxMessageBox(CString(_T("Nie wprowadzono poprawnie danych w ATEXie na emisje z\n")) + sMsg, MB_ICONEXCLAMATION);
        return FALSE;
    }

    m_initAdno = adno;
    CheckDlgButton(IDC_ALWAYS, BST_CHECKED);
    SetDlgItemText(IDC_LOGPAGE, _T(""));
    if (dupNoSpacer != nullptr) { // usuñ emisjê wylan¹ za wczeœnie, dubluj¹c¹ spacer
        dupNoSpacer->m_pub_xx = -1;
        theApp.activeDoc->Remove(dupNoSpacer);
        delete dupNoSpacer;
    }

    return TRUE;
}

void CAddDlg::OnCbpowt()
{
    GetDlgItem(IDC_WERSJA)->EnableWindow(!GetDlgItem(ID_OLDADNO)->EnableWindow(GetDlgItem(IDC_POWT)->EnableWindow(IsDlgButtonChecked(IDC_CBPOWT))));
    if (IsDlgButtonChecked(IDC_CBPOWT)) {
        CTime t;
        ((CDateTimeCtrl*)GetDlgItem(IDC_POWT))->GetTime(t);
        CDrawApp::CTimeToShortDate(t, m_candidateAdnoDate);
    }
}

void CAddDlg::OnKeyDown(const UINT nChar, const UINT nRepCnt, const UINT nFlags)
{
    if (nChar == 's' && (nFlags & KF_ALTDOWN))
        GetDlgItem(IDC_STUDIO)->SetFocus();
    CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CAddDlg::OnBnClickedOdblokuj()
{
    if (theApp.isRDBMS && UpdateData()) {
        auto adno = static_cast<int>(GetDlgItemInt(ID_NREPS));
        if (adno < 0x1000000) return;
        CString tytul(theApp.activeDoc->gazeta.Left(3)), mutacja(theApp.activeDoc->gazeta.Mid(4, 2));
        CManODPNETParms orapar {
            { CManDbType::DbTypeVarchar2, &tytul },
            { CManDbType::DbTypeVarchar2, &mutacja },
            { CManDbType::DbTypeVarchar2, &theApp.activeDoc->data },
            { CManDbType::DbTypeInt32, &adno }
        };
        theManODPNET.EI("begin atex.spacer.odblokuj(:tytul,:mutacja,:kiedy,:adno); end;", orapar);
    }
}

void CAddDlg::OnDtnDatetimechangePowt(NMHDR* pNMHDR, LRESULT* pResult)
{
    auto pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
    m_candidateAdnoDate.Format(c_formatDaty, pDTChange->st.wDay, pDTChange->st.wMonth, pDTChange->st.wYear);
    CString oldAdno;
    auto dlgItem = GetDlgItem(ID_OLDADNO);
    dlgItem->GetWindowText(oldAdno);
    if (oldAdno != this->m_nreps)
        dlgItem->SetWindowText(_T(""));
    *pResult = 0;
}

void CAddDlg::OnEnUpdateOldadno()
{
    static CString lastTypedOldAdno;
    if (matchingOldAdnoUpdate) return;

    if (!m_candidateAdnoDate.IsEmpty()) { // odczytaj kandyduj¹ce do powtórki numery adno
        CManODPNETParms orapar {
            { CManDbType::DbTypeVarchar2, &m_candidateAdnoDate },
            { CManDbType::DbTypeInt32, &m_pub_xx },
            { CManDbType::DbTypeRefCursor, CManDbDir::ParameterOut, nullptr }
        };
        theManODPNET.FillArr(&m_candidateAdno, "begin select_candidate_adno(:kiedy,:pub_xx,:retCur); end;", orapar);
        m_candidateAdnoDate.Empty();
    }

    CString typed;
    auto pebOldAdno = reinterpret_cast<CEdit*>(GetDlgItem(ID_OLDADNO));
    pebOldAdno->GetWindowText(typed);
    if (typed.IsEmpty()) return;

    for (const auto& match : m_candidateAdno)
        if (typed == match.Left(typed.GetLength())) {
            lastTypedOldAdno = typed;
            matchingOldAdnoUpdate = TRUE;
            pebOldAdno->SetWindowText(match);
            pebOldAdno->SetSel(typed.GetLength(), match.GetLength());
            matchingOldAdnoUpdate = FALSE;
            return;
        }

    pebOldAdno->SetWindowText(lastTypedOldAdno);
}

void CAddDlg::OnOK()
{
    // ustaw m_kolor
    switch (m_kolorcombo.GetCurSel()) {
        case 0:
            m_kolor = ColorId::brak;
            break;
        case 1:
            m_kolor = ColorId::full;
            break;
        default:
            m_kolor = (m_kolorcombo.GetCurSel() * 8 + ColorId::spot);
    }
    // ustaw szpalty
    int i = m_kratkacombo.GetCurSel();
    if (!m_fizpage && i != CB_ERR) {
        m_szpalt_x = (int)theApp.szpalt_xarr[i];
        m_szpalt_y = (int)theApp.szpalt_yarr[i];
    }

    // sprawdz czy nie ma powtorki na nreps
    CString newAdno;
    GetDlgItemText(ID_NREPS, newAdno);
    if (newAdno != m_initAdno && !newAdno.IsEmpty())
        if (!UniqueAdno(newAdno)) return;

    // weryfikacja z ATEXem
    if (m_add_xx > 0 && newAdno != m_initAdno && newAdno.GetLength() > 6)
        if (AtexVerify(newAdno) == FALSE)
            SetDlgItemText(ID_NREPS, _T(""));

    // powtorki
    IsDlgButtonChecked(IDC_CBPOWT) ? m_powtctrl.GetTime(m_powt) : m_powt = 0;

    CDialog::OnOK();

    i = m_typ_ogl_combo.GetCurSel();
    if (!m_wymiarowe && i != CB_ERR) {
        m_typ_ogl_combo.GetWindowText(m_kod_modulu);
        m_typ_xx = (int)m_typ_ogl_arr[i];
        m_sizex = (int)m_typ_sizex_arr[i];
        m_sizey = (int)m_typ_sizey_arr[i];
        m_precel_flag = m_typ_precel_arr[i];
    } else {
        m_kod_modulu.Format(_T("%iX%i"), m_sizex, m_sizey);
        m_typ_xx = 0;
    }

    m_txtposx += TXTSHIFT;
    m_txtposy += TXTSHIFT;
    if (m_powt == 0) m_oldadno = -1;
    // spad
    m_spad = 0;
    if (m_spad_right) m_spad += 1;
    if (m_spad_top) m_spad += 2;
    if (m_spad_left) m_spad += 4;
    if (m_spad_bottom) m_spad += 8;
    // nazwa zajawki wpisana rêcznie
    i = m_zajawkacombo.GetCurSel();
    if (m_wersja.Find(_T("z")) >= 0 && i != LB_ERR)
        m_zajawkacombo.GetLBText(i, m_nazwa);
    // nag³ówek redakcyjny
    m_nag_xx = static_cast<CComboBox*>(GetDlgItem(IDC_NAGL_RED))->GetCurSel() + 1;
}

/////////////////////////////////////////////////////////////////////////////
// CInfoDlg dialog
CInfoDlg::CInfoDlg(CWnd* pParent /*=NULL*/) : CDialog(CInfoDlg::IDD, pParent), m_objetosc(0), m_numerrok(0),
                                              m_numer(0), m_drukarnie(0), m_naklad(0), m_modogl(0), m_modred(0), m_modwol(0), m_modrez(0), m_modoglp(0),
                                              m_modwolp(0), m_modrezp(0), m_modredp(0), m_data_deadline(0), m_godz_deadline(0), m_godz_zamkniecia(0),
                                              m_godz_studio(0), m_godz_wykupu(0), m_data_zamkniecia(0), m_data_studio(0), m_data_wykupu(0), m_quecnt(0),
                                              m_modcnt(0), m_set_papier(FALSE), m_set_kraty(FALSE), m_set_deadline(FALSE), m_szyj(FALSE)
{
}

BOOL CInfoDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    int i, modtotal = m_modogl + m_modred + m_modrez + m_modwol;
    TCHAR title[32];
    ::StringCchPrintf(title, 32, _T("Makieta zawiera %i modu³ów"), modtotal);
    if (modtotal < pmodcnt * m_objetosc) {
        const TCHAR sekcja[] = _T(" Sekcja");
        memcpy(title, sekcja, sizeof(sekcja) - sizeof(TCHAR));
    }
    GetDlgItem(IDOK)->EnableWindow(!m_isRO);
    SetDlgItemText(IDC_MODSTATISTICS, title);
    GetDlgItemText(IDC_SIGN_TEXT, title, 3);
    if (title[0] || !(theApp.grupa & UserRole::kie))
        GetDlgItem(IDB_SIGN)->EnableWindow(FALSE);
    GetDlgItemText(ID_DATA_STUDIO, title, 11);
    if (m_grzbiet.GetLength() == 2) { //mutacja grzbietu
        ((CEdit*)GetDlgItem(IDC_SUBDIR))->SetReadOnly(FALSE);
        GetDlgItem(IDC_GNAZWA)->SetWindowText(_T("Zasiêg"));
        GetDlgItem(IDC_SZYJ)->SetWindowText(_T("Przefalc"));
        GetDlgItem(IDC_GUWAGI)->SetWindowText(_T("Grama."));
        ((CEdit*)GetDlgItem(IDC_SIGN_TEXT))->SetReadOnly(FALSE);
        GetDlgItem(IDC_ISSN)->EnableWindow(FALSE);
        GetDlgItem(IDC_CENA)->EnableWindow(FALSE);
        GetDlgItem(IDC_CENA2)->EnableWindow(FALSE);
        GetDlgItem(IDC_NUMERROK)->EnableWindow(FALSE);
        GetDlgItem(IDC_NUMER)->EnableWindow(FALSE);
        GetDlgItem(ID_PROWADZ1)->EnableWindow(FALSE);
        GetDlgItem(ID_PROWADZ2)->EnableWindow(FALSE);
        GetDlgItem(ID_SEKRETARZ)->EnableWindow(FALSE);
        GetDlgItem(ID_KTO_MAKIETUJE)->EnableWindow(FALSE);
        GetDlgItem(IDC_UWAGI)->EnableWindow(FALSE);
        GetDlgItem(IDC_INSERT)->EnableWindow(FALSE);
        GetDlgItem(ID_DATA_WYKUPU)->EnableWindow(FALSE);
        GetDlgItem(ID_DATA_STUDIO)->EnableWindow(FALSE);
        GetDlgItem(ID_DATA_ZAMKNIECIA)->EnableWindow(FALSE);
        GetDlgItem(ID_GODZ_WYKUPU)->EnableWindow(FALSE);
        GetDlgItem(ID_GODZ_STUDIO)->EnableWindow(FALSE);
        GetDlgItem(ID_GODZ_ZAMKNIECIA)->EnableWindow(FALSE);
        GetDlgItem(IDC_DEADLINEDAY)->EnableWindow(FALSE);
    } else
        GetDlgItem(IDC_TYP_PAPIERU)->EnableWindow(FALSE);
    // formatuj ilosc modulow z procentem calosci
    if (modtotal == 0) modtotal = 1;
    ::StringCchPrintf(title, 32, _T("%u [%.1f%%]"), m_modogl, ((float)m_modogl) / modtotal * 100);
    SetDlgItemText(IDC_MODOGL, title);
    ::StringCchPrintf(title, 32, _T("%u [%.1f%%]"), m_modred, ((float)m_modred) / modtotal * 100);
    SetDlgItemText(IDC_MODRED, title);
    ::StringCchPrintf(title, 32, _T("%u [%.1f%%]"), m_modrez, ((float)m_modrez) / modtotal * 100);
    SetDlgItemText(IDC_MODREZ, title);
    ::StringCchPrintf(title, 32, _T("%u [%.1f%%]"), m_modwol, ((float)m_modwol) / modtotal * 100);
    SetDlgItemText(IDC_MODWOL, title);
    // drukarnie
    for (const auto& d : theApp.drukarnie)
        m_drukarniecombo.AddString(d);
    unsigned long mask = 1L;
    for (i = 0; i < m_drukarniecombo.GetCount(); ++i, mask <<= 1)
        if (m_drukarnie & mask)
            m_drukarniecombo.SetSel(i);
    if (m_drukarnie < 0) {
        CRect r;
        GetWindowRect(&r);
        SetWindowPos(&wndTop, 0, 0, r.Width() - 98, r.Height(), SWP_NOMOVE);
    }
    // wydawcy NSS - WWWWW; N={0|1|2} czy jest studio(1) i korekta(2); SS symbol; WWWWW nazwa
    for (const auto& z : theApp.zsylajacy) {
        const auto role = z[0];
        const CString opis{z.Mid(1)};
        if (role != _T('0')) {
            modtotal = m_wydawcycombo.AddString(opis);
            if (z.Find(m_wydawca) == 1)
                m_wydawcycombo.SetCurSel(modtotal);
            if (role == _T('2')) {
                modtotal = m_korektacombo.AddString(opis);
                if (z.Find(m_korekta) == 1)
                    m_korektacombo.SetCurSel(modtotal);
            }
        }

        modtotal = m_wydawcyredcombo.AddString(opis);
        if (z.Find(m_wydawcared) == 1)
            m_wydawcyredcombo.SetCurSel(modtotal);
    }

    return TRUE;
}

void CInfoDlg::OnOK()
{
    int i;
    CDialog::OnOK();
    unsigned long mask = 1L;
    m_drukarnie = 0L;
    for (i = 0; i < m_drukarniecombo.GetCount(); ++i, mask <<= 1)
        if (m_drukarniecombo.GetSel(i))
            m_drukarnie |= mask;
    if ((i = m_wydawcycombo.GetCurSel()) != CB_ERR) {
        m_wydawcycombo.GetLBText(i, m_wydawca);
        m_wydawca.Truncate(2);
    }
    if ((i = m_korektacombo.GetCurSel()) != CB_ERR) {
        m_korektacombo.GetLBText(i, m_korekta);
        m_korekta.Truncate(2);
    }
    if ((i = m_wydawcyredcombo.GetCurSel()) != CB_ERR) {
        m_wydawcyredcombo.GetLBText(i, m_wydawcared);
        m_wydawcared.Truncate(2);
    }
}

void CInfoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CInfoDlg)
    DDX_Text(pDX, ID_DATA, m_data);
    DDV_MaxChars(pDX, m_data, 10);
    DDX_Text(pDX, ID_GAZETA, m_gazeta);
    DDV_MaxChars(pDX, m_gazeta, 8);
    DDX_Text(pDX, ID_PROWADZ1, m_prowadz1);
    DDV_MaxChars(pDX, m_prowadz1, 32);
    DDX_Text(pDX, ID_PROWADZ2, m_prowadz2);
    DDV_MaxChars(pDX, m_prowadz2, 32);
    DDX_Text(pDX, ID_SEKRETARZ, m_sekretarz);
    DDV_MaxChars(pDX, m_sekretarz, 32);
    DDX_Text(pDX, IDC_OBJETOSC, m_objetosc);
    DDX_Text(pDX, IDC_CENA, m_cena);
    DDX_Text(pDX, IDC_CENA2, m_cena2);
    DDX_Text(pDX, IDC_NAKLAD, m_naklad);
    DDX_Text(pDX, IDC_NUMER, m_numer);
    DDX_Text(pDX, IDC_NUMERROK, m_numerrok);
    DDX_Text(pDX, IDC_MODREDP, m_modredp);
    DDX_Text(pDX, IDC_MODREZ2, m_modrezp);
    DDX_Text(pDX, IDC_MODWOLP, m_modwolp);
    DDX_Text(pDX, IDC_MODOGLP, m_modoglp);
    DDX_Text(pDX, IDC_SIGN_TEXT, m_sign_text);
    DDX_DateTimeCtrl(pDX, ID_DATA_WYKUPU, m_data_wykupu);
    DDX_DateTimeCtrl(pDX, ID_DATA_STUDIO, m_data_studio);
    DDX_DateTimeCtrl(pDX, ID_DATA_ZAMKNIECIA, m_data_zamkniecia);
    DDX_DateTimeCtrl(pDX, ID_DATA_DEADLINE, m_data_deadline);
    DDX_DateTimeCtrl(pDX, ID_GODZ_WYKUPU, m_godz_wykupu);
    DDX_DateTimeCtrl(pDX, ID_GODZ_STUDIO, m_godz_studio);
    DDX_DateTimeCtrl(pDX, ID_GODZ_ZAMKNIECIA, m_godz_zamkniecia);
    DDX_DateTimeCtrl(pDX, ID_GODZ_DEADLINE, m_godz_deadline);
    DDX_Text(pDX, IDC_MODCNT, m_modcnt);
    DDX_Check(pDX, IDC_DEADLINE, m_set_deadline);
    DDX_Check(pDX, IDC_KRATKA, m_set_kraty);
    DDX_Check(pDX, IDC_PAPIER, m_set_papier);
    DDX_Text(pDX, IDC_SUBDIR, m_grzbiet);
    DDX_Check(pDX, IDC_SZYJ, m_szyj);
    DDX_Control(pDX, IDC_PRINTHOUSE, m_drukarniecombo);
    DDX_Text(pDX, IDC_UWAGI, m_uwagi);
    DDV_MaxChars(pDX, m_uwagi, 255);
    DDX_Control(pDX, IDC_ISSN, m_wydawcycombo);
    DDX_Control(pDX, IDC_INSERT, m_wydawcyredcombo);
    DDX_Control(pDX, IDC_CBRIP, m_korektacombo);
    DDX_Text(pDX, IDC_PAP, m_opis_papieru);
    DDX_Text(pDX, IDC_QUECNT, m_quecnt);
    DDX_Text(pDX, IDC_WYDAWCA, m_wydaw_str);
    DDX_Text(pDX, ID_KTO_MAKIETUJE, m_kto_makietuje);
    DDX_Text(pDX, ID_POMOC, m_typ_dodatku);
    //}}AFX_DATA_MAP
}

void CInfoDlg::OnSign()
{
    TCHAR buf[30]; // dialog siê pokazuje tylko , gdy jesteœmy po³¹czeni
    GetDlgItemText(IDC_SIGN_TEXT, buf, 30);
    if (buf[0] == TCHAR(0))
        SetDlgItemText(IDC_SIGN_TEXT, theManODPNET.m_userName);
}

void CInfoDlg::OnShowPageDeadlines()
{
    CString sUrl;
    sUrl.Format(_T("m=%i"), theApp.activeDoc->m_mak_xx);
    CDrawApp::OpenWebBrowser(8, sUrl);
}

BEGIN_MESSAGE_MAP(CInfoDlg, CDialog)
//{{AFX_MSG_MAP(CInfoDlg)
ON_BN_CLICKED(IDOK, &CInfoDlg::OnOK)
ON_BN_CLICKED(IDB_SIGN, &CInfoDlg::OnSign)
ON_BN_CLICKED(IDC_DEADLINEDAY, &CInfoDlg::OnShowPageDeadlines)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInfoDlg message handlers
/////////////////////////////////////////////////////////////////////////////
// COpisDlg dialog
COpisDlg::COpisDlg(CWnd* pParent /*=NULL*/) : CDialog(COpisDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(COpisDlg)
    //}}AFX_DATA_INIT
}

void COpisDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(COpisDlg)
    DDX_Text(pDX, IDC_OPIS, m_opis);
    DDX_Check(pDX, IDC_CAPTION, m_Centruj);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COpisDlg, CDialog)
//{{AFX_MSG_MAP(COpisDlg)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZoomDlg message handlers
// ZOOM

CZoomDlg::CZoomDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CZoomDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CZoomDlg)
    m_zoomX = 0;
    //}}AFX_DATA_INIT
}

void CZoomDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CZoomDlg)
    DDX_Text(pDX, IDC_OPIS, m_zoomX);
    DDV_MinMaxInt(pDX, m_zoomX, 100, 750);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CZoomDlg, CDialog)
//{{AFX_MSG_MAP(CZoomDlg)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP() /////////////////////////////////////////////////////////////////////////////
// CDbDlg dialog

CDbDlg::CDbDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CDbDlg::IDD, pParent)
{
    m_dtime = CTime::GetCurrentTime();
    //{{AFX_DATA_INIT(CDbDlg)
    m_pap = theApp.activeDoc->gazeta.Left(3);
    m_mut = theApp.activeDoc->gazeta.Mid(4, 2);
    m_client = m_zakres = FALSE;
    //}}AFX_DATA_INIT
}

void CDbDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDbDlg)
    DDX_Text(pDX, ID_PAP, m_pap);
    DDV_MaxChars(pDX, m_pap, 3);
    DDX_Text(pDX, ID_MUTACJA, m_mut);
    DDV_MaxChars(pDX, m_mut, 4);
    DDX_Text(pDX, ID_STRLOG, m_strlog);
    DDX_Check(pDX, IDC_CLIENT, m_client);
    DDX_DateTimeCtrl(pDX, IDC_DT, m_dtime);
    DDX_Text(pDX, ID_TYTUL, m_alttyt);
    DDX_Text(pDX, ID_MUTACJA2, m_altmut);
    DDX_Text(pDX, ID_STREFA, m_zone);
    DDX_Check(pDX, IDC_ZAKRES, m_zakres);
    //}}AFX_DATA_MAP
}

BOOL CDbDlg::OnInitDialog()
{
    return CDialog::OnInitDialog();
}

void CDbDlg::OnDefineZone()
{
    GetDlgItem(ID_STREFA)->EnableWindow(IsDlgButtonChecked(IDC_ZAKRES));
}

void CDbDlg::OnOK()
{
    CDialog::OnOK();
    m_dt = m_dtime.Format(c_ctimeDataWs);
}

BEGIN_MESSAGE_MAP(CDbDlg, CDialog)
//{{AFX_MSG_MAP(CDbDlg)
ON_BN_CLICKED(IDC_ZAKRES, &CDbDlg::OnDefineZone)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDrzDlg dialog

CDrzDlg::CDrzDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CDrzDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CDrzDlg)
    m_ile_kolumn = _T("4");
    m_id_drw = -1;
    //}}AFX_DATA_INIT
}

void CDrzDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDrzDlg)
    DDX_Control(pDX, IDC_ILEKOLUMN, m_kol_combo);
    DDX_Control(pDX, IDC_DRZEWO, m_drz_combo);
    DDX_CBString(pDX, IDC_ILEKOLUMN, m_ile_kolumn);
    DDV_MaxChars(pDX, m_ile_kolumn, 3);
    DDX_Control(pDX, IDC_LASTEMISION, m_kiedy);
    //}}AFX_DATA_MAP
}

void CDrzDlg::SelectDrzewo(const CTime& t)
{
    CString kiedy = t.Format(c_ctimeData);
    CManODPNETParms orapar {
        { CManDbType::DbTypeVarchar2, &kiedy },
        { CManDbType::DbTypeRefCursor, CManDbDir::ParameterOut, nullptr }
    };

    const int iCurSel = m_drz_combo.GetCurSel();
    m_drz_combo.ResetContent();
    auto cur = ::SetCursor(theApp.LoadStandardCursor(IDC_WAIT));
    if (theManODPNET.FillCombo(&m_drz_combo, "begin open_drzewo_rename(:kiedy,:refCur); end;", orapar, 1) && theApp.activeDoc != nullptr)
        m_drz_combo.SelectString(-1, theApp.activeDoc->gazeta);
    ::SetCursor(cur);
    m_drz_combo.SetCurSel(iCurSel);
}

BOOL CDrzDlg::OnInitDialog()
{
    if (!CDialog::OnInitDialog()) return FALSE;
    const int maxObj = theApp.GetProfileInt(_T("Settings"), _T("MaxObj"), 256);
    CString fstr;
    m_kol_combo.AddString(_T("2"));
    for (int i = 4; i <= maxObj; i += 4) {
        fstr.Format(_T("%i"), i);
        m_kol_combo.AddString(fstr);
    }
    m_kol_combo.SelectString(0, m_ile_kolumn);
    const auto ct = CTime::GetCurrentTime();
    m_kiedy.SetTime(&ct);
    SelectDrzewo(ct);
    m_drz_combo.SetCurSel(0);
    return TRUE;
}

void CDrzDlg::OnKiedyChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    static CTime oldDate = 0;
    auto pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
    CTime t{pDTChange->st};
    if (t != oldDate) {
        oldDate = t;
        SelectDrzewo(t);
    }
    *pResult = 0;
}

void CDrzDlg::OnOK()
{
    CString s;
    m_drz_combo.GetWindowText(s); // GetCurSel nie dzia³a przy podpowiadaniu koñcówek
    int ind = m_drz_combo.SelectString(-1, s);
    if (ind != CB_ERR) {
        m_id_drw = (int)m_drz_combo.GetItemData(ind);
        m_drz_combo.GetLBText(ind, m_gazeta);
        const int pos = m_gazeta.Find(_T(" -"), 0);
        m_gazeta.Truncate(pos);
    }
    CDialog::OnOK();
}

void CDrzDlg::OnCbnEditupdateDrzewo()
{
    static CString sLastTytul;
    CString typed, match;
    auto pebDrzewo = (CComboBox*)GetDlgItem(IDC_DRZEWO);

    pebDrzewo->GetWindowText(typed);
    if (typed.IsEmpty()) return;

    if (pebDrzewo->SelectString(-1, typed) != CB_ERR) {
        pebDrzewo->GetWindowText(match);
        pebDrzewo->SetEditSel(typed.GetLength(), match.GetLength());
        sLastTytul = match;
    } else
        pebDrzewo->SelectString(-1, sLastTytul);
}

BEGIN_MESSAGE_MAP(CDrzDlg, CDialog)
//{{AFX_MSG_MAP(CDrzDlg)
ON_NOTIFY(DTN_DATETIMECHANGE, IDC_LASTEMISION, &CDrzDlg::OnKiedyChanged)
ON_CBN_EDITUPDATE(IDC_DRZEWO, &CDrzDlg::OnCbnEditupdateDrzewo)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDrzDlg message handlers
/////////////////////////////////////////////////////////////////////////////
// CDrz1Dlg dialog
CDrz1Dlg::CDrz1Dlg(CWnd* pParent /*=NULL*/)
    : CDialog(CDrz1Dlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CDrz1Dlg)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    m_id_drw = -1;
}

void CDrz1Dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDrz1Dlg)
    DDX_Control(pDX, IDC_DRZEWO, m_drz_combo);
    //}}AFX_DATA_MAP
}

BOOL CDrz1Dlg::OnInitDialog()
{
    if (!theApp.isRDBMS || !CDialog::OnInitDialog()) return FALSE;

    CManODPNETParms orapar {
        { CManDbType::DbTypeVarchar2, &theApp.activeDoc->data },
        { CManDbType::DbTypeRefCursor, CManDbDir::ParameterOut, nullptr }
    };

    auto cur = ::SetCursor(theApp.LoadStandardCursor(IDC_WAIT));
    if (theManODPNET.FillCombo(&m_drz_combo, "begin open_drzewo_rename(:kiedy,:refCur); end;", orapar, 1))
        m_drz_combo.SelectString(-1, theApp.activeDoc->gazeta);
    ::SetCursor(cur);

    return TRUE;
}

void CDrz1Dlg::OnOK()
{
    const int ind = m_drz_combo.GetCurSel();
    if (ind != CB_ERR) {
        m_id_drw = (int)m_drz_combo.GetItemData(ind);
        m_drz_combo.GetLBText(ind, m_gazeta);
        const int pos = m_gazeta.Find(_T(" -"), 0);
        m_gazeta.Truncate(pos);
    }
    CDialog::OnOK();
}

BEGIN_MESSAGE_MAP(CDrz1Dlg, CDialog)
//{{AFX_MSG_MAP(CDrz1Dlg)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDBOpenDlg dialog
CDBOpenDlg::CDBOpenDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CDBOpenDlg::IDD, pParent)
{
    bEnableCtrls = TRUE;
    m_dtime = CTime::GetCurrentTime();
    //{{AFX_DATA_INIT(CDBOpenDlg)
    m_tytul = theApp.default_title;
    m_mutacja = theApp.default_mut;
    m_doctype = m_okres_ind = 0;
    //}}AFX_DATA_INIT
}

void CDBOpenDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDBOpenDlg)
    DDX_Control(pDX, IDC_WERSJE_LIB, m_wersje_lib);
    DDX_Text(pDX, ID_TYTUL, m_tytul);
    DDV_MaxChars(pDX, m_tytul, 3);
    DDX_Text(pDX, ID_MUTACJA, m_mutacja);
    DDV_MaxChars(pDX, m_mutacja, 2);
    DDX_DateTimeCtrl(pDX, IDC_DT, m_dtime);
    DDX_CBIndex(pDX, IDC_OKRES, m_okres_ind);
    DDX_Control(pDX, IDC_DATY, m_date_combo);
    DDX_Control(pDX, IDC_MUTACJA, m_mutacja_combo);
    DDX_Radio(pDX, IDC_MAKIETA, m_doctype);
    //}}AFX_DATA_MAP
}

void CDBOpenDlg::OnChangeTytul()
{
    if (((CEdit*)GetDlgItem(ID_TYTUL))->LineLength() == 3) {
        ((CEdit*)GetDlgItem(ID_MUTACJA))->SetSel(0, -1);
        bEnableCtrls = FALSE;
        RefreshCombo();
        bEnableCtrls = TRUE;
        GetDlgItem(ID_MUTACJA)->SetFocus();
    } else if (IsDlgButtonChecked(IDC_LIB))
        m_wersje_lib.ResetContent();
    else
        m_date_combo.ResetContent();
}

void CDBOpenDlg::OnChangeMutacja()
{
    if (((CEdit*)GetDlgItem(ID_MUTACJA))->LineLength() == 2) {
        bEnableCtrls = FALSE;
        RefreshCombo();
        bEnableCtrls = TRUE;
    } else if (IsDlgButtonChecked(IDC_LIB))
        m_wersje_lib.ResetContent();
    else
        m_date_combo.ResetContent();
}

void CDBOpenDlg::OnBnClickedAllMut()
{
    if (IsDlgButtonChecked(IDC_MEMSTAT)) {
        bEnableCtrls = FALSE;
        RefreshCombo();
        bEnableCtrls = TRUE;
    } else
        m_mutacja_combo.ResetContent();
}

void CDBOpenDlg::OnOK()
{
    CString s, m;
    if (IsDlgButtonChecked(IDC_LIB))
        GetDlgItemText(IDC_WERSJE_LIB, m_dt);
    CDialog::OnOK();
    if (m_mutacja.IsEmpty()) m_mutacja = _T(' ');
    if (!IsDlgButtonChecked(IDC_LIB))
        m_dt = m_dtime.Format(c_ctimeData);
    const bool ignoreSel = m_date_combo.GetSelCount() == 0;
    const int rc = m_date_combo.GetCount();
    for (int i = 0; i < rc; ++i)
        if (ignoreSel || m_date_combo.GetSel(i)) {
            m_date_combo.GetText(i, s);
            s.SetAt(2, _T('-'));
            s.SetAt(5, _T('-'));
            m_arrDaty.emplace_back(m_tytul + _T(' ') + m_mutacja + _T('_') + s);
            if (IsDlgButtonChecked(IDC_MEMSTAT)) {
                const bool ignoreSelMut = m_mutacja_combo.GetSelCount() == 0;
                const int rc2 = m_mutacja_combo.GetCount();
                for (int j = 0; j < rc2; ++j)
                    if (ignoreSelMut || m_mutacja_combo.GetSel(j)) {
                        m_mutacja_combo.GetText(j, m);
                        m_arrDaty.emplace_back(m_tytul + _T(' ') + m + _T('_') + s);
                    }
            }
        }

    if (rc == 0 && IsDlgButtonChecked(IDC_MEMSTAT)) {
        const bool ignoreSelMut = m_mutacja_combo.GetSelCount() == 0;
        m_dt.SetAt(2, _T('-'));
        m_dt.SetAt(5, _T('-'));
        const int rc2 = m_mutacja_combo.GetCount();
        for (int j = 0; j < rc2; ++j)
            if (ignoreSelMut || m_mutacja_combo.GetSel(j)) {
                m_mutacja_combo.GetText(j, m);
                m_arrDaty.emplace_back(m_tytul + _T(' ') + m + _T('_') + m_dt);
            }
    }
}

void CDBOpenDlg::RefreshCombo()
{
    CWnd* wnOldFocus = GetFocus();
    const BOOL isON = IsDlgButtonChecked(IDC_LIB);
    if (bEnableCtrls) { //aktywacja kontrolek
        GetDlgItem(ID_DATA_TEXT)->ShowWindow(isON ? SW_HIDE : SW_SHOW);
        GetDlgItem(IDC_DT)->ShowWindow(isON ? SW_HIDE : SW_SHOW);
        GetDlgItem(ID_WERSJA_TEXT)->ShowWindow(isON ? SW_SHOW : SW_HIDE);
        GetDlgItem(IDC_WERSJE_LIB)->ShowWindow(isON ? SW_SHOW : SW_HIDE);
        GetDlgItem(IDC_OKRES)->EnableWindow(!isON);
        GetDlgItem(IDC_DATY)->EnableWindow(!isON);
        GetDlgItem(IDC_MUTACJA)->EnableWindow(!isON);
        GetDlgItem(IDC_MEMSTAT)->EnableWindow(!isON);
        isON ? GetDlgItem(IDC_WERSJE_LIB)->SetFocus() : GetDlgItem(IDC_DT)->SetFocus();
    }

    CString tyt, mut;
    GetDlgItemText(ID_TYTUL, tyt);
    GetDlgItemText(ID_MUTACJA, mut);
    if (mut.IsEmpty()) mut = _T(' ');
    CManODPNETParms orapar {
        { CManDbType::DbTypeVarchar2, &tyt },
        { CManDbType::DbTypeVarchar2, &mut }
    };

    if (isON) {
        if (m_wersje_lib.GetCount() == 0)
            theManODPNET.FillCombo(&m_wersje_lib, "select wersja from makieta_lib, drzewo where tytul=:tytul and mutacja=:mutacja and drw_xx=drzewo.xx order by 1", orapar);
    } else { //makieta wyprzedzeniowa lub grzbiet
        CTime t;
        int aIleDni[] = {1, 7, 14, 31, 62, 93, 185, 365};
        m_date_combo.ResetContent();
        m_mutacja_combo.ResetContent();
        int iIleDni = aIleDni[((CComboBox*)GetDlgItem(IDC_OKRES))->GetCurSel()];
        if (iIleDni > 1 || IsDlgButtonChecked(IDC_MEMSTAT)) {
            ((CDateTimeCtrl*)GetDlgItem(IDC_DT))->GetTime(t);
            CString kiedy(t.Format(c_ctimeData));
            int isGrb = IsDlgButtonChecked(IDC_GRZBIET);

            CManODPNETParms orapar2 {
                { CManDbType::DbTypeVarchar2, &tyt },
                { CManDbType::DbTypeVarchar2, &mut },
                { CManDbType::DbTypeVarchar2, &kiedy },
                { CManDbType::DbTypeInt32, &iIleDni },
                { CManDbType::DbTypeInt32, &isGrb },
                { CManDbType::DbTypeRefCursor, CManDbDir::ParameterOut, nullptr },
                { CManDbType::DbTypeRefCursor, CManDbDir::ParameterOut, nullptr }
            };
            CListBox* carr[] = {&m_date_combo, &m_mutacja_combo};
            theManODPNET.FillListArr(carr[0], "begin list_emisje_mutacje2(:tytul,:mutacja,:kiedy,:ileDni,:is_grb,:dateCur,:mutCur); end;", orapar2, IsDlgButtonChecked(IDC_MEMSTAT));
        }
    }

    wnOldFocus->SetFocus();
}

void CDBOpenDlg::OnDtnDatetimechangeDt(NMHDR* pNMHDR, LRESULT* pResult)
{
    static CTime oldDate = 0;
    auto pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
    const CTime t(pDTChange->st);
    if (t != oldDate) {
        oldDate = t;
        RefreshCombo();
    }
    *pResult = 0;
}

BEGIN_MESSAGE_MAP(CDBOpenDlg, CDialog)
//{{AFX_MSG_MAP(CDBOpenDlg)
ON_EN_CHANGE(ID_TYTUL, &CDBOpenDlg::OnChangeTytul)
ON_EN_CHANGE(ID_MUTACJA, &CDBOpenDlg::OnChangeMutacja)
ON_BN_CLICKED(IDC_LIB, &CDBOpenDlg::RefreshCombo)
ON_BN_CLICKED(IDC_GRZBIET, &CDBOpenDlg::RefreshCombo)
ON_BN_CLICKED(IDC_MAKIETA, &CDBOpenDlg::RefreshCombo)
ON_BN_CLICKED(IDC_MEMSTAT, &CDBOpenDlg::OnBnClickedAllMut)
ON_CBN_SELCHANGE(IDC_OKRES, &CDBOpenDlg::RefreshCombo)
ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DT, &CDBOpenDlg::OnDtnDatetimechangeDt)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDBSaveAsDlg dialog
CDBSaveAsDlg::CDBSaveAsDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CDBSaveAsDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CDBSaveAsDlg)
    m_lib = FALSE;
    m_dtime = CTime::GetCurrentTime();
    //}}AFX_DATA_INIT
}

void CDBSaveAsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDBSaveAsDlg)
    DDX_Check(pDX, IDC_LIB, m_lib);
    DDX_DateTimeCtrl(pDX, IDC_DT, m_dtime);
    DDX_Text(pDX, IDC_WERSJA, m_wersja);
    DDV_MaxChars(pDX, m_wersja, 12);
    //}}AFX_DATA_MAP
}

void CDBSaveAsDlg::OnOK()
{
    CDialog::OnOK();
    m_dt = IsDlgButtonChecked(IDC_LIB) ? m_wersja : m_dtime.Format(c_ctimeData);
}

BOOL CDBSaveAsDlg::OnInitDialog()
{
    if (!CDialog::OnInitDialog())
        return FALSE;
    SetDlgItemText(IDC_TYTMUT, m_tytmut);
    OnLib();
    return TRUE;
}

void CDBSaveAsDlg::OnLib()
{
    const BOOL isON = IsDlgButtonChecked(IDC_LIB);
    GetDlgItem(ID_DATA_TEXT)->ShowWindow(isON ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_DT)->ShowWindow(isON ? SW_HIDE : SW_SHOW);
    GetDlgItem(ID_WERSJA_TEXT)->ShowWindow(isON ? SW_SHOW : SW_HIDE);
    GetDlgItem(IDC_WERSJA)->ShowWindow(isON ? SW_SHOW : SW_HIDE);
    isON ? GetDlgItem(IDC_WERSJA)->SetFocus() : GetDlgItem(IDC_DT)->SetFocus();
}

BEGIN_MESSAGE_MAP(CDBSaveAsDlg, CDialog)
//{{AFX_MSG_MAP(CDBSaveAsDlg)
ON_BN_CLICKED(IDC_LIB, &CDBSaveAsDlg::OnLib)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// velvet : CAddDesc dialog - konfigurowanie opisów do og³oszeñ

CAddDesc::CAddDesc(const int top, const int bottom, CWnd* pParent /*=NULL*/)
    : CDialog(CAddDesc::IDD, pParent)
{
    //{{AFX_DATA_INIT(CAddDesc)
    m_top = top;
    m_bottom = bottom;
    //}}AFX_DATA_INIT
}

void CAddDesc::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAddDesc)
    DDX_Radio(pDX, IDC_GNAZWA, m_top);
    DDX_Radio(pDX, IDC_DNAZWA, m_bottom);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddDesc, CDialog)
//{{AFX_MSG_MAP(CAddDesc)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddFindDlg dialog

CAddFindDlg::CAddFindDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CAddFindDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CAddFindDlg)
    m_nreps = 0;
    m_spacer = 0;
    m_pObList = nullptr;
    //}}AFX_DATA_INIT
}

void CAddFindDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAddFindDlg)
    DDX_Text(pDX, IDC_NR, m_nreps);
    DDX_Text(pDX, IDC_NAZWA, m_nazwa);
    DDX_Text(pDX, IDC_SPACER, m_spacer);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddFindDlg, CDialog)
//{{AFX_MSG_MAP(CAddFindDlg)
ON_EN_UPDATE(IDC_NR, &CAddFindDlg::OnUpdateNr)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CAddFindDlg::OnUpdateNr()
{
    static bool matchingUpdate{false};
    if (matchingUpdate) return;
    CString typed, match;
    GetDlgItem(IDC_NR)->GetWindowText(typed);
    if (typed.IsEmpty()) return;

    for (const auto& pObj : *m_pObList)
        if (auto pAdd = dynamic_cast<CDrawAdd*>(pObj)) {
            match.Format(_T("%ld"), pAdd->nreps);
            if (typed == match.Left(typed.GetLength())) {
                matchingUpdate = true;
                auto dlgItem = (CEdit*)GetDlgItem(IDC_NR);
                dlgItem->SetWindowText(match);
                dlgItem->SetSel(typed.GetLength(), match.GetLength());
                matchingUpdate = false;
                return;
            }
        }
}
/////////////////////////////////////////////////////////////////////////////
// CInfoDlgLib dialog

CInfoDlgLib::CInfoDlgLib(CWnd* pParent /*=NULL*/)
    : CDialog(CInfoDlgLib::IDD, pParent)
{
    //{{AFX_DATA_INIT(CInfoDlgLib)
    m_szycie = m_set_papier = FALSE;
    //}}AFX_DATA_INIT
}

void CInfoDlgLib::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CInfoDlgLib)
    DDX_Text(pDX, IDC_OPIS_LIB, m_opis);
    DDV_MaxChars(pDX, m_opis, 255);
    DDX_Text(pDX, IDC_TYTMUT, m_tytmut);
    DDX_Text(pDX, IDC_WERSJA, m_wersja);
    DDX_Check(pDX, IDC_PAPIER, m_set_papier);
    DDX_Text(pDX, IDC_PAP, m_papier);
    DDX_Check(pDX, IDC_SZYCIE, m_szycie);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CInfoDlgLib, CDialog)
//{{AFX_MSG_MAP(CInfoDlgLib)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfDlg dialog
CConfDlg::CConfDlg(CWnd* pParent /*=NULL*/) : CDialog(CConfDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CConfDlg)
    m_iPagesInRow = m_hashing = m_strcnt = 0;
    m_format = m_makietujAll = -1;
    m_opi_mode = m_podwal_subdir = m_subdir = m_makietujDoKupy = m_podwaly = m_copyold = m_daydirs = m_contnum = m_save_dirs = FALSE;
    //}}AFX_DATA_INIT
}

void CConfDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CConfDlg)
    DDX_Text(pDX, IDC_DEADLINE, m_deadline);
    DDV_MaxChars(pDX, m_deadline, 5);
    DDX_Text(pDX, IDC_STRCNT, m_strcnt);
    DDV_MinMaxInt(pDX, m_strcnt, 0, 256);
    DDX_Radio(pDX, IDC_A4_PAGE, m_format);
    DDX_Check(pDX, IDC_MAKDOKUPY, m_makietujDoKupy);
    DDX_Radio(pDX, IDC_MAKOUTER, m_makietujAll);
    DDX_Text(pDX, IDC_HASHING, m_hashing);
    DDX_Check(pDX, IDC_SUBDIR, m_subdir);
    DDX_Check(pDX, IDC_OPIMODE, m_opi_mode);
    DDX_Check(pDX, IDC_SETDEA, m_contnum);
    DDX_Check(pDX, IDC_DAYDIRS, m_daydirs);
    DDX_Check(pDX, IDC_COPYOLDEPS, m_copyold);
    DDX_Check(pDX, IDC_PODWALY, m_podwaly);
    DDX_Check(pDX, IDC_INSERT, m_save_dirs);
    DDX_Text(pDX, IDC_EPS_SRC, m_epssrc);
    DDV_MaxChars(pDX, m_epssrc, MAX_STUDIO_PATH);
    DDX_Text(pDX, IDC_EPS_OLD, m_epsold);
    DDV_MaxChars(pDX, m_epsold, MAX_STUDIO_PATH);
    DDX_Text(pDX, IDC_EPS_DST, m_epsdst);
    DDV_MaxChars(pDX, m_epsdst, MAX_STUDIO_PATH);
    DDX_Text(pDX, IDC_EPS_ZAJ, m_epszaj);
    DDV_MaxChars(pDX, m_epszaj, MAX_STUDIO_PATH);
    DDX_Text(pDX, IDC_EPS_POD, m_epspod);
    DDV_MaxChars(pDX, m_epspod, MAX_STUDIO_PATH);
    DDX_Text(pDX, IDC_EPS_DRO, m_epsdro);
    DDV_MaxChars(pDX, m_epsdro, MAX_STUDIO_PATH);
    DDX_Text(pDX, IDC_EPS_IRF, m_epsirf);
    DDV_MaxChars(pDX, m_epsirf, MAX_STUDIO_PATH);
    DDX_Text(pDX, IDC_PS_DST, m_psdst);
    DDV_MaxChars(pDX, m_psdst, MAX_STUDIO_PATH);
    DDX_Text(pDX, IDC_EPS_UZU, m_epsuzu);
    DDV_MaxChars(pDX, m_epsuzu, MAX_STUDIO_PATH);
    DDX_Text(pDX, IDC_EPS_KOK, m_epskok);
    DDV_MaxChars(pDX, m_epskok, MAX_STUDIO_PATH);
    DDX_Check(pDX, IDC_PODSUBDIR, m_podwal_subdir);
    DDX_CBIndex(pDX, IDC_PAGESPERROW, m_iPagesInRow);
    //}}AFX_DATA_MAP
}

BOOL CConfDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    OnDaydirs();
    GetDlgItem(IDC_SETDEA)->EnableWindow(theApp.grupa & UserRole::kie);
    GetDlgItem(IDC_INSERT)->EnableWindow(!theApp.activeDoc->isRO && (theApp.grupa & UserRole::stu));
    if (theApp.grupa & UserRole::dea) CheckDlgButton(IDC_SETDEA, BST_CHECKED);
    OnBnClickedCopyPodwaly();
    OnBnClickedOpiMode();

    for (int id = IDC_EPS_SRC; id <= IDC_EPS_KOK; ++id)
        ((CMFCEditBrowseCtrl*)GetDlgItem(id))->EnableFolderBrowseButton();

    return TRUE;
}

void CConfDlg::OnSubdir()
{
    GetDlgItem(IDC_HASHING)->EnableWindow(IsDlgButtonChecked(IDC_SUBDIR));
}

void CConfDlg::OnDaydirs()
{
    const BOOL isClear = !IsDlgButtonChecked(IDC_DAYDIRS);
    if (isClear)
        OnSubdir();
    else
        GetDlgItem(IDC_HASHING)->EnableWindow(FALSE);
    GetDlgItem(IDC_COPYOLDEPS)->EnableWindow(!isClear);
    GetDlgItem(IDC_SUBDIR)->EnableWindow(isClear);
    GetDlgItem(IDC_EPS_OLD)->EnableWindow(isClear);
}

void CConfDlg::OnBnClickedCopyPodwaly()
{
    GetDlgItem(IDC_PODSUBDIR)->EnableWindow(IsDlgButtonChecked(IDC_COPYOLDEPS));
}

void CConfDlg::OnBnClickedOpiMode()
{
    const BOOL isOpiUnchecked = !IsDlgButtonChecked(IDC_OPIMODE);
    GetDlgItem(IDC_DAYDIRS)->EnableWindow(isOpiUnchecked);
    GetDlgItem(IDC_INSERT)->EnableWindow(isOpiUnchecked);
    GetDlgItem(IDC_SUBDIR)->EnableWindow(isOpiUnchecked);
    GetDlgItem(IDC_HASHING)->EnableWindow(isOpiUnchecked);
    GetDlgItem(IDC_EPS_SRC)->EnableWindow(isOpiUnchecked);
    GetDlgItem(IDC_EPS_OLD)->EnableWindow(isOpiUnchecked);
    GetDlgItem(IDC_EPS_DST)->EnableWindow(isOpiUnchecked);
    GetDlgItem(IDC_PS_DST)->EnableWindow(isOpiUnchecked);
    GetDlgItem(IDC_EPS_ZAJ)->EnableWindow(isOpiUnchecked);
    GetDlgItem(IDC_EPS_POD)->EnableWindow(isOpiUnchecked);
    GetDlgItem(IDC_EPS_DRO)->EnableWindow(isOpiUnchecked);
    GetDlgItem(IDC_EPS_UZU)->EnableWindow(isOpiUnchecked);
    GetDlgItem(IDC_EPS_IRF)->EnableWindow(isOpiUnchecked);
    GetDlgItem(IDC_EPS_KOK)->EnableWindow(isOpiUnchecked);
    if (!isOpiUnchecked) CheckDlgButton(IDC_DAYDIRS, BST_CHECKED);
}

void CConfDlg::OnOK()
{
    CString olddir;
    GetDlgItemText(IDC_EPS_OLD, olddir);
    if (olddir.Find(_T(":"), 0) >= 0) {
        GetDlgItem(IDC_EPS_OLD)->SetFocus();
        MessageBox(_T("W tej pozycji wpisujemy tylko nazwê podkatalogu"), _T("Informacja"), MB_ICONINFORMATION);
        return;
    }

    CDialog::OnOK();
}

BEGIN_MESSAGE_MAP(CConfDlg, CDialog)
//{{AFX_MSG_MAP(CConfDlg)
ON_BN_CLICKED(IDC_DAYDIRS, &CConfDlg::OnDaydirs)
ON_BN_CLICKED(IDC_SUBDIR, &CConfDlg::OnSubdir)
ON_BN_CLICKED(IDC_COPYOLDEPS, &CConfDlg::OnBnClickedCopyPodwaly)
ON_BN_CLICKED(IDC_OPIMODE, &CConfDlg::OnBnClickedOpiMode)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CPassDlg::CPassDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CPassDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CPassDlg)
    //}}AFX_DATA_INIT
}

void CPassDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPassDlg)
    DDX_Text(pDX, IDC_NEWPASS, m_newpass);
    DDV_MaxChars(pDX, m_newpass, 50);
    DDX_Text(pDX, IDC_NEWPASS2, m_newpass2);
    DDV_MaxChars(pDX, m_newpass2, 50);
    DDX_Text(pDX, IDC_OLDPASS, m_oldpass);
    DDV_MaxChars(pDX, m_oldpass, 50);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPassDlg, CDialog)
//{{AFX_MSG_MAP(CPassDlg)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPassDlg::OnOK()
{
    UpdateData();

    if (m_newpass != m_newpass2)
        AfxMessageBox(_T("Niezgodnoœæ hase³"), MB_ICONSTOP);
    else
        CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CPrnEpsDlg dialog
CPrnEpsDlg::CPrnEpsDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CPrnEpsDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CPrnEpsDlg)
    m_page = -1;
    m_format_padded = 0;
    //m_korekta = TRUE;
    m_exclude_emptypages = m_korekta = m_isprint = m_streamed = m_markfound = m_signall = m_preview = FALSE;
    //}}AFX_DATA_INIT
}

void CPrnEpsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPrnEpsDlg)
    DDX_Control(pDX, IDC_CBRIP, m_cbrip);
    DDX_Control(pDX, IDC_CBDRUK, m_cbdruk);
    DDX_Radio(pDX, IDC_ALLPAGES, m_page);
    DDX_Text(pDX, IDC_FROMPAGE, m_od);
    DDX_Check(pDX, IDC_PREV, m_preview);
    DDX_Text(pDX, IDC_TOPAGE, m_do);
    DDX_Check(pDX, IDC_SIGNALL, m_signall);
    DDX_Radio(pDX, IDC_FMTEPS, m_format_padded);
    DDX_Check(pDX, IDC_MARKFOUND, m_markfound);
    DDX_Check(pDX, IDC_SEND, m_streamed);
    DDX_Check(pDX, IDC_WHERE, m_korekta);
    DDX_Text(pDX, IDC_DWARLOG, m_subset);
    DDX_Check(pDX, IDC_STRCNT, m_exclude_emptypages);
    //}}AFX_DATA_MAP
}

CFlag CPrnEpsDlg::GetChoosenPages(CDrawDoc* pDoc) const noexcept
{
    const auto pc = (int)pDoc->m_pages.size();
    if (m_page == 0) return {1, pc, 1, pc};

    TCHAR* tok;
    int beg, end;
    CFlag wyborStron{0, 0, 1, pc};
    const TCHAR septok[] = _T(",-");

    switch (m_page) {
        case 1:
            beg = pDoc->Nr2NrPorz(m_od);
            end = pDoc->Nr2NrPorz(m_do);
            if (beg < 0 || end < 0) goto subseterr;
            if (end == 0) end = pc;
            if (beg == 0) beg = pc;
            for (int i = beg; i <= end; ++i)
                wyborStron.FlipBit(i % pc);
            break;
        case 2:
            ::StringCchCopy(theApp.bigBuf, n_size, m_subset);
            tok = _tcstok(theApp.bigBuf, septok);
            while (tok != nullptr) {
                beg = pDoc->Nr2NrPorz(tok);
                if (beg == -1)
                    goto subseterr;
                else if (beg == 0)
                    beg = pc;
                switch (m_subset.GetAt((int)(tok - theApp.bigBuf + _tcslen(tok)))) {
                    case '\0':
                    case ',':
                        wyborStron.FlipBit(beg % pc);
                        tok = _tcstok(nullptr, septok);
                        break;
                    case '-':
                        tok = _tcstok(nullptr, septok);
                        end = pDoc->Nr2NrPorz(tok);
                        if (end == -1)
                            goto subseterr;
                        else if (end == 0)
                            end = pc;
                        for (int i = beg; i <= end; ++i)
                            wyborStron.FlipBit(i % pc);
                        tok = _tcstok(nullptr, septok);
                        break;
                    default:
                        goto subseterr;
                }
            }
            break;
        subseterr:
            AfxMessageBox(_T("Nie okreœlono poprawnego podzbioru stron"));
    }

    return wyborStron;
}

BEGIN_MESSAGE_MAP(CPrnEpsDlg, CDialog)
//{{AFX_MSG_MAP(CPrnEpsDlg)
ON_WM_LBUTTONDOWN()
ON_BN_CLICKED(IDC_ALLPAGES, &CPrnEpsDlg::OnAllPages)
ON_BN_CLICKED(IDC_ZAKRES, &CPrnEpsDlg::OnAllPages)
ON_BN_CLICKED(IDC_FMTEPS, &CPrnEpsDlg::OnFmteps)
ON_BN_CLICKED(IDC_FMTPS, &CPrnEpsDlg::OnFmteps)
ON_BN_CLICKED(IDC_FMTPDF, &CPrnEpsDlg::OnFmteps)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPrnEpsDlg::OnAllPages()
{
    const BOOL all = IsDlgButtonChecked(IDC_ALLPAGES);
    GetDlgItem(IDC_FROMPAGE)->EnableWindow(!all);
    GetDlgItem(IDC_TOPAGE)->EnableWindow(!all);
    GetDlgItem(IDC_DWARLOG)->EnableWindow(!all);
}

void CPrnEpsDlg::OnFmteps()
{
    const BOOL bIsEPS = IsDlgButtonChecked(IDC_FMTEPS);
    if (m_isprint) {
        GetDlgItem(IDC_PREV)->EnableWindow(bIsEPS);
        CheckDlgButton(IDC_PREV, bIsEPS ? BST_CHECKED : BST_UNCHECKED);
    }
}

void CPrnEpsDlg::OnLButtonDown(const UINT nFlags, CPoint point)
{
    BOOL odClicked = FALSE;
    CRect odR, doR, ssR, dR, cdR;
    GetWindowRect(dR);
    GetClientRect(cdR);
    const int borderWidth = (dR.Width() - cdR.Width()) / 2;
    GetDlgItem(IDC_FROMPAGE)->GetWindowRect(odR);
    GetDlgItem(IDC_TOPAGE)->GetWindowRect(doR);
    GetDlgItem(IDC_DWARLOG)->GetWindowRect(ssR);
    point.x += dR.left + borderWidth;
    point.y += dR.bottom - cdR.Height() - borderWidth;
    UpdateData(TRUE);
    if ((odClicked = odR.PtInRect(point)) || doR.PtInRect(point))
        m_page = 1;
    else if (ssR.PtInRect(point))
        m_page = 2;
    UpdateData(FALSE);
    OnAllPages();
    GetDlgItem(m_page == 2 ? IDC_DWARLOG : (odClicked ? IDC_FROMPAGE : IDC_TOPAGE))->SetFocus();

    CDialog::OnLButtonDown(nFlags, point);
}

BOOL CPrnEpsDlg::OnInitDialog()
{
    if (m_isprint) m_format_padded = theApp.GetProfileInt(_T("GenEPS"), _T("Format"), 0);
    CDialog::OnInitDialog();
    if (!m_isprint) SetWindowText(_T("Sprawdzanie postscriptu"));
    GetDlgItem(IDC_SEND)->EnableWindow(CGenEpsInfoDlg::GetCpuCnt() > 1);
    GetDlgItem(IDC_MARKFOUND)->EnableWindow(!m_isprint);
    GetDlgItem(IDC_WHERE)->EnableWindow(m_isprint);
    GetDlgItem(IDC_FMTPS)->EnableWindow(m_isprint);
    GetDlgItem(IDC_FMTEPS)->EnableWindow(m_isprint);
    GetDlgItem(IDC_FMTPDF)->EnableWindow(m_isprint);
    GetDlgItem(IDC_SIGNALL)->EnableWindow(m_isprint);
    GetDlgItem(IDC_STRCNT)->EnableWindow(m_isprint);
    GetDlgItem(IDC_PREV)->EnableWindow(m_isprint && IsDlgButtonChecked(IDC_FMTEPS));
    CheckDlgButton(IDC_PREV, IsDlgButtonChecked(IDC_FMTEPS));
    OnAllPages();
    return TRUE;
}

void CPrnEpsDlg::OnOK()
{
    CDialog::OnOK();
    if (m_isprint) theApp.WriteProfileInt(_T("GenEPS"), _T("Format"), m_format_padded);
}

/////////////////////////////////////////////////////////////////////////////
// CUserDlg dialog

CUserDlg::CUserDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CUserDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CUserDlg)
    //}}AFX_DATA_INIT
}

BOOL CUserDlg::OnInitDialog()
{
    yesNext = FALSE;

    CDialog::OnInitDialog();

    return TRUE;
}

void CUserDlg::OnYesnnext()
{
    yesNext = TRUE;
    OnOK();
}

void CUserDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CUserDlg)
    DDX_CBString(pDX, IDC_GRUPA, m_grupa);
    DDX_Text(pDX, IDC_IMIE, m_imie);
    DDV_MaxChars(pDX, m_imie, 20);
    DDX_Text(pDX, IDC_LOGINNAME, m_loginname);
    DDX_Text(pDX, IDC_NAZWISKO, m_nazwisko);
    DDV_MaxChars(pDX, m_nazwisko, 30);
    DDX_Text(pDX, IDC_PASS, m_pass);
    DDX_Text(pDX, IDC_TELEFON, m_telefon);
    DDV_MaxChars(pDX, m_telefon, 13);
    DDX_Text(pDX, IDC_UWAGI, m_uwagi);
    DDV_MaxChars(pDX, m_uwagi, 255);
    DDX_Text(pDX, IDC_ISSN, m_login_nds);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUserDlg, CDialog)
//{{AFX_MSG_MAP(CUserDlg)
ON_BN_CLICKED(IDOK, &CUserDlg::OnZaloz)
ON_BN_CLICKED(IDC_YESNNEXT, &CUserDlg::OnYesnnext)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUserDlg message handlers

void CUserDlg::OnZaloz()
{
    TCHAR* s = theApp.bigBuf;
    int aiGets[6] = {IDC_IMIE, IDC_NAZWISKO, IDC_LOGINNAME, IDC_PASS, IDC_GRUPA, IDC_TELEFON};
    for (int aiGet : aiGets) {
        GetDlgItemText(aiGet, s, 255);
        if (!s[0]) {
            GetDlgItem(aiGet)->SetFocus();
            return;
        }
    }
    CDialog::OnOK();
}
/////////////////////////////////////////////////////////////////////////////
// CAccDlg dialog

CAccDlg::CAccDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CAccDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CAccDlg)
    m_gacc = m_pacc = FALSE;
    //}}AFX_DATA_INIT
}

void CAccDlg::OnOK()
{
    CString s;
    GetDlgItemText(IDC_LOGINNAME, s);
    if (s.IsEmpty()) {
        GetDlgItem(IDC_LOGINNAME)->SetFocus();
        return;
    }
    if (!IsDlgButtonChecked(IDC_ALLTYT) && !IsDlgButtonChecked(IDC_ALIKE)) {
        GetDlgItemText(IDC_TYTUL, s);
        if (s.IsEmpty()) {
            GetDlgItem(IDC_TYTUL)->SetFocus();
            return;
        }
    }
    CDialog::OnOK();
}

void CAccDlg::OnYesnnext()
{
    yesNext = TRUE;
    OnOK();
}

BOOL CAccDlg::OnInitDialog()
{
    m_racc = TRUE;
    m_sacc = m_wacc = m_dacc = m_alltyt = FALSE;
    yesNext = FALSE;

    return CDialog::OnInitDialog();
}

void CAccDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAccDlg)
    DDX_Check(pDX, IDC_ALLTYT, m_alltyt);
    DDX_Check(pDX, IDC_DACC, m_dacc);
    DDX_Text(pDX, IDC_LOGINNAME, m_loginname);
    DDX_Text(pDX, IDC_MUTACJA, m_mutacja);
    DDX_Check(pDX, IDC_RACC, m_racc);
    DDX_Text(pDX, IDC_TYTUL, m_tytul);
    DDX_Check(pDX, IDC_WACC, m_wacc);
    DDX_Text(pDX, IDC_SENIOR, m_senior);
    DDX_Check(pDX, IDC_SACC, m_sacc);
    DDX_Check(pDX, IDC_PACC, m_pacc);
    DDX_Check(pDX, IDC_GACC, m_gacc);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAccDlg, CDialog)
//{{AFX_MSG_MAP(CAccDlg)
ON_BN_CLICKED(IDC_ALLTYT, &CAccDlg::OnAlltyt)
ON_BN_CLICKED(IDC_YESNNEXT, &CAccDlg::OnYesnnext)
ON_BN_CLICKED(IDC_ALIKE, &CAccDlg::OnGrantAlike)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAccDlg message handlers

void CAccDlg::OnGrantAlike()
{
    BOOL isClear = !IsDlgButtonChecked(IDC_ALIKE);
    GetDlgItem(IDC_TYTUL)->EnableWindow(isClear);
    GetDlgItem(IDC_MUTACJA)->EnableWindow(isClear);
    GetDlgItem(IDC_ALLTYT)->EnableWindow(isClear);
    GetDlgItem(IDC_RACC)->EnableWindow(isClear);
    GetDlgItem(IDC_SACC)->EnableWindow(isClear);
    GetDlgItem(IDC_WACC)->EnableWindow(isClear);
    GetDlgItem(IDC_DACC)->EnableWindow(isClear);
    GetDlgItem(IDC_PACC)->EnableWindow(isClear);
    GetDlgItem(IDC_SENIOR)->EnableWindow(!isClear);
    if (isClear)
        SetDlgItemText(IDC_SENIOR, _T(""));
}

void CAccDlg::OnAlltyt()
{
    BOOL isClear = !IsDlgButtonChecked(IDC_ALLTYT);
    GetDlgItem(IDC_TYTUL)->EnableWindow(isClear);
    GetDlgItem(IDC_MUTACJA)->EnableWindow(isClear);
}

/////////////////////////////////////////////////////////////////////////////
// CNewTitleDlg dialog

CNewTitleDlg::CNewTitleDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CNewTitleDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CNewTitleDlg)
    m_kra_x = pszpalt_x;
    m_kra_y = pszpalt_y;
    m_strona_x = 250;
    m_strona_y = 362;
    m_sw_w = 3;
    m_sw_h = 3;
    //}}AFX_DATA_INIT
    m_do_kiedy = CTime::GetCurrentTime() + CTimeSpan(30, 0, 0, 0);
}

void CNewTitleDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CNewTitleDlg)
    DDX_Text(pDX, IDC_MUTACJA, m_mutacja);
    DDV_MaxChars(pDX, m_mutacja, 4);
    DDX_Text(pDX, IDC_OPIS, m_opis);
    DDX_Text(pDX, IDC_TYTUL, m_tytul);
    DDV_MaxChars(pDX, m_tytul, 3);
    DDX_Text(pDX, IDC_SIZEX, m_kra_x);
    DDV_MinMaxInt(pDX, m_kra_x, 1, 255);
    DDX_Text(pDX, IDC_SIZEY, m_kra_y);
    DDV_MinMaxInt(pDX, m_kra_y, 1, 255);
    DDX_Text(pDX, IDC_TXTPOSX, m_strona_x);
    DDV_MinMaxInt(pDX, m_strona_x, 50, 2000);
    DDX_Text(pDX, IDC_TXTPOSY, m_strona_y);
    DDV_MinMaxInt(pDX, m_strona_y, 50, 2000);
    DDX_Text(pDX, IDC_DEPS, m_sw_w);
    DDV_MinMaxInt(pDX, m_sw_w, 0, 100);
    DDX_Text(pDX, IDC_GEPS, m_sw_h);
    DDV_MinMaxInt(pDX, m_sw_h, 0, 100);
    DDX_Text(pDX, IDC_ALIKE, m_tytul_upraw);
    DDX_Text(pDX, IDC_ZAKRES, m_mutacja_upraw);
    DDX_DateTimeCtrl(pDX, IDC_DATA_CZOB, m_do_kiedy);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNewTitleDlg, CDialog)
//{{AFX_MSG_MAP(CNewTitleDlg)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewTitleDlg message handlers

/////////////////////////////////////////////////////////////////////////////
// CEPSDateDlg dialog

CEPSDateDlg::CEPSDateDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CEPSDateDlg::IDD, pParent)
{
    vDoc = theApp.activeDoc;
}

void CEPSDateDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CEPSDateDlg)
    DDX_Control(pDX, IDC_EPSDATE, m_epsdate);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEPSDateDlg, CDialog)
//{{AFX_MSG_MAP(CEPSDateDlg)
ON_WM_CLOSE()
ON_WM_RBUTTONDOWN()
ON_WM_LBUTTONDOWN()
ON_BN_CLICKED(ID_FILE_UPDATE, &CEPSDateDlg::OnUznaj)
ON_NOTIFY(NM_CLICK, IDC_EPSDATE, &CEPSDateDlg::OnClickEpsdate)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEPSDateDlg message handlers

BOOL CEPSDateDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    GetDlgItem(ID_FILE_UPDATE)->EnableWindow(!vDoc->isRO);

    SetCursor(theApp.LoadStandardCursor(IDC_WAIT));
    m_SmallImageList.Create(IDB_SMALLICONS, 16, 1, BIALY);
    m_StateImageList.Create(IDB_STATEICONS, 16, 1, RGB(255, 0, 0));

    m_epsdate.SetImageList(&m_SmallImageList, LVSIL_SMALL);
    m_epsdate.SetImageList(&m_StateImageList, LVSIL_STATE);

    m_epsdate.InsertColumn(0, _T("EPS"), LVCFMT_LEFT, 90);
    m_epsdate.InsertColumn(1, _T("Str."), LVCFMT_LEFT, 35);
    m_epsdate.InsertColumn(2, _T("Nowe fileid"), LVCFMT_LEFT, 100);
    m_epsdate.InsertColumn(3, _T("Poprzednie fileid"), LVCFMT_LEFT, 100);
    m_epsdate.InsertColumn(4, _T("Kto wprowadzi³"), LVCFMT_LEFT, 90);
    m_epsdate.InsertColumn(5, _T("Data wprowadzenia"), LVCFMT_LEFT, 110);

    BOOL ret = theManODPNET.F4(vDoc, &m_epsdate, TRUE);
    SetCursor(theApp.LoadStandardCursor(IDC_ARROW));
    return ret;
}

void CEPSDateDlg::OnUznaj()
{
    theManODPNET.F4(vDoc, &m_epsdate, FALSE);
}

void CEPSDateDlg::OnClickEpsdate(NMHDR* pNMHDR, LRESULT* pResult)
{
    UINT uFlags = 0;
    const auto& ptAction = ((LPNMITEMACTIVATE)pNMHDR)->ptAction;
    auto ind = m_epsdate.HitTest(ptAction, &uFlags);

    if (ind >= 0 && (uFlags & LVHT_ONITEMSTATEICON))
        m_epsdate.SetCheck(ind, !m_epsdate.GetCheck(ind));

    *pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CDirDaysDlg dialog

CDirDaysDlg::CDirDaysDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CDirDaysDlg::IDD, pParent), m_odkiedy(CTime::GetCurrentTime())
{
    //{{AFX_DATA_INIT(CDirDaysDlg)
    m_dokiedy = m_odkiedy + CTimeSpan(30, 0, 0, 0);
    m_oneday = CTimeSpan(1, 0, 0, 0);
    //}}AFX_DATA_INIT
}

void CDirDaysDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDirDaysDlg)
    DDX_Text(pDX, IDC_NAME, m_path);
    DDX_DateTimeCtrl(pDX, IDC_POSX, m_odkiedy);
    DDX_DateTimeCtrl(pDX, IDC_POSY, m_dokiedy);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDirDaysDlg, CDialog)
//{{AFX_MSG_MAP(CDirDaysDlg)
ON_BN_CLICKED(IDC_DAYDIRS, &CDirDaysDlg::OnDaydirs)
ON_BN_CLICKED(IDC_CAB, &CDirDaysDlg::OnCab)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDirDaysDlg message handlers

void CDirDaysDlg::OnDaydirs()
{
    ::GetCurrentDirectory(_MAX_DIR, theApp.bigBuf);
    CFileDialog fd(TRUE, nullptr, _T("dir"));
    if (fd.DoModal() == IDOK)
        GetDlgItem(IDC_NAME)->SetWindowText(fd.GetPathName().Left(fd.m_ofn.nFileOffset));
    ::SetCurrentDirectory(theApp.bigBuf);
}

void CDirDaysDlg::OnCab()
{
    UpdateData(TRUE);
    CString sPath, sZip;
    CTime l_odkiedy = m_odkiedy;
    while (l_odkiedy <= m_dokiedy) {
        if (l_odkiedy.GetDayOfWeek() == 1) continue; // w niedziele nie
        sZip = l_odkiedy.Format(c_ctimeDataWs);
        sPath.AppendFormat(_T("%s%s\\"), (LPCTSTR)m_path, (LPCTSTR)sZip);
        _spawnlp(_P_NOWAIT, "zip", "zip", "-j", (LPCTSTR)(m_path + sZip), (LPCTSTR)(sPath + "*.eps"), (LPCTSTR)(sPath + "*.pdf"), NULL);
        l_odkiedy += m_oneday;
    }
    ::WriteProfileString(_T("GenEPS"), _T("DayDirsRoot"), m_path);
    CDialog::OnOK();
}

void CDirDaysDlg::OnOK()
{
    CDialog::OnOK();
    while (m_odkiedy <= m_dokiedy) {
        if (m_odkiedy.GetDayOfWeek() != 1) // w niedziele nie
            ::CreateDirectory(m_path + m_odkiedy.Format(c_ctimeDataWs), nullptr);
        m_odkiedy += m_oneday;
    }
    ::WriteProfileString(_T("GenEPS"), _T("DayDirsRoot"), m_path);
}
/////////////////////////////////////////////////////////////////////////////
// CPageDerv dialog

CPageDerv::CPageDerv(CWnd* pParent /*=NULL*/)
    : CDialog(CPageDerv::IDD, pParent)
{
    //{{AFX_DATA_INIT(CPageDerv)
    m_ilekol = 1;
    m_direction = m_base_nr = m_nr = 0;
    //}}AFX_DATA_INIT
}

void CPageDerv::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPageDerv)
    DDX_Radio(pDX, IDC_FROMPAGE, m_direction);
    DDX_Control(pDX, IDC_TYTMUT, m_tytmut);
    DDX_Text(pDX, IDC_ILEKOLUMN, m_ilekol);
    DDV_MinMaxInt(pDX, m_ilekol, 1, 255);
    DDX_Text(pDX, IDC_NR, m_nr);
    DDV_MinMaxInt(pDX, m_nr, 1, 255);
    DDX_Text(pDX, IDC_BASE_NR, m_base_nr);
    DDV_MinMaxInt(pDX, m_base_nr, 0, 255);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPageDerv, CDialog)
//{{AFX_MSG_MAP(CPageDerv)
ON_CBN_SELCHANGE(IDC_DERVLVL, OnSelchangeDervlvl)
ON_CONTROL_RANGE(BN_CLICKED, IDC_FROMPAGE, IDC_TOPAGE, &CPageDerv::OnDirectionChange)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageDerv message handlers

BOOL CPageDerv::OnInitDialog()
{
    if (!theApp.isRDBMS || !CDialog::OnInitDialog()) return FALSE;
    ((CComboBox*)GetDlgItem(IDC_DERVLVL))->SetCurSel((int)m_idervlvl);

    m_drw_xx = m_mak_xx;
    m_base_nr = m_nr;
    CManODPNETParms orapar {
        { CManDbType::DbTypeInt32, CManDbDir::ParameterInOut, &m_drw_xx },
        { CManDbType::DbTypeInt32, CManDbDir::ParameterInOut, &m_base_nr },
        { CManDbType::DbTypeRefCursor, CManDbDir::ParameterOut, nullptr }
    };
    orapar.outParamsCount = 2;
    theManODPNET.FillList(&m_tytmut, "begin derv.info(:mak_xx,:nr_porz,:retCur); end;", orapar, 0);

    SetDlgItemInt(IDC_BASE_NR, m_base_nr);
    OnDirectionChange(IDC_FROMPAGE);
    return TRUE;
}

void CPageDerv::OnOK()
{
    if (m_idervlvl == DervType::fixd) {
        UpdateData(TRUE);
        if ((m_base_nr - m_nr) & 1) {
            AfxMessageBox(_T("Pe³ne dziedziczenie musi zachowywaæ parzystoœæ stron"));
            return;
        }
    }

    if (m_direction == 0 && m_tytmut.IsWindowEnabled())
        if (m_tytmut.GetSelCount() != 1) {
            m_tytmut.SetFocus();
            AfxMessageBox(_T("Wska¿ jedn¹ makietê"));
            return;
        } else {
            m_drw_xx = int(m_tytmut.GetItemData(m_tytmut.GetCurSel()));
            if (m_drw_xx < 0) m_drw_xx *= -1;
        }

    // zapisz w tablicach pomocniczych zmienione pozycje
    const int cc = m_tytmut.GetCount();
    for (int i = 0; i < cc; ++i) {
        const int isSelected = m_tytmut.GetSel(i);
        const auto sel_drw_xx = (int)m_tytmut.GetItemData(i);
        if (isSelected && sel_drw_xx > 0) // nie by³ wczeœniej zaznaczony
            m_derv_add.push_back(-1 * sel_drw_xx);
        else if (!isSelected && (sel_drw_xx < 0)) // zosta³ odznaczony
            m_derv_del.push_back(sel_drw_xx);
    }

    CDialog::OnOK();
}

void CPageDerv::OnSelchangeDervlvl()
{
    m_idervlvl = (DervType)((CComboBox*)GetDlgItem(IDC_DERVLVL))->GetCurSel();
    const BOOL isEnabled = ((m_idervlvl != DervType::none || m_direction == 1) && m_idervlvl != DervType::proh && m_idervlvl != DervType::druk);
    GetDlgItem(IDC_TYTMUT)->EnableWindow(isEnabled);
    GetDlgItem(IDC_BASE_NR)->EnableWindow(isEnabled);
}

void CPageDerv::OnDirectionChange(const UINT mode)
{
    const int cc = m_tytmut.GetCount();
    if (mode == IDC_FROMPAGE) {
        m_direction = 0;
        for (int i = 0; i < cc; ++i)
            m_tytmut.SetSel(i, (int)m_tytmut.GetItemData(i) == m_drw_xx);
    } else {
        m_direction = 1;
        for (int i = 0; i < cc; ++i)
            m_tytmut.SetSel(i, (int)m_tytmut.GetItemData(i) < 0);
    }

    OnSelchangeDervlvl();
}

/////////////////////////////////////////////////////////////////////////////
// CGrzbDlg dialog

CGrzbDlg::CGrzbDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CGrzbDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CGrzbDlg)
    bRefreshOnClose = FALSE;
    m_delete = m_insert = m_incordec = 0;
    m_split_cnt = m_delete_cnt = m_insert_cnt = 4;
    //}}AFX_DATA_INIT
}

void CGrzbDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CGrzbDlg)
    DDX_Control(pDX, IDC_TYTMUT, m_tytmut);
    DDX_Radio(pDX, IDC_PODWALY, m_insert);
    DDX_Text(pDX, IDC_NUMER, m_insert_cnt);
    DDV_MinMaxInt(pDX, m_insert_cnt, 1, 316);
    DDX_Text(pDX, IDC_NR, m_delete_cnt);
    DDV_MinMaxInt(pDX, m_delete_cnt, 1, 320);
    DDX_Radio(pDX, IDC_DODAJ, m_incordec);
    DDX_Text(pDX, IDC_SPLIT_CNT, m_split_cnt);
    DDV_MinMaxInt(pDX, m_split_cnt, 1, 316);
    DDX_Radio(pDX, IDC_DELETEINSERT, m_delete);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGrzbDlg, CDialog)
//{{AFX_MSG_MAP(CGrzbDlg)
ON_BN_CLICKED(IDC_DODAJ, OnExpandGrzbiet)
ON_BN_CLICKED(IDC_USUN, OnShrinkGrzbiet)
ON_BN_CLICKED(IDC_ILEKOLUMN, &CGrzbDlg::OnBnClicked4x4)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGrzbDlg message handlers

BOOL CGrzbDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    GetDlgItem(IDC_ILEKOLUMN)->EnableWindow(theApp.activeDoc->m_mak_xx > 0 && theApp.activeDoc->m_pages.size() == 4);

    CManODPNETParms orapar {
        { CManDbType::DbTypeVarchar2, &theApp.activeDoc->data },
        { CManDbType::DbTypeInt32, &theApp.activeDoc->m_mak_xx },
        { CManDbType::DbTypeRefCursor, CManDbDir::ParameterOut, nullptr }
    };

    return theManODPNET.FillCombo(&m_tytmut, "begin grb.open_drw(:kiedy,:grb_xx,:retCur); end;", orapar, 0);
}

void CGrzbDlg::OnExpandGrzbiet()
{
    ChangeMode(TRUE);
}

void CGrzbDlg::OnShrinkGrzbiet()
{
    ChangeMode(FALSE);
}

void CGrzbDlg::ChangeMode(const BOOL isExpand) const
{
    GetDlgItem(IDC_PODWALY)->EnableWindow(isExpand);
    GetDlgItem(IDC_DELETEINSERT)->EnableWindow(!isExpand);
    GetDlgItem(IDC_SEKCJA)->EnableWindow(isExpand);
    GetDlgItem(IDC_NUMER)->EnableWindow(isExpand);
    GetDlgItem(IDC_ALIKE)->EnableWindow(!isExpand);
    GetDlgItem(IDC_SPLIT_CNT)->EnableWindow(!isExpand);
    GetDlgItem(IDC_GAZETA)->EnableWindow(!isExpand);
}

void CGrzbDlg::OnBnClicked4x4()
{
    if (AfxMessageBox(_T("Czy chcesz utworzyæ grzbiet o objêtoœci 16 stron, zawieraj¹cy czterokrotnie ten produkt"), MB_YESNO | MB_ICONQUESTION) != IDYES)
        return;

    CString tytul = theApp.activeDoc->gazeta.Left(3);
    CString mutacja = theApp.activeDoc->gazeta.Mid(4, 2);
    CManODPNETParms orapar {
        { CManDbType::DbTypeVarchar2, &tytul },
        { CManDbType::DbTypeVarchar2, &mutacja },
        { CManDbType::DbTypeVarchar2, &theApp.activeDoc->data }
    };
    theManODPNET.EI("call create_grb_4x4(:tytul,:mutacja,:kiedy)", orapar);

    if (bRefreshOnClose)
        this->OnCancel();
}

void CGrzbDlg::OnOK()
{
    UpdateData();
    bool isExpand = !(m_incordec == 1 && m_delete == 0);
    if (isExpand && m_tytmut.GetCurSel() == CB_ERR) {
        m_tytmut.SetFocus();
        return;
    }
    if ((m_incordec == 0 && m_insert == -1) || (m_incordec == 1 && m_delete == -1)) return;
    m_drw_xx = isExpand ? (int)m_tytmut.GetItemData(m_tytmut.GetCurSel()) : m_delete;

    CDialog::OnOK();
    if (m_incordec == 0 && m_insert == 0) m_insert_cnt = 0;
    if (m_incordec == 1 && m_delete < 2) m_split_cnt = 0;
}

// CAccGrbDlg dialog
IMPLEMENT_DYNAMIC(CAccGrbDlg, CDialog)
CAccGrbDlg::CAccGrbDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CAccGrbDlg::IDD, pParent)
{
    m_ckporg = m_ckpcdrz = m_ckpldrz = FALSE;
}

BOOL CAccGrbDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    return TRUE;
}

void CAccGrbDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_PRINT_LDRZ, m_print_ldrz);
    DDX_Text(pDX, IDC_PRINT_CDRZ, m_print_cdrz);
    DDX_Text(pDX, IDC_PRINT_ORG, m_print_org);
    DDX_Check(pDX, IDC_CKPLDRZ, m_ckpldrz);
    DDX_Check(pDX, IDC_CKPCDRZ, m_ckpcdrz);
    DDX_Check(pDX, IDC_CKPORG, m_ckporg);
}

BEGIN_MESSAGE_MAP(CAccGrbDlg, CDialog)
END_MESSAGE_MAP()

// COstWer dialog
IMPLEMENT_DYNAMIC(COstWer, CDialog)
COstWer::COstWer(CWnd* pParent /*=NULL*/)
    : CDialog(COstWer::IDD, pParent)
{
}

COstWer::COstWer(std::vector<CDrawAdd*>* aNewAdds, std::vector<CDrawAdd*>* aOldAdds, std::vector<CDrawAdd*>* aModifAdds, std::vector<CDrawAdd*>* aDelAdds, BOOL bBankOnly)
    : CDialog(COstWer::IDD, nullptr)
{
    m_aNewAdds = aNewAdds;
    m_aOldAdds = aOldAdds;
    m_aModifAdds = aModifAdds;
    m_aDelAdds = aDelAdds;
    m_bBankOnly = bBankOnly;
}

void COstWer::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ADDLIST, m_adds);
}

BEGIN_MESSAGE_MAP(COstWer, CDialog)
ON_BN_CLICKED(IDOK, &COstWer::OnOK)
ON_BN_CLICKED(IDC_SIGNALL, &COstWer::OnSelectAll)
ON_NOTIFY(NM_CLICK, IDC_ADDLIST, &COstWer::OnNMClickAddlist)
END_MESSAGE_MAP()

void COstWer::AppendAdd(CDrawAdd* pAdd, const int status)
{
    const int i = m_adds.InsertItem(m_adds.GetItemCount(), pAdd->logpage, status);
    ::StringCchPrintf(theApp.bigBuf, n_size, _T("%li"), pAdd->nreps);
    m_adds.SetItemText(i, 1, theApp.bigBuf);
    ::StringCchPrintf(theApp.bigBuf, n_size, _T("%iX%i "), pAdd->sizex, pAdd->sizey);
    m_adds.SetItemText(i, 2, theApp.bigBuf);
    m_adds.SetItemText(i, 3, pAdd->nazwa);
    ::StringCchPrintf(theApp.bigBuf, n_size, _T("%i"), pAdd->fizpage >> 16);
    m_adds.SetItemText(i, 4, theApp.bigBuf);
    if (pAdd->kolor != ColorId::brak) {
        if (pAdd->kolor == ColorId::full)
            ::StringCchCopy(theApp.bigBuf, n_size, FULL);
        else
            ::StringCchCopy(theApp.bigBuf, n_size, CDrawDoc::kolory[pAdd->kolor >> 3]);
    } else
        ::StringCchCopy(theApp.bigBuf, n_size, BRAK);
    m_adds.SetItemText(i, 5, theApp.bigBuf);
    m_adds.SetItemText(i, 6, pAdd->remarks_atex);
    if (pAdd->bank.insid > 0) {
        if (!m_bBankOnly) {
            LVITEM li;
            li.mask = LVIF_IMAGE;
            li.iSubItem = 0;
            li.iItem = i;
            m_adds.GetItem(&li);
            li.iImage = (pAdd->bank.n < pAdd->bank.k ? 3 : 1);
            m_adds.SetItem(&li);
        }
        ::StringCchPrintf(theApp.bigBuf, n_size, _T("%u"), pAdd->bank.n);
        m_adds.SetItemText(i, 7, theApp.bigBuf);
        ::StringCchPrintf(theApp.bigBuf, n_size, _T("%u"), pAdd->bank.k);
        m_adds.SetItemText(i, 8, theApp.bigBuf);
    }
    m_adds.SetCheck(i, FALSE);
    m_adds.SetItemData(i, (DWORD_PTR)pAdd);
}

BOOL COstWer::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_SmallImageList.Create(IDB_SMALLICONS, 16, 1, BIALY);
    m_StateImageList.Create(IDB_STATEICONS, 16, 1, RGB(255, 0, 0));

    m_adds.SetImageList(&m_SmallImageList, LVSIL_SMALL);
    m_adds.SetImageList(&m_StateImageList, LVSIL_STATE);

    const DWORD es = m_adds.GetExtendedStyle();
    m_adds.SetExtendedStyle(es | LVS_EX_GRIDLINES);
    m_adds.InsertColumn(0, _T("Miejsce"), LVCFMT_LEFT, 90);
    m_adds.InsertColumn(1, _T("EPS"), LVCFMT_LEFT, 70);
    m_adds.InsertColumn(2, _T("Rozmiar"), LVCFMT_LEFT, 50);
    m_adds.InsertColumn(3, _T("Nazwa"), LVCFMT_LEFT, 150);
    m_adds.InsertColumn(4, _T("Strona"), LVCFMT_LEFT, 55);
    m_adds.InsertColumn(5, _T("Kolor"), LVCFMT_LEFT, 86);
    m_adds.InsertColumn(6, _T("Uwagi"), LVCFMT_LEFT, 220);
    m_adds.InsertColumn(7, _T("Emisji w banku"), LVCFMT_LEFT, 85);
    m_adds.InsertColumn(8, _T("Dostêpnych wydañ"), LVCFMT_LEFT, 120);

    if (m_bBankOnly) {
        for (const auto& a : *m_aNewAdds) // nowe
            if (a->bank.insid > 0)
                AppendAdd(a, a->bank.n < a->bank.k ? 3 : 1);
    } else {
        for (const auto& a : *m_aNewAdds) // nowe
            AppendAdd(a, 2);
        for (const auto& a : *m_aModifAdds) // modyfikowane
            AppendAdd(a, 0);
        for (const auto& a : *m_aDelAdds) // usuniete
            AppendAdd(a, 5);
    }

    return TRUE;
}

void COstWer::OnOK()
{
    CDialog::OnOK();

    std::vector<CDrawAdd*> to_del;
    LVITEM li;
    li.mask = LVIF_IMAGE;
    li.iSubItem = 0;
    for (int i = 0; i < m_adds.GetItemCount(); ++i) {
        li.iItem = i;
        m_adds.GetItem(&li);
        auto pAdd = reinterpret_cast<CDrawAdd*>(m_adds.GetItemData(i));
        switch (li.iImage) {
            case 1: // obligatoryjne z banku
            case 2: // nowe
            case 3: // nowe z banku
                if (!m_adds.GetCheck(i)) {
                    theApp.activeDoc->Remove(pAdd);
                    to_del.push_back(pAdd);
                }
                break;
            case 0: // modyfikowane
                if (!m_adds.GetCheck(i)) {
                    theApp.activeDoc->Remove(pAdd);
                    to_del.push_back(pAdd);
                } else {
                    const long adno = pAdd->nreps;
                    for (const auto& a : *m_aOldAdds)
                        if (a->nreps == adno) {
                            theApp.activeDoc->Remove(a);
                            to_del.push_back(a);
                        }
                }
                break;
            case 5: // usuniete
                if (m_adds.GetCheck(i)) {
                    theApp.activeDoc->Remove(pAdd);
                    to_del.push_back(pAdd);
                }
        }
    }

    for (const auto& a : to_del)
        delete a;
}

void COstWer::OnNMClickAddlist(NMHDR* pNMHDR, LRESULT* pResult)
{
    UINT uFlags = 0;
    const auto& ptAction = ((LPNMITEMACTIVATE)pNMHDR)->ptAction;
    const auto ind = m_adds.HitTest(ptAction, &uFlags);

    if (ind >= 0 && (uFlags & LVHT_ONITEMSTATEICON))
        m_adds.SetCheck(ind, !m_adds.GetCheck(ind));

    *pResult = 0;
}

void COstWer::OnSelectAll()
{
    const auto rc = m_adds.GetItemCount();
    const BOOL bChecked = IsDlgButtonChecked(IDC_SIGNALL);
    for (int i = 0; i < rc; ++i)
        m_adds.SetCheck(i, bChecked);
}

// CAcDeadDlg
IMPLEMENT_DYNAMIC(CAcDeadDlg, CDialog)

CAcDeadDlg::CAcDeadDlg(CWnd* pParent /*=NULL*/) : CDialog(CAcDeadDlg::IDD, pParent)
{
}

void CAcDeadDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_DateTimeCtrl(pDX, IDC_RED, m_red);
    DDX_DateTimeCtrl(pDX, IDC_FOT, m_fot);
    DDX_DateTimeCtrl(pDX, IDC_KOL, m_kol);
}

BEGIN_MESSAGE_MAP(CAcDeadDlg, CDialog)
END_MESSAGE_MAP()

// CAboutDlg
IMPLEMENT_DYNAMIC(CAboutDlg, CDialog)

#pragma unmanaged

extern "C" void GetProcId(char* procId);

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
    //{{AFX_DATA_INIT(CAboutDlg)
    //}}AFX_DATA_INIT
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);
    GlobalMemoryStatusEx(&ms);

    char procId[48];
    GetProcId(procId);
    CString sProcInfo(procId);
    if (sProcInfo.IsEmpty())
        sProcInfo = _T("Procesor < Pentium 4");
    else
        sProcInfo.Trim().Replace(_T("  "), _T(" "));

    m_memstat.Format(_T("Zu¿ycie pamiêci: %u/%u [MB]\n%s"), static_cast<uint32_t>((ms.ullTotalPhys - ms.ullAvailPhys) / 0x100000), static_cast<uint32_t>(ms.ullTotalPhys / 0x100000), (LPCTSTR)sProcInfo);
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
    DDX_Text(pDX, IDC_CLIENT, m_client);
    DDX_Text(pDX, IDC_MEMSTAT, m_memstat);
    //}}AFX_DATA_MAP
}

BOOL CAboutDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    CString s;
    s.Format(ID_APP_ABOUT, APP_NAME, (LPCTSTR)theApp.m_app_version,
#ifdef _WIN64
             _T("64"));
#else
             _T("86"));
#endif
    SetDlgItemText(IDC_SIGN_TEXT, s);

    return TRUE;
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
//{{AFX_MSG_MAP(CAboutDlg)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()
