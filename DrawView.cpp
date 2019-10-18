
#include "StdAfx.h"
#include "ManPDF.h"
#include "DrawAdd.h"
#include "DrawDoc.h"
#include "DrawOpis.h"
#include "DrawPage.h"
#include "DrawTool.h"
#include "DrawView.h"
#include "GenEpsInfoDlg.h"
#include "GridFrm.h"
#include "KolDlg.h"
#include "MainFrm.h"
#include "Manam.h"
#include "QueView.h"

extern BOOL disableMenu;
extern BOOL drawErrorBoxes;
extern BOOL cancelGenEPS;

// private clipboard format (list of Draw objects)
CLIPFORMAT CDrawView::m_cfDraw = (CLIPFORMAT)::RegisterClipboardFormat(_T("Automatyczne makietowanie"));
/////////////////////////////////////////////////////////////////////////////

static constexpr std::array rgiZoomFactor = { 60, 70, 85, 100, 125, 150, 200, 250, 300, 400, 500, 750, 1000 };

/////////////////////////////////////////////////////////////////////////////
// CDrawView

IMPLEMENT_DYNCREATE(CDrawView, CScrollView)

BEGIN_MESSAGE_MAP(CDrawView, CScrollView)
    //{{AFX_MSG_MAP(CDrawView)
    ON_WM_LBUTTONUP()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSEWHEEL()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_SIZE()
    ON_WM_DESTROY()
    ON_WM_SETFOCUS()
    ON_WM_SETCURSOR()
    ON_WM_ERASEBKGND()
    ON_COMMAND(ID_CANCEL_EDIT, &CDrawView::OnCancelEdit)
    ON_COMMAND_RANGE(ID_DRAW_ADD, ID_DRAW_SPACELOCKED, &CDrawView::OnDrawTool)
    ON_UPDATE_COMMAND_UI(ID_DRAW_ADD, &CDrawView::OnUpdateDrawAdd)
    ON_UPDATE_COMMAND_UI_RANGE(ID_DRAW_RED, ID_DRAW_SPACELOCKED, &CDrawView::OnUpdateDrawTool)
    ON_UPDATE_COMMAND_UI_RANGE(ID_OBJECT_MOVETOFRONT, ID_OBJECT_MOVEBACK, &CDrawView::OnUpdateSingleSelect)
    ON_COMMAND(ID_EDIT_SELECT_ALL, &CDrawView::OnEditSelectAll)
    ON_COMMAND(ID_EDIT_CLEAR, &CDrawView::OnEditClear)
    ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, &CDrawView::OnUpdateAnySelect)
    ON_COMMAND(ID_VIEW_SPACELOCKS, &CDrawView::OnViewSpaceLocks)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SPACELOCKS, &CDrawView::OnUpdateViewSpaceLocks)
    ON_COMMAND(ID_ADD_OPCJE, &CDrawView::OnImportOpcje)
    ON_UPDATE_COMMAND_UI(ID_ADD_OPCJE, &CDrawView::OnUpdateImportOpcje)
    ON_COMMAND(ID_OBJECT_MOVEBACK, &CDrawView::OnObjectMoveBack)
    ON_COMMAND(ID_OBJECT_MOVEFORWARD, &CDrawView::OnObjectMoveForward)
    ON_COMMAND(ID_OBJECT_MOVETOBACK, &CDrawView::OnObjectMoveToBack)
    ON_COMMAND(ID_OBJECT_MOVETOFRONT, &CDrawView::OnObjectMoveToFront)
    ON_COMMAND(ID_EDIT_COPY, &CDrawView::OnEditCopy)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &CDrawView::OnUpdateEditCopy)
    ON_COMMAND(ID_EDIT_CUT, &CDrawView::OnEditCut)
    ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, &CDrawView::OnUpdateEditCut)
    ON_COMMAND(ID_EDIT_PASTE, &CDrawView::OnEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &CDrawView::OnUpdateEditPaste)
    ON_COMMAND(ID_EDIT_PROPERTIES, &CDrawView::OnEditProperties)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PROPERTIES, &CDrawView::OnUpdateEditProperties)
    ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, &CDrawView::OnUpdateEditSelectAll)
    ON_COMMAND(ID_VIEW_ZOOMCUSTOM, &CDrawView::OnViewZoomCustom)
    ON_COMMAND_RANGE(ID_VIEW_ZOOM100, ID_VIEW_ZOOM750, &CDrawView::OnViewZoomN)
    ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_ZOOM100, ID_VIEW_ZOOMCUSTOM, &CDrawView::OnUpdateViewZoomN)
    ON_COMMAND(ID_VIEW_LUPKA_PLUS, &CDrawView::OnViewLupkaPlus)
    ON_COMMAND(ID_VIEW_LUPKA_MINUS, &CDrawView::OnViewLupkaMinus)
    ON_COMMAND(ID_VU_MAK_STRONY, &CDrawView::OnDrawMakStrony)
    ON_UPDATE_COMMAND_UI(ID_VU_MAK_STRONY, &CDrawView::OnUpdateDrawMakStrony)
    ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, &CDrawView::OnDisableMenu) //OnDisableMenu - wygaszanie kontrolek menu i toolbaru
    ON_UPDATE_COMMAND_UI(ID_FILE_DBOPEN, &CDrawView::OnDisableMenuRDBMS)
    ON_UPDATE_COMMAND_UI(ID_ADD_IMPORTMINUS, &CDrawView::OnDisableMenuLIB)
    ON_UPDATE_COMMAND_UI(ID_ADD_DBIMPORTMINUS, &CDrawView::OnDisableMenuDEAL)
    ON_UPDATE_COMMAND_UI(ID_VU_CK_MAKIETOWANIE, &CDrawView::OnUpdateVuCkMakietowanie)
    ON_COMMAND(ID_VIEW_ADDDESC, &CDrawView::OnViewAdddesc)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CDrawView::OnFilePrintPreview)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW_1P, &CDrawView::OnFilePrintPreview1p)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW_ALL, &CDrawView::OnFilePrintPreviewAll)
    ON_COMMAND(ID_FILE_PRINT, &CDrawView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_1P, &CDrawView::OnFilePrint1p)
    ON_COMMAND(ID_FILE_PRINT_ALL, &CDrawView::OnFilePrintAll)
    ON_COMMAND(ID_DRAW_OPCJE, &CDrawView::OnDrawOpcje)
    ON_UPDATE_COMMAND_UI(ID_VIEW_CZASOBOW, &CDrawView::OnUpdateViewCzasobow)
    ON_COMMAND(ID_VIEW_CZASOBOW, &CDrawView::OnViewCzasobow)
    ON_COMMAND(ID_FILE_PRINT_EPS, &CDrawView::OnPrintEps) //GN
    ON_COMMAND(ID_CHECK_EPS, &CDrawView::OnCheckEps)      //GN
    ON_COMMAND(IDM_ASIDE_ADDS, &CDrawView::OnAsideAdds)
    ON_COMMAND(IDM_VIEW_STUDIO, &CDrawView::OnViewStudio)
    ON_UPDATE_COMMAND_UI(IDM_VIEW_STUDIO, &CDrawView::OnUpdateViewStudio)
    ON_UPDATE_COMMAND_UI_RANGE(ID_FILE_NEW, ID_EDIT_REDO, &CDrawView::OnDisableMenu)
    ON_UPDATE_COMMAND_UI_RANGE(ID_ADD_FIND, IDM_ASIDE_ADDS, &CDrawView::OnDisableMenu)
    ON_UPDATE_COMMAND_UI(ID_ADD_SYNCHRONIZE, &CDrawView::OnDisableMenuDEAL)
    ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_EPS, &CDrawView::OnDisableMenuSTU)
    ON_UPDATE_COMMAND_UI(ID_ADD_EXPORT, &CDrawView::OnDisableMenuLIB)
    ON_UPDATE_COMMAND_UI(ID_VU_MAKIETOWANIE, &CDrawView::OnDisableMenuLIB)
    ON_UPDATE_COMMAND_UI(ID_CHECK_EPS, &CDrawView::OnDisableMenuSTU)
    ON_COMMAND(IDM_CHANGEVIEW, &CDrawView::OnChangeView)
    ON_COMMAND(IDM_PREV_KOR, &CDrawView::OnPrevKor)
    ON_UPDATE_COMMAND_UI(IDM_PREV_KOR, &CDrawView::OnDisableMenu)
    ON_COMMAND(ID_FILE_PRINT, &CDrawView::OnFilePrint)
    ON_COMMAND(ID_VIEW_PAGEFONT, &CDrawView::OnChoosePageFont)
    ON_COMMAND(ID_VIEW_ADDFONT, &CDrawView::OnChooseAddFont)
    ON_COMMAND(IDM_IRFAN, &CDrawView::OnIrfan)
    ON_UPDATE_COMMAND_UI(IDM_IRFAN, &CDrawView::OnDisableMenuAdSelOPI)
    ON_COMMAND(IDM_PREVPDF, &CDrawView::OnPrevPdf)
    ON_UPDATE_COMMAND_UI(IDM_PREVPDF, &CDrawView::OnDisableMenuAdPageSel)
    ON_COMMAND(IDM_PREVDIG, &CDrawView::OnPrevDig)
    ON_UPDATE_COMMAND_UI(IDM_PREVDIG, &CDrawView::OnDisableMenuAdSel)
    ON_COMMAND(IDM_ATEXSYG, &CDrawView::OnAtexSyg)
    ON_UPDATE_COMMAND_UI(IDM_ATEXSYG, &CDrawView::OnDisableMenuAdSel)
    ON_UPDATE_COMMAND_UI(ID_FULLSCREEN, &CDrawView::OnDisableMenu)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDrawView construction/destruction

