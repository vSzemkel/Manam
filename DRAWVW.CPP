
#include "stdafx.h"
#include "manam.h"
#include "drawdoc.h"
#include "drawpage.h"
#include "drawadd.h"
#include "drawopis.h"
#include "drawvw.h"
#include "gridfrm.h"
#include "queview.h"
#include "drawtool.h"
#include "koldlg.h"
#include "mainfrm.h"
#include "genepsinfodlg.h"
#include "ManPDF.h"

extern CDrawApp NEAR theApp ;
extern BOOL disableMenu ;
extern BOOL drawErrorBoxes ;
extern BOOL cancelGenEPS; 

// private clipboard format (list of Draw objects)
CLIPFORMAT CDrawView::m_cfDraw = (CLIPFORMAT)::RegisterClipboardFormat(_T("Automatyczne makietowanie"));
/////////////////////////////////////////////////////////////////////////////

static std::array<int, 13> rgiZoomFactor = { 60, 70, 85, 100, 125, 150, 200, 250, 300, 400, 500, 750, 1000 };

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
	ON_COMMAND(ID_CANCEL_EDIT, OnCancelEdit)
	ON_COMMAND_RANGE(ID_DRAW_ADD, ID_DRAW_SPACELOCKED, OnDrawTool)
	ON_UPDATE_COMMAND_UI(ID_DRAW_ADD, OnUpdateDrawAdd)
	ON_UPDATE_COMMAND_UI_RANGE(ID_DRAW_RED, ID_DRAW_SPACELOCKED, OnUpdateDrawTool)
	ON_UPDATE_COMMAND_UI_RANGE(ID_OBJECT_MOVETOFRONT, ID_OBJECT_MOVEBACK, OnUpdateSingleSelect)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateAnySelect)
	ON_COMMAND(ID_VIEW_SPACELOCKS, OnViewSpaceLocks)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SPACELOCKS, OnUpdateViewSpaceLocks)
	ON_COMMAND(ID_ADD_OPCJE, OnImportOpcje)
	ON_UPDATE_COMMAND_UI(ID_ADD_OPCJE, OnUpdateImportOpcje)
	ON_COMMAND(ID_OBJECT_MOVEBACK, OnObjectMoveBack)
	ON_COMMAND(ID_OBJECT_MOVEFORWARD, OnObjectMoveForward)
	ON_COMMAND(ID_OBJECT_MOVETOBACK, OnObjectMoveToBack)
	ON_COMMAND(ID_OBJECT_MOVETOFRONT, OnObjectMoveToFront)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_PROPERTIES, OnEditProperties)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PROPERTIES, OnUpdateEditProperties)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
	ON_COMMAND(ID_VIEW_ZOOMCUSTOM, OnViewZoomCustom)
	ON_COMMAND_RANGE(ID_VIEW_ZOOM100, ID_VIEW_ZOOM750, OnViewZoomN)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_ZOOM100, ID_VIEW_ZOOMCUSTOM, OnUpdateViewZoomN)
	ON_COMMAND(ID_VIEW_LUPKA_PLUS, OnViewLupkaPlus)
	ON_COMMAND(ID_VIEW_LUPKA_MINUS, OnViewLupkaMinus)
	ON_COMMAND(ID_VU_MAK_STRONY, OnDrawMakStrony)
	ON_UPDATE_COMMAND_UI(ID_VU_MAK_STRONY, OnUpdateDrawMakStrony)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnDisableMenu)	//OnDisableMenu - wygaszanie kontrolek menu i toolbaru
	ON_UPDATE_COMMAND_UI(ID_FILE_DBOPEN, OnDisableMenuRDBMS)
	ON_UPDATE_COMMAND_UI(ID_ADD_IMPORTMINUS, OnDisableMenuLIB)
	ON_UPDATE_COMMAND_UI(ID_ADD_DBIMPORTMINUS, OnDisableMenuDEAL)
	ON_UPDATE_COMMAND_UI(ID_VU_CK_MAKIETOWANIE, OnUpdateVuCkMakietowanie)
	ON_COMMAND(ID_VIEW_ADDDESC, OnViewAdddesc)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW_1P, OnFilePrintPreview1p)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW_ALL, OnFilePrintPreviewAll)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_1P, OnFilePrint1p)
	ON_COMMAND(ID_FILE_PRINT_ALL, OnFilePrintAll)
	ON_COMMAND(ID_DRAW_OPCJE, OnDrawOpcje)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CZASOBOW, OnUpdateViewCzasobow)
	ON_COMMAND(ID_VIEW_CZASOBOW, OnViewCzasobow)
	ON_COMMAND(ID_FILE_PRINT_EPS, OnPrintEps)                    //GN
	ON_COMMAND(ID_CHECK_EPS,OnCheckEps)                          //GN
	ON_COMMAND(IDM_ASIDE_ADDS, OnAsideAdds)
	ON_COMMAND(IDM_VIEW_STUDIO, OnViewStudio)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_STUDIO, OnUpdateViewStudio)
	ON_UPDATE_COMMAND_UI_RANGE(ID_FILE_NEW, ID_EDIT_REDO, OnDisableMenu)
	ON_UPDATE_COMMAND_UI_RANGE(ID_ADD_FIND, IDM_ASIDE_ADDS, OnDisableMenu)
	ON_UPDATE_COMMAND_UI(ID_ADD_SYNCHRONIZE, OnDisableMenuDEAL)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_EPS, OnDisableMenuSTU)
	ON_UPDATE_COMMAND_UI(ID_ADD_EXPORT, OnDisableMenuLIB)
	ON_UPDATE_COMMAND_UI(ID_VU_MAKIETOWANIE, OnDisableMenuLIB)
	ON_UPDATE_COMMAND_UI(ID_CHECK_EPS, OnDisableMenuSTU) 
	ON_COMMAND(IDM_CHANGEVIEW, OnChangeView)
	ON_COMMAND(IDM_PREV_KOR, OnPrevKor)
	ON_UPDATE_COMMAND_UI(IDM_PREV_KOR, OnDisableMenu)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_VIEW_PAGEFONT, OnChoosePageFont)
	ON_COMMAND(ID_VIEW_ADDFONT, OnChooseAddFont)
	ON_COMMAND(IDM_IRFAN, &CDrawView::OnIrfan)
	ON_UPDATE_COMMAND_UI(IDM_IRFAN, OnDisableMenuAdSelOPI)
	ON_COMMAND(IDM_PREVPDF, &CDrawView::OnPrevPdf)
	ON_UPDATE_COMMAND_UI(IDM_PREVPDF, OnDisableMenuAdPageSel)
	ON_COMMAND(IDM_PREVDIG, &CDrawView::OnPrevDig)
	ON_UPDATE_COMMAND_UI(IDM_PREVDIG, OnDisableMenuAdSel)
	ON_COMMAND(IDM_ATEXSYG, &CDrawView::OnAtexSyg)
	ON_UPDATE_COMMAND_UI(IDM_ATEXSYG, OnDisableMenuAdSel)
	ON_UPDATE_COMMAND_UI(ID_FULLSCREEN, OnDisableMenu)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDrawView construction/destruction

CDrawView::CDrawView() :
	m_pagesPrinted(PRINT_DOC), m_bActive(FALSE),
	m_zoomNum(theApp.m_initZoom, theApp.m_initZoom), m_zoomDenom(100 * vscale, -100 * vscale)
{}

int CDrawView::OnDisableMenuInt(CCmdUI* pCmdUI) {
	/* vu: jesli zwroci 0 to ustawienie statusu odbedzie sie w funkcji wolajacej */
	if (disableMenu) pCmdUI->Enable(FALSE);
	return disableMenu ;
}

void CDrawView::OnDisableMenu(CCmdUI* pCmdUI)
{	pCmdUI->Enable(!disableMenu);	}

void CDrawView::OnDisableMenuRDBMS(CCmdUI* pCmdUI) 
{	pCmdUI->Enable(!disableMenu && theApp.isRDBMS);	}

void CDrawView::OnDisableMenuLIB(CCmdUI* pCmdUI) 
{	pCmdUI->Enable(!disableMenu && !GetDocument()->isLIB);	}

