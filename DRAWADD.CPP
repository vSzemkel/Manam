
#include "stdafx.h"
#include "manam.h"
#include "drawdoc.h"
#include "drawvw.h"
#include "drawtool.h"
#include "drawpage.h"
#include "drawadd.h"
#include "koldlg.h"
#include "mainfrm.h"
#include "spacer.h" 
#include "manpdf.h" 
#include "gridfrm.h" 
#include "genepsinfodlg.h"

extern BOOL drawErrorBoxes ;
extern CDrawApp NEAR theApp ;
extern SelectMode selectMode ;

const SpadInfo aSpadInfo[16] = {
	{0,     0, 1, 1}, //0
	{0,     0, 0, 1},
	{0,     0, 1, 0},
	{0,     0, 0, 0},
	{1,     0, 0, 1},
	{0.5,   0, 0, 1}, //5
	{1,     0, 0, 0},
	{0.5,   0, 0, 0},
	{0,     1, 1, 0},
	{0,     1, 0, 0},
	{0,   0.5, 1, 0}, //10
	{0,   0.5, 0, 0},
	{1,     1, 0, 0},
	{0.5,   1, 0, 0},
	{1,   0.5, 0, 0},
	{0.5, 0.5, 0, 0}, //15
};

const int CDrawAdd::ciMaxRings = 4;

IMPLEMENT_SERIAL(CDrawAdd, CDrawObj, 0)

CDrawAdd::CDrawAdd() : // u¿ywany tylko przez ::Serialize
	m_pub_xx(-1), nag_xx(1), bank{0}, precelWertexCnt(0), precelRingCnt(0), iFileId(0)
{}

CDrawAdd::CDrawAdd(const CRect* position) : CDrawAdd::CDrawAdd()
{
	ASSERT_VALID(this);
	m_position = *position;
	m_add_xx = -1;
	nreps = oldAdno = -1;
	czaskto = "# []";
	nazwa = "MAKUMBA";
	epsDate = powtorka = posx = posy = fizpage = typ_xx = spad_flag = 0;
	szpalt_x = pszpalt_x;
	szpalt_y = pszpalt_y;
	memset(&flags, 0, 2);
	flags.epsok = 2;
	txtposx = txtposy = 1 + TXTSHIFT;
	SetSpaceSize(1, 1);
}

CDrawAdd::~CDrawAdd() {
	if (fizpage != 0)
		m_pDocument->GetPage(fizpage)->RemoveAdd(this);
}

void CDrawAdd::Serialize(CArchive& ar)
{
	ASSERT_VALID(this);

	CDrawObj::Serialize(ar);
	if (ar.IsStoring())
	{
		ar << m_add_xx;
		ar << (WORD)typ_xx;
		ar << nazwa;
		ar << logpage;
		ar << remarks;
		ar << remarks_atex;
		ar << wersja;
		ar << czaskto;
		ar << kodModulu;
		ar << (WORD)szpalt_x; 
		ar << (WORD)szpalt_y;
		ar << (WORD)sizex; 
		ar << (WORD)sizey;
		ar << (WORD)fizpage;
		ar << (WORD)posx;
		ar << (WORD)posy;
		ar << (WORD)txtposx;
		ar << (WORD)txtposy;
		ar << *(WORD*)&flags ;
		ar << (LONG)nreps;
		ar << (LONG)oldAdno;
		ar << powtorka;
		ar << (WORD)spad_flag;
		ar << (WORD)precelWertexCnt;
		if (precelWertexCnt) {
			for (int i = 0; i < precelWertexCnt; i++)
				ar << this->aPrecelWertex[i];
			ar << (WORD) precelRingCnt;
			for (int i = 0; i < precelRingCnt; i++)
				ar << aRingWertexCnt[i];
		}
	  space.Serialize(ar);
	} else {
		ar >> m_add_xx;
		WORD wTemp,wTemp0,wTemp1,wTemp2;
		ar >> wTemp; typ_xx=wTemp;
		ar >> nazwa;
		ar >> logpage;
		ar >> remarks;
		ar >> remarks_atex;
		ar >> wersja;
		ar >> czaskto;
		ar >> kodModulu;
		ar >> wTemp; szpalt_x = wTemp;
		ar >> wTemp; szpalt_y = wTemp;
		ar >> wTemp; sizex = wTemp;
		ar >> wTemp; sizey = wTemp;
		ar >> wTemp0; fizpage = 0;
		ar >> wTemp1; posx = 0;
		ar >> wTemp2; posy = 0;
		ar >> wTemp; txtposx = wTemp;
		ar >> wTemp; txtposy = wTemp;
		ar >> wTemp; *((WORD*)&flags) = wTemp ;
		ar >> nreps ;
		ar >> oldAdno ;
		ar >> powtorka ;
		ar >> wTemp; spad_flag = (BYTE)wTemp ;
		ar >> wTemp; precelWertexCnt = wTemp ;
		if (precelWertexCnt) {
			aPrecelWertex = std::make_unique<CPoint []>(precelWertexCnt + 2); // na koñcu int[4]
			aRingWertexCnt = reinterpret_cast<int*>(&aPrecelWertex[precelWertexCnt]);
			for (int i = 0; i < precelWertexCnt; i++)
				ar >> aPrecelWertex[i];
			ar >> wTemp; precelRingCnt = wTemp;
			ASSERT(precelRingCnt <= ciMaxRings);
			for (int i = 0; i < precelRingCnt; i++)
				ar >> aRingWertexCnt[i];
		}
		space.Serialize(ar);
		SetSpaceAndPosition(wTemp0, wTemp1, wTemp2);
	}
}

////////////////////////// DRAW
void CDrawAdd::Draw(CDC* pDC)
{
	ASSERT_VALID(this);

	auto mainWnd = reinterpret_cast<CMainFrame*>(AfxGetMainWnd());
	CBrush* pOldBrush = pDC->SelectObject(mainWnd->GetSpotBrush((kolor>>3)));
	CPen* pOldPen = pDC->SelectObject(&(mainWnd->pen));

	int iClipRegions = 1;
	CRect r1,*aRectClip = nullptr,rect = m_position;
	// clipping dla precli
	if (precelWertexCnt > 0) {
		CRgn rgn;
		if (precelRingCnt == 1)		// clipping dla precli
			rgn.CreatePolygonRgn(aPrecelWertex.get(), precelWertexCnt, ALTERNATE);
		else if (precelRingCnt > 1)	//clipping dla precli z wieloma obwodniacami
			rgn.CreatePolyPolygonRgn(aPrecelWertex.get(), aRingWertexCnt, precelRingCnt, ALTERNATE);
		rgn.OffsetRgn(m_position.left, m_position.top);
		rgn.GetRegionData((RGNDATA*)theApp.bigBuf, bigSize);
		aRectClip = (CRect*)((RGNDATA*)theApp.bigBuf)->Buffer;
		iClipRegions = ((RGNDATA*)theApp.bigBuf)->rdh.nCount;
		rgn.DeleteObject();
	}
	// wype³nianie kolorem
	for (int i = 0; i < iClipRegions; ++i) {
		if (aRectClip)
			pDC->IntersectClipRect(aRectClip[i]);
		switch (m_pDocument->swCZV) {
		case 0:
			DrawKolor(pDC, &m_position);
			break ;
		case 1:
			r1 = CRect(m_position.left + 1, m_position.top - 1, m_position.right - 1, m_position.bottom + 1);
			switch (flags.isok) {
			case 1: // 'OK.'
				pDC->FillRect(r1, &mainWnd->yellow);
				break ;
			case 2: // 'ATX'
				pDC->FillRect(r1, &mainWnd->magenta);
				break ;
			default: // 'WER'
				pDC->FillRect(r1, mainWnd->GetSpotBrush(0)); 
			}
			break ;
		case 2: 
			r1 = CRect(m_position.left + 1, m_position.top - 1, m_position.right - 1, m_position.bottom + 1);
			int iKolInd = ((CMainFrame*) AfxGetMainWnd())->GetKolorInd(wersja.Right(wersja.GetLength() - wersja.Find(_T(".")) - 1));
			if (iKolInd == -1) 
				iKolInd = 0;
			pDC->FillRect(r1, mainWnd->GetSpotBrush(iKolInd ? (iKolInd+1) % mainWnd->Spot_Brush.size() : 0));
		}
		if (aRectClip)
			pDC->SelectClipRgn(NULL); 
	}

	pDC->SelectStockObject(NULL_BRUSH);
	switch (precelRingCnt) {
		case 0:
			pDC->Rectangle(rect); 
			break;
		case 1:
			pDC->OffsetWindowOrg(-m_position.left, -m_position.top);
			pDC->Polygon(aPrecelWertex.get(), precelWertexCnt);
			pDC->OffsetWindowOrg(m_position.left, m_position.top);
			break;
		default:
			pDC->OffsetWindowOrg(-m_position.left, -m_position.top);
			pDC->PolyPolygon(aPrecelWertex.get(), aRingWertexCnt, precelRingCnt);
			pDC->OffsetWindowOrg(m_position.left, m_position.top);
			break;
	}
	
	if (flags.derived) {
		CRect r = rect;
		r.InflateRect(-2 * CLIENT_SCALE, 2 * CLIENT_SCALE);
		pDC->Rectangle(r);
	}

	if (selectMode != SelectMode::move) {
		DrawDesc(pDC, &m_position) ;
		if ((!(m_pDocument->swCZV&1) && flags.locked) || (m_pDocument->swCZV&1 && flags.reserv))
			DrawPadlock(pDC, &m_position);
		int retCkPageLocation = 0;
		if (m_pDocument->swCZV == 0 && drawErrorBoxes) 
			retCkPageLocation = CkPageLocation(fizpage) ;
		bool simpleCase = (m_pDocument->swCZV == 1 && flags.zagroz) || (m_pDocument->swCZV == 2 && flags.showeps);
		if (simpleCase || retCkPageLocation > 0) {
			pDC->MoveTo(rect.left, rect.top) ;
			pDC->LineTo(rect.right, rect.bottom) ;
			if (simpleCase || retCkPageLocation > 1) {
				pDC->MoveTo(rect.right, rect.top) ;
				pDC->LineTo(rect.left, rect.bottom) ;
			}
		}
	}

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}

void CDrawAdd::DrawDesc(CDC* pDC, const CRect& rect) const
{
	/* vu : Rysuje opis tekstowy og³oszenia na podanym urz¹dzeniu	end vu */
	
	CString buf;
	switch (theApp.m_view_top) { // vu : opis gorny
		case TEXT_NAZWA:
			DrawTx(pDC, rect, nazwa, TRUE);
			break;
		case TEXT_PLIK:
			DrawTx(pDC, rect, kodModulu, TRUE);
			break;
		case TEXT_WARLOG:
			DrawTx(pDC, rect, logpage, TRUE);
			break;
		case TEXT_CZASKTO:
			DrawTx(pDC, rect, czaskto, TRUE);
			break;
		case TEXT_UWAGI:
			DrawTx(pDC, rect, remarks, TRUE);
			break;
		case TEXT_STUDIO:
			DrawTx(pDC, rect, CGridFrm::studioStats[this->flags.studio], TRUE);
			break;
		case TEXT_EPS:
			if (nreps > -1L) {
				buf.Format(_T("%li"), nreps);
				DrawTx(pDC, rect, buf, TRUE);
			}
	}

	switch (theApp.m_view_bottom) { // vu : opis dolny
		case TEXT_NAZWA:
			DrawTx(pDC, rect, nazwa, FALSE);
			break;
		case TEXT_PLIK:
			DrawTx(pDC, rect, kodModulu, FALSE);
			break;
		case TEXT_WARLOG:
			DrawTx(pDC, rect, logpage, FALSE);
			break;
		case TEXT_CZASKTO:
			DrawTx(pDC, rect, czaskto, FALSE);
			break;
		case TEXT_UWAGI:
			DrawTx(pDC, rect, remarks, FALSE);
			break;
		case TEXT_STUDIO:
			DrawTx(pDC, rect, CGridFrm::studioStats[this->flags.studio], FALSE);
			break;
		case TEXT_EPS:
			if (nreps > -1L) {
				buf.Format(_T("%li"), nreps);
				DrawTx(pDC, rect, buf, FALSE);
			}
	}
}

void CDrawAdd::DrawTx(CDC* pDC, const CRect& rect, LPCTSTR tx, BOOL top) const
{
	CFont *oldFont = pDC->SelectObject(&(m_pDocument->m_addfont));
	CRect r = rect;
	r += CPoint(vscale*(txtposx-TXTSHIFT),-vscale*(txtposy-TXTSHIFT));
	pDC->SetBkColor(BIALY);
	if (top)
		DrawNapis(pDC, tx, (int)_tcslen(tx), r, DT_LEFT|DT_TOP|DT_SINGLELINE|DT_NOCLIP, OPAQUE);
	else {
		LOGFONT lf;
		m_pDocument->m_addfont.GetObject(sizeof(LOGFONT), &lf);
		r += CPoint(0, -vscale*abs(lf.lfHeight)) ;
		DrawNapis(pDC, tx, (int) _tcslen(tx), r, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOCLIP, OPAQUE);
	} 
	pDC->SelectObject(oldFont);
}

