
#include "pch.h"
#include "DrawAdd.h"
#include "DrawDoc.h"
#include "DrawPage.h"
#include "DrawTool.h"
#include "DrawView.h"
#include "GenEpsInfoDlg.h"
#include "GridFrm.h"
#include "KolDlg.h"
#include "MainFrm.h"
#include "Manam.h"
#include "ManPDF.h"
#include "Spacer.h"

extern BOOL drawErrorBoxes;
extern SelectMode selectMode;

constinit const SpadInfo aSpadInfo[] = {
    { SpadInfo::bleed_right, SpadInfo::bleed_right, true,  true  }, // 0
    { SpadInfo::bleed_right, SpadInfo::bleed_right, false, true  },
    { SpadInfo::bleed_right, SpadInfo::bleed_right, true,  false },
    { SpadInfo::bleed_right, SpadInfo::bleed_right, false, false },
    { SpadInfo::bleed_left,  SpadInfo::bleed_right, false, true  },
    { SpadInfo::just_center, SpadInfo::bleed_right, false, true  }, // 5
    { SpadInfo::bleed_left,  SpadInfo::bleed_right, false, false },
    { SpadInfo::just_center, SpadInfo::bleed_right, false, false },
    { SpadInfo::bleed_right, SpadInfo::bleed_left,  true,  false },
    { SpadInfo::bleed_right, SpadInfo::bleed_left,  false, false },
    { SpadInfo::bleed_right, SpadInfo::just_center, true,  false }, // 10
    { SpadInfo::bleed_right, SpadInfo::just_center, false, false },
    { SpadInfo::bleed_left,  SpadInfo::bleed_left,  false, false },
    { SpadInfo::just_center, SpadInfo::bleed_left,  false, false },
    { SpadInfo::bleed_left,  SpadInfo::just_center, false, false },
    { SpadInfo::just_center, SpadInfo::just_center, false, false }, // 15
};

IMPLEMENT_SERIAL(CDrawAdd, CDrawObj, 0)

CDrawAdd::CDrawAdd() noexcept : bank{0}
{
}

