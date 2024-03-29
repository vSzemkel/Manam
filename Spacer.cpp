
#include "pch.h"
#include "DrawAdd.h"
#include "DrawDoc.h"
#include "DrawPage.h"
#include "MainFrm.h"
#include "Manam.h"
#include "QueView.h"
#include "Spacer.h"

static CString kroklastChoice = _T("*");
static CTime emisionlastChoice;

/////////////////////////////////////////////////////////////////////////////
// CSpacerDlg dialog

BOOL CSpacerDlg::Deal(CDrawAdd* vAdd)
{
    // Aby zrobic rezerwacje ogloszenie musi byc nowe, stac na konkretnej stronie,
    // makieta musi byc zapisana w bazie a uzytkownik musi miec stosowne uprawnienia
    if (vAdd->m_pub_xx > 0 || vAdd->m_pDocument->m_mak_xx < 0 || !(theApp.grupa&UserRole::dea))
        return FALSE;

    // Sprawdz uprawnienia do sprzedazy
    if (!theManODPNET.CkAccess(vAdd->m_pDocument->gazeta, _T("S"))) return FALSE;

    // Nie moze byc otwarta zadna inna makieta, je�li nie przenosimy z kolejki
    if (vAdd->m_pub_xx >= -1 && theApp.m_pDocManager->GetOpenDocumentCount() > 1) {
        AfxMessageBox(_T("Podczas dokonywania rezerwacji mo�e by� otwarta tylko jedna makieta."), MB_OK, MB_ICONINFORMATION);
        return TRUE;
    }

    vAdd->flags.isok = 1;
    CSpacerDlg dlg(vAdd);
    dlg.m_que_deal = !vAdd->fizpage;
    if (dlg.DoModal() != IDOK && (vAdd->m_add_xx < 1 || dlg.m_quepub_xx > 0)) { //jezeli nie udalo sie sprzedac, to usun ogloszenie z makiety, z kolejki nie usuwaj
        CDrawDoc* vDoc = vAdd->m_pDocument;
        CDrawPage* pPage = vDoc->GetPage(vAdd->fizpage);
        vDoc->Remove(vAdd);
        if (vAdd->m_add_xx < 1) delete vAdd;
        if (pPage) pPage->Invalidate();
        else vDoc->UpdateAllViews(nullptr);
        vDoc->SetModifiedFlag(FALSE);
    } else dlg.m_onclose_refresh ? theApp.FileRefresh() : vAdd->UpdateInfo();

    return TRUE;
}

CSpacerDlg::CSpacerDlg(CDrawAdd* vAdd, CWnd* pParent /*=NULL*/)
    : CDialog(CSpacerDlg::IDD, pParent)
{
    _stscanf_s(vAdd->m_pDocument->data, c_formatDaty, &m_dd, &m_mm, &m_rrrr);
    emisionlastChoice = m_lastemision = CTime(m_rrrr, m_mm, m_dd, 0, 0, 0);
    kroklastChoice = _T("*");
    //{{AFX_DATA_INIT(CSpacerDlg)
    m_pageparity = m_pagelayout = FALSE;
    m_exactplace = TRUE;
    m_nazwa.Empty(); m_wersja.Empty(); m_uwagi.Empty();
    m_dealappend = m_wsekcji = m_sekcja = FALSE;
    m_quepub_xx = m_spacer = 0;
    //}}AFX_DATA_INIT
    m_que_deal = m_onclose_refresh = FALSE;
    pub = vAdd;
    m_posx = vAdd->posx;
    m_posy = vAdd->posy;
    m_sizex = vAdd->sizex;
    m_sizey = vAdd->sizey;
    m_szpalt_x = vAdd->szpalt_x;
    m_szpalt_y = vAdd->szpalt_y;
    m_kolor = vAdd->kolor;
    m_typ_xx = vAdd->typ_xx;
    m_blokada = vAdd->flags.locked;
    m_niekratowe = vAdd->typ_xx > 0;
    m_first_emision_pub_xx = -1;
    if (vAdd->m_pub_xx < -1) { // z kolejki
        m_nazwa = vAdd->nazwa;
        m_wersja = vAdd->wersja;
        m_uwagi = vAdd->remarks;
        m_dealappend = TRUE;
        m_spacer = vAdd->m_add_xx;
        m_quepub_xx = -1 * vAdd->m_pub_xx;
    }
}

void CSpacerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSpacerDlg)
    DDX_Control(pDX, IDC_LASTEMISION, m_lastemisionctl);
    DDX_Control(pDX, IDC_TYP_OGL, m_typ_ogl_combo);
    DDX_Control(pDX, IDC_KOLOR, m_kolorcombo);
    DDX_Control(pDX, IDC_EMISJE, m_emisjelist);
    DDX_Check(pDX, IDC_EXACTPLACE, m_exactplace);
    DDX_Check(pDX, IDC_KRATOWE, m_niekratowe);
    DDX_DateTimeCtrl(pDX, IDC_LASTEMISION, m_lastemision);
    DDX_Check(pDX, IDC_LOCKED, m_blokada);
    DDX_Text(pDX, IDC_NAZWA, m_nazwa);
    DDX_Check(pDX, IDC_PAGELAYOUT, m_pagelayout);
    DDX_Check(pDX, IDC_PAGEPARITY, m_pageparity);
    DDX_Text(pDX, IDC_REMARKS, m_uwagi);
    DDX_Check(pDX, IDC_SEKCJA, m_sekcja);
    DDX_Text(pDX, IDC_SIZEX, m_sizex);
    DDV_MinMaxInt(pDX, m_sizex, 1, m_szpalt_x);
    DDX_Text(pDX, IDC_SIZEY, m_sizey);
    DDV_MinMaxInt(pDX, m_sizey, 1, m_szpalt_y);
    DDX_Text(pDX, IDC_POSX, m_posx);
    DDX_Text(pDX, IDC_POSY, m_posy);
    DDX_Text(pDX, IDC_WERSJA, m_wersja);
    DDX_Check(pDX, IDC_WSEKCJI, m_wsekcji);
    DDX_Check(pDX, IDC_DEALAPPEND, m_dealappend);
    DDX_Text(pDX, IDC_SPACER, m_spacer);
    DDX_Control(pDX, IDC_MUTACJA, m_ollist);

    const int base = m_que_deal ? 0 : 1;
    const int ax = m_szpalt_x - m_sizex + 1;
    const int ay = m_szpalt_y - m_sizey + 1;
    DDV_MinMaxInt(pDX, m_posx, base, ax);
    DDV_MinMaxInt(pDX, m_posy, base, ay);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSpacerDlg, CDialog)
    //{{AFX_MSG_MAP(CSpacerDlg)
    ON_BN_CLICKED(IDC_LOCKED, &CSpacerDlg::OnLocked)
    ON_BN_CLICKED(IDC_KRATOWE, &CSpacerDlg::OnKratowe)
    ON_BN_CLICKED(IDC_DEALAPPEND, &CSpacerDlg::OnDealappend)
    ON_NOTIFY(DTN_DATETIMECHANGE, IDC_LASTEMISION, &CSpacerDlg::OnCloseupLastemision)
    ON_BN_CLICKED(IDEMISJE, &CSpacerDlg::OnEmisje)
    ON_CBN_SELCHANGE(IDC_KROK, &CSpacerDlg::OnSelchangeKrok)
    ON_BN_CLICKED(IDC_EXACTPLACE, &CSpacerDlg::OnExactplace)
    ON_BN_CLICKED(IDC_SEKCJA, &CSpacerDlg::OnSekcja)
    ON_BN_CLICKED(IDC_WSEKCJI, &CSpacerDlg::OnWsekcji)
    ON_BN_CLICKED(IDC_PAGEPARITY, &CSpacerDlg::OnPageparity)
    ON_BN_CLICKED(IDC_PAGELAYOUT, &CSpacerDlg::OnPagelayout)
    ON_BN_CLICKED(IDDELSEL, &CSpacerDlg::OnDelsel)
    ON_BN_CLICKED(IDDELALL, &CSpacerDlg::OnDelall)
    ON_BN_CLICKED(IDQUE, &CSpacerDlg::OnQue)
    ON_BN_CLICKED(IDZDNIA, &CSpacerDlg::OnZDnia)
    ON_BN_CLICKED(IDC_OL, &CSpacerDlg::OnBnClickedOl)
    ON_BN_CLICKED(IDC_ADDLIST, &CSpacerDlg::OnBnClickedMutacje)
    ON_BN_CLICKED(IDC_BASE_NR, &CSpacerDlg::OnBnClickedMalaSiec)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpacerDlg message handlers