void CDrawAdd::DrawPadlock(CDC* pDC, const CRect& rect) const
{
	CBrush* pOldBrush = (CBrush*)pDC->SelectStockObject(BLACK_BRUSH);
	CPen* pOldPen = (CPen*)pDC->SelectStockObject(BLACK_PEN);

	if (!(m_pDocument->swCZV & 1)) { // k³ódka
		CRect r(rect.right - 4 * vscale, rect.bottom + 4 * vscale, rect.right, rect.bottom);
		pDC->Rectangle(r);
		(CBrush*)pDC->SelectStockObject(NULL_BRUSH);
		r.SetRect(r.left, r.top + 2 * vscale, r.right, r.bottom + 2 * vscale);
		pDC->Ellipse(r);
	} else { // flaga
		CPoint tr[3];
		tr[2].x = tr[0].x = rect.right - 6 * vscale; tr[1].x = rect.right - 2 * vscale;
		tr[0].y = tr[1].y = rect.bottom + 4 * vscale; tr[2].y = rect.bottom + 7 * vscale;
		pDC->Polygon(tr, 3);
		pDC->MoveTo(tr[0].x, tr[0].y);
		pDC->LineTo(tr[0].x, tr[0].y - 4 * vscale);
	}
	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}

////////////////////////// PRINT
void CDrawAdd::Print(CDC* pDC)
{
	ASSERT_VALID(this);

	CBrush* pOldBrush = pDC->SelectObject(((CMainFrame*)AfxGetMainWnd())->GetSpotBrush(kolor>>3));
	CPen* pOldPen = (CPen*)pDC->SelectStockObject(BLACK_PEN);

	CRgn rgn;
	int iClipRegions;
	CRect *aRectClip, rect = GetPrintRect() ;

	switch (precelRingCnt) {
		case 0:
			pDC->Rectangle(rect); 
			if (kolor==c_full)
			   DrawKolor(pDC, &rect);
			break;
		case 1:
			for (int i = 0; i < precelWertexCnt; i++) {
				aPrecelWertex[i].x /= CLIENT_SCALE;
				aPrecelWertex[i].y /= CLIENT_SCALE;
			}
			rgn.CreatePolygonRgn(aPrecelWertex.get(), precelWertexCnt, ALTERNATE);
			rgn.OffsetRgn(rect.left, rect.top);
			rgn.GetRegionData((RGNDATA*)theApp.bigBuf, bigSize);
			aRectClip = (CRect*)((RGNDATA*)theApp.bigBuf)->Buffer;
			iClipRegions = ((RGNDATA*)theApp.bigBuf)->rdh.nCount;
			rgn.DeleteObject();
			for (int i = 0; i < iClipRegions; i++) {
				pDC->IntersectClipRect(aRectClip[i]); 
				DrawKolor(pDC, &rect);
				pDC->SelectClipRgn(NULL);
			}
		   	pDC->SelectStockObject(NULL_BRUSH);
			pDC->OffsetWindowOrg(-rect.left, -rect.top);
			pDC->Polygon(aPrecelWertex.get(), precelWertexCnt);
			pDC->OffsetWindowOrg(rect.left, rect.top);
			for (int i = 0; i < precelWertexCnt; i++) {
				aPrecelWertex[i].x *= CLIENT_SCALE;
				aPrecelWertex[i].y *= CLIENT_SCALE;
			}
			break;
		default:
			for (int i = 0; i < precelWertexCnt; i++) {
				aPrecelWertex[i].x /= CLIENT_SCALE;
				aPrecelWertex[i].y /= CLIENT_SCALE;
			}
			rgn.CreatePolyPolygonRgn(aPrecelWertex.get(), aRingWertexCnt, precelRingCnt, ALTERNATE);
			rgn.OffsetRgn(rect.left, rect.top);
			rgn.GetRegionData((RGNDATA*)theApp.bigBuf, bigSize);
			aRectClip = (CRect*)((RGNDATA*)theApp.bigBuf)->Buffer;
			iClipRegions = ((RGNDATA*)theApp.bigBuf)->rdh.nCount;
			rgn.DeleteObject();
			for (int i = 0; i < iClipRegions; i++) {
				pDC->IntersectClipRect(aRectClip[i]);
				DrawKolor(pDC, &rect);
				pDC->SelectClipRgn(NULL);
			}
		   	pDC->SelectStockObject(NULL_BRUSH);
			pDC->OffsetWindowOrg(-rect.left, -rect.top);
			pDC->PolyPolygon(aPrecelWertex.get(), aRingWertexCnt, precelRingCnt);
			pDC->OffsetWindowOrg(rect.left, rect.top);
			for (int i = 0; i < precelWertexCnt; i++) {
				aPrecelWertex[i].x *= CLIENT_SCALE;
				aPrecelWertex[i].y *= CLIENT_SCALE;
			}
			break;
	}
	if (flags.locked)
		PrintPadlock(pDC, &rect);
	pDC->SelectStockObject(NULL_BRUSH);
	if (flags.derived) {
		CRect r = rect;
		r.InflateRect(-2, 2);
		pDC->Rectangle(r); 
	}

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}

void CDrawAdd::PrintPadlock(CDC* pDC, const CRect& rect)
{
	auto pOldBrush = static_cast<CBrush*>(pDC->SelectStockObject(BLACK_BRUSH));
	auto pOldPen = static_cast<CPen*>(pDC->SelectObject(&(((CMainFrame*)AfxGetMainWnd())->pen)));

	CRect r(rect.right - 3 * vscale, rect.bottom + 3 * vscale, rect.right, rect.bottom);
	pDC->Rectangle(r);
	pDC->SelectStockObject(NULL_BRUSH);
	r.SetRect(r.left, r.top + 2 * vscale, r.right, r.bottom + 1 * vscale);
	pDC->Ellipse(r);

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}

BOOL CDrawAdd::Lock()
{
	if (fizpage != 0) {
		if (m_pDocument->swCZV & 1) flags.reserv = !flags.reserv;
		else flags.locked = !flags.locked;
		SetDirty();
	}
	Invalidate();
	return flags.locked;
}

void CDrawAdd::UpdateInfo()
{
	info.Format(_T("Og³: %ix%i"), sizex, sizey);
	if (typ_xx) info += _T(" niestandardowe");
	if (!nazwa.IsEmpty()) info.AppendFormat(_T(" | %s"), nazwa);
	if (nreps > 0) info.AppendFormat(_T(" | %i%s"), nreps, wersja);
	if (m_add_xx > 0) info.AppendFormat(_T(" | Spacer: %i"), m_add_xx);
	if (!logpage.IsEmpty()) info.AppendFormat(_T(" | %s"), logpage);
	if (!remarks.IsEmpty() || !remarks_atex.IsEmpty()) info.AppendFormat(_T(" | %s"), remarks.IsEmpty() ? remarks_atex : remarks);
	info.AppendFormat(_T(" | %s"), (kolor == c_full ) ? FULL : ((CMainFrame*)theApp.m_pMainWnd)->Spot_Kolor[(kolor>>3)]);
	if (fizpage) info.AppendFormat(_T(" | na str: %s (%i,%i)"), m_pDocument->GetPage(fizpage)->GetNrPaginy(), posx, posy);
}

CString CDrawAdd::PrepareBuf(TCHAR* ch) const
{
	CString bufor;
	bufor.AppendFormat(_T("%ix%i%s"), sizex, sizey, ch);
	if (theApp.includeKratka) 
		bufor.AppendFormat(_T("%ix%i%s"), szpalt_x, szpalt_y, ch);

	if ((fizpage & c_rzym) == c_rzym)
		bufor.Append(CDrawObj::Rzymska(fizpage >> 3));
	else
		bufor.AppendFormat(_T("%i"), fizpage >> 3);

	bufor.AppendFormat(_T("%s%i%s%i%s%s%s"), ch, posx, ch, posy, ch, nazwa, ch);
	if (nreps > -1)
		bufor.AppendFormat(_T("%i"), nreps);
	bufor.AppendFormat(_T("%s%s%s%s%s%s%s%s\n"), ch,  ((CMainFrame*)AfxGetMainWnd())->Spot_Kolor[kolor == c_full ? 1 : kolor >> 3], ch, logpage, ch, remarks, ch, wersja);

	return bufor;
}

BOOL CDrawAdd::OnOpen(CDrawView* pView)
{
	ASSERT_VALID(this);

	if (CSpacerDlg::Deal(this)) return TRUE;

	CAddDlg dlg;
	dlg.m_pub_xx = m_pub_xx;
	dlg.m_kolor = kolor;
	dlg.m_nazwa = nazwa;
	dlg.m_wersja = wersja;
	(nreps == -1) ? dlg.m_nreps.Empty() : dlg.m_nreps.Format(_T("%li"), nreps);
	dlg.m_epsok = flags.epsok;
	dlg.m_logpage = logpage;
	dlg.m_remarks = remarks;
	dlg.m_uwagi_atex = remarks_atex;
	dlg.m_rzymnum = ((fizpage & c_rzym) == c_rzym);
	dlg.m_fizpage = (fizpage >> 3);
	dlg.m_posx = posx;
	dlg.m_posy = posy;
	dlg.m_sizex = sizex;
	dlg.m_sizey = sizey;
	dlg.m_txtposx = txtposx;
	dlg.m_txtposy = txtposy;
	dlg.m_locked = flags.locked;
	dlg.m_szpalt_x = szpalt_x;
	dlg.m_szpalt_y = szpalt_y;
	dlg.m_typ_xx = typ_xx;
	dlg.m_flaga_rezerw = flags.reserv;
	dlg.m_add_xx = m_add_xx;
	dlg.m_fromQue = !pView;
	dlg.m_always = czaskto.Find('#') >= 0;
	if (!dlg.m_always) {
		int d, m, r, g, min;
		if (_stscanf_s(czaskto, c_formatCzasu, &d, &m, &r, &g, &min) == 5)
			dlg.m_godz_czob = dlg.m_data_czob = CTime(r, m, d, g, min, 0);
	}
	dlg.m_sprzedal = czaskto.Mid(czaskto.Find(_T('[')) + 1);
	dlg.m_sprzedal.SetAt(dlg.m_sprzedal.GetLength() - 1, _T(' '));
	dlg.m_zagroz = flags.zagroz;
	dlg.m_digital = flags.digital;
	dlg.m_powt = powtorka;
	dlg.m_oldadno = oldAdno > -1 ? oldAdno : nreps;
	dlg.m_studio = flags.studio;
	dlg.m_derived = flags.derived;
	dlg.m_spad = spad_flag;
	dlg.m_nag_xx = nag_xx;
	if (dlg.DoModal() != IDOK)
		return FALSE;

	nreps = dlg.m_nreps.IsEmpty() ? -1 : _ttol(dlg.m_nreps);
	oldAdno = dlg.m_oldadno == nreps ? -1 : dlg.m_oldadno;
	if (flags.isok == 0 && nreps != -1) flags.isok=3 ;
	if (flags.isok == 1 && nreps != -1) flags.isok=2 ;
	if (flags.isok == 2 && nreps == -1) flags.isok=1 ;
	nazwa = dlg.m_nazwa;
	wersja = dlg.m_wersja.Trim(); 
	kolor = dlg.m_kolor;
	logpage = dlg.m_logpage;
	remarks = dlg.m_remarks;
	remarks_atex = dlg.m_uwagi_atex;
	txtposx = dlg.m_txtposx;
	txtposy = dlg.m_txtposy;
	typ_xx = dlg.m_typ_xx;
	kodModulu = dlg.m_kod_modulu;
	nag_xx = dlg.m_nag_xx;
	m_add_xx = dlg.m_add_xx;
	spad_flag = dlg.m_spad;
	powtorka = dlg.m_powt.GetTime() ? CTime(dlg.m_powt.GetYear(), dlg.m_powt.GetMonth(), dlg.m_powt.GetDay(), 1, 0, 0) : 0;
	if (flags.epsok != dlg.m_epsok) flags.epsok = dlg.m_epsok;
	if (dlg.m_epsok<2) flags.showeps = dlg.m_epsok;
	flags.reserv = dlg.m_flaga_rezerw ;
	flags.locked = (fizpage ? dlg.m_locked : FALSE);
	flags.studio = dlg.m_studio;
	dlg.m_always ? czaskto.Format(_T("# [%s]"), (LPCTSTR)dlg.m_sprzedal) : czaskto.Format(_T("%s %s [%s]"), dlg.m_data_czob.Format(c_ctimeData), dlg.m_godz_czob.Format(_T("%H:%M")), (LPCTSTR)dlg.m_sprzedal);
	int pom_fizpage=((dlg.m_fizpage<<3) + ((dlg.m_rzymnum) ? c_rzym : c_normal));
	SetPosition(pom_fizpage, dlg.m_posx, dlg.m_posy, dlg.m_sizex, dlg.m_sizey); // robi updateinfo
	if (!dlg.m_precel_flag.IsEmpty() && dlg.m_precel_flag != m_precel_flag) {
		m_precel_flag = dlg.m_precel_flag;
		if (fizpage)
			m_pDocument->GetPage(fizpage)->RemoveAdd(this);
		InitPrecel(dlg.m_precel_flag);
		if (fizpage) {
			auto pPage = m_pDocument->GetPage(fizpage);
			fizpage = 0;
			if (pPage->CheckSpace(this, posx, posy)) 
				pPage->AddAdd(this);
			else 
				m_position -= CSize(m_position.left - pPage->m_position.left, m_position.top - pPage->m_position.top);
			UpdateInfo() ;
			pPage->Invalidate();
		}
	} else if (dlg.m_precel_flag.IsEmpty()) {
		precelWertexCnt = precelRingCnt = 0;
		m_precel_flag.Empty();
	}
	if (szpalt_x != dlg.m_szpalt_x || szpalt_y != dlg.m_szpalt_y) {
		szpalt_x = dlg.m_szpalt_x ;
		szpalt_y = dlg.m_szpalt_y ;
		space = CFlag(sizex, sizey, szpalt_x, szpalt_y) ;
		CRect vPos(m_position.left, m_position.top, m_position.left + (int) (sizex*modulx), m_position.top - (int) (sizey*moduly));
		MoveTo(vPos) ;
	}
	if (!dlg.m_fromQue && (((theApp.grupa&R_DEA) > 0) || (theApp.grupa&R_KIE && m_pDocument->isRO)))
		if (m_add_xx > 0) {
			int eok = this->flags.epsok;
			int nrs = (int)((CMainFrame*)AfxGetMainWnd())->Spot_ID[((this->kolor)>>3)];
			int powt = (int)(this->powtorka == 0 ? 0 : (this->powtorka - CTime(POWTSEED_0)).GetDays());
			int ik = ((this->kolor) & c_full)==c_full ? c_full : (((this->kolor) & c_brak)==c_brak ? c_brak : c_spot);
			CString czk = this->flags.reserv == 1 ? _T("") : this->czaskto;

			CManODPNETParms orapar {
				{ CManODPNET::DbTypeInt32, &this->m_pub_xx },
				{ CManODPNET::DbTypeInt32, &ik },
				{ CManODPNET::DbTypeInt32, &nrs },
				{ CManODPNET::DbTypeInt32, &powt },
				{ CManODPNET::DbTypeInt32, &this->oldAdno },
				{ CManODPNET::DbTypeVarchar2, &this->remarks },
				{ CManODPNET::DbTypeVarchar2, &this->wersja },
				{ CManODPNET::DbTypeVarchar2, &czk },
				{ CManODPNET::DbTypeInt32, &eok }
			};
			theManODPNET.EI("begin spacer.update_reservation(:pub_xx,:ile_kol,:nr_spotu,:powtorka,:old_adno,:uwagi,:wersja,:czaskto,:eps_present); end;", orapar);
		} else 
			AfxMessageBox(_T("Nie mo¿na w ten sposób modyfikowaæ rezerwacji, która nie pochodzi za Spacera"));
		SetDirty(); UpdateInfo(); Invalidate();
		if (!pView) m_pDocument->ArrangeQue();
	return TRUE;
}