CDrawAdd::CDrawAdd(const CRect& position) noexcept : CDrawAdd::CDrawAdd()
{
    ASSERT_VALID(this);
    m_position = position;
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

CDrawAdd::~CDrawAdd()
{
    if (fizpage != 0)
        m_pDocument->GetPage(fizpage)->RemoveAdd(this);
}

void CDrawAdd::Serialize(CArchive& ar)
{
    ASSERT_VALID(this);

    CDrawObj::Serialize(ar);
    if (ar.IsStoring()) {
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
        ar << (DWORD)fizpage;
        ar << (WORD)posx;
        ar << (WORD)posy;
        ar << (WORD)txtposx;
        ar << (WORD)txtposy;
        ar << *(WORD*)&flags;
        ar << (LONG)nreps;
        ar << (LONG)oldAdno;
        ar << powtorka;
        ar << (WORD)spad_flag;
        ar << (WORD)precelWertexCnt;
        if (precelWertexCnt) {
            ar << (WORD)precelRingCnt;
            ar.Write(aPrecelWertex.get(), static_cast<UINT>(precelWertexCnt * sizeof(CPoint)));
            ar.Write(aRingWertexCnt, static_cast<UINT>(precelRingCnt * sizeof(int)));
        }
        space.Serialize(ar);
    } else {
        ar >> m_add_xx;
        DWORD wTemp0;
        WORD wTemp, wTemp1, wTemp2;
        ar >> wTemp; typ_xx = wTemp;
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
        ar >> wTemp; *((WORD*)&flags) = wTemp;
        ar >> nreps;
        ar >> oldAdno;
        ar >> powtorka;
        ar >> wTemp; spad_flag = (BYTE)wTemp;
        ar >> wTemp; precelWertexCnt = wTemp;
        if (precelWertexCnt) {
            ar >> wTemp; precelRingCnt = wTemp;
            ASSERT(precelRingCnt <= ciMaxRings);
            aPrecelWertex = std::make_unique<CPoint[]>(precelWertexCnt + 2); // na ko�cu int[ciMaxRings]
            aRingWertexCnt = reinterpret_cast<int*>(&aPrecelWertex[precelWertexCnt]);
            ar.Read(aPrecelWertex.get(), precelWertexCnt * sizeof(CPoint) + precelRingCnt * sizeof(int));
        }
        space = CFlag(0, 0, szpalt_x, szpalt_y);
        space.Serialize(ar);
        SetSpaceAndPosition(wTemp0, wTemp1, wTemp2);
    }
}

////////////////////////// DRAW
void CDrawAdd::Draw(CDC* pDC)
{
    ASSERT_VALID(this);

    auto mainWnd = static_cast<CMainFrame*>(AfxGetMainWnd());
    CBrush* pOldBrush = pDC->SelectObject(CDrawDoc::GetSpotBrush(kolor >> 3));
    CPen* pOldPen = pDC->SelectObject(&(mainWnd->pen));

    int iClipRegions = 1;
    CRect rect{m_position}, *aRectClip{nullptr};
    // clipping dla precli
    if (precelWertexCnt > 0) {
        CRgn rgn;
        if (precelRingCnt == 1)     // clipping dla precli
            rgn.CreatePolygonRgn(aPrecelWertex.get(), precelWertexCnt, ALTERNATE);
        else if (precelRingCnt > 1) //clipping dla precli z wieloma obwodniacami
            rgn.CreatePolyPolygonRgn(aPrecelWertex.get(), aRingWertexCnt, precelRingCnt, ALTERNATE);
        rgn.OffsetRgn(m_position.left, m_position.top);
        rgn.GetRegionData((RGNDATA*)theApp.bigBuf, bigSize);
        aRectClip = (CRect*)((RGNDATA*)theApp.bigBuf)->Buffer;
        iClipRegions = ((RGNDATA*)theApp.bigBuf)->rdh.nCount;
        rgn.DeleteObject();
    }
    // wype�nianie kolorem
    for (int i = 0; i < iClipRegions; ++i) {
        if (aRectClip)
            pDC->IntersectClipRect(aRectClip[i]);
        switch (m_pDocument->swCZV) {
            case ToolbarMode::normal:
                DrawKolor(pDC, &rect);
                break;
            case ToolbarMode::czas_obow:
                switch (flags.isok) {
                    case 1: // 'OK.'
                        pDC->FillRect(rect, &mainWnd->yellow);
                        break;
                    case 2: // 'ATX'
                        pDC->FillRect(rect, &mainWnd->magenta);
                        break;
                    default: // 'WER'
                        pDC->FillRect(rect, CDrawDoc::GetSpotBrush(0));
                }
                break;
            case ToolbarMode::tryb_studia:
                int kolInd = 0;
                if (auto pos = _tcschr(wersja, _T('.')))
                    kolInd = (1 + mainWnd->GetKolorInd(pos + 1)) % CDrawDoc::brushe.size();
                pDC->FillRect(rect, CDrawDoc::GetSpotBrush(kolInd));
        }
        if (aRectClip)
            pDC->SelectClipRgn(nullptr);
    }

    pDC->SelectStockObject(NULL_BRUSH);
    if (precelRingCnt == 0)
        pDC->Rectangle(rect);
    else {
        pDC->OffsetWindowOrg(-m_position.left, -m_position.top);
        if (precelRingCnt == 1)
            pDC->Polygon(aPrecelWertex.get(), precelWertexCnt);
        else
            pDC->PolyPolygon(aPrecelWertex.get(), aRingWertexCnt, precelRingCnt);
        pDC->OffsetWindowOrg(m_position.left, m_position.top);
    }

    if (flags.derived) {
        CRect r{rect};
        r.InflateRect(-2 * CLIENT_SCALE, 2 * CLIENT_SCALE);
        pDC->Rectangle(r);
    }

    if (selectMode != SelectMode::move) {
        DrawDesc(pDC, &m_position);
        if ((m_pDocument->swCZV != ToolbarMode::czas_obow && flags.locked) || (m_pDocument->swCZV == ToolbarMode::czas_obow && flags.reserv))
            DrawPadlock(pDC, &m_position);
        int retCkPageLocation = 0;
        if (m_pDocument->swCZV == ToolbarMode::normal && drawErrorBoxes)
            retCkPageLocation = CkPageLocation(fizpage);
        const bool simpleCase = (m_pDocument->swCZV == ToolbarMode::czas_obow && flags.zagroz) || (m_pDocument->swCZV == ToolbarMode::tryb_studia && flags.showeps);
        if (simpleCase || retCkPageLocation > 0) {
            pDC->MoveTo(rect.left, rect.top);
            pDC->LineTo(rect.right, rect.bottom);
            if (simpleCase || retCkPageLocation > 1) {
                pDC->MoveTo(rect.right, rect.top);
                pDC->LineTo(rect.left, rect.bottom);
            }
        }
    }

    pDC->SelectObject(pOldBrush);
    pDC->SelectObject(pOldPen);
}

void CDrawAdd::DrawDesc(CDC* pDC, const CRect& rect) const
{
    /* vu : Rysuje opis tekstowy og�oszenia na podanym urz�dzeniu	end vu */

    CString buf;
    switch (theApp.m_view_top) { // vu : opis gorny
        case TEXT_NAZWA:
            DrawTxt(pDC, rect, nazwa, true);
            break;
        case TEXT_PLIK:
            DrawTxt(pDC, rect, kodModulu, true);
            break;
        case TEXT_WARLOG:
            DrawTxt(pDC, rect, logpage, true);
            break;
        case TEXT_CZASKTO:
            DrawTxt(pDC, rect, czaskto, true);
            break;
        case TEXT_UWAGI:
            DrawTxt(pDC, rect, remarks, true);
            break;
        case TEXT_STUDIO:
            DrawTxt(pDC, rect, CGridFrm::studioStats[(uint8_t)this->flags.studio], true);
            break;
        case TEXT_EPS:
            if (nreps > -1L) {
                buf.Format(_T("%li"), nreps);
                DrawTxt(pDC, rect, buf, true);
            }
    }

    switch (theApp.m_view_bottom) { // vu : opis dolny
        case TEXT_NAZWA:
            DrawTxt(pDC, rect, nazwa, false);
            break;
        case TEXT_PLIK:
            DrawTxt(pDC, rect, kodModulu, false);
            break;
        case TEXT_WARLOG:
            DrawTxt(pDC, rect, logpage, false);
            break;
        case TEXT_CZASKTO:
            DrawTxt(pDC, rect, czaskto, false);
            break;
        case TEXT_UWAGI:
            DrawTxt(pDC, rect, remarks, false);
            break;
        case TEXT_STUDIO:
            DrawTxt(pDC, rect, CGridFrm::studioStats[(uint8_t)this->flags.studio], false);
            break;
        case TEXT_EPS:
            if (nreps > -1L) {
                buf.Format(_T("%li"), nreps);
                DrawTxt(pDC, rect, buf, false);
            }
    }
}

void CDrawAdd::DrawTxt(CDC* pDC, const CRect& rect, LPCTSTR tx, const bool top) const
{
    CRect r{rect};
    r += CPoint(vscale*(txtposx - TXTSHIFT), -vscale*(txtposy - TXTSHIFT));
    CFont* oldFont = pDC->SelectObject(&(m_pDocument->m_addfont));
    pDC->SetBkColor(ManColor::White);
    if (top)
        DrawNapis(pDC, tx, (int)_tcslen(tx), r, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOCLIP, OPAQUE);
    else {
        LOGFONT lf;
        m_pDocument->m_addfont.GetObject(sizeof(LOGFONT), &lf);
        r += CPoint(0, -vscale * std::abs(lf.lfHeight));
        DrawNapis(pDC, tx, (int)_tcslen(tx), r, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOCLIP, OPAQUE);
    }
    pDC->SelectObject(oldFont);
}

void CDrawAdd::DrawPadlock(CDC* pDC, const CRect& rect) const
{
    auto pOldBrush = (CBrush*)pDC->SelectStockObject(BLACK_BRUSH);
    auto pOldPen = (CPen*)pDC->SelectStockObject(BLACK_PEN);

    if (m_pDocument->swCZV != ToolbarMode::czas_obow) { // k��dka
        CRect r{rect.right - 4 * vscale, rect.bottom + 4 * vscale, rect.right, rect.bottom};
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

void CDrawAdd::Print(CDC* pDC)
{
    ASSERT_VALID(this);

    auto pOldBrush = pDC->SelectObject(CDrawDoc::GetSpotBrush(kolor >> 3));
    auto pOldPen = (CPen*)pDC->SelectStockObject(BLACK_PEN);

    CRgn rgn;
    int iClipRegions;
    CRect *aRectClip, rect = GetPrintRect();

    switch (precelRingCnt) {
        case 0:
            pDC->Rectangle(rect);
            if (kolor == ColorId::full)
                DrawKolor(pDC, &rect);
            break;
        case 1:
            for (int i = 0; i < precelWertexCnt; ++i) {
                aPrecelWertex[i].x /= CLIENT_SCALE;
                aPrecelWertex[i].y /= CLIENT_SCALE;
            }
            rgn.CreatePolygonRgn(aPrecelWertex.get(), precelWertexCnt, ALTERNATE);
            rgn.OffsetRgn(rect.left, rect.top);
            rgn.GetRegionData((RGNDATA*)theApp.bigBuf, bigSize);
            aRectClip = (CRect*)((RGNDATA*)theApp.bigBuf)->Buffer;
            iClipRegions = ((RGNDATA*)theApp.bigBuf)->rdh.nCount;
            rgn.DeleteObject();
            for (int i = 0; i < iClipRegions; ++i) {
                pDC->IntersectClipRect(aRectClip[i]);
                DrawKolor(pDC, &rect);
                pDC->SelectClipRgn(nullptr);
            }
            pDC->SelectStockObject(NULL_BRUSH);
            pDC->OffsetWindowOrg(-rect.left, -rect.top);
            pDC->Polygon(aPrecelWertex.get(), precelWertexCnt);
            pDC->OffsetWindowOrg(rect.left, rect.top);
            for (int i = 0; i < precelWertexCnt; ++i) {
                aPrecelWertex[i].x *= CLIENT_SCALE;
                aPrecelWertex[i].y *= CLIENT_SCALE;
            }
            break;
        default:
            for (int i = 0; i < precelWertexCnt; ++i) {
                aPrecelWertex[i].x /= CLIENT_SCALE;
                aPrecelWertex[i].y /= CLIENT_SCALE;
            }
            rgn.CreatePolyPolygonRgn(aPrecelWertex.get(), aRingWertexCnt, precelRingCnt, ALTERNATE);
            rgn.OffsetRgn(rect.left, rect.top);
            rgn.GetRegionData((RGNDATA*)theApp.bigBuf, bigSize);
            aRectClip = (CRect*)((RGNDATA*)theApp.bigBuf)->Buffer;
            iClipRegions = ((RGNDATA*)theApp.bigBuf)->rdh.nCount;
            rgn.DeleteObject();
            for (int i = 0; i < iClipRegions; ++i) {
                pDC->IntersectClipRect(aRectClip[i]);
                DrawKolor(pDC, &rect);
                pDC->SelectClipRgn(nullptr);
            }
            pDC->SelectStockObject(NULL_BRUSH);
            pDC->OffsetWindowOrg(-rect.left, -rect.top);
            pDC->PolyPolygon(aPrecelWertex.get(), aRingWertexCnt, precelRingCnt);
            pDC->OffsetWindowOrg(rect.left, rect.top);
            for (int i = 0; i < precelWertexCnt; ++i) {
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

    CRect r{rect.right - 3 * vscale, rect.bottom + 3 * vscale, rect.right, rect.bottom};
    pDC->Rectangle(r);
    pDC->SelectStockObject(NULL_BRUSH);
    r.SetRect(r.left, r.top + 2 * vscale, r.right, r.bottom + 1 * vscale);
    pDC->Ellipse(r);

    pDC->SelectObject(pOldBrush);
    pDC->SelectObject(pOldPen);
}

void CDrawAdd::Lock()
{
    if (fizpage != 0) {
        if (m_pDocument->swCZV == ToolbarMode::czas_obow)
            flags.reserv = !flags.reserv;
        else 
            flags.locked = !flags.locked;
        SetDirty();
    }
    Invalidate();
}

void CDrawAdd::UpdateInfo()
{
    info.Format(_T("Og�: %ix%i"), sizex, sizey);
    if (typ_xx) info += _T(" niestandardowe");
    if (!nazwa.IsEmpty()) info.AppendFormat(_T(" | %s"), (LPCTSTR)nazwa);
    if (nreps > 0) info.AppendFormat(_T(" | %li%s"), nreps, (LPCTSTR)wersja);
    if (m_add_xx > 0) info.AppendFormat(_T(" | Spacer: %i"), m_add_xx);
    if (!logpage.IsEmpty()) info.AppendFormat(_T(" | %s"), (LPCTSTR)logpage);
    if (!remarks.IsEmpty() || !remarks_atex.IsEmpty()) info.AppendFormat(_T(" | %s"), remarks.IsEmpty() ? (LPCTSTR)remarks_atex : (LPCTSTR)remarks);
    info.AppendFormat(_T(" | %s"), (kolor == ColorId::full) ? FULL : (LPCTSTR)CDrawDoc::kolory[kolor >> 3]);
    if (fizpage) info.AppendFormat(_T(" | na str: %s (%i,%i)"), (LPCTSTR)m_pDocument->GetPage(fizpage)->GetNrPaginy(), posx, posy);
}

CString CDrawAdd::PrepareBuf(const TCHAR* const ch) const
{
    CString bufor;
    bufor.AppendFormat(_T("%ix%i%s"), sizex, sizey, ch);
    if (theApp.includeKratka)
        bufor.AppendFormat(_T("%ix%i%s"), szpalt_x, szpalt_y, ch);

    if ((fizpage & PaginaType::roman) == PaginaType::roman)
        bufor.Append(CDrawObj::Rzymska(fizpage >> 16));
    else
        bufor.AppendFormat(_T("%i"), fizpage >> 16);

    bufor.AppendFormat(_T("%s%i%s%i%s%s%s"), ch, posx, ch, posy, ch, (LPCTSTR)nazwa, ch);
    bufor.AppendFormat(_T("%li"), nreps);
    bufor.AppendFormat(_T("%s%s%s%s%s%s%s%s\n"), ch, (LPCTSTR)CDrawDoc::kolory[kolor == ColorId::full ? 1 : kolor >> 3], ch, (LPCTSTR)logpage, ch, (LPCTSTR)remarks, ch, (LPCTSTR)wersja);

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
    dlg.m_rzymnum = ((fizpage & PaginaType::roman) == PaginaType::roman);
    dlg.m_fizpage = (fizpage >> 16);
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
    dlg.m_studio = (int)flags.studio;
    dlg.m_derived = flags.derived;
    dlg.m_spad = spad_flag;
    dlg.m_nag_xx = nag_xx;
    if (dlg.DoModal() != IDOK)
        return FALSE;

    nreps = dlg.m_nreps.IsEmpty() ? -1 : _ttol(dlg.m_nreps);
    oldAdno = dlg.m_oldadno == nreps ? -1 : dlg.m_oldadno;
    if (flags.isok == 0 && nreps != -1) flags.isok = 3;
    if (flags.isok == 1 && nreps != -1) flags.isok = 2;
    if (flags.isok == 2 && nreps == -1) flags.isok = 1;
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
    flags.epsok = dlg.m_epsok;
    if (dlg.m_epsok < 2) flags.showeps = dlg.m_epsok;
    flags.reserv = dlg.m_flaga_rezerw;
    flags.locked = (fizpage ? dlg.m_locked : FALSE);
    flags.studio = static_cast<StudioStatus>(dlg.m_studio);
    dlg.m_always ? czaskto.Format(_T("# [%s]"), (LPCTSTR)dlg.m_sprzedal) : czaskto.Format(_T("%s %s [%s]"), (LPCTSTR)dlg.m_data_czob.Format(c_ctimeData), (LPCTSTR)dlg.m_godz_czob.Format(_T("%H:%M")), (LPCTSTR)dlg.m_sprzedal);
    const int pom_fizpage = MAKELONG(dlg.m_rzymnum ? PaginaType::roman : PaginaType::arabic, dlg.m_fizpage);
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
            UpdateInfo();
            pPage->Invalidate();
        }
    } else if (dlg.m_precel_flag.IsEmpty()) {
        precelWertexCnt = precelRingCnt = 0;
        m_precel_flag.Empty();
    }
    if (szpalt_x != dlg.m_szpalt_x || szpalt_y != dlg.m_szpalt_y) {
        szpalt_x = dlg.m_szpalt_x;
        szpalt_y = dlg.m_szpalt_y;
        space = CFlag{sizex, sizey, szpalt_x, szpalt_y};
        const CRect pos{m_position.left, m_position.top, m_position.left + (int)(sizex*modulx), m_position.top - (int)(sizey*moduly)};
        MoveTo(pos);
    }
    if (!dlg.m_fromQue && (((theApp.grupa&UserRole::dea) > 0) || (theApp.grupa&UserRole::kie && m_pDocument->isRO)))
        if (m_add_xx > 0) {
            uint8_t eok = this->flags.epsok;
            auto powt = (int)(this->powtorka == 0 ? 0 : (this->powtorka - CTime(POWTSEED_0)).GetDays());
            uint8_t ile_kol = this->kolor & 0x07;
            CString czk;
            if (this->flags.reserv != 1) czk = this->czaskto;

            CManODPNETParms orapar {
                { CManDbType::DbTypeInt32, &this->m_pub_xx },
                { CManDbType::DbTypeByte,  &ile_kol },
                { CManDbType::DbTypeInt32, &CDrawDoc::spoty[this->kolor >> 3]},
                { CManDbType::DbTypeInt32, &powt },
                { CManDbType::DbTypeInt32, &this->oldAdno },
                { CManDbType::DbTypeVarchar2, &this->remarks },
                { CManDbType::DbTypeVarchar2, &this->wersja },
                { CManDbType::DbTypeVarchar2, &czk },
                { CManDbType::DbTypeByte, &eok }
            };
            theManODPNET.EI("begin spacer.update_reservation(:pub_xx,:ile_kol,:nr_spotu,:powtorka,:old_adno,:uwagi,:wersja,:czaskto,:eps_present); end;", orapar);
        } else
            AfxMessageBox(_T("Nie mo�na w ten spos�b modyfikowa� rezerwacji, kt�ra nie pochodzi za Spacera"));

    SetDirty();
    UpdateInfo();
    Invalidate();
    if (!pView) m_pDocument->ArrangeQue();
    return TRUE;
}

CFlag CDrawAdd::GetPlacementFlag() const
{
    return GetPlacementFlag(posx, posy);
}

CFlag CDrawAdd::GetPlacementFlag(const int px, const int py) const
{
#ifdef DEBUG
    ASSERT(0 < px && 0 < py && px <= szpalt_x && py <= szpalt_y);
#endif
    return space << ((szpalt_y - sizey - py + 2) * szpalt_x - sizex - px + 1);
}

void CDrawAdd::SetPosition(const int fizp, int px, int py, int sx, int sy)
{
    // gdy zmiana przez open wlasnosci lub import
    dirty = TRUE;
    if (fizpage == fizp && posx == px && posy == py && sizex == sx && sizey == sy) {
        UpdateInfo(); return;
    }

    CRect m_pos{m_position};
    auto pPage = m_pDocument->GetPage(fizp);
    sx = min(szpalt_x, sx); px = min(px, szpalt_x + 1 - sx);
    sy = min(szpalt_y, sy); py = min(py, szpalt_y + 1 - sy);
    bool ok;
    if (pPage == nullptr)
        ok = (fizp == 0);
    else
        ok = pPage->FindSpace(this, &px, &py, sx, sy);

    if (!ok) {
        if (pPage == nullptr) {  //nie istnieje ta strona np. makieta ma 44 strony a chcemy postawic na 48
            SetSpaceSize(sx, sy);
            m_pos.SetRect(m_position.left, m_position.top, m_position.left + (int)(sizex*modulx), m_position.top - (int)(sizey*moduly));
        } else { //tzn pPage nie jest NULL ale nie mozemy postawi�
            if (fizpage == 0) {
                SetSpaceSize(sx, sy);
                m_pos.SetRect(pPage->m_position.left, pPage->m_position.top, pPage->m_position.left + (int)(sizex*modulx), pPage->m_position.top - (int)(sizey*moduly));
            } else {
                pPage = m_pDocument->GetPage(fizpage);
                m_pos.SetRect(pPage->m_position.left + (int)(modulx * (posx - 1)), pPage->m_position.bottom + (int)(moduly * (szpalt_y - posy + 1)),
                              pPage->m_position.left + (int)(modulx * (posx + sizex - 1)), pPage->m_position.bottom + (int)(moduly * (szpalt_y - posy - sizey + 1)));
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

    if (pPage == nullptr)
        if (fizpage != 0) { // stawia z boku
            pPage = m_pDocument->GetPage(fizpage);
            m_pos.SetRect(pPage->m_position.left, pPage->m_position.top, pPage->m_position.left + (int)(sizex * modulx), pPage->m_position.top - (int)(sizey * moduly));
            posx = posy = fizpage = 0;
        } else // zmienia rozmiar
            m_pos.SetRect(m_position.left, m_position.top, m_position.left + (int)(sizex * modulx), m_position.top - (int)(sizey * moduly));
    else {
        m_pos.SetRect(pPage->m_position.left + (int)(modulx * (posx - 1)), pPage->m_position.bottom + (int)(moduly * (szpalt_y - posy + 1)), pPage->m_position.left + (int)(modulx * (posx + sizex - 1)), pPage->m_position.bottom + (int)(moduly * (szpalt_y - posy - sizey + 1)));
        pPage->AddAdd(this);
    }

    MoveTo(m_pos);
    UpdateInfo();
}

void CDrawAdd::SetPosition(CRect* m_pos, CDrawPage* pPage)
{
    int sx = min((int)std::nearbyint(std::abs(m_pos->right - m_pos->left) / modulx), szpalt_x);
    int sy = min((int)std::nearbyint(std::abs(m_pos->top - m_pos->bottom) / moduly), szpalt_y);
    int px = 0;
    int py = 0;
    bool ok{true};

    dirty = TRUE;
    if (!sx) sx = 1;
    if (!sy) sy = 1;

    if (pPage && (szpalt_x != pPage->szpalt_x || szpalt_y != pPage->szpalt_y)) {
        *m_pos -= CPoint(m_pos->left - pPage->m_position.left, m_pos->top - pPage->m_position.top);
        pPage = nullptr;
    }

    if (pPage != nullptr) {
        px = (int)nearbyint((m_pos->left - pPage->m_position.left) / modulx) + 1;
        const auto cx = szpalt_x + 1 - sx;
        if (cx < px) px = cx;
        py = szpalt_y - sy - (int)nearbyint((m_pos->bottom - pPage->m_position.bottom) / moduly) + 1;
        const auto cy = szpalt_y + 1 - sy;
        if (cy < py) py = cy;
        ok = pPage->FindSpace(this, &px, &py, sx, sy);
    }

    if (!ok) { // pPage nie jest NULL ale nie mozemy postawi�
        if (fizpage == 0) { // tworzymy nowe albo przesuwamy z boku
            SetSpaceSize(sx, sy);
            m_pos->SetRect(pPage->m_position.left, pPage->m_position.top, pPage->m_position.left + (int)(sizex*modulx), pPage->m_position.top - (int)(sizey*moduly));
        } else {
            pPage = m_pDocument->GetPage(fizpage);
            m_pos->SetRect(pPage->m_position.left + (int)(modulx*(posx - 1)), pPage->m_position.bottom + (int)(moduly*(szpalt_y - posy + 1)), pPage->m_position.left + (int)(modulx*(posx + sizex - 1)), pPage->m_position.bottom + (int)(moduly*(szpalt_y - posy - sizey + 1)));
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
        m_pos->SetRect(m_pos->left, m_pos->top, m_pos->left + (int)(sizex*modulx), m_pos->top - (int)(sizey*moduly));
        fizpage = 0;
    } else {
        pPage->AddAdd(this);
        m_pos->SetRect(pPage->m_position.left + (int)(modulx*(posx - 1)), pPage->m_position.bottom + (int)(moduly*(szpalt_y - posy + 1)), pPage->m_position.left + (int)(modulx*(posx + sizex - 1)), pPage->m_position.bottom + (int)(moduly*(szpalt_y - posy - sizey + 1)));
    }

    UpdateInfo();
}

void CDrawAdd::SetSpaceSize(const int sx, const int sy)
{
    sizex = sx;
    sizey = sy;
    if (precelWertexCnt == 0)
        space = CFlag(sx, sy, szpalt_x, szpalt_y);
}

void CDrawAdd::SetSpaceAndPosition(const int fizp, int px, int py)
{
    // gdy copy paste i save i open
    dirty = TRUE;
    if (!precelWertexCnt)
        space = CFlag(sizex, sizey, szpalt_x, szpalt_y);
    auto pPage = m_pDocument->GetPage(fizp);
    if (pPage == nullptr)
        return;
    pPage->SetBaseKrata(szpalt_x, szpalt_y, FALSE);
    if (pPage->FindSpace(this, &px, &py, sizex, sizey)) {
        posx = px; posy = py;
        pPage->AddAdd(this);
        m_position.SetRect(pPage->m_position.left + (int)(modulx*(posx - 1)), pPage->m_position.bottom + (int)(moduly*(szpalt_y - posy + 1)),
            pPage->m_position.left + (int)(modulx*(posx + sizex - 1)), pPage->m_position.bottom + (int)(moduly*(szpalt_y - posy - sizey + 1)));
    } else
        m_position.SetRect(pPage->m_position.left, pPage->m_position.top, pPage->m_position.left + (int)(sizex*modulx), pPage->m_position.top - (int)(sizey*moduly));
    UpdateInfo();
}

bool CDrawAdd::PtOnRing(const CPoint p) const
{
    for (int i = 0; i < precelWertexCnt - 1; ++i) {
        if (aPrecelWertex[i].y != aPrecelWertex[i + 1].y || aPrecelWertex[i].y != p.y)
            continue;
        int r1 = aPrecelWertex[i].x;
        int r2 = aPrecelWertex[i + 1].x;
        if (r1 > r2) std::swap(r1, r2);
        if (r1 <= p.x && p.x + 1 <= r2)
            return true;
    }
    return false;
}

int CDrawAdd::FindRing(const CPoint p0, const bool bOuterRing)
{
    enum class Kierunek : uint8_t { E, S, W, N }; // okresla kierunek ostatnio znalezionego fragmentu obwodnicy
    Kierunek eKierunek = Kierunek::E;
    const int iInitVertexCnt = precelWertexCnt;
    aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p0.x*modulx), static_cast<int>(-p0.y*moduly));
    CPoint p(p0.x + 1, p0.y);
    aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));

    ASSERT(0 <= p0.x && 0 <= p0.y && p0.x <= sizex && p0.y <= sizey && precelRingCnt < ciMaxRings);

    while (p != p0) {
        switch (eKierunek) {
            case Kierunek::E:
                if (p.y > 0 && p.x < szpalt_x && space[(p.y - 1)*szpalt_x + p.x] == bOuterRing) {
                    p.y--;
                    eKierunek = Kierunek::N;
                    aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
                } else if (p.x < sizex && p.y < szpalt_y && space[p.y*szpalt_x + p.x] == bOuterRing) {
                    p.x++;
                    if (aPrecelWertex[precelWertexCnt - 2].y == -p.y*moduly)
                        aPrecelWertex[precelWertexCnt - 1] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
                    else
                        aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
                } else {
                    ASSERT(p.y < sizey);
                    p.y++;
                    eKierunek = Kierunek::S;
                    aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
                }
                break;
            case Kierunek::S:
                if (p.x < sizex && p.y < sizey && space[p.y*szpalt_x + p.x] == bOuterRing) {
                    p.x++;
                    eKierunek = Kierunek::E;
                    aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
                } else if (p.y < sizey && space[p.y*szpalt_x + p.x - 1] == bOuterRing) {
                    p.y++;
                    if (aPrecelWertex[precelWertexCnt - 2].x == p.x*modulx)
                        aPrecelWertex[precelWertexCnt - 1] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
                    else
                        aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
                } else {
                    ASSERT(p.x - 1 >= 0);
                    p.x--;
                    eKierunek = Kierunek::W;
                    aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
                }
                break;
            case Kierunek::W:
                if (p.x > 0 && p.y < sizey && space[p.y*szpalt_x + p.x - 1] == bOuterRing) {
                    p.y++;
                    eKierunek = Kierunek::S;
                    aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
                } else if (p.x > 0 && p.y > 0 && space[(p.y - 1)*szpalt_x + p.x - 1] == bOuterRing) {
                    p.x--;
                    if (aPrecelWertex[precelWertexCnt - 2].y == -p.y*moduly)
                        aPrecelWertex[precelWertexCnt - 1] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
                    else
                        aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
                } else {
                    ASSERT(p.y > 0);
                    p.y--;
                    eKierunek = Kierunek::N;
                    aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
                }
                break;
            case Kierunek::N:
                if (p.x > 0 && p.y > 0 && space[(p.y - 1)*szpalt_x + p.x - 1] == bOuterRing) {
                    p.x--;
                    eKierunek = Kierunek::W;
                    aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
                } else if (p.y > 0 && space[(p.y - 1)*szpalt_x + p.x] == bOuterRing) {
                    p.y--;
                    if (aPrecelWertex[precelWertexCnt - 2].x == p.x*modulx)
                        aPrecelWertex[precelWertexCnt - 1] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
                    else
                        aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
                } else {
                    ASSERT(p.x + 1 <= sizex);
                    p.x++;
                    eKierunek = Kierunek::E;
                    aPrecelWertex[precelWertexCnt++] = CPoint(static_cast<int>(p.x*modulx), static_cast<int>(-p.y*moduly));
                }
                break;
        }
    }

    return --precelWertexCnt - iInitVertexCnt;
}

void CDrawAdd::InitPrecel(const CString& sPrecelFlag)
{
    m_precel_flag = sPrecelFlag;
    precelWertexCnt = precelRingCnt = 0;
    // zarzadanie pamiecia
    const size_t maxWertexCnt = (sizex + 1) * (sizey + 1);
    aPrecelWertex = std::make_unique<CPoint[]>(maxWertexCnt + 2); // na ko�cu int[4]
    aRingWertexCnt = reinterpret_cast<int*>(&aPrecelWertex[maxWertexCnt]);
    // poczatkowo precel jest rowny jego otoczeniu i trzeba wylaczyc niektore moduly
    space = CFlag(sizex, sizey, szpalt_x, szpalt_y);
    // zera do r�nicy d�ugo�ci + (zera do pe�nych 32 bit�w + bity flagi)
    space ^= CFlag(CString('0', 8 * ((int)space.GetSize() / 4 - (int)std::ceil((float)sPrecelFlag.GetLength() / 8))) + CString('0', (8888 - sPrecelFlag.GetLength()) % 8) + sPrecelFlag);

    // skanuj flag� w poszukiwaniu obwodnic
    for (int x = 0; x < sizex; ++x)
        if (space[x] && !PtOnRing(CPoint(static_cast<int>(x * modulx), 0)))
            aRingWertexCnt[precelRingCnt++] = FindRing(CPoint(x, 0), true);
    for (int y = 1; y < sizey; ++y)
        for (int x = 0; x < sizex; ++x) {
            if (!space[(y - 1)*szpalt_x + x] && space[y*szpalt_x + x] && !PtOnRing(CPoint(static_cast<int>(x*modulx), static_cast<int>(-y*moduly)))) {
                aRingWertexCnt[precelRingCnt++] = FindRing(CPoint(x, y), true);
                continue;
            }
            if (space[(y - 1)*szpalt_x + x] && !space[y*szpalt_x + x] && !PtOnRing(CPoint(static_cast<int>(x*modulx), static_cast<int>(-y*moduly))))
                aRingWertexCnt[precelRingCnt++] = FindRing(CPoint(x, y), false);
        }

    // wyrownanie do kraty	
    const auto iRightBound = static_cast<int>(sizex * modulx);
    for (int i = 0; i < precelWertexCnt; ++i) {
        if (aPrecelWertex[i].x == iRightBound) aPrecelWertex[i].x -= 11;
    }
    // dosun mape precla do prawej krawedzi strony
    space.Reverse(szpalt_x * sizey);
    space >>= szpalt_x - sizex;
}

void CDrawAdd::MoveWithPage(const CRect& position, CDrawView* pView)
{
    dirty = TRUE;
    const CRect toPos(position.left + (int)(modulx*(posx - 1)), position.bottom + (int)(moduly*(szpalt_y - posy + 1)), position.left + (int)(modulx*(posx + sizex - 1)), position.bottom + (int)(moduly*(szpalt_y - posy - sizey + 1)));
    MoveTo(toPos, pView);
}

void CDrawAdd::SetLogpage(const CString& m_op_zew, const CString& m_sekcja, const CString& m_op_sekcji, const int m_nr_w_sekcji, const CString& m_PL, const CString& m_op_PL, const int m_nr_PL, CString& m_poz_na_str)
{
    /* vu : Formatuje wartosci dostarczone na zmiennych tak, aby powstal napis warunkow logicznych
            Zalozenie:   SetLogpage o ParseLogpage = Id									end vu */

    dirty = TRUE;
    logpage = m_op_zew;
    if (!m_sekcja.IsEmpty()) {
        logpage += m_sekcja;
        if (m_op_sekcji.GetLength() == 1 && (m_op_sekcji[0] == _T('=') || m_op_sekcji[0] == _T('<') || m_op_sekcji[0] == _T('>'))) {
            logpage += m_op_sekcji;
            if (m_nr_w_sekcji > 0) 
                logpage.AppendFormat(_T("%i"), m_nr_w_sekcji);
        }
        logpage.Append(_T(" "));
    }
    if (m_PL == _T("P") || m_PL == _T("L")) {
        logpage += m_PL;
        if (m_op_PL.GetLength() == 1 && (m_op_PL[0] == _T('=') || m_op_PL[0] == _T('<') || m_op_PL[0] == _T('>'))) {
            logpage += m_op_PL;
            if (m_nr_PL > 0)
                logpage.AppendFormat(_T("%i"), m_nr_PL);
        }
        logpage.Append(_T(" "));
    }
    if (m_poz_na_str.GetLength() > 0) {
        m_poz_na_str.MakeUpper();
        if (m_poz_na_str == _T("G") || m_poz_na_str == _T("D") || m_poz_na_str == _T("L") || m_poz_na_str == _T("P") ||
            m_poz_na_str == _T("DL") || m_poz_na_str == _T("DP") || m_poz_na_str == _T("GL") || m_poz_na_str == _T("GP") ||
            m_poz_na_str == _T("X") || m_poz_na_str == _T("Y") || m_poz_na_str == _T("XY"))
            logpage += ("# " + m_poz_na_str);
    }
}

void CDrawAdd::ParseLogpage(TCHAR* op_zew, TCHAR* sekcja, TCHAR* op_sekcji, int* nr_sek, TCHAR* pl, TCHAR* op_pl, int* nr_pl, TCHAR* poz_na_str)
{
    /* vu :	Obiekt ogloszenie ma pole CString logpage. Funkcja ParseLogpage interpretuje jego zawartosc zgodnie
            z sugestiami zawartymi w dokumencie warlog.doc i uzyskane wartosci przekazuje do argumentow funkcji.
            Jako separator pomiedzy specyfikacja strony a atrybutem poz_na_str stosuje sie znak '#'		end vu */

    int j, i = 0;
    TCHAR ch, nr_buf[20], lp[50];

    ::StringCchCopy(lp, 50, _T(" "));
    if (logpage.SpanExcluding(lp).GetLength() < 1) {
        op_zew[0] = sekcja[0] = op_sekcji[0] = pl[0] = op_pl[0] = TCHAR(0);
        if (poz_na_str) poz_na_str[0] = TCHAR(0);
        *nr_sek = *nr_pl = 0;
        return;
    }

    // potrzebuje duze litery i '\0' na koncu napisu
    if (logpage[0] == _T('"')) logpage.SetAt(0, _T(' '));
    logpage.MakeUpper();
    ::StringCchCopy(lp, 50, logpage);

    // wczytaj op_zew jesli jest
    ch = lp[i];
    op_zew[0] = TCHAR{0};
    while (ch == _T(' ') || ch == _T('<') || ch == _T('=') || ch == _T('>')) {
        if (ch != _T(' ')) {
            op_zew[0] = ch;
            op_zew[1] = TCHAR(0);
        }
        ch = lp[++i];
    }

    // wczytaj sekcje je�li jest okre�lona
    if (ch != _T('#') && !((ch == _T('L') || ch == _T('P')) && !_istalpha(lp[i + 1]))) { //sekcja okreslona
        j = 0;
        while ((int8_t)ch < 0 || isalnum((int)ch) || ch == _T('/')) { //ch<0 to polski znak
            sekcja[j++] = ch;
            ch = lp[++i];
        }
        sekcja[j] = TCHAR(0);

        // wczytaj op_sekcji
        while (ch != TCHAR(0) && ch != _T('<') && ch != _T('=') && ch != _T('>') && ch != _T('L') && ch != _T('P') && ch != _T('#'))
            ch = lp[++i];
        if (ch == TCHAR(0) || ch == _T('#') || ch == _T('L') || ch == _T('P')) op_sekcji[0] = TCHAR(0);
        else {
            op_sekcji[0] = ch;
            op_sekcji[1] = TCHAR(0);
        }

        // wczytaj nr_sek
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
        sekcja[0] = op_sekcji[0] = '\0'; // end of wczytaj sekcje je�li jest okre�lona
        *nr_sek = 0;
    }

    // wczytaj lp
    while (ch != '\0' && ch != 'L' && ch != 'P' && ch != '#')
        ch = lp[++i];
    if (ch != '#' && ch != '\0') { // wczytaj op_lp
        pl[0] = ch;
        pl[1] = '\0';
        while (ch != '#' && ch != '\0' && ch != '<' && ch != '=' && ch != '>')
            ch = lp[++i];
        if (ch != '#' && ch != '\0') { // wczytaj nr_lp
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
        while (ch != '\0' && !isupper((int)ch))
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

int CDrawAdd::CkPageLocation(const int nr_porz)
{
    /* vu: 	Sprawdza czy ogloszenie vAdd moze stac na stronie o numerze nr_porz
            ze wzgledu na warunki logiczne ogloszenia i strony. Jezeli nie moze,
            to zwraca 2, jezeli moze, ale stoi na dobrej stronie nieprawidlowo
            zwraca 1, a jezeli wszystko jest w porzadku zwraca 0		end vu */

    int nr_sek, nr_pl, nr_off, vnr_sek = 0;
    TCHAR op_zew[2], op_sekcji[2], op_pl[2], sekcja[30], pl[2];

    // gdy nie stoi na �adnej stronie, to dobrze
    if (!nr_porz) return 0;
    // parsuj napis i ustaw zmienne 
    ParseLogpage(op_zew, sekcja, op_sekcji, &nr_sek, pl, op_pl, &nr_pl);
    // przejdz liste stron w dokumencie, zeby zapamietac jaki numer ma strona w danej sekcji 
    const auto pc = (int)m_pDocument->m_pages.size();
    for (int j = 1; j <= pc; ++j) {
        CDrawPage* page = m_pDocument->m_pages[j % pc];
        CString vStrLog{page->name};
        vStrLog.MakeUpper();
        // zliczaj strony z odpowiedni� sekcj�
        if (_tcsstr(vStrLog, sekcja))
            vnr_sek++;
        // czy to jest testowana strona ? otherwise nie ma co sprawedzac
        if (page->nr != nr_porz) continue;
        // niezgodne kolory - kolor strony 2 oznacza spot anonimowy
        if ((kolor & 7) > (page->kolor & 7) ||
            ((kolor&ColorId::spot) && (page->kolor&ColorId::spot) && (kolor >> 3) != m_pDocument->m_spot_makiety[(page->kolor >> 3)] && m_pDocument->m_spot_makiety[(page->kolor >> 3)])) return 2;
        // na stronach z dziedziczeniem koloru tylko czarne og�oszenia
        if (page->m_dervlvl == DervType::colo && kolor != ColorId::brak) return 2;
        // nie na sciezce
        if (sekcja[0] && !_tcsstr(vStrLog, sekcja)) return 2;
        // lewa - nieparzysty numer
        if (!_tcscmp(pl, _T("L")) && (page->pagina % 2)) return 2;
        // prawa - parzysty numer
        if (!_tcscmp(pl, _T("P")) && !(page->pagina % 2)) return 2;
        // polozenie pl
        if (pl[0] && op_pl[0]) {
            //brak numeru
            if (nr_pl < 1) return 2;
            if ((page->pagina - vnr_sek) % 2)	//czy sekcja zaczyna sie na stronie o parzstym numerze
                nr_off = (sekcja[0] ? ((page->pagina % 2) ? vnr_sek - 1 : vnr_sek + 1) : page->pagina); //czy numer odnosi sie do sekcji czy do gazety
            else
                nr_off = (sekcja[0] ? vnr_sek : page->pagina); //czy numer odnosi sie do sekcji czy do gazety
            // inna strona prawa/lewa niz zadano
            if (!_tcscmp(op_pl, _T("=")) && !_tcscmp(pl, _T("P")) && nr_off != 2 * nr_pl - 1) return 2;
            if (!_tcscmp(op_pl, _T("=")) && !_tcscmp(pl, _T("L")) && nr_off != 2 * nr_pl) return 2;
            // strona prawa/lewa o mniejszym numerze niz zadano
            if (!_tcscmp(op_pl, _T(">")) && !_tcscmp(pl, _T("P")) && nr_off <= 2 * nr_pl - 1) return 2;
            if (!_tcscmp(op_pl, _T(">")) && !_tcscmp(pl, _T("L")) && nr_off <= 2 * nr_pl) return 2;
            // strona prawa/lewa o wiekszym numerze niz zadano
            if (!_tcscmp(op_pl, _T("<")) && !_tcscmp(pl, _T("P")) && nr_off >= 2 * nr_pl - 1) return 2;
            if (!_tcscmp(op_pl, _T("<")) && !_tcscmp(pl, _T("L")) && nr_off >= 2 * nr_pl) return 2;
        }
        // polozenie w sekcji
        if (sekcja[0] && op_sekcji[0]) {
            // brak numeru
            if (nr_sek < 1) return 2;
            // inna strona w sekcji niz zadano
            if (!_tcscmp(op_sekcji, _T("=")) && vnr_sek != nr_sek) return 2;
            // strona o mniejszym numerze w sekcji niz zadano
            if (!_tcscmp(op_sekcji, _T(">")) && vnr_sek <= nr_sek) return 2;
            // strona o wiekszym numerze w sekcji niz zadano
            if (!_tcscmp(op_sekcji, _T("<")) && vnr_sek >= nr_sek) return 2;
        }
    }

    // jest na dobrej stronie, ale czy w dobrym miejscu?
    const int hashpos = logpage.ReverseFind(_T('#'));
    if (hashpos >= 0) {
        int j, k = 0;
        TCHAR tail[20];
        const int rc = logpage.GetLength();
        for (j = hashpos + 1; j < rc; ++j)
            if (isupper((int)logpage[j])) tail[k++] = logpage[j];
        tail[k] = TCHAR(0);
        if (_tcsstr(tail, _T("G")))
            if (posy > 1) return 1;
        if (_tcsstr(tail, _T("D")))
            if (posy + sizey - 1 < szpalt_y) return 1;
        if (_tcsstr(tail, _T("L")))
            if (posx > 1) return 1;
        if (_tcsstr(tail, _T("P")))
            if (posx + sizex - 1 < szpalt_x) return 1;
        if (_tcsstr(tail, _T("GP")))
            if (posy > 1 || posx + sizex - 1 < szpalt_x) return 1;
        if (_tcsstr(tail, _T("GL")))
            if (posx > 1 || posy > 1) return 1;
        if (_tcsstr(tail, _T("DL")))
            if (posx > 1 || posy + sizey - 1 < szpalt_y) return 1;
        if (_tcsstr(tail, _T("DP")))
            if (posx + sizex - 1 < szpalt_x || posy + sizey - 1 < szpalt_y) return 1;
    }

    return 0; // wszystko w porz�dku
}

void CDrawAdd::SetEstPagePos(const TCHAR* const description, CRect* vRect, CDrawPage* pPage)
{
    /* vu : Jako description - opis polozenia ogloszenia na stronie podaje sie
            jeden ze skrotow in ('P','L','G','D','Gl','GP','DL','DP')
            Algorytm szuka polozenia dla ogloszenia na stronie pPage najblizszego
            oczekiwaniom z description. Jezeli znajdzie, to stawia ogloszenie na
            stronie														end vu */

    int i, j;
    dirty = TRUE;

    if (!_tcscmp(description, _T("P")))               //na prawo - domyslnie od dolu
        for (i = 0; i <= szpalt_x - sizex; ++i)       //dla kazdego polozenia w poziomie
            for (j = 0; j <= szpalt_y - sizey; ++j)   //dla kazdego polozenia w pionie
                if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i, szpalt_y - sizey + 1 - j))
                    goto postaw;

    if (!_tcscmp(description, _T("L")))               //na lewo - domyslnie od dolu
        for (i = szpalt_x - sizex; i >= 0; --i)       //dla kazdego polozenia w poziomie
            for (j = 0; j <= szpalt_y - sizey; ++j)   //dla kazdego polozenia w pionie
                if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i, szpalt_y - sizey + 1 - j))
                    goto postaw;

    if (!_tcscmp(description, _T("D")))               //u dolu - domyslnie od prawej
        for (j = 0; j <= szpalt_y - sizey; ++j)       //dla kazdego polozenia w pionie
            for (i = 0; i <= szpalt_x - sizex; ++i)   //dla kazdego polozenia w poziomie
                if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i, szpalt_y - sizey + 1 - j))
                    goto postaw;

    if (!_tcscmp(description, _T("G")))               //u gory - domyslnie od prawej
        for (j = szpalt_y - sizey; j >= 0; --j)       //dla kazdego polozenia w pionie
            for (i = 0; i <= szpalt_x - sizex; ++i)   //dla kazdego polozenia w poziomie
                if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i, szpalt_y - sizey + 1 - j))
                    goto postaw;

    if (!_tcscmp(description, _T("DP")))              //na dole i na prawo - domyslnie na NE
        for (i = 0; i <= szpalt_x - sizex; ++i)       //dla kazdego polozenia w poziomie
            for (j = 0; j <= i; ++j)                  //dla kazdego polozenia w pionie
                if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i + j, szpalt_y - sizey + 1 - j)) {
                    i -= j; goto postaw;
                }

    if (!_tcscmp(description, _T("DL")))             //na dole i na lewo - domyslnie na NW
        for (i = szpalt_x - sizex; i >= 0; --i)      //dla kazdego polozenia w poziomie
            for (j = 0; j <= szpalt_x - sizex - i; ++j) //dla kazdego polozenia w pionie
                if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i - j, szpalt_y - sizey + 1 - j)) {
                    i += j; goto postaw;
                }

    if (!_tcscmp(description, _T("GP")))             //na gorze i na lewo - domyslnie na EW
        for (i = 0; i <= szpalt_x - sizex + 1; ++i)  //dla kazdego polozenia w poziomie
            for (j = szpalt_y - sizey; j >= szpalt_y - sizey - i; --j) //dla kazdego polozenia w pionie
                if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i + szpalt_y - sizey + 1 - j, szpalt_y - sizey + 1 - j)) {
                    i -= szpalt_y - sizey + 1 - j; goto postaw;
                }

    if (!_tcscmp(description, _T("GL")))             //na gorze i na lewo - domyslnie na EW
        for (i = szpalt_x - sizex - 1; i >= -1; --i) //dla kazdego polozenia w poziomie
            for (j = szpalt_y - sizey; j >= i - szpalt_x + sizex + szpalt_y - sizey; --j) //dla kazdego polozenia w pionie
                if (pPage->CheckSpace(this, szpalt_x - sizex - i - szpalt_y + sizey + j, szpalt_y - sizey + 1 - j)) {
                    i += szpalt_y - sizey + 1 - j; goto postaw;
                }

    // nie udalo sie - postaw gdziekolwiek - zacznij od lewego dolnego rogu
    vRect->SetRect(pPage->m_position.left, pPage->m_position.bottom + (int)(sizey*moduly),
        pPage->m_position.left + (int)(sizex*modulx), pPage->m_position.bottom);
    SetPosition(vRect, pPage);
    return;

