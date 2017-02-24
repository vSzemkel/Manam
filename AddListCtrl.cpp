
#include "stdafx.h"
#include "addlistctrl.h"
#include "drawadd.h"
#include "drawdoc.h"
#include "gridfrm.h"

const int CAddListCtrl::iWheelUnitY = 3 * ROWHEIGHT;

IMPLEMENT_DYNAMIC(CAddListCtrl, CMFCListCtrl)

CAddListCtrl::CAddListCtrl(CGridFrm *pView) : m_pContainer(pView)
{
}

BEGIN_MESSAGE_MAP(CAddListCtrl, CMFCListCtrl)
    ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

void CAddListCtrl::InitHeader()
{
    CMFCListCtrl::InitHeader();

    InsertColumn(lp, _T("Lp."), LVCFMT_RIGHT, 50);
    InsertColumn(logiczna, _T("Miejsce"), LVCFMT_RIGHT, 60);
    InsertColumn(zamowienie, _T("Zamówienie"), LVCFMT_LEFT, 75);
    InsertColumn(kod, _T("Kod"), LVCFMT_LEFT, 75);
    InsertColumn(rozmiar, _T("Rozmiar"), LVCFMT_RIGHT, 60);
    InsertColumn(nazwa, _T("Nazwa"), LVCFMT_LEFT, 130);
    InsertColumn(strona, _T("Strona"), LVCFMT_RIGHT, 45);
    InsertColumn(kolorek, _T("Kolor"), LVCFMT_RIGHT, 62);
    InsertColumn(uwagi, _T("Uwagi"), LVCFMT_LEFT, 165);
    InsertColumn(powtorka, _T("Powtórka"), LVCFMT_RIGHT, 85);
    InsertColumn(oldadno, _T("z ATEX"), LVCFMT_RIGHT, 75);
    InsertColumn(studio, _T("Studio"), LVCFMT_RIGHT, 85);

    SetBkColor(BIALY);
    SetTextBkColor(RGB(250, 250, 250));
    SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    m_SmallImageList.Create(IDB_ZNACZKI, 16, 1, BIALY);
    SetImageList(&m_SmallImageList, LVSIL_SMALL);
}

int CAddListCtrl::OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn)
{
    bool bResult = false;
    auto a1 = reinterpret_cast<CDrawAdd*>(lParam1);
    auto a2 = reinterpret_cast<CDrawAdd*>(lParam2);

    switch (iColumn)
		case lp:
    {
        CGridFrm::eLastOrder = GridSortCol::lp;
        bResult = a1->m_pDocument->GetAddPosition(a1) < a1->m_pDocument->GetAddPosition(a2);
        break;
        case logiczna:
            CGridFrm::eLastOrder = GridSortCol::logiczna;
            return _tcsicmp(a1->logpage, a2->logpage);
        case zamowienie:
            CGridFrm::eLastOrder = GridSortCol::zamowienie;
            bResult = a1->nreps < a2->nreps;
            break;
        case kod:
            return _tcsicmp(a1->kodModulu, a2->kodModulu);
        case rozmiar:
            CGridFrm::eLastOrder = GridSortCol::rozmiar;
            bResult = a1->sizex < a2->sizex || (a1->sizex == a2->sizex && a1->sizey < a2->sizey);
            break;
        case nazwa:
            CGridFrm::eLastOrder = GridSortCol::nazwa;
            return _tcsicmp(a1->nazwa, a2->nazwa);
        case strona:
            CGridFrm::eLastOrder = GridSortCol::strona;
            bResult = a1->fizpage >> 3 < a2->fizpage >> 3;
            break;
        case kolorek:
            bResult = a1->kolor < a2->kolor;
            break;
        case uwagi:
            return _tcsicmp(a1->remarks.IsEmpty() ? a1->remarks_atex : a1->remarks, a2->remarks.IsEmpty() ? a2->remarks_atex : a2->remarks);
        case powtorka:
            bResult = a1->powtorka < a2->powtorka;
            break;
        case oldadno:
            return m_pContainer->showLastAdnoUsed ? _tcsicmp(a1->skad_ol, a2->skad_ol) : (a1->oldAdno < a2->oldAdno ? -1 : 1);
        case studio:
            bResult = a1->flags.studio < a2->flags.studio;
            break;
    }

    return bResult ? -1 : 1;
}

BOOL CAddListCtrl::OnMouseWheel(UINT, short zDelta, CPoint)
{
    if (m_pContainer == nullptr)
        return FALSE;

    BOOL bHasHorzBar, bHasVertBar;
    m_pContainer->CheckScrollBars(bHasHorzBar, bHasVertBar);
    if (!bHasVertBar)
        return FALSE;

    CPoint p = m_pContainer->GetScrollPosition();
    p.y -= zDelta / WHEEL_DELTA*iWheelUnitY;
    m_pContainer->ScrollToPosition(p);
    return TRUE;
}