CFlag CDrawAdd::GetPlacementFlag() const {
	return GetPlacementFlag(posx, posy);
}

CFlag CDrawAdd::GetPlacementFlag(int px, int py) const
{
#ifdef DEBUG
	ASSERT(0 < px && 0 < py && px <= szpalt_x && py <= szpalt_y);
#endif
	return space << ((szpalt_y - sizey - py + 2)*szpalt_x - sizex - px + 1);
}

void CDrawAdd::SetPosition(int fizp, int px, int py ,int sx, int sy)
{
	// gdy zmiana przez open wlasnosci lub import
	dirty = TRUE;
	if (fizpage == fizp && posx == px && posy == py && sizex == sx && sizey == sy) {
		UpdateInfo(); return;
	}

	CRect m_pos = m_position;
	auto pPage = m_pDocument->GetPage(fizp);
	sx = min(szpalt_x, sx); px = min(px, szpalt_x+1-sx);
	sy = min(szpalt_y, sy); py = min(py, szpalt_y+1-sy);
	BOOL ok;
	if (pPage == nullptr)
		ok = (fizp == 0);
	else
		ok = pPage->FindSpace(this, &px, &py, sx, sy);

	if (!ok) {
		if (pPage == nullptr) {  //nie istnieje ta strona np. makieta ma 44 strony a chcemy postawic na 48
			SetSpaceSize(sx, sy);
			m_pos.SetRect(m_position.left, m_position.top, m_position.left+(int)(sizex*modulx), m_position.top-(int)(sizey*moduly));
		} else { //tzn pPage nie jest NULL ale nie mozemy postawiæ
			if (fizpage == 0) {
				SetSpaceSize(sx, sy);
				m_pos.SetRect(pPage->m_position.left, pPage->m_position.top, pPage->m_position.left+(int)(sizex*modulx), pPage->m_position.top-(int)(sizey*moduly));
			} else  {
				pPage = m_pDocument->GetPage(fizpage);
				m_pos.SetRect(pPage->m_position.left+(int)(modulx*(posx-1)), pPage->m_position.bottom + (int)(moduly*(szpalt_y-posy+1)),
					pPage->m_position.left+(int)(modulx*(posx+sizex-1)) , pPage->m_position.bottom + (int)(moduly*(szpalt_y-posy-sizey+1))) ;
			}
		}
		MoveTo(m_pos);
		UpdateInfo();
		return;
	}

	// usuwa z listy lub zwalnia space zajetosc
	if (fizpage != 0)
		m_pDocument->GetPage(fizpage)->RemoveAdd(this);

	// ustawia docelowe parametry
	posx = px; posy = py; 
	SetSpaceSize(sx, sy);

	if (pPage == nullptr) // tylko ok=(fizp==0) moge tylko postawic na bok, jezeli stawiam zle to cofam -not ok
		if (fizpage != 0) { // stawiam na bok
			pPage=m_pDocument->GetPage(fizpage);
			m_pos.SetRect(pPage->m_position.left, pPage->m_position.top, pPage->m_position.left + (int)(sizex*modulx), pPage->m_position.top - (int)(sizey*moduly));         
			posx = posy = fizpage = 0;
		} else	//zmieniam rozmiar
			m_pos.SetRect(m_position.left, m_position.top, m_position.left+(int)(sizex*modulx), m_position.top-(int)(sizey*moduly));
	else {
		m_pos.SetRect(pPage->m_position.left+(int)(modulx*(posx-1)), pPage->m_position.bottom + (int)(moduly*(szpalt_y-posy+1)), pPage->m_position.left + (int)(modulx*(posx+sizex-1)) , pPage->m_position.bottom + (int)(moduly*(szpalt_y-posy-sizey+1))) ; 
		pPage->AddAdd(this);
	}

	MoveTo(m_pos);
	UpdateInfo();
}

void CDrawAdd::SetPosition(CRect* m_pos, CDrawPage* pPage) 
{
	int sx = min((int)ceil(abs(m_pos->right - m_pos->left) / modulx - 0.5), szpalt_x);
	int sy = min((int)ceil(abs(m_pos->top - m_pos->bottom) / moduly - 0.5), szpalt_y);
	int px = 0;
	int py = 0;
	BOOL ok = TRUE;

	dirty = TRUE;
	if (!sx) sx = 1;
	if (!sy) sy = 1;

	if (pPage && (szpalt_x != pPage->szpalt_x || szpalt_y != pPage->szpalt_y)) {
		*m_pos -= CPoint(m_pos->left - pPage->m_position.left, m_pos->top - pPage->m_position.top);
		pPage = nullptr ;
	}

	if (pPage != nullptr) {
		px = (int) ceil((m_pos->left - pPage->m_position.left) / modulx - 0.5) + 1;
		auto cx = szpalt_x + 1 - sx;
		if (cx < px) px = cx;
		py = szpalt_y - sy - (int) ceil((m_pos->bottom - pPage->m_position.bottom) / moduly - 0.5) + 1;
		auto cy = szpalt_y + 1 - sy;
		if (cy < py) py = cy;
		ok = pPage->FindSpace(this, &px, &py, sx, sy);
	}

	if (!ok) { // pPage nie jest NULL ale nie mozemy postawiæ
		if (fizpage == 0) { // tworzymy nowe albo przesuwamy z boku
			SetSpaceSize(sx, sy);
			m_pos->SetRect(pPage->m_position.left, pPage->m_position.top, pPage->m_position.left + (int) (sizex*modulx), pPage->m_position.top - (int) (sizey*moduly));
		} else {
			pPage = m_pDocument->GetPage(fizpage);
			m_pos->SetRect(pPage->m_position.left + (int) (modulx*(posx - 1)), pPage->m_position.bottom + (int) (moduly*(szpalt_y - posy + 1)), pPage->m_position.left + (int) (modulx*(posx + sizex - 1)), pPage->m_position.bottom + (int) (moduly*(szpalt_y - posy - sizey + 1)));
		}
		UpdateInfo();
		return;
	}

	// zwalnia space zajetosc
	if (fizpage)
		m_pDocument->GetPage(fizpage)->RemoveAdd(this);

	// ustawia docelowe parametry
	posx = px; posy = py; 
	SetSpaceSize(sx, sy);

	if (pPage == nullptr) {
		m_pos->SetRect(m_pos->left, m_pos->top, m_pos->left + (int) (sizex*modulx), m_pos->top - (int) (sizey*moduly));
		fizpage = 0;
	} else {
		pPage->AddAdd(this);
		m_pos->SetRect(pPage->m_position.left+(int)(modulx*(posx-1)), pPage->m_position.bottom + (int)(moduly*(szpalt_y-posy+1)), pPage->m_position.left + (int)(modulx*(posx+sizex-1)) , pPage->m_position.bottom + (int)(moduly*(szpalt_y-posy-sizey+1))) ;
	}

	UpdateInfo();
}

void CDrawAdd::SetSpaceSize(int sx, int sy)
{
	sizex = sx; 
	sizey = sy;
	if (precelWertexCnt == 0)
		space = CFlag(sx, sy, szpalt_x, szpalt_y);
}

void CDrawAdd::SetSpaceAndPosition(int fizp, int px, int py)
{
	// gdy copy paste i save i open
	dirty = TRUE;
	if (!precelWertexCnt)
		space = CFlag(sizex, sizey, szpalt_x, szpalt_y);
	auto pPage = m_pDocument->GetPage(fizp);
	if (pPage == nullptr)
		return;
	pPage->SetBaseKrata(szpalt_x, szpalt_y, FALSE) ;
	if (pPage->FindSpace(this, &px, &py, sizex, sizey)) {
		posx = px; posy = py;
		pPage->AddAdd(this);
		m_position.SetRect(pPage->m_position.left + (int)(modulx*(posx - 1)), pPage->m_position.bottom + (int)(moduly*(szpalt_y - posy + 1)),
						   pPage->m_position.left + (int)(modulx*(posx + sizex - 1)), pPage->m_position.bottom + (int)(moduly*(szpalt_y - posy - sizey + 1)));
	} else
		m_position.SetRect(pPage->m_position.left,pPage->m_position.top,pPage->m_position.left+(int)(sizex*modulx),pPage->m_position.top-(int)(sizey*moduly)); 
	UpdateInfo();
}

BOOL CDrawAdd::PtOnRing(CPoint p) const {
	for (int i = 0; i < precelWertexCnt - 1; i++) {
		if (aPrecelWertex[i].y != aPrecelWertex[i+1].y || aPrecelWertex[i].y != p.y) 
			continue;
		int r1 = aPrecelWertex[i].x;
		int r2 = aPrecelWertex[i+1].x;
		if (r1 > r2) std::swap(r1, r2);
		if (r1 <= p.x && p.x + 1 <= r2)
			return TRUE;
	}
	return FALSE; 
}

int CDrawAdd::FindRing(CPoint p0, BOOL bOuterRing) {
	enum Kierunek { E, S, W, N }; // okresla kierunek ostatnio znalezionego fragmentu obwodnicy
	Kierunek eKierunek = E;
	int iInitVertexCnt = precelWertexCnt;
	aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p0.x*modulx), static_cast<int>(-p0.y*moduly));
	CPoint p(p0.x + 1, p0.y);
	aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));

	ASSERT(0 <= p0.x && 0 <= p0.y && p0.x <= sizex && p0.y <= sizey && precelRingCnt<ciMaxRings);

	while (p != p0) {
		switch (eKierunek) {
		case E:
			if (p.y>0 && p.x<szpalt_x && space.GetBit((p.y - 1)*szpalt_x + p.x)>0 == bOuterRing) {
				p.y--;
				eKierunek = N;
				aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
			} else if (p.x < sizex && p.y<szpalt_y && space.GetBit(p.y*szpalt_x + p.x)>0 == bOuterRing) {
				p.x++;
				if (aPrecelWertex[precelWertexCnt - 2].y == -p.y*moduly)
					aPrecelWertex[precelWertexCnt - 1] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
				else
					aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
			} else {
				ASSERT(p.y < sizey);
				p.y++;
				eKierunek = S;
				aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
			}
			break;
		case S:
			if (p.x < sizex && p.y<sizey && space.GetBit(p.y*szpalt_x + p.x)>0 == bOuterRing) {
				p.x++;
				eKierunek = E;
				aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
			} else if (p.y<sizey && space.GetBit(p.y*szpalt_x + p.x - 1)>0 == bOuterRing) {
				p.y++;
				if (aPrecelWertex[precelWertexCnt - 2].x == p.x*modulx)
					aPrecelWertex[precelWertexCnt - 1] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
				else
					aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
			} else {
				ASSERT(p.x - 1 >= 0);
				p.x--;
				eKierunek = W;
				aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
			}
			break;
		case W:
			if (p.x > 0 && p.y<sizey && space.GetBit(p.y*szpalt_x + p.x - 1)>0 == bOuterRing) {
				p.y++;
				eKierunek = S;
				aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
			} else if (p.x > 0 && p.y > 0 && space.GetBit((p.y - 1)*szpalt_x + p.x - 1) > 0 == bOuterRing) {
				p.x--;
				if (aPrecelWertex[precelWertexCnt - 2].y == -p.y*moduly)
					aPrecelWertex[precelWertexCnt - 1] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
				else
					aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
			} else {
				ASSERT(p.y > 0);
				p.y--;
				eKierunek = N;
				aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
			}
			break;
		case N:
			if (p.x > 0 && p.y > 0 && space.GetBit((p.y - 1)*szpalt_x + p.x - 1) > 0 == bOuterRing) {
				p.x--;
				eKierunek = W;
				aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
			} else if (p.y > 0 && space.GetBit((p.y - 1)*szpalt_x + p.x) > 0 == bOuterRing) {
				p.y--;
				if (aPrecelWertex[precelWertexCnt - 2].x == p.x*modulx)
					aPrecelWertex[precelWertexCnt - 1] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
				else
					aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
			} else {
				ASSERT(p.x + 1 <= sizex);
				p.x++;
				eKierunek = E;
				aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
			}
			break;
		}
	}

	return --precelWertexCnt - iInitVertexCnt;
}