void CDrawView::OnDisableMenuDEAL(CCmdUI* pCmdUI)
{	pCmdUI->Enable(!disableMenu && !GetDocument()->isLIB && !(theApp.grupa&(R_DEA)));	}

void CDrawView::OnDisableMenuSTU(CCmdUI* pCmdUI)
{	pCmdUI->Enable(!disableMenu && !GetDocument()->isLIB && (theApp.grupa&R_STU));	}

void CDrawView::OnDisableMenuAdSel(CCmdUI* pCmdUI)
{	pCmdUI->Enable(!disableMenu && !GetDocument()->isLIB && m_selection.GetCount() == 1 && dynamic_cast<CDrawAdd*>(m_selection.GetHead()));	}

void CDrawView::OnDisableMenuAdPageSel(CCmdUI* pCmdUI)
{	pCmdUI->Enable(!disableMenu && !GetDocument()->isLIB && m_selection.GetCount() == 1 && (dynamic_cast<CDrawAdd*>(m_selection.GetHead()) || dynamic_cast<CDrawPage*>(m_selection.GetHead()))); }

void CDrawView::OnDisableMenuAdSelOPI(CCmdUI* pCmdUI)
{	pCmdUI->Enable(!disableMenu && !GetDocument()->isLIB && !theApp.isOpiMode && m_selection.GetCount() == 1 && dynamic_cast<CDrawAdd*>(m_selection.GetHead()));	}

void CDrawView::OnDisableMenuAdSelSTU(CCmdUI* pCmdUI)
{	pCmdUI->Enable(!disableMenu && !GetDocument()->isLIB && (theApp.grupa&R_STU) && m_selection.GetCount() == 1 && dynamic_cast<CDrawAdd*>(m_selection.GetHead()));	}

BOOL CDrawView::PreCreateWindow(CREATESTRUCT& cs)
{
	ASSERT(cs.style & WS_CHILD);
	if (cs.lpszClass == NULL)
		cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS); 
	return TRUE;
}

void CDrawView::OnActivateView(BOOL bActivate, CView* pActiveView, CView* pDeactiveView)
{
	CView::OnActivateView(bActivate, pActiveView, pDeactiveView);

	// invalidate selections when active status changes
	
	if (m_bActive != bActivate)
	{
		if (bActivate)  // if becoming active update as if active
			OnUpdate(pActiveView, HINT_UPDATE_COMBOBOXY, pDeactiveView);
		else if (dynamic_cast<CGridFrm*>(pActiveView))
			CDrawTool::c_drawShape = DrawShape::select; 

		if (!m_selection.IsEmpty())
			OnUpdate(NULL, HINT_UPDATE_SELECTION, NULL);
		m_bActive = bActivate;
	}

	if (bActivate) {
		if (pActiveView && theApp.activeDoc->swCZV != theApp.swCZV) {
			BYTE bPrevMode = theApp.swCZV;
			theApp.swCZV = ((CDrawDoc*)pActiveView->GetDocument())->swCZV ;
			((CMainFrame*)AfxGetMainWnd())->SetToolbarBitmap((ToolbarMode)bPrevMode, (ToolbarMode)theApp.swCZV) ;
		}
		((CMainFrame*)AfxGetMainWnd())->SetOpenStatus(GetDocument()->isRO ? _T("READ") : _T("WRITE"));
	}
}

BOOL CDrawView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{

	if (CDrawTool::c_drawShape == DrawShape::add || CDrawTool::c_drawShape == DrawShape::opis)
		return ::SetCursor(::LoadCursor(NULL, IDC_CROSS)) > 0;
		
	int cursor = -1;
	if (CDrawTool::c_drawShape == DrawShape::caption)
		cursor = IDC_CAPTION;
	else if (CDrawTool::c_drawShape == DrawShape::color)
		cursor = IDC_KOLOR;
	else if (CDrawTool::c_drawShape == DrawShape::lock)
		cursor = GetDocument()->swCZV&1 ? IDC_FLAG : IDC_LOCK;
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
	if (dynamic_cast<CDrawAdd*>(pObj)) {
		rect.bottom += (rect.Height() > 1.2*TXTSHIFT ? 3 : (int) (1.2*TXTSHIFT));
		rect.left -= TXTSHIFT;
		rect.right += TXTSHIFT;
		rect.top -= TXTSHIFT;
	} else {
		auto inflation = 3 * vscale;
		rect.InflateRect(inflation, inflation);
	}

	InvalidateRect(rect, TRUE);
} 