CDrawView::CDrawView() noexcept : m_zoomNum(theApp.m_initZoom, theApp.m_initZoom),
    m_zoomDenom(100 * vscale, -100 * vscale) 
{
}

int CDrawView::OnDisableMenuInt(CCmdUI* pCmdUI)
{
    /* vu: jesli zwroci 0 to ustawienie statusu odbedzie sie w funkcji wolajacej */
    if (disableMenu) pCmdUI->Enable(FALSE);
    return disableMenu;
}

void CDrawView::OnDisableMenu(CCmdUI* pCmdUI) { pCmdUI->Enable(!disableMenu); }

void CDrawView::OnDisableMenuRDBMS(CCmdUI* pCmdUI) { pCmdUI->Enable(!disableMenu && theApp.isRDBMS); }

void CDrawView::OnDisableMenuLIB(CCmdUI* pCmdUI) { pCmdUI->Enable(!disableMenu && GetDocument()->iDocType != DocType::makieta_lib); }

void CDrawView::OnDisableMenuDEAL(CCmdUI* pCmdUI) { pCmdUI->Enable(!disableMenu && GetDocument()->iDocType != DocType::makieta_lib && !(theApp.grupa & (UserRole::dea))); }

void CDrawView::OnDisableMenuSTU(CCmdUI* pCmdUI) { pCmdUI->Enable(!disableMenu && GetDocument()->iDocType != DocType::makieta_lib && (theApp.grupa & UserRole::stu)); }

void CDrawView::OnDisableMenuAdSel(CCmdUI* pCmdUI) { pCmdUI->Enable(!disableMenu && GetDocument()->iDocType != DocType::makieta_lib && m_selection.size() == 1 && dynamic_cast<CDrawAdd*>(m_selection.front())); }

void CDrawView::OnDisableMenuAdPageSel(CCmdUI* pCmdUI) { pCmdUI->Enable(!disableMenu && GetDocument()->iDocType != DocType::makieta_lib && m_selection.size() == 1 && (dynamic_cast<CDrawAdd*>(m_selection.front()) || dynamic_cast<CDrawPage*>(m_selection.front()))); }

void CDrawView::OnDisableMenuAdSelOPI(CCmdUI* pCmdUI) { pCmdUI->Enable(!disableMenu && GetDocument()->iDocType != DocType::makieta_lib && !theApp.isOpiMode && m_selection.size() == 1 && dynamic_cast<CDrawAdd*>(m_selection.front())); }

void CDrawView::OnDisableMenuAdSelSTU(CCmdUI* pCmdUI) { pCmdUI->Enable(!disableMenu && GetDocument()->iDocType != DocType::makieta_lib && (theApp.grupa & UserRole::stu) && m_selection.size() == 1 && dynamic_cast<CDrawAdd*>(m_selection.front())); }

BOOL CDrawView::PreCreateWindow(CREATESTRUCT& cs)
{
    ASSERT(cs.style & WS_CHILD);
    if (cs.lpszClass == nullptr)
        cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS);
    return TRUE;
}

void CDrawView::OnActivateView(const BOOL bActivate, CView* pActiveView, CView* pDeactiveView)
{
    CView::OnActivateView(bActivate, pActiveView, pDeactiveView);

    // invalidate selections when active status changes

    if (m_bActive != bActivate) {
        if (bActivate) // if becoming active update as if active
            OnUpdate(pActiveView, HINT_UPDATE_COMBOBOXY, pDeactiveView);
        else if (dynamic_cast<CGridFrm *>(pActiveView))
            CDrawTool::c_drawShape = DrawShape::select;

        if (!m_selection.empty())
            OnUpdate(nullptr, HINT_UPDATE_SELECTION, nullptr);
        m_bActive = bActivate;
    }

    if (bActivate) {
        if (pActiveView && theApp.activeDoc->swCZV != theApp.swCZV) {
            const auto bPrevMode = theApp.swCZV;
            theApp.swCZV = ((CDrawDoc*)pActiveView->GetDocument())->swCZV;
            ((CMainFrame *)AfxGetMainWnd())->SetToolbarBitmap(bPrevMode, theApp.swCZV);
        }
        ((CMainFrame *)AfxGetMainWnd())->SetOpenStatus(GetDocument()->isRO ? _T("READ") : _T("WRITE"));
    }
}

BOOL CDrawView::OnSetCursor(CWnd* pWnd, const UINT nHitTest, const UINT message)
{

    if (CDrawTool::c_drawShape == DrawShape::add || CDrawTool::c_drawShape == DrawShape::opis)
        return ::SetCursor(::LoadCursor(NULL, IDC_CROSS)) > 0;

    int cursor = -1;
    if (CDrawTool::c_drawShape == DrawShape::caption)
        cursor = IDC_CAPTION;
    else if (CDrawTool::c_drawShape == DrawShape::color)
        cursor = IDC_KOLOR;
    else if (CDrawTool::c_drawShape == DrawShape::lock)
        cursor = (GetDocument()->swCZV == ToolbarMode::czas_obow) ? IDC_FLAG : IDC_LOCK;
    else if (CDrawTool::c_drawShape == DrawShape::deadline)
        cursor = IDC_DEADLINE;
    else if (CDrawTool::c_drawShape == DrawShape::space)
        cursor = IDC_SPA;
    else if (CDrawTool::c_drawShape == DrawShape::red)
        cursor = IDC_RED;

    if (cursor > 0)
        return ::SetCursor(theApp.LoadCursor(cursor)) > 0;
    else
        return CScrollView::OnSetCursor(pWnd, nHitTest, message);
}

/////////////////////////////////////////////////////////////////////////////
// CDrawView drawing
void CDrawView::InvalObj(CDrawObj* pObj)
{
    CRect rect = pObj->m_position;
    DocToClient(rect);
    if (dynamic_cast<CDrawAdd *>(pObj)) {
        rect.bottom += (rect.Height() > 1.2 * TXTSHIFT ? 3 : (int)(1.2 * TXTSHIFT));
        rect.left -= TXTSHIFT;
        rect.right += TXTSHIFT;
        rect.top -= TXTSHIFT;
    } else {
        auto inflation = 3 * vscale;
        rect.InflateRect(inflation, inflation);
    }

    InvalidateRect(rect, TRUE);
}

void CDrawView::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* pHint)
{
    switch (lHint) {
        case HINT_UPDATE_WINDOW:   // redraw entire window
        case HINT_UPDATE_DRAWVIEW: // redraw entire window
            Invalidate(TRUE);
            break;

        case HINT_UPDATE_DRAWOBJ: // a single object has changed
            InvalObj((CDrawObj *)pHint);
            break;

        case HINT_UPDATE_COMBOBOXY:
            ((CMainFrame *)AfxGetMainWnd())->InsComboNrSpotow((int)GetDocument()->m_spot_makiety.size());
            ((CMainFrame *)AfxGetMainWnd())->IniCaptionBox(-1, GetDocument()->id_drw);
            break;

        case HINT_SAVEAS_DELETE_SELECTION: // to usuwa ogloszenia z m_selection
            m_selection.clear();
            Invalidate(TRUE);
            break;

        case HINT_UPDATE_SELECTION:
        { // an entire selection has changed
            for (const auto& pObj : m_selection)
                InvalObj(pObj);
            break;
        }

        case HINT_DELETE_SELECTION: // an entire selection has been removed
        case HINT_DELETE_FROM_GRID: // nie dla nas tylko dla gridu
        case HINT_UPDATE_GRID:
        case HINT_EDIT_PASTE: // an entire selection has changed
            break;

        default:
            ASSERT(FALSE);
            break;
    }
}

void CDrawView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
    if (pInfo && pInfo->GetMaxPage() == 1 && m_pagesPrinted == PrintFormat::doc && theApp.GetProfileInt(_T("General"), _T("ForceLandscape"), 0) > 0) {
        LPDEVMODE dm = SetLandscape();
        if (dm) pDC->ResetDC(dm);
    }

    CScrollView::OnPrepareDC(pDC, pInfo);

    pDC->SetMapMode(MM_ANISOTROPIC);
    pDC->SetViewportExt(m_zoomNum);
    pDC->SetWindowExt(m_zoomDenom);
    if (pInfo) {
        const auto deviceModuleWidth = pDC->GetDeviceCaps(HORZRES) / (6 * GetDocument()->m_pagerow_size);
        const auto deviceModuleHeigth = pDC->GetDeviceCaps(VERTRES) / (theApp.colsPerPage * 40 + 2); // 40 - inaczej wchodza nastepne strony - 2 na naglowek
        pDC->SetViewportOrg(2 * deviceModuleWidth, 0);
        pDC->SetWindowOrg(0, (pInfo->m_nCurPage - 1) * (-(theApp.colsPerPage * 40 + 2)) * (pmoduly)+(int)floor((float)pmoduly / 3));
        pDC->SetViewportExt(deviceModuleWidth, deviceModuleHeigth);
        pDC->SetWindowExt(pmodulx, -pmoduly);
    }
}