postaw:
    posx = szpalt_x - sizex + 1 - i;
    posy = szpalt_y - sizey + 1 - j;
    vRect->SetRect(pPage->m_position.left + (int)((posx - 1) * modulx), pPage->m_position.bottom + (int)((szpalt_y - posy + 1) * moduly),
                   pPage->m_position.left + (int)((posx + sizex - 1) * modulx), pPage->m_position.bottom + (int)((szpalt_y - posy - sizey + 1) * moduly));
    pPage->AddAdd(this);
}

bool CDrawAdd::SetPagePosition(CRect* pRect, CDrawPage* pPage)
{
    const int hashpos = logpage.ReverseFind('#') + 1;
    const auto sufix = logpage.GetBuffer() + hashpos;
    const auto placements = {_T("DL"), _T("DP"), _T("GP"), _T("GL"), _T("D"), _T("G"), _T("L"), _T("P")};
    ASSERT(hashpos > 0);

    for (const auto& p : placements)
        if (_tcsstr(sufix, p))
            return SetStrictDescPos(p, pRect, pPage);

    return false;
}

bool CDrawAdd::SetStrictDescPos(LPCTSTR description, CRect* pRect, CDrawPage* pPage)
{
    /* vu : Jako description - opis polozenia ogloszenia na stronie podaje sie
            jeden ze skrotow in ('P','L','G','D','GL','GP','DL','DP')
            Algorytm szuka polozenia dla ogloszenia na stronie pPage
            spe�niaj�cego oczekiwaniom z description. Jezeli znajdzie, to stawia
            ogloszenie na stronie je�eli nie, to stawia obok makiety
            Je�li !vRect to funkcja jedynie sprawdza dopuszczalno��		end vu */

    int i, j;

    if (!_tcscmp(description, _T("P")))          //na prawo - domyslnie od dolu
        for (j = 0; j <= szpalt_y - sizey; ++j)  //dla kazdego polozenia w pionie     
            if (pPage->CheckSpace(this, szpalt_x - sizex + 1, szpalt_y - sizey + 1 - j)) {
                i = szpalt_x - sizex + 1; j = szpalt_y - sizey + 1 - j; goto postaw;
            }


    if (!_tcscmp(description, _T("L")))            //na lewo - domyslnie od dolu
        for (j = 0; j <= szpalt_y - sizey; ++j)    //dla kazdego polozenia w pionie
            if (pPage->CheckSpace(this, 1, szpalt_y - sizey + 1 - j)) {
                i = 1; j = szpalt_y - sizey + 1 - j; goto postaw;
            }

    if (!_tcscmp(description, _T("D")))            //u dolu - domyslnie od prawej
        for (i = 0; i <= szpalt_x - sizex; ++i)    //dla kazdego polozenia w poziomie
            if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i, szpalt_y - sizey + 1)) {
                i = szpalt_x - sizex + 1 - i; j = szpalt_y - sizey + 1; goto postaw;
            }

    if (!_tcscmp(description, _T("G")))            //u gory - domyslnie od prawej
        for (i = 0; i <= szpalt_x - sizex; ++i)    //dla kazdego polozenia w poziomie 
            if (pPage->CheckSpace(this, szpalt_x - sizex + 1 - i, 1)) {
                i = szpalt_x - sizex + 1 - i; j = 1; goto postaw;
            }

    if (!_tcscmp(description, _T("DP")))           //na dole i na prawo - domyslnie na NE
        if (pPage->CheckSpace(this, szpalt_x - sizex + 1, szpalt_y - sizey + 1)) {
            i = szpalt_x - sizex + 1; j = szpalt_y - sizey + 1; goto postaw;
        }

    if (!_tcscmp(description, _T("DL")))           //na dole i na lewo - domyslnie na NW
        if (pPage->CheckSpace(this, 1, szpalt_y - sizey + 1)) {
            i = 1; j = szpalt_y - sizey + 1; goto postaw;
        }

    if (!_tcscmp(description, _T("GP")))           //na gorze i na lewo - domyslnie na EW
        if (pPage->CheckSpace(this, szpalt_x - sizex + 1, 1)) {
            i = szpalt_x - sizex + 1; j = 1; goto postaw;
        }

    if (!_tcscmp(description, _T("GL")))           //na gorze i na lewo - domyslnie na EW
        if (pPage->CheckSpace(this, 1, 1)) {
            i = 1; j = 1; goto postaw;
        }

    // nie udalo sie postawic
    return false;