void CDrawView::OnUpdate(CView* , LPARAM lHint, CObject* pHint)
{
	switch (lHint)
	{
	case HINT_UPDATE_WINDOW:		// redraw entire window
	case HINT_UPDATE_DRAWVIEW:		// redraw entire window
		Invalidate(TRUE);
		break;

	case HINT_UPDATE_DRAWOBJ:		// a single object has changed
		InvalObj((CDrawObj*)pHint);
		break;

	case HINT_UPDATE_COMBOBOXY:
		((CMainFrame*)AfxGetMainWnd())->InsComboNrSpotow((int)GetDocument()->m_spot_makiety.size());
		((CMainFrame*)AfxGetMainWnd())->IniCaptionBox(-1, GetDocument()->id_drw);
		break;

	case HINT_SAVEAS_DELETE_SELECTION: // to usuwa ogloszenia z m_selection
		m_selection.RemoveAll();
		Invalidate(TRUE);
		break;

	case HINT_UPDATE_SELECTION: { // an entire selection has changed
		CObList* pList = pHint != NULL ? (CObList*)pHint : &m_selection;
		POSITION pos = pList->GetHeadPosition();
		while (pos != NULL)
			InvalObj((CDrawObj*)pList->GetNext(pos));
		break;
	}

	case HINT_DELETE_SELECTION: // an entire selection has been removed
		if (pHint != &m_selection) {
			CObList* pList = (CObList*)pHint;
			POSITION pos = pList->GetHeadPosition();
			while (pos != NULL) {
				auto pObj = reinterpret_cast<CDrawObj*>(pList->GetNext(pos));
				InvalObj(pObj);
				Remove(pObj);    // remove it from this view's selection
			}
		}
		break;

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
	if(pInfo && pInfo->GetMaxPage()==1 && m_pagesPrinted == PRINT_DOC && theApp.GetProfileInt(_T("General"), _T("ForceLandscape"), 0) > 0) {
		LPDEVMODE dm = SetLandscape();
		if (dm) pDC->ResetDC(dm);
	}
		
	CScrollView::OnPrepareDC(pDC, pInfo);

	pDC->SetMapMode(MM_ANISOTROPIC);
	pDC->SetViewportExt(m_zoomNum);
	pDC->SetWindowExt(m_zoomDenom);
	if (pInfo) {
		int i, l;
		i = (int)floor((float)pDC->GetDeviceCaps(HORZRES) / 6 / GetDocument()->iPagesInRow);
		l = (int)floor((float)pDC->GetDeviceCaps(VERTRES) / (theApp.colsPerPage * 40 + 2));  //40 - inaczej wchodza nastepne strony - 2 na naglowek
		pDC->SetViewportOrg(2 * i, 0);
		pDC->SetWindowOrg(0, (pInfo->m_nCurPage - 1)*(-(theApp.colsPerPage * 40 + 2))*(pmoduly)+(int)floor((float)pmoduly / 3));
		pDC->SetViewportExt(i, l);
		pDC->SetWindowExt(pmodulx, -pmoduly);
	}
}

BOOL CDrawView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
{
	// do the scroll
	if (!CScrollView::OnScrollBy(sizeScroll, bDoScroll))
		return FALSE;

	// update the position of any in-place active item
	if (bDoScroll)
	{
		Invalidate(TRUE);
		UpdateActiveItem();  //inv
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
	if (!pDC->IsPrinting())
	{
		// draw to offscreen bitmap for fast looking repaints
		if (bitmapDC.CreateCompatibleDC(pDC))
		{
			if (bitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height()))
			{
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

	if (pDrawDC != pDC)
	{
		pDC->SetViewportOrg(0, 0);
		pDC->SetWindowOrg(0, 0);
		pDC->SetMapMode(MM_TEXT);
		bitmapDC.SetViewportOrg(0, 0);
		bitmapDC.SetWindowOrg(0, 0);
		bitmapDC.SetMapMode(MM_TEXT);
		pDC->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), pDrawDC, 0, 0, SRCCOPY);
		bitmapDC.SelectObject(pOldBitmap);
	}
}  

void CDrawView::Remove(CDrawObj* pObj)
{
	POSITION pos = m_selection.Find(pObj);
	if (pos != NULL)
		m_selection.RemoveAt(pos);
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
	GetDocument()->UpdateAllViews(NULL, HINT_UPDATE_WINDOW, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// OLE Client support and commands

BOOL CDrawView::IsSelected(const CObject* pDocItem) const
{
	return m_selection.Find(const_cast<CObject*>(pDocItem)) != NULL;
}

// The following command handler provides the standard keyboard
//  user interface to cancel an in-place editing session.
void CDrawView::OnCancelEdit()
{
	// deactivate any in-place active item on this view!
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL)
	{
		// if we found one, deactivate it
		pActiveItem->Close();
	}
	ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);

	// escape also brings us back into select mode
	ReleaseCapture();

	CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
	if (pTool != nullptr)
		pTool->OnCancel();

	CDrawTool::c_drawShape = DrawShape::select;
}

void CDrawView::OnSetFocus(CWnd* pOldWnd)
{
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL &&
		pActiveItem->GetItemState() == COleClientItem::activeUIState)
	{
		// need to set focus to this item if it is in the same view
		CWnd* pWnd = pActiveItem->GetInPlaceWindow();
		if (pWnd != NULL)
		{
			pWnd->SetFocus();
			return;
		}
	}

	CScrollView::OnSetFocus(pOldWnd);
}

CRect CDrawView::GetInitialPosition()
{
	CRect rect(10, 10, 10, 10);
	ClientToDoc(rect);
	return rect;
}

void CDrawView::ClientToDoc(CPoint& point)
{
	CClientDC dc(this);
	OnPrepareDC(&dc, NULL);
	dc.DPtoLP(&point);
}

void CDrawView::ClientToDoc(CRect& rect)
{
	CClientDC dc(this);
	OnPrepareDC(&dc, NULL);
	dc.DPtoLP(rect);
	ASSERT(rect.left <= rect.right);
	ASSERT(rect.bottom <= rect.top);
}

void CDrawView::DocToClient(CPoint& point)
{
	CClientDC dc(this);
	OnPrepareDC(&dc, NULL);
	dc.LPtoDP(&point);
}

void CDrawView::DocToClient(CRect& rect)
{
	CClientDC dc(this);
	OnPrepareDC(&dc, NULL);
	dc.LPtoDP(rect);
	rect.NormalizeRect();
}

void CDrawView::Select(CDrawObj* pObj, BOOL bAdd)
{
	if (!bAdd) {
		OnUpdate(NULL, HINT_UPDATE_SELECTION, NULL);
		m_selection.RemoveAll();
	}

	if (pObj == nullptr || IsSelected(pObj))
		return;

	if (m_selection.GetCount()) {
		auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
		if (pAdd && pAdd->fizpage) return ;

		auto pPage = dynamic_cast<CDrawPage*>(pObj);
		if (bAdd && pPage) {
			int iHitPos = GetDocument()->GetIPage(pPage);
			for (int i = iHitPos - 1; i >= 0; i--)
				if (IsSelected(GetDocument()->m_pages[i]))
					for (int j = i + 1; j < iHitPos; j++)
						Select((CDrawObj*) GetDocument()->m_pages[j], TRUE);
			size_t iPC = GetDocument()->m_pages.size();
			for (size_t i = iHitPos + 1; i < iPC; i++)
				if (IsSelected(GetDocument()->m_pages[i]))
					for (size_t j = iHitPos + 1; j < i; j++)
						Select((CDrawObj*) GetDocument()->m_pages[j], TRUE);
		}
	}	

	m_selection.AddTail(pObj);
	InvalObj(pObj);
	GetDocument()->UpdateAllViews(this, HINT_UPDATE_GRID, (CDrawAdd*)pObj);  //zeby grid zareagowal
}

// rect is in device coordinates
void CDrawView::SelectWithinRect(CRect rect, BOOL bAdd)
{
	if (!bAdd)
		Select(NULL);

	ClientToDoc(rect);

	for (const auto& pObj : GetDocument()->m_objects)
		if (pObj->Intersects(rect))
			Select(pObj, TRUE);
}

void CDrawView::Deselect(CDrawObj* pObj)
{
	POSITION pos = m_selection.Find(pObj);
	if (pos != NULL)
	{
		InvalObj(pObj);
		m_selection.RemoveAt(pos);
	}
}

void CDrawView::CloneSelection()
{
	POSITION prev, pos = m_selection.GetHeadPosition();
	while (pos != NULL)
	{
		prev = pos;
		auto pObj = (CDrawObj*)m_selection.GetNext(pos);
		if (!dynamic_cast<CDrawAdd*>(pObj)) { // og³oszeñ nie klonujemy
			m_selection.SetAt(prev, pObj->Clone(pObj->m_pDocument));
			InvalObj(pObj);
		}
	}
}

void CDrawView::UpdateActiveItem()
{
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL &&
		pActiveItem->GetItemState() == COleClientItem::activeUIState)
	{
		// this will update the item rectangles by calling
		//  OnGetPosRect & OnGetClipRect.
		pActiveItem->SetItemRects();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDrawView message handlers

void CDrawView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!m_bActive)
		return;
	CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
	if (pTool != nullptr)
		pTool->OnLButtonDown(this, nFlags, point);
}

void CDrawView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!m_bActive)
		return;
	CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
	if (pTool != nullptr)
		pTool->OnLButtonUp(this, nFlags, point);
}

void CDrawView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (theApp.unQueing && !m_bActive) {
		CDrawTool::c_drawShape = DrawShape::select ;
		m_selection.RemoveAll();
		m_selection.AddTail(CQueView::selected_add);
		SetCapture() ;
		m_bActive = TRUE ;
	}
	if (!m_bActive)
		return;
	CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
	if (pTool != nullptr)
		pTool->OnMouseMove(this, nFlags, point);
}

BOOL CDrawView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	auto delta = zDelta / WHEEL_DELTA;
	float fFactor = (15 + (float)delta) / 15;
	if ((nFlags & MK_CONTROL) > 0) {
		CDrawOpis* pOpi;
		if (m_selection.GetCount() == 1 && (pOpi = dynamic_cast<CDrawOpis*>(m_selection.GetHead()))) {
			if ((0.1 < pOpi->m_Scale || fFactor < 1.0) && (pOpi->m_Scale < 10 || fFactor > 1.0))
				pOpi->m_Scale /= fFactor;
			pOpi->SetDirty();
			pOpi->Invalidate();
		} else {
			CSize zoomNum((int)(m_zoomNum.cx * fFactor), (int)(m_zoomNum.cy * fFactor));
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

void CDrawView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (!m_bActive)
		return;
	CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
	if (pTool != nullptr)
		pTool->OnLButtonDblClk((CDrawView*)this, nFlags, point);
}

void CDrawView::OnDestroy()
{
	CScrollView::OnDestroy();

	// deactivate the inplace active item on this view
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL && pActiveItem->GetActiveView() == this)
	{
		pActiveItem->Deactivate();
		ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);
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

void CDrawView::OnDrawTool(UINT tool)
{
	auto selected_tool = static_cast<DrawShape>(tool);
	if  (CDrawTool::c_drawShape == selected_tool)
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
	if (GetDocument()->isLIB) 
		pCmdUI->Enable(FALSE) ;
	else 
		OnUpdateDrawTool(pCmdUI);
}

void CDrawView::OnUpdateSingleSelect(CCmdUI* pCmdUI)
{
	if (!OnDisableMenuInt(pCmdUI)) 
		pCmdUI->Enable(m_selection.GetCount() == 1);
}

void CDrawView::OnEditSelectAll()
{
	for (const auto& pObj : GetDocument()->m_objects)
		Select(pObj, TRUE);
}

void CDrawView::OnUpdateEditSelectAll(CCmdUI* pCmdUI)
{
	if (!OnDisableMenuInt(pCmdUI)) pCmdUI->Enable(GetDocument()->m_objects.size() != 0);
}

void CDrawView::OnEditClear()
{
	BOOL to_tell = FALSE;
	// update all the views before the selection goes away
	GetDocument()->UpdateAllViews(NULL, HINT_DELETE_SELECTION, &m_selection);
	OnUpdate(NULL, HINT_UPDATE_SELECTION, NULL);

	// now remove the selection from the document
	POSITION pos = m_selection.GetHeadPosition();
	while (pos != NULL)	{
		CDrawObj* pObj = (CDrawObj*)m_selection.GetNext(pos);
		CDrawAdd* pAdd = dynamic_cast<CDrawAdd*>(pObj);
		if (pAdd) {
			if (pAdd->flags.derived) continue;
			if (pAdd->m_add_xx > 0) {
				AfxMessageBox(_T("Wska¿ emisje przeznaczone do usuniêcia"), MB_ICONINFORMATION|MB_OK);
				continue;
			}
			to_tell = TRUE;
		} else {
			CDrawPage *pPage = dynamic_cast<CDrawPage*>(pObj);
			if (pPage) {
				if (pPage->m_dervlvl == DERV_ADDS && !pPage->m_adds.empty()) 
					continue;
				if (std::any_of(pPage->m_adds.cbegin(), pPage->m_adds.cend(), [](CDrawAdd* const& pAdd){ return pAdd->flags.derived; }))
					continue;
				to_tell = TRUE;
			}
		}		
		GetDocument()->Remove(pObj);
		delete pObj;
	}
	m_selection.RemoveAll();  
	if (to_tell)
		GetDocument()->UpdateAllViews(this, HINT_DELETE_FROM_GRID, NULL);  //zeby grid zareagowal
}

void CDrawView::OnUpdateAnySelect(CCmdUI* pCmdUI)
{
	if (!OnDisableMenuInt(pCmdUI)) pCmdUI->Enable(!m_selection.IsEmpty());
}

void CDrawView::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);
	UpdateActiveItem();
}