BOOL CDrawView::OnScrollBy(const CSize sizeScroll, const BOOL bDoScroll)
{
    // do the scroll
    if (!CScrollView::OnScrollBy(sizeScroll, bDoScroll))
        return FALSE;

    // update the position of any in-place active item
    if (bDoScroll) {
        Invalidate(TRUE);
        UpdateActiveItem(); //inv
        UpdateWindow();
    }
    return TRUE;
}

void CDrawView::OnDraw(CDC* pDC)
{
    CDrawDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

    CBitmap bitmap;
    CBitmap* pOldBitmap;

    // only paint the rect that needs repainting
    CRect client;
    pDC->GetClipBox(client);
    CRect rect;
    GetClientRect(&rect);

    CDC bitmapDC;
    auto pDrawDC = pDC;
    if (!pDC->IsPrinting()) {
        // draw to offscreen bitmap for fast looking repaints
        if (bitmapDC.CreateCompatibleDC(pDC)) {
            if (bitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height())) {
                OnPrepareDC(&bitmapDC, nullptr);
                pDrawDC = &bitmapDC;

                // offset origin more because bitmap is just piece of the whole drawing
                //..,,	dc.OffsetViewportOrg(-rect.left, -rect.top);
                bitmapDC.OffsetViewportOrg(-rect.left, -rect.top);
                pOldBitmap = bitmapDC.SelectObject(&bitmap);

                // might as well clip to the same rectangle
                bitmapDC.IntersectClipRect(client);
            }
        }
    }

    // paint background
    CBrush brush;
    if (!brush.CreateStockObject(WHITE_BRUSH))
        return;

    pDrawDC->FillRect(client, &brush);

    pDoc->Draw(pDrawDC, this);

    if (pDrawDC != pDC) {
        pDC->SetViewportOrg(0, 0);
        pDC->SetMapMode(MM_TEXT);
        bitmapDC.SetViewportOrg(0, 0);
        bitmapDC.SetMapMode(MM_TEXT);
        pDC->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), pDrawDC, 0, 0, SRCCOPY);
        bitmapDC.SelectObject(pOldBitmap);
    }
}

void CDrawView::Remove(CDrawObj* pObj)
{
    auto iter = std::find(std::begin(m_selection), std::end(m_selection), pObj);
    if (iter != std::end(m_selection))
        m_selection.erase(iter);
}

void CDrawView::OnInitialUpdate()
{
    CSize size = GetDocument()->GetSize();
    size.cx = MulDiv(size.cx, m_zoomNum.cx, 100);
    size.cy = MulDiv(size.cy, m_zoomNum.cy, 100);
    SetScrollSizes(MM_TEXT, size);
}

void CDrawView::SetPageSize()
{
    OnInitialUpdate();
    GetDocument()->UpdateAllViews(nullptr, HINT_UPDATE_WINDOW, nullptr);
}

/////////////////////////////////////////////////////////////////////////////
// OLE Client support and commands

BOOL CDrawView::IsSelected(const CObject* pDocItem) const
{
    return std::end(m_selection) != std::find(std::begin(m_selection), std::end(m_selection), const_cast<CObject *>(pDocItem));
}

// The following command handler provides the standard keyboard
//  user interface to cancel an in-place editing session.
void CDrawView::OnCancelEdit()
{
    // deactivate any in-place active item on this view!
    COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
    if (pActiveItem != nullptr) {
        // if we found one, deactivate it
        pActiveItem->Close();
    }
    ASSERT(GetDocument()->GetInPlaceActiveItem(this) == nullptr);

    // escape also brings us back into select mode
    ReleaseCapture();

    CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
    if (pTool != nullptr)
        CDrawTool::OnCancel();

    CDrawTool::c_drawShape = DrawShape::select;
}

void CDrawView::OnSetFocus(CWnd* pOldWnd)
{
    COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
    if (pActiveItem != nullptr &&
        pActiveItem->GetItemState() == COleClientItem::activeUIState) {
        // need to set focus to this item if it is in the same view
        CWnd* pWnd = pActiveItem->GetInPlaceWindow();
        if (pWnd != nullptr) {
            pWnd->SetFocus();
            return;
        }
    }

    CScrollView::OnSetFocus(pOldWnd);
}

void CDrawView::ClientToDoc(CPoint& point)
{
    CClientDC dc(this);
    OnPrepareDC(&dc, nullptr);
    dc.DPtoLP(&point);
}

void CDrawView::ClientToDoc(CRect& rect)
{
    CClientDC dc(this);
    OnPrepareDC(&dc, nullptr);
    dc.DPtoLP(rect);
    ASSERT(rect.left <= rect.right);
    ASSERT(rect.bottom <= rect.top);
}

void CDrawView::DocToClient(CPoint& point)
{
    CClientDC dc(this);
    OnPrepareDC(&dc, nullptr);
    dc.LPtoDP(&point);
}

void CDrawView::DocToClient(CRect& rect)
{
    CClientDC dc(this);
    OnPrepareDC(&dc, nullptr);
    dc.LPtoDP(rect);
    rect.NormalizeRect();
}

void CDrawView::Select(CDrawObj* pObj, const bool bAdd)
{
    if (!bAdd) {
        OnUpdate(nullptr, HINT_UPDATE_SELECTION, nullptr);
        m_selection.clear();
    }

    if (pObj == nullptr || IsSelected(pObj))
        return;

    if (!m_selection.empty()) {
        auto pAdd = dynamic_cast<CDrawAdd *>(pObj);
        if (pAdd && pAdd->fizpage) return;

        auto pPage = dynamic_cast<CDrawPage *>(pObj);
        if (bAdd && pPage) {
            const auto& pages = GetDocument()->m_pages;
            const int iHitPos = GetDocument()->GetIPage(pPage);
            for (int i = iHitPos - 1; i >= 0; --i)
                if (IsSelected(pages[i]))
                    for (int j = i + 1; j < iHitPos; ++j)
                        Select((CDrawObj *)pages[j], true);
            const auto pc = (int)pages.size();
            for (int i = iHitPos + 1; i < pc; ++i)
                if (IsSelected(pages[i]))
                    for (int j = iHitPos + 1; j < i; ++j)
                        Select((CDrawObj *)pages[j], true);
        }
    }

    m_selection.push_back(pObj);
    InvalObj(pObj);
    GetDocument()->UpdateAllViews(this, HINT_UPDATE_GRID, (CDrawAdd *)pObj);
}

// rect is in device coordinates
void CDrawView::SelectWithinRect(CRect rect, const bool bAdd)
{
    if (!bAdd)
        Select(nullptr);

    ClientToDoc(rect);

    for (const auto& pObj : GetDocument()->m_objects)
        if (pObj->Intersects(rect))
            Select(pObj, true);
}

void CDrawView::OpenSelected()
{
    if (m_selection.size() == 1)
        m_selection.front()->OnOpen(this);
}

void CDrawView::Deselect(CDrawObj* pObj)
{
    auto iter = std::find(begin(m_selection), end(m_selection), pObj);
    if (iter != end(m_selection)) {
        InvalObj(pObj);
        m_selection.erase(iter);
    }
}

void CDrawView::CloneSelection()
{
    for (auto& pObj : m_selection)
        if (!dynamic_cast<CDrawAdd *>(pObj)) { // og³oszeñ nie klonujemy
            pObj = pObj->Clone(pObj->m_pDocument);
            InvalObj(pObj);
        }
}

void CDrawView::UpdateActiveItem()
{
    COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
    if (pActiveItem != nullptr &&
        pActiveItem->GetItemState() == COleClientItem::activeUIState) {
        // this will update the item rectangles by calling
        //  OnGetPosRect & OnGetClipRect.
        pActiveItem->SetItemRects();
    }
}

/////////////////////////////////////////////////////////////////////////////
// CDrawView message handlers

void CDrawView::OnLButtonDown(const UINT nFlags, const CPoint point)
{
    if (!m_bActive)
        return;
    CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
    if (pTool != nullptr)
        pTool->OnLButtonDown(this, nFlags, point);
}

void CDrawView::OnLButtonUp(const UINT nFlags, const CPoint point)
{
    if (!m_bActive)
        return;
    CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
    if (pTool != nullptr)
        pTool->OnLButtonUp(this, nFlags, point);
}

void CDrawView::OnMouseMove(const UINT nFlags, const CPoint point)
{
    if (theApp.unQueing && !m_bActive) {
        CDrawTool::c_drawShape = DrawShape::select;
        m_selection.clear();
        m_selection.push_back(CQueView::selected_add);
        SetCapture();
        m_bActive = TRUE;
    }
    if (!m_bActive)
        return;
    CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
    if (pTool != nullptr)
        pTool->OnMouseMove(this, nFlags, point);
}