BOOL CSpacerDlg::OnInitDialog()
{
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
            m_kolorcombo.SetCurSel((m_kolor >> 3));
    }

    // krok combo
    ((CComboBox*)GetDlgItem(IDC_KROK))->SetCurSel(0);

    // mimic OnLocked
    m_blokada = !m_blokada;
    OnLocked();

    // mimic OnKratowe
    m_niekratowe = !m_niekratowe;
    OnKratowe();

    // ustaw label kratki i strony
    ::StringCchPrintf(theApp.bigBuf, n_size, _T("Kratka: %ix%i"), pub->szpalt_x, pub->szpalt_y);
    SetDlgItemText(IDC_KRATKA, theApp.bigBuf);
    ::StringCchPrintf(theApp.bigBuf, n_size, _T("Strona fizyczna: %i"), (pub->fizpage >> 16));
    SetDlgItemText(IDC_FIZPAGE, theApp.bigBuf);

    // set narrow
    SetWindowPos(&wndTop, 20, 20, narrowcx, normalcy, SWP_NOMOVE);

    if (pub->m_pub_xx < -1) // qued
        GetDlgItem(IDQUE)->EnableWindow(FALSE);

    m_mak_xx = pub->m_pDocument->m_mak_xx;

    if (m_que_deal) {
        m_str_xx = 0;
        GetDlgItem(IDOK)->EnableWindow(FALSE);
        if (CDrawApp::ShortDateToCTime(pub->m_pDocument->data) < CTime::GetCurrentTime()) {
            AfxMessageBox(_T("Zako�czono ju� kolejkowanie rezerwacji do tej makiety"), MB_ICONINFORMATION);
            OnCancel();
        }
    } else {
        // kolejne emisje tylko pozniej
        const CTime hightime(m_lastemision.GetYear() + 5, m_lastemision.GetMonth(), m_lastemision.GetDay(), 0, 0, 0);
        m_lastemisionctl.SetRange(&m_lastemision, &hightime);

        CDrawPage *p = pub->m_pDocument->GetPage(pub->fizpage);
        auto count = (int)(p->m_adds.size() - 1);
        m_str_xx = p->id_str;

        CManODPNETParms orapar {
            { CManDbType::DbTypeInt32, &m_mak_xx },
            { CManDbType::DbTypeInt32, &m_str_xx },
            { CManDbType::DbTypeInt32, &count }
        };
        if (!theManODPNET.EI("begin spacer.begin_deal(:mak_xx,:str_xx,:add_cnt); end;", orapar))
            OnCancel();
    }
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSpacerDlg::OnLocked()
{
    m_blokada = !m_blokada;
    GetDlgItem(IDC_POSX)->EnableWindow(!m_blokada);
    GetDlgItem(IDC_POSY)->EnableWindow(!m_blokada);
    GetDlgItem(IDC_SIZEX)->EnableWindow(!m_blokada);
    GetDlgItem(IDC_SIZEY)->EnableWindow(!m_blokada);
    GetDlgItem(IDC_TYP_OGL)->EnableWindow(!m_blokada);
    GetDlgItem(IDC_KRATOWE)->EnableWindow(!m_blokada);
}

void CSpacerDlg::OnKratowe()
{
    m_niekratowe = !m_niekratowe;
    if (m_niekratowe && !m_typ_ogl_combo.GetCount())
        theManODPNET.FillNiekratowe(this, pub->szpalt_x, pub->szpalt_y);

    GetDlgItem(IDC_TYP_OGL)->ShowWindow(m_niekratowe ? SW_SHOW : SW_HIDE);
    const auto showSize = m_niekratowe ? SW_HIDE : SW_SHOW;
    GetDlgItem(IDC_SIZEX)->ShowWindow(showSize);
    GetDlgItem(IDC_SIZEY)->ShowWindow(showSize);
}

void CSpacerDlg::OnDealappend()
{
    GetDlgItem(IDC_SPACER)->EnableWindow(IsDlgButtonChecked(IDC_DEALAPPEND));
    SetDlgItemText(IDC_SPACER, _T("0"));
    ((CEdit*)GetDlgItem(IDC_SPACER))->SetSel(0, -1);
    GetDlgItem(IDC_SPACER)->SetFocus();
}

void CSpacerDlg::OnCloseupLastemision(NMHDR * /*unused*/, LRESULT *pResult)
{
    CTime t;
    m_lastemisionctl.GetTime(t);
    const BOOL interval = t > CTime(m_rrrr, m_mm, m_dd, 0, 0, 0);
    const BOOL changed = emisionlastChoice != t;
    emisionlastChoice = t;

    GetDlgItem(IDC_KROK)->EnableWindow(interval);
    GetDlgItem(IDC_SEKCJA)->EnableWindow(interval);
    GetDlgItem(IDC_WSEKCJI)->EnableWindow(interval);
    GetDlgItem(IDC_PAGEPARITY)->EnableWindow(interval);
    GetDlgItem(IDC_PAGELAYOUT)->EnableWindow(interval);
    GetDlgItem(IDC_EXACTPLACE)->EnableWindow(interval);
    GetDlgItem(IDC_DEALAPPEND)->EnableWindow(!interval);
    if (changed) {
        GetDlgItem(IDEMISJE)->EnableWindow(interval);
        GetDlgItem(IDZDNIA)->EnableWindow(interval);
        GetDlgItem(IDOK)->EnableWindow(!interval);
    }

    SetWindowPos(&wndTop, 20, 20, interval ? (IsDlgButtonChecked(IDC_OL) ? olcx : widecx) : narrowcx, normalcy, SWP_NOMOVE);

    *pResult = 0;
}

