
#include "StdAfx.h"
#include "DrawDoc.h"
#include "DrawView.h"
#include "KolDlg.h"
#include "MainFrm.h"
#include "Manam.h"

extern BOOL drawErrorBoxes;

IMPLEMENT_SERIAL(CDrawObj, CObject, 0)

CDrawObj::CDrawObj() noexcept
{
}

CDrawObj::CDrawObj(const CRect& position) noexcept : CDrawObj::CDrawObj()
{
    m_position = position;
}

void CDrawObj::DrawNapis(CDC* pDC, LPCTSTR napis, int cnt, LPRECT r, UINT format, int bkMode)
{
    const auto saved = pDC->SetBkMode(bkMode);
    if (vscale == 1) 
        pDC->DrawText(napis, cnt, r, format);
    else {
        CRect lRect = r;
        const CSize& initExt = pDC->SetWindowExt(CSize(100, -100));
        lRect.top /= vscale; lRect.bottom /= vscale; lRect.left /= vscale; lRect.right /= vscale;
        pDC->DrawText(napis, cnt, lRect, format);
        pDC->SetWindowExt(initExt);
    }
    pDC->SetBkMode(saved);
}

inline int CDrawObj::GetVertPrintShift() const
{
    return PRINT_VOFFSET - (int)(2 * pmoduly * floor((float)m_position.top / CLIENT_SCALE / pmoduly / 40 / theApp.colsPerPage));
}

CRect CDrawObj::GetPrintRect() const
{
    return {m_position.left / CLIENT_SCALE,
        m_position.top / CLIENT_SCALE - GetVertPrintShift(),
        m_position.right / CLIENT_SCALE,
        m_position.bottom / CLIENT_SCALE - GetVertPrintShift()};
}

void CDrawObj::Serialize(CArchive& ar)
{
    CObject::Serialize(ar);

    if (ar.IsStoring()) {
        ar << m_position;
        ar << kolor;
        ar << info;
    } else {
        // get the document back pointer from the archive
        m_pDocument = (CDrawDoc*)ar.m_pDocument;
        ASSERT_VALID(m_pDocument);
        ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDrawDoc)));

        ar >> m_position;
        ar >> kolor;
        ar >> info;
        dirty = TRUE;
    }
}

void CDrawObj::SetDirty()
{
    dirty = TRUE;
    m_pDocument->SetModifiedFlag();
}

void CDrawObj::Remove()
{
    delete this;
}

void CDrawObj::DrawTracker(CDC* pDC, TrackerState state) const
{
    ASSERT_VALID(this);

    switch (state) {
        case normal:
            break;

        case selected:
        case active:
        {
            const int nHandleCount = GetHandleCount();
            for (int nHandle = 1; nHandle <= nHandleCount; ++nHandle) {
                const int scale = 18;
                CPoint handle = GetHandle(nHandle);

                pDC->PatBlt(handle.x - 2 * scale, handle.y - 2 * scale, 4 * scale, 4 * scale, BLACKNESS);
            }
        }
        break;
    }
}

// position is in logical
void CDrawObj::MoveTo(const CRect& position, CDrawView* pView)
{
    ASSERT_VALID(this);

    if (position == m_position)
        return;

    if (pView == nullptr) {
        Invalidate();
        m_position = position;
        Invalidate();
    } else {
        pView->InvalObj(this);
        m_position = position;
        pView->InvalObj(this);
    }
    m_pDocument->SetModifiedFlag();
}


// Note: if bSelected, hit-codes start at one for the top-left
// and increment clockwise, 0 means no hit.
// If !bSelected, 0 = no hit, 1 = hit (anywhere)

// point is in logical coordinates
int CDrawObj::HitTest(const CPoint& point, CDrawView* pView, const bool selected) const
{
    ASSERT_VALID(this);
    ASSERT(pView != nullptr);

    if (selected) {
        const int nHandleCount = GetHandleCount();
        for (int nHandle = 1; nHandle <= nHandleCount; ++nHandle) {
            // GetHandleRect returns in logical coords
            const CRect rc = GetHandleRect(nHandle, pView);
            if (rc.PtInRect(point))
                return nHandle;
        }
    } else {
        if (point.x >= m_position.left && point.x < m_position.right &&
            point.y <= m_position.top && point.y > m_position.bottom)
            return 1;
    }
    return 0;
}

// rect must be in logical coordinates
bool CDrawObj::Intersects(const CRect& rect) const
{
    ASSERT_VALID(this);

    CRect fixed = m_position;
    fixed.NormalizeRect();
    CRect rectT = rect;
    rectT.NormalizeRect();
    return !(rectT & fixed).IsRectEmpty();
}

