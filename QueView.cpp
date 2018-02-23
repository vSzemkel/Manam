
#include "StdAfx.h"
#include "DrawAdd.h"
#include "DrawDoc.h"
#include "DrawView.h"
#include "MainFrm.h"
#include "Manam.h"
#include "QueView.h"

#define MARGIN 4

CDrawAdd *CQueView::selected_add = nullptr;

/////////////////////////////////////////////////////////////////////////////
// CQueView

IMPLEMENT_DYNCREATE(CQueView, CScrollView)

CQueView::CQueView() : moving(FALSE)
{
}

BEGIN_MESSAGE_MAP(CQueView, CScrollView)
    //{{AFX_MSG_MAP(CQueView)
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_LBUTTONDOWN()
    ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
    ON_WM_LBUTTONUP()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQueView drawing

void CQueView::OnInitialUpdate()
{
    CScrollView::OnInitialUpdate();
    SetScrollSizes(MM_TEXT, CSize(100, 100));
}

void CQueView::OnUpdate(CView *, LPARAM lHint, CObject *)
{
    switch (lHint) {
        case HINT_UPDATE_WINDOW: // redraw entire window
            Invalidate(FALSE);
            break;
    }
}

void CQueView::OnPrepareDC(CDC *pDC, CPrintInfo *pInfo)
{
    CScrollView::OnPrepareDC(pDC, pInfo);
    pDC->SetMapMode(MM_ANISOTROPIC);
    pDC->SetViewportExt(CSize(100, 100));
    pDC->SetWindowExt(CSize(100 * vscale, -100 * vscale));
}

void CQueView::OnDraw(CDC *pDC)
{
    CDrawDoc *pDoc = GetDocument();
    ASSERT_VALID(pDoc);

    CBitmap bitmap;
    CBitmap *pOldBitmap;

    // only paint the rect that needs repainting
    CRect client;
    pDC->GetClipBox(client);
    CRect rect;
    GetClientRect(&rect);

    CDC bitmapDC;
    auto pDrawDC = pDC;
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

    // paint background
    CBrush brush;
    if (!brush.CreateStockObject(WHITE_BRUSH))
        return;

    pDrawDC->FillRect(client, &brush);

    pDoc->DrawQue(pDrawDC, this);

    if (pDrawDC != pDC) {
        pDC->SetMapMode(MM_TEXT);
        bitmapDC.SetMapMode(MM_TEXT);
        pDC->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), pDrawDC, 0, 0, SRCCOPY);
        bitmapDC.SelectObject(pOldBitmap);
    }
}

void CQueView::DocToClient(CRect *rect)
{
    CClientDC dc(this);
    OnPrepareDC(&dc, NULL);
    dc.LPtoDP(rect);
    rect->NormalizeRect();
}

void CQueView::ClientToDoc(CPoint *point)
{
    CClientDC dc(this);
    OnPrepareDC(&dc, NULL);
    dc.DPtoLP(point);
}

void CQueView::ClientToDoc(CRect *rect)
{
    CClientDC dc(this);
    OnPrepareDC(&dc, NULL);
    dc.DPtoLP(rect);
}

void CQueView::DocToClient(CPoint *point)
{
    CClientDC dc(this);
    OnPrepareDC(&dc, NULL);
    dc.LPtoDP(point);
}

void CQueView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    ClientToDoc(&point);
    CDrawAdd *pObj = GetDocument()->ObjectAtQue(point);
    if (pObj)
        pObj->OnOpen(NULL);
    else {
        GetDocument()->ArrangeQue();
        GetDocument()->GetPanelView<CQueView>()->Invalidate(FALSE);
        CScrollView::OnLButtonDblClk(nFlags, point);
    }
}

void CQueView::OnLButtonDown(UINT nFlags, CPoint point)
{
    ClientToDoc(&point);
    CDrawAdd *pObj = GetDocument()->ObjectAtQue(point);
    if (!pObj) {
        if (selected_add) {
            CRect rect = selected_add->m_position;
            selected_add = nullptr;
            RepaintRect(&rect);
        }
        return;
    }

    if (selected_add) vPos = selected_add->m_position;
    selected_add = pObj;
    RepaintRect(&vPos);
    vPoint = point;
    vPos = selected_add->m_position;
    moving = TRUE;
    RepaintRect(&vPos);
    SetCapture();
    CScrollView::OnLButtonDown(nFlags, point);
}

void CQueView::OnMouseMove(UINT nFlags, CPoint point)
{
    CScrollView::OnMouseMove(nFlags, point);
    ClientToDoc(&point);

    if (moving && selected_add) {
        RepaintRect(&selected_add->m_position);
        CRect screen;
        GetClientRect(&screen);
        ClientToDoc(&screen);
        if (point.x < screen.left || point.x > screen.right ||
            point.y > screen.top || point.y < screen.bottom) { // out of view mouse movement
            ReleaseCapture();
            theApp.unQueing = TRUE;
            moving = FALSE;
            selected_add->m_position = vPos;
            Invalidate(FALSE);
        } else
            selected_add->m_position += CPoint(point.x - vPoint.x, point.y - vPoint.y);
        RepaintRect(&selected_add->m_position);
        vPoint = point;
        return;
    }
    if (theApp.unQueing && selected_add) {
        RepaintRect(&selected_add->m_position);
        theApp.unQueing = FALSE;
    }

    CDrawObj *pObj = GetDocument()->ObjectAtQue(point);
    ((CMainFrame *)AfxGetMainWnd())->SetStatusBarInfo(pObj ? pObj->info : "");
}

void CQueView::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (moving) {
        if (selected_add && selected_add->m_position != vPos) {
            CRect rect = selected_add->m_position;
            selected_add->m_position = vPos;
            RepaintRect(&rect);
            RepaintRect(&selected_add->m_position);
        }
        moving = theApp.unQueing = FALSE;
    }
    ReleaseCapture();
    CScrollView::OnLButtonUp(nFlags, point);
}

void CQueView::RepaintRect(CRect *rect)
{ // indirect LPtoDP
    InvalidateRect(CRect(rect->left / vscale - MARGIN, rect->top / vscale - MARGIN, rect->right / vscale + MARGIN, -rect->bottom / vscale + MARGIN), FALSE);
}

void CQueView::OnEditClear()
{
    if (selected_add && AfxMessageBox(_T("Czy usun¹æ wybrane og³oszenie z kolejki"), MB_YESNO) == IDYES) {
        GetDocument()->m_del_obj.emplace_back(EntityType::addque, selected_add->m_pub_xx);
        auto& q = GetDocument()->m_addsque;
        q.erase(std::find(std::begin(q), std::end(q), CQueView::selected_add));
        GetDocument()->ArrangeQue();
        GetDocument()->SetModifiedFlag(TRUE);
        Invalidate(TRUE);
    }
}