void CSpacerDlg::OnEmisje()
{
    CTime czas;
    ((CDateTimeCtrl*)GetDlgItem(IDC_LASTEMISION))->GetTime(czas);
    CString kiedy = czas.Format(c_ctimeData);
    int krok = ((CComboBox*)GetDlgItem(IDC_KROK))->GetCurSel();

    m_olSelected = FALSE;
    m_ollist.ResetContent();
    m_emisjelist.ResetContent();
    CheckDlgButton(IDC_OL, BST_UNCHECKED);

    CManODPNETParms orapar {
        { CManDbType::DbTypeInt32, &m_mak_xx },
        { CManDbType::DbTypeInt32, &krok },
        { CManDbType::DbTypeVarchar2, &kiedy },
        { CManDbType::DbTypeRefCursor, CManDbDir::ParameterOut, nullptr }
    };
    theManODPNET.FillList(&m_emisjelist, "begin spacer.list_emisje(:mak_xx,:krok,:kiedy,:retCur); end;", orapar, 0);

    if (!m_que_deal) GetDlgItem(IDOK)->EnableWindow(TRUE);
    GetDlgItem(IDEMISJE)->EnableWindow(FALSE);
}

void CSpacerDlg::OnZDnia()
{
    int mak_xx;
    CTime czas;
    ((CDateTimeCtrl*)GetDlgItem(IDC_LASTEMISION))->GetTime(czas);
    CString kiedy = czas.Format(c_ctimeData);

    CManODPNETParms orapar {
        { CManDbType::DbTypeInt32, &m_mak_xx },
        { CManDbType::DbTypeVarchar2, &kiedy },
        { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &mak_xx }
    };
    orapar.outParamsCount = 1;
    if (theManODPNET.EI("begin spacer.data_emisji(:mak_xx,:kiedy,:vzdniamak_xx); end;", orapar)) {
        if (!m_emisjelist.GetCount())
            m_emisjelist.SetItemData(m_emisjelist.AddString(pub->m_pDocument->data), (DWORD)m_mak_xx);

        if (m_emisjelist.FindString(0, kiedy) == LB_ERR)
            m_emisjelist.SetItemData(m_emisjelist.AddString(kiedy), (DWORD)mak_xx);
    }
    if (!m_que_deal) GetDlgItem(IDOK)->EnableWindow(TRUE);
}

void CSpacerDlg::OnSelchangeKrok()
{
    CString krok;
    GetDlgItemText(IDC_KROK, krok);
    if (krok != kroklastChoice) {
        GetDlgItem(IDEMISJE)->EnableWindow(TRUE);
        GetDlgItem(IDZDNIA)->EnableWindow(TRUE);
        GetDlgItem(IDOK)->EnableWindow(FALSE);
    }
    kroklastChoice = krok;
}

void CSpacerDlg::OnExactplace()
{
    const BOOL isClear = !IsDlgButtonChecked(IDC_EXACTPLACE);
    GetDlgItem(IDC_SEKCJA)->EnableWindow(isClear);
    GetDlgItem(IDC_WSEKCJI)->EnableWindow(isClear);
    GetDlgItem(IDC_PAGEPARITY)->EnableWindow(isClear);
    GetDlgItem(IDC_PAGELAYOUT)->EnableWindow(isClear);
    SetBlokadaState();
}

void CSpacerDlg::OnSekcja()
{
    if (!IsDlgButtonChecked(IDC_SEKCJA)) {
        if (!IsDlgButtonChecked(IDC_WSEKCJI) && !IsDlgButtonChecked(IDC_PAGEPARITY) && !IsDlgButtonChecked(IDC_PAGELAYOUT)) {
            GetDlgItem(IDC_EXACTPLACE)->EnableWindow(TRUE);
            CheckDlgButton(IDC_EXACTPLACE, BST_CHECKED);
        }
    } else {
        CheckDlgButton(IDC_EXACTPLACE, BST_UNCHECKED);
        GetDlgItem(IDC_EXACTPLACE)->EnableWindow(FALSE);
    }
    SetBlokadaState();
}

void CSpacerDlg::OnWsekcji()
{
    if (!IsDlgButtonChecked(IDC_WSEKCJI)) {
        if (!IsDlgButtonChecked(IDC_SEKCJA) && !IsDlgButtonChecked(IDC_PAGEPARITY) && !IsDlgButtonChecked(IDC_PAGELAYOUT)) {
            GetDlgItem(IDC_EXACTPLACE)->EnableWindow(TRUE);
            CheckDlgButton(IDC_EXACTPLACE, BST_CHECKED);
        }
    } else {
        CheckDlgButton(IDC_EXACTPLACE, BST_UNCHECKED);
        GetDlgItem(IDC_EXACTPLACE)->EnableWindow(FALSE);
        CheckDlgButton(IDC_SEKCJA, BST_CHECKED);
    }
    SetBlokadaState();
}

void CSpacerDlg::OnPageparity()
{
    if (!IsDlgButtonChecked(IDC_PAGEPARITY)) {
        if (!IsDlgButtonChecked(IDC_WSEKCJI) && !IsDlgButtonChecked(IDC_SEKCJA) && !IsDlgButtonChecked(IDC_PAGELAYOUT)) {
            GetDlgItem(IDC_EXACTPLACE)->EnableWindow(TRUE);
            CheckDlgButton(IDC_EXACTPLACE, BST_CHECKED);
        }
    } else {
        CheckDlgButton(IDC_EXACTPLACE, BST_UNCHECKED);
        GetDlgItem(IDC_EXACTPLACE)->EnableWindow(FALSE);
    }
    SetBlokadaState();
}