BOOL CDrawView::OnMouseWheel(const UINT nFlags, const short zDelta, const CPoint pt)
{
    const auto delta = zDelta / WHEEL_DELTA;
    const float fFactor = (15 + (float)delta) / 15;
    if ((nFlags & MK_CONTROL) > 0) {
        CDrawOpis *pOpi;
        if (m_selection.size() == 1 && (pOpi = dynamic_cast<CDrawOpis *>(m_selection.front()))) {
            if ((0.1 < pOpi->m_Scale || fFactor < 1.0) && (pOpi->m_Scale < 10 || fFactor > 1.0))
                pOpi->m_Scale /= fFactor;
            pOpi->SetDirty();
            pOpi->Invalidate();
        } else {
            const CSize zoomNum((int)(m_zoomNum.cx * fFactor), (int)(m_zoomNum.cy * fFactor));
            // regulacja jest dopuszczalna w zakresie rgiZoomFactor[0]..rgiZoomFactor[size-1]
            if ((zoomNum.cx >= 60 || fFactor > 1.0) && (zoomNum.cx <= 1000 || fFactor < 1.0))
                SetZoomFactor(zoomNum, m_zoomDenom);
        }
    } else {
        CPoint p = GetScrollPosition();
        p.y -= delta * CLIENT_SCALE * 2;
        ScrollToPosition(p);
    }

    return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}

void CDrawView::OnLButtonDblClk(const UINT nFlags, const CPoint point)
{
    if (!m_bActive)
        return;
    CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
    if (pTool != nullptr)
        pTool->OnLButtonDblClk((CDrawView *)this, nFlags, point);
}

void CDrawView::OnDestroy()
{
    CScrollView::OnDestroy();

    // deactivate the inplace active item on this view
    COleClientItem *pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
    if (pActiveItem != nullptr && pActiveItem->GetActiveView() == this) {
        pActiveItem->Deactivate();
        ASSERT(GetDocument()->GetInPlaceActiveItem(this) == nullptr);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
// GUZIKI I MENU + ICH ZMIANA
void CDrawView::OnViewLupkaPlus()
{
    CSize zoomDenom;
    CSize zoomNum;

    zoomDenom.cx = zoomDenom.cy = 0;

    for (const auto& fact : rgiZoomFactor)
        if (fact > m_zoomNum.cx) {
            zoomNum.cx = fact;
            zoomNum.cy = fact;
            SetZoomFactor(zoomNum, zoomDenom);
            break;
        }
}

void CDrawView::OnViewLupkaMinus()
{
    CSize zoomDenom;
    CSize zoomNum;

    zoomDenom.cx = zoomDenom.cy = 0;

    for (auto it = rgiZoomFactor.rbegin(); it != rgiZoomFactor.rend(); ++it)
        if (*it < m_zoomNum.cx) {
            zoomNum.cx = *it;
            zoomNum.cy = *it;
            SetZoomFactor(zoomNum, zoomDenom);
            break;
        }
}

void CDrawView::OnDrawTool(const UINT tool)
{
    auto selected_tool = static_cast<DrawShape>(tool);
    if (CDrawTool::c_drawShape == selected_tool)
        CDrawTool::c_drawShape = DrawShape::select;
    else
        CDrawTool::c_drawShape = selected_tool;
}

void CDrawView::OnUpdateDrawTool(CCmdUI* pCmdUI)
{
    if (!OnDisableMenuInt(pCmdUI))
        pCmdUI->SetRadio(CDrawTool::c_drawShape == static_cast<DrawShape>(pCmdUI->m_nID));
}

void CDrawView::OnUpdateDrawAdd(CCmdUI* pCmdUI)
{
    if (GetDocument()->iDocType == DocType::makieta_lib)
        pCmdUI->Enable(FALSE);
    else
        OnUpdateDrawTool(pCmdUI);
}

void CDrawView::OnUpdateSingleSelect(CCmdUI* pCmdUI)
{
    if (!OnDisableMenuInt(pCmdUI))
        pCmdUI->Enable(m_selection.size() == 1);
}

void CDrawView::OnEditSelectAll()
{
    for (const auto& pObj : GetDocument()->m_objects)
        Select(pObj, true);
}

void CDrawView::OnUpdateEditSelectAll(CCmdUI* pCmdUI)
{
    if (!OnDisableMenuInt(pCmdUI)) pCmdUI->Enable(!GetDocument()->m_objects.empty());
}

void CDrawView::OnEditClear()
{
    bool to_tell{false};
    // update all the views before the selection goes away
    OnUpdate(nullptr, HINT_UPDATE_SELECTION, nullptr);

    // now remove the selection from the document
    for (auto& pObj : m_selection) {
        if (auto pAdd = dynamic_cast<CDrawAdd *>(pObj)) {
            if (pAdd->flags.derived) continue;
            if (pAdd->m_add_xx > 0) {
                AfxMessageBox(_T("Wska¿ emisje przeznaczone do usuniêcia"), MB_ICONINFORMATION | MB_OK);
                continue;
            }
            to_tell = true;
        } else {
            if (auto pPage = dynamic_cast<CDrawPage *>(pObj)) {
                if (pPage->m_dervlvl == DervType::adds && !pPage->m_adds.empty())
                    continue;
                if (std::any_of(pPage->m_adds.cbegin(), pPage->m_adds.cend(), [](auto pAdd) noexcept { return pAdd->flags.derived > 0; }))
                    continue;
                to_tell = true;
            }
        }
        
        auto tmp = pObj;
        pObj = nullptr;
        GetDocument()->Remove(tmp); // it removes item from m_selection, so without a copy it would clober iterator 
        delete tmp;
    }
    m_selection.clear();
    if (to_tell)
        GetDocument()->UpdateAllViews(this, HINT_DELETE_FROM_GRID, nullptr); //zeby grid zareagowal
}

void CDrawView::OnUpdateAnySelect(CCmdUI* pCmdUI)
{
    if (!OnDisableMenuInt(pCmdUI)) pCmdUI->Enable(!m_selection.empty());
}

void CDrawView::OnSize(const UINT nType, const int cx, const int cy)
{
    CScrollView::OnSize(nType, cx, cy);
    UpdateActiveItem();
}

void CDrawView::OnViewSpaceLocks()
{
    ((CMainFrame *)AfxGetMainWnd())->show_spacelocks = !(((CMainFrame *)AfxGetMainWnd())->show_spacelocks);
    Invalidate(FALSE);
}

void CDrawView::OnUpdateViewSpaceLocks(CCmdUI* pCmdUI)
{
    if (!OnDisableMenuInt(pCmdUI)) pCmdUI->SetCheck(((CMainFrame *)AfxGetMainWnd())->show_spacelocks);
}

void CDrawView::OnImportOpcje()
{
    theApp.includeKratka = !(theApp.includeKratka);
    Invalidate(FALSE);
}

void CDrawView::OnUpdateImportOpcje(CCmdUI* pCmdUI)
{
    if (!OnDisableMenuInt(pCmdUI)) {
        if (GetDocument()->iDocType == DocType::makieta_lib) 
            pCmdUI->Enable(FALSE);
        pCmdUI->SetCheck(theApp.includeKratka);
    }
}

BOOL CDrawView::OnEraseBkgnd(CDC* /*unused*/)
{
    return TRUE;
}

void CDrawView::OnObjectMoveForward()
{
    CDrawDoc* pDoc = GetDocument();
    CDrawObj* pObj = m_selection.front();
    auto pObjects = &pDoc->m_objects;

    auto pos = std::find(pObjects->begin(), pObjects->end(), pObj);
    if (pos == pObjects->end())
        return; // tzn ze to strona

    if (std::next(pos) != pObjects->end()) {
        std::swap(pDoc->m_objects[pos - pObjects->begin()], pDoc->m_objects[pos - pObjects->begin() + 1]);
        InvalObj(pObj);

        if (dynamic_cast<CDrawAdd *>(pObj))
            pDoc->UpdateAllViews(this, HINT_EDIT_PASTE, pObj); // dla gridu by zmienic
    }
}

void CDrawView::OnObjectMoveBack()
{
    CDrawDoc* pDoc = GetDocument();
    CDrawObj* pObj = m_selection.front();
    auto pObjects = &pDoc->m_objects;

    auto pos = std::find(pObjects->begin(), pObjects->end(), pObj);
    if (pos == pObjects->end())
        return; // tzn ze to strona

    if (pos != pObjects->begin()) {
        std::swap(pDoc->m_objects[pos - pObjects->begin() - 1], pDoc->m_objects[pos - pObjects->begin()]);
        InvalObj(pObj);

        if (dynamic_cast<CDrawAdd *>(pObj))
            pDoc->UpdateAllViews(this, HINT_EDIT_PASTE, pObj); // dla gridu by zmienic
    }
}

void CDrawView::OnObjectMoveToFront()
{
    CDrawDoc* pDoc = GetDocument();
    CDrawObj* pObj = m_selection.front();
    auto pObjects = &pDoc->m_objects;

    auto pos = std::find(pObjects->begin(), pObjects->end(), pObj);
    if (pos == pObjects->end())
        return; // tzn ze to strona

    std::swap(pDoc->m_objects[pos - pObjects->begin()], pDoc->m_objects[pDoc->m_objects.size() - 1]);
    InvalObj(pObj);

    if (dynamic_cast<CDrawAdd *>(pObj))
        pDoc->UpdateAllViews(this, HINT_EDIT_PASTE, pObj); // dla gridu by zmienic
}

void CDrawView::OnObjectMoveToBack()
{
    CDrawDoc* pDoc = GetDocument();
    CDrawObj* pObj = m_selection.front();
    auto pObjects = &pDoc->m_objects;

    auto pos = std::find(pObjects->begin(), pObjects->end(), pObj);
    if (pos == pObjects->end())
        return; // tzn ze to strona

    std::swap(pDoc->m_objects[0], pDoc->m_objects[pos - pObjects->begin()]);
    InvalObj(pObj);

    if (dynamic_cast<CDrawAdd *>(pObj))
        pDoc->UpdateAllViews(this, HINT_EDIT_PASTE, pObj); // dla gridu by zmienic
}

/////////////////////////////////////////////////////////////
////////// COPY PASTE

void CDrawView::OnEditCopy()
{
    ASSERT_VALID(this);
    ASSERT(m_cfDraw != NULL);

    // Create a shared file and associate a CArchive with it
    CSharedFile file;
    CArchive ar(&file, CArchive::store);

    // Serialize selected objects to the archive
    ar.WriteCount(m_selection.size());
    for (const auto& pObj : m_selection)
        ar << pObj;
    ar.Close();

    auto pDataSource = new COleDataSource;
    // put on local format instead of or in addation to
    pDataSource->CacheGlobalData(m_cfDraw, file.Detach());
    pDataSource->SetClipboard();
}

void CDrawView::Paste(COleDataObject& dataObject)
{
    // get file refering to clipboard data
    std::unique_ptr<CFile> pFile(dataObject.GetFileData(m_cfDraw));
    if (pFile == nullptr)
        return;

    // connect the file to the archive
    CArchive ar(pFile.get(), CArchive::load);
    ar.m_pDocument = GetDocument(); // set back-pointer in archive

    CDrawObj* newData;
    auto nNewCount = ar.ReadCount();
    while (nNewCount--) {
        ar >> newData;
        m_selection.push_back(newData);
    }
    ar.Close();
}

void CDrawView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
    if (!OnDisableMenuInt(pCmdUI)) pCmdUI->Enable(!m_selection.empty());
}

void CDrawView::OnEditCut()
{
    OnEditCopy();
    OnEditClear();
}

void CDrawView::OnUpdateEditCut(CCmdUI* pCmdUI)
{
    if (!OnDisableMenuInt(pCmdUI)) pCmdUI->Enable(!m_selection.empty());
}

void CDrawView::OnEditPaste()
{
    COleDataObject dataObject;
    dataObject.AttachClipboard();
    if (!dataObject.IsDataAvailable(m_cfDraw))
        return;

    // invalidate current selection since it will be deselected
    OnUpdate(nullptr, HINT_UPDATE_SELECTION, nullptr);
    m_selection.clear();
    // deserialize objects from clipboard into m_selection
    Paste(dataObject);
    // add all items : m_selection to document
    bool newAds = false;
    for (const auto& pObj : m_selection) {
        if (dynamic_cast<CDrawPage *>(pObj))
            GetDocument()->AddPage(reinterpret_cast<CDrawPage *>(pObj));
        else {
            pObj->m_position += CSize(15 * vscale, -15 * vscale);
            if (dynamic_cast<CDrawOpis *>(pObj)) {
                auto pOpi = reinterpret_cast<CDrawOpis *>(pObj);
                GetDocument()->Add(pOpi);
            } else { // must be CDrawAdd
                newAds = true;
                auto pAdd = reinterpret_cast<CDrawAdd *>(pObj);
                pAdd->nreps = -1L;
                pAdd->m_add_xx = 0;
                pAdd->flags.locked = 0;
                GetDocument()->Add(pAdd);
            }
        }
        pObj->Invalidate();
    }

    // invalidate new pasted stuff
    GetDocument()->UpdateAllViews(this);
    if (newAds)
        GetDocument()->UpdateAllViews(this, HINT_EDIT_PASTE, nullptr); // dla gridu by dodac
}

void CDrawView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
    // determine if private or standard OLE formats are on the clipboard
    COleDataObject dataObject;
    BOOL bEnable = dataObject.AttachClipboard() &&
        (dataObject.IsDataAvailable(m_cfDraw) ||
        COleClientItem::CanCreateFromData(&dataObject));

    // enable command based on availability
    if (!OnDisableMenuInt(pCmdUI)) pCmdUI->Enable(bEnable);
}