void CDrawView::OnViewSpaceLocks()
{
	((CMainFrame*)AfxGetMainWnd())->show_spacelocks = !(((CMainFrame*)AfxGetMainWnd())->show_spacelocks);
	Invalidate(FALSE);
}

void CDrawView::OnUpdateViewSpaceLocks(CCmdUI* pCmdUI)
{
	if (!OnDisableMenuInt(pCmdUI)) pCmdUI->SetCheck(((CMainFrame*)AfxGetMainWnd())->show_spacelocks);
}

void CDrawView::OnImportOpcje()
{ 
	theApp.includeKratka = !(theApp.includeKratka);
	Invalidate(FALSE);
}

void CDrawView::OnUpdateImportOpcje(CCmdUI* pCmdUI)
{
	if (!OnDisableMenuInt(pCmdUI)) {
		if (GetDocument()->isLIB) pCmdUI->Enable(FALSE) ;
		pCmdUI->SetCheck(theApp.includeKratka);
	}
}

BOOL CDrawView::OnEraseBkgnd(CDC* )
{
	return TRUE;
}

void CDrawView::OnObjectMoveForward()
{
	CDrawDoc* pDoc = GetDocument();
	CDrawObj* pObj = reinterpret_cast<CDrawObj*>(m_selection.GetHead());
	auto pObjects = &pDoc->m_objects;

	auto pos = std::find(pObjects->begin(), pObjects->end(), pObj);
	if (pos == pObjects->end())
		return; // tzn ze to strona

	if (std::next(pos) != pObjects->end()) {
		std::swap(pDoc->m_objects[pos - pObjects->begin()], pDoc->m_objects[pos - pObjects->begin() + 1]);
		InvalObj(pObj);    

		if (dynamic_cast<CDrawAdd*>(pObj))
			pDoc->UpdateAllViews(this, HINT_EDIT_PASTE, pObj); // dla gridu by zmienic
	}
}

void CDrawView::OnObjectMoveBack()
{
	CDrawDoc* pDoc = GetDocument();
	CDrawObj* pObj = reinterpret_cast<CDrawObj*>(m_selection.GetHead());
	auto pObjects = &pDoc->m_objects;

	auto pos = std::find(pObjects->begin(), pObjects->end(), pObj);
	if (pos == pObjects->end())
	   return; // tzn ze to strona

	if (pos != pObjects->begin()) {
		std::swap(pDoc->m_objects[pos - pObjects->begin() - 1], pDoc->m_objects[pos - pObjects->begin()]);
		InvalObj(pObj);

		if (dynamic_cast<CDrawAdd*>(pObj))
			pDoc->UpdateAllViews(this, HINT_EDIT_PASTE, pObj); // dla gridu by zmienic
	}
}

void CDrawView::OnObjectMoveToFront()
{
	CDrawDoc* pDoc = GetDocument();
	CDrawObj* pObj = reinterpret_cast<CDrawObj*>(m_selection.GetHead());
	auto pObjects = &pDoc->m_objects;

	auto pos = std::find(pObjects->begin(), pObjects->end(), pObj);
	if (pos == pObjects->end()) 
		return; // tzn ze to strona

	std::swap(pDoc->m_objects[pos - pObjects->begin()], pDoc->m_objects[pDoc->m_objects.size() - 1]);
	InvalObj(pObj);

	if (dynamic_cast<CDrawAdd*>(pObj))
		pDoc->UpdateAllViews(this, HINT_EDIT_PASTE, pObj); // dla gridu by zmienic
}

void CDrawView::OnObjectMoveToBack()
{
	CDrawDoc* pDoc = GetDocument();
	CDrawObj* pObj = (CDrawObj*)m_selection.GetHead();
	auto pObjects = &pDoc->m_objects;

	auto pos = std::find(pObjects->begin(), pObjects->end(), pObj);
	if (pos == pObjects->end()) 
		return; // tzn ze to strona

	std::swap(pDoc->m_objects[0], pDoc->m_objects[pos - pObjects->begin()]);
	InvalObj(pObj);

	if (dynamic_cast<CDrawAdd*>(pObj))
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
	m_selection.Serialize(ar);
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
	if (pFile.get() == nullptr)
		return;

	// connect the file to the archive
	CArchive ar(pFile.get(), CArchive::load);
	ar.m_pDocument = GetDocument(); // set back-pointer in archive
	m_selection.Serialize(ar);
	ar.Close();
}

void CDrawView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	if (!OnDisableMenuInt(pCmdUI)) pCmdUI->Enable(!m_selection.IsEmpty());
}

void CDrawView::OnEditCut()
{
	OnEditCopy();
	OnEditClear();
}

void CDrawView::OnUpdateEditCut(CCmdUI* pCmdUI)
{
	if (!OnDisableMenuInt(pCmdUI)) pCmdUI->Enable(!m_selection.IsEmpty());
}

