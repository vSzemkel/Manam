
#include "pch.h"
#include "DrawDoc.h"
#include "DrawView.h"
#include "GridFrm.h"
#include "Manam.h"
#include "QueView.h"
#include "SplitFrm.h"

extern BOOL disableMenu;

/////////////////////////////////////////////////////////////////////////////
// CSplitFrame

IMPLEMENT_DYNCREATE(CSplitFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CSplitFrame, CMDIChildWndEx)
    //{{AFX_MSG_MAP(CSplitFrame)
    ON_WM_SIZE()
    ON_WM_MDIACTIVATE()
    ON_COMMAND(IDM_SHOWDTLS, &CSplitFrame::OnShowDtls)
    ON_UPDATE_COMMAND_UI(IDM_SHOWDTLS, &CSplitFrame::OnUpdateShowDtls)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CSplitFrame::OnCreateClient(LPCREATESTRUCT cs, CCreateContext* pContext)
{
    BOOL bSuccess;
    int cx = cs->cx, cy = cs->cy;
    bSuccess = m_Splitter.CreateStatic(this, 1, 2, WS_CHILD | WS_VISIBLE, AFX_IDW_PANE_FIRST);
    bSuccess &= m_Splitter2.CreateStatic(&m_Splitter, 2, 1, WS_CHILD | WS_VISIBLE | WS_BORDER, m_Splitter.IdFromRowCol(0, 0));
    bSuccess &= m_Splitter2.CreateView(0, 0, pContext->m_pNewViewClass, CSize(cx, cy), pContext);
    bSuccess &= m_Splitter2.CreateView(1, 0, CQueView::GetThisClass(), CSize(cx, 0), pContext);
    bSuccess &= m_Splitter.CreateView(0, 1, CGridFrm::GetThisClass(), CSize(0, cy), pContext);

    m_Splitter.SetColumnInfo(0, cx, 0);
    m_Splitter.RecalcLayout();

    return bSuccess;
}

void CSplitFrame::ResizeQueView()
{
    int cy, w;

    m_Splitter2.GetRowInfo(1, cy, w);
    if (!cy) { // nie widac kolejki
        m_Splitter2.GetRowInfo(0, cy, w);
        w = cy - (((CDrawDoc*)GetActiveDocument())->m_addsque.empty() ? 0 : NONEMPTY_QUEUE_HEIGHT);
        if (w > 0 && w < cy) {
            m_Splitter2.SetRowInfo(0, w, 0);
            m_Splitter2.RecalcLayout();
        }
    }

    m_initialized = true;
}

/////////////////////////////////////////////////////////////////////////////
// CSplitFrame message handlers

void CSplitFrame::OnMDIActivate(const BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{

    theApp.activeDoc = bActivate ? (CDrawDoc*)GetActiveDocument() : nullptr;

    CMDIChildWndEx::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

    if (bActivate) ResizeQueView();
}


void CSplitFrame::OnShowDtls()
{
    auto pView = reinterpret_cast<CDrawView*>(m_Splitter2.GetPane(0, 0));
    pView->OpenSelected();
}

void CSplitFrame::OnUpdateShowDtls(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!disableMenu && ((CDrawView*)m_Splitter2.GetPane(0, 0))->m_selection.size() == 1);
}

void CSplitFrame::OnSize(const UINT nType, const int cx, const int cy)
{
    CMDIChildWndEx::OnSize(nType, cx, cy);

    static UINT uiLastType = UINT_MAX;

    if (nType != uiLastType && (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED) && m_initialized && theApp.activeDoc) {
        CRect r;
        GetClientRect(&r);
        m_Splitter2.SetRowInfo(0, r.Height() - (theApp.activeDoc->m_addsque.empty() ? 0 : NONEMPTY_QUEUE_HEIGHT + 9), 0);
        m_Splitter2.RecalcLayout();
        uiLastType = nType;
    }
}
