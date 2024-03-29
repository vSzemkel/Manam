
#include "pch.h"
#include "DrawDoc.h"
#include "KolDlg.h"
#include "MainFrm.h"
#include "Manam.h"

/////////////////////////////////////////////////////////////////////////////
// CMainFrame
IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
    //{{AFX_MSG_MAP(CMainFrame)
    ON_WM_CREATE()
    ON_COMMAND(ID_VIEW_CUST_TLB, &CMainFrame::OnToolBarCustomize)
    ON_COMMAND(ID_FULLSCREEN, &CMainFrame::OnFullScreen)
    ON_REGISTERED_MESSAGE(AFX_WM_RESETTOOLBAR, &CMainFrame::OnToolBarReset)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// arrays of IDs used to initialize control bars

static constexpr UINT BASED_CODE indicators[] =
{
    ID_SEPARATOR,           // status line indicator
    ID_INDICATOR_OPENSTAT,
    ID_INDICATOR_LOGIN,
    ID_INDICATOR_ROLE,
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
    cyan.DeleteObject();
    magenta.DeleteObject();
    yellow.DeleteObject();
    rzym.DeleteObject();
    robgcolor.DeleteObject();
    pen.DeleteObject();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
        return -1;

    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
    CMFCToolBar::EnableQuickCustomization();
    EnableFullScreenMode(ID_FULLSCREEN);
    afxGlobalData.EnableAccessibilitySupport(FALSE);

    if (!CreateManamToolBar()) {
        TRACE("Failed to create toolbar\n");
        return -3;      // fail to create
    }

    if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(indicators, _countof(indicators))) {
        TRACE("Failed to create status bar\n");
        return -4;      // fail to create
    }

    LOGPEN m_logpen;
    m_logpen.lopnWidth.x = 1;
    m_logpen.lopnWidth.y = 1;
    m_logpen.lopnStyle = PS_SOLID;
    m_logpen.lopnColor = ManColor::Black;
    if (!pen.CreatePenIndirect(&m_logpen))
        return -5;

    //brush
    LOGBRUSH m_logbrush;
    m_logbrush.lbHatch = HS_HORIZONTAL;
    m_logbrush.lbStyle = BS_SOLID;

    m_logbrush.lbColor = ManColor::Cyan;
    if (!cyan.CreateBrushIndirect(&m_logbrush))
        return -6;

    m_logbrush.lbColor = ManColor::Magenta;
    if (!magenta.CreateBrushIndirect(&m_logbrush))
        return -7;

    m_logbrush.lbColor = ManColor::Yellow;
    if (!yellow.CreateBrushIndirect(&m_logbrush))
        return -8;

    m_logbrush.lbColor = ManColor::RomanPage;
    if (!rzym.CreateBrushIndirect(&m_logbrush))
        return -9;

    m_logbrush.lbColor = ManColor::ReadOnlyDoc;
    if (!robgcolor.CreateBrushIndirect(&m_logbrush))
        return -10;

    SetOpenStatus(_T(""));

    return 0;
}

void inline CMainFrame::StoreComboHandlers()
{
    auto m_mfcKolorCombo = dynamic_cast<CMFCToolBarComboBoxButton*>(m_wndToolBar.GetButton(m_wndToolBar.CommandToIndex(IDV_COMBO_COLOR)));
    m_KolorBox = m_mfcKolorCombo->GetComboBox();
    m_LastSessionKolor = m_mfcKolorCombo->GetText();

    auto cbHeadCombo = dynamic_cast<CMFCToolBarComboBoxButton*>(m_wndToolBar.GetButton(m_wndToolBar.CommandToIndex(IDV_COMBO_HEAD)));
    cbHeadCombo->SetDropDownHeight(300);
    m_CaptionBox = cbHeadCombo->GetComboBox();
    m_CaptionEditBox = cbHeadCombo->GetEditCtrl();
}

BOOL CMainFrame::LoadFrame(const UINT nIDResource, const DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
    BOOL bRet = CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext);

    StoreComboHandlers();
    UpdateWindow();

    return bRet;
}

void CMainFrame::OnFullScreen()
{
    ShowFullScreen();

    if (!IsFullScreen())
        StoreComboHandlers();
}