void CDrawAdd::InitPrecel(const CString& sPrecelFlag) {
	m_precel_flag = sPrecelFlag;
	precelWertexCnt = precelRingCnt = 0;
	// zarzadanie pamiecia
	size_t wertexCnt = (sizex + 1)*(sizey + 1);
	aPrecelWertex = std::make_unique<CPoint []>(wertexCnt + 2); // na koñcu int[4]
	aRingWertexCnt = reinterpret_cast<int*>(&aPrecelWertex[wertexCnt]);
	// poczatkowo precel jest rowny jego otoczeniu i trzeba wylaczyc niektore moduly
	space = CFlag(sizex, sizey, szpalt_x, szpalt_y);
	// zera do ró¿nicy d³ugoœci + (zera do pe³nych 32 bitów + bity flagi)
	space ^= CFlag(CString('0', 8*((int)space.GetSize()/4-(int)ceil((float)sPrecelFlag.GetLength()/8))) + CString('0', (8888-sPrecelFlag.GetLength()) % 8) + sPrecelFlag);

	// skanuj flagê w poszukiwaniu obwodnic
	for (int x = 0; x < sizex; x++)
		if (space.GetBit(x) && !PtOnRing(CPoint(static_cast<int>(x*modulx), 0)))
			aRingWertexCnt[precelRingCnt++] = FindRing(CPoint(x, 0), TRUE);
	for (int y = 1; y < sizey; y++)
		for (int x = 0; x < sizex; x++) {
			if (space.GetBit((y - 1)*szpalt_x + x) == 0 && space.GetBit(y*szpalt_x + x)>0 && !PtOnRing(CPoint(static_cast<int>(x*modulx), static_cast<int>(-y*moduly)))) {
				aRingWertexCnt[precelRingCnt++] = FindRing(CPoint(x, y), TRUE);
				continue;
			}
			if (space.GetBit((y - 1)*szpalt_x + x)>0 && space.GetBit(y*szpalt_x + x) == 0 && !PtOnRing(CPoint(static_cast<int>(x*modulx), static_cast<int>(-y*moduly))))
				aRingWertexCnt[precelRingCnt++] = FindRing(CPoint(x, y), FALSE);
		}

	// wyrownanie do kraty	
	int iRightBound = static_cast<int>(sizex * modulx);
	for (int i = 0; i < precelWertexCnt; i++) {
		if (aPrecelWertex[i].x == iRightBound) aPrecelWertex[i].x -= 11;
		//if (aPrecelWertex[i].y == sizey*moduly) aPrecelWertex[i].y+=3;
	}
	// dosun mape precla do prawej krawedzi strony
	space.Reverse(szpalt_x * sizey);
	space >>= szpalt_x - sizex;
}

void CDrawAdd::MoveWithPage(const CRect& position, CDrawView* pView)
{
	dirty = TRUE;
	CRect toPos(position.left + (int) (modulx*(posx - 1)), position.bottom + (int) (moduly*(szpalt_y - posy + 1)), position.left + (int) (modulx*(posx + sizex - 1)), position.bottom + (int) (moduly*(szpalt_y - posy - sizey + 1)));
	MoveTo(toPos, pView);
}

void CDrawAdd::SetLogpage(CString& m_op_zew, CString& m_sekcja, CString& m_op_sekcji, int m_nr_w_sekcji, CString& m_PL, CString& m_op_PL, int m_nr_PL, CString& m_poz_na_str)
{
	/* vu : Formatuje wartosci dostarczone na zmiennych tak, aby powstal napis warunkow logicznych
			Zalozenie:   SetLogpage o ParseLogpage = Id									end vu */

	dirty = TRUE;
	logpage = m_op_zew;
	if (!m_sekcja.IsEmpty()) {
		logpage += m_sekcja;
		if (m_op_sekcji.GetLength() == 1 && (m_op_sekcji[0] == _T('=') || m_op_sekcji[0] == _T('<') || m_op_sekcji[0] == _T('>'))) {
			logpage += m_op_sekcji;
			if (m_nr_w_sekcji > 0) {
				CString snr;
				snr.Format(_T("%i"), m_nr_w_sekcji);
				logpage += snr;
			}
		}
		logpage += CString(_T(" "));
	}
	if (m_PL == _T("P") || m_PL == _T("L")) {
		logpage += m_PL ;
		if (m_op_PL.GetLength()==1 && (m_op_PL[0] == _T('=') || m_op_PL[0] == _T('<') || m_op_PL[0] == _T('>'))) {
			logpage += m_op_PL ;
			if (m_nr_PL > 0) {
				CString snr;
				snr.Format(_T("%i"), m_nr_PL);
				logpage += snr ;
			}
		}
		logpage += CString(_T(" ")) ;
	}   
	if (m_poz_na_str.GetLength() > 0) {
		m_poz_na_str.MakeUpper();
		if (m_poz_na_str == _T("G")  || m_poz_na_str == _T("D")  || m_poz_na_str == _T("L")  || m_poz_na_str == _T("P")  ||
			m_poz_na_str == _T("DL") || m_poz_na_str == _T("DP") || m_poz_na_str == _T("GL") || m_poz_na_str == _T("GP") || 
				m_poz_na_str == _T("X")  || m_poz_na_str == _T("Y")  || m_poz_na_str == _T("XY")) 
				logpage += ("# " + m_poz_na_str) ;
	}
}

void CDrawAdd::ParseLogpage(TCHAR *op_zew, TCHAR *sekcja, TCHAR *op_sekcji, int *nr_sek, TCHAR *pl, TCHAR *op_pl, int *nr_pl, TCHAR *poz_na_str) 
{
	/* vu :	Obiekt ogloszenie ma pole CString logpage. Funkcja ParseLogpage interpretuje jego zawartosc zgodnie
		   	z sugestiami zawartymi w dokumencie warlog.doc i uzyskane wartosci przekazuje do argumentow funkcji. 
		   	Jako separator pomiedzy specyfikacja strony a atrybutem poz_na_str stosuje sie znak '#'		end vu */

	int j, i = 0 ;
	TCHAR ch, nr_buf[20], lp[50] ;

	::StringCchCopy(lp, 50, _T(" "));
	if (logpage.SpanExcluding(lp).GetLength() < 1) {
		op_zew[0] = sekcja[0] = op_sekcji[0] = pl[0] = op_pl[0] = TCHAR(0);
		if (poz_na_str) poz_na_str[0] = TCHAR(0);
		*nr_sek = *nr_pl = 0;
		return;
	}
	
	//potrzebuje duze litery i '\0' na koncu napisu
	if (logpage[0] == _T('"')) logpage.SetAt(0, _T(' '));
	logpage.MakeUpper() ;
	::StringCchCopy(lp, 50, logpage) ;

	//wczytaj op_zew jesli jest
	ch = lp[i] ;  
	op_zew[0] = TCHAR(0) ;
	while (ch == _T(' ') || ch == _T('<') || ch == _T('=') || ch == _T('>')) {
		if (ch != _T(' ')) { 
			op_zew[0] = ch ;
			op_zew[1] = TCHAR(0) ;
		}
		ch = lp[++i] ;
	}
		  
	//wczytaj sekcje jeœli jest okreœlona
	if (ch != _T('#') && !((ch == _T('L') || ch == _T('P')) && !_istalpha(lp[i + 1])) ) { //sekcja okreslona
		j = 0;
		while ((int8_t)ch < 0 || isalnum((int)ch) || ch == _T('/')) { //ch<0 to polski znak
			sekcja[j++] = ch ;
			ch = lp[++i] ;
		}
		sekcja[j] = TCHAR(0) ;

		//wczytaj op_sekcji
		while (ch != TCHAR(0) && ch != _T('<') && ch != _T('=') && ch != _T('>') && ch != _T('L') && ch != _T('P') && ch != _T('#')) 
			ch = lp[++i] ;
		if (ch == TCHAR(0) || ch == _T('#') || ch == _T('L') || ch == _T('P')) op_sekcji[0] = TCHAR(0) ;
		else {
			op_sekcji[0] = ch ;
			op_sekcji[1] = TCHAR(0) ;
		}

		//wczytaj nr_sek
		if (op_sekcji[0] != TCHAR(0)) {
			j = 0;
			while (ch != _T('#') && ch != TCHAR(0) && !_istdigit(ch))
				ch = lp[++i];
			if (ch == '#' || ch == '\0') *nr_sek = 0;
			else {
				nr_buf[j++] = ch;
				ch = lp[++i];
				while (ch != '#' && ch != '\0' && _istdigit(ch)) {
					nr_buf[j++] = ch;
					ch = lp[++i];
				}
				nr_buf[j] = '\0';
				*nr_sek = _ttoi(nr_buf);
			}
		} else *nr_sek = 0;
	} else {
		sekcja[0] = op_sekcji[0] = '\0'; // end of wczytaj sekcje jeœli jest okreœlona
		*nr_sek = 0;
	}

	//wczytaj lp
	while (ch != '\0' && ch != 'L' && ch != 'P' && ch != '#')
		ch = lp[++i];
	if (ch != '#' && ch != '\0') { //wczytaj op_lp
		pl[0] = ch;
		pl[1] = '\0';
		while (ch != '#' && ch != '\0' && ch != '<' && ch != '=' && ch != '>')
			ch = lp[++i];
		if (ch != '#' && ch != '\0') { //wczytaj nr_lp
			op_pl[0] = ch;
			op_pl[1] = '\0';
			while (ch != '#' && ch != '\0' && !_istdigit(ch))
				ch = lp[++i];
			if (ch == '#' || ch == '\0') {
				*nr_pl = 0;
			} else {
				j = 0;
				nr_buf[j++] = ch;
				ch = lp[++i];
				while (ch != '#' && ch != '\0' && _istdigit(ch)) {
					nr_buf[j++] = ch;
					ch = lp[++i];
				}
				nr_buf[j] = '\0';
				*nr_pl = _ttoi(nr_buf);
			}
		} else {
			op_pl[0] = '\0';
			*nr_pl = 0;
		}
	} else {
		pl[0] = op_pl[0] = '\0';
		*nr_pl = 0;
	}

	// wczytaj poz_na_str
	if (!poz_na_str) return; // ten atrybut jest istotny tylko w przypadku makietowania strony - NULL default
	while (ch != '\0' && ch != '#')
		ch = lp[++i];
	if (ch != '#') poz_na_str[0] = '\0';
	else {
		ch = lp[++i];
		while (ch != '\0' && !isupper((int) ch))
			ch = lp[++i];
		poz_na_str[0] = '\0';
		switch (ch) {
			case 'G': // G,GP,GL,D,DP,DL
			case 'D':
				if (lp[i + 1] == '\0' || lp[i + 1] == ' ') { ::StringCchCopyN(poz_na_str, 2, &lp[i], 1); poz_na_str[1] = '\0'; };
				if (lp[i + 1] == 'L' || lp[i + 1] == 'P')
					if (lp[i + 2] == '\0' || lp[i + 2] == ' ') { ::StringCchCopyN(poz_na_str, 2, &lp[i], 2); poz_na_str[2] = '\0'; };
				break;
			case 'X': // X,XY
				if (lp[i + 1] == '\0' || lp[i + 1] == ' ') { ::StringCchCopyN(poz_na_str, 2, &lp[i], 1); poz_na_str[1] = '\0'; };
				if (lp[i + 1] == 'Y')
					if (lp[i + 2] == '\0' || lp[i + 2] == ' ') { ::StringCchCopyN(poz_na_str, 2, &lp[i], 2); poz_na_str[2] = '\0'; };
				break;
			case 'Y': // Y, L, P
			case 'L':
			case 'P':
				if (lp[i + 1] == '\0' || lp[i + 1] == ' ') { ::StringCchCopyN(poz_na_str, 2, &lp[i], 1); poz_na_str[1] = '\0'; };
		}
	}
}

