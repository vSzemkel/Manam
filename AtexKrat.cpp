// AtexKrat.cpp : implementation file
// p.xsize*pg.colwidth # p.ysizemm # p.xsize #	pg.columns # pg.height

#include "StdAfx.h"
#include "AtexKrat.h"
#include "Manam.h"
#include "DrawDoc.h"

const int CAtexKrat::DIM_LIMIT = 16;
const int CAtexKrat::ADJUST_LEVEL = 6;
const int CAtexKrat::KRATA_PASKOWA = 10;
const float CAtexKrat::TOLERANCE = 0.05f;

#pragma region CAtexKrat
CAtexKrat::CAtexKrat(const TCHAR *atexKratInfo, CDrawDoc *vDoc)
{
    doc = vDoc;
    isValid = _stscanf_s(atexKratInfo, _T("%f#%f#%f#%f#%f"), &xmm, &ymm, &xcol, &szpalt_x, &dy) == 5;
    isValid = isValid && ymm <= dy && xcol <= szpalt_x;
    isToSmall = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CAtexKrat message handlers

BOOL CAtexKrat::CompX(int *sizex, int *s_x) const
{
    for (*s_x = 1; *s_x < DIM_LIMIT; (*s_x)++) {
        const auto tmpsizex = (float)xcol / szpalt_x * (float)*s_x;
        *sizex = (int)nearbyintf(tmpsizex);
        if (fabs(tmpsizex - *sizex) < *sizex*TOLERANCE)
            return TRUE;
    }
    return FALSE;
}

BOOL CAtexKrat::CompY(int *sizey, int *s_y) const
{
    for (*s_y = 1; *s_y < DIM_LIMIT; (*s_y)++) {
        const auto tmpsizey = (float)ymm / (float)dy * (float)*s_y;
        *sizey = (int)nearbyintf(tmpsizey);
        if (fabs(tmpsizey - *sizey) < *sizey * TOLERANCE)
            return TRUE;
    }
    return FALSE;
}

BOOL CAtexKrat::Compute(int *sizex, int *sizey, int *s_x, int *s_y)
{
    isToSmall = (dy / ymm > (float)DIM_LIMIT || szpalt_x / xcol > (float)DIM_LIMIT);
    if (isToSmall) return FALSE;
    // dopasowanie	
    if (xcol == floor(xcol)) { // sprzedane do kraty domyslnej
        *sizex = (int)xcol;
        *s_x = (int)szpalt_x;
        if (!CompY(sizey, s_y)) return FALSE;
    } else {
        if (!CompX(sizex, s_x)) return FALSE;
        if (!CompY(sizey, s_y)) return FALSE;
    }
    // krata paskowa
    if (*s_y >= KRATA_PASKOWA && *s_x == *sizex)
        *s_x = *sizex = 1;
    // rzutuj na istniejaca krate
    for (int x = 1; x < 2 * ADJUST_LEVEL; x++)
        for (int y = (x < ADJUST_LEVEL ? 1 : x - ADJUST_LEVEL + 1); y <= (x < ADJUST_LEVEL ? x : ADJUST_LEVEL); y++)
            if (doc->GetCRozm(*s_x*(x - y + 1), *s_y*y, -1)) {
                *sizex *= (x - y + 1);
                *s_x *= (x - y + 1);
                *sizey *= y;
                *s_y *= y;
                return TRUE;
            }
    return FALSE;
}

#pragma endregion

#pragma region CKratCalc 
const double CKratCalc::TOLERANCE = 0.011f;

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
    stronax = stronay = 0;
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
    if (theApp.activeDoc->m_Rozm.empty())
        theApp.activeDoc->IniRozm();

    theApp.FillKrataCombo(m_kratycombo);

    return TRUE;
}

void CKratCalc::OnDeltaposSpin(UINT spinCtrlId, NMHDR* pNMHDR, LRESULT* pResult)
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
        stronax = szpalt_x*(r->w + r->sw) - r->sw;
        stronay = szpalt_y*(r->h + r->sh) - r->sh;
        Calculate();
    } else m_wynik.SetWindowText(_T("Brak rozmiaru"));
}