postaw:
    if (pRect) {
        posx = i;
        posy = j;
        pRect->SetRect(pPage->m_position.left + (int)((posx - 1) * modulx), pPage->m_position.bottom + (int)((szpalt_y - posy + 1) * moduly),
                       pPage->m_position.left + (int)((posx + sizex - 1) * modulx), pPage->m_position.bottom + (int)((szpalt_y - posy - sizey + 1) * moduly));
        pPage->AddAdd(this);
    }
    return true;
}

void CDrawAdd::SetDotM(const bool setFlag)
{
    int p = this->wersja.Find('m');
    if (setFlag) {
        if (p >= 0 || wersja.Find('z') >= 0) return;
        p = wersja.Find('.');
        if (p == -1)
            wersja += ".m";
        else
            wersja.Insert(p + 1, 'm');
    } else {
        if (p == -1) return;
        wersja.Delete(p, 1);
    }

    SetDirty();
}

bool CDrawAdd::BBoxFromFile(GENEPSARG* pArg, CFile& handle, float* x1, float* y1, float* x2, float* y2)
{
    size_t res = 1;
    bool bRet = false;
    const char pat[] = "%BoundingBox:";
    char* s = reinterpret_cast<char*>(pArg->cBigBuf);
    const auto filepos = (unsigned long)handle.GetPosition();

    while (res > 0) {                 // read block from the file
        res = handle.Read(s, n_size); // check if anything was read and no error occurred
        if (res == 0) break;          // buffer has to end with zero TCHAR to be treated as string
        s[res] = 0;                   // move the file pointer back, lets we can loose string read in partially
        if (res == n_size) handle.Seek(-(LONGLONG)sizeof(pat), CFile::current);
        const char* p = CManPDF::memstr(s, pat, sizeof(pat) - 1);
        if (p != nullptr && s + n_size - p >= sizeof(pat)) { // search for substring and get it's position
            bRet = sscanf_s(p + sizeof(pat), "%f %f %f %f", x1, y1, x2, y2) == 4;
            break;
        }
    }

    handle.Seek(filepos, CFile::begin);
    return bRet;
}