void CDrawView::OnEditPaste()
{
	COleDataObject dataObject;
	dataObject.AttachClipboard();
	if (!dataObject.IsDataAvailable(m_cfDraw))
		return;

	// invalidate current selection since it will be deselected
	OnUpdate(NULL, HINT_UPDATE_SELECTION, NULL);
	m_selection.RemoveAll();
	// deserialize objects from clipboard into m_selection
	Paste(dataObject);
	// add all items : m_selection to document
	bool newAds = false;
	for (POSITION pos = m_selection.GetHeadPosition(); pos != NULL; /* GetNext */) {
		auto pObj = reinterpret_cast<CDrawObj*>(m_selection.GetNext(pos));
		if (dynamic_cast<CDrawPage*>(pObj))
			GetDocument()->AddPage(reinterpret_cast<CDrawPage*>(pObj));
		else if (dynamic_cast<CDrawOpis*>(pObj)) {
			auto pOpi = reinterpret_cast<CDrawOpis*>(pObj);
			pOpi->m_position += CSize(15 * vscale, -15 * vscale);
			GetDocument()->Add(pOpi);
		} else if (dynamic_cast<CDrawAdd*>(pObj)) {
				newAds = true;
				AfxMessageBox(_T("Aby nie dopuœciæ do powtórek, sklonowane og³oszenie nie bêdzie mia³o numeru"));
				auto pAdd = reinterpret_cast<CDrawAdd*>(pObj);
				pAdd->nreps = -1L;
				GetDocument()->Add(pAdd);
		}
		pObj->Invalidate();
	}

	// invalidate new pasted stuff
	GetDocument()->UpdateAllViews(this);
	if (newAds)
		GetDocument()->UpdateAllViews(this, HINT_EDIT_PASTE, NULL); // dla gridu by dodac
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
LPDEVMODE CDrawView::SetLandscape() {
	//nazwa drukarki domyœlnej
	DWORD lSize = n_size;
	if (!::GetDefaultPrinter(theApp.bigBuf, &lSize))
		return NULL;
	//otwórz
	HANDLE hPrinter;
	if (!::OpenPrinter(theApp.bigBuf, &hPrinter, NULL))
		return NULL;
	//pobierz ustawienia
	DWORD dwNeeded = DocumentProperties(this->m_hWnd, hPrinter, theApp.bigBuf, NULL, NULL, 0);
	PDEVMODE dm = (PDEVMODE)LocalAlloc(LMEM_FIXED, dwNeeded);
	//zmieñ
	DocumentProperties(this->m_hWnd, hPrinter, theApp.bigBuf, dm, NULL, DM_OUT_BUFFER);
	if (dm && dm->dmFields & DM_ORIENTATION)
		dm->dmOrientation = DMORIENT_LANDSCAPE ;
	DocumentProperties(this->m_hWnd, hPrinter, theApp.bigBuf, dm, dm, DM_IN_BUFFER|DM_OUT_BUFFER);
	//zamknij
	::ClosePrinter(hPrinter);
	return dm;
}

BOOL CDrawView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation  
	int format = theApp.colsPerPage==1 ? A4 : A3;
	switch (m_pagesPrinted) {
		case PRINT_PAGE:
			pInfo->SetMaxPage((UINT)GetDocument()->m_pages.size()); break;
		case PRINT_DOC:
			pInfo->SetMaxPage((int)ceil((float)GetDocument()->m_pages.size()/(float)format)); break;
		case PRINT_ALL:
			pInfo->SetMaxPage(1); break;
	}

	BOOL doPrint = DoPreparePrinting(pInfo);
	if (!doPrint)
		theApp.SetScale(CLIENT_SCALE);
	return doPrint ;
}

void CDrawView::OnBeginPrinting(CDC* , CPrintInfo* )
{
	theApp.SetScale(1);
	GetDocument()->ComputeCanvasSize();
}

void CDrawView::OnEndPrinting(CDC* , CPrintInfo* )
{
	theApp.SetScale(CLIENT_SCALE);
	GetDocument()->UpdateAllViews(NULL);
};

void CDrawView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	CDrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	int pc,wspol = theApp.colsPerPage ;
	if (wspol == 2 || m_pagesPrinted != PRINT_PAGE) 
		pDoc->PrintInfo(pDC, pInfo->GetMaxPage(), wspol); 
	CDrawPage *pPage ;
	switch (m_pagesPrinted) {
	case PRINT_PAGE:
		pPage = (GetDocument()->m_pages[pInfo->m_nCurPage - 1]);
		pDC->SetMapMode(MM_ANISOTROPIC);
		pDC->SetViewportOrg(0, 0);
		pDC->SetViewportExt(pDC->GetDeviceCaps(HORZRES), pDC->GetDeviceCaps(VERTRES));
		pDC->SetWindowOrg(pPage->m_position.left / CLIENT_SCALE - 5 * vscale, pPage->m_position.top / CLIENT_SCALE - 18 * vscale - 2 * (pInfo->m_nCurPage / (wspol * 50 + 1))*pmoduly - PRINT_VOFFSET);
		pDC->SetWindowExt((pmodulx + 2 * vscale)*pszpalt_x, -(pmoduly + 4 * vscale)*pszpalt_y);
		pDoc->PrintPage(pPage, pDC);
		break;
	case PRINT_DOC:
		pDoc->Print(pDC);
		break;
	case PRINT_ALL:
		pc = (int)pDoc->m_pages.size();
		if ((theApp.colsPerPage == 1 && pc > A4) || (theApp.colsPerPage == 2 && pc > A3)) {
			pDC->SetMapMode(MM_ANISOTROPIC);
			pDC->SetViewportOrg(0, 0);
			pDC->SetViewportExt(pDC->GetDeviceCaps(HORZRES), pDC->GetDeviceCaps(VERTRES));
			pDC->SetWindowOrg(0, -2 * PRINT_VOFFSET);
			pDC->SetWindowExt(((int) (5.5*pDoc->iPagesInRow) + 1)*pmodulx, -(int) ((pDoc->m_pages.size() / pDoc->iPagesInRow + 1.7) * 8 * pmoduly));
		}
		pDoc->Print(pDC);
		break;
	}
}

void CDrawView::OnFilePrint()
{
	if (m_pagesPrinted == PRINT_NULL) m_pagesPrinted = PRINT_DOC ;
	CScrollView::OnFilePrint();
	GetDocument()->ComputeCanvasSize();
	m_pagesPrinted = PRINT_NULL ;
}

void CDrawView::OnFilePrint1p()
{
	m_pagesPrinted = PRINT_PAGE ;
	CScrollView::OnFilePrint();
	GetDocument()->ComputeCanvasSize();
	m_pagesPrinted = PRINT_NULL ;
}

void CDrawView::OnFilePrintAll()
{
	m_pagesPrinted = PRINT_ALL ;
	CScrollView::OnFilePrint();
	GetDocument()->ComputeCanvasSize();
	m_pagesPrinted = PRINT_NULL ;
}

void CDrawView::OnFilePrintPreview1p()
{
	m_pagesPrinted = PRINT_PAGE ;
	AFXPrintPreview (this);
}

void CDrawView::OnFilePrintPreviewAll()
{
	m_pagesPrinted = PRINT_ALL ;
	AFXPrintPreview (this);
}

void CDrawView::OnFilePrintPreview()
{
	m_pagesPrinted = PRINT_DOC ;
	AFXPrintPreview (this);
}

void CDrawView::OnEditProperties()
{
	if (m_selection.GetCount() == 1 && CDrawTool::c_drawShape == DrawShape::select)
	{
		CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
		if (pTool) 
			pTool->OnLButtonDblClk(this, 0, CPoint(0, 0));

		ASSERT(pTool != NULL);
	}
}

void CDrawView::OnUpdateEditProperties(CCmdUI* pCmdUI)
{
	if (!OnDisableMenuInt(pCmdUI)) 
		pCmdUI->Enable(m_selection.GetCount() == 1 && CDrawTool::c_drawShape == DrawShape::select);
}

//FONTY
void CDrawView::OnChooseFont(CFont& m_font, BOOL IsPageFont)
{
	LOGFONT lf;
	if (m_font.m_hObject != nullptr)
		m_font.GetObject(sizeof(LOGFONT), &lf);
	else
		::GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &lf);

	CFontDialog dlg(&lf, CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT);
	if (dlg.DoModal() == IDOK)	{
		theApp.WriteProfileString(_T("Settings"), IsPageFont ? _T("PageFontFace") : _T("AddFontFace"), lf.lfFaceName);
		theApp.WriteProfileInt(_T("Settings"), IsPageFont ? _T("PageFontSize") : _T("AddFontSize"), lf.lfHeight);
		m_font.DeleteObject();
		if (m_font.CreateFontIndirect(&lf))
			SetFont(&m_font);
		GetDocument()->UpdateAllViews(NULL);
	}
}

void CDrawView::OnChoosePageFont()
{
	OnChooseFont(GetDocument()->m_pagefont, TRUE);
}

