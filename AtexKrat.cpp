// AtexKrat.cpp : implementation file
// p.xsize*pg.colwidth # p.ysizemm # p.xsize #	pg.columns # pg.height

#include "StdAfx.h"
#include "AtexKrat.h"
#include "DrawDoc.h"
#include "DrawPage.h"
#include "Manam.h"

#pragma region CAtexKrat
CAtexKrat::CAtexKrat(const TCHAR *atexKratInfo, CDrawDoc* vDoc)
{
    doc = vDoc;
    isValid = _stscanf_s(atexKratInfo, _T("%f#%f#%f#%f#%f"), &xmm, &ymm, &xcol, &szpalt_x, &dy) == 5;
    isValid = isValid && ymm <= dy && xcol <= szpalt_x;
    isToSmall = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CAtexKrat message handlers

bool CAtexKrat::CompX(int* sizex, int* s_x) const noexcept
{
    for (*s_x = 1; *s_x < DIM_LIMIT; ++(*s_x)) {
        const auto tmpsizex = (float)xcol / szpalt_x * (float)*s_x;
        *sizex = (int)std::nearbyintf(tmpsizex);
        if (fabs(tmpsizex - *sizex) < *sizex * TOLERANCE)
            return true;
    }
    return false;
}

bool CAtexKrat::CompY(int* sizey, int* s_y) const noexcept
{
    for (*s_y = 1; *s_y < DIM_LIMIT; ++(*s_y)) {
        const auto tmpsizey = (float)ymm / (float)dy * (float)*s_y;
        *sizey = (int)std::nearbyintf(tmpsizey);
        if (fabs(tmpsizey - *sizey) < *sizey * TOLERANCE)
            return true;
    }
    return false;
}

bool CAtexKrat::Compute(int* sizex, int* sizey, int* s_x, int* s_y)
{
    isToSmall = (dy / ymm > (float)DIM_LIMIT || szpalt_x / xcol > (float)DIM_LIMIT);
    if (isToSmall) return false;
    // dopasowanie
    if (xcol == floor(xcol)) { // sprzedane do kraty domyslnej
        *sizex = (int)xcol;
        *s_x = (int)szpalt_x;
        if (!CompY(sizey, s_y)) return false;
    } else {
        if (!CompX(sizex, s_x)) return false;
        if (!CompY(sizey, s_y)) return false;
    }
    // krata paskowa
    if (*s_y >= KRATA_PASKOWA && *s_x == *sizex)
        *s_x = *sizex = 1;
    // rzutuj na istniejaca krate
    for (int x = 1; x < 2 * ADJUST_LEVEL; ++x)
        for (int y = (x < ADJUST_LEVEL ? 1 : x - ADJUST_LEVEL + 1); y <= (x < ADJUST_LEVEL ? x : ADJUST_LEVEL); ++y)
            if (doc->GetCRozm(*s_x*(x - y + 1), *s_y*y, -1)) {
                *sizex *= (x - y + 1);
                *s_x *= (x - y + 1);
                *sizey *= y;
                *s_y *= y;
                return true;
            }
    return false;
}

#pragma endregion

#pragma region CKratCalc 
IMPLEMENT_DYNAMIC(CKratCalc, CDialog)

BEGIN_MESSAGE_MAP(CKratCalc, CDialog)
    ON_NOTIFY_RANGE(UDN_DELTAPOS, IDC_SPINX, IDC_SPINY, &CKratCalc::OnDeltaposSpin)
    ON_CBN_SELCHANGE(IDC_KRATKA, &CKratCalc::OnCbnSelchangeKratka)
    ON_EN_CHANGE(IDC_SP, &CKratCalc::Calculate)
    ON_EN_CHANGE(IDC_SPA, &CKratCalc::Calculate)
    ON_EN_CHANGE(IDC_SIZEX, &CKratCalc::Calculate)
    ON_EN_CHANGE(IDC_SIZEY, &CKratCalc::Calculate)
    ON_BN_CLICKED(IDB_DODAJ, &CKratCalc::OnDefineModelid)
    ON_BN_CLICKED(IDC_DIR_UZU, &CKratCalc::OnModeSwitch)
END_MESSAGE_MAP()

CKratCalc::CKratCalc(CWnd* pParent /*=NULL*/) : CDialog(CKratCalc::IDD, pParent), m_modelid(_T(""))
{
    m_lightx = 40;
    m_lighty = 34;
    m_sizex = m_sizey = "0";
    m_stronax = m_stronay = 0;
    m_userdef_sizex = m_userdef_sizey = 1;
}

void CKratCalc::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_KRATKA, m_kratycombo);
    DDX_Text(pDX, IDC_SIZEX, m_sizex);
    DDX_Text(pDX, IDC_SIZEY, m_sizey);
    DDX_Control(pDX, IDC_DUWAGI, m_wynik);
    DDX_Text(pDX, IDC_SP, m_lightx);
    DDX_Text(pDX, IDC_SPA, m_lighty);
    DDX_Text(pDX, IDC_MODELID, m_modelid);
    DDX_Text(pDX, IDC_TXTPOSX, m_userdef_sizex);
    DDV_MinMaxInt(pDX, m_userdef_sizex, 1, 999);
    DDX_Text(pDX, IDC_TXTPOSY, m_userdef_sizey);
    DDV_MinMaxInt(pDX, m_userdef_sizey, 1, 999);
}

// CKratCalc message handlers
BOOL CKratCalc::OnInitDialog()
{
    CDialog::OnInitDialog();

    CheckDlgButton(IDC_DIR_UZU, BST_CHECKED);

    // ustaw kratki
    if (theApp.activeDoc->m_rozm.empty())
        theApp.activeDoc->IniRozm();

    theApp.FillKrataCombo(m_kratycombo);

    return TRUE;
}