///////////////////////////////// toolbar
bool CMainFrame::CreateManamToolBar()
{
    if (!m_wndMenuBar.CreateEx(this, CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_TOP)) {
        TRACE("Failed to create menubar\n");
        return false;
    }

    if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC)) {
        TRACE("Failed to create toolbar\n");
        return false;
    }
    if (!m_wndToolBar.LoadToolBar(IDR_TOOL_MAIN)) {
        TRACE("Failed to load toolbar\n");
        return false;
    }

    m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUST_TLB, _T("Modyfikuj..."));

    m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockPane(&m_wndMenuBar);
    DockPane(&m_wndToolBar);

    return true;
}

LRESULT CMainFrame::OnToolBarReset(WPARAM wp, LPARAM /*unused*/)
{
    auto uiToolBarId = (UINT)wp;
    switch (uiToolBarId) {
        case IDR_TOOL_MAIN:
            CMFCToolBarComboBoxButton cbKolorCombo(IDV_COMBO_COLOR, GetCmdMgr()->GetCmdImage(IDV_COMBO_COLOR, FALSE));
            m_wndToolBar.ReplaceButton(IDV_COMBO_COLOR, cbKolorCombo);

            CMFCToolBarComboBoxButton cbHeadCombo(IDV_COMBO_HEAD, GetCmdMgr()->GetCmdImage(IDV_COMBO_HEAD, FALSE), CBS_DROPDOWN);
            m_wndToolBar.ReplaceButton(IDV_COMBO_HEAD, cbHeadCombo);

            StoreComboHandlers();
            if (theApp.isRDBMS) {
                lastColToolBar = false;
                InsKolorBox();
            }

            m_wndToolBar.SetWindowText(_T("Narz�dzia"));
            break;
    }
    return 0;
}

void CMainFrame::OnToolBarCustomize()
{
    auto pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE);
    this->OnToolBarReset(IDR_TOOL_MAIN, 0);
    pDlgCust->Create();
}

void CMainFrame::SwapToolbarImages(const int iCmd1, const int iCmd2)
{
    int iPos1 = m_wndToolBar.CommandToIndex(iCmd1);
    auto btn1 = dynamic_cast<CMFCToolBarButton*>(m_wndToolBar.GetButton(iPos1));
    iPos1 = btn1->GetImage();

    int iPos2 = m_wndToolBar.CommandToIndex(iCmd2);
    auto btn2 = dynamic_cast<CMFCToolBarButton*>(m_wndToolBar.GetButton(iPos2));
    iPos2 = btn2->GetImage();

    btn1->SetImage(iPos2);
    btn2->SetImage(iPos1);
}

void CMainFrame::SetToolbarBitmap(const ToolbarMode bPrevMode, const ToolbarMode bNewMode)
{
    if ((bPrevMode == ToolbarMode::normal && bNewMode == ToolbarMode::czas_obow)
        || (bPrevMode == ToolbarMode::czas_obow && bNewMode == ToolbarMode::normal))
        SwapToolbarImages(ID_DRAW_LOCK, IDV_COMBO_COLOR);
    else if ((bPrevMode == ToolbarMode::normal && bNewMode == ToolbarMode::tryb_studia)
        || (bPrevMode == ToolbarMode::tryb_studia && bNewMode == ToolbarMode::normal))
        SwapToolbarImages(ID_DRAW_KOLOR, IDV_COMBO_HEAD);
    else if (bPrevMode != bNewMode) {
        SwapToolbarImages(ID_DRAW_LOCK, IDV_COMBO_COLOR);
        SwapToolbarImages(ID_DRAW_KOLOR, IDV_COMBO_HEAD);
    }

    m_wndToolBar.Invalidate(FALSE);
    // by� mo�e trzeba od�wie�y� combo
    LoadKolorCombo();
    DBIniCaptionCombo(theApp.GetProfileInt(_T("General"), _T("Captions"), 1) == 1, theApp.activeDoc->id_drw);
}

void CMainFrame::SetOpenStatus(LPCTSTR t)
{
    int width = 36; // only two nonempty cases: WRITE and READ 
    if (t[0] == 'R') width -= 6;
    m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_OPENSTAT, t[0] != TCHAR{0} ? SBPS_NORMAL : SBPS_NOBORDERS | SBPS_DISABLED, width);
    m_wndStatusBar.SetPaneText(1, t);
    if (MDIGetActive()) { // w grzbiecie ukryj toolbar
        const BOOL showToolbar = theApp.activeDoc->iDocType != DocType::grzbiet_drukowany;
        ShowPane(&m_wndToolBar, showToolbar, TRUE, showToolbar);
    }
}