bool CDrawObj::Contains(const CPoint& point) const
{
    CRect fixed = m_position;
    fixed.NormalizeRect();
    return fixed.PtInRect(point);
}

int CDrawObj::GetHandleCount() const
{
    ASSERT_VALID(this);
    return 8;
}

// returns logical coords of center of handle
CPoint CDrawObj::GetHandle(int nHandle) const
{
    ASSERT_VALID(this);
    int x, y, xCenter, yCenter;

    // this gets the center regardless of left/right and top/bottom ordering
    xCenter = m_position.left + m_position.Width() / 2;
    yCenter = m_position.top + m_position.Height() / 2;

    switch (nHandle) {
        default:
            ASSERT(FALSE);

        case 1:
            x = m_position.left;
            y = m_position.top;
            break;

        case 2:
            x = xCenter;
            y = m_position.top;
            break;

        case 3:
            x = m_position.right;
            y = m_position.top;
            break;

        case 4:
            x = m_position.right;
            y = yCenter;
            break;

        case 5:
            x = m_position.right;
            y = m_position.bottom;
            break;

        case 6:
            x = xCenter;
            y = m_position.bottom;
            break;

        case 7:
            x = m_position.left;
            y = m_position.bottom;
            break;

        case 8:
            x = m_position.left;
            y = yCenter;
            break;
    }

    return {x, y};
}

// return rectange of handle in logical coords
CRect CDrawObj::GetHandleRect(int nHandleID, CDrawView* pView) const
{
    ASSERT_VALID(this);
    ASSERT(pView != nullptr);

    // get the center of the handle in logical coords
    CPoint point = GetHandle(nHandleID);
    // convert to client/device coords
    pView->DocToClient(point);
    // return CRect of handle in device coords
    CRect rect(point.x - 3, point.y - 3, point.x + 3, point.y + 3);
    pView->ClientToDoc(rect);
    rect.NormalizeRect();
    return rect;
}

HCURSOR CDrawObj::GetHandleCursor(int nHandle) const
{
    ASSERT_VALID(this);

    const LPCTSTR kierunek[4] = { IDC_SIZEWE, IDC_SIZENWSE, IDC_SIZENS, IDC_SIZENESW };
    return ::LoadCursor(nullptr, kierunek[nHandle & 0x03]);
}

// point must be in logical
void CDrawObj::MoveHandleTo(int nHandle, const CPoint& point, CDrawView* pView)
{
    ASSERT_VALID(this);

    CRect position{m_position};
    switch (nHandle) {
        default:
            ASSERT(FALSE);

        case 1:
            position.left = point.x;
            position.top = point.y;
            break;

        case 2:
            position.top = point.y;
            break;

        case 3:
            position.right = point.x;
            position.top = point.y;
            break;

        case 4:
            position.right = point.x;
            break;

        case 5:
            position.right = point.x;
            position.bottom = point.y;
            break;

        case 6:
            position.bottom = point.y;
            break;

        case 7:
            position.left = point.x;
            position.bottom = point.y;
            break;

        case 8:
            position.left = point.x;
            break;
    }

    MoveTo(position, pView);
}


void CDrawObj::Invalidate()
{
    ASSERT_VALID(this);
    m_pDocument->UpdateAllViews(nullptr, HINT_UPDATE_DRAWOBJ, this);
}

BOOL CDrawObj::OnOpen(CDrawView* /*unused*/)
{
    ASSERT_VALID(this);
    return TRUE;
}

void CDrawObj::ChangeKolor(UINT new_kolor)
{
    kolor = (kolor == new_kolor) ? ColorId::brak : new_kolor;
    SetDirty();
    UpdateInfo();
    Invalidate();
}

void CDrawObj::DrawKolor(CDC* pDC, const CRect& pos) const
{
    CRect r1, r2, r3;
    if (kolor == ColorId::full) {
        r1 = CRect(pos.left + 1, pos.top - 1, pos.left + (int)floor((float)abs(pos.left - pos.right) / 3), pos.bottom + 1);
        r2 = CRect(pos.left + (int)floor((float)abs(pos.left - pos.right) / 3), pos.top - 1, pos.right - (int)floor((float)abs(pos.left - pos.right) / 3), pos.bottom + 1);
        r3 = CRect(pos.right - (int)floor((float)abs(pos.left - pos.right) / 3), pos.top - 1, pos.right - 1, pos.bottom + 1);
        pDC->FillRect(r1, &(((CMainFrame*)AfxGetMainWnd())->cyjan));
        pDC->FillRect(r2, &(((CMainFrame*)AfxGetMainWnd())->magenta));
        pDC->FillRect(r3, &(((CMainFrame*)AfxGetMainWnd())->yellow));
    } else {
        r1 = CRect(pos.left + 1, pos.top - 1, pos.right - 1, pos.bottom + 1);
        pDC->FillRect(r1, CDrawDoc::GetSpotBrush(kolor >> 3)); // 0 brak Spot_Kolor,Spot_Brush 0-brak, 1 full
    }
}