void CDrawView::OnChooseAddFont()
{
	OnChooseFont(GetDocument()->m_addfont, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CDrawView diagnostics
#ifdef _DEBUG
void CDrawView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CDrawView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
BOOL CDrawView::SetZoomFactor(CSize zoomNum, CSize zoomDenom)
{
	if (zoomDenom != m_zoomDenom || zoomNum != m_zoomNum)
	{
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

void CDrawView::OnViewZoomN(UINT nID)
{
	ASSERT(nID >= ID_VIEW_ZOOM100 && nID <= ID_VIEW_ZOOM500);

	CSize zoomDenom;
	CSize zoomNum;

	zoomDenom.cx = zoomDenom.cy = 0;

	// get zoom factor numerator and set it
	ASSERT(nID - ID_VIEW_ZOOM100 < rgiZoomFactor.size());
	int iZoomFactor = rgiZoomFactor[nID-ID_VIEW_ZOOM100];
	zoomNum.cx = iZoomFactor;
	zoomNum.cy = iZoomFactor;
	// change the zoom factor to new zoom factor
	SetZoomFactor(zoomNum, zoomDenom);
}

void CDrawView::OnUpdateViewZoomN(CCmdUI* pCmdUI)
{
	if (pCmdUI->m_nID != ID_VIEW_ZOOMCUSTOM)
	{
		int iZoomFactor = rgiZoomFactor[pCmdUI->m_nID - ID_VIEW_ZOOM100];

		if (iZoomFactor == m_zoomNum.cx && iZoomFactor == m_zoomNum.cy)
		{
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

	if (dlg.DoModal() == IDOK)
	{
		CSize zoomNum(dlg.m_zoomX, dlg.m_zoomX);
		SetZoomFactor(zoomNum, m_zoomDenom);
	}
}

void CDrawView::OnDrawMakStrony()
{
	POSITION pos = m_selection.GetHeadPosition() ;
	//na liscie m_selection jest dokladnie jedna strona - OnUpdateDrawMakStrony
	CDrawDoc *vDoc = GetDocument() ;
	//przemakietuj ogloszenia na zaznaczonej stronie
	vDoc->MakietujStrone((CDrawPage*)m_selection.GetNext(pos));
	//ustaw z boku te, ktorych sie nie zmakietowalo automatycznie
	vDoc->AsideAdds() ;
	vDoc->UpdateAllViews(NULL);
}

void CDrawView::OnUpdateDrawMakStrony(CCmdUI* pCmdUI)
{
	if (OnDisableMenuInt(pCmdUI)) return ;
	if (GetDocument()->isLIB || m_selection.GetCount() != 1) {
		pCmdUI->Enable(FALSE);
		return;
	}
	
	auto pPage = dynamic_cast<CDrawPage*>(m_selection.GetHead());
	pCmdUI->Enable(pPage && !pPage->niemakietuj);
}

void CDrawView::OnUpdateVuCkMakietowanie(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(drawErrorBoxes) ;
	OnDisableMenu(pCmdUI) ;
}

void CDrawView::OnViewAdddesc()
{
	/* vu : Otwiera dialog pozwalaj¹cy ustawiæ rodzaj opisu og³oszenia.
			S¹ dwa poziomy opisu - dolny i górny, dla nich obowi¹zuj¹ 
			kody zgodne z kodami radio buttonów					end vu */
			
	CAddDesc dlg(theApp.m_view_top, theApp.m_view_bottom) ;

	if (dlg.DoModal() != IDOK) return ;
	
	if (dlg.m_top == dlg.m_bottom) dlg.m_bottom = TEXT_BRAK ; // nie mog¹ byæ takie same
	theApp.m_view_bottom = dlg.m_bottom ;
	theApp.m_view_top = dlg.m_top ;
	GetDocument()->UpdateAllViews(NULL);
}

void CDrawView::OnDrawOpcje()
{   GetDocument()->OnDrawOpcje() ;	}

void CDrawView::OnUpdateViewCzasobow(CCmdUI* pCmdUI)
{
	if (!OnDisableMenuInt(pCmdUI)) 
		pCmdUI->SetCheck(GetDocument()->swCZV&1) ;
}

void CDrawView::OnViewCzasobow()
{	
	BYTE bPrevMode = theApp.swCZV;
	theApp.swCZV = GetDocument()->swCZV = (GetDocument()->swCZV-1)*(GetDocument()->swCZV-1) ;	//f(x) = (x-1)^2
	((CMainFrame*)AfxGetMainWnd())->SetToolbarBitmap((ToolbarMode)bPrevMode, (ToolbarMode)theApp.swCZV) ;
	Invalidate(FALSE) ;
}

void CDrawView::OnChangeView()
{	
	BYTE bPrevMode = theApp.swCZV;
	theApp.swCZV = GetDocument()->swCZV = (GetDocument()->swCZV + 1) % 3;
	((CMainFrame*)AfxGetMainWnd())->SetToolbarBitmap((ToolbarMode)bPrevMode, (ToolbarMode)theApp.swCZV) ;
	Invalidate(FALSE) ;
}

void CDrawView::OnViewStudio()
{	
	BYTE bPrevMode = theApp.swCZV;
	theApp.swCZV = GetDocument()->swCZV = 2 - (GetDocument()->swCZV&2) ;	//f(x) = 2-(x&2)
	((CMainFrame*)AfxGetMainWnd())->SetToolbarBitmap((ToolbarMode)bPrevMode, (ToolbarMode)theApp.swCZV) ;
	Invalidate(FALSE) ;
}

void CDrawView::OnUpdateViewStudio(CCmdUI* pCmdUI)
{
	if (!OnDisableMenuInt(pCmdUI))
		pCmdUI->SetCheck((GetDocument()->swCZV&2) > 0 ? 1 : 0) ;
}

void CDrawView::OnAsideAdds()
{ GetDocument()->OnAsideAdds(); }

//GN
DWORD WINAPI CDrawView::DelegateGenEPS(LPVOID pArg) {
	BOOL bStatus = TRUE;
	PGENEPSARG pA = reinterpret_cast<PGENEPSARG>(pArg);
	CDrawPage *pPage = pA->pPage;
	if (pA->pDlg->m_bIsGen) {
		bStatus = pPage->GenEPS(pA);
		if (!bStatus) 
			pA->pDlg->cancelGenEPS = TRUE;
	} else 
		pPage->CheckSrcFile(pA); 

	return bStatus;
}

void CDrawView::OnPrintEps(){
	CheckPrintEps(TRUE);
}

void CDrawView::OnCheckEps(){
	CheckPrintEps(FALSE);
}

void CDrawView::CheckPrintEps(BOOL isprint) {
	int p = -1, k = -1;
	CPrnEpsDlg d;
	auto pDoc = GetDocument();

	if (!pDoc->m_pages.size()) return;

	theManODPNET.LoadMakietaDirs(pDoc->m_mak_xx);

	auto pPage = pDoc->m_pages[0];
	d.m_isprint = isprint; d.m_signall = 1;
	d.m_do = pPage->GetNrPaginy();
	d.m_od = pDoc->m_pages.size() == 1 ? d.m_do : pDoc->m_pages[1]->GetNrPaginy();
	d.m_bPotokowe = (BOOL)theApp.isParalellGen;
	d.m_markfound = (BOOL)theApp.GetProfileInt(_T("GenEPS"), _T("autoMark"), 0);

	if (m_selection.GetCount() == 1 && (pPage = dynamic_cast<CDrawPage*>(m_selection.GetHead()))) {
		d.m_page = 2;
		d.m_subset = pPage->GetNrPaginy();
	} else
		d.m_page = 0;

	if (d.DoModal() == IDCANCEL) return;
	theApp.isParalellGen = d.m_bPotokowe ? 1 : 0;
	theApp.WriteProfileInt(_T("GenEPS"), _T("autoMark"), theApp.autoMark = d.m_markfound);
	
	int pc = (int)pDoc->m_pages.size();
	CFlag wyborStron(0, 0, 1, pc);
	switch (d.m_page) {
		case 0:
			wyborStron = CFlag(1, pc, 1, pc);
			break;
		case 1:
			p = pDoc->Nr2NrPorz(d.m_od);
			k = pDoc->Nr2NrPorz(d.m_do);
			if (p < 0 || k < 0) goto subseterr;
			if (k == 0) k = pc;
			if (p == 0) p = pc;
			for (int i = p; i <= k; i++)
				wyborStron.SetBit(i % pc);
			break;
		case 2:
			int liczbaA, liczbaB;
			TCHAR *tok;
			const TCHAR septok[] = {',', '-'};
			::StringCchCopy(theApp.bigBuf, n_size, d.m_subset);
			tok = _tcstok(theApp.bigBuf, septok);
			while (tok != NULL) {
				liczbaA = pDoc->Nr2NrPorz(tok);
				if (liczbaA == -1) goto subseterr;
				else if (liczbaA == 0) liczbaA = pc;
				switch (d.m_subset.GetAt((int)(tok - theApp.bigBuf + _tcslen(tok)))) {
					case '\0':
					case ',':
						wyborStron.SetBit(liczbaA % pc);
						tok = _tcstok(NULL, septok);
						break;
					case '-':
						tok = _tcstok(NULL, septok);
						liczbaB = pDoc->Nr2NrPorz(tok);
						if (liczbaB == -1) goto subseterr;
						else if (liczbaB == 0) liczbaB = pc;
						for (int i = liczbaA; i <= liczbaB; i++)
							wyborStron.SetBit(i % pc);
						tok = _tcstok(NULL, septok);
						break;
					default:
						goto subseterr;
				}
			}
			break;
subseterr:
			AfxMessageBox(_T("Nie okreœlono poprawnego podzbioru stron"));
			return;
	}

	if (!pDoc->m_Rozm.size())
		pDoc->IniRozm();
	if (isprint && !pDoc->SaveModified()) return;

	int iCpuCnt, iThreadCnt = 0;
	HANDLE *ThreadHandles;
	GENEPSARG *ThreadArgs;
	BeginWaitCursor();

	if (isprint) {
		CString err(' ', 1024);
		pDoc->sOpiServerUrl = CString(' ', 64);

		CManODPNETParms orapar {
			{ CManODPNET::DbTypeInt32, &pDoc->m_mak_xx },
			{ CManODPNET::DbTypeVarchar2, CManODPNET::ParameterOut, &pDoc->sOpiServerUrl },
			{ CManODPNET::DbTypeVarchar2, CManODPNET::ParameterOut, &err }
		};
		orapar.outParamsCount = 2;
		theManODPNET.EI("begin epstest.check_makieta_prod2(:mak_xx,:opi_url,:err); end;", orapar);

		if (err.GetLength() > 8 && IDNO == AfxMessageBox(CString("Stwierdzono niezgodnoœæ potrzebnych i dostarczonych materia³ów graficznych dla og³oszeñ:\n\n") + err + "\n\nCzy chcesz kontynuowaæ produkcjê kolumn?", MB_YESNO))
			return;

		if (theApp.sManamEpsName.IsEmpty()) 
			theManODPNET.GetManamEps();
	}

	CGenEpsInfoDlg *pDlg = CGenEpsInfoDlg::GetGenEpsInfoDlg(isprint);

	if (d.m_bPotokowe) {
		iCpuCnt = CGenEpsInfoDlg::GetCpuCnt();
		pDlg->SetChannelCount(iCpuCnt);
		int iIleStron = wyborStron.GetBitCnt(1);
		if (iIleStron < iCpuCnt)
			iCpuCnt = iIleStron;

		ThreadHandles = (HANDLE*)LocalAlloc(LMEM_FIXED, iCpuCnt * sizeof(HANDLE));
		ThreadArgs = (GENEPSARG*)LocalAlloc(LMEM_FIXED, iCpuCnt * sizeof(GENEPSARG));
		for (int i = 0; i < iCpuCnt; i++) {
			auto& ta = ThreadArgs[i];
			ta.iChannelId = i;
			ta.bIsPRN = (BOOL)d.m_format;
			ta.bIsPreview = d.m_preview;
			ta.bSignAll = d.m_signall;
			ta.bDoKorekty = d.m_korekta;
			ta.pDlg = (CGenEpsInfoDlg*)pDlg;
			ta.cBigBuf = i==0 ? theApp.bigBuf : (TCHAR*)VirtualAlloc(NULL, bigSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			if (!ta.cBigBuf) {
				AfxMessageBox(_T("Zbyt ma³o pamiêci do uruchomienia dodatkowego potoku (Buf)"), MB_ICONSTOP);
				return;
			}
		}
	}

	BOOL bInitOpiMode = theApp.isOpiMode;
	if (isprint && d.m_format == F_EPS) theApp.isOpiMode = FALSE; // Dla F_EPS nie u¿ywaj OPI

	GENEPSARG genEpsArg;
	auto& channId = genEpsArg.iChannelId = 0;
	genEpsArg.bIsPRN = (BOOL)d.m_format;
	genEpsArg.bIsPreview = d.m_preview;
	genEpsArg.bSignAll = d.m_signall;
	genEpsArg.bDoKorekty = d.m_korekta;
	genEpsArg.cBigBuf = theApp.bigBuf;
	genEpsArg.pPage = pPage;
	genEpsArg.pDlg = (CGenEpsInfoDlg*)pDlg;
	pDoc->ovEPS = FALSE;

	for (int i = 0; !pDlg->cancelGenEPS && i < pc; i++) {
		if (wyborStron.GetBit(i) > 0) {
			pPage = pDoc->m_pages[i];
			int dl = pPage->m_dervlvl;
			if (dl == DERV_FIXD || dl == DERV_PROH || (d.m_exclude_emptypages && pPage->m_adds.size()==0 && pPage->name.Find(_T("DROB"))==-1))
				continue;

			if (isprint) { // generate
				if (d.m_format != F_PDF) {
					if (d.m_bPotokowe) {
						if (iThreadCnt >= iCpuCnt) {
							channId = ::WaitForMultipleObjects(iCpuCnt, ThreadHandles, FALSE, INFINITE) - WAIT_OBJECT_0;
							::CloseHandle(ThreadHandles[channId]);
						} else
							channId = iThreadCnt++;
						ThreadArgs[channId].pPage = pPage;
						ThreadHandles[channId] = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CDrawView::DelegateGenEPS, &ThreadArgs[channId], 0, (LPDWORD)&genEpsArg.iThreadId);
					} else {
						if (!pPage->GenEPS(&genEpsArg)) {
							pDlg->cancelGenEPS = TRUE;
							break; 
						}
					}
				} else {
					if (!pPage->GenPDF(&genEpsArg))
						break; 
				}
			} else { // check
				if (d.m_bPotokowe) {
					if (iThreadCnt >= iCpuCnt) {
						channId = ::WaitForMultipleObjects(iCpuCnt, ThreadHandles, FALSE, INFINITE) - WAIT_OBJECT_0;
						::CloseHandle(ThreadHandles[channId]);
					} else
						channId = iThreadCnt++;
					ThreadArgs[channId].pPage = pPage;
					ThreadHandles[channId] = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CDrawView::DelegateGenEPS, &ThreadArgs[channId], 0, (LPDWORD)&genEpsArg.iThreadId);
				} else 
					pPage->CheckSrcFile(&genEpsArg);
			}
		} 
	}

	if (d.m_bPotokowe) {
		::WaitForMultipleObjects(iThreadCnt, ThreadHandles, TRUE, INFINITE);
		for (int i = 0; i < iThreadCnt; i++) 
			::CloseHandle(ThreadHandles[i]);
		::LocalFree(ThreadHandles);

		for (int i = 1; i < iCpuCnt; i++) 
			::VirtualFree(ThreadArgs[i].cBigBuf, 0, MEM_RELEASE);
		::LocalFree(ThreadArgs);
	}
	AfxGetMainWnd()->ActivateTopParent();
	EndWaitCursor();

	if (!isprint) {
		pDlg->ShowWindow(SW_HIDE);
		CString msg;
		for (int i = 0; i < pc; i++)
			if (wyborStron.GetBit(i) > 0)
				msg += pDoc->m_pages[i]->f5_errInfo;
		::MessageBox(theApp.GetMainWnd()->m_hWnd, msg.IsEmpty() ? _T("Nie ma b³êdów") : msg , _T("Raport sprawdzenia EPS"), MB_OK);
		if (theApp.autoMark && theApp.activeDoc) pDoc->UpdateAllViews(NULL);
	} else if (isprint && theApp.isOpiMode && !pDlg->cancelGenEPS) {
		pDlg->ShowWindow(SW_HIDE);
		::MessageBox(NULL, _T("Kolumny zosta³y wys³ane do serwera OPI"), APP_NAME, MB_OK);
	} else if (isprint && d.m_format == F_EPS)
		theApp.isOpiMode = bInitOpiMode;

	CGenEpsInfoDlg::ReleaseGenEpsInfoDlg(pDlg);

} //CheckPrintEps

void CDrawView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CScrollView::OnRButtonDown(nFlags, point);
	CPoint local = point;
	ClientToDoc(local);
	CDrawDoc *vDoc = GetDocument();
	CDrawPage *pPage = vDoc->PageAt(local);
	if (!pPage) return;
	if (theApp.isRDBMS && vDoc->isGRB && !vDoc->isRO) {
		CMenu mmutczas;
		Select(pPage, FALSE);
		ClientToScreen(&point);
		mmutczas.LoadMenu(IDR_MENUMUTCZAS);
		mmutczas.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, this);
	} else vDoc->DerivePages(pPage);
}

BOOL CDrawView::ModifyMutczas(int n) {
	auto pPage = (CDrawPage*)m_selection.GetHead();
	if (pPage) {
		if (pPage->m_mutczas == 1 && n == -1) return TRUE;
		pPage->m_mutczas += n;	
		int pos = pPage->m_dervinfo.ReverseFind('a');
		CString cs;
		cs.Format(_T("%s %i)"), pPage->m_dervinfo.Left(++pos), pPage->m_mutczas);
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
	CString eps_name = dynamic_cast<CDrawAdd*>(m_selection.GetHead())->EpsName(NULL, F_EPS, FALSE);
	if (eps_name.Mid(1, 1) != _T(":")) { AfxMessageBox(_T("Preview niedostêpne. Brak materia³u")); return;}
	CString irfDir = theApp.GetProfileString(_T("GenEPS"), _T("EpsIrfan"), _T(""));
	if (irfDir.IsEmpty()) AfxMessageBox(_T("Ustaw œcie¿kê do IrfanView"));
	else _wspawnl(_P_DETACH, irfDir + _T("\\i_view32"), _T("i_view32"), eps_name, NULL);
}

void CDrawView::OnPrevKolumnaDruk() // single Page selected
{
	CString sURL;
	auto pPage = dynamic_cast<CDrawPage*>(m_selection.GetHead());
	int iNrPorz = theApp.activeDoc->GetIPage(pPage);
	sURL.Format(_T("tyt=%s&mut=%s&kiedy=%s&nrstrony=%i&format=pdf&rozmiar=1"), theApp.activeDoc->gazeta.Left(3), theApp.activeDoc->gazeta.Mid(4,2), theApp.activeDoc->dayws, iNrPorz==0 ? (int)theApp.activeDoc->m_pages.size() : iNrPorz);
	theApp.OpenWebBrowser(4, sURL);
}

void CDrawView::OnPrevPdf() // single Add or Page selected
{
	auto pAdd = dynamic_cast<CDrawAdd*>(m_selection.GetHead());
	if (!pAdd) { 
		OnPrevKolumnaDruk();
		return;
	} else if (pAdd->nreps < MIN_VALID_ADNO) {
		AfxMessageBox(_T("Brak prawid³owego numeru atexowego"));
		return;
	}

	CString sUrl;
	auto doc = GetDocument();
	if (pAdd->powtorka == 0 || pAdd->powtorka > CTime::GetCurrentTime()) { // nowe lub powtórka niearchiwalna
		int status = pAdd->nreps;
		CString sOrgDate = GetDocument()->data;
		sUrl = theManODPNET.GetHttpSource(GetDocument()->gazeta, GetDocument()->data, &status);

		if (!sUrl.IsEmpty()) {
			if (status < 0) { // dane z qfiltr_material lub z archiwum
				if (status < -1)
					AfxMessageBox(_T("Ten materia³ nie zosta³ dopuszczony do druku przez EpsTest"));
				if (sUrl.Find(_T('?')) < 0) 
					sUrl.AppendFormat(_T("?date=%s&nreps=%li&name=%s"), doc->dayws, pAdd->nreps, doc->gazeta.Left(3) + doc->gazeta.Mid(4, 2));
			}
		} else {
			int d, m, r;
			_stscanf_s(doc->data, c_formatDaty, &d, &m, &r);
			CTime tEdycja(r, m, d, 0, 0, 0);
			if (tEdycja > CTime::GetCurrentTime()) {
				AfxMessageBox(doc->symWydawcy == _T("-") ? _T("Brak zsy³aj¹cego") : _T("Brak materia³u"));
				return;
			} else
				sUrl.Format(IDS_ARCH_URL, tEdycja.Format(c_ctimeDataWs), pAdd->nreps);
		}
	} else // powtorka jest pokazywana z katalogu docelowego
		sUrl.Format(IDS_ARCH_URL, doc->dayws, pAdd->nreps);
	
	if (sUrl.IsEmpty())
		AfxMessageBox(_T("Brak materia³u"));
	else
		theApp.OpenWebBrowser(sUrl);
}

void CDrawView::OnPrevDig()
{
	auto pAdd = dynamic_cast<CDrawAdd*>(m_selection.GetHead());
	if (pAdd->nreps < MIN_VALID_ADNO) {
		AfxMessageBox(_T("Brak prawid³owego numeru atexowego"));
		return;
	}

	UINT iXmlLen = 0;
	CString dataSepia = GetDocument()->data;
	auto buf = reinterpret_cast<char*>(theApp.bigBuf);
	dataSepia = dataSepia.Mid(6) + "-" + dataSepia.Mid(3, 2) + "-" + dataSepia.Mid(0, 2);
	CString sURL, sUrlTemplate = _T("adno=%li&kiedy=%s");
	sURL.Format(sUrlTemplate, pAdd->nreps, dataSepia);
	auto pFile = theApp.OpenURL(5, sURL);
	if (pFile) {
		iXmlLen = pFile->Read(buf, bigSize);
		pFile->Close();
		if (iXmlLen == bigSize) {
			AfxMessageBox(_T("Plik z dodatkow¹ treœci¹ jest za du¿y"));
			return;
		}
	}

	const char *kon = nullptr, *pocz = CManPDF::memstr(buf, "<artykul", bigSize);
	if (pocz) kon = 10 + CManPDF::memstr(pocz, "</artykul>", bigSize - (pocz - buf));

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
			digContent.Open(digCntFilename, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive);
			if (digContent.m_hFile == INVALID_HANDLE_VALUE) continue;
			digContent.Write(pocz, (UINT)(kon - pocz));
			digContent.Close();
			break;
		} catch (CFileException* fe) {
			fe->Delete(); // ignore, try next name
		}
	}

	::ShellExecute(::GetDesktopWindow(), _T("Edit"), digCntFilename, NULL, NULL, SW_SHOWNORMAL);

	if (GetLastError() > 0)
		AfxMessageBox(_T("Brak skojarzenia z .xmlw"), MB_ICONERROR);
}

void CDrawView::OnAtexSyg() //single Add selected
{
	auto pAdd = dynamic_cast<CDrawAdd*>(m_selection.GetHead());
	if (pAdd->nreps < MIN_VALID_ADNO) {
		AfxMessageBox(_T("Brak prawid³owego numeru atexowego"));
		return;
	}

	CString sUrl;
	sUrl.Format(_T("t=%s&m=%s&k=%s&a=%li"), GetDocument()->gazeta.Left(3), pAdd->flags.derived ? "RP" : GetDocument()->gazeta.Mid(4,2), GetDocument()->data, pAdd->nreps);
	theApp.OpenWebBrowser(7, sUrl);
}

void CDrawView::OnPrevKor() 
{
	if (this->m_selection.GetCount() == 1) {
		auto pPage = dynamic_cast<CDrawPage*>(m_selection.GetHead());
		if (pPage) {
			CString num, fname; /*0925dlowa001-1ake.pdf*/
			CDrawDoc *pDoc = GetDocument();
			int lnr_porz = pDoc->GetIPage(pPage);
			num.Format(_T("%03i"), lnr_porz ? lnr_porz : (int)pDoc->m_pages.size());
			GENEPSARG genEpsArg;
			genEpsArg.cBigBuf = theApp.bigBuf;
			genEpsArg.bIsPRN = TRUE;
			genEpsArg.bDoKorekty = TRUE;
			pPage->GetDestName(&genEpsArg, num, fname);
			lnr_porz = fname.ReverseFind('\\');
			if (lnr_porz > 0)
				fname = fname.Mid(lnr_porz);
			lnr_porz = fname.ReverseFind('.');
			if (lnr_porz > 0)
				fname = fname.Left(lnr_porz + 1);
			fname = theApp.GetProfileString(_T("GenEPS"), _T("KorektaDobre"), _T("")) + pDoc->daydir + pDoc->gazeta.Left(3) + pDoc->gazeta.Mid(4,2) + fname + "pdf";

			theApp.OpenWebBrowser(fname);
		}
	}
}