bool CDrawAdd::GetProdInfo(GENEPSARG* pArg, TCHAR* cKolor, float* bx1, float* by1, float* bx2, float* by2, int* ileMat)
{
    bool bReqProdInfo{false};
    // szukaj danych w bazie
    if (m_pub_xx > 0) {
        CString sBBox = theManODPNET.GetProdInfo(m_pub_xx, cKolor, ileMat);
        if (sBBox != _T("!")) {
            const bool status = _stscanf_s(sBBox, _T("%f %f %f %f"), bx1, by1, bx2, by2) == 4;
            if (!status) { // przesz�o przez EpsTest, ale nie ma ProdInfo
                bReqProdInfo = true;
                CManODPNETParms orapar { CManDbType::DbTypeInt32, &m_pub_xx };
                theManODPNET.EI("begin epstest.request_prod_info(:pub_xx); end;", orapar);
            } else
                return true;
        }
    }

    // szukaj BoundingBox w pliku
    if (powtorka > 0) pArg->pDlg->OglInfo(pArg->iChannelId, CString(_T("Pobieranie pliku z archiwum")));
    const CString& eps_name = EpsName(CManFormat::EPS, false);
    pArg->pDlg->OglInfo(pArg->iChannelId, eps_name);
    if (eps_name[1] != _T(':')) {
        f5_errInfo += (eps_name + _T(", "));
        return false;
    }

    CFile eps;
    if (!eps.Open(eps_name, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite)) {
        if (eps_name.Left(2) == _T("::"))
            f5_errInfo += eps_name.Mid(3) + (bReqProdInfo ? _T(" zam�wiono dodatkowe sprawdzenie, ") : _T(" brak materia�u, "));
        else {
            CDrawApp::SetErrorMessage(pArg->cBigBuf);
            f5_errInfo.AppendFormat(_T(" %li: %s"), nreps, pArg->cBigBuf);
        }

        return false;
    }
    if (!BBoxFromFile(pArg, eps, bx1, by1, bx2, by2))
        f5_errInfo.AppendFormat(_T("%li z�e BoundingBox, "), nreps);
    if (cKolor == nullptr) {
        eps.Close();
        return true;
    }
    // szukaj Koloru w pliku
    eps.SeekToBegin();
    CManPDF mp(pArg);
    const auto DPCpos = mp.SearchPattern(eps, "DocumentProcessColors:");
    if (DPCpos == CManPDF::ulNotFound)
        *cKolor = _T('-');
    else {
        auto buf = reinterpret_cast<char*>(pArg->cBigBuf);
        eps.Seek(DPCpos, CFile::begin);
        eps.Read(buf, 512);
        buf[512] = '\0';
        std::ignore = strtok(buf, CManPDF::sepline);
        bool jestBlack = false;
        int tokcount = 0;
        std::ignore = strtok(buf, CManPDF::septok);
        while (char* p = strtok(nullptr, CManPDF::septok)) {
            if (strtok(p, "Black") == nullptr) jestBlack = true;
            tokcount++;
        }
        *cKolor = (!jestBlack || tokcount > 1) ? _T('F') : _T('B');
    }
    eps.Close();

    return true;
} // GetProdInfo