void CSpacerDlg::OnPagelayout()
{
    if (!IsDlgButtonChecked(IDC_PAGELAYOUT)) {
        if (!IsDlgButtonChecked(IDC_WSEKCJI) && !IsDlgButtonChecked(IDC_PAGEPARITY) && !IsDlgButtonChecked(IDC_SEKCJA)) {
            GetDlgItem(IDC_EXACTPLACE)->EnableWindow(TRUE);
            CheckDlgButton(IDC_EXACTPLACE, BST_CHECKED);
        }
    } else {
        CheckDlgButton(IDC_EXACTPLACE, BST_UNCHECKED);
        GetDlgItem(IDC_EXACTPLACE)->EnableWindow(FALSE);
    }
    SetBlokadaState();
}

void CSpacerDlg::OnDelsel()
{
    long pub_xx;
    const int rc = m_emisjelist.GetCount();
    for (int i = rc - 1; i > 0; --i) // pierwsze zostaje
        if (m_emisjelist.GetSel(i)) {
            pub_xx = (long)m_emisjelist.GetItemData(i);

            if (m_add_xx > 0 && pub_xx < 0) {
                pub_xx *= -1;
                CManODPNETParms orapar {
                    { CManDbType::DbTypeInt32, &m_add_xx },
                    { CManDbType::DbTypeInt32, &pub_xx }
                };
                theManODPNET.EI("begin spacer.del_pub2(:add_xx,:pub_xx); end;", orapar);
            } else {
                CString sMut;
                m_emisjelist.GetText(i, sMut);
                if (sMut.GetLength() == 2)
                    m_ollist.SetItemData(m_ollist.AddString(sMut), pub_xx);
            }
            m_emisjelist.DeleteString(i);
        }
    GetDlgItem(IDEMISJE)->EnableWindow(TRUE);
    GetDlgItem(IDZDNIA)->EnableWindow(TRUE);
}

void CSpacerDlg::OnDelall()
{
    m_olSelected = FALSE;
    m_ollist.ResetContent();
    m_emisjelist.ResetContent();
    if (!m_que_deal && m_add_xx > 0) {
        CManODPNETParms orapar { CManDbType::DbTypeInt32, &m_add_xx };
        theManODPNET.EI("begin spacer.del_all(:add_xx); end;", orapar);
    }
    GetDlgItem(IDEMISJE)->EnableWindow(TRUE);
    GetDlgItem(IDZDNIA)->EnableWindow(TRUE);
    GetDlgItem(IDOK)->EnableWindow(FALSE);
}

void CSpacerDlg::SetBlokadaState()
{
    const int iQF = GetQueryFlag();
    if (iQF != CSpacerDlg::qfExact && iQF != CSpacerDlg::qfSectionExact && iQF != CSpacerDlg::qfSecParExact) {
        CheckDlgButton(IDC_LOCKED, BST_UNCHECKED);
        GetDlgItem(IDC_LOCKED)->EnableWindow(FALSE);
    } else
        GetDlgItem(IDC_LOCKED)->EnableWindow(TRUE);
}

int CSpacerDlg::GetQueryFlag() const
{
    int iQF = 2;
    if (IsDlgButtonChecked(IDC_SEKCJA)) iQF += 1; iQF *= 2;     // iQF&16
    if (IsDlgButtonChecked(IDC_WSEKCJI)) iQF += 1; iQF *= 2;    // iQF&8
    if (IsDlgButtonChecked(IDC_PAGEPARITY)) iQF += 1; iQF *= 2; // iQF&4
    if (IsDlgButtonChecked(IDC_PAGELAYOUT)) iQF += 1; iQF *= 2; // iQF&2
    if (IsDlgButtonChecked(IDC_EXACTPLACE)) iQF += 1;           // iQF&1
    return iQF;                                                 // free::==iQF=32
}