void CKratCalc::OnDeltaposSpin(const UINT spinCtrlId, NMHDR* pNMHDR, LRESULT* pResult)
{
    CString sn;
    const auto idc_edit = spinCtrlId == IDC_SPINX ? IDC_SIZEX : IDC_SIZEY;
    GetDlgItem(idc_edit)->GetWindowText(sn);
    sn.Format(_T("%i"), _ttoi(sn) - reinterpret_cast<LPNMUPDOWN>(pNMHDR)->iDelta);
    if (sn[0] != _T('-')) GetDlgItem(idc_edit)->SetWindowText(sn);
    *pResult = 0;
}

void CKratCalc::OnCbnSelchangeKratka()
{
    CString k;
    m_kratycombo.GetWindowText(k);

    int szpalt_x, szpalt_y;
    if (_stscanf_s(k, _T("%ix%i"), &szpalt_x, &szpalt_y) != 2) return;
    auto r = theApp.activeDoc->GetCRozm(szpalt_x, szpalt_y);
    if (r) {
        m_stronax = szpalt_x * (r->w + r->sw) - r->sw;
        m_stronay = szpalt_y * (r->h + r->sh) - r->sh;
        Calculate();
    } else
        m_wynik.SetWindowText(_T("Brak rozmiaru"));
}

void CKratCalc::Calculate()
{
    CString s;
    UpdateData(TRUE);
    BOOL found{FALSE};
    const double lightx = m_lightx;
    const double lighty = m_lighty;
    const double addx = _ttof(m_sizex) * 10;
    const double addy = _ttof(m_sizey) * 10;

    if (m_stronax <= 0 || m_stronay <= 0 || addx <= 0 || addy <= 0) {
        m_wynik.SetWindowText(_T("Z³e dane"));
        return;
    }

    int i{0}, j{0}, k{0}, l{0};
    double w, h;             // rozmiar modu³u
    double modw{0}, modh{0}; // rozmiar ogloszenia pasuj¹cego do kraty

    for (i = 1; i < CAtexKrat::DIM_LIMIT; ++i) {
        w = (m_stronax - (i - 1) * lightx) / i;
        j = 1;
        while ((modw = j * (w + lightx) - lightx) <= m_stronax) {
            if (fabs(modw - addx) < addx * TOLERANCE)
                break;
            j++;
        }
        if (modw <= m_stronax) break;
    }

    for (k = 1; k < CAtexKrat::DIM_LIMIT; ++k) {
        h = (m_stronay - (k - 1) * lighty) / k;
        l = 1;
        while ((modh = l * (h + lighty) - lighty) <= m_stronay) {
            if (fabs(modh - addy) < addy * TOLERANCE)
                break;
            l++;
        }
        if (modh <= m_stronay) break;
    }

    if (i < CAtexKrat::DIM_LIMIT && modw <= m_stronax) {
        if (k < CAtexKrat::DIM_LIMIT && modh <= m_stronay) {
            m_x = j;
            m_y = l;
            m_kra_sym.Format(_T("%ix%i"), i, k);
            s.Format(_T("%ix%i na %s (%.1fx%.1f)"), j, l, (LPCTSTR)m_kra_sym, (j * (w + lightx) - lightx) / 10, (l * (h + lighty) - lighty) / 10 - (pkt2mm * podpisH));
            found = TRUE;
        } else
            s.Format(_T("%ix# na %ix#"), j, i);
    } else {
        if (k < CAtexKrat::DIM_LIMIT && modh <= m_stronay)
            s.Format(_T("#x%i na #x%i"), l, k);
        else
            s.Format(_T("Nie pasuje"));
    }

    m_wynik.SetWindowText(s);
    GetDlgItem(IDB_DODAJ)->EnableWindow(found);
}


void CKratCalc::OnDefineModelid()
{
    if (!m_autocalcMode) {
        UpdateData(TRUE);
        m_kratycombo.GetWindowText(m_kra_sym);
    }

    GetDlgItem(IDC_MODELID)->GetWindowText(m_modelid);
    CManODPNETParms orapar {
        { CManDbType::DbTypeInt32, m_autocalcMode ? &m_x : &m_userdef_sizex },
        { CManDbType::DbTypeInt32, m_autocalcMode ? &m_y : &m_userdef_sizey },
        { CManDbType::DbTypeVarchar2, &m_kra_sym },
        { CManDbType::DbTypeVarchar2, &m_modelid },
    };

    if (theManODPNET.EI("begin spacer_adm.define_modelid(:sizex,:sizey,:sym,:modelid); end;", orapar))
        AfxMessageBox(_T("Zarejestrowano nowy kod"));
}

void CKratCalc::OnModeSwitch()
{
    m_autocalcMode = IsDlgButtonChecked(IDC_DIR_UZU);
    GetDlgItem(IDC_SIZEX)->EnableWindow(m_autocalcMode);
    GetDlgItem(IDC_SIZEY)->EnableWindow(m_autocalcMode);
    GetDlgItem(IDC_SP)->EnableWindow(m_autocalcMode);
    GetDlgItem(IDC_SPA)->EnableWindow(m_autocalcMode);
    GetDlgItem(IDC_TXTPOSX)->EnableWindow(!m_autocalcMode);
    GetDlgItem(IDC_TXTPOSY)->EnableWindow(!m_autocalcMode);
    GetDlgItem(IDB_DODAJ)->EnableWindow(!m_autocalcMode);
}
#pragma endregion