bool CDrawAdd::CheckSrcFile(GENEPSARG* pArg)
{
    CString msg;
    f5_errInfo.Empty();

    int search_mode = typ_xx;
    const auto& rozm = m_pDocument->m_rozm;
dajWymiarKraty:
    // szukaj wymiar�w w cachu
    const auto pR = std::find_if(rozm.begin(), rozm.end(), [=](const CRozm& r) {
        return search_mode ? (search_mode == r.typ_xx) : (szpalt_x == r.szpalt_x && szpalt_y == r.szpalt_y);
    });

    if (pR == rozm.end()) {
        if (search_mode > 0) {
            search_mode = 0;
            goto dajWymiarKraty;
        }
        f5_errInfo += _T("Brak rozmiaru w bazie "); return false;
    }

    // check scale
    float x1{}, x2{}, y1{}, y2{};
    if (pArg->format != CManFormat::PDF) {
        int ileMat;
        TCHAR cKolor[2];
        if (!GetProdInfo(pArg, cKolor, &x1, &y1, &x2, &y2, &ileMat))
            return FALSE;

        if (this->kolor == ColorId::full && cKolor[0] == _T('-')) {
            msg.Format(_T("%li materia� jest prawdopodobnie Black, "), nreps);
            f5_errInfo += msg;
        }
        if (this->kolor == ColorId::brak && cKolor[0] == _T('F')) { // czy kolorowy material nie jest zadeklarowany jako Black
            msg.Format(_T("%li nie jest Black, "), nreps);
            f5_errInfo += msg;
        }
        if (this->flags.digital && ileMat < 2) { // czy dostarczono zar�wno tre�� papierow� i cyfrow�
            msg.Format(_T("%li nie dostarczono tre�ci cyfrowej lub papierowej, "), nreps);
        }
    } else { // PDF
        CManPDF pdf{pArg};
        CString eps_name = EpsName(pArg->format, false);
        if (!pdf.GetMediaBox(eps_name, &x1, &y1, &x2, &y2)) {
            msg.Format(_T("%li z�e MediaBox, "), nreps);
            f5_errInfo += msg;
        }
    }
    if (x1 == x2 || y1 == y2) {
        msg.Format(_T("%li zdegenerowany rozmiar, "), nreps);
        f5_errInfo += msg;
        return FALSE;
    }
    const auto scalx = (float)(((sizex * (pR->w + pR->sw) - pR->sw) * mm2pkt) / (x2 - x1));
    const auto scaly = (float)(((sizey * (pR->h + pR->sh) - pR->sh) * mm2pkt) / (y2 - y1));
    if (std::fabs(scalx - 1.0) > 0.1 || std::fabs(scaly - 1.0) > 0.1) {
        msg.Format(_T("%li inny rozmiar %.2fx%.2f, "), nreps, scalx, scaly);
        f5_errInfo += msg;
    } else if ((sizex == szpalt_x && scalx < 0.95) || (sizey == szpalt_y && scaly < 0.95)) {
        msg.Format(_T("%li ma pe�ne pole zadruku, "), nreps);
        f5_errInfo += msg;
    }

    if (powtorka != 0) { // skopiuj powtorke
        msg.Format(_T("ad=%li&dd=%s"), nreps, (LPCTSTR)m_pDocument->dayws);
        auto pFile = theApp.OpenURL(1, msg);
        if (pFile)
            try {
            pFile->Read(pArg->cBigBuf, bigSize);
            if (*reinterpret_cast<short*>(pArg->cBigBuf) != 0x4b4f) { // "OK"
                msg.Format(_T("%li nie mo�na skopiowa� powt�rki, "), nreps);
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

bool CDrawAdd::CopyNewFile(const CString& srcPath, const CString& dstPath)
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
                cs.Format(_T("Nieudana pr�ba kopiowania pliku\n\nz\t%s\ndo\t%s\n\nPlik �r�d�owy jest starszy ni� plik docelowy.\nSprawd� czy poprawnie zaznaczono pow�rk�."), (LPCTSTR)srcPath, (LPCTSTR)dstPath);
                ::MessageBox(nullptr, cs, APP_NAME, MB_OK | MB_ICONERROR);
            }
            return false;
        }
    }
    if (!::CopyFile(srcPath, dstPath, FALSE)) {
        CString cs;
        cs.Format(_T("B��d kopiowania (errno=%lu) pliku %s"), GetLastError(), (LPCTSTR)srcPath);
        ::MessageBox(nullptr, cs, APP_NAME, MB_OK | MB_ICONERROR);
        return false;
    }
    return true;
}

bool CDrawAdd::EpsFromATEX(const CString& num, const CString& dstPath)
{
    CString sAtexEPS = theApp.GetProfileString(_T("GenEPS"), _T("EpsATEX"), _T("T:\\ATEX\\EPS\\"));
    CFileFind ff;
    CString sAtexFile;
    sAtexFile.Format(_T("%s%i\\%s.eps"), (LPCTSTR)sAtexEPS, _ttoi(num.Mid(num.GetLength() - 2)), (LPCTSTR)num);
    return ff.FindFile(sAtexFile) && CopyFile(sAtexFile, dstPath + ".eps", FALSE);
}

CString CDrawAdd::FindZajawka(CString& root, const CString& ext) const
{
    CFileFind ff;
    CDrawApp::TryAppendDirSep(root);
    CString s = root + nazwa + ext;
    if (ff.FindFile(s)) return s;
    if (root == _T("\\") || !ff.FindFile(root + _T("*.*"))) return _T("");
    while (ff.FindNextFile()) {
        if (ff.IsDots()) continue;
        if (ff.IsDirectory()) {
            CString newRoot{root + ff.GetFileName()};
            s = FindZajawka(newRoot, ext);
            if (!s.IsEmpty()) return s;
        }
    }
    return _T("");
}

bool CDrawAdd::LocatePreview(CFile& fEps, unsigned long* lOffset, unsigned long* lSize)
{
    unsigned long header[3];
    fEps.SeekToBegin();
    if (fEps.Read(header, sizeof(header)) != sizeof(header))
        return false;

    if (header[0] == 0xC6D3D0C5L) {
        *lOffset = header[1];
        *lSize = header[2];
    } else {
        *lOffset = 0;
        *lSize = (unsigned long)fEps.GetLength();
    }

    return true;
}