int CDrawAdd::CkPageLocation(int vFizPage) 
{
	/* vu: 	Sprawdza czy ogloszenie vAdd moze stac na stronie o numerze vFizPage 
			ze wzgledu na warunki logiczne ogloszenia i strony. Jezeli nie moze,
			to zwraca 2, jezeli moze, ale stoi na dobrej stronie nieprawidlowo
			zwraca 1, a jezeli wszystko jest w porzadku zwraca 0		end vu */

	CDrawPage *vPage;
	int pc, nr_sek, nr_pl, nr_off, vnr_sek = 0;
	TCHAR op_zew[2], op_sekcji[2], op_pl[2], sekcja[30], pl[2];

	//gdy nie stoi na ¿adnej stronie, to dobrze
	if (!vFizPage) return 0;
	//parsuj napis i ustaw zmienne 
	ParseLogpage(op_zew, sekcja, op_sekcji, &nr_sek, pl, op_pl, &nr_pl);
	//przejdz liste stron w dokumencie, zeby zapamietac jaki numer ma strona w danej sekcji 
	pc = (int)m_pDocument->m_pages.size();
	for (int j = 1; j <= pc; j++)	{
		vPage = m_pDocument->m_pages[j % pc] ;
		CString vStrLog(vPage->name) ;
		vStrLog.MakeUpper() ;
		//zliczaj strony z odpowiedni¹ sekcj¹
		if (_tcsstr(vStrLog,sekcja)) 
			vnr_sek++;
		//czy to jest testowana strona ? otherwise nie ma co sprawedzac
		if (vPage->nr != vFizPage) continue ;
		//nie zgodne kolory - kolor strony 2 oznacza spot anonimowy
		if ((kolor&7)>(vPage->kolor&7) ||
			((kolor&c_spot) && (vPage->kolor&c_spot) && (kolor>>3)!=m_pDocument->m_spot_makiety[(vPage->kolor>>3)] && m_pDocument->m_spot_makiety[(vPage->kolor>>3)])) return 2 ;
		//na stronach z dziedziczeniem koloru tylko czarne og³oszenia
		if (vPage->m_dervlvl == DERV_COLO && kolor != c_brak) return 2 ;
		//nie na sciezce
		if (sekcja[0] && !_tcsstr(vStrLog,sekcja)) return 2 ;
		//lewa - nieparzysty numer
		if (!_tcscmp(pl, _T("L")) && ((vPage->nr>>3)%2)) return 2 ;
		//prawa - parzysty numer
		if (!_tcscmp(pl, _T("P")) && !((vPage->nr>>3)%2)) return 2 ;
		//polozenie pl
		if (pl[0] && op_pl[0]) { 
			//brak numeru
			if (nr_pl<1) return 2 ;
			if (((vPage->nr>>3) - vnr_sek)%2)	//czy sekcja zaczyna sie na stronie o parzstym numerze
				nr_off = (sekcja[0] ? (((vPage->nr>>3)%2) ? vnr_sek-1 : vnr_sek+1) : vPage->nr>>3) ; //czy numer odnosi sie do sekcji czy do gazety
			else
				nr_off = (sekcja[0] ? vnr_sek : vPage->nr>>3) ; //czy numer odnosi sie do sekcji czy do gazety
			//inna strona prawa/lewa niz zadano
			if (!_tcscmp(op_pl, _T("=")) && !_tcscmp(pl, _T("P")) && nr_off!=2*nr_pl-1) return 2 ;
			if (!_tcscmp(op_pl, _T("=")) && !_tcscmp(pl, _T("L")) && nr_off!=2*nr_pl) return 2 ;
			//strona prawa/lewa o mniejszym numerze niz zadano
			if (!_tcscmp(op_pl, _T(">")) && !_tcscmp(pl, _T("P")) && nr_off<=2*nr_pl-1) return 2 ;
			if (!_tcscmp(op_pl, _T(">")) && !_tcscmp(pl, _T("L")) && nr_off<=2*nr_pl) return 2 ;
			//strona prawa/lewa o wiekszym numerze niz zadano
			if (!_tcscmp(op_pl, _T("<")) && !_tcscmp(pl, _T("P")) && nr_off>=2*nr_pl-1) return 2 ;
			if (!_tcscmp(op_pl, _T("<")) && !_tcscmp(pl, _T("L")) && nr_off>=2*nr_pl) return 2 ;
		}
		//polozenie w sekcji
		if (sekcja[0] && op_sekcji[0]) {
			//brak numeru
			if (nr_sek < 1) return 2 ;
			//inna strona w sekcji niz zadano
			if (!_tcscmp(op_sekcji, _T("=")) && vnr_sek != nr_sek) return 2 ;
			//strona o mniejszym numerze w sekcji niz zadano
			if (!_tcscmp(op_sekcji, _T(">")) && vnr_sek <= nr_sek) return 2 ;
			//strona o wiekszym numerze w sekcji niz zadano
			if (!_tcscmp(op_sekcji, _T("<")) && vnr_sek >= nr_sek) return 2 ;
		}
	}
	
	//jest na dobrej stronie, ale czy w dobrym miejscu?
	int hashpos = logpage.ReverseFind(_T('#')) ;
	if (hashpos >= 0) {
		int j, k = 0;
		TCHAR tail[20] ;
		for (j = hashpos + 1; j < logpage.GetLength(); j++)
			if (isupper((int)logpage[j])) tail[k++] = logpage[j] ;
		tail[k] = TCHAR(0) ;
		if (_tcsstr(tail, _T("G")))
			if (posy>1) return 1 ;
		if (_tcsstr(tail, _T("D")))
			if (posy+sizey-1<szpalt_y) return 1 ;
		if (_tcsstr(tail, _T("L")))
			if (posx>1) return 1 ;
		if (_tcsstr(tail, _T("P")))
			if (posx+sizex-1<szpalt_x) return 1 ;
		if (_tcsstr(tail, _T("GP")))
			if (posy>1 || posx+sizex-1 < szpalt_x) return 1 ;
		if (_tcsstr(tail, _T("GL")))
			if (posx>1 || posy>1) return 1 ;
		if (_tcsstr(tail, _T("DL")))
			if (posx>1 || posy+sizey-1 < szpalt_y) return 1 ;
		if (_tcsstr(tail, _T("DP")))
			if (posx+sizex-1 < szpalt_x || posy+sizey-1 < szpalt_y) return 1 ;
	}
	
	return 0 ;	//wszystko w porz¹dku
}

void CDrawAdd::SetEstPagePos(TCHAR *description, CRect *vRect, CDrawPage *pPage) //vu  
{
	/* vu : Jako description - opis polozenia ogloszenia na stronie podaje sie
			jeden ze skrotow in ('P','L','G','D','Gl','GP','DL','DP')
			Algorytm szuka polozenia dla ogloszenia na stronie pPage najblizszego
			oczekiwaniom z description. Jezeli znajdzie, to stawia ogloszenie na 
			stronie														end vu */

	int i, j;
	dirty = TRUE;

	if (!_tcscmp(description, _T("P")))  			//na prawo - domyslnie od dolu
		for (i = 0; i <= szpalt_x - sizex; i++)  			//dla kazdego polozenia w poziomie
			for (j = 0; j <= szpalt_y - sizey; j++)			//dla kazdego polozenia w pionie
				if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i, szpalt_y - sizey + 1 - j))
					goto postaw;

	if (!_tcscmp(description, _T("L")))  			//na lewo - domyslnie od dolu
		for (i = szpalt_x - sizex; i >= 0; i--)  			//dla kazdego polozenia w poziomie
			for (j = 0; j <= szpalt_y - sizey; j++)			//dla kazdego polozenia w pionie
				if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i, szpalt_y - sizey + 1 - j))
					goto postaw;

	if (!_tcscmp(description, _T("D")))  			//u dolu - domyslnie od prawej
		for (j = 0; j <= szpalt_y - sizey; j++)  			//dla kazdego polozenia w pionie
			for (i = 0; i <= szpalt_x - sizex; i++)			//dla kazdego polozenia w poziomie
				if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i, szpalt_y - sizey + 1 - j))
					goto postaw;

	if (!_tcscmp(description, _T("G")))  			//u gory - domyslnie od prawej
		for (j = szpalt_y - sizey; j >= 0; j--)  			//dla kazdego polozenia w pionie
			for (i = 0; i <= szpalt_x - sizex; i++)			//dla kazdego polozenia w poziomie
				if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i, szpalt_y - sizey + 1 - j))
					goto postaw;

	if (!_tcscmp(description, _T("DP")))  			//na dole i na prawo - domyslnie na NE
		for (i = 0; i <= szpalt_x - sizex; i++)	  			//dla kazdego polozenia w poziomie
			for (j = 0; j <= i; j++)						//dla kazdego polozenia w pionie
				if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i + j, szpalt_y - sizey + 1 - j)) {
					i -= j; goto postaw;
				}

	if (!_tcscmp(description, _T("DL")))  			//na dole i na lewo - domyslnie na NW
		for (i = szpalt_x - sizex; i >= 0; i--)  			//dla kazdego polozenia w poziomie
			for (j = 0; j <= szpalt_x - sizex - i; j++) 		//dla kazdego polozenia w pionie
				if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i - j, szpalt_y - sizey + 1 - j)) {
					i += j; goto postaw;
				}

	if (!_tcscmp(description, _T("GP")))  			//na gorze i na lewo - domyslnie na EW
		for (i = 0; i <= szpalt_x - sizex + 1; i++)  			//dla kazdego polozenia w poziomie
			for (j = szpalt_y - sizey; j >= szpalt_y - sizey - i; j--)	//dla kazdego polozenia w pionie
				if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i + szpalt_y - sizey + 1 - j, szpalt_y - sizey + 1 - j)) {
					i -= szpalt_y - sizey + 1 - j; goto postaw;
				}

	if (!_tcscmp(description, _T("GL")))  			//na gorze i na lewo - domyslnie na EW
		for (i = szpalt_x - sizex - 1; i >= -1; i--)  		//dla kazdego polozenia w poziomie
			for (j = szpalt_y - sizey; j >= i - szpalt_x + sizex + szpalt_y - sizey; j--)	//dla kazdego polozenia w pionie
				if (pPage->CheckSpace(this, szpalt_x - sizex - i - szpalt_y + sizey + j, szpalt_y - sizey + 1 - j)) {
					i += szpalt_y - sizey + 1 - j; goto postaw;
				}
	
	//nie udalo sie - postaw gdziekolwiek - zacznij od lewego dolnego rogu
	vRect->SetRect(pPage->m_position.left, pPage->m_position.bottom + (int) (sizey*moduly),
				   pPage->m_position.left + (int) (sizex*modulx), pPage->m_position.bottom);
	SetPosition(vRect, pPage) ;
	return ;

postaw:
	posx = szpalt_x - sizex + 1 - i;
	posy = szpalt_y - sizey + 1 - j;
	vRect->SetRect(pPage->m_position.left + (int) ((posx - 1)*modulx), pPage->m_position.bottom + (int) ((szpalt_y - posy + 1)*moduly),
				   pPage->m_position.left + (int) ((posx + sizex - 1)*modulx), pPage->m_position.bottom + (int) ((szpalt_y - posy - sizey + 1)*moduly));
	pPage->AddAdd(this);
}

BOOL CDrawAdd::SetPagePosition(CRect *vRect, CDrawPage *vPage) //vu  
{
	/*	vRect ? stawia : sprawdza */
	int j, k = 0;
	dirty = TRUE;
	BOOL mozna = FALSE ;
	TCHAR tail[20] ;
	int hashpos = logpage.ReverseFind('#') ;
	for (j = hashpos  +1; j < logpage.GetLength(); j++)
		if (isupper((int)logpage[j])) tail[k++] = logpage[j] ;
	tail[k] = '\0' ;	
	if (_tcsstr(tail, _T("DL")))
		mozna = SetStrictDescPos(_T("DL"), vRect, vPage);
	else if (_tcsstr(tail, _T("DP")))
		mozna = SetStrictDescPos(_T("DP"), vRect, vPage);
	else if (_tcsstr(tail, _T("D")))
		mozna = SetStrictDescPos(_T("D"), vRect, vPage);
	else if (_tcsstr(tail, _T("GP")))
		mozna = SetStrictDescPos(_T("GP"), vRect, vPage);
	else if (_tcsstr(tail, _T("GL")))
		mozna = SetStrictDescPos(_T("GL"), vRect, vPage);
	else if (_tcsstr(tail, _T("G")))
		mozna = SetStrictDescPos(_T("G"), vRect, vPage);
	else if (_tcsstr(tail, _T("L")))
		mozna = SetStrictDescPos(_T("L"), vRect, vPage);
	else if (_tcsstr(tail, _T("P")))
		mozna = SetStrictDescPos(_T("P"), vRect, vPage);
	return mozna ;
}

