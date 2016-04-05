
#include "stdafx.h"
#include "manam.h"
#include "drawdoc.h"
#include "drawvw.h"
#include "cntritem.h"
#include "koldlg.h"
#include "mainfrm.h"

extern BOOL drawErrorBoxes ;
extern CDrawApp NEAR theApp ;

IMPLEMENT_SERIAL(CDrawObj, CObject, 0)

CDrawObj::CDrawObj()
{
	dirty = TRUE;
	info = _T("");
	kolor = c_brak;
	m_pDocument = nullptr;
}

CDrawObj::CDrawObj(const CRect* position) : CDrawObj::CDrawObj()
{
	m_position = *position;
}

void CDrawObj::DrawNapis(CDC *pDC, LPCTSTR napis, int cnt, LPRECT r, UINT format, int bkMode) {
	int saved = pDC->SetBkMode(bkMode);
	if (vscale == 1) {
		pDC->DrawText(napis, cnt, r, format);
		return ;
	} else {
		CSize& initExt = pDC->SetWindowExt(CSize(100, -100));
		CRect lRect = r ;
		lRect.top /= vscale; lRect.bottom /= vscale; lRect.left /= vscale; lRect.right /= vscale;
		pDC->DrawText(napis, cnt, lRect, format);
		pDC->SetWindowExt(initExt);
	}
	pDC->SetBkMode(saved);
}

inline int CDrawObj::GetVertPrintShift() const {
	return PRINT_VOFFSET - (int)(2 * pmoduly * floor((float)m_position.top / CLIENT_SCALE / pmoduly / 40 / theApp.colsPerPage));
}

CRect CDrawObj::GetPrintRect() const {
	return CRect(m_position.left / CLIENT_SCALE,
				 m_position.top / CLIENT_SCALE - GetVertPrintShift(),
				 m_position.right / CLIENT_SCALE,
				 m_position.bottom / CLIENT_SCALE - GetVertPrintShift());
}

void CDrawObj::Serialize(CArchive& ar)
{
	CObject::Serialize(ar);

	if (ar.IsStoring())
	{
		ar << m_position;
		ar << (WORD)kolor;
		ar << info;
	}
	else
	{
		// get the document back pointer from the archive
		m_pDocument = (CDrawDoc*)ar.m_pDocument;
		ASSERT_VALID(m_pDocument);
		ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDrawDoc)));

		ar >> m_position;
		WORD wTemp;
		ar >> wTemp; kolor = wTemp;
		ar >> info; 
		dirty = TRUE;
	}
}

void CDrawObj::SetDirty() {
	dirty = TRUE ;
	m_pDocument->SetModifiedFlag() ;
}

void CDrawObj::Remove()
{
	delete this;
}

void CDrawObj::Draw(CDC* )
{
}