CString CDrawAdd::EpsName(const CManFormat format, bool copyOldEPS, const bool bModifTest)
{
    const TCHAR* extension = manamExt[(int)format];
    const bool czy_zajawka = wersja.Find(_T('z')) >= 0;
    if (nreps == -1 && !czy_zajawka) return CString(_T("brak nr atexa"));
    theApp.SetRegistryBase(_T("GenEPS"));

    CFileFind ff;
    CString s, num, oldnum, t2e(_T(" zamie� .tif na ")), eps_path(theApp.GetString(_T("EpsSrc"), _T("")));
    t2e += extension;
    num.Format(_T("%ld"), nreps);
    if (eps_path.IsEmpty() && theApp.isOpiMode && powtorka == 0 && !czy_zajawka)
        return _T("::\\") + num + extension;
    CDrawApp::TryAppendDirSep(eps_path);

    if (czy_zajawka) {
        int status;
        CManODPNETParms orapar {
            { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &status },
            { CManDbType::DbTypeInt32, &nreps }
        };
        orapar.outParamsCount = 1;
        theManODPNET.EI("begin :cc := check_emisja_zajawki(:adno); end;", orapar);

        if (status == 0) { // zajawka nie przesz�a przez interface Zajawki 
            CString zajawkiDir = theApp.GetString((LPCTSTR) _T("EpsZajawki"), (LPCTSTR) _T("."));
            const CString zaj_path = FindZajawka(zajawkiDir, CString(extension));
            return zaj_path.IsEmpty() ? nazwa + CString(extension) + _T(" - brak zajawki") : zaj_path;
        }
        if (powtorka == 0) { // adno_seed zajawki
            num.Format(_T("%s%i%s"), (LPCTSTR)m_pDocument->daydir, status, extension);
            return (theApp.isOpiMode ? CString(_T("::\\")) : eps_path) + num;
        }
    }

    if (theApp.GetInt(_T("DayDirs"), 0)) { // katalogi dniowe
        if (theApp.isOpiMode) {
            s = _T("::\\") + num;
            goto modifTest;
        } else if (powtorka == 0) { //nowy material
            s = eps_path + m_pDocument->daydir + num;
            if (ff.FindFile(s + extension)) goto modifTest;
            ff.Close();
            if (ff.FindFile(s + _T(".tif"))) return num + t2e;
            if (format != CManFormat::PDF && EpsFromATEX(num, s))
                goto modifTest;
        } else { // powtorka
            if (oldAdno > 0) // z numeru
                oldnum.Format(_T("%ld"), oldAdno);
            CString sFileName(oldnum.IsEmpty() ? num : oldnum);
            CString pathtail = powtorka.Format(c_ctimeDataWs) + _T("\\") + sFileName;
            s = eps_path + pathtail;

            if (ff.FindFile(s + extension)) { // znaleziono plik z powt�rk�
                WIN32_FILE_ATTRIBUTE_DATA actualAttr;
                if (copyOldEPS && ::GetFileAttributesEx(eps_path + m_pDocument->daydir + num + _T(".eps"), GetFileExInfoStandard, &actualAttr)) { // mo�e wgrano nowy material i nie zdjeto flagi powtorki
                    WIN32_FILE_ATTRIBUTE_DATA powtAttr;
                    if (::GetFileAttributesEx(s + extension, GetFileExInfoStandard, &powtAttr))
                        if (actualAttr.ftLastWriteTime.dwHighDateTime > powtAttr.ftLastWriteTime.dwHighDateTime ||
                            (actualAttr.ftLastWriteTime.dwHighDateTime == powtAttr.ftLastWriteTime.dwHighDateTime && actualAttr.ftLastWriteTime.dwLowDateTime > powtAttr.ftLastWriteTime.dwLowDateTime)) {
                            CString msg;
                            msg.Format(_T("Sprawd� czy poprawnie zaznaczono pow�rk�:\n\n%s\n\nPlik docelowy jest nowszy ni� archiwum."), (LPCTSTR)num);
                            ::MessageBox(nullptr, msg, APP_NAME, MB_OK | MB_ICONERROR);
                            copyOldEPS = FALSE;
                        }
                }

                if (copyOldEPS /*&& !inArch*/) { // skopiuj powt�rk�
                    CString newPath;
                    newPath.Format(_T("%s%s%s"), (LPCTSTR)eps_path, (LPCTSTR)m_pDocument->daydir, (LPCTSTR)num);
                    CopyNewFile(s + extension, newPath + _T(".eps"));
                    s = newPath;
                }
                goto modifTest;
            }
            ff.Close();
            if (ff.FindFile(s + _T(".tif"))) return num + t2e;
        }
    } else {
        CString zera;
        int poIle = theApp.GetInt(_T("Hashing"), 2);
        const int numlen = num.GetLength();
        if (numlen == 7) poIle--;
        zera = CString('0', max(0, numlen - poIle));

        if (theApp.GetInt(_T("SubDirSearch"), 1)) {
            s.Format(_T("%s%s%s\\%s"), (LPCTSTR)eps_path, (LPCTSTR)num.Left(poIle), (LPCTSTR)zera, (LPCTSTR)num);
            if (ff.FindFile(s + extension)) goto modifTest;
            ff.Close();
            if (ff.FindFile(s + ".tif")) return num + t2e;
            ff.Close();
            zera = theApp.GetString(_T("EpsOld"), _T("!!stare"));
            CDrawApp::TryAppendDirSep(zera);
            zera = eps_path + zera + num;
            if (ff.FindFile(zera + extension)) {
                s = zera;
                goto modifTest;
            }
            ff.Close();
            if (ff.FindFile(zera + _T(".tif"))) return num + t2e;
        } else {
            s = eps_path + num;
            if (ff.FindFile(s + extension)) goto modifTest;
            ff.Close();
            if (ff.FindFile(s + _T(".tif"))) return num + t2e;
        }
        if (format != CManFormat::PDF && EpsFromATEX(num, s))
            goto modifTest;
    }
    return (num + _T(" brak ") + extension);
modifTest:
    if (bModifTest) {
        ff.FindNextFile();
        ff.GetLastWriteTime(epsDate);
    }
    return s + extension;
} // EpsName

bool CDrawAdd::RewriteEps(GENEPSARG* pArg, CFile& dest)
{
    CString eps_name;
    CDrawAdd* pLeftAdd = this;
    auto s = reinterpret_cast<char*>(pArg->cBigBuf);
    auto pPage = m_pDocument->GetPage(fizpage);
    const bool bLewaStrona = (m_pDocument->GetIPage(pPage) & 1) == 0;
    const auto pRozAdd = m_pDocument->GetCRozm(szpalt_x, szpalt_y, spad_flag ? 0 : typ_xx); // gdy zaznaczono spad, to montuj do kraty
    const auto pRozKraty = typ_xx ? m_pDocument->GetCRozm(szpalt_x, szpalt_y, 0) : pRozAdd; // pobierz rozmiar kraty, je�li dotychczas nie jest znany

    theApp.SetRegistryBase(_T("GenEPS"));
    const BOOL copyEps = theApp.GetInt(_T("CopyOldEPS"), 0);
    pArg->pDlg->OglInfo(pArg->iChannelId, CString(_T("kopiowanie powtorki")));
    if (pPage->m_typ_pary == 2 && !bLewaStrona) { // prawa strona rozkladowki
        CDrawPage *pPrevPage = fizpage > 0xffff ? m_pDocument->GetPage(fizpage - 0x10000) : m_pDocument->m_pages[0];
        if (pPrevPage->m_adds.empty()) {
            ::MessageBox(pArg->pDlg->m_hWnd, _T("Na lewej stronie rozklad�wki nie ma og�oszenia"), _T("Brak og�oszenia"), MB_ICONSTOP);
            return false;
        }
        pLeftAdd = pPrevPage->m_adds.front();
        if (pLeftAdd) eps_name = pLeftAdd->EpsName(CManFormat::EPS, copyEps);
    } else
        eps_name = wersja == DERV_TMPL_WER ? nazwa : EpsName(CManFormat::EPS, copyEps); // albo nazwa pliku ze sciezka, albo info o bledzie
    pArg->pDlg->OglInfo(pArg->iChannelId, eps_name);

    auto x = (float)((posx - 1) * (pRozKraty->w + pRozKraty->sw) * mm2pkt);
    auto y = (float)((szpalt_y + 1 - posy - sizey) * (pRozKraty->h + pRozKraty->sh) * mm2pkt + (pArg->bSignAll ? podpisH : podpisH / 2)); // miejsce gdzie sie zaczyna eps bez podpisu
    const auto gpx = (float)((sizex * (pRozAdd->w + pRozAdd->sw) - pRozAdd->sw) * mm2pkt);
    const auto gpy = (float)((sizey * (pRozAdd->h + pRozAdd->sh) - pRozAdd->sh) * mm2pkt);

    if (eps_name.Mid(1, 1) != _T(":")) {
        if (pArg->format == CManFormat::PS && pArg->bDoKorekty == 0)
            ::MessageBox(pArg->pDlg->m_hWnd, _T("Na produkcyjnej wersji kolumny brakuje materia�u: ") + eps_name, _T("Brak og�oszenia"), MB_ICONWARNING);

        ::StringCchPrintfA(s, n_size, ".25 .1 .9 %.2f %.2f %.2f %.2f R\r\n", gpx, gpy, x, y); dest.Write(s, static_cast<UINT>(std::strlen(s)));
        ::StringCchPrintfA(s, n_size, "(%s) %.2f %.2f %d /Switzerland Po\r\n", (LPCSTR)CStringA(eps_name), (x + gpx), (y - podpisH), podpisH); dest.Write(s, static_cast<UINT>(std::strlen(s)));
        return false;
    }

    TRY {
        // kopiuj podwaly
        CString podwal(theApp.GetString(_T("EpsPodwaly"), _T("")));
    if (copyEps && podwal.Find(_T(':')) >= 0 && wersja != DERV_TMPL_WER && pPage->name.Find(_T("RED")) >= 0) {
        CDrawApp::TryAppendDirSep(podwal);
        const CString fname = eps_name.Mid(eps_name.ReverseFind(_T('\\')) + 1);
        if (theApp.GetInt(_T("PodwalySubDir"), 0) == 0)
            podwal += fname;
        else {
            CString sPodwalDir;
            sPodwalDir.Format(_T("%s%s%s\\"), (LPCTSTR)podwal, (LPCTSTR)theApp.activeDoc->data.Mid(3, 2), (LPCTSTR)theApp.activeDoc->data.Mid(0, 2));
            if (::CreateDirectory(sPodwalDir, nullptr) || GetLastError() == ERROR_ALREADY_EXISTS) {
                sPodwalDir.AppendFormat(_T("%s%s\\"), (LPCTSTR)theApp.activeDoc->gazeta.Mid(0, 3), (LPCTSTR)theApp.activeDoc->gazeta.Mid(4, 2));
                if (::CreateDirectory(sPodwalDir, nullptr) || GetLastError() == ERROR_ALREADY_EXISTS)
                    podwal = sPodwalDir + fname;
            } else
                ::MessageBox(pArg->pDlg->m_hWnd, CString(_T("Nie mo�na utworzy� katalogu: ")) + sPodwalDir, _T("B��d"), MB_ICONERROR);
        }
        pArg->pDlg->OglInfo(pArg->iChannelId, _T("Podwa�: ") + podwal);
        CopyNewFile(eps_name, podwal);
        if (!theApp.GetInt(_T("Podwaly"), 0))
            return true;
    }

    if (spad_flag == 0 && pPage->m_typ_pary < 2) { // podpisujemy tylko pasowane
        CStringA dopodpisu;
        if (theApp.activeDoc->gazeta.Left(3) == _T("TBP")) {
            CString uPodpis(_T(' '), 64);
            CManODPNETParms orapar {
                { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &uPodpis },
                { CManDbType::DbTypeInt32, &this->m_pub_xx }
            };
            orapar.outParamsCount = 1;
            theManODPNET.EI("begin :pod := pagina.podpis(:pub_xx); end;", orapar);
            dopodpisu = uPodpis;
        } else
            dopodpisu.Format("%li%s", nreps, (LPCSTR)CStringA(wersja));

        ::StringCchPrintfA(s, n_size, "(%s) %.2f %.2f %d /Switzerland Po\r\n", (LPCSTR)dopodpisu, (x + gpx), (y - podpisH), podpisH); dest.Write(s, static_cast<UINT>(std::strlen(s)));
        if (this->flags.reksbtl) {
            ::StringCchPrintfA(s, n_size, "(REKLAMA) %.2f %.2f 1 %d /Switzerland textLR\r\n", x, y - podpisH, podpisH);
            dest.Write(s, static_cast<UINT>(std::strlen(s)));
        }
    }

    float x1{}, x2{}, y1{}, y2{};
    if (!pLeftAdd->GetProdInfo(pArg, nullptr, &x1, &y1, &x2, &y2, nullptr)) {
        ::MessageBox(pArg->pDlg->m_hWnd, f5_errInfo, _T("B��d"), MB_ICONERROR);
        f5_errInfo.Empty();
        return false;
    }
    const float adw = x2 - x1, adh = y2 - y1;

    ::StringCchPrintfA(s, n_size, "beginManamEPS \r\n"); dest.Write(s, static_cast<UINT>(std::strlen(s)));
    if (pPage->m_typ_pary < 2) {
        // spad zdefiniowany dla krawedzi
        x += static_cast<uint8_t>(aSpadInfo[spad_flag].adjust_x) * (gpx - adw) / 2;
        y += static_cast<uint8_t>(aSpadInfo[spad_flag].adjust_y) * (gpy - adh) / 2;
        ::StringCchPrintfA(s, n_size, "%.2f %.2f translate\r\n", x, y);  dest.Write(s, static_cast<UINT>(std::strlen(s)));
        // skalowanie
        const float scalx = (x2 == x1 || !pRozAdd->scale_it || !aSpadInfo[spad_flag].scale_x) ? 1 : gpx / adw;
        const float scaly = (y2 == y1 || !pRozAdd->scale_it || !aSpadInfo[spad_flag].scale_y) ? 1 : gpy / adh;
        if (scalx != 1 || scaly != 1) {
            ::StringCchPrintfA(s, n_size, "%.4f %.4f scale\r\n", scalx, scaly);  dest.Write(s, static_cast<UINT>(std::strlen(s)));
        }
    }

    if (pPage->m_typ_pary == 2 || spad_flag == 15 || // produkcja pary z jednego materia�u || przy pe�nym spadzie
        (bLewaStrona && spad_flag == 11 && sizex == szpalt_x && sizey == szpalt_y)) { // na lewej rozk�ad�wce dodatkowy napis REKLAMA
        double dMargDoGrb;
        CString uTrans(_T(' '), 8);
        CString uReklama(_T(' '), 32);
        CManODPNETParms orapar {
            { CManDbType::DbTypeInt32, &m_pDocument->m_mak_xx },
            { CManDbType::DbTypeInt32, &pPage->id_str },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &uTrans },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &uReklama },
            { CManDbType::DbTypeDouble, CManDbDir::ParameterOut, &dMargDoGrb }
        };
        orapar.outParamsCount = 3;
        theManODPNET.EI("begin pagina.get_ppole_trans(:mak_xx,:str_xx,:trans,:rekl,:mdg); end;", orapar);

        CStringA sTrans(uTrans);
        CStringA sReklama(uReklama);
        if (!sTrans.IsEmpty()) {
            if (pPage->m_typ_pary == 2) {
                const auto margDoGrb = static_cast<float>(dMargDoGrb);
                const auto halfSize = 0.5F * adw;
                if (bLewaStrona)
                    x += margDoGrb - halfSize + gpx;
                else
                    x -= margDoGrb + halfSize;
                y += 0.5F * (gpy - adh);
                ::StringCchPrintfA(s, n_size, "%.2f %.2f translate %s translate\r\n", x, y, (LPCSTR)sTrans); dest.Write(s, static_cast<UINT>(std::strlen(s)));
                if (bLewaStrona) {
                    ::StringCchPrintfA(s, n_size, ".5 (%s) 0 -5.5 1 7 /GWFranklinBold textKLR\r\n", (LPCSTR)sReklama); dest.Write(s, static_cast<UINT>(std::strlen(s)));
                }
            } else {
                const char* f = (strncmp("0 26", sTrans, 4) == 0) ? "/GWTimesCondensed" : "/GWFranklinBold";
                if (!bLewaStrona || spad_flag == 11)
                    ::StringCchPrintfA(s, n_size, "%s translate 90 rotate 2 (%s) 0 1.5 1 7 %s textKLR -90 rotate\r\n", (LPCSTR)sTrans, (LPCSTR)sReklama, f);
                else
                    ::StringCchPrintfA(s, n_size, "%s translate 90 rotate 2 (%s) 0 %.2f 1 7 %s textKLR -90 rotate\r\n", (LPCSTR)sTrans, (LPCSTR)sReklama, x1 - x2 - 7, f);
                dest.Write(s, static_cast<UINT>(std::strlen(s)));
            }
        } else {
            ::MessageBox(pArg->pDlg->m_hWnd, _T("Aby uzyska� par� kolumn z jednego materia�u wyga� wszystkie elementy paginy"), APP_NAME, MB_ICONINFORMATION);
            return false;
        }
    }

    ::StringCchPrintfA(s, n_size, "%.2f %.2f translate\r\n", -x1, -y1); dest.Write(s, static_cast<UINT>(std::strlen(s)));
    ::StringCchPrintfA(s, n_size, "%.2f %.2f %.2f %.2f clipEPS \r\nclip newpath\r\n", x1, y1, adw, adh); dest.Write(s, static_cast<UINT>(std::strlen(s)));
    ::StringCchPrintfA(s, n_size, "%%%%BeginDocument: %s\r\n", (LPCSTR)CStringA(eps_name)); dest.Write(s, static_cast<UINT>(std::strlen(s)));
    dest.Flush();

    if (theApp.isOpiMode) {
        ::StringCchPrintfA(s, n_size, "%s%s %s\r\n", OPI_TAG, "OG", CStringA(eps_name));
        dest.Write(s, static_cast<UINT>(std::strlen(s)));
    } else
        CDrawAdd::EmbedEpsFile(pArg, dest, eps_name);

    ::StringCchPrintfA(s, n_size, "%%%%EndDocument\r\nendManamEPS\r\n");
    dest.Write(s, static_cast<UINT>(std::strlen(s)));
    } CATCH(CFileException, e) {
        CDrawApp::SetErrorMessage(pArg->cBigBuf);
        ::MessageBox(pArg->pDlg->m_hWnd, CString("B��d przy przepisywaniu eps'a ") + eps_name + _T("\n ") + pArg->cBigBuf, _T("B��d"), MB_OK | MB_ICONERROR);
        return false;
    } END_CATCH

        return true;
} // RewriteEps