void CMainFrame::SetLogonStatus(LPCTSTR t)
{
    auto dc = GetDC();
    const CSize s = dc->GetTextExtent(t, (int)_tcslen(t));
    ReleaseDC(dc);
    m_wndStatusBar.SetPaneInfo(2, ID_INDICATOR_LOGIN, SBPS_NORMAL, (int)(0.8 * s.cx));
    m_wndStatusBar.SetPaneText(2, t);
    SetRoleStatus();
}

void CMainFrame::SetRoleStatus()
{
    LPCTSTR role = [] {
        if (theApp.grupa & UserRole::dea)
            return _T("DEALER");
        else if (theApp.grupa & UserRole::mas)
            return _T("MASTER");
        else if (theApp.grupa & UserRole::adm)
            return _T("ADMINISTRATOR");
        else if (theApp.grupa & UserRole::kie)
            return _T("KIEROWNIK");
        else if (theApp.grupa & UserRole::stu)
            return _T("STUDIO");
        else if (theApp.grupa & UserRole::red)
            return _T("REDAKTOR");
        else
            return _T("GO��");
    }();

    auto dc = GetDC();
    const CSize s = dc->GetTextExtent(role);
    ReleaseDC(dc);

    m_wndStatusBar.SetPaneInfo(3, ID_INDICATOR_ROLE, SBPS_NORMAL, (int)(0.8 * s.cx));
    m_wndStatusBar.SetPaneText(3, role);
}

//////////////// ini spoty o ile sa
void CMainFrame::InsComboNrSpotow(const int new_i)
{
    if (theApp.swCZV == ToolbarMode::tryb_studia) return;
    const auto old_i = m_KolorBox->GetCount() - (int)CDrawDoc::kolory.size();
    if (old_i > new_i)
        for (int i = old_i - 1; i >= new_i; --i)
            m_KolorBox->DeleteString(i);
    else if (old_i >= 0)
        for (int i = old_i; i < new_i; ++i)
            m_KolorBox->InsertString(i, (LPCTSTR)_T("SPOT ") + CDrawObj::Rzymska(i + 1));
    m_KolorBox->Invalidate();
}

void CMainFrame::LoadKolorCombo()
{
    const bool isStudio = theApp.swCZV == ToolbarMode::tryb_studia;
    if (!lastColToolBar && isStudio) // ostatnio od�wie�ono wersje
        return;

    if (isStudio) {
        m_KolorBox->ResetContent();
        m_KolorBox->AddString(_T(""));
        theManODPNET.FillCombo(m_KolorBox, "select wersja from spacer_wersje_eps order by 1", CManODPNET::emptyParm);
    } else if (m_KolorBox->GetCount() != (int)CDrawDoc::kolory.size()) {
        m_KolorBox->ResetContent();
        // kolory
        for (const auto& k : CDrawDoc::kolory)
            m_KolorBox->AddString(k);
        m_KolorBox->SelectString(CB_ERR, m_LastSessionKolor);
        // spoty
        if (MDIGetActive())
            InsComboNrSpotow(theApp.activeDoc->DBReadSpot((int)theApp.activeDoc->m_pages.size()));
    }
    m_KolorBox->Invalidate();
    lastColToolBar = !isStudio;
}

void CMainFrame::InsKolorBox()
{
    theApp.isRDBMS ? theManODPNET.IniKolorTable() : CDrawDoc::IniKolorTable();
    LoadKolorCombo();
}

////////////////////////////////////////////////////////// captions /lub str_log
void CMainFrame::IniCaptionBox(const int id_drw, const int new_id_drw)
{
    if (new_id_drw == id_drw) // tylko przy zmianie produktu
        return;

    m_CaptionEditBox->SetWindowText(_T(""));
    if (new_id_drw == INT_MAX)
        m_CaptionBox->ResetContent();
    else {
        const auto isCap = theApp.GetProfileInt(_T("General"), _T("Captions"), 1) == 1;
        if (new_id_drw > 0 && theApp.isRDBMS && !DBIniCaptionCombo(isCap, new_id_drw))
            IniCaptionCombo(isCap);
    }
}