BOOL CDrawAdd::SetStrictDescPos(TCHAR *description, CRect *vRect, CDrawPage *pPage) //vu  
{
	/* vu : Jako description - opis polozenia ogloszenia na stronie podaje sie
			jeden ze skrotow in ('P','L','G','D','GL','GP','DL','DP')
			Algorytm szuka polozenia dla ogloszenia na stronie pPage 
			spe³niaj¹cego oczekiwaniom z description. Jezeli znajdzie, to stawia 
			ogloszenie na stronie je¿eli nie, to stawia obok makiety 	
			Jeœli !vRect to funkcja jedynie sprawdza dopuszczalnoœæ		end vu */

	int i, j;

	if (!_tcscmp(description, _T("P")))  				//na prawo - domyslnie od dolu
		for (j = 0; j <= szpalt_y - sizey; j++)			//dla kazdego polozenia w pionie     
			if (pPage->CheckSpace(this, szpalt_x - sizex + 1, szpalt_y - sizey + 1 - j)) {
				i = szpalt_x - sizex + 1; j = szpalt_y - sizey + 1 - j; goto postaw;
			}


	if (!_tcscmp(description, _T("L")))  				//na lewo - domyslnie od dolu
		for (j = 0; j <= szpalt_y - sizey; j++)			//dla kazdego polozenia w pionie
			if (pPage->CheckSpace(this, 1, szpalt_y - sizey + 1 - j)) {
				i = 1; j = szpalt_y - sizey + 1 - j; goto postaw;
			}

	if (!_tcscmp(description, _T("D")))  				//u dolu - domyslnie od prawej
		for (i = 0; i <= szpalt_x - sizex; i++)			//dla kazdego polozenia w poziomie
			if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i, szpalt_y - sizey + 1)) {
				i = szpalt_x - sizex + 1 - i; j = szpalt_y - sizey + 1; goto postaw;
			}

	if (!_tcscmp(description, _T("G")))  				//u gory - domyslnie od prawej
		for (i = 0; i <= szpalt_x - sizex; i++)			//dla kazdego polozenia w poziomie 
			if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i, 1)) {
				i = szpalt_x - sizex + 1 - i; j = 1; goto postaw;
			}

	if (!_tcscmp(description, _T("DP")))  				//na dole i na prawo - domyslnie na NE
		if (pPage->CheckSpace(this, szpalt_x - sizex + 1, szpalt_y - sizey + 1)) {
			i = szpalt_x - sizex + 1; j = szpalt_y - sizey + 1; goto postaw;
		}

	if (!_tcscmp(description, _T("DL")))  				//na dole i na lewo - domyslnie na NW
		if (pPage->CheckSpace(this, 1, szpalt_y - sizey + 1)) {
			i = 1; j = szpalt_y - sizey + 1; goto postaw;
		}

	if (!_tcscmp(description, _T("GP")))  				//na gorze i na lewo - domyslnie na EW
		if (pPage->CheckSpace(this, szpalt_x - sizex + 1, 1)) {
			i = szpalt_x - sizex + 1; j = 1; goto postaw;
		}

	if (!_tcscmp(description, _T("GL")))  				//na gorze i na lewo - domyslnie na EW
		if (pPage->CheckSpace(this, 1, 1)) {
			i = 1; j = 1; goto postaw;
		}
	
	//nie udalo sie postawic
	return FALSE;

postaw:
	if (vRect) {
		posx = i;
		posy = j;
		vRect->SetRect(pPage->m_position.left+(int)((posx-1)*modulx), pPage->m_position.bottom+(int)((szpalt_y-posy+1)*moduly), 
					   pPage->m_position.left+(int)((posx+sizex-1)*modulx), pPage->m_position.bottom+(int)((szpalt_y-posy-sizey+1)*moduly)) ;
		pPage->AddAdd(this) ;
	}
	return TRUE;
}

void CDrawAdd::SetDotM(BOOL setFlag) {
	int p = this->wersja.Find('m');
	if (setFlag) {
		if (p >= 0 || wersja.Find('z') >= 0) return;
		p = wersja.Find('.');
		if (p == -1) 
			wersja += ".m";
		else 
			wersja = wersja.Left(p) + ".m" + wersja.Mid(p + 1);
		SetDirty();
	} else { 
		if (p == -1) return;
		wersja = wersja.Left(p) + wersja.Mid(p + 1);
		SetDirty();
	}
}

//////////////////////// GEN 
BOOL CDrawAdd::BBoxFromFile(PGENEPSARG pArg, CFile *handle, float *x1, float *y1, float *x2, float *y2) { 
	const int len = 30; //bo obetniemy w srodku wartosci
	BOOL bRet = false;
	size_t res = 1;
	char *p, *s = reinterpret_cast<char*>(pArg->cBigBuf);
	unsigned long filepos = (unsigned long)handle->GetPosition();

	while (res > 0)  { // read block from the file
		res = handle->Read(s, n_size);    // check if anything was read and no error occurred
		if((res > 0)&&(res != unsigned(-1)))     {      // move file pointer back, lest we can loose string read in partially
			if (res == n_size) handle->Seek(-len, CFile::current);      // buffer has to end with zero TCHAR to be treated as string
			s[res] = 0;
		} else break;// break on error or EOF
		if((p = (char*)CManPDF::memstr(s, "%BoundingBox:", res)) != nullptr && s + n_size - p >= len)    { // search for substring and get it's position
			bRet = sscanf_s(p+14, "%f %f %f %f", x1, y1, x2, y2) == 4; //14 == _tcslen("%BoundingBox:")+1
			break;
		}
	}
	handle->Seek(filepos, CFile::begin);
	return bRet;
}

BOOL CDrawAdd::GetProdInfo(PGENEPSARG pArg, TCHAR *cKolor, float *bx1, float *by1, float *bx2, float *by2, int *ileMat) 
{
	BOOL bReqProdInfo = false;
	// szukaj danych w bazie
	if (m_pub_xx > 0) {
		CString sBBox = theManODPNET.GetProdInfo(m_pub_xx, cKolor, ileMat);
		if (sBBox != _T("!")) {
			pArg->bStatus = _stscanf_s(sBBox, _T("%f %f %f %f"), bx1, by1, bx2, by2) == 4;
			if (!pArg->bStatus) { // przesz³o przez EpsTest, ale nie ma ProdInfo
				bReqProdInfo = true;
				if (powtorka > 0) EpsName(F_EPS, FALSE);
				CManODPNETParms orapar { CManODPNET::DbTypeInt32, &m_pub_xx };
				theManODPNET.EI("begin epstest.request_prod_info(:pub_xx); end;", orapar);
			} else
				return true;
		}
	}

	// szukaj BoundingBox w pliku
	if (powtorka > 0) pArg->pDlg->OglInfo(pArg->iChannelId, CString(_T("Pobieranie pliku z archiwum")));
	const CString& eps_name = EpsName(F_EPS, FALSE);
	pArg->pDlg->OglInfo(pArg->iChannelId, eps_name);
	if (eps_name.Mid(1, 1) != _T(":")) { 
		f5_errInfo += (eps_name + _T(", ")); 
		return false; 
	}

	CFile eps;
	CFileException e;
	if (!eps.Open(eps_name, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, &e)) {
		if (eps_name.Left(2) == _T("::"))
			f5_errInfo += eps_name.Mid(3) + (bReqProdInfo ? _T(" zamówiono dodatkowe sprawdzenie, ") : _T(" brak materia³u, "));
		else 
			f5_errInfo.AppendFormat(_T(" %li (b³¹d IO: %li), %s"), nreps, e.m_lOsError, m_pDocument->ErrMess(e.m_cause));

		return false;
	}
	if (!BBoxFromFile(pArg, &eps, bx1, by1, bx2, by2))
		f5_errInfo.AppendFormat(_T("%li z³e BoundingBox, "), nreps);
	if (cKolor == nullptr) {
		eps.Close();
		return true;
	}
	// szukaj Koloru w pliku
	eps.SeekToBegin();
	CManPDF mp(pArg);
	unsigned long DPCpos = mp.SearchPattern(eps, "DocumentProcessColors:");
	if (DPCpos == CManPDF::ulNotFound) 
		*cKolor = _T('-');
	else {
		auto buf = reinterpret_cast<char*>(pArg->cBigBuf);
		eps.Seek(DPCpos, CFile::begin);
		eps.Read(buf, 512);
		buf[512] = '\0';
		strtok(buf, CManPDF::sepline);
		BOOL jestBlack = FALSE;
		int tokcount = 0;
		char* p = strtok(buf, CManPDF::septok);
		while (p = strtok(nullptr, CManPDF::septok)) {
			if (strtok(p, "Black") == nullptr) jestBlack = TRUE;
			tokcount++;
		}
		*cKolor = (!jestBlack || tokcount>1) ? _T('F') : _T('B');
	}
	eps.Close();

	return true;
} // GetProdInfo

BOOL CDrawAdd::CheckSrcFile(PGENEPSARG pArg) {
	CString msg;
	f5_errInfo.Empty();
	
	int iSearchTypXX = typ_xx;
dajWymiarKraty:	
	// szukaj wymiarów w cachu
	auto pR = std::find_if(m_pDocument->m_Rozm.cbegin(), m_pDocument->m_Rozm.cend(), [=](const CRozm& r) {
		return (!iSearchTypXX && szpalt_x == r.szpalt_x && szpalt_y == r.szpalt_y) || (iSearchTypXX && iSearchTypXX == r.typ_xx);
	});

	if (pR == m_pDocument->m_Rozm.end()) { 
		if (iSearchTypXX > 0) {
			iSearchTypXX = 0;
			goto dajWymiarKraty;
		}
		f5_errInfo += _T("Brak rozmiaru w bazie "); return FALSE; 
	}
	
	// check scale
	float x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
	if (pArg->bIsPRN != F_PDF) {
		int ileMat;
		TCHAR cKolor[2];
		if (!GetProdInfo(pArg, cKolor, &x1, &y1, &x2, &y2, &ileMat))
			return FALSE;

		if (this->kolor == c_full && cKolor[0] == _T('-')) {
			msg.Format(_T("%li materia³ jest prawdopodobnie Black, "), nreps);
			f5_errInfo += msg;
		}
		if (this->kolor == c_brak && cKolor[0] == _T('F')) { // czy kolorowy material nie jest zadeklarowany jako Black
			msg.Format(_T("%li nie jest Black, "), nreps);
			f5_errInfo += msg;
		}
		if (this->flags.digital && ileMat < 2) { // czy dostarczono zarówno treœæ papierow¹ i cyfrow¹
			msg.Format(_T("%li nie dostarczono treœci cyfrowej lub papierowej, "), nreps);
		}
	} else { // PDF
		CManPDF pdf(pArg);
		CString eps_name = EpsName(pArg->bIsPRN, FALSE);
		if (!pdf.GetMediaBox(eps_name, &x1, &y1, &x2, &y2)) {
			msg.Format(_T("%li z³e MediaBox, "), nreps);
			f5_errInfo += msg;
		}
	}
	if (x1 == x2 || y1 == y2) {
		msg.Format(_T("%li zdegenerowany rozmiar, "), nreps);
		f5_errInfo += msg;
		return FALSE;
	}
	const float scalx = (float) (((sizex * (pR->w + pR->sw) - pR->sw) * mm2pkt) / (x2 - x1));
	const float scaly = (float) (((sizey * (pR->h + pR->sh) - pR->sh) * mm2pkt) / (y2 - y1));
	if (fabs(scalx - 1.0)>0.1 || fabs(scaly - 1.0)>0.1) {
		msg.Format(_T("%li inny rozmiar %.2fx%.2f, "), nreps, scalx, scaly);
		f5_errInfo += msg;
	} else if ((sizex == szpalt_x && scalx < 0.95) || (sizey == szpalt_y && scaly < 0.95)) {
		msg.Format(_T("%li ma pe³ne pole zadruku, "), nreps);
		f5_errInfo += msg;
	}

	if (powtorka != 0) { // skopiuj powtorke
		msg.Format(_T("ad=%li&dd=%s"), nreps, m_pDocument->dayws);
		auto pFile = theApp.OpenURL(1, msg);
		if (pFile)
			try {
				pFile->Read(pArg->cBigBuf, bigSize);
				if (*reinterpret_cast<short*>(pArg->cBigBuf) != 0x4b4f) { // "OK"
					msg.Format(_T("%li nie mo¿na skopiowac powtórki, "), nreps);
					f5_errInfo += msg;
				}
				pFile->Close();
			} catch (CInternetException* iex) {
				iex->GetErrorMessage(pArg->cBigBuf, DLGMSG_MAX_LEN);
				f5_errInfo += pArg->cBigBuf;
				iex->Delete();
			}
	}

	return TRUE;
} // CheckSrcFile

BOOL CDrawAdd::CopyNewFile(const CString& srcPath, const CString& dstPath)
{
	// Kopiuje plik <=> src jest nowszy od dst
	CFileStatus srcStat;
	CFileFind ff;
	CFile::GetStatus(srcPath, srcStat);
	if (ff.FindFile(dstPath)) {
		ff.FindNextFile();
		CTime dstTime;
		ff.GetLastWriteTime(dstTime);
		if (srcStat.m_mtime <= dstTime) {
			dstTime -= 2; // blad - dlaczego nowy plik czasem jest 2 sekundy pozniejszy
			if (srcStat.m_mtime < dstTime) {
				CString cs;
				cs.Format(_T("Nieudana próba kopiowania pliku\n\nz\t%s\ndo\t%s\n\nPlik Ÿród³owy jest starszy ni¿ plik docelowy.\nSprawdŸ czy poprawnie zaznaczono powórkê."), srcPath, dstPath);
				::MessageBox(NULL, cs, APP_NAME, MB_OK|MB_ICONERROR);
			}
			return FALSE;
		}
	}
	if (!::CopyFile(srcPath, dstPath, FALSE)) {
		CString cs;
		cs.Format(_T("B³¹d kopiowania (errno=%lu) pliku %s"), GetLastError(), srcPath);
		::MessageBox(NULL, cs, APP_NAME, MB_OK|MB_ICONERROR);  
		return FALSE;
	}
	return TRUE;
}

BOOL CDrawAdd::EpsFromATEX(const CString& num, const CString& dstPath)
{
	CString sAtexEPS = theApp.GetProfileString(_T("GenEPS"), _T("EpsATEX"), _T("T:\\ATEX\\EPS\\"));
	CFileFind ff;
	CString sAtexFile;
	sAtexFile.Format(_T("%s%i\\%s.eps"), sAtexEPS, _ttoi(num.Mid(num.GetLength() - 2)), num);
	return ff.FindFile(sAtexFile) && CopyFile(sAtexFile, dstPath + ".eps", FALSE);
}