// CDrawView printing methods
LPDEVMODE CDrawView::SetLandscape()
{
    // nazwa drukarki domyœlnej
    DWORD lSize = n_size;
    if (!::GetDefaultPrinter(theApp.bigBuf, &lSize))
        return nullptr;
    // otwórz
    HANDLE hPrinter;
    if (!::OpenPrinter(theApp.bigBuf, &hPrinter, nullptr))
        return nullptr;
    // pobierz ustawienia
    const DWORD dwNeeded = DocumentProperties(this->m_hWnd, hPrinter, theApp.bigBuf, nullptr, nullptr, 0);
    auto dm = (PDEVMODE)LocalAlloc(LMEM_FIXED, dwNeeded);
    // zmieñ
    DocumentProperties(this->m_hWnd, hPrinter, theApp.bigBuf, dm, nullptr, DM_OUT_BUFFER);
    if (dm && dm->dmFields & DM_ORIENTATION)
        dm->dmOrientation = DMORIENT_LANDSCAPE;
    DocumentProperties(this->m_hWnd, hPrinter, theApp.bigBuf, dm, dm, DM_IN_BUFFER | DM_OUT_BUFFER);
    // zamknij
    ::ClosePrinter(hPrinter);
    return dm;
}

BOOL CDrawView::OnPreparePrinting(CPrintInfo* pInfo)
{
    // default preparation
    const int format = theApp.colsPerPage == 1 ? A4 : A3;
    switch (m_pagesPrinted) {
        case PrintFormat::page:
            pInfo->SetMaxPage((UINT)GetDocument()->m_pages.size());
            break;
        case PrintFormat::doc:
            pInfo->SetMaxPage((int)ceil((float)GetDocument()->m_pages.size() / (float)format));
            break;
        case PrintFormat::all:
            pInfo->SetMaxPage(1);
            break;
    }

    BOOL doPrint = DoPreparePrinting(pInfo);
    if (!doPrint)
        theApp.SetScale(CLIENT_SCALE);
    return doPrint;
}

void CDrawView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    theApp.SetScale(1);
    GetDocument()->ComputeCanvasSize();
}

void CDrawView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    theApp.SetScale(CLIENT_SCALE);
    GetDocument()->UpdateAllViews(nullptr);
};

void CDrawView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
    CDrawDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    int pc, wspol = theApp.colsPerPage;
    if (wspol == 2 || m_pagesPrinted != PrintFormat::page)
        pDoc->PrintInfo(pDC, pInfo->GetMaxPage(), wspol);
    CDrawPage* pPage;
    switch (m_pagesPrinted) {
        case PrintFormat::page:
            pPage = (GetDocument()->m_pages[pInfo->m_nCurPage - 1]);
            pDC->SetMapMode(MM_ANISOTROPIC);
            pDC->SetViewportOrg(0, 0);
            pDC->SetViewportExt(pDC->GetDeviceCaps(HORZRES), pDC->GetDeviceCaps(VERTRES));
            pDC->SetWindowOrg(pPage->m_position.left / CLIENT_SCALE - 5 * vscale, pPage->m_position.top / CLIENT_SCALE - 18 * vscale - 2 * (pInfo->m_nCurPage / (wspol * 50 + 1)) * pmoduly - PRINT_VOFFSET);
            pDC->SetWindowExt((pmodulx + 2 * vscale) * pszpalt_x, -(pmoduly + 4 * vscale) * pszpalt_y);
            pDoc->PrintPage(pDC, pPage);
            break;
        case PrintFormat::doc:
            pDoc->Print(pDC);
            break;
        case PrintFormat::all:
            pc = (int)pDoc->m_pages.size();
            if ((theApp.colsPerPage == 1 && pc > A4) || (theApp.colsPerPage == 2 && pc > A3)) {
                pDC->SetMapMode(MM_ANISOTROPIC);
                pDC->SetViewportOrg(0, 0);
                pDC->SetViewportExt(pDC->GetDeviceCaps(HORZRES), pDC->GetDeviceCaps(VERTRES));
                pDC->SetWindowOrg(0, -2 * PRINT_VOFFSET);
                pDC->SetWindowExt(((int)(5.5 * pDoc->m_pagerow_size) + 1) * pmodulx, -(int)((pDoc->m_pages.size() / pDoc->m_pagerow_size + 1.7) * 8 * pmoduly));
            }
            pDoc->Print(pDC);
            break;
    }
}

void CDrawView::OnFilePrint()
{
    if (m_pagesPrinted == PrintFormat::null) m_pagesPrinted = PrintFormat::doc;
    CScrollView::OnFilePrint();
    GetDocument()->ComputeCanvasSize();
    m_pagesPrinted = PrintFormat::null;
}