void CSpacerDlg::OnQue()
{
    if (!UpdateData(TRUE)) return;

    if (m_nazwa.IsEmpty()) {
        GetDlgItem(IDC_NAZWA)->SetFocus();
        AfxMessageBox(_T("Brak nazwy og�oszenia"));
        return;
    }

    // ustaw typ_xx
    int i;
    if ((i = m_typ_ogl_combo.GetCurSel()) == CB_ERR)
        pub->typ_xx = 0;
    else {
        pub->typ_xx = (int)m_typ_ogl_arr[i];
        m_sizex = (int)m_typ_sizex_arr[i];
        m_sizey = (int)m_typ_sizey_arr[i];
    }

    pub->sizex = m_sizex;
    pub->sizey = m_sizey;
    pub->nazwa = m_nazwa;
    pub->wersja = m_wersja;
    pub->remarks = m_uwagi;

    // ustaw m_kolor
    switch (m_kolorcombo.GetCurSel()) {
        case 0:
            pub->kolor = ColorId::brak;
            break;
        case 1:
            pub->kolor = ColorId::full;
            break;
        default:
            pub->kolor = (m_kolorcombo.GetCurSel() * 8 + ColorId::spot);
    }

    uint8_t ile_kol = pub->kolor & 0x07;
    auto spo_xx = (int)CDrawDoc::spoty[pub->kolor >> 3];
    if (IsDlgButtonChecked(IDC_DEALAPPEND))
        m_add_xx = GetDlgItemInt(IDC_SPACER);
    else if (m_firstSearch) {
        m_add_xx = 0;
        m_firstSearch = false;
        GetDlgItem(IDC_DEALAPPEND)->EnableWindow(FALSE);
    }

    int next_mak_xx = m_mak_xx;
    CManODPNETParms orapar {
        { CManDbType::DbTypeInt32, &next_mak_xx },
        { CManDbType::DbTypeInt32, &pub->szpalt_x },
        { CManDbType::DbTypeInt32, &pub->szpalt_y },
        { CManDbType::DbTypeInt32, &m_sizex },
        { CManDbType::DbTypeInt32, &m_sizey },
        { CManDbType::DbTypeVarchar2, &m_nazwa },
        { CManDbType::DbTypeVarchar2, &m_wersja },
        { CManDbType::DbTypeVarchar2, &m_uwagi },
        { CManDbType::DbTypeByte,  &ile_kol },
        { CManDbType::DbTypeInt32, &spo_xx },
        { CManDbType::DbTypeInt32, &pub->typ_xx },
        { CManDbType::DbTypeInt32, CManDbDir::ParameterInOut, &m_add_xx },
        { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &pub->m_pub_xx },
    };
    orapar.outParamsCount = 2;
    const int rc = m_emisjelist.GetCount();
    const char* sql = "begin spacer.que(:mak_xx,:szpalt_x,:szpalt_y,:sizex,:sizey,:nazwa,:wersja,:uwagi,:ile_kol,:spo_xx,:typ_xx,:add_xx,:pub_xx); end;";
    if (m_que_deal) {
        if (!theManODPNET.EI(sql, orapar))
            return;
        for (i = 1; i < rc; ++i) { // wieloemisyjny queDeal
            next_mak_xx = static_cast<int>(m_emisjelist.GetItemData(i));
            if (!theManODPNET.EI(sql, orapar))
                return;
        }
    } else {
        CString errText;
        bool isDone = rc > 0;
        for (i = rc - 1; i >= 0; i--) {
            if (m_emisjelist.GetSel(i)) {
                next_mak_xx = (int)m_emisjelist.GetItemData(i);
                if (m_add_xx > 0 && next_mak_xx > 0) {
                    if (!theManODPNET.EI(sql, orapar))
                        return;
                    m_emisjelist.GetText(i, errText);
                    const int pos = errText.Find(_T(" # ")) + 3;
                    if (pos >= 3)
                        errText.Truncate(pos);
                    else
                        errText += _T(" # ");
                    errText += _T("QUED");
                    m_emisjelist.DeleteString(i);
                    m_emisjelist.InsertString(i, errText);
                    m_emisjelist.SetItemData(i, (DWORD_PTR)(-1 * next_mak_xx));
                }
            }
            isDone &= (long)m_emisjelist.GetItemData(i) < 0;
        }
        if (!isDone) return;
    }

    m_onclose_refresh = TRUE;

    CDialog::OnOK();
}