CString CDrawAdd::FindZajawka(CString& root, const CString& ext) const {
	CFileFind ff;
	root += ((root.Right(1) == _T("\\")) ? _T("") : _T("\\"));
	CString s = root + nazwa + ext;
	if (ff.FindFile(s)) return s;
	if (root == _T("\\") || !ff.FindFile(root + _T("*.*"))) return _T("");
	while (ff.FindNextFile()) {
		if (ff.IsDots()) continue;
		if (ff.IsDirectory()) {
			s = FindZajawka(root + ff.GetFileName(), ext);
			if (!s.IsEmpty()) return s;
		}
	}
	return _T("");
}

BOOL CDrawAdd::LocatePreview(CFile& fEps, unsigned long *lOffset, unsigned long *lSize) {
	unsigned long header[3];
	fEps.SeekToBegin();
	if (fEps.Read(header, sizeof(header)) != sizeof(header))
		return FALSE;

	if (header[0] == 0xC6D3D0C5L) {
		*lOffset = header[1]; 
		*lSize = header[2];
	} else {
		*lOffset = 0; 
		*lSize = (unsigned long)fEps.GetLength();
	}

	return TRUE;
}

CString CDrawAdd::EpsName(int format, BOOL copyOldEPS, BOOL bModifTest) {
	static const TCHAR* aExt[] = { _T(".eps"), _T(".ps"), _T(".pdf") };
	bool czy_zajawka = wersja.Find(_T("z")) >= 0;
	if (nreps == -1 && !czy_zajawka) return CString(_T("brak nr atexa"));
	theApp.SetRegistryBase(_T("GenEPS"));

	CFileFind ff;
	CString s, num, oldnum, t2e(_T(" zamieñ .tif na ")), eps_path(theApp.GetString(_T("EpsSrc"), _T("")));
	t2e += aExt[format];
	num.Format(_T("%ld"), nreps);
	if (eps_path.IsEmpty() && theApp.isOpiMode && powtorka == 0 && !czy_zajawka)
		return _T("::\\") + num + aExt[format];
	eps_path += ((eps_path.Right(1) == _T("\\")) ? _T("") : _T("\\"));

	if (czy_zajawka) {
		int status;
		CManODPNETParms orapar {
			{ CManODPNET::DbTypeInt32, CManODPNET::ParameterOut, &status },
			{ CManODPNET::DbTypeInt32, &nreps }
		};
		orapar.outParamsCount = 1;
		theManODPNET.EI("begin :cc := check_emisja_zajawki(:adno); end;", orapar);

		if (status == 0) { // zajawka nie przesz³a przez interface Zajawki 
			const auto& zaj_path = FindZajawka(theApp.GetString(_T("EpsZajawki"), _T(".")), CString(aExt[format]));
			return zaj_path.IsEmpty() ? nazwa + CString(aExt[format]) + _T(" - brak zajawki") : zaj_path;
		} else if (powtorka == 0) { // adno_seed zajawki
			num.Format(_T("%i"), status);
			return (theApp.isOpiMode ? _T("::\\") : eps_path) + m_pDocument->daydir + num + aExt[format];
		}
	}

	if (theApp.GetInt(_T("DayDirs"), 0)) { // katalogi dniowe
		if (theApp.isOpiMode) {
			s = _T("::\\") + num;
			goto modifTest;
		} else if (powtorka == 0) { //nowy material
			s = eps_path + m_pDocument->daydir + num;
			if (ff.FindFile(s + aExt[format])) goto modifTest;
			ff.Close();
			if (ff.FindFile(s + _T(".tif"))) return num + t2e;
			if (format != F_PDF && EpsFromATEX(num, s))
				goto modifTest;
		} else { // powtorka
			if (oldAdno > 0) // z numeru
				oldnum.Format(_T("%ld"), oldAdno);
			CString sFileName(oldnum.IsEmpty() ? num : oldnum);
			CString pathtail = powtorka.Format(c_ctimeDataWs) + _T("\\") + sFileName;
			s = eps_path + pathtail;
						
			if (ff.FindFile(s + aExt[format])) { // znaleziono plik z powtórk¹

				WIN32_FILE_ATTRIBUTE_DATA actualAttr;
				if (copyOldEPS && ::GetFileAttributesEx(eps_path + m_pDocument->daydir + num + _T(".eps"), GetFileExInfoStandard, &actualAttr)) { // mo¿e wgrano nowy material i nie zdjeto flagi powtorki
					WIN32_FILE_ATTRIBUTE_DATA powtAttr;
					if (::GetFileAttributesEx(s + aExt[format], GetFileExInfoStandard, &powtAttr))
						if (actualAttr.ftLastWriteTime.dwHighDateTime > powtAttr.ftLastWriteTime.dwHighDateTime ||
							(actualAttr.ftLastWriteTime.dwHighDateTime == powtAttr.ftLastWriteTime.dwHighDateTime && actualAttr.ftLastWriteTime.dwLowDateTime > powtAttr.ftLastWriteTime.dwLowDateTime)) {
							CString msg;
							msg.Format(_T("SprawdŸ czy poprawnie zaznaczono powórkê:\n\n%s\n\nPlik docelowy jest nowszy ni¿ archiwum."), num);
							::MessageBox(NULL, msg, APP_NAME, MB_OK|MB_ICONERROR);
							copyOldEPS = FALSE;
						}
				}

				if (copyOldEPS /*&& !inArch*/) { // skopiuj powtórkê
					CopyNewFile(s + aExt[format], eps_path + m_pDocument->daydir + num + _T(".eps"));
					s = eps_path + m_pDocument->daydir + num;
				}
				goto modifTest;
			}
			ff.Close();
			if (ff.FindFile(s + _T(".tif"))) return num + t2e;
		}
	} else {
		CString zera;
		int poIle = theApp.GetInt(_T("Hashing"), 2);
		int numlen = num.GetLength();
		if (numlen == 7) poIle--;
		zera = CString('0', max(0, numlen - poIle));
		
		if (theApp.GetInt(_T("SubDirSearch"), 1)) {
			s = eps_path + num.Left(poIle) + zera + _T("\\") + num;
			if (ff.FindFile(s + aExt[format])) goto modifTest;
			ff.Close();
			if (ff.FindFile(s + ".tif")) return num + t2e;
			ff.Close();
			zera = theApp.GetString(_T("EpsOld"), _T("!!stare"));
			zera += ((zera.Right(1) == _T("\\")) ? _T("") : _T("\\"));
			zera = eps_path + zera + num;
			if (ff.FindFile(zera + aExt[format])) { 
				s = zera; 
				goto modifTest;
			}
			ff.Close();
			if (ff.FindFile(zera + _T(".tif"))) return num + t2e;
		} else {
			s = eps_path + num;
			if (ff.FindFile(s + aExt[format])) goto modifTest;
			ff.Close();
			if (ff.FindFile(s + _T(".tif"))) return num + t2e;
		}
		if (format != F_PDF && EpsFromATEX(num,s)) 
			goto modifTest;
	}
	return (num + _T(" brak ") + aExt[format]);
modifTest:
	if (bModifTest) {
		ff.FindNextFile();
		ff.GetLastWriteTime(epsDate);
	}
	return s + aExt[format];
} // EpsName