void CKratCalc::Calculate()
{
    CString s;
    UpdateData(TRUE);
    BOOL bFound = FALSE;
    const double lightx = m_lightx;
    const double lighty = m_lighty;
    const double addx = _ttof(m_sizex) * 10;
    const double addy = _ttof(m_sizey) * 10;

    if (stronax <= 0 || stronay <= 0 || addx <= 0 || addy <= 0) {
        m_wynik.SetWindowText(_T("Z³e dane"));
        return;
    }

    int i = 0, j = 0, k = 0, l = 0;
    double w, h;				// rozmiar modu³u
    double modw = 0, modh = 0;	// rozmiar ogloszenia pasuj¹cego do kraty

    for (i = 1; i < CAtexKrat::DIM_LIMIT; ++i) {
        w = (stronax - (i - 1) * lightx) / i;
        j = 1;
        while ((modw = j * (w + lightx) - lightx) <= stronax) {
            if (fabs(modw - addx) < addx * TOLERANCE)
                break;
            j++;
        }
        if (modw <= stronax) break;
    }

    for (k = 1; k < CAtexKrat::DIM_LIMIT; k++) {
        h = (stronay - (k - 1) * lighty) / k;
        l = 1;
        while ((modh = l * (h + lighty) - lighty) <= stronay) {
            if (fabs(modh - addy) < addy * TOLERANCE)
                break;
            l++;
        }
        if (modh <= stronay) break;
    }

    if (i < CAtexKrat::DIM_LIMIT && modw <= stronax) {
        if (k < CAtexKrat::DIM_LIMIT && modh <= stronay) {
            m_x = j;
            m_y = l;
            m_kra_sym.Format(_T("%ix%i"), i, k);
            s.Format(_T("%ix%i na %s (%.1fx%.1f)"), j, l, (LPCTSTR)m_kra_sym, (j * (w + lightx) - lightx) / 10, (l * (h + lighty) - lighty) / 10 - (pkt2mm * podpisH));
            bFound = TRUE;
        } else
            s.Format(_T("%ix# na %ix#"), j, i);
    } else {
        if (k < CAtexKrat::DIM_LIMIT && modh <= stronay)
            s.Format(_T("#x%i na #x%i"), l, k);
        else
            s.Format(_T("Nie pasuje"));
    }

    m_wynik.SetWindowText(s);
    GetDlgItem(IDB_DODAJ)->EnableWindow(bFound);
}


void CKratCalc::OnDefineModelid()
{
    if (!bCalcMode) {
        UpdateData(TRUE);
        m_kratycombo.GetWindowText(m_kra_sym);
    }

    GetDlgItem(IDC_MODELID)->GetWindowText(m_modelid);
    CManODPNETParms orapar {
        { CManDbType::DbTypeInt32, bCalcMode ? &m_x : &m_userdef_sizex },
        { CManDbType::DbTypeInt32, bCalcMode ? &m_y : &m_userdef_sizey },
        { CManDbType::DbTypeVarchar2, &m_kra_sym },
        { CManDbType::DbTypeVarchar2, &m_modelid },
    };

    if (theManODPNET.EI("begin spacer_adm.define_modelid(:sizex,:sizey,:sym,:modelid); end;", orapar))
        AfxMessageBox(_T("Zarejestrowano nowy kod"));
}

void CKratCalc::OnModeSwitch()
{
    bCalcMode = IsDlgButtonChecked(IDC_DIR_UZU);
    GetDlgItem(IDC_SIZEX)->EnableWindow(bCalcMode);
    GetDlgItem(IDC_SIZEY)->EnableWindow(bCalcMode);
    GetDlgItem(IDC_SP)->EnableWindow(bCalcMode);
    GetDlgItem(IDC_SPA)->EnableWindow(bCalcMode);
    GetDlgItem(IDC_TXTPOSX)->EnableWindow(!bCalcMode);
    GetDlgItem(IDC_TXTPOSY)->EnableWindow(!bCalcMode);
    GetDlgItem(IDB_DODAJ)->EnableWindow(!bCalcMode);
}
#pragma endregion