void CSpacerDlg::OnOK()
{
    GetDlgItem(IDC_LASTEMISION)->EnableWindow(FALSE);
    if (!UpdateData(TRUE)) return;

    if (m_nazwa.IsEmpty()) {
        GetDlgItem(IDC_NAZWA)->SetFocus();
        AfxMessageBox(_T("Brak nazwy og�oszenia"));
        return;
    }

    int i;
    CString m_precel_flag;
    // ustaw typ_xx
    if ((i = m_typ_ogl_combo.GetCurSel()) == CB_ERR)
        pub->typ_xx = 0;
    else {
        pub->typ_xx = (int)m_typ_ogl_arr[i];
        m_sizex = (int)m_typ_sizex_arr[i];
        m_sizey = (int)m_typ_sizey_arr[i];
        m_precel_flag = m_typ_precel_arr[i];
    }

    m_onclose_refresh = (pub->posx != m_posx || pub->posy != m_posy || pub->sizex != m_sizex || pub->sizey != m_sizey || !m_precel_flag.IsEmpty());

    pub->nazwa = m_nazwa;
    pub->wersja = m_wersja;
    pub->remarks = m_uwagi;
    pub->flags.locked = m_blokada;

    // ustaw m_kolor
    switch (m_kolorcombo.GetCurSel()) {
        case 0:
            pub->kolor = ColorId::brak;
            break;
        case 1:
            pub->kolor = ColorId::full;
            break;
        default:
            pub->kolor = (m_kolorcombo.GetCurSel() * 8 + ColorId::spot);
    }

    if (IsDlgButtonChecked(IDC_DEALAPPEND))
        m_add_xx = GetDlgItemInt(IDC_SPACER);
    else if (m_firstSearch) {
        m_add_xx = 0;
        m_firstSearch = false;
        GetDlgItem(IDC_DEALAPPEND)->EnableWindow(FALSE);
    }

    int qf = GetQueryFlag();
    CString czas_kto(' ', 32);
    uint8_t ile_kol = pub->kolor & 0x07;
    auto spo_xx = (int)CDrawDoc::spoty[pub->kolor >> 3];

    CManODPNETParms orapar {
        { CManDbType::DbTypeInt32, &m_mak_xx },	// 0
        { CManDbType::DbTypeInt32, &m_str_xx },
        { CManDbType::DbTypeInt32, &pub->szpalt_x },
        { CManDbType::DbTypeInt32, &pub->szpalt_y },
        { CManDbType::DbTypeInt32, &m_posx },
        { CManDbType::DbTypeInt32, &m_posy },
        { CManDbType::DbTypeInt32, &m_sizex },
        { CManDbType::DbTypeInt32, &m_sizey },
        { CManDbType::DbTypeInt32, &qf },
        { CManDbType::DbTypeInt32, &m_blokada },
        { CManDbType::DbTypeVarchar2, &m_nazwa },	// 10
        { CManDbType::DbTypeVarchar2, &m_wersja },
        { CManDbType::DbTypeVarchar2, &m_uwagi },
        { CManDbType::DbTypeByte,  &ile_kol },
        { CManDbType::DbTypeInt32, &spo_xx },
        { CManDbType::DbTypeInt32, &pub->typ_xx },
        { CManDbType::DbTypeInt32, CManDbDir::ParameterInOut, &m_add_xx },
        { CManDbType::DbTypeInt32, CManDbDir::ParameterInOut, &pub->m_pub_xx },
        { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &czas_kto }	// 18
    };
    orapar.outParamsCount = 3;

    bool saleSucc = true;
    const int rc = m_emisjelist.GetCount();
    if (rc <= 1) {
        if (rc == 0 || (rc == 1 && ((long)m_emisjelist.GetItemData(0) > 0)))
            if (!theManODPNET.EI("begin spacer.reserve(:mak_xx,:str_xx,:szpalt_x,:szpalt_y,:x,:y,:sizex,:sizey,:qf,:blokada,:nazwa,:wersja,:uwagi,:ile_kol,:spo_xx,:typ_xx,:add_xx,:pub_xx,:czaskto); end;", orapar))
                return;
    } else {
        CString errText;
        std::vector<int> mak_xxArr;

        for (i = 0; i < rc; ++i) { // wysylamy tylko niesprzedane
            const auto xx = (int)m_emisjelist.GetItemData(i);
            if (xx > 0) mak_xxArr.push_back(xx);
        }

        if (!mak_xxArr.empty()) { // usunieto niezrealizowane recznie
            std::vector<CString> msgArr;
            if (!theManODPNET.SpacerMulti(mak_xxArr, msgArr, orapar))
                return;

            if (m_first_emision_pub_xx < 0)
                m_first_emision_pub_xx = pub->m_pub_xx;

            int maxwidth = 0;
            const auto pDC = m_emisjelist.GetDC();
            const auto mrc = (int)mak_xxArr.size();
            for (int j = 0; j < mrc; ++j) {
                // ustal i na podstawie j
                int mak_xx = mak_xxArr[j];
                for (i = 0; i < rc; ++i)
                    if (mak_xx == (int)m_emisjelist.GetItemData(i)) break;
                if (i == rc) continue;

                if (msgArr[j].IsEmpty()) { // succeded
                    msgArr[j] = _T("OK");
                    mak_xx *= -1;
                } else
                    saleSucc = false;

                m_emisjelist.GetText(i, errText);
                errText = errText.Left(10) + _T(" # ") + msgArr[j];
                m_emisjelist.DeleteString(i);
                m_emisjelist.InsertString(i, errText);
                m_emisjelist.SetItemData(i, mak_xx);

                const auto& w = pDC->GetTextExtent(errText);
                if (w.cx > maxwidth) maxwidth = w.cx;
            }
            m_emisjelist.SetHorizontalExtent(maxwidth);
            m_emisjelist.ReleaseDC(pDC);
        }
    }

    if (pub->czaskto[0] == _T('#'))
        pub->czaskto = czas_kto;
    if (pub->m_add_xx < 0 && m_add_xx > 0)
        pub->m_add_xx = m_add_xx;

    if (saleSucc) {
        if (m_first_emision_pub_xx > 0)
            pub->m_pub_xx = m_first_emision_pub_xx;
        if (m_quepub_xx) {
            CManODPNETParms orapar2 { CManDbType::DbTypeInt32, &m_quepub_xx };
            theManODPNET.EI("begin spacer.del_que(:pub_xx); end;", orapar2);
        }
        CDialog::OnOK();
    }
    pub->UpdateInfo(); pub->Invalidate();
}