BOOL CDrawAdd::RewriteEps(PGENEPSARG pArg, CFile *dest) {
	CString eps_name;
	char* s = reinterpret_cast<char*>(pArg->cBigBuf);
	CDrawAdd *pLeftAdd = this;
	unsigned long lOffset = 0, lSize = 0;
	auto pPage = m_pDocument->GetPage(fizpage);
	bool bLewaStrona = (m_pDocument->GetIPage(pPage) & 1) == 0;
	auto pRozAdd = m_pDocument->GetCRozm(pArg, szpalt_x, szpalt_y, spad_flag ? 0 : typ_xx);	// gdy zaznaczono spad, to montuj do kraty
	auto pRozKraty = typ_xx ? m_pDocument->GetCRozm(pArg, szpalt_x, szpalt_y, 0) : pRozAdd;	// pobierz rozmiar kraty, jeœli dotychczas nie jest znany

	theApp.SetRegistryBase(_T("GenEPS"));
	BOOL copyEps = theApp.GetInt(_T("CopyOldEPS"), 0);
	pArg->pDlg->OglInfo(pArg->iChannelId, CString(_T("kopiowanie powtorki")));
	if (pPage->m_typ_pary == 2 && !bLewaStrona) { // prawa strona rozkladowki
		CDrawPage *pPrevPage = fizpage>15 ? m_pDocument->GetPage(fizpage-8) : m_pDocument->m_pages[0];
		if (pPrevPage->m_adds.empty()) {
			::MessageBox(pArg->pDlg->m_hWnd, _T("Na lewej stronie rozkladówki nie ma og³oszenia"), _T("Brak og³oszenia"), MB_ICONSTOP);
			return FALSE;
		}
		pLeftAdd = pPrevPage->m_adds.front();
		if (pLeftAdd) eps_name = pLeftAdd->EpsName(F_EPS, copyEps);
	} else
		eps_name = wersja==DERV_TMPL_WER ? nazwa : EpsName(F_EPS, copyEps); // albo nazwa pliku ze sciezka, albo info o bledzie
	pArg->pDlg->OglInfo(pArg->iChannelId, eps_name);

	float x = (float)((posx-1)*(pRozKraty->w+pRozKraty->sw)*mm2pkt);
	float y = (float)((szpalt_y+1- posy-sizey)*(pRozKraty->h+pRozKraty->sh)*mm2pkt+(pArg->bSignAll ? podpisH : podpisH/2));   // miejsce gdzie sie zaczyna eps bez podpisu
	const float gpx = (float)((sizex*(pRozAdd->w+pRozAdd->sw) - pRozAdd->sw)*mm2pkt); 
	const float gpy = (float)((sizey*(pRozAdd->h+pRozAdd->sh) - pRozAdd->sh)*mm2pkt);
	
	if (eps_name.Mid(1,1) != _T(":")) {
		if (pArg->bIsPRN == 1 && pArg->bDoKorekty == 0)
			::MessageBox(pArg->pDlg->m_hWnd, _T("Na produkcyjnej wersji kolumny brakuje materia³u: ") + eps_name, _T("Brak og³oszenia"), MB_ICONWARNING);

		::StringCchPrintfA(s, n_size, ".25 .1 .9 %.2f %.2f %.2f %.2f R\r\n", gpx, gpy, x, y); dest->Write(s, (UINT)strlen(s));
		::StringCchPrintfA(s, n_size, "(%s) %.2f %.2f %d /Switzerland Po\r\n", CStringA(eps_name), (x + gpx), (y - podpisH), podpisH);  dest->Write(s, (UINT)strlen(s));
		return FALSE;
	}

	TRY {
		// kopiuj podwaly
		CString podwal(theApp.GetString(_T("EpsPodwaly"), _T(""))); 
		if (copyEps && podwal.Find(_T(":")) >= 0 && wersja != DERV_TMPL_WER && pPage->name.Find(_T("RED")) >= 0) {
			podwal += ((podwal.Right(1) == _T("\\")) ? _T("") : _T("\\"));
			CString fname = eps_name.Mid(eps_name.ReverseFind(_T('\\')) + 1);
			if (theApp.GetInt(_T("PodwalySubDir"), 0) == 0)
				podwal += fname;
			else {
				CString sPodwalDir(podwal + theApp.activeDoc->data.Mid(3, 2) + theApp.activeDoc->data.Mid(0, 2) + _T("\\"));
				if (::CreateDirectory(sPodwalDir, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
					sPodwalDir +=  theApp.activeDoc->gazeta.Mid(0, 3) + theApp.activeDoc->gazeta.Mid(4, 2) + _T("\\");
					if (::CreateDirectory(sPodwalDir, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) 
						podwal = sPodwalDir + fname;
				} else
					::MessageBox(pArg->pDlg->m_hWnd, CString(_T("Nie mo¿na utworzyæ katalogu: ")) + sPodwalDir, _T("B³¹d"), MB_ICONERROR);
			}
			pArg->pDlg->OglInfo(pArg->iChannelId, _T("Podwa³: ") + podwal);
			CopyNewFile(eps_name, podwal);
			if (!theApp.GetInt(_T("Podwaly"), 0))
				return TRUE;
		}

		if (spad_flag == 0 && pPage->m_typ_pary < 2) { // podpisujemy tylko pasowane
			CStringA dopodpisu;
			if (theApp.activeDoc->gazeta.Left(3) == _T("TBP")) {
				CString uPodpis(_T(' '), 64);
				CManODPNETParms orapar {
					{ CManODPNET::DbTypeVarchar2, CManODPNET::ParameterOut, &uPodpis },
					{ CManODPNET::DbTypeInt32, &this->m_pub_xx }
				};
				orapar.outParamsCount = 1;
				theManODPNET.EI("begin :pod := pagina.podpis(:pub_xx); end;", orapar);
				dopodpisu = uPodpis;
			} else
				dopodpisu.Format("%li%s", nreps, CStringA(wersja));

			::StringCchPrintfA(s, n_size, "(%s) %.2f %.2f %d /Switzerland Po\r\n", dopodpisu, (x+gpx), (y-podpisH), podpisH); dest->Write(s, (UINT)strlen(s));
			if (this->flags.reksbtl) {
				::StringCchPrintfA(s, n_size, "(REKLAMA) %.2f %.2f 1 %d /Switzerland textLR\r\n", x, y-podpisH, podpisH);
				dest->Write(s, (UINT)strlen(s));
			}
		} 

		float x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
		if (!pLeftAdd->GetProdInfo(pArg, nullptr, &x1, &y1, &x2, &y2, nullptr)) {
			::MessageBox(pArg->pDlg->m_hWnd, f5_errInfo, _T("B³¹d"), MB_ICONERROR);
			f5_errInfo.Empty();
			return FALSE;
		}
		
		::StringCchPrintfA(s, n_size, "beginManamEPS \r\n"); dest->Write(s, (UINT)strlen(s));
		if (pPage->m_typ_pary < 2) {
			// spad zdefiniowany dla krawedzi
			x += aSpadInfo[spad_flag].fOffX*(gpx - (x2 - x1));
			y += aSpadInfo[spad_flag].fOffY*(gpy - (y2 - y1));
			::StringCchPrintfA(s, n_size, "%.2f %.2f translate\r\n", x, y);  dest->Write(s, (UINT)strlen(s));
			// skalowanie
			const float scalx = (x2 == x1 || !pRozAdd->scale_it || !aSpadInfo[spad_flag].bScaX) ? 1 : gpx / (x2 - x1);
			const float scaly = (y2 == y1 || !pRozAdd->scale_it || !aSpadInfo[spad_flag].bScaY) ? 1 : gpy / (y2 - y1);
			if (scalx != 1 || scaly != 1) {
				::StringCchPrintfA(s, n_size, "%.4f %.4f scale\r\n", scalx, scaly);  dest->Write(s, (UINT)strlen(s));
			}
		}

		if (pPage->m_typ_pary == 2 || spad_flag == 15 || // produkcja pary z jednego materia³u || przy pe³nym spadzie
			(bLewaStrona && spad_flag == 11 && sizex == szpalt_x && sizey == szpalt_y)) { // na lewej rozk³adówce dodatkowy napis REKLAMA
			double dMargDoGrb;
			CString uTrans(_T(' '), 8);
			CString uReklama(_T(' '), 32);
			CManODPNETParms orapar {
				{ CManODPNET::DbTypeInt32, &m_pDocument->m_mak_xx },
				{ CManODPNET::DbTypeInt32, &pPage->id_str },
				{ CManODPNET::DbTypeVarchar2, CManODPNET::ParameterOut, &uTrans },
				{ CManODPNET::DbTypeVarchar2, CManODPNET::ParameterOut, &uReklama },
				{ CManODPNET::DbTypeDouble, CManODPNET::ParameterOut, &dMargDoGrb }
			};
			orapar.outParamsCount = 3;
			theManODPNET.EI("begin pagina.get_ppole_trans(:mak_xx,:str_xx,:trans,:rekl,:mdg); end;", orapar);

			CStringA sTrans(uTrans);
			CStringA sReklama(uReklama);
			if (!sTrans.IsEmpty()) {
				if (pPage->m_typ_pary == 2) {
					float fMargDoGrb = (float)dMargDoGrb;
					if (bLewaStrona)
						x += fMargDoGrb - 0.5f*(x2 - x1) + gpx;
					else
						x -= fMargDoGrb + 0.5f*(x2 - x1);
					y += 0.5f * (gpy+y1-y2);
					::StringCchPrintfA(s, n_size, "%.2f %.2f translate %s translate\r\n", x, y, sTrans);  dest->Write(s, (UINT)strlen(s));
					if (bLewaStrona) {
						::StringCchPrintfA(s, n_size, ".5 (%s) 0 -5.5 1 7 /GWFranklinBold textKLR\r\n", sReklama); dest->Write(s, (UINT)strlen(s));
					}
				} else {
					const char* f = (strncmp("0 26", sTrans, 4) == 0) ? "/GWTimesCondensed" : "/GWFranklinBold";
					if (!bLewaStrona || spad_flag == 11)
						::StringCchPrintfA(s, n_size, "%s translate 90 rotate 2 (%s) 0 1.5 1 7 %s textKLR -90 rotate\r\n", sTrans, sReklama, f);
					else
						::StringCchPrintfA(s, n_size, "%s translate 90 rotate 2 (%s) 0 %.2f 1 7 %s textKLR -90 rotate\r\n", sTrans, sReklama, x1-x2-7, f);
					dest->Write(s, (UINT)strlen(s));
				}
			} else {
				::MessageBox(pArg->pDlg->m_hWnd, _T("Aby uzyskaæ parê kolumn z jednego materia³u wygaœ wszystkie elementy paginy"), APP_NAME, MB_ICONINFORMATION);
				return FALSE;
			}
		}
		 
		::StringCchPrintfA(s, n_size, "%.2f %.2f translate\r\n", -x1, -y1); dest->Write(s, (UINT)strlen(s));
		::StringCchPrintfA(s, n_size, "%.2f %.2f %.2f %.2f clipEPS \r\nclip newpath\r\n", x1, y1, x2 - x1, y2 - y1); dest->Write(s, (UINT)strlen(s));
		::StringCchPrintfA(s, n_size, "%%%%BeginDocument: %s\r\n", CStringA(eps_name)); dest->Write(s, (UINT)strlen(s));
		dest->Flush();

		if (theApp.isOpiMode) {
			::StringCchPrintfA(s, n_size, "%s%s %s\r\n", OPI_TAG, "OG", CStringA(eps_name));
			dest->Write(s, (UINT)strlen(s));
		} else {
			CFile eps;
			CFileException e;
			if (!eps.Open(eps_name, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, &e ) ) {
				::MessageBox(pArg->pDlg->m_hWnd, _T("Nie mo¿na uzyskaæ dostêpu do pliku: ") + eps_name + _T("\n ") + m_pDocument->ErrMess(e.m_cause), _T("B³¹d"), MB_OK);
				return FALSE ;
			} 
			CDrawAdd::LocatePreview(eps, &lOffset, &lSize);
			eps.Seek(lOffset, CFile::begin);
			unsigned long res;
			do {
				res = eps.Read(s, n_size);
				if (res > 0 && res != unsigned(-1)) {
					if (lSize < (unsigned long)res) res = int(lSize);
					dest->Write(s, (UINT)res);
					lSize -= res;
				} else break;
			} while (res > 0 && lSize > 0);
			eps.Close();
		}

		::StringCchPrintfA(s, n_size, "%%%%EndDocument\r\nendManamEPS\r\n");
		dest->Write(s, (UINT)strlen(s));
	} CATCH( CFileException, e ) {
		::MessageBox(pArg->pDlg->m_hWnd, _T("B³¹d przy przepisywaniu eps'a ") + eps_name + _T("\n ") + m_pDocument->ErrMess(e->m_cause), _T("B³¹d"), MB_OK|MB_ICONERROR); 
		return FALSE;
	} END_CATCH

		return TRUE;
} // RewriteEps

BOOL CDrawAdd::RewriteDrob(PGENEPSARG pArg, CFile *dest) {
	auto s = reinterpret_cast<char*>(pArg->cBigBuf);
	unsigned long lOffset = 0, lSize = 0;
	auto pRoz = m_pDocument->GetCRozm(pArg, szpalt_x, szpalt_y, typ_xx);

	float scaly, x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
	float x = (float)((posx - 1)*(pRoz->w + pRoz->sw)*mm2pkt);
	float y = (float)((szpalt_y + 1 - posy - sizey)*(pRoz->h + pRoz->sh)*mm2pkt);

	if (theApp.isOpiMode)
		nazwa.Format(_T("%s%s%03i.eps"), m_pDocument->gazeta.Left(3), m_pDocument->gazeta.Mid(4,2), txtposx);

	pArg->pDlg->OglInfo(pArg->iChannelId, "Drobne: " + nazwa);

	BOOL bReadFromATEX = FALSE;
	CString sURL;
	sURL.Format(_T("drw_xx=%i&kiedy=%s&nr_porz=%i"), m_pDocument->id_drw, m_pDocument->dayws, txtposx);
	auto pFile = theApp.OpenURL(2, sURL);
	if (pFile) {
		try {
			bReadFromATEX = pFile->Read(s, 50) > 10 && sscanf_s(s, "%f %f %f %f", &x1, &y1, &x2, &y2) == 4;
		} catch (CInternetException* iex) {
			if (theApp.isOpiMode) {
				iex->GetErrorMessage(pArg->cBigBuf, DLGMSG_MAX_LEN);
				::MessageBox(pArg->pDlg->m_hWnd, CString(_T("Nie uda³o siê pobraæ rozmiaru drobnych: ")) + pArg->cBigBuf, APP_NAME, MB_OK|MB_ICONERROR);
			}
			iex->Delete();
		}

		pFile->Close();
	}

	if (!bReadFromATEX) {
		CFile eps;
		CFileException e;
		if (!eps.Open(nazwa, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, &e ) ) {
			::MessageBox(pArg->pDlg->m_hWnd, _T("Nie wyeksportowano strony z Drobnixa lub nie mo¿na uzyskaæ dostêpu do pliku: ") + nazwa + _T("\n ") + m_pDocument->ErrMess(e.m_cause), APP_NAME, MB_OK|MB_ICONERROR);
			return FALSE ;
		} 
		eps.SeekToBegin();
		BBoxFromFile(pArg, &eps, &x1, &y1, &x2, &y2);
		eps.Close();
	} 

	::StringCchPrintfA(s, n_size, "beginManamEPS \r\n");  dest->Write(s, (UINT)strlen(s));
	::StringCchPrintfA(s, n_size, "%.2f %.2f translate\r\n", x, y);  dest->Write(s, (UINT)strlen(s));
	scaly = (y2 == y1) ? 1 : m_pDocument->GetDrobneH()/(y2 - y1);
	::StringCchPrintfA(s, n_size, "1 %.4f scale\r\n", scaly);  dest->Write(s, (UINT)strlen(s));
	  
	::StringCchPrintfA(s, n_size, "%.2f %.2f translate\r\n",-x1,-y1);  dest->Write(s, (UINT)strlen(s)); 
	::StringCchPrintfA(s, n_size, "%.2f %.2f %.2f %.2f clipEPS \r\nclip newpath\r\n", x1, y1, x2-x1, y2-y1);  dest->Write(s, (UINT)strlen(s));
	::StringCchPrintfA(s, n_size, "%%%%BeginDocument: %s\r\n", CStringA(nazwa)); dest->Write(s, (UINT)strlen(s)); 
	dest->Flush();

	if (theApp.isOpiMode) {
		::StringCchPrintfA(s, n_size, "%sDR %s\r\n", OPI_TAG, CStringA(nazwa));
		dest->Write(s, (UINT)strlen(s));
	} else {
		CFile eps;
		CFileException e;
		if (!eps.Open(nazwa, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, &e ) ) {
			::MessageBox(pArg->pDlg->m_hWnd, _T("Nie mo¿na uzyskaæ dostêpu do pliku: ") + nazwa + "\n " + m_pDocument->ErrMess(e.m_cause), _T("B³¹d"), MB_OK);
			return FALSE ;
		} 
		CDrawAdd::LocatePreview(eps, &lOffset, &lSize);
		eps.Seek(lOffset, CFile::begin);
		unsigned long res;
		do {
			res = eps.Read(s, n_size);
			if (res > 0 && res != unsigned(-1)) {
				if (lSize < res) res = int(lSize);
				dest->Write(s, res);
				lSize -= res;
			} else break;
		} while (res > 0 && lSize > 0);
		eps.Close();
	}

	::StringCchPrintfA(s, n_size, "%%%%EndDocument\r\nendManamEPS\r\n"); dest->Write(s, (UINT)strlen(s));

	return TRUE;
} // RewriteDrob

void CDrawAdd::Preview(PGENEPSARG pArg,int x,int y,int dy,int szer) const
{
	BYTE *p = (BYTE*)pArg->cBigBuf;
	auto pRoz = m_pDocument->GetCRozm(pArg, szpalt_x, szpalt_y, typ_xx);

	div_t x1 = div((int) ((posx - 1)*(pRoz->w + pRoz->sw)) / 10 - x, 8);
	div_t x2 = div((int) (((posx - 1 + sizex)*(pRoz->w + pRoz->sw) - pRoz->sw) / 10) - x, 8);
	int i, y1 = (int) (((szpalt_y + 1 - posy - sizey)*(pRoz->h + pRoz->sh)) / 10) - y + 2;
	int j, y2 = (int) (((szpalt_y + 1 - posy)*(pRoz->h + pRoz->sh) - pRoz->sh) / 10) - y + 2;
	y1 = min(dy - y1, dy - 2);   // koniec
	y2 = max(dy - y2, 0);   //poczatek

	BYTE p1 = (BYTE) ((1 << (8 - x1.rem)) - 1);
	BYTE p2 = (BYTE)(255 ^ (BYTE)((1 << (8 - x2.rem)) -1));
	BYTE m1 = (BYTE)(1 << (7 - x1.rem));
	BYTE m2 = (BYTE)(1 << (7 - x2.rem));
	BYTE c1=(BYTE)170; //136
	BYTE c2=(BYTE)85;   //34
	BYTE ci=c1;
   
	for (i=1;  i < 3; i++)
       *(p+(y1+1)*szer+x2.quot-i) = (BYTE)255;   //podpis  
   
	for (i=y2;i<=y1;i++)  {
		*(p+ i*szer+x1.quot) |=  (m1 | p1) & ci;       //0000100000 
		*(p+ i*szer+x2.quot) |=  (m2 | p2) & ci;
		for (j=x1.quot+1;j<x2.quot;j++)
			*(p+ i*szer+j) = ci;      //11111111 
		ci=(ci==c1) ? c2 : c1;
	}
} //Preview
