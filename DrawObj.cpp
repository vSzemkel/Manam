
#include "pch.h"
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

CDrawObj::CDrawObj(const CRect& position) noexcept : m_position(position)
{
}

void CDrawObj::DrawNapis(CDC* pDC, LPCTSTR napis, const int cnt, LPRECT r, const UINT format, const int bkMode)
{
    const auto saved = pDC->SetBkMode(bkMode);
    if (vscale == 1) 
        pDC->DrawText(napis, cnt, r, format);
    else {
        CRect lRect{r};
        const CSize& initExt = pDC->SetWindowExt(CSize(100, -100));
        lRect.top /= vscale; lRect.bottom /= vscale; lRect.left /= vscale; lRect.right /= vscale;
        pDC->DrawText(napis, cnt, lRect, format);
        pDC->SetWindowExt(initExt);
    }
    pDC->SetBkMode(saved);
}

inline int CDrawObj::GetVertPrintShift() const
{
    return PRINT_VOFFSET - (int)(2 * pmoduly * std::floor((float)m_position.top / CLIENT_SCALE / pmoduly / 40 / theApp.colsPerPage));
}

CRect CDrawObj::GetPrintRect() const
{
    const auto shift = GetVertPrintShift();
    return {m_position.left   / CLIENT_SCALE,
            m_position.top    / CLIENT_SCALE - shift,
            m_position.right  / CLIENT_SCALE,
            m_position.bottom / CLIENT_SCALE - shift};
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

void CDrawObj::DrawTracker(CDC* pDC, const TrackerState state) const
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
                constexpr int scale = 18;
                const CPoint handle = GetHandle(nHandle);
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

constexpr int CDrawObj::GetHandleCount() const
{
    return 8;
}

// returns logical coords of center of handle
CPoint CDrawObj::GetHandle(const int nHandle) const
{
    const auto center = m_position.CenterPoint();

    switch (nHandle) {
        case 1:
            return {m_position.left, m_position.top};

        case 2:
            return {center.x, m_position.top};

        case 3:
            return {m_position.right, m_position.top};

        case 4:
            return {m_position.right, center.y};

        case 5:
            return {m_position.right, m_position.bottom};

        case 6:
            return {center.x, m_position.bottom};

        case 7:
            return {m_position.left, m_position.bottom};

        case 8:
            return {m_position.left, center.y};

        default:
            ASSERT(FALSE);
            return {};
    }
}

// return rectange of handle in logical coords
CRect CDrawObj::GetHandleRect(const int nHandleID, CDrawView* pView) const
{
    ASSERT_VALID(this);
    ASSERT(pView != nullptr);

    // get the center of the handle in logical coords
    CPoint point = GetHandle(nHandleID);
    // convert to client/device coords
    pView->DocToClient(point);
    // return CRect of handle in device coords
    CRect rect{point.x - 3, point.y - 3, point.x + 3, point.y + 3};
    pView->ClientToDoc(rect);
    rect.NormalizeRect();
    return rect;
}

HCURSOR CDrawObj::GetHandleCursor(const int nHandle) const
{
    ASSERT_VALID(this);

    const LPCTSTR kierunek[4] = { IDC_SIZEWE, IDC_SIZENWSE, IDC_SIZENS, IDC_SIZENESW };
    return ::LoadCursor(nullptr, kierunek[nHandle & 0x03]);
}

// point must be in logical
void CDrawObj::MoveHandleTo(const int nHandle, const CPoint& point, CDrawView* pView)
{
    ASSERT_VALID(this);

    CRect position{m_position};
    switch (nHandle) {
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

        default:
            ASSERT(FALSE);
    }

    MoveTo(position, pView);
}

void CDrawObj::MoveResize(const UINT nChar)
{
    int x{}, y{};

    switch (nChar) {
        case VK_LEFT:
            x -= vscale;
            break;

        case VK_UP:
            y += vscale;
            break;

        case VK_RIGHT:
            x += vscale;
            break;

        case VK_DOWN:
            y -= vscale;
            break;

        default:
            break;
    }

    if ((::GetKeyState(VK_SHIFT) & 0xff00) == 0) // move
        m_position.MoveToXY(m_position.left + x, m_position.top + y);
    else { // resize
        const auto s = CSize{x >> 1, y >> 1};
        m_position.InflateRect(s);
        m_position += s;
    }

    SetDirty();
    Invalidate();
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

void CDrawObj::ChangeKolor(const UINT new_kolor)
{
    kolor = (kolor == new_kolor) ? ColorId::brak : new_kolor;
    SetDirty();
    UpdateInfo();
    Invalidate();
}

void CDrawObj::DrawKolor(CDC* pDC, const CRect& pos) const
{
    if (kolor == ColorId::full) {
        const auto width = pos.Width() / 3;
        const auto wnd = (CMainFrame*)AfxGetMainWnd();
        const CRect r1{pos.left + 1, pos.top - 1, pos.left + width, pos.bottom + 1};
        const CRect r2{pos.left + width, pos.top - 1, pos.right - width, pos.bottom + 1};
        const CRect r3{pos.right - width, pos.top - 1, pos.right - 1, pos.bottom + 1};
        pDC->FillRect(r1, &wnd->cyan);
        pDC->FillRect(r2, &wnd->magenta);
        pDC->FillRect(r3, &wnd->yellow);
    } else {
        const CRect r1{pos.left + 1, pos.top - 1, pos.right - 1, pos.bottom + 1};
        pDC->FillRect(r1, CDrawDoc::GetSpotBrush(kolor >> 3)); // 0 brak Spot_Kolor,Spot_Brush 0-brak, 1 full
    }
}

CString CDrawObj::RzCyfra(const int digit, const int offset)
{
    /* vu: Pierwszym argumentem jest cyfra dziesietna, drugim trzypozycyjna tablica cyfr
    rzymskich. Jej zawartosc ustalana jest na podstawie miejsca w zapisie pozycyjnym
    calej liczby cyfry bedacej pierwszym argumentem. Wywolywana przez CDrawObj::Rzymska end vu	*/

    constexpr const TCHAR rz_cyfry[8] = _T("IVXLCDM");
    const auto grupa_cyfr = &rz_cyfry[offset];
    TCHAR ret[5]{0};

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

CString CDrawObj::Rzymska(const int i)
{
    /* vu: liczby rzymskie do kodowania pozycji systemu dziesietnego uzywaja 3 roznych znakow,
           a sasiednie pozycje maja jeden znak wspolny - dalej juz latwo :end vu */

    if (std::abs(i) >= 3999) return _T("Big one");
    if (i < 0) return CString("-") + Rzymska(-i);
    return CString('M', i / 1000) + RzCyfra((i % 1000) / 100, 4) + RzCyfra((i % 100) / 10, 2) + RzCyfra(i % 10, 0);
}

int CDrawObj::Arabska(LPCTSTR rz)
{
    /* vu: liczby rzymskie zbudowane sa z cyfr o stalej wadze. Na poczatek liczy sie wage liczby w instrukcji switch.
           drugi krok, to uwzglednienie sekwencji specjalnych zawyzajacych wage jak 'IV' czy 'CM' :end vu */

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

double CDrawObj::modx(const int x)
{
    return ((double)pwidth / x);
}

double CDrawObj::mody(const int y)
{
    return ((double)phight / y);
}