bool CDrawAdd::RewriteDrob(GENEPSARG* pArg, CFile& dest)
{
    auto s = reinterpret_cast<char*>(pArg->cBigBuf);
    auto pRoz = m_pDocument->GetCRozm(szpalt_x, szpalt_y, typ_xx);

    float x1{}, x2{}, y1{}, y2{};
    const auto x = (float)((posx - 1) * (pRoz->w + pRoz->sw) * mm2pkt);
    const auto y = (float)((szpalt_y + 1 - posy - sizey) * (pRoz->h + pRoz->sh) * mm2pkt);

    if (theApp.isOpiMode)
        nazwa.Format(_T("%s%s%03i.eps"), (LPCTSTR)m_pDocument->gazeta.Left(3), (LPCTSTR)m_pDocument->gazeta.Mid(4, 2), txtposx);

    pArg->pDlg->OglInfo(pArg->iChannelId, "Drobne: " + nazwa);

    bool bReadFromATEX = false;
    CString sURL;
    sURL.Format(_T("drw_xx=%i&kiedy=%s&nr_porz=%i"), m_pDocument->id_drw, (LPCTSTR)m_pDocument->dayws, txtposx);
    auto pFile = theApp.OpenURL(2, sURL);
    if (pFile) {
        try {
            bReadFromATEX = pFile->Read(s, 50) > 10 && sscanf_s(s, "%f %f %f %f", &x1, &y1, &x2, &y2) == 4;
        } catch (CInternetException* iex) {
            if (theApp.isOpiMode) {
                iex->GetErrorMessage(pArg->cBigBuf, DLGMSG_MAX_LEN);
                ::MessageBox(pArg->pDlg->m_hWnd, CString(_T("Nie uda�o si� pobra� rozmiaru drobnych: ")) + pArg->cBigBuf, APP_NAME, MB_OK | MB_ICONERROR);
            }
            iex->Delete();
        }

        pFile->Close();
    }

    if (!bReadFromATEX) {
        CFile eps;
        if (!eps.Open(nazwa, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite)) {
            CDrawApp::SetErrorMessage(pArg->cBigBuf);
            ::MessageBox(pArg->pDlg->m_hWnd, CString("Brakuje strony z Drobnixa: ") + nazwa + _T('\n') + pArg->cBigBuf, APP_NAME, MB_OK | MB_ICONERROR);
            return false;
        }
        eps.SeekToBegin();
        BBoxFromFile(pArg, eps, &x1, &y1, &x2, &y2);
        eps.Close();
    }

    ::StringCchPrintfA(s, n_size, "beginManamEPS \r\n");  dest.Write(s, static_cast<UINT>(std::strlen(s)));
    ::StringCchPrintfA(s, n_size, "%.2f %.2f translate\r\n", x, y);  dest.Write(s, static_cast<UINT>(std::strlen(s)));
    const float scaly = (y2 == y1) ? 1 : m_pDocument->GetDrobneH() / (y2 - y1);
    ::StringCchPrintfA(s, n_size, "1 %.4f scale\r\n", scaly);  dest.Write(s, static_cast<UINT>(std::strlen(s)));

    ::StringCchPrintfA(s, n_size, "%.2f %.2f translate\r\n", -x1, -y1);  dest.Write(s, static_cast<UINT>(std::strlen(s)));
    ::StringCchPrintfA(s, n_size, "%.2f %.2f %.2f %.2f clipEPS \r\nclip newpath\r\n", x1, y1, x2 - x1, y2 - y1);  dest.Write(s, static_cast<UINT>(std::strlen(s)));
    ::StringCchPrintfA(s, n_size, "%%%%BeginDocument: %s\r\n", (LPCSTR)CStringA(nazwa)); dest.Write(s, static_cast<UINT>(std::strlen(s)));
    dest.Flush();

    if (theApp.isOpiMode) {
        ::StringCchPrintfA(s, n_size, "%sDR %s\r\n", OPI_TAG, (LPCSTR)CStringA(nazwa));
        dest.Write(s, static_cast<UINT>(std::strlen(s)));
    } else 
        CDrawAdd::EmbedEpsFile(pArg, dest, nazwa);

    ::StringCchPrintfA(s, n_size, "%%%%EndDocument\r\nendManamEPS\r\n"); dest.Write(s, static_cast<UINT>(std::strlen(s)));

    return true;
} // RewriteDrob

bool CDrawAdd::EmbedEpsFile(GENEPSARG* pArg, CFile& dstFile, const CString& srcPath)
{
    CFile srcFile;
    if (!srcFile.Open(srcPath, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite)) {
        CDrawApp::SetErrorMessage(pArg->cBigBuf);
        ::MessageBox(pArg->pDlg->m_hWnd, _T("Brakuje pliku: ") + srcPath + _T("\n") + pArg->cBigBuf, APP_NAME, MB_OK | MB_ICONERROR);
        return false;
    }

    bool ret = false;
    HANDLE mapHandle = ::CreateFileMapping(srcFile.m_hFile, nullptr, PAGE_READONLY | SEC_RESERVE, 0, 0, nullptr);
    if (mapHandle != nullptr) {
        auto mapPtr = (const char*)::MapViewOfFile(mapHandle, FILE_MAP_READ, 0, 0, 0);
        if (mapPtr != nullptr) {
            unsigned long offset, nCount;
            if (CDrawAdd::LocatePreview(srcFile, &offset, &nCount)) {
                dstFile.Write(mapPtr + offset, nCount);
                ret = true;
            }
            ::UnmapViewOfFile(mapPtr);
        }
        ::CloseHandle(mapHandle);
    }

    srcFile.Close();
    return ret;
}

void CDrawAdd::Preview(GENEPSARG* pArg, const int x, const int y, const int scanlinesCount, const int bytesPerScanline) const
{
    auto target = (BYTE*)pArg->cBigBuf;
    auto pRoz = m_pDocument->GetCRozm(szpalt_x, szpalt_y, typ_xx);

    div_t x1 = div((int)((posx - 1)*(pRoz->w + pRoz->sw)) / 10 - x, 8);
    div_t x2 = div((int)(((posx - 1 + sizex)*(pRoz->w + pRoz->sw) - pRoz->sw) / 10) - x, 8);
    int i, y1 = (int)(((szpalt_y + 1 - posy - sizey)*(pRoz->h + pRoz->sh)) / 10) - y + 2;
    int j, y2 = (int)(((szpalt_y + 1 - posy)*(pRoz->h + pRoz->sh) - pRoz->sh) / 10) - y + 2;
    y1 = min(scanlinesCount - y1, scanlinesCount - 2);   // koniec
    y2 = max(scanlinesCount - y2, 0);   //poczatek

    auto p1 = (BYTE)((1 << (8 - x1.rem)) - 1);
    auto p2 = (BYTE)(255 ^ (BYTE)((1 << (8 - x2.rem)) - 1));
    auto m1 = (BYTE)(1 << (7 - x1.rem));
    auto m2 = (BYTE)(1 << (7 - x2.rem));
    auto c1 = (BYTE)170;
    auto c2 = (BYTE)85;
    BYTE ci = c1;

    for (i = 1; i < 3; ++i)
        *(target + (y1 + 1)*bytesPerScanline + x2.quot - i) = (BYTE)255; // podpis

    for (i = y2; i <= y1; ++i) {
        const PBYTE currentScanLine = target + i * bytesPerScanline;
        *(currentScanLine + x1.quot) |= (m1 | p1) & ci; // edges
        *(currentScanLine + x2.quot) |= (m2 | p2) & ci;
        for (j = x1.quot + 1; j < x2.quot; ++j) // shape
            *(currentScanLine + j) = ci;
        ci = (ci == c1) ? c2 : c1;
    }
} // Preview