void CDrawView::OnFilePrint1p()
{
    m_pagesPrinted = PrintFormat::page;
    CScrollView::OnFilePrint();
    GetDocument()->ComputeCanvasSize();
    m_pagesPrinted = PrintFormat::null;
}

void CDrawView::OnFilePrintAll()
{
    m_pagesPrinted = PrintFormat::all;
    CScrollView::OnFilePrint();
    GetDocument()->ComputeCanvasSize();
    m_pagesPrinted = PrintFormat::null;
}

void CDrawView::OnFilePrintPreview1p()
{
    m_pagesPrinted = PrintFormat::page;
    AFXPrintPreview(this);
}

void CDrawView::OnFilePrintPreviewAll()
{
    m_pagesPrinted = PrintFormat::all;
    AFXPrintPreview(this);
}

void CDrawView::OnFilePrintPreview()
{
    m_pagesPrinted = PrintFormat::doc;
    AFXPrintPreview(this);
}

void CDrawView::OnEditProperties()
{
    if (m_selection.size() == 1 && CDrawTool::c_drawShape == DrawShape::select) {
        CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
        if (pTool)
            pTool->OnLButtonDblClk(this, 0, CPoint(0, 0));

        ASSERT(pTool != nullptr);
    }
}

void CDrawView::OnUpdateEditProperties(CCmdUI* pCmdUI)
{
    if (!OnDisableMenuInt(pCmdUI))
        pCmdUI->Enable(m_selection.size() == 1 && CDrawTool::c_drawShape == DrawShape::select);
}

void CDrawView::OnChooseFont(CFont& m_font, const bool IsPageFont)
{
    LOGFONT lf;
    if (m_font.m_hObject != nullptr)
        m_font.GetObject(sizeof(LOGFONT), &lf);
    else
        ::GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &lf);

    CFontDialog dlg(&lf, CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT);
    if (dlg.DoModal() == IDOK) {
        theApp.WriteProfileString(_T("Settings"), IsPageFont ? _T("PageFontFace") : _T("AddFontFace"), lf.lfFaceName);
        theApp.WriteProfileInt(_T("Settings"), IsPageFont ? _T("PageFontSize") : _T("AddFontSize"), lf.lfHeight);
        m_font.DeleteObject();
        if (m_font.CreateFontIndirect(&lf))
            SetFont(&m_font);
        GetDocument()->UpdateAllViews(nullptr);
    }
}

void CDrawView::OnChoosePageFont()
{
    OnChooseFont(GetDocument()->m_pagefont, true);
}

void CDrawView::OnChooseAddFont()
{
    OnChooseFont(GetDocument()->m_addfont, false);
}

/////////////////////////////////////////////////////////////////////////////
// CDrawView diagnostics
#ifdef _DEBUG
void CDrawView::AssertValid() const
{
    CScrollView::AssertValid();
}