BOOL CSpacerDlg::DestroyWindow()
{
    pub->m_pDocument->SetModifiedFlag(FALSE);

    return CDialog::DestroyWindow();
}

void CSpacerDlg::OnCancel()
{
    int add_xx = GetDlgItemInt(IDC_SPACER);
    if (pub != CQueView::GetSelectedAdd() && !IsDlgButtonChecked(IDC_DEALAPPEND) && add_xx > 0) {
        CManODPNETParms orapar2 { CManDbType::DbTypeInt32, &add_xx };
        theManODPNET.EI("begin spacer.del_all(:add_xx); end;", orapar2);
    }

    CManODPNETParms orapar {
        { CManDbType::DbTypeInt32, &m_mak_xx },
        { CManDbType::DbTypeInt32, &m_str_xx }
    };
    theManODPNET.EI("begin spacer.cancel_deal(:mak_xx,:str_xx); end;", orapar);

    CDialog::OnCancel();
}

void CSpacerDlg::OnBnClickedOl()
{
    if (IsDlgButtonChecked(IDC_OL)) {
        SetWindowPos(&wndTop, 20, 20, olcx, normalcy, SWP_NOMOVE);
        if (!m_olSelected) {
            m_olSelected = TRUE;
            if (!m_emisjelist.GetCount())
                m_emisjelist.SetItemData(m_emisjelist.AddString(pub->m_pDocument->data), (DWORD)m_mak_xx);

            CManODPNETParms orapar {
                { CManDbType::DbTypeInt32, &m_mak_xx },
                { CManDbType::DbTypeRefCursor, CManDbDir::ParameterOut, nullptr }
            };
            theManODPNET.FillList(&m_ollist, "begin spacer.mutacje_makiety(:mak_xx,:retCur); end;", orapar, 0);
        }
    };
}

void CSpacerDlg::EnableMultiCond(const BOOL flag) const
{
    GetDlgItem(IDC_SEKCJA)->EnableWindow(flag);
    GetDlgItem(IDC_WSEKCJI)->EnableWindow(flag);
    GetDlgItem(IDC_PAGEPARITY)->EnableWindow(flag);
    GetDlgItem(IDC_PAGELAYOUT)->EnableWindow(flag);
    GetDlgItem(IDC_EXACTPLACE)->EnableWindow(flag);
    GetDlgItem(IDC_DEALAPPEND)->EnableWindow(!flag);
}

// sprawdza, czy dla zadanej kombinacji istnieje makieta oraz czy jeszcze jej nie zam�wiono
void CSpacerDlg::InsertRequestNoDup(CString& kiedy, CString& mut)
{
    int xx;
    CManODPNETParms orapar {
        { CManDbType::DbTypeInt32, CManDbDir::ReturnValue, &xx },
        { CManDbType::DbTypeInt32, &m_mak_xx },
        { CManDbType::DbTypeVarchar2, &kiedy },
        { CManDbType::DbTypeVarchar2, &mut }
    };
    theManODPNET.EI("select spacer.check_mak_exists(:mak_xx,:kiedy,:mut) from dual", orapar);

    const int rc = m_emisjelist.GetCount();
    for (int i = 0; i < rc; ++i)
        if (std::abs((long)m_emisjelist.GetItemData(i)) == xx) return;
    if (~xx)
        m_emisjelist.SetItemData(m_emisjelist.AddString(kiedy + _T(" (") + mut + _T(")")), xx);
}

void CSpacerDlg::OnBnClickedMutacje()
{
    if (!m_emisjelist.GetSelCount() || !m_ollist.GetSelCount()) {
        AfxMessageBox(_T("Prosz� o zaznaczenie dat i mutacji"));
        return;
    }

    CString mut, kiedy;
    EnableMultiCond(TRUE);
    const auto olArr = reinterpret_cast<LPINT>(theApp.bigBuf);
    const auto olCnt = m_ollist.GetSelItems(0x40, olArr);
    const auto emArr = reinterpret_cast<LPINT>(theApp.bigBuf + 0x0100);
    const auto emCnt = m_emisjelist.GetSelItems(0x40, emArr);

    for (int i = 0; i < olCnt; ++i) {
        m_ollist.GetText(olArr[i], mut);
        for (int j = 0; j < emCnt; ++j) {
            m_emisjelist.GetText(emArr[j], kiedy);
            kiedy = kiedy.Left(10);
            InsertRequestNoDup(kiedy, mut);
        }
    }
}

void CSpacerDlg::OnBnClickedMalaSiec()
{
    for (const auto& s : { _T("BY"), _T("GD"), _T("KA"), _T("KR"), _T("LU"), _T("LO"), _T("PO"), _T("SZ"), _T("WR"), _T("WA") }) {
        const auto found = m_ollist.FindString(0, s);
        if (found != LB_ERR)
            m_ollist.SetSel(found);
    }

    EnableMultiCond(TRUE);
}