DWORD CMainFrame::GetCaptionDataItem(int ind) const
{
    if (ind < 0) ind = m_CaptionBox->GetCurSel();
    return ind == CB_ERR ? 0 : (DWORD)m_CaptionBox->GetItemData(ind);
}

void CMainFrame::IniCaptionCombo(const bool iscaption)
{
    CString bf, tx = (iscaption) ? _T("Caption") : _T("StrLog");
    const int max_col = theApp.GetProfileInt(tx, _T("Amount"), 0);
    m_CaptionBox->ResetContent();
    m_CaptionBox->AddString(_T(""));
    for (int i = 1; i <= max_col; ++i) {
        bf.Format(_T("%i"), i);
        m_CaptionBox->AddString(theApp.GetProfileString(tx, tx + bf, _T("")));
    }
    m_CaptionBox->SetCurSel(0);
    m_CaptionBox->Invalidate();
}

BOOL CMainFrame::DBIniCaptionCombo(const bool iscaption, int id_drw)
{
    // nie od�wie�aj niepotrzebnie
    const auto checksum = (iscaption << 24) + ((int)theApp.swCZV << 16) + id_drw;
    if (lastCapToolBar == checksum) return TRUE;
    lastCapToolBar = checksum;

    const auto czyListaPagin = theApp.swCZV == ToolbarMode::tryb_studia;
    auto sql = reinterpret_cast<char*>(theApp.bigBuf);
    StringCchPrintfA(sql, bigSize, "select %s drw_xx=:xx order by 1",
        czyListaPagin ? "text,xx from spacer_naglowki_prn where"
        : (iscaption ? "naglowek from spacer_naglowki where"
        : "sciezka from strukt_drzewa where co_to='L' and"));

    m_CaptionBox->ResetContent();
    const int ind = m_CaptionBox->AddString(_T(""));
    m_CaptionBox->SetItemData(ind, (WORD)0);

    CManODPNETParms orapar { CManDbType::DbTypeInt32, &id_drw };
    if (!theManODPNET.FillCombo(m_CaptionBox, sql, orapar, czyListaPagin ? 1 : -1)) return FALSE;

    m_CaptionBox->SetCurSel(0);
    m_CaptionBox->Invalidate();

    return TRUE;
}

//////////////////////////////// wyciaganie danych z tablic
CString CMainFrame::GetKolorText() const noexcept
{
    CString s;
    const int sel = m_KolorBox->GetCurSel();
    if (sel != CB_ERR)
        m_KolorBox->GetLBText(sel, s);

    return s;
}

int CMainFrame::GetKolorInd(LPCTSTR text) const noexcept
{
    const int ind = m_KolorBox->FindString(0, text);
    return ind == CB_ERR ? -1 : ind;
}

int CMainFrame::GetKolor(const int ile_spotow) const // kolejno�� na liscie: spoty, brak, full, kolory
{
    const auto k = m_KolorBox->GetCurSel();

    if (k == ile_spotow)
        return ColorId::brak;
    if (k == ile_spotow + 1)
        return ColorId::full;
    if (k == CB_ERR)
        return CB_ERR;
    if (k < ile_spotow)
        return ((k << 3) + ColorId::spot); // index w tablicy spotow
    return ((k - ile_spotow) << 3); // gdy nie jest ustawiony spot bit
}

////////////////////////////////// naglowek
CString CMainFrame::GetCaption() const
{
    CString rs;
    m_CaptionEditBox->GetWindowText(rs);
    return rs;
}

CString CMainFrame::GetCaption(const int i) const
{
    CString rs;
    m_CaptionBox->GetLBText(i, rs);
    return rs;
}

CString CMainFrame::GetCapStrFromData(const DWORD_PTR captionId) const
{
    CString cap;
    const int captionCnt = m_CaptionBox->GetCount();

    for (int i = 0; i < captionCnt; ++i)
        if (m_CaptionBox->GetItemData(i) == captionId)
            m_CaptionBox->GetLBText(i, cap);

    return cap;
}

int CMainFrame::GetCaptionBoxSize() const
{
    return m_CaptionBox->GetCount();
}

void CMainFrame::SetStatusBarInfo(LPCTSTR tx)
{
    m_wndStatusBar.SetPaneText(ID_SEPARATOR, tx);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG
