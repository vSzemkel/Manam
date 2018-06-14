
#include "StdAfx.h"
#include "DrawAdd.h"
#include "DrawDoc.h"
#include "DrawOpis.h"
#include "DrawPage.h"
#include "DrawTool.h"
#include "DrawView.h"
#include "MainFrm.h"
#include "Manam.h"
#include "QueView.h"
#include "Spacer.h"

/////////////////////////////////////////////////////////////////////////////
// CDrawTool implementation

extern BOOL drawErrorBoxes;

std::vector<CDrawTool*> CDrawTool::c_tools;

static CSelectTool selectTool;
static CRectTool addTool(DrawShape::add);
static CRectTool opisTool(DrawShape::opis);
static CKolorTool kolorTool(DrawShape::color);
static CKolorTool captionTool(DrawShape::caption);
static CLockTool lockTool(DrawShape::lock);
static CLockTool deadlineTool(DrawShape::deadline);
static CSpaceTool spaceTool(DrawShape::space);
static CSpaceTool spaceTool2(DrawShape::red);

CPoint CDrawTool::c_down;
CPoint CDrawTool::c_last;
UINT CDrawTool::c_nDownFlags;
int CSelectTool::m_DragHandle = 1;
DrawShape CDrawTool::c_drawShape = DrawShape::select;

CDrawTool::CDrawTool(DrawShape drawShape)
{
    m_drawShape = drawShape;
    c_tools.push_back(this);
}

CDrawTool* CDrawTool::FindTool(DrawShape drawShape)
{
    for (const auto& t : c_tools)
        if (t->m_drawShape == drawShape)
            return t;

    return nullptr;
}

void CDrawTool::OnLButtonDown(CDrawView *pView, UINT nFlags, const CPoint& point)
{
    // deactivate any in-place active item on this view!
    COleClientItem* pActiveItem = pView->GetDocument()->GetInPlaceActiveItem(pView);
    if (pActiveItem != nullptr) {
        pActiveItem->Close();
        ASSERT(pView->GetDocument()->GetInPlaceActiveItem(pView) == nullptr);
    }

    pView->SetCapture();
    c_nDownFlags = nFlags;
    c_down = point;
    c_last = point;
}

void CDrawTool::OnLButtonDblClk(CDrawView *pView, UINT nFlags, const CPoint& point)
{
    if ((nFlags & MK_SHIFT) != 0) {
        // Shift+DblClk deselects object...
        CPoint local = point;
        pView->ClientToDoc(local);
        CDrawObj *pObj = pView->GetDocument()->ObjectAt(local);
        if (pObj != nullptr)
            pView->Deselect(pObj);
    } else
        pView->OpenSelected();
}

void CDrawTool::OnLButtonUp(CDrawView*, UINT, const CPoint& point)
{
    ReleaseCapture();

    if (point == c_down)
        OnCancel();
}

void CDrawTool::OnMouseMove(CDrawView*, UINT, const CPoint& point)
{
    c_last = point;
}

void CDrawTool::OnCancel()
{
    c_drawShape = DrawShape::select;
    ::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
}

////////////////////////////////////////////////////////////////////////////
// CResizeTool

CPoint lastPoint;
SelectMode selectMode = SelectMode::none;

CSelectTool::CSelectTool() : CDrawTool(DrawShape::select)
{
}

void CSelectTool::OnLButtonDown(CDrawView *pView, UINT nFlags, const CPoint& point)
{
    CPoint local = point;
    pView->ClientToDoc(local);

    selectMode = SelectMode::none;

    // Check for resizing ads (only allowed on single selections)
    if (pView->m_selection.size() == 1) {
        const auto pObj = pView->m_selection.front();
        m_DragHandle = pObj->HitTest(local, pView, TRUE);
        if (m_DragHandle != 0) {
            selectMode = SelectMode::size;
            const auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
            if (pAdd) {
                if (pAdd->typ_xx || pAdd->flags.locked || pAdd->flags.derived)
                    selectMode = SelectMode::dontsize;
            } else if (!dynamic_cast<CDrawOpis*>(pObj))
                selectMode = SelectMode::dontsize;
        }
    }

    // See if the click was on an object, select and start move if so
    if (selectMode == SelectMode::none) {
        CDrawObj *pObj = pView->GetDocument()->ObjectAt(local);

        if (pObj != nullptr) {
            selectMode = SelectMode::move;

            if (!pView->IsSelected(pObj))
                pView->Select(pObj, (nFlags & MK_SHIFT) != 0);

            // Ctrl+Click clones the selection...
            if ((nFlags & MK_CONTROL) != 0)
                pView->CloneSelection();

            auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
            if (pAdd) {
                if (pAdd->flags.locked)
                    selectMode = SelectMode::dontmove;
                if (pAdd->fizpage)
                    pView->GetDocument()->m_pages[pView->GetDocument()->GetIPage(pAdd->fizpage)]->SetBaseKrata(pAdd->szpalt_x, pAdd->szpalt_y);
            }
        }
    }

    // Click on background, start a net-selection
    if (selectMode == SelectMode::none) {
        if ((nFlags & MK_SHIFT) == 0)
            pView->Select(nullptr);

        selectMode = SelectMode::netSelect;

        CClientDC dc(pView);
        m_last_rect.SetRect(point.x, point.y, point.x, point.y);
        dc.DrawFocusRect(m_last_rect);
    }

    if (pView->GetDocument()->isGRB || (theApp.grupa & UserRole::dea) > 0)
        selectMode = SelectMode::none;

    lastPoint = local;
    CDrawTool::OnLButtonDown(pView, nFlags, point);
}

void CSelectTool::OnLButtonUp(CDrawView *pView, UINT nFlags, const CPoint& point)
{
    CDrawObj *pObj = nullptr;

    if (pView->GetCapture() == pView && point != c_down) // czy kliknelismy w to samo miejsce
        switch (selectMode) {
            default:
                pView->GetDocument()->UpdateAllViews(pView);

            case SelectMode::netSelect:
            {
                CClientDC dc(pView);
                dc.DrawFocusRect(m_last_rect);
                pView->SelectWithinRect(m_last_rect, TRUE);
            }
            break;

            case SelectMode::move:
            {
                if (pView->m_selection.empty()) break;
                CDrawDoc *pDoc = pView->GetDocument();
                pObj = pView->m_selection.front();
                pObj->SetDirty();
                auto pPage = dynamic_cast<CDrawPage*>(pObj);
                if (pPage) { // strona
                    int prev_pos = pDoc->GetIPage(pPage);
                    while (prev_pos > 0 && pView->IsSelected(pDoc->m_pages[prev_pos - 1]))
                        pPage = pDoc->m_pages[--prev_pos];
                    const int new_pos = pDoc->ComputePageOrderNr(pPage->m_position);
                    if (prev_pos == new_pos)
                        pDoc->SetPageRectFromOrd(pPage, prev_pos);
                    else {
                        int iCnt = 1;
                        while (prev_pos + iCnt < (int)pDoc->m_pages.size() && pView->IsSelected(pDoc->m_pages[prev_pos + iCnt]))
                            iCnt++;
                        pDoc->MoveBlockOfPages(prev_pos, new_pos, iCnt);
                    }
                } else { // ogloszenie dopasowanie do gridu
                    auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
                    if (pAdd) {
                        const int szpalt_x = pAdd->szpalt_x;
                        const int szpalt_y = pAdd->szpalt_y;
                        const auto adCentr = pAdd->m_position.CenterPoint();
                        const auto srcPage = pDoc->GetPage(pAdd->fizpage);

                        CRect toPos;
                        auto dstPage = pDoc->PageAt(adCentr);
                        if (dstPage && (dstPage->szpalt_x != szpalt_x || dstPage->szpalt_y != szpalt_y)) {
                            if (srcPage) dstPage = srcPage;
                            toPos.SetRect(dstPage->m_position.left + (int)(modulx*(pAdd->posx - 1)), dstPage->m_position.bottom + (int)(moduly*(szpalt_y - pAdd->posy + 1)),
                                dstPage->m_position.left + (int)(modulx*(pAdd->posx + pAdd->sizex - 1)), dstPage->m_position.bottom + (int)(moduly*(szpalt_y - pAdd->posy - pAdd->sizey + 1)));
                            pAdd->MoveTo(toPos, pView);
                        } else {
                            if (!dstPage && pAdd->flags.derived) {
                                auto itAdd = std::find(srcPage->m_adds.cbegin(), srcPage->m_adds.cend(), pAdd);
                                if (itAdd != srcPage->m_adds.cend()) srcPage->m_adds.erase(itAdd);
                                pAdd->SetSpaceAndPosition(pAdd->fizpage, pAdd->posx, pAdd->posy);
                                toPos = pAdd->m_position;
                            } else {
                                if (dstPage)
                                    toPos = pAdd->m_position;
                                else {
                                    const int mx = min(pmodulx, (int)modulx);
                                    const int my = -min(pmoduly, (int)moduly);
                                    const auto x = (int)(mx * nearbyint((double)pAdd->m_position.left / mx));
                                    const auto y = (int)(my * nearbyint((double)pAdd->m_position.top / my));
                                    toPos.SetRect(x, y, x + pAdd->m_position.Width(), y + pAdd->m_position.Height());
                                }
                                pAdd->SetPosition(&toPos, dstPage);
                            }
                            pAdd->MoveTo(toPos, pView);
                        }

                        if (pAdd == CQueView::selected_add) { // z kolejki mo¿na ustawiæ tylko na stronê
                            const int xx = pAdd->m_pub_xx;
                            pAdd->m_pub_xx *= -1;
                            selectMode = SelectMode::none;
                            CDrawTool::OnLButtonUp(pView, nFlags, point);
                            if (pAdd->fizpage)
                                CSpacerDlg::Deal(pAdd);

                            if (!pAdd->fizpage || pAdd->m_pub_xx < 0) {
                                pAdd->m_pub_xx = xx;
                                if (dstPage) dstPage->RemoveAdd(pAdd);
                                pDoc->Remove(pAdd);
                                pDoc->AddQue(pAdd);
                                auto vView = pDoc->GetPanelView<CQueView>();
                                if (vView) pAdd->m_position = *vView->GetStoredPosition();
                            }
                            CQueView::selected_add = nullptr;
                            pView->m_bActive = FALSE;
                            pView->Invalidate(FALSE);
                            pDoc->UpdateAllViews(pView);
                            return;
                        }
                        pAdd->Invalidate();
                    }
                }
            }
            break;

            case SelectMode::size:
            {
                pObj = pView->m_selection.front();
                pObj->SetDirty();
                CRect toPos = pObj->m_position;
                toPos.NormalizeRect();
                auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
                if (pAdd) {
                    const int szpalt_x = pAdd->szpalt_x;
                    const int szpalt_y = pAdd->szpalt_y;
                    const int mx = min(pmodulx, (int)modulx);
                    const int my = -min(pmoduly, (int)moduly);
                    const auto x = (int)(mx * ceil((double)toPos.left / mx - 0.5));
                    const auto y = (int)(my * ceil((double)toPos.bottom / my - 0.5));
                    const auto cx = ceil((toPos.right - toPos.left) / modulx - 0.5);
                    const auto sx = (int)(cx == 0 ? modulx : cx*modulx);
                    const auto cy = ceil((toPos.top - toPos.bottom) / moduly - 0.5);
                    const auto sy = (int)(cy == 0 ? moduly : cy*moduly);
                    const CPoint& p = toPos.CenterPoint();
                    CDrawPage *vPage = pView->GetDocument()->PageAt(p);
                    toPos.SetRect(x, y, x + sx, y + sy);
                    pAdd->SetPosition(&toPos, vPage);
                } else {
                    if (toPos.right - toPos.left < pmodulx) toPos.right = toPos.left + pmodulx;
                    if (toPos.bottom - toPos.top < pmoduly) toPos.top = toPos.bottom - pmoduly;
                    std::swap(toPos.top, toPos.bottom);
                }

                pObj->MoveTo(toPos, pView);
            }
            break;

            case SelectMode::dontsize:
            case SelectMode::dontmove:
            case SelectMode::none:
                break;
        }

    if (selectMode == SelectMode::move || selectMode == SelectMode::size) {
        pView->Invalidate(FALSE);
        if (pObj) pObj->Invalidate();
    }

    selectMode = SelectMode::none;
    CDrawTool::OnLButtonUp(pView, nFlags, point);
}

void CSelectTool::OnMouseMove(CDrawView *pView, UINT, const CPoint& point)
{
    c_last = point;
    CDrawObj* pObj;
    CPoint l{point};
    pView->ClientToDoc(l);

    if (theApp.unQueing) { // wprowadzamy og³oszenie z widoku CQueView
        lastPoint = l;
        theApp.unQueing = FALSE;
        selectMode = SelectMode::move;
        CRect& rect = CQueView::selected_add->m_position;
        rect.SetRect(l.x - rect.Width() / 2, l.y - rect.Height() / 2, l.x + rect.Width() / 2, l.y + rect.Height() / 2);
        pView->GetDocument()->Add(CQueView::selected_add);
        pObj = CQueView::selected_add;
        pView->GetDocument()->RemoveQue(CQueView::selected_add);
    } else
        pObj = pView->GetDocument()->ObjectAt(l);

    ((CMainFrame*)AfxGetMainWnd())->SetStatusBarInfo(pObj ? pObj->info : "");

    if (pView->GetCapture() != pView) {
        if (c_drawShape == DrawShape::select && pView->m_selection.size() == 1) {
            pObj = pView->m_selection.front();
            CPoint local = point;
            pView->ClientToDoc(local);
            m_DragHandle = pObj->HitTest(local, pView, TRUE);
            if (m_DragHandle != 0 && (theApp.grupa & UserRole::dea) == 0)
                ::SetCursor(pObj->GetHandleCursor(m_DragHandle));
            else
                OnCancel();
        }
        return;
    }

    if (selectMode == SelectMode::none)
        return;

    if (selectMode == SelectMode::netSelect) {
        CClientDC dc(pView);
        dc.DrawFocusRect(m_last_rect);
        m_last_rect.SetRect(c_down.x, c_down.y, point.x, point.y);
        m_last_rect.NormalizeRect();
        dc.DrawFocusRect(m_last_rect);
        return;
    }

    for (const auto& s : pView->m_selection) {
        if (selectMode == SelectMode::move) {
            if (l != lastPoint) {
                const CPoint delta = l - lastPoint;
                s->MoveTo(s->m_position + delta, pView);
            }
        } else if (m_DragHandle != 0 && selectMode != SelectMode::dontsize)
            s->MoveHandleTo(m_DragHandle, l, pView);
    }

    if (selectMode == SelectMode::size && c_drawShape == DrawShape::select)
        SetCursor(pView->m_selection.front()->GetHandleCursor(m_DragHandle));

    lastPoint = l;
}

////////////////////////////////////////////////////////////////////////////
// CRectTool (does rectangles, round-rectangles, and ellipses)

CRectTool::CRectTool(DrawShape drawShape)
    : CDrawTool(drawShape)
{
}

void CRectTool::OnLButtonDown(CDrawView *pView, UINT nFlags, const CPoint& point)
{
    CDrawTool::OnLButtonDown(pView, nFlags, point);

    CRect r;
    CPoint local = point;
    pView->ClientToDoc(local);
    switch (m_drawShape) {
        case DrawShape::add:
        {
            auto pDoc = pView->GetDocument();
            auto pPage = pDoc->PageAt(local);
            if (pPage && pPage->m_dervlvl == DervType::proh) {
                pView->Select(nullptr);
                return;
            }
            r = CRect(local, CSize(0, 0));
            auto pAdd = new CDrawAdd(r);
            if (pPage || !pDoc->m_pages.empty()) {
                if (!pPage) pPage = pDoc->m_pages[0];
                pAdd->szpalt_x = pPage->szpalt_x;
                pAdd->szpalt_y = pPage->szpalt_y;
                if (pAdd->space.GetSize() != pPage->space.GetSize())
                    pAdd->space = CFlag(pPage->space.GetSize());
            }
            pDoc->Add(pAdd);
            pView->Select(pAdd);
            break;
        }
        case DrawShape::opis:
        {
            r = CRect(local, CSize(0, 0));
            auto pObj = new CDrawOpis(r);
            pView->GetDocument()->Add(pObj);
            pView->Select(pObj);
        }
    }

    selectMode = SelectMode::size;
    CSelectTool::m_DragHandle = 1;
    lastPoint = local;
}

void CRectTool::OnLButtonDblClk(CDrawView*, UINT, const CPoint&)
{
}

void CRectTool::OnLButtonUp(CDrawView *pView, UINT nFlags, const CPoint& point)
{
    if (point == c_down) {
        // Don't create empty objects...
        auto pObj = pView->m_selection.back();
        pView->Select(pObj, FALSE);
        pView->GetDocument()->Remove(pObj);
        pObj->Remove();
        selectTool.OnLButtonDown(pView, nFlags, point); // try a select!
    }

    selectTool.OnLButtonUp(pView, nFlags, point);
}

void CRectTool::OnMouseMove(CDrawView *pView, UINT nFlags, const CPoint& point)
{
    selectTool.OnMouseMove(pView, nFlags, point);
}


/////////////////////////////////////////////////////////////////////////////
CKolorTool::CKolorTool(DrawShape drawShape) : CDrawTool(drawShape)
{
}

void CKolorTool::OnLButtonDown(CDrawView* pView, UINT nFlags, const CPoint& point)
{
    CPoint local{point};
    pView->ClientToDoc(local);

    CDrawObj* pObj = pView->GetDocument()->ObjectAt(local);
    if (!pObj) return;

    auto frame = (CMainFrame*)AfxGetMainWnd();
    auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
    auto pPage = dynamic_cast<CDrawPage*>(pObj);
    if (pPage != nullptr && pPage->m_dervlvl == DervType::fixd) return;
    if (pAdd != nullptr && pAdd->flags.derived) return;
    pObj->SetDirty();
    if (!pView->IsSelected(pObj))
        pView->Select(pObj, (nFlags & MK_SHIFT) != 0);
    if (m_drawShape == DrawShape::color) {
        int kolorek = frame->GetKolor((int)pObj->m_pDocument->m_spot_makiety.size());
        if (kolorek == CB_ERR) return;
        if (pPage != nullptr) {
            if ((kolorek & ColorId::spot) == ColorId::spot || kolorek == ColorId::brak || kolorek == ColorId::full)
                pPage->ChangeKolor(kolorek);
            else
                pPage->SetSpotKolor(kolorek >> 3);
        } else { // CDrawAdd   czyli nic nie robimy gdy wybrany kolor np.spot I
            if (theApp.swCZV == ToolbarMode::tryb_studia && pAdd != nullptr) { //wersje_eps
                if (pAdd->wersja.IsEmpty() || pAdd->wersja == _T(" "))
                    pAdd->wersja = ("." + frame->GetKolorText());
                else {
                    int p = pAdd->wersja.Find(_T("."));
                    if (p < 0) pAdd->wersja += ("." + frame->GetKolorText());
                    else pAdd->wersja = pAdd->wersja.Left(p + 1) + frame->GetKolorText();
                }
                pAdd->Invalidate();
            } else // kolor
                if ((kolorek & ColorId::spot) == 0 || kolorek == ColorId::brak || kolorek == ColorId::full)
                    pObj->ChangeKolor(((kolorek == ColorId::full) ? ColorId::full : ((kolorek == ColorId::brak) ? ColorId::brak : (kolorek + ColorId::spot))));
        }
    } else  // caption tool
        if (theApp.swCZV == ToolbarMode::tryb_studia && pPage != nullptr) { // naglowki_prn
            pPage->prn_mak_xx = (int)frame->GetCaptionDataItem(-1) + (pPage->pagina & 1);
            CManODPNETParms orapar {
                { CManODPNET::DbTypeInt32, &pPage->m_pDocument->m_mak_xx },
                { CManODPNET::DbTypeInt32, &pPage->id_str },
                { CManODPNET::DbTypeInt32, &pPage->prn_mak_xx }
            };
            theManODPNET.EI("begin pagina.set_config(:mak_xx,:str_xx,:prn_mak_xx); end;", orapar);
        } else { // nag³ówki
            BOOL iscaption = ((CDrawApp*)AfxGetApp())->GetProfileInt(_T("General"), _T("Captions"), 1) == 1;
            if (pPage != nullptr)
                pPage->ChangeCaption(iscaption, frame->GetCaption());
            else if (pAdd != nullptr && pAdd->fizpage != 0)
                (pView->GetDocument()->GetPage(pAdd->fizpage))->ChangeCaption(iscaption, frame->GetCaption());
        }
}

void CKolorTool::OnMouseMove(CDrawView *pView, UINT, const CPoint& point)
{
    c_last = point;

    //..,,pokazywanie na status barze co to za obiekt
    CPoint l = point;
    pView->ClientToDoc(l);
    CDrawObj *pObj = pView->GetDocument()->ObjectAt(l);
    if (pObj != nullptr)
        ((CMainFrame*)AfxGetMainWnd())->SetStatusBarInfo(pObj->info);
    else
        ((CMainFrame*)AfxGetMainWnd())->SetStatusBarInfo((LPCTSTR)_T("ekran"));
}

void CKolorTool::OnLButtonUp(CDrawView*, UINT, const CPoint&)
{
}

/// LOCK TOOL
CLockTool::CLockTool(DrawShape drawShape)
    : CDrawTool(drawShape)
{
}

void CLockTool::OnLButtonDown(CDrawView *pView, UINT nFlags, const CPoint& point)
{
    CPoint local = point;
    pView->ClientToDoc(local);

    // See if the click was on an page
    CDrawObj *pObj = pView->GetDocument()->ObjectAt(local);
    if (pObj != nullptr) {
        if (!pView->IsSelected(pObj))
            pView->Select(pObj, (nFlags & MK_SHIFT) != 0);

        auto pPage = dynamic_cast<CDrawPage*>(pObj);
        if (m_drawShape == DrawShape::lock) {
            if (pPage != nullptr) {
                pPage->niemakietuj = !pPage->niemakietuj;
                pPage->SetDirty(); pPage->Invalidate();
            } else {
                auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
                if (pAdd != nullptr)
                    pAdd->Lock();
            }
        } else if (pPage != nullptr) { // DrawShape::deadline
            const CRect r(local, CSize((int)(2.7 * pmodulx), -pmoduly));
            auto pOpi = new CDrawOpis(r, theApp.GetProfileString(_T("General"), _T("Deadline"), _T("")));
            pView->GetDocument()->Add(pOpi);
            pView->Select(pOpi);
        }
    }
}

void CLockTool::OnMouseMove(CDrawView *pView, UINT, const CPoint& point)
{
    c_last = point;
    // pokazywanie na status barze co to za obiekt
    CPoint l = point;
    pView->ClientToDoc(l);
    CDrawObj *pObj = pView->GetDocument()->ObjectAt(l);
    if (pObj != nullptr)
        ((CMainFrame*)AfxGetMainWnd())->SetStatusBarInfo(pObj->info);
    else
        ((CMainFrame*)AfxGetMainWnd())->SetStatusBarInfo((LPCTSTR)_T("ekran"));
}

SpaceMode spaceMode = SpaceMode::avail;

CSpaceTool::CSpaceTool(DrawShape dsh)
    : CDrawTool(dsh) //vu - by³o 'space'
{
}

void CSpaceTool::OnLButtonDown(CDrawView *pView, UINT nFlags, const CPoint& point)
{
    spaceMode = (CDrawTool::c_drawShape == DrawShape::red ? SpaceMode::redlock : SpaceMode::spacelock);
    CClientDC dc(pView);
    CRect rect(point.x, point.y, point.x, point.y);
    rect.NormalizeRect();
    dc.DrawFocusRect(rect);
    CDrawTool::OnLButtonDown(pView, nFlags, point);
}

void CSpaceTool::OnMouseMove(CDrawView *pView, UINT, const CPoint& point)
{
    //..,,pokazywanie na status barze co to za obiekt
    CPoint l = point;
    pView->ClientToDoc(l);
    CDrawObj *pObj = pView->GetDocument()->ObjectAt(l);
    if (pObj != nullptr)
        ((CMainFrame*)AfxGetMainWnd())->SetStatusBarInfo(pObj->info);
    else
        ((CMainFrame*)AfxGetMainWnd())->SetStatusBarInfo((LPCTSTR)_T("ekran"));

    if (spaceMode != SpaceMode::avail) {
        CClientDC dc(pView);
        CRect rect(c_down.x, c_down.y, c_last.x, c_last.y);
        rect.NormalizeRect();
        dc.DrawFocusRect(rect);
        rect.SetRect(c_down.x, c_down.y, point.x, point.y);
        rect.NormalizeRect();
        dc.DrawFocusRect(rect);
        c_last = point;
    }
}

void CSpaceTool::OnLButtonUp(CDrawView *pView, UINT nFlags, const CPoint& point)
{
    if (pView->GetCapture() == pView && point != c_down && spaceMode != SpaceMode::avail) { // czy kliknelismy w to samo miejsce
        CClientDC dc(pView);
        CRect rect(c_down.x, c_down.y, c_last.x, c_last.y);
        rect.NormalizeRect();
        dc.DrawFocusRect(rect);
        CPoint l1 = c_down;
        pView->ClientToDoc(l1);
        CPoint l2 = c_last;
        pView->ClientToDoc(l2);
        CRect rInter, rPage, rDocSelection(min(l1.x, l2.x), min(l1.y, l2.y), max(l1.x, l2.x), max(l1.y, l2.y));
        for (const auto& pObj : theApp.activeDoc->m_pages) {
            rPage = pObj->m_position;
            rPage.NormalizeRect();
            if (rInter.IntersectRect(rPage, rDocSelection)) {
                const auto ilemod = static_cast<size_t>(pObj->szpalt_x * pObj->szpalt_y);
                for (size_t module = 0; module < ilemod; ++module) {
                    const CRect& rMod = pObj->GetNormalizedModuleRect(module);
                    if (rInter.IntersectRect(rDocSelection, rMod))
                        pObj->ChangeMark(module, spaceMode);
                }
            }
        }
    }
    CDrawTool::OnLButtonUp(pView, nFlags, point);
    spaceMode = SpaceMode::avail;
}