void CDrawObj::DrawTracker(CDC* pDC, TrackerState state) const
{
	ASSERT_VALID(this);

	switch (state)
	{
	case normal:
		break;

	case selected:
	case active:
		{
			int nHandleCount = GetHandleCount();
			for (int nHandle = 1; nHandle <= nHandleCount; nHandle += 1)
			{
				CPoint handle = GetHandle(nHandle);
				CMDIChildWnd *vWnd ;
				double zoom = 100 ;
				if (vWnd = ((CMainFrame*)AfxGetMainWnd())->MDIGetActive()) {
					auto view = dynamic_cast<CDrawView*>(vWnd->GetActiveView());
					if (view != nullptr)
						zoom = view->GetZoomFactor();
				}
				zoom = vscale*(-zoom*(3.0 / 1600.0) + 19.0 / 16.0);     // :-)
				pDC->PatBlt(handle.x - (int) (2 * zoom), handle.y - (int) (2 * zoom), (int) (5 * zoom), (int) (5 * zoom), BLACKNESS);
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

	if (pView == nullptr)
	{
		Invalidate();
		m_position = position;
		Invalidate();
	}
	else
	{
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
int CDrawObj::HitTest(const CPoint& point, CDrawView* pView, BOOL bSelected) const
{
	ASSERT_VALID(this);
	ASSERT(pView != nullptr);

	if (bSelected)
	{
		int nHandleCount = GetHandleCount();
		for (int nHandle = 1; nHandle <= nHandleCount; ++nHandle)
		{
			// GetHandleRect returns in logical coords
			CRect rc = GetHandleRect(nHandle, pView);
			if (rc.PtInRect(point))
				return nHandle;
		}
	}
	else
	{
		if (point.x >= m_position.left && point.x < m_position.right &&
			point.y <= m_position.top && point.y > m_position.bottom)
			return 1;
	}
	return 0;
}

// rect must be in logical coordinates
BOOL CDrawObj::Intersects(const CRect& rect) const
{
	ASSERT_VALID(this);

	CRect fixed = m_position;
	fixed.NormalizeRect();
	CRect rectT = rect;
	rectT.NormalizeRect();
	return !(rectT & fixed).IsRectEmpty();
}

BOOL CDrawObj::Contains(const CPoint& point) const
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

	switch (nHandle)
	{
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

	return CPoint(x, y);
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

	LPCTSTR kierunek[4] = {IDC_SIZEWE, IDC_SIZENWSE, IDC_SIZENS, IDC_SIZENESW};
	return ::LoadCursor(0, kierunek[nHandle & 0x03]);
}

// point must be in logical
void CDrawObj::MoveHandleTo(int nHandle, const CPoint& point, CDrawView* pView)
{
	ASSERT_VALID(this);

	CRect position = m_position;
	switch (nHandle)
	{
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
	m_pDocument->UpdateAllViews(NULL, HINT_UPDATE_DRAWOBJ, this);
}

CDrawObj* CDrawObj::Clone(CDrawDoc* pDoc) const
{
	ASSERT_VALID(this);

	auto pClone = new CDrawObj(&m_position);
	pClone->kolor = kolor;

	ASSERT_VALID(pClone);

	if (pDoc != nullptr)
		pDoc->Add(pClone);

	return pClone;
}

BOOL CDrawObj::OnOpen(CDrawView* )
{
	ASSERT_VALID(this);
	return TRUE;
}

void CDrawObj::ChangeKolor(UINT new_kolor)
{
	kolor = (kolor == new_kolor) ? c_brak : new_kolor;
	SetDirty();
	UpdateInfo();
	Invalidate();
}

void CDrawObj::DrawKolor(CDC* pDC, const CRect& pos) const
{ 
	CRect r1, r2, r3;
	if (kolor == c_full) {
		r1 = CRect(pos.left + 1, pos.top - 1, pos.left + (int)floor((float)abs(pos.left - pos.right) / 3), pos.bottom + 1);
		r2 = CRect(pos.left + (int)floor((float)abs(pos.left - pos.right) / 3), pos.top - 1, pos.right - (int)floor((float)abs(pos.left - pos.right) / 3), pos.bottom + 1);
		r3 = CRect(pos.right - (int)floor((float)abs(pos.left - pos.right) / 3), pos.top - 1, pos.right - 1, pos.bottom + 1);
		pDC->FillRect(r1, &(((CMainFrame*)AfxGetMainWnd())->cyjan));
		pDC->FillRect(r2, &(((CMainFrame*)AfxGetMainWnd())->magenta));
		pDC->FillRect(r3, &(((CMainFrame*)AfxGetMainWnd())->yellow));
	} else {
		r1 = CRect(pos.left + 1, pos.top - 1, pos.right - 1, pos.bottom + 1);
		pDC->FillRect(r1, ((CMainFrame*)AfxGetMainWnd())->GetSpotBrush((kolor >> 3))); // 0 brak Spot_Kolor,Spot_Brush 0-brak, 1 full
	}
}

CString CDrawObj::RzCyfra(int i, const CString* znaki) 
{
	/* vu: Pierwszym argumentem jest cyfra dziesietna, drugim trzypozycyjna tablica cyfr 
	rzymskich. Jej zawartosc ustalana jest na podstawie miejsca w zapisie pozycyjnym 
	calej liczby cyfry bedacej pierwszym argumentem. Wywolywana przez CDrawObj::Rzymska end vu	*/

	switch (i) {
	case 0:
		return CString("");
	case 1:
		return znaki[0];
	case 2:
		return znaki[0]+znaki[0];
	case 3:
		return znaki[0]+znaki[0]+znaki[0];
	case 4:
		return znaki[0]+znaki[1];
	case 5:
		return znaki[1];
	case 6:
		return znaki[1]+znaki[0];
	case 7:
		return znaki[1]+znaki[0]+znaki[0];
	case 8:
		return znaki[1]+znaki[0]+znaki[0]+znaki[0];
	case 9:
		return znaki[0]+znaki[2];
	default:
		return CString("err");
	}
} 

CString CDrawObj::Rzymska(int i)
{
	/* vu: liczby rzymskie do kodowania pozycji systemu dziesiêtnego u¿ywaj¹ 3 ró¿nych znaków,
		   a s¹siednie pozycje maj¹ jeden znak wspólny - dalej ju¿ ³atwo :end vu */

	CString rz_cyfry[7] = { "I", "V", "X", "L", "C", "D", "M" };
	if (abs(i) >= 9000) return "Big one";
	if (i < 0) return CString("-") + Rzymska(-i);
	return RzCyfra(i / 100, &rz_cyfry[4]) + RzCyfra((i % 100) / 10, &rz_cyfry[2]) + RzCyfra(i % 10, &rz_cyfry[0]);
}

int CDrawObj::Arabska(LPCTSTR rz) 
{ 
  /* vu: liczby rzymskie zbudowane s¹ z cyfr o sta³ej wadze. Na pocz¹tek liczy siê wagê liczby w instrukcji switch.
  	     drugi krok, to uwzglêdnienie sekwencji specjalnych zawy¿aj¹cych wagê jak 'IV' czy 'CM' :end vu */
  	     
	int li_sum = 0;
	size_t i, li_len = _tcslen(rz);

	if (li_len == 0) return 0;

	for (i = 0; i < li_len; i++)
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

	if (_tcsstr((const TCHAR*) rz, _T("IV")) || _tcsstr((const TCHAR*) rz, _T("IX"))) li_sum -= 2;
	if (_tcsstr((const TCHAR*) rz, _T("XL")) || _tcsstr((const TCHAR*) rz, _T("XC"))) li_sum -= 20;
	if (_tcsstr((const TCHAR*) rz, _T("CD")) || _tcsstr((const TCHAR*) rz, _T("CM"))) li_sum -= 200;

	return li_sum;
}

double CDrawObj::modx(double x) 
{ 
	return ((double)pwidth / x);
}

double CDrawObj::mody(double y) 
{ return ((double)phight/y); } 

#ifdef _DEBUG
void CDrawObj::AssertValid()
{
	ASSERT(m_position.left <= m_position.right);
	ASSERT(m_position.bottom <= m_position.top);
}
#endif

IMPLEMENT_SERIAL(CDrawOleObj, CDrawObj, 0)

BOOL CDrawOleObj::c_bShowItems = TRUE;

CDrawOleObj::CDrawOleObj() : m_extent(0,0)
{
	m_pClientItem = nullptr;
}

CDrawOleObj::CDrawOleObj(const CRect* position)	: CDrawObj(position), m_extent(0, 0)
{
	m_pClientItem = nullptr;
}

CDrawOleObj::~CDrawOleObj()
{
	if (m_pClientItem != nullptr)
	{
		m_pClientItem->Release();
		m_pClientItem = nullptr;
	}
}

void CDrawOleObj::Serialize( CArchive& ar )
{
	ASSERT_VALID(this);

	CDrawObj::Serialize(ar);

	if (ar.IsStoring())
	{
		ar << m_extent;
		ar << m_pClientItem;
	}
	else
	{
		ar >> m_extent;
		ar >> m_pClientItem;
		m_pClientItem->m_pDrawObj = this;
	}
}

CDrawObj* CDrawOleObj::Clone(CDrawDoc* pDoc) const
{
	ASSERT_VALID(this);

	AfxGetApp()->BeginWaitCursor();

	CDrawOleObj* pClone = nullptr;
	CDrawItem* pItem = nullptr;
	TRY
	{
		// perform a "deep copy" -- need to copy CDrawOleObj and the CDrawItem
		//  that it points to.
		pClone = new CDrawOleObj(&m_position);
		pItem = new CDrawItem(m_pDocument, pClone);
		if (!pItem->CreateCloneFrom(m_pClientItem))
			AfxThrowMemoryException();

		pClone->m_pClientItem = pItem;
		pClone->kolor = kolor;
		pClone->info = info;
		ASSERT_VALID(pClone);

		if (pDoc != nullptr)
			pDoc->Add(pClone);
	}
	CATCH_ALL(e)
	{
		pItem->Delete();
		pClone->m_pClientItem = nullptr;
		pClone->Remove();
		AfxGetApp()->EndWaitCursor();

		THROW_LAST();
	}
	END_CATCH_ALL

	AfxGetApp()->EndWaitCursor();
	return pClone;
}

void CDrawOleObj::Draw(CDC* pDC)
{
	ASSERT_VALID(this);

	CDrawItem* pItem = m_pClientItem;
	if (pItem != nullptr)
	{
		// draw the OLE item itself
		pItem->Draw(pDC, m_position);

		// don't draw tracker in print preview or on printer
		if (!pDC->IsPrinting())
		{
			// use a CRectTracker to draw the standard effects
			CRectTracker tracker;
			tracker.m_rect = m_position;
			pDC->LPtoDP(tracker.m_rect);

			if (c_bShowItems)
			{
				// put correct border depending on item type
				if (pItem->GetType() == OT_LINK)
					tracker.m_nStyle |= CRectTracker::dottedLine;
				else
					tracker.m_nStyle |= CRectTracker::solidLine;
			}

			// put hatching over the item if it is currently open
			if (pItem->GetItemState() == COleClientItem::openState ||
				pItem->GetItemState() == COleClientItem::activeUIState)
			{
				tracker.m_nStyle |= CRectTracker::hatchInside;
			}
			tracker.Draw(pDC);
		}
	}
}

BOOL CDrawOleObj::OnOpen(CDrawView* pView)
{
	AfxGetApp()->BeginWaitCursor();
	m_pClientItem->DoVerb(
		GetKeyState(VK_CONTROL) < 0 ? OLEIVERB_OPEN : OLEIVERB_PRIMARY,
		pView);
	AfxGetApp()->EndWaitCursor();
	return TRUE;
}

// position is in logical
void CDrawOleObj::MoveTo(const CRect& position, CDrawView* pView)
{
	ASSERT_VALID(this);

	if (position == m_position)
		return;

	// call base class to update position
	CDrawObj::MoveTo(position, pView);

	// update position of in-place editing session on position change
	if (m_pClientItem->IsInPlaceActive())
		m_pClientItem->SetItemRects();
}

// CRozm
CRozm::CRozm() { // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!use delegating constructor in VS2013
	// domyœlne rozmiary kraty standardowej, bez logowania
	w = 468; h = 575; sw = 40; sh = 34;
	szpalt_x = 5;
	szpalt_y = 6;
	scale_it = typ_xx = 0;
}

CRozm::CRozm(int m_w, int m_h, int m_sw,int m_sh, BYTE m_szpalt_x, BYTE m_szpalt_y, int m_typ_xx, BOOL m_scale_it)
{
	w = (WORD)m_w;
	h = (WORD)m_h;
	sw = (WORD)m_sw;
	sh = (WORD)m_sh;
	szpalt_x = m_szpalt_x;
	szpalt_y = m_szpalt_y;
	typ_xx = m_typ_xx;
	scale_it = m_scale_it;
}