void CDrawView::Dump(CDumpContext &dc) const
{
    CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
BOOL CDrawView::SetZoomFactor(const CSize zoomNum, const CSize zoomDenom)
{
    if (zoomDenom != m_zoomDenom || zoomNum != m_zoomNum) {
        // sync to new zoom factor
        if (theApp.m_initZoom = zoomNum.cx) m_zoomNum = zoomNum;
        if (zoomDenom.cx) m_zoomDenom = zoomDenom;

        // resync to new sizes
        Invalidate(TRUE);
        SetPageSize();
        return TRUE;
    }
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Zooming user interface

void CDrawView::OnViewZoomN(const UINT nID)
{
    ASSERT(nID >= ID_VIEW_ZOOM100 && nID <= ID_VIEW_ZOOM500);

    CSize zoomDenom;
    CSize zoomNum;

    zoomDenom.cx = zoomDenom.cy = 0;

    // get zoom factor numerator and set it
    ASSERT(nID - ID_VIEW_ZOOM100 < rgiZoomFactor.size());
    int iZoomFactor = rgiZoomFactor[nID - ID_VIEW_ZOOM100];
    zoomNum.cx = iZoomFactor;
    zoomNum.cy = iZoomFactor;
    // change the zoom factor to new zoom factor
    SetZoomFactor(zoomNum, zoomDenom);
}

void CDrawView::OnUpdateViewZoomN(CCmdUI* pCmdUI)
{
    if (pCmdUI->m_nID != ID_VIEW_ZOOMCUSTOM) {
        const int iZoomFactor = rgiZoomFactor[pCmdUI->m_nID - ID_VIEW_ZOOM100];

        if (iZoomFactor == m_zoomNum.cx && iZoomFactor == m_zoomNum.cy) {
            if (!OnDisableMenuInt(pCmdUI)) pCmdUI->SetCheck(TRUE);
            return;
        }
    }

    if (!OnDisableMenuInt(pCmdUI)) pCmdUI->SetCheck(FALSE);
}

void CDrawView::OnViewZoomCustom()
{
    // prepare dialog data
    CZoomDlg dlg;
    dlg.m_zoomX = m_zoomNum.cx;

    if (dlg.DoModal() == IDOK) {
        CSize zoomNum(dlg.m_zoomX, dlg.m_zoomX);
        SetZoomFactor(zoomNum, m_zoomDenom);
    }
}

void CDrawView::OnDrawMakStrony()
{
    // na liscie m_selection jest dokladnie jedna strona - OnUpdateDrawMakStrony
    auto pDoc = GetDocument();
    // przemakietuj ogloszenia na zaznaczonej stronie
    pDoc->MakietujStrone(reinterpret_cast<CDrawPage *>(m_selection.front()));
    // ustaw z boku te, ktorych sie nie zmakietowalo automatycznie
    pDoc->AsideAdds();
    pDoc->UpdateAllViews(nullptr);
}

void CDrawView::OnUpdateDrawMakStrony(CCmdUI* pCmdUI)
{
    if (OnDisableMenuInt(pCmdUI)) return;
    if (GetDocument()->iDocType == DocType::makieta_lib || m_selection.size() != 1) {
        pCmdUI->Enable(FALSE);
        return;
    }

    auto pPage = dynamic_cast<CDrawPage *>(m_selection.front());
    pCmdUI->Enable(pPage && !pPage->niemakietuj);
}

void CDrawView::OnUpdateVuCkMakietowanie(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(drawErrorBoxes);
    OnDisableMenu(pCmdUI);
}

void CDrawView::OnViewAdddesc()
{
    /* vu : Otwiera dialog pozwalaj¹cy ustawiæ rodzaj opisu og³oszenia.
            S¹ dwa poziomy opisu - dolny i górny, dla nich obowi¹zuj¹
            kody zgodne z kodami radio buttonów					end vu */

    CAddDesc dlg(theApp.m_view_top, theApp.m_view_bottom);

    if (dlg.DoModal() != IDOK) return;

    if (dlg.m_top == dlg.m_bottom) dlg.m_bottom = TEXT_BRAK; // nie mog¹ byæ takie same
    theApp.m_view_bottom = dlg.m_bottom;
    theApp.m_view_top = dlg.m_top;
    GetDocument()->UpdateAllViews(nullptr);
}

void CDrawView::OnDrawOpcje() { GetDocument()->OnDrawOpcje(); }

void CDrawView::OnUpdateViewCzasobow(CCmdUI* pCmdUI)
{
    if (!OnDisableMenuInt(pCmdUI))
        pCmdUI->SetCheck(GetDocument()->swCZV == ToolbarMode::czas_obow);
}

void CDrawView::OnUpdateViewStudio(CCmdUI* pCmdUI)
{
    if (!OnDisableMenuInt(pCmdUI))
        pCmdUI->SetCheck(GetDocument()->swCZV == ToolbarMode::tryb_studia);
}

void CDrawView::OnViewCzasobow()
{
    const auto tmp = (uint8_t)GetDocument()->swCZV - 1;
    const auto newMode = (ToolbarMode)(tmp * tmp); // f(x) = (x-1)^2
    UpdateToolbar(newMode);
}

void CDrawView::OnChangeView()
{
    const auto newMode = (ToolbarMode)(((uint8_t)GetDocument()->swCZV + 1) % 3);
    UpdateToolbar(newMode);
}

void CDrawView::OnViewStudio()
{
    const auto newMode = (ToolbarMode)(2 - ((uint8_t)GetDocument()->swCZV & 2)); // f(x) = 2-(x&2)
    UpdateToolbar(newMode);
}

void CDrawView::UpdateToolbar(const ToolbarMode newMode)
{
    const auto prevMode = theApp.swCZV;
    theApp.swCZV = GetDocument()->swCZV = newMode;
    ((CMainFrame *)AfxGetMainWnd())->SetToolbarBitmap(prevMode, newMode);
    Invalidate(FALSE);
}

void CDrawView::OnAsideAdds()
{
    GetDocument()->OnAsideAdds();
}

void CDrawView::OnPrintEps()
{
    CheckPrintEps(TRUE);
}

void CDrawView::OnCheckEps()
{
    CheckPrintEps(FALSE);
}

void CALLBACK CDrawView::DelegateGenEPS(PTP_CALLBACK_INSTANCE /*unused*/, PVOID parameter, PTP_WORK work)
{
    auto pA = static_cast<PGENEPSARG>(parameter);
    CDrawPage* pPage = pA->pPage;
    if (pA->pDlg->m_bIsGen) {
        if (!pPage->GenEPS(pA))
            pA->pDlg->cancelGenEPS = TRUE;
    } else
        pPage->CheckSrcFile(pA);

    ::CloseThreadpoolWork(work);
    ::SetEvent(pA->hCompletedEvent);
}

void CDrawView::CheckPrintEps(const BOOL isprint)
{
    auto pDoc = GetDocument();
    if (pDoc->m_pages.empty()) return;
    const auto pc = (int)pDoc->m_pages.size();
    auto pPage = pDoc->m_pages[0];

    CPrnEpsDlg d;
    d.m_isprint = isprint;
    d.m_signall = 1;
    d.m_do = pPage->GetNrPaginy();
    d.m_od = pc == 1 ? d.m_do : pDoc->m_pages[1]->GetNrPaginy();
    d.m_streamed = (BOOL)theApp.isParalellGen;
    d.m_markfound = (BOOL)theApp.GetProfileInt(_T("GenEPS"), _T("autoMark"), 0);

    if (m_selection.size() == 1 && (pPage = dynamic_cast<CDrawPage *>(m_selection.front()))) {
        d.m_page = 2;
        d.m_subset = pPage->GetNrPaginy();
    } else
        d.m_page = 0;

    if (d.DoModal() == IDCANCEL) return;
    if (isprint && !pDoc->SaveModified()) return;
    if (pDoc->m_rozm.empty()) pDoc->IniRozm();
    theApp.isParalellGen = d.m_streamed ? 1 : 0;
    theApp.WriteProfileInt(_T("GenEPS"), _T("autoMark"), theApp.autoMark = d.m_markfound);

    BeginWaitCursor();
    theManODPNET.LoadMakietaDirs(pDoc->m_mak_xx);

    if (isprint) {
        CString err(' ', 1024);
        pDoc->sOpiServerUrl = CString(' ', 64);

        CManODPNETParms orapar {
            { CManDbType::DbTypeInt32, &pDoc->m_mak_xx },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &pDoc->sOpiServerUrl },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &err } };
        orapar.outParamsCount = 2;
        theManODPNET.EI("begin epstest.check_makieta_prod2(:mak_xx,:opi_url,:err); end;", orapar);

        if (err.GetLength() > 8 && IDNO == AfxMessageBox(CString("Stwierdzono niezgodnoœæ potrzebnych i dostarczonych materia³ów graficznych dla og³oszeñ:\n\n") + err + "\n\nCzy chcesz kontynuowaæ produkcjê kolumn?", MB_YESNO))
            return;

        if (theApp.sManamEpsName.IsEmpty())
            theManODPNET.GetManamEps();
    }

    PBYTE bigBufArr;
    HANDLE* waitEvents;
    PGENEPSARG threadArgs;
    WORD iCpuCnt, iThreadCnt = 0;
    bool streamed = d.m_streamed;
    CGenEpsInfoDlg* pDlg = CGenEpsInfoDlg::GetGenEpsInfoDlg(isprint);

    const CFlag wyborStron = d.GetChoosenPages(pDoc);
    if (streamed) {
        iCpuCnt = CGenEpsInfoDlg::GetCpuCnt();
        const int iIleStron = wyborStron.GetBitCnt(true);
        if (iIleStron < iCpuCnt)
            iCpuCnt = (WORD)iIleStron;

        if (iCpuCnt > 1 && d.m_format != CManFormat::PDF) {
            pDlg->SetChannelCount(iCpuCnt);
            waitEvents = (HANDLE *)LocalAlloc(LMEM_FIXED, iCpuCnt * sizeof(HANDLE));
            threadArgs = (PGENEPSARG)LocalAlloc(LMEM_FIXED, iCpuCnt * sizeof(GENEPSARG));
            bigBufArr  = (PBYTE)VirtualAlloc(nullptr, bigSize * iCpuCnt, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (!bigBufArr) {
                AfxMessageBox(_T("Zbyt ma³o pamiêci do uruchomienia potokowego"), MB_ICONSTOP);
                return;
            }

            for (WORD i = 0; i < iCpuCnt; ++i) {
                auto& ta = threadArgs[i];
                ta.iChannelId = i;
                ta.format = d.m_format;
                ta.bIsPreview = d.m_preview;
                ta.bSignAll = d.m_signall;
                ta.bDoKorekty = d.m_korekta;
                ta.pDlg = (CGenEpsInfoDlg *)pDlg;
                ta.hCompletedEvent = waitEvents[i] = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
                ta.cBigBuf = (TCHAR*)(bigBufArr + bigSize * i);
            }
        } else
            streamed = false;
    }

    pDoc->ovEPS = FALSE;
    const BOOL bInitOpiMode = theApp.isOpiMode;
    if (isprint && d.m_format == CManFormat::EPS) theApp.isOpiMode = FALSE; // Dla F_EPS nie u¿ywaj OPI

    GENEPSARG genEpsArg;
    if (!streamed) {
        genEpsArg.iChannelId = 0;
        genEpsArg.format = d.m_format;
        genEpsArg.bIsPreview = d.m_preview;
        genEpsArg.bSignAll = d.m_signall;
        genEpsArg.bDoKorekty = d.m_korekta;
        genEpsArg.cBigBuf = theApp.bigBuf;
        genEpsArg.pPage = pPage;
        genEpsArg.pDlg = (CGenEpsInfoDlg *)pDlg;
    }

    for (int i = 0; !pDlg->cancelGenEPS && i < pc; ++i)
        if (wyborStron[i]) {
            pPage = pDoc->m_pages[i];
            const auto dl = pPage->m_dervlvl;
            if (dl == DervType::fixd || dl == DervType::proh || (d.m_exclude_emptypages && pPage->m_adds.empty() && pPage->name.Find(_T("DROB")) == -1))
                continue;

            if (streamed) {                      // delegate
                WORD channId;
                if (iThreadCnt >= iCpuCnt)
                    channId = (WORD)::WaitForMultipleObjects(iCpuCnt, waitEvents, FALSE, INFINITE) - WAIT_OBJECT_0;
                else
                    channId = iThreadCnt++;

                auto& channelArg = threadArgs[channId];
                channelArg.pPage = pPage;
                PTP_WORK work = ::CreateThreadpoolWork(CDrawView::DelegateGenEPS, &channelArg, nullptr);
                ::SubmitThreadpoolWork(work);
            } else if (!isprint)                 // check
                pPage->CheckSrcFile(&genEpsArg);
            else {                               // generate
                const auto generator = d.m_format != CManFormat::PDF ? &CDrawPage::GenEPS : &CDrawPage::GenPDF;
                if (!(pPage->*generator)(&genEpsArg)) {
                    pDlg->cancelGenEPS = TRUE;
                    break;
                }
            }
        }

    if (streamed) {
        ::WaitForMultipleObjects(iThreadCnt, waitEvents, TRUE, INFINITE);
        for (WORD i = 0; i < iCpuCnt; ++i) {
            auto& arg = threadArgs[i];
            ::CloseHandle(arg.hCompletedEvent);
        }
        ::LocalFree(waitEvents);
        ::LocalFree(threadArgs);
        ::VirtualFree(bigBufArr, 0, MEM_RELEASE);
    }
    AfxGetMainWnd()->ActivateTopParent();
    EndWaitCursor();

    if (!isprint) {
        pDlg->ShowWindow(SW_HIDE);
        CString msg;
        for (int i = 0; i < pc; ++i)
            if (wyborStron[i])
                msg += pDoc->m_pages[i]->f5_errInfo;
        ::MessageBox(theApp.GetMainWnd()->m_hWnd, msg.IsEmpty() ? _T("Nie ma b³êdów") : msg, _T("Raport sprawdzenia EPS"), MB_OK);
        if (theApp.autoMark && theApp.activeDoc) pDoc->UpdateAllViews(nullptr);
    } else if (isprint && theApp.isOpiMode && !pDlg->cancelGenEPS) {
        pDlg->ShowWindow(SW_HIDE);
        ::MessageBox(nullptr, _T("Kolumny zosta³y wys³ane do serwera OPI"), APP_NAME, MB_OK);
    } else if (isprint && d.m_format == CManFormat::EPS)
        theApp.isOpiMode = bInitOpiMode;

    CGenEpsInfoDlg::ReleaseGenEpsInfoDlg(pDlg);

} // CheckPrintEps

void CDrawView::OnRButtonDown(const UINT nFlags, CPoint point)
{
    CScrollView::OnRButtonDown(nFlags, point);
    CPoint local{point};
    ClientToDoc(local);
    CDrawDoc* pDoc = GetDocument();
    CDrawPage* pPage = pDoc->PageAt(local);
    if (!pPage) return;
    if (theApp.isRDBMS && pDoc->iDocType == DocType::grzbiet_drukowany && !pDoc->isRO) {
        CMenu mmutczas;
        Select(pPage, false);
        ClientToScreen(&point);
        mmutczas.LoadMenu(IDR_MENUMUTCZAS);
        mmutczas.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
    } else
        pDoc->DerivePages(pPage);
}

BOOL CDrawView::ModifyMutczas(const int n)
{
    auto pPage = (CDrawPage *)m_selection.front();
    if (pPage) {
        if (pPage->m_mutczas == 1 && n == -1) return TRUE;
        pPage->m_mutczas += n;
        int pos = pPage->m_dervinfo.ReverseFind('a');
        CString cs;
        cs.Format(_T("%s %i)"), (LPCTSTR)pPage->m_dervinfo.Left(++pos), pPage->m_mutczas);
        pPage->m_dervinfo = cs;
        pPage->UpdateInfo();
        GetDocument()->SetModifiedFlag();
        return TRUE;
    }
    return FALSE;
}

BOOL CDrawView::OnCommand(WPARAM wParam, LPARAM lParam)
{
    if ((WORD)wParam == IDM_INCMUTCZAS) return ModifyMutczas(1);
    if ((WORD)wParam == IDM_DECMUTCZAS) return ModifyMutczas(-1);

    return CScrollView::OnCommand(wParam, lParam);
}

void CDrawView::OnIrfan() // single Add selected
{
    CString eps_name = dynamic_cast<CDrawAdd*>(m_selection.front())->EpsName(CManFormat::EPS, false);
    if (eps_name.Mid(1, 1) != _T(':')) {
        AfxMessageBox(_T("Preview niedostêpne. Brak materia³u"));
        return;
    }
    CString irfDir = theApp.GetProfileString(_T("GenEPS"), _T("EpsIrfan"), _T(""));
    if (irfDir.IsEmpty())
        AfxMessageBox(_T("Ustaw œcie¿kê do IrfanView"));
    else
        _wspawnl(_P_DETACH, (LPCTSTR)(irfDir + _T("\\i_view32")), (LPCTSTR)_T("i_view32"), (LPCTSTR)eps_name, NULL);
}

void CDrawView::OnPrevKolumnaDruk() // single Page selected
{
    CString sURL;
    auto pPage = dynamic_cast<CDrawPage *>(m_selection.front());
    const int iNrPorz = theApp.activeDoc->GetIPage(pPage);
    sURL.Format(_T("tyt=%s&mut=%s&kiedy=%s&nrstrony=%i&format=pdf&rozmiar=1"), (LPCTSTR)theApp.activeDoc->gazeta.Left(3), (LPCTSTR)theApp.activeDoc->gazeta.Mid(4, 2), (LPCTSTR)theApp.activeDoc->dayws, iNrPorz == 0 ? (int)theApp.activeDoc->m_pages.size() : iNrPorz);
    CDrawApp::OpenWebBrowser(4, sURL);
}

void CDrawView::OnPrevPdf() // single Add or Page selected
{
    auto pAdd = dynamic_cast<CDrawAdd *>(m_selection.front());
    if (!pAdd) {
        OnPrevKolumnaDruk();
        return;
    }
    if (pAdd->nreps < MIN_VALID_ADNO) {
        AfxMessageBox(_T("Brak prawid³owego numeru atexowego"));
        return;
    }

    CString sUrl;
    auto doc = GetDocument();
    if (pAdd->powtorka == 0 || pAdd->powtorka > CTime::GetCurrentTime()) { // nowe lub powtórka niearchiwalna
        int status = pAdd->nreps;
        sUrl = theManODPNET.GetHttpSource(GetDocument()->gazeta, GetDocument()->data, &status);

        if (!sUrl.IsEmpty()) {
            if (status < 0) { // dane z qfiltr_material lub z archiwum
                if (status < -1)
                    AfxMessageBox(_T("Ten materia³ nie zosta³ dopuszczony do druku przez EpsTest"));
                if (sUrl.Find(_T('?')) < 0)
                    sUrl.AppendFormat(_T("?date=%s&nreps=%li&name=%s"), (LPCTSTR)doc->dayws, pAdd->nreps, (LPCTSTR)(doc->gazeta.Left(3) + doc->gazeta.Mid(4, 2)));
            }
            CDrawApp::OpenWebBrowser(sUrl);
            return;
        } else {
            int d, m, r;
            _stscanf_s(doc->data, c_formatDaty, &d, &m, &r);
            CTime tEdycja(r, m, d, 0, 0, 0);
            if (tEdycja > CTime::GetCurrentTime()) {
                AfxMessageBox(doc->symWydawcy == _T('-') ? _T("Brak zsy³aj¹cego") : _T("Brak materia³u"));
                return;
            }
            sUrl.Format(IDS_ARCH_URL, (LPCTSTR)tEdycja.Format(c_ctimeDataWs), pAdd->nreps);
        }
    } else // powtorka jest pokazywana z katalogu docelowego
        sUrl.Format(IDS_ARCH_URL, (LPCTSTR)doc->dayws, pAdd->nreps);

    CDrawApp::OpenWebBrowser(9, sUrl);
}

void CDrawView::OnPrevDig()
{
    auto pAdd = dynamic_cast<CDrawAdd *>(m_selection.front());
    if (pAdd->nreps < MIN_VALID_ADNO) {
        AfxMessageBox(_T("Brak prawid³owego numeru atexowego"));
        return;
    }

    CString dataSepia = GetDocument()->data;
    auto buf = reinterpret_cast<char *>(theApp.bigBuf);
    dataSepia = dataSepia.Mid(6) + '-' + dataSepia.Mid(3, 2) + '-' + dataSepia.Mid(0, 2);
    CString sURL;
    sURL.Format(_T("adno=%li&kiedy=%s"), pAdd->nreps, (LPCTSTR)dataSepia);
    auto pFile = theApp.OpenURL(5, sURL);
    if (pFile) {
        const auto iXmlLen = pFile->Read(buf, bigSize);
        pFile->Close();
        if (iXmlLen == bigSize) {
            AfxMessageBox(_T("Plik z dodatkow¹ treœci¹ jest za du¿y"));
            return;
        }
    }

    const char *kon = nullptr, *pocz = strstr(buf, "<artykul");
    if (pocz) kon = 10 + strstr(pocz, "</artykul>");

    TCHAR tmpPath[_MAX_PATH];
    if (!pocz || !kon || ::GetTempPath(_MAX_PATH, tmpPath) == 0) {
        AfxMessageBox(_T("Brak dodatkowej treœci"), MB_ICONERROR);
        return;
    }

    CFile digContent;
    CString digCntFilename;
    for (int i = 0; i < 10; ++i) {
        digCntFilename.Format(_T("%sdigCnt%i.xmlw"), tmpPath, i);
        try {
            digContent.Open(digCntFilename, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive);
            if (digContent.m_hFile == INVALID_HANDLE_VALUE) continue;
            digContent.Write(pocz, (UINT)(kon - pocz));
            digContent.Close();
            break;
        } catch (CFileException *fe) {
            fe->Delete(); // ignore, try next name
        }
    }

    ::ShellExecute(::GetDesktopWindow(), _T("Edit"), digCntFilename, nullptr, nullptr, SW_SHOWNORMAL);

    if (GetLastError() > 0)
        AfxMessageBox(_T("Brak skojarzenia z .xmlw"), MB_ICONERROR);
}

void CDrawView::OnAtexSyg() // single Add selected
{
    auto pAdd = dynamic_cast<CDrawAdd *>(m_selection.front());
    if (pAdd->nreps < MIN_VALID_ADNO) {
        AfxMessageBox(_T("Brak prawid³owego numeru atexowego"));
        return;
    }

    CString sUrl;
    sUrl.Format(_T("t=%s&m=%s&k=%s&a=%li"), (LPCTSTR)GetDocument()->gazeta.Left(3), (LPCTSTR)(pAdd->flags.derived ? _T("RP") : GetDocument()->gazeta.Mid(4, 2)), (LPCTSTR)GetDocument()->data, pAdd->nreps);
    CDrawApp::OpenWebBrowser(7, sUrl);
}

void CDrawView::OnPrevKor()
{
    if (this->m_selection.size() == 1) {
        auto pPage = dynamic_cast<CDrawPage *>(m_selection.front());
        if (pPage) {
            CString num, fname; /*0925dlowa001-1ake.pdf*/
            CDrawDoc* pDoc = GetDocument();
            int lnr_porz = pDoc->GetIPage(pPage);
            num.Format(_T("%03i"), lnr_porz ? lnr_porz : (int)pDoc->m_pages.size());
            GENEPSARG genEpsArg;
            genEpsArg.cBigBuf = theApp.bigBuf;
            genEpsArg.format = CManFormat::PDF;
            genEpsArg.bDoKorekty = true;
            pPage->GetDestName(&genEpsArg, num, fname);
            lnr_porz = fname.ReverseFind('\\');
            if (lnr_porz > 0)
                fname.Delete(0, lnr_porz);
            fname = theApp.GetProfileString(_T("GenEPS"), _T("KorektaDobre"), _T("")) + pDoc->daydir + pDoc->gazeta.Left(3) + pDoc->gazeta.Mid(4, 2) + fname;

            CDrawApp::OpenWebBrowser(fname);
        }
    }
}