CString CDrawObj::RzCyfra(const int digit, const int offset) noexcept
{
    /* vu: Pierwszym argumentem jest cyfra dziesietna, drugim trzypozycyjna tablica cyfr
    rzymskich. Jej zawartosc ustalana jest na podstawie miejsca w zapisie pozycyjnym
    calej liczby cyfry bedacej pierwszym argumentem. Wywolywana przez CDrawObj::Rzymska end vu	*/

    TCHAR ret[5]{0};
    const TCHAR rz_cyfry[7] = { 'I', 'V', 'X', 'L', 'C', 'D', 'M' };
    const auto grupa_cyfr = &rz_cyfry[offset];

    switch (digit) {
        case 0:
            break;
        case 1:
            ret[0] = grupa_cyfr[0];
            break;
        case 2:
            ret[0] = grupa_cyfr[0];
            ret[1] = grupa_cyfr[0];
            break;
        case 3:
            ret[0] = grupa_cyfr[0];
            ret[1] = grupa_cyfr[0];
            ret[2] = grupa_cyfr[0];
            break;
        case 4:
            ret[0] = grupa_cyfr[0];
            ret[1] = grupa_cyfr[1];
            break;
        case 5:
            ret[0] = grupa_cyfr[1];
            break;
        case 6:
            ret[0] = grupa_cyfr[1];
            ret[1] = grupa_cyfr[0];
            break;
        case 7:
            ret[0] = grupa_cyfr[1];
            ret[1] = grupa_cyfr[0];
            ret[2] = grupa_cyfr[0];
            break;
        case 8:
            ret[0] = grupa_cyfr[1];
            ret[1] = grupa_cyfr[0];
            ret[2] = grupa_cyfr[0];
            ret[3] = grupa_cyfr[0];
            break;
        case 9:
            ret[0] = grupa_cyfr[0];
            ret[1] = grupa_cyfr[2];
            break;
        default:
            return _T("err");
    }

    return ret;
}

CString CDrawObj::Rzymska(const int i) noexcept
{
    /* vu: liczby rzymskie do kodowania pozycji systemu dziesiêtnego u¿ywaj¹ 3 ró¿nych znaków,
           a s¹siednie pozycje maj¹ jeden znak wspólny - dalej ju¿ ³atwo :end vu */

    if (abs(i) >= 3999) return _T("Big one");
    if (i < 0) return CString("-") + Rzymska(-i);
    return CString('M', i / 1000) + RzCyfra((i % 1000) / 100, 4) + RzCyfra((i % 100) / 10, 2) + RzCyfra(i % 10, 0);
}

int CDrawObj::Arabska(LPCTSTR rz) noexcept
{
    /* vu: liczby rzymskie zbudowane s¹ z cyfr o sta³ej wadze. Na pocz¹tek liczy siê wagê liczby w instrukcji switch.
           drugi krok, to uwzglêdnienie sekwencji specjalnych zawy¿aj¹cych wagê jak 'IV' czy 'CM' :end vu */

    int li_sum = 0;
    size_t i, li_len = _tcslen(rz);

    if (li_len == 0) return 0;

    for (i = 0; i < li_len; ++i)
        switch (rz[i]) {
            case 'I':
                li_sum += 1;
                break;
            case 'V':
                li_sum += 5;
                break;
            case 'X':
                li_sum += 10;
                break;
            case 'L':
                li_sum += 50;
                break;
            case 'C':
                li_sum += 100;
                break;
            case 'D':
                li_sum += 500;
                break;
            case 'M':
                li_sum += 1000;
                break;
            default:
                return -1;
        }

    if (_tcsstr((const TCHAR*)rz, _T("IV")) || _tcsstr((const TCHAR*)rz, _T("IX"))) li_sum -= 2;
    if (_tcsstr((const TCHAR*)rz, _T("XL")) || _tcsstr((const TCHAR*)rz, _T("XC"))) li_sum -= 20;
    if (_tcsstr((const TCHAR*)rz, _T("CD")) || _tcsstr((const TCHAR*)rz, _T("CM"))) li_sum -= 200;

    return li_sum;
}

double CDrawObj::modx(double x)
{
    return ((double)pwidth / x);
}

double CDrawObj::mody(double y)
{
    return ((double)phight / y);
}
