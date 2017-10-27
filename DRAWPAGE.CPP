
#include "stdafx.h"
#include "drawadd.h"
#include "drawdoc.h"
#include "drawpage.h"
#include "drawvw.h"
#include "genepsinfodlg.h"
#include "koldlg.h"
#include "mainfrm.h"
#include "manam.h"
#include "manpdf.h"

extern BOOL drawErrorBoxes;

IMPLEMENT_SERIAL(CDrawPage, CDrawObj, 0)

CDrawPage::CDrawPage(const CRect& position) noexcept :
    CDrawObj(position), id_str(-1), szpalt_x(pszpalt_x), szpalt_y(pszpalt_y),
    nr(c_normal), prn_mak_xx(0), wyd_xx(-1), m_typ_pary(0), space(), space_locked(), space_red(),
    niemakietuj(0), m_dervlvl(0), m_mutczas(1), m_drukarnie(0), m_deadline(CTime::GetCurrentTime()),
    m_ac_red(0), m_ac_fot(0), m_ac_kol(0)
{
    ASSERT_VALID(this);
    kolor = c_full;
}

CDrawPage::~CDrawPage()
{
    // ogloszenia spadaja poza makiete
    for (const auto& pAdd : m_adds) {
        auto px = pszpalt_x*m_pDocument->iPagesInRow + m_pDocument->iPagesInRow / 2 + pAdd->posx;
        const CRect rect(pmodulx*px, pmoduly*(-1 - pAdd->posy), pmodulx*(px + pAdd->sizex), pmoduly*(-1 - pAdd->posy - pAdd->sizey));
        pAdd->MoveTo(rect);
        pAdd->fizpage = pAdd->posx = pAdd->posy = 0;
        pAdd->UpdateInfo();
        pAdd->SetDirty();
        pAdd->Invalidate();
    }
}

void CDrawPage::Serialize(CArchive& ar)
{
    ASSERT_VALID(this);

    CDrawObj::Serialize(ar);
    if (ar.IsStoring()) {
        ar << (WORD)nr;
        ar << (WORD)szpalt_x;
        ar << (WORD)szpalt_y;
        ar << name;
        ar << caption;
        ar << caption_alt;
        ar << (BYTE)niemakietuj;
        space_locked.Serialize(ar);
        space_red.Serialize(ar);
        WORD ile_krat = (WORD)m_kraty_niebazowe.size();
        ar << ile_krat;
        for (WORD i = 0; i < ile_krat; ++i) {
            auto& krata = m_kraty_niebazowe[i];
            ar << krata.m_szpalt_x;
            ar << krata.m_szpalt_y;
            krata.m_space_locked.Serialize(ar);
            krata.m_space_red.Serialize(ar);
        }
        ar << (WORD)prn_mak_xx;
        ar << (WORD)m_dervlvl;
        ar << (WORD)m_mutczas;
        ar << (DWORD)m_drukarnie;
        ar << m_deadline;
        ar << m_dervinfo;
        ar << (UINT)wyd_xx;
        ar << m_ac_red;
        ar << m_ac_fot;
        ar << m_ac_kol;
        ar << m_typ_pary;
    } else {
        WORD wTemp;
        ar >> wTemp; nr = wTemp;
        ar >> wTemp; szpalt_x = wTemp;
        ar >> wTemp; szpalt_y = wTemp;
        ar >> name;
        ar >> caption;
        ar >> caption_alt;
        BYTE bTemp;
        ar >> bTemp; niemakietuj = bTemp;
        space_locked.Serialize(ar);
        space_red.Serialize(ar);
        space = space_locked | space_red;
        ar >> wTemp;
        for (WORD i = 0; i < wTemp; ++i) {
            int sx, sy;
            CFlag sp, sp_l, sp_r;
            ar >> sx;
            ar >> sy;
            sp_l.Serialize(ar);
            sp_r.Serialize(ar);
            sp = sp_l | sp_r;
            m_kraty_niebazowe.emplace_back(sx, sy, std::move(sp), std::move(sp_l), std::move(sp_r));
        }
        ar >> wTemp; prn_mak_xx = wTemp;
        ar >> wTemp; m_dervlvl = wTemp;
        ar >> wTemp; m_mutczas = wTemp;
        DWORD dwTemp;
        ar >> dwTemp; m_drukarnie = dwTemp;
        ar >> m_deadline;
        ar >> m_dervinfo;
        ar >> wyd_xx;
        ar >> m_ac_red;
        ar >> m_ac_fot;
        ar >> m_ac_kol;
        ar >> m_typ_pary;
        id_str = -1;
        dirty = TRUE;
    }
}

inline CString CDrawPage::GenerateGUIDString()
{
    const int MAX_CHAR_IN_GUID = 50;
    wchar_t buf[MAX_CHAR_IN_GUID];
    GUID Guid = { 0 };

    ::CoCreateGuid(&Guid);
    StringFromGUID2(Guid, buf, MAX_CHAR_IN_GUID);
    return CString(buf);
}

CString CDrawPage::GetNrPaginy() const
{
    CString pagina;
    if ((nr & c_rzym) == c_rzym)
        pagina = CDrawObj::Rzymska(nr >> 3);
    else
        pagina.Format(_T("%i"), nr >> 3);

    return pagina;
}

void CDrawPage::Draw(CDC *pDC)
{
    ASSERT_VALID(this);

    CBrush* pOldBrush;
    if ((nr & c_rzym) == c_rzym) {
        pOldBrush = pDC->SelectObject(&(((CMainFrame*)AfxGetMainWnd())->rzym));
        pDC->SetBkColor(RGB(135, 135, 135));
    } else {
        pOldBrush = reinterpret_cast<CBrush*>(pDC->SelectStockObject(WHITE_BRUSH));
        pDC->SetBkColor(BIALY);
    }

    CPen *pOldPen = pDC->SelectObject(&(((CMainFrame*)AfxGetMainWnd())->pen));
    CFont *pOldFont = pDC->SelectObject(&(m_pDocument->m_pagefont));

    CRect rect = m_position;
    pDC->Rectangle(rect);
    DrawGrid(pDC);
    DrawKolor(pDC, &m_position);
    DrawReserved(pDC);

    if (drawErrorBoxes && niemakietuj) {
        pDC->MoveTo(rect.left, rect.top + 4 * vscale);
        pDC->LineTo(rect.right, rect.top + 4 * vscale);
    }

    const int nr_porz = nr >> 3;
    const bool isRoman = (nr & c_rzym) > 0;
    const CString& cnr = GetNrPaginy();
    const CString& cap = theApp.swCZV & 2 ? ((CMainFrame*)AfxGetMainWnd())->GetCapStrFromData((WORD)(prn_mak_xx - (nr_porz & 1))) : caption;
    CRect r(rect.left + 2 * vscale, rect.top - vscale, rect.right - 2 * vscale, rect.top - pmoduly + vscale);
    if ((!isRoman && (nr_porz & 1) == 0) || (isRoman && (nr_porz & 1) == 1)) { // lewe arabskie i prawe rzymskie
        DrawNapis(pDC, cap, cap.GetLength(), r, DT_RIGHT | DT_VCENTER | DT_SINGLELINE, OPAQUE);
        DrawNapis(pDC, cnr, cnr.GetLength(), r, DT_LEFT | DT_VCENTER | DT_SINGLELINE, OPAQUE);
    } else { // prawe arabskie i lewe rzymskie
        DrawNapis(pDC, cap, cap.GetLength(), r, DT_LEFT | DT_VCENTER | DT_SINGLELINE, OPAQUE);
        DrawNapis(pDC, cnr, cnr.GetLength(), r, DT_RIGHT | DT_VCENTER | DT_SINGLELINE, OPAQUE);
    }

    if (m_pDocument->isGRB) {
        pDC->SelectObject(&m_pDocument->m_addfont);
        DrawNapis(pDC, m_dervinfo, m_dervinfo.GetLength(), CRect(rect.TopLeft() + CSize(0, rect.Height() - pmoduly + 3 * vscale), CSize(pwidth, pmoduly)), DT_CENTER | DT_VCENTER | DT_SINGLELINE, OPAQUE);
    }
    pDC->SelectObject(pOldBrush);
    pDC->SelectObject(pOldPen);
    pDC->SelectObject(pOldFont);
}

void CDrawPage::DrawDeadline(CDC *pDC, const CRect& pos) const
{
    CFont *oldFont = pDC->SelectObject(&(m_pDocument->m_pagefont));
    CRect dlBox(CPoint((int)(pos.left + 1.25*pmodulx), pos.top - 2 * pmoduly), CSize((int)(2.5*pmodulx), -pmoduly - 2));
    pDC->Rectangle(dlBox);
    CString t = this->m_deadline.Format(_T("%H:%M"));
    DrawNapis(pDC, t, 5, dlBox, DT_CENTER | DT_BOTTOM | DT_SINGLELINE | DT_NOCLIP, TRANSPARENT);
    pDC->SelectObject(oldFont);
}

void CDrawPage::DrawAcDeadline(CDC *pDC, const CRect& pos) const
{
    if (m_ac_red == 0L) return;

    CString sDeadlines;
    sDeadlines.Format(_T("R: %02i/%02i\r\nF: %02i/%02i\r\nK: %02i/%02i"), m_ac_red.GetDay(), m_ac_red.GetMonth(), m_ac_fot.GetDay(), m_ac_fot.GetMonth(), m_ac_kol.GetDay(), m_ac_kol.GetMonth());

    CFont *oldFont = pDC->SelectObject(&(m_pDocument->m_pagefont));
    CRect dlBox(CPoint((int)(pos.left + 0.8*pmodulx), (int)(pos.top - 1.8*pmoduly)), CSize((int)(3.5*pmodulx - 10), (int)-3.5*pmoduly));
    pDC->Rectangle(dlBox);
    DrawNapis(pDC, sDeadlines, sDeadlines.GetLength(), dlBox, DT_CENTER | DT_VCENTER | DT_NOCLIP, TRANSPARENT);
    pDC->SelectObject(oldFont);
}

CFlag CDrawPage::GetReservedFlag()
{
    if (space.IsZero())
        return space;

    CFlag space_add(space.GetSize());
    for (const auto& pAdd : m_adds)
        if (szpalt_x == pAdd->szpalt_x && szpalt_y == pAdd->szpalt_y)
            space_add |= pAdd->GetPlacementFlag();

    space_add |= space_locked;
    space_add |= space_red;
    return space ^ space_add;
}

void CDrawPage::DrawReserved(CDC *pDC)
{

    /*	vu : zaznacza moduly za statusem zajete OZ, na ktorych jeszcze
             nie stoja ogloszenia. Wizualizuje w ten sposob miejsce z
             ogloszeniami dziedziczonymi	end vu */
    const CFlag& vspace = GetReservedFlag();
    if (vspace.IsSet()) {
        pDC->SelectStockObject(((nr & c_rzym) == c_rzym ? LTGRAY_BRUSH : GRAY_BRUSH));
        auto ilemod = static_cast<size_t>(szpalt_x * szpalt_y);
        for (size_t module = 0; module < ilemod; ++module)
            if (vspace.GetBit(module) > 0)
                pDC->Rectangle(GetNormalizedModuleRect(module));
        pDC->SelectStockObject(WHITE_BRUSH);
    }
}

void CDrawPage::DrawGrid(CDC *pDC)
{
    if (((CMainFrame*)AfxGetMainWnd())->show_spacelocks && (space_red || space_locked)) {
        auto pOldBrush = reinterpret_cast<CBrush*>(pDC->SelectStockObject(WHITE_BRUSH));
        auto pOldPen = reinterpret_cast<CPen*>(pDC->SelectStockObject(NULL_PEN));
        auto ilemod = static_cast<size_t>(szpalt_x * szpalt_y);
        for (size_t module = 0; module < ilemod; ++module) {
            CRect& rect = GetNormalizedModuleRect(module);
            if (space_locked.GetBit(module) > 0) {
                pDC->SelectStockObject(DKGRAY_BRUSH);
                pDC->Rectangle(rect);
            } else if (space_red.GetBit(module) > 0) {
                if ((nr & c_rzym) == c_rzym)
                    pDC->SelectObject(&(((CMainFrame*)AfxGetMainWnd())->rzym));
                else
                    pDC->SelectStockObject(WHITE_BRUSH);

                if (abs(rect.Height()) < 5 * vscale) {
                    pDC->SelectStockObject(BLACK_PEN);
                    pDC->MoveTo(rect.left, rect.top);
                    pDC->LineTo(rect.right, rect.bottom);
                    pDC->SelectStockObject(NULL_PEN);
                } else {
                    auto tmp = rect.bottom; rect.bottom = rect.top; rect.top = tmp;
                    DrawNapis(pDC, _T("R"), 1, rect, DT_CENTER | DT_VCENTER, TRANSPARENT);
                }
            }
        }
        pDC->SelectObject(pOldPen);
        pDC->SelectObject(pOldBrush);
    }

    if (m_dervlvl != DERV_NONE) {
        COLORREF bkColor = 0, oldBkColor = pDC->GetBkColor();
        switch (m_dervlvl) {
            case DERV_ADDS: bkColor = 0xFF64BC; break; // fioletowy
            case DERV_TMPL: bkColor = 0x43C1FB; break; // pomaranczowy
            case DERV_FIXD: bkColor = 0x7B78F1; break; // czerwony
            case DERV_PROH: bkColor = 0x6EC27C; break; // zielony
            case DERV_COLO: bkColor = 0x111111; break; // czarny
            case DERV_DRUK: bkColor = 0xCAA71C;		   // granatowy
        }
        if (m_dervlvl == DERV_PROH)
            pDC->FillSolidRect(m_position.left, m_position.top, m_position.Width(), m_position.Height() - 3 * vscale, bkColor);
        else
            pDC->FillSolidRect(m_position.left, m_position.bottom, m_position.Width(), -3 * vscale, bkColor);
        pDC->SetBkColor(oldBkColor);
    }

    int k;
    for (k = 1; k <= szpalt_y; k++) {
        pDC->MoveTo(m_position.left, m_position.bottom + (int)(k*mody(szpalt_y)));
        pDC->LineTo(m_position.right, m_position.bottom + (int)(k*mody(szpalt_y)));
    }
    for (k = 1; k < szpalt_x; k++) {
        pDC->MoveTo(m_position.left + (int)(k*modx(szpalt_x)), m_position.bottom);
        pDC->LineTo(m_position.left + (int)(k*modx(szpalt_x)), m_position.bottom + (int)(mody(szpalt_y)*szpalt_y));
    }
}

CRect CDrawPage::GetNormalizedModuleRect(size_t module) const
{
#ifdef DEBUG
    ASSERT(0 <= module && module < (size_t)szpalt_x * szpalt_y);
#endif
    const auto tmp = div((int)module, szpalt_x);
    const auto posx = szpalt_x - tmp.rem;
    const auto posy = szpalt_y - tmp.quot;
    return CRect(m_position.left + (int)(modulx*(posx - 1)), m_position.bottom + (int)(moduly*(szpalt_y - posy)), m_position.left + (int)(modulx*posx), m_position.bottom + (int)(moduly*(szpalt_y - posy + 1)));
}

void CDrawPage::SetSpotKolor(UINT spot_kolor) // spot kolor to index spotkoloru w tablicy Spot_Kolor
{
    if ((kolor & c_spot) == c_spot) {
        dirty = TRUE;
        m_pDocument->SetSpotKolor((kolor >> 3), spot_kolor);
    }
}

void CDrawPage::DrawKolor(CDC *pDC, const CRect& pos) const
{
    if (kolor == c_brak)
        return;

    if (kolor == c_full) {
        const auto r1 = CRect(pos.left, pos.top + 3 * vscale, pos.left + (int)floor(szpalt_x*modulx / 3), pos.top);
        const auto r2 = CRect(pos.left + (int)floor(szpalt_x*modulx / 3), pos.top + 3 * vscale, pos.right - (int)floor(szpalt_x*modulx / 3), pos.top);
        const auto r3 = CRect(pos.right - (int)floor(szpalt_x*modulx / 3), pos.top + 3 * vscale, pos.right, pos.top);
        pDC->FillRect(r1, &((CMainFrame*)AfxGetMainWnd())->cyjan);
        pDC->FillRect(r2, &((CMainFrame*)AfxGetMainWnd())->magenta);
        pDC->FillRect(r3, &((CMainFrame*)AfxGetMainWnd())->yellow);
    } else {
        auto r1 = CRect(pos.left, pos.top + 3 * vscale, pos.right, pos.top);
        CBrush* pOldBrush;
        CPen* pOldPen = reinterpret_cast<CPen*>(pDC->SelectStockObject(NULL_PEN));
        if (m_pDocument->isGRB) pOldBrush = reinterpret_cast<CBrush*>(pDC->SelectStockObject(LTGRAY_BRUSH));
        else pOldBrush = pDC->SelectObject(((CMainFrame*)AfxGetMainWnd())->GetSpotBrush(m_pDocument->m_spot_makiety[kolor >> 3]));
        COLORREF bk = pDC->SetBkColor(BIALY);
        pDC->Rectangle(r1);
        DrawNapis(pDC, Rzymska(((kolor >> 3) + 1)), Rzymska(((kolor >> 3) + 1)).GetLength(), r1, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP, OPAQUE);
        pDC->SetBkColor(bk);
        pDC->SelectObject(pOldBrush);
        pDC->SelectObject(pOldPen);
    }
}

void CDrawPage::ChangeCaption(BOOL iscaption, const CString& cap)
{
    if (iscaption)
        caption = cap;
    else {
        name = cap;
        caption = cap.Right(cap.GetLength() - cap.ReverseFind(_T('/')) - 1);
    }
    SetDirty();
    UpdateInfo();
    Invalidate();
}

void CDrawPage::DBChangeName(int id_drw)
{
    if (caption.IsEmpty()) return;

    CManODPNETParms orapar {
        { CManODPNET::DbTypeVarchar2, CManODPNET::ReturnValue, &name },
        { CManODPNET::DbTypeInt32, &id_drw },
        { CManODPNET::DbTypeVarchar2, &caption }
    };
    if (!theManODPNET.EI("select sciezka from strukt_drzewa where drw_xx=:drw_xx and str_log=:capt", orapar)) {
        caption.Empty(); name.Empty();
    }

    SetDirty();  UpdateInfo();  Invalidate();
}

/////////////// PRINT
void CDrawPage::Print(CDC *pDC)
{
    ASSERT_VALID(this);

    CBrush* pOldBrush;
    if (((nr & c_rzym) == c_rzym))
        pOldBrush = pDC->SelectObject(&(((CMainFrame*)AfxGetMainWnd())->rzym));
    else
        pOldBrush = (CBrush*)pDC->SelectStockObject(WHITE_BRUSH);

    auto pOldPen = reinterpret_cast<CPen*>(pDC->SelectStockObject(BLACK_PEN));
    auto pOldFont = pDC->SelectObject(&(m_pDocument->m_pagefont));

    pDC->SetBkMode(TRANSPARENT);

    const CRect saved_position = m_position;
    m_position = GetPrintRect();

    pDC->Rectangle(m_position);
    DrawGrid(pDC);
    DrawKolor(pDC, &m_position);
    DrawReserved(pDC);

    const int nr_porz = nr >> 3;
    const bool isRoman = (nr & c_rzym) > 0;
    const CString& cnr = GetNrPaginy();
    CRect r(m_position.left + 2 * vscale, m_position.top - vscale, m_position.right - 2 * vscale, m_position.top - pmoduly + vscale);
    if ((!isRoman && (nr_porz & 1) == 0) || (isRoman && (nr_porz & 1) == 1)) { // lewe arabskie i prawe rzymskie
        DrawNapis(pDC, caption, caption.GetLength(), r, DT_NOCLIP | DT_RIGHT | DT_VCENTER | DT_SINGLELINE, OPAQUE);
        DrawNapis(pDC, cnr, cnr.GetLength(), r, DT_LEFT | DT_VCENTER | DT_SINGLELINE, OPAQUE);
    } else { // prawe arabskie i lewe rzymskie
        DrawNapis(pDC, caption, caption.GetLength(), r, DT_LEFT | DT_VCENTER | DT_SINGLELINE, OPAQUE);
        DrawNapis(pDC, cnr, cnr.GetLength(), r, DT_NOCLIP | DT_RIGHT | DT_VCENTER | DT_SINGLELINE, OPAQUE);
    }

    if (m_pDocument->isGRB && pDC->GetViewportExt().cx < 1000) {
        pDC->SelectObject(&m_pDocument->m_addfont);
        DrawNapis(pDC, m_dervinfo, m_dervinfo.GetLength(), CRect(m_position.TopLeft() + CSize(0, m_position.Height() - pmoduly + 3 * vscale), CSize(pwidth, pmoduly)), DT_CENTER | DT_VCENTER | DT_SINGLELINE, OPAQUE);
    }
    pDC->SelectObject(pOldBrush);
    pDC->SelectObject(pOldPen);
    pDC->SelectObject(pOldFont);
    m_position = saved_position;
}

// rect must be in logical coordinates
CDrawObj* CDrawPage::Clone(CDrawDoc *pDoc) const
{
    ASSERT_VALID(this);

    auto pClone = new CDrawPage(m_position);
    pClone->name = name;
    pClone->caption = caption;
    pClone->nr = nr;
    pClone->kolor = kolor;
    pClone->wyd_xx = wyd_xx;
    pClone->space_locked = CFlag(space_locked.GetSize());
    pClone->space_red = CFlag(space_red.GetSize());
    pClone->space = CFlag(space.GetSize());
    pClone->niemakietuj = niemakietuj;
    pClone->szpalt_x = szpalt_x;
    pClone->szpalt_y = szpalt_y;
    pClone->m_deadline = m_deadline;

    if (pDoc != nullptr) {
        if (pDoc->m_pages.size() > 1)
            pClone->m_drukarnie = (pDoc->m_pages[1])->m_drukarnie;
        pDoc->AddPage(pClone);
    }

    ASSERT_VALID(pClone);
    return pClone;
}

BOOL CDrawPage::OnOpen(CDrawView*)
{
    ASSERT_VALID(this);

    if (m_pDocument->docmutred.IsEmpty() && m_pDocument->gazeta.GetLength() > 5) {
        CString mr = m_pDocument->gazeta.Left(1) == _T("Z") ? _T("Z1") : _T("RP");
        CManODPNETParms orapar {
            { CManODPNET::DbTypeVarchar2, CManODPNET::ReturnValue, &m_pDocument->docmutred },
            { CManODPNET::DbTypeVarchar2, &mr }
        };
        theManODPNET.EI("select grb.get_mutred(:rootmut) from dual", orapar);
    }

    CPageDlg dlg;
    dlg.m_mak_xx = m_pDocument->m_mak_xx;
    dlg.m_id_str = id_str;
    dlg.m_ile_spotow = (UINT)m_pDocument->m_spot_makiety.size();
    dlg.m_kolor = kolor;
    dlg.m_name = name;
    dlg.m_caption = caption;
    dlg.m_iscaption = m_pDocument->isRED;
    dlg.m_nr = (nr >> 3);
    dlg.m_rzymska = ((nr & c_rzym) == c_rzym);
    dlg.m_szpalt_x = szpalt_x;
    dlg.m_szpalt_y = szpalt_y;
    dlg.m_niemakietuj = niemakietuj;
    dlg.m_deadline = m_deadline;
    dlg.m_dervinfo = m_dervinfo;
    dlg.m_drukarnie = m_drukarnie;
    dlg.m_dervlvl = m_dervlvl;
    dlg.m_mutred = mutred;
    dlg.m_docmutred = m_pDocument->docmutred;
    dlg.m_typ_pary = m_typ_pary;
    dlg.m_wyd_xx = wyd_xx;
    dlg.m_red = m_ac_red;
    dlg.m_fot = m_ac_fot;
    dlg.m_kol = m_ac_kol;

    if (dlg.DoModal() != IDOK)
        return FALSE;

    name = dlg.m_name;
    caption = dlg.m_caption;
    kolor = dlg.m_kolor;
    mutred = dlg.m_mutred;
    wyd_xx = dlg.m_wyd_xx;
    m_ac_red = dlg.m_red;
    m_ac_fot = dlg.m_fot;
    m_ac_kol = dlg.m_kol;
    if (m_drukarnie != dlg.m_drukarnie) { // zmiana dla czworki stron drukowanych na jednej kartce
        m_drukarnie = dlg.m_drukarnie;
        int obj = (int)m_pDocument->m_pages.size();
        int nr_porz = m_pDocument->GetIPage(this);
        auto pPage = m_pDocument->m_pages[(obj - nr_porz + 1) % obj];
        pPage->m_drukarnie = m_drukarnie;
        pPage->SetDirty();
        nr_porz += 2 * (nr_porz & 1) - 1;
        if (nr_porz < 0) nr_porz += obj;
        pPage = m_pDocument->m_pages[nr_porz % obj];
        pPage->m_drukarnie = m_drukarnie;
        pPage->SetDirty();
        pPage = m_pDocument->m_pages[(obj - nr_porz + 1) % obj];
        if (pPage->m_dervlvl != DERV_FIXD) pPage->m_drukarnie = m_drukarnie;
        pPage->SetDirty();
    }

    if (m_typ_pary != dlg.m_typ_pary) {
        m_typ_pary = dlg.m_typ_pary;
        int nr_porz = m_pDocument->GetIPage(this);
        auto pPage = m_pDocument->m_pages[nr_porz + 1 - 2 * (nr_porz & 1)];
        pPage->m_typ_pary = m_typ_pary;
        pPage->SetDirty();
    }

    SetBaseKrata(dlg.m_szpalt_x, dlg.m_szpalt_y);

    int i, pom_nr = ((dlg.m_nr << 3) + ((dlg.m_rzymska) ? c_rzym : c_normal));
    int pc = (int)m_pDocument->m_pages.size();
    if (nr != pom_nr) {
        // sprawdz czy istnieje juz taka strona - bo numer/typ_num nie moze sie powtarzac
        for (i = 0; i < pc; i++)
            if ((m_pDocument->m_pages[i])->nr == pom_nr) break;
        if (i < pc)
            AfxMessageBox(_T("Strona o podanym numerze i typie numeracji ju¿ istnieje. Numer nie zostanie zmieniony."), MB_ICONEXCLAMATION);
        else {
            nr = pom_nr;
            for (const auto& pAdd : m_adds) {
                pAdd->fizpage = nr;
                pAdd->UpdateInfo();
            }
        }
    }
    niemakietuj = dlg.m_niemakietuj;
    m_deadline = dlg.m_deadline;
    m_dervinfo = dlg.m_dervinfo;

    if (theApp.swCZV == 2)
        prn_mak_xx = dlg.m_prn_mak_xx;
    pom_nr = m_pDocument->GetIPage(this);
    i = dlg.m_topage;
    while (--i) {
        auto vPage = m_pDocument->m_pages[div(i + pom_nr, pc).rem];
        if (vPage->m_dervlvl != DERV_FIXD && vPage->m_dervlvl != DERV_PROH) {
            if (theApp.swCZV == 2)
                vPage->prn_mak_xx = dlg.m_prn_mak_xx + ((i + pom_nr) & 1) - (pom_nr & 1);
            else {
                vPage->wyd_xx = dlg.m_wyd_xx;
                vPage->SetDirty();
            }
            vPage->Invalidate();
        }
    }
    SetDirty(); UpdateInfo(); Invalidate();
    return TRUE;
}

std::vector<int> CDrawPage::CleanKraty(BOOL dbSave)
{	// lista krat w postaci hiword == szpalt_x, loword == szpalt_y
    std::vector<int> ret;
    auto& adds = m_adds;
    auto end = std::end(m_kraty_niebazowe);
    auto new_end = std::remove_if(begin(m_kraty_niebazowe), end,
        [&adds](const CKrataNiebazowa& kn) {
        for (const auto& pAdd : adds)
            if (pAdd->szpalt_x == kn.m_szpalt_x && pAdd->szpalt_y == kn.m_szpalt_y)
                return false;
        return true;
    });
    if (dbSave)
        for (auto it = new_end; it != end; ++it)
            ret.push_back(((*it).m_szpalt_x << 16) + (*it).m_szpalt_y);
    m_kraty_niebazowe.erase(new_end, end);

    return ret;
}

void CDrawPage::SetNr(int i)
{
    nr = i;
    SetDirty();
    UpdateInfo();

    for (const auto& pAdd : m_adds) {
        pAdd->fizpage = i;
        pAdd->UpdateInfo();
    }
}

void CDrawPage::UpdateInfo()
{
    info.Format(_T("Strona %s %s"), GetNrPaginy(), caption);
    if (!name.IsEmpty()) info.AppendFormat(_T(" - %s"), name);
    if (!m_pDocument->isGRB && (kolor & c_spot) > 0)
        info.AppendFormat(_T(" | %s"), ((CMainFrame*)AfxGetMainWnd())->Spot_Kolor[m_pDocument->m_spot_makiety[(kolor >> 3)]]); //w SPOT_kOlorach 0-brak, 1==full
    info.AppendFormat(_T(" | %s"), m_deadline.Format(c_ctimeCzas));
    if (!m_dervinfo.IsEmpty())
        info.AppendFormat(_T(" | %s"), m_dervinfo);
    if (!this->mutred.IsEmpty()) {
        CString mr;
        int ilePokazac = 30, mutredLen = mutred.GetLength();
        if (ilePokazac > mutredLen) ilePokazac = mutredLen;
        for (int i = 0; i < ilePokazac; i += 2)
            mr.AppendFormat(_T(", %s"), mutred.Mid(i, 2));
        info.AppendFormat(_T(" | redakcyjna: %s"), mr.Mid(2));
        if (mutredLen > 30) info.Append(_T("..."));
    }
}

//////////////////////////////////////////////////////
///////// obsluga listy ogloszen ///////////////// myadds    i blokad miejsca

void CDrawPage::AddAdd(CDrawAdd *pAdd)
{
    ASSERT(pAdd != nullptr);
    pAdd->fizpage = this->nr;
    pAdd->SetDirty();

    SetBaseKrata(pAdd->szpalt_x, pAdd->szpalt_y, FALSE);
    m_adds.push_back(pAdd);
    SetSpace(pAdd);
    SetDirty();
}

void CDrawPage::RemoveAdd(CDrawAdd *pAdd, BOOL bRemodeFromAdds)
{
    auto itAdd = std::find(m_adds.cbegin(), m_adds.cend(), pAdd);
    if (itAdd != m_adds.end()) {
        SetBaseKrata((*itAdd)->szpalt_x, (*itAdd)->szpalt_y);
        RealizeSpace(*itAdd);
        if (bRemodeFromAdds)
            m_adds.erase(itAdd);
    } else { // gdy dwie strony maja ten sam numer - to nie ma prawa sie zdarzyc
        for (const auto& p : m_pDocument->m_pages)
            if (p != this && p->nr == this->nr) {
                CString sMsg;
                sMsg.Format(_T("W makiecie powtarza siê numer strony %i. Proszê poprawiæ"), this->nr >> 3);
                MessageBox(NULL, sMsg, _T("Makieta uszkodzna"), MB_ICONSTOP);
            }
    }
    pAdd->SetDirty();
    SetDirty();
}

void CDrawPage::MoveTo(const CRect& position, CDrawView *pView)
{
    CDrawObj::MoveTo(position, pView);
    for (const auto& pAdd : m_adds)
        pAdd->MoveWithPage(position, pView);
    dirty = TRUE;
}

void CDrawPage::SetSpace(const CDrawAdd *pObj)
{
    space |= pObj->GetPlacementFlag();
    dirty = TRUE;
}

void CDrawPage::RealizeSpace(const CDrawAdd *pObj)
{
    space ^= pObj->GetPlacementFlag();
    dirty = TRUE;
}

BOOL CDrawPage::FindSpace(CDrawAdd *pObj, int *px, int *py, int sx, int sy) const
{
    BOOL ret = FALSE;
    if (m_dervlvl == DERV_FIXD || m_dervlvl == DERV_PROH) return FALSE;

    if (*px <= 0) *px = 1;
    if (*py <= 0) *py = 1;

    // kontekst wyolania
    bool interPage = pObj->fizpage == nr;
    bool symulateNewShape = pObj->sizex != sx || pObj->sizey != sy;

    // zwolnij miejsce zajmowane poprzednio jezeli przesuniecie w obrebie strony
    // const CFlag& sp = (interPage) ? space ^ pObj->GetPlacementFlag() : space; kompiluje sie fatalnie
    CFlag sp(space);
    if (interPage)
        sp ^= pObj->GetPlacementFlag();

    // przy zmianie rozmiaru sprawdzac na docelowych wymiarach
    int sx_org, sy_org;
    if (symulateNewShape) {
        sx_org = pObj->sizex;
        sy_org = pObj->sizey;
        pObj->SetSpaceSize(sx, sy);
    }

    // sprawdz moduly na kracie bazowej
    if (!(sp & pObj->GetPlacementFlag(*px, *py)) && CheckSpaceDiffKraty(pObj, *px, *py, sx, sy)) {
        ret = TRUE;
        goto restoreShape;
    }
    if (interPage) // pozostan na miejscu
        goto restoreShape;

    // szukaj innego miejsca na stronie docelowej
    for (int y = (szpalt_y + 1 - sy); y > 0; y--)
        for (int x = (szpalt_x + 1 - sx); x > 0; x--)
            if (!(sp & pObj->GetPlacementFlag(x, y)) && CheckSpaceDiffKraty(pObj, x, y, sx, sy)) {
                *px = x; *py = y;
                ret = TRUE;
                goto restoreShape;
            }

    *px = *py = 0;

restoreShape:
    // przywroc rzeczywisty ksztalt
    if (symulateNewShape)
        pObj->SetSpaceSize(sx_org, sy_org);

    return ret;
}

BOOL CDrawPage::CheckSpace(const CDrawAdd *pObj, int px, int py) const
{
    /* vu : Sprawdza czy dane ogloszenie mozna postawic we wspolrzednych px i py na stronie		end vu */
    if (px < 1 || px + pObj->sizex - 1 > szpalt_x || py < 1 || py + pObj->sizey - 1 > szpalt_y) return FALSE;

    // zwolnij miejsce zajmowane poprzednio jezeli przesuniecie w obrebie strony
    CFlag sp(space);
    if (nr == pObj->fizpage)
        sp ^= pObj->GetPlacementFlag();

    return (sp & pObj->GetPlacementFlag(px, py)).IsZero();
}

BOOL CDrawPage::CheckSpaceDiffKraty(const CDrawAdd *pObj, int x, int y, int sx, int sy) const
{
    if (m_kraty_niebazowe.empty()) return TRUE;
    // sprawdz przecinanie z innymi kratami
    CRect dstRect(m_position.left + (int)(modulx*(x - 1)), m_position.bottom + (int)(moduly*(szpalt_y - y - sy + 1)), m_position.left + (int)(modulx*(x + sx - 1)), m_position.bottom + (int)(moduly*(szpalt_y - y + 1)));
    for (const auto& pAdd : m_adds)
        if ((pAdd->szpalt_x != pObj->szpalt_x || pAdd->szpalt_y != pObj->szpalt_y) && pAdd->precelWertexCnt == 0 && pAdd->Intersects(dstRect))
            return FALSE;

    // dla wielokratowych stron dziedziczonych
    if (m_dervlvl != DERV_NONE) {
        for (const auto& kn : m_kraty_niebazowe) {
            int s_x = kn.m_szpalt_x, s_y = kn.m_szpalt_y;
            auto sp = &kn.m_space;
            CFlag mask(sp->GetSize());
            mask |= 1;
            for (int k = s_y; k > 0; k--)
                for (int l = s_x; l > 0; l--, mask <<= 1)
                    if ((*sp & mask).IsSet()) {
                        CRect inter, dst(m_position.left + (int)(CDrawObj::modx(s_x)*(l - 1)), m_position.bottom + (int)(CDrawObj::mody(s_y)*(s_y - k)),
                            /* normalized */m_position.left + (int)(CDrawObj::modx(s_x)*l), m_position.bottom + (int)(CDrawObj::mody(s_y)*(s_y - k + 1)));
                        if (inter.IntersectRect(dstRect, dst))
                            return FALSE;
                    }
        }
    }

    return TRUE;
}

void CDrawPage::SetBaseKrata(int s_x, int s_y, BOOL refresh)
{
    /*  poszukaj czy taka krata jest juz zdefiniowana na tej stronie.
        jezli nie to zmien krate bazowa. jeeli incremental to dopisz */
    if (szpalt_x == s_x && szpalt_y == s_y) return;

    CKrataNiebazowa* cached = nullptr;
    for (auto& kn : m_kraty_niebazowe)
        if (kn.m_szpalt_x == s_x &&  kn.m_szpalt_y == s_y) {
            cached = &kn; break;
        };

    if (cached != nullptr) {
        cached->m_szpalt_x = szpalt_x;
        cached->m_szpalt_y = szpalt_y;
        std::swap(cached->m_space, space);
        std::swap(cached->m_space_locked, space_locked);
        std::swap(cached->m_space_red, space_red);
    } else {
        m_kraty_niebazowe.emplace_back(szpalt_x, szpalt_y, std::move(space), std::move(space_locked), std::move(space_red));
        space = space_locked = space_red = CFlag(0, 0, s_x, s_y);
        if (refresh) { // ustaw moduly na nowej kracie, przeniesienie info o red i lock
            auto& kn = m_kraty_niebazowe.back();
            CFlag& lastred = kn.m_space_red;
            CFlag& lastlocked = kn.m_space_locked;
            auto ilemod = static_cast<size_t>(szpalt_x * szpalt_y);
            for (size_t module = 0; module < ilemod; ++module) {
                bool isRed = lastred.GetBit(module) > 0;
                bool isLock = lastlocked.GetBit(module) > 0;
                if (isRed || isLock) {
                    CRect intsec, dst;
                    const CRect& src = GetNormalizedModuleRect(module);
                    for (int k = 1; k <= s_x; k++)
                        for (int l = 1; l <= s_y; l++) {
                            dst.SetRect(m_position.left + (int)(CDrawObj::modx(s_x)*(k - 1)), m_position.bottom + (int)(CDrawObj::mody(s_y)*(s_y - l)),
                                m_position.left + (int)(CDrawObj::modx(s_x)*k), m_position.bottom + (int)(CDrawObj::mody(s_y)*(s_y - l + 1))); // normalized
                            if (intsec.IntersectRect(src, dst)) {
                                int outerBit = (s_y - l)*s_x + s_x - k;
                                if (space.GetBit(outerBit) > 0) continue;
                                space.SetBit(outerBit);
                                if (isRed) space_red.SetBit(outerBit);
                                else space_locked.SetBit(outerBit);
                            }
                        }
                }
            }
        }
    }

    szpalt_x = s_x;
    szpalt_y = s_y;

    if (refresh) { SetDirty(); Invalidate(); }
}

void CDrawPage::ChangeMark(size_t module, SpaceMode mode)
{
    if (m_dervlvl == DERV_FIXD) return;

    CFlag& space_mark = mode == SpaceMode::spacelock ? space_locked : space_red;
    int bitToSet = space_mark.GetBit(module) > 0 ? 0 : 1;
    if (bitToSet > 0 && space.GetBit(module) > 0) // stoi ogloszenie lub inny marker
        return;

    if (!m_kraty_niebazowe.empty()) {
        CRect intsec;
        const CRect& src = GetNormalizedModuleRect(module);
        int s_x = szpalt_x, s_y = szpalt_y;
        for (const auto& kn : m_kraty_niebazowe) {
            SetBaseKrata(kn.m_szpalt_x, kn.m_szpalt_y, FALSE);
            CFlag& space_mark2 = mode == SpaceMode::spacelock ? space_locked : space_red;

            auto ilemod = static_cast<size_t>(szpalt_x * szpalt_y);
            for (size_t m = 0; m < ilemod; ++m) {
                const CRect& dst = GetNormalizedModuleRect(m);
                if (intsec.IntersectRect(src, dst))
                    if ((space.GetBit(m) > 0 ? 1 : 0) != bitToSet && (space_mark2.GetBit(m) > 0 ? 1 : 0) != bitToSet) {
                        space.SetBit(m, bitToSet);
                        space_mark2.SetBit(m, bitToSet);
                    } else if (intsec == dst)
                        break; // w kracie niebazowej kolizja na powierzchni pelnego modulu
            }
        }
        SetBaseKrata(s_x, s_y, FALSE);
    }

    space.SetBit(module, bitToSet);
    space_mark.SetBit(module, bitToSet);

    SetDirty();
    Invalidate();
}

// GN *****************************************************************************************
void CDrawPage::BoundingBox(PGENEPSARG pArg, int *bx1, int *by1, int *bx2, int *by2)
{
    BOOL first = TRUE;
    for (const auto& pAdd : m_adds) {
        auto pRozAdd = m_pDocument->GetCRozm(pArg, pAdd->szpalt_x, pAdd->szpalt_y, pAdd->spad_flag ? 0 : pAdd->typ_xx); // gdy zaznaczono spad, to montuj do kraty
        auto pRozKraty = pAdd->typ_xx ? m_pDocument->GetCRozm(pArg, pAdd->szpalt_x, pAdd->szpalt_y, 0) : pRozAdd;       // pobierz rozmiar kraty, jesli dotychczas nie jest znany

        const auto x = (int)(pAdd->posx - 1) * (pRozKraty->w + pRozKraty->sw);
        const auto y = (int)(pAdd->szpalt_y + 1 - pAdd->posy - pAdd->sizey) * (pRozKraty->h + pRozKraty->sh);
        const auto gpx = (int)(pAdd->sizex * (pRozAdd->w + pRozAdd->sw) - pRozAdd->sw + 3);
        const auto gpy = (int)(pAdd->sizey * (pRozAdd->h + pRozAdd->sh) - pRozAdd->sh + 3);

        if (first) { // liczymy w mm
            *bx1 = x; *bx2 = x + gpx; *by1 = y; *by2 = y + gpy; first = FALSE;
        } else {
            *bx1 = min(*bx1, x);		*by1 = min(*by1, y);
            *bx2 = max(*bx2, x + gpx);	*by2 = max(*by2, y + gpy);
        }
    }
    *bx1 = (int)ceil(*bx1 * mm2pkt - 0.5);
    *by1 = (int)ceil(*by1 * mm2pkt - 0.5); //-1 zeby nie obcinac u dolu
    *bx2 = (int)ceil(*bx2 * mm2pkt - 0.5);
    *by2 = (int)ceil(*by2 * mm2pkt - 0.5) + podpisH;
} // BoundingBox

BOOL CDrawPage::CheckSrcFile(PGENEPSARG pArg)
{
    BOOL isOK = TRUE;
    f5_errInfo.Empty();
    CString sNrStr = _T("Str. ") + GetNrPaginy() + _T(": ");

    pArg->pDlg->StrInfo(pArg->iChannelId, sNrStr);
    if (!CheckRozmKrat(pArg)) {
        isOK = FALSE;
        f5_errInfo = _T("brak wymiarów krat ");
    } else {
        for (const auto& pAdd : m_adds) {
            if (pAdd->CheckSrcFile(pArg)) {
                if (theApp.autoMark) pAdd->flags.showeps = TRUE;
                if (pAdd->flags.studio == STUDIO_BRAK) {
                    pAdd->flags.studio = STUDIO_JEST;
                    pAdd->SetDirty();
                }
            } else {
                isOK = FALSE;
                if (theApp.autoMark) pAdd->flags.showeps = FALSE;
                if (pAdd->flags.studio == STUDIO_JEST) {
                    pAdd->flags.studio = STUDIO_BRAK;
                    pAdd->SetDirty();
                }
            }
            f5_errInfo += pAdd->f5_errInfo;
        }
    }
    if (!f5_errInfo.IsEmpty())
        f5_errInfo = (sNrStr + f5_errInfo + _T("\n"));
    return isOK;
} //CheckSrcFile

BOOL CDrawPage::StaleElementy(PGENEPSARG pArg, CFile *handle)
{
    BOOL isOK = TRUE;
    std::vector<CString> elementy;
    CManODPNETParms orapar {
        { CManODPNET::DbTypeInt32, &m_pDocument->m_mak_xx },
        { CManODPNET::DbTypeInt32, &id_str },
        { CManODPNET::DbTypeRefCursor, CManODPNET::ParameterOut, nullptr }
    };
    if (theManODPNET.FillArr(&elementy, "begin pagina.gen_ps(:mak_xx,:str_xx,:retCur); end;", orapar)) {
        for (auto& elem : elementy) {
            if (elem[0] == _T('#')) { // wklej eps
                float x, y;
                TCHAR logoName[64] = _T("");
                if (_stscanf_s(elem, _T("#%s %f %f logo"), logoName, 64, &x, &y) != 3) return FALSE;
                elem.Format(_T("%.3f %.3f translate\r\n"), x, y); handle->Write(CStringA(elem), elem.GetLength());
                CString logo = theApp.GetProfileString(_T("GenEPS"), _T("EpsSrc"), _T(""));
                logo += CString(((logo.Right(1) == "\\") ? "" : "\\")) + "logo\\" + logoName;
                elem.Format(_T("beginManamEPS\r\n%%%%BeginDocument: %s\r\n"), logo); handle->Write(CStringA(elem), elem.GetLength());

                if (theApp.isOpiMode) {
                    auto buf = reinterpret_cast<char*>(pArg->cBigBuf);
                    StringCchPrintfA(buf, n_size, "%sLG %s\r\n", OPI_TAG, CStringA(logoName));
                    handle->Write(buf, (UINT)strlen(buf));
                } else {
                    CFile eps;
                    if (!eps.Open(logo, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite)) {
                        ::MessageBox(pArg->pDlg->m_hWnd, _T("Nie mo¿na otworzyæ pliku ") + logo, _T("B³¹d"), MB_ICONERROR);
                        return FALSE;
                    }
                    UINT res;
                    while ((res = eps.Read(pArg->cBigBuf, n_size)) > 0)
                        handle->Write(pArg->cBigBuf, (UINT)res);
                    eps.Close();
                }

                handle->Write("%%EndDocument\r\nendManamEPS\r\n", 28);
                elem.Format(_T("%.3f %.3f translate\r\n"), -x, -y); handle->Write(CStringA(elem), elem.GetLength());
            } else {
                elem += _T("\r\n");
                handle->Write(CStringA(elem), elem.GetLength());
            }
        }
    } else isOK = FALSE;

    return isOK;
} // StaleElementy

BOOL CDrawPage::CheckRozmKrat(PGENEPSARG pArg)
{
    BOOL isValid = TRUE;

    if (!m_pDocument->GetCRozm(pArg, szpalt_x, szpalt_y))
        isValid = FALSE;
    else for (const auto& kn : m_kraty_niebazowe)
        if (!m_pDocument->GetCRozm(pArg, kn.m_szpalt_x, kn.m_szpalt_y)) {
            isValid = FALSE;
            break;
        }

    return isValid;
} // CheckRozmKrat

BOOL CDrawPage::GetDestName(PGENEPSARG pArg, const CString& sNum, CString& destName)
{
    TCHAR* aExt[3] = { _T(".eps"), _T(".ps"), _T(".pdf") };
    destName = theApp.GetProfileString(_T("GenEPS"), pArg->bIsPRN ? _T("PsDst") : _T("EpsDst"), _T(""));
    destName += ((destName.Right(1) == _T("\\")) ? _T("") : _T("\\"));
    int pos = m_pDocument->gazeta.Find(_T(" "));
    if (pArg->bIsPRN) {
        CString dbDestName = CString(' ', 20);
        CManODPNETParms orapar {
            { CManODPNET::DbTypeInt32, &m_pDocument->m_mak_xx },
            { CManODPNET::DbTypeInt32, &id_str },
            { CManODPNET::DbTypeVarchar2, CManODPNET::ParameterOut, &dbDestName }
        };
        orapar.outParamsCount = 1;
        theManODPNET.EI("begin pagina.get_dest_name(:mak_xx,:str_xx,:name); end;", orapar);
        if (pArg->bDoKorekty) {
            dbDestName.SetAt(15, _T('k'));
            if (name.Find(_T("RED")) >= 0)
                dbDestName.SetAt(16, _T('e'));
        }
        destName.Append(dbDestName);
        destName.Append(aExt[pArg->bIsPRN]);
    } else
        destName += m_pDocument->dayws + (pos < 0 ? m_pDocument->gazeta : m_pDocument->gazeta.Left(pos) + m_pDocument->gazeta.Mid(pos + 1))
        + sNum + aExt[pArg->bIsPRN];
    CFileFind ff;
    return !m_pDocument->ovEPS && ff.FindFile(destName);
}

BOOL CDrawPage::GenEPS(PGENEPSARG pArg)
{
    CString num;
    auto wThreadBuf = pArg->cBigBuf;
    auto cThreadBuf = reinterpret_cast<char*>(pArg->cBigBuf);
    if (pArg->bIsPRN) {
        int lnr_porz = m_pDocument->GetIPage(this);
        num.Format(_T("%03i"), lnr_porz ? lnr_porz : (int)m_pDocument->m_pages.size());
    } else
        ((nr & c_rzym) == c_rzym) ? num = Rzymska(nr >> 3) : num.Format(_T("%03i"), (nr >> 3));

    CString dest_name;
    BOOL isDrobEPS = name.Find(_T("DR")) > -1;
    BOOL fileWarn = GetDestName(pArg, num, dest_name);
    CString sUid;
    sUid.Format(_T(" guid: %s"), GenerateGUIDString());
    const char *tofind[] = { "%!PS-Adobe-3.1 EPSF-3.0", "%%BoundingBox:", "%%Creator:", "%%Title:", "%%CreationDate:", "%%Copyright:", "%%DocumentProcessColors:", "MIEJSCE_NA_EPS" };
    CStringA towrite[7];
    towrite[0] = "%!PS-Adobe-3.1";
    towrite[1] = "<dynamic>";
    towrite[2] = CStringA(APP_NAME) + (theApp.isOpiMode ? "-OPI" : "") + CStringA(theApp.m_app_version);
    towrite[3] = CStringA(dest_name + sUid);
    towrite[4] = CStringA(CTime::GetCurrentTime().Format(c_ctimeCzas));
    towrite[5] = "Agora SA";
    towrite[6] = " Cyan Magenta Yellow Black";

    pArg->pDlg->StrInfo(pArg->iChannelId, CString(_T("Strona ")) + num + CString(_T(" do pliku ")) + dest_name);

    if (isDrobEPS && dirty) {
        ::MessageBox(pArg->pDlg->m_hWnd, _T("Przed wyeksportowaniem strony z drobnymi nale¿y zachowaæ makietê"), APP_NAME, MB_OK | MB_ICONINFORMATION);
        return FALSE;
    } else if (m_drukarnie == 0 && pArg->bIsPRN == 1 && pArg->bDoKorekty == 0) {
        ::MessageBox(pArg->pDlg->m_hWnd, "Proszê wybraæ drukarnie dla strony " + num, _T("Brak danych"), MB_OK);
        return FALSE;
    } else if (!CheckRozmKrat(pArg)) {
        ::MessageBox(pArg->pDlg->m_hWnd, "Brak wymiarów dla wszystkich krat strony " + num, _T("Brak danych"), MB_OK);
        return FALSE;
    }
    if (fileWarn && !theApp.isOpiMode) { // czy plik istnieje
        if (IDNO == ::MessageBox(pArg->pDlg->m_hWnd, _T("Docelowy plik ") + dest_name + _T(" ju¿ istnieje. Czy chcesz zastapiæ wszystkie istniej¹ce pliki?"), APP_NAME, MB_YESNO | MB_ICONQUESTION))
            return FALSE;
        m_pDocument->ovEPS = TRUE;
    }

    CMemFile destOpi;
    CFile destNonOpi;
    CFile *dest = nullptr;

    if (theApp.isOpiMode) {
        destOpi.SetFilePath(dest_name);
        dest = &destOpi;
    } else {
        dest = &destNonOpi;
        if (!dest->Open(dest_name, CFile::modeCreate | CFile::modeReadWrite | CFile::typeBinary)) {
            theApp.SetErrorMessage(pArg->cBigBuf);
            ::MessageBox(pArg->pDlg->m_hWnd, CString(pArg->cBigBuf) + "\n" + dest_name, _T("B³¹d"), MB_OK);
            return FALSE;
        }
    }

    CMemFile fPagina;
    if (pArg->bIsPRN && !StaleElementy(pArg, &fPagina))
        return FALSE;

    CFile externFile;
    CFileException fEx;
    // drobne
    CString drobneEpsPath;
    if (isDrobEPS && !theApp.isOpiMode) {
        drobneEpsPath = theApp.GetProfileString(_T("GenEPS"), _T("EpsDrobne"), _T(""));
        drobneEpsPath += (((drobneEpsPath.Right(1) == "\\") ? "" : "\\") + m_pDocument->data.Left(2) + "\\" + m_pDocument->gazeta.Left(3) + m_pDocument->gazeta.Mid(4, 2) + num + ".eps");
        if (externFile.Open(drobneEpsPath, CFile::modeRead | CFile::shareDenyWrite, &fEx)) {
            externFile.Close();
        } else if (fEx.m_cause != CFileException::fileNotFound && fEx.m_cause != CFileException::none) {
            fEx.GetErrorMessage(pArg->cBigBuf, DLGMSG_MAX_LEN);
            AfxMessageBox(pArg->cBigBuf, MB_ICONSTOP);
            isDrobEPS = FALSE;
        } else {
            ::MessageBox(pArg->pDlg->m_hWnd, CString("Nie odnaleziono pliku z drobnymi: ") + drobneEpsPath, APP_NAME, MB_OK | MB_ICONERROR);
            isDrobEPS = FALSE;
        }
    }

    // uzupelnienie dziedziczenia
    BOOL isUzupEPS = FALSE;
    CString uzupEpsPath;
    if (this->m_dervlvl == DERV_TMPL) {
        uzupEpsPath = theApp.GetProfileString(_T("GenEPS"), _T("EpsUzupel"), _T(""));
        if (!uzupEpsPath.IsEmpty()) {
            int dervNum;
            CString sdervNum;
            if (_stscanf_s(m_dervinfo.Mid(7), _T("%i"), &dervNum) == 1)
                sdervNum.Format(_T("%s%03i"), theApp.activeDoc->dayws, dervNum);
            uzupEpsPath += (((uzupEpsPath.Right(1) == "\\") ? "" : "\\") + m_dervinfo.Left(3) + m_dervinfo.Mid(4, 2) + sdervNum + _T(".eps"));
            if (externFile.Open(uzupEpsPath, CFile::modeRead | CFile::shareDenyWrite, &fEx)) {
                isUzupEPS = TRUE;
                externFile.Close();
            } else if (fEx.m_cause != CFileException::fileNotFound && fEx.m_cause != CFileException::none) {
                fEx.GetErrorMessage(pArg->cBigBuf, DLGMSG_MAX_LEN);
                AfxMessageBox(pArg->cBigBuf, MB_ICONSTOP);
            } else
                ::MessageBox(pArg->pDlg->m_hWnd, CString("Nie odnaleziono strony bazowej: ") + uzupEpsPath, APP_NAME, MB_OK | MB_ICONERROR);
        }
    }

    int BX1 = 0, BY1 = 0, BX2 = 0, BY2 = 0;
    if (!pArg->bIsPRN) {
        if (isDrobEPS) { BX1 = 0; BX2 = 709; BY1 = 0; BY2 = (int)m_pDocument->GetDrobneH() + 5; } else BoundingBox(pArg, &BX1, &BY1, &BX2, &BY2);
        pArg->bIsPreview = (pArg->bIsPreview && BX1 != BX2 && BY1 != BY2);
    }

    BOOL ok = TRUE;

    CStdioFile fManamEps;
    if (!fManamEps.Open(theApp.sManamEpsName, CFile::modeRead | CFile::typeText | CFile::shareDenyWrite)) {
        theApp.SetErrorMessage(pArg->cBigBuf);
        ::MessageBox(pArg->pDlg->m_hWnd, CString("Otwarcie pliku Manam.eps nie powiod³o siê\n") + pArg->cBigBuf, _T("B³¹d"), MB_ICONERROR);
        return FALSE;
    }

    try {
        WORD header[15];
        if (pArg->bIsPreview)
            dest->Write(header, sizeof(header));

        int i = 0;
        while (ok && (fManamEps.ReadString(wThreadBuf, n_size) != NULL)) {
            CStringA line = wThreadBuf;
            if (strstr(line, tofind[i]) != nullptr) {
                switch (i) {
                    case 0: line = (pArg->bIsPRN) ? towrite[i] : tofind[i];
                        break;
                    case 1: if (pArg->bIsPRN) {
                        if (sBoundingBox.IsEmpty()) { // pobierz BoundingBox z formatu papieru
                            CStringW wBoundingBox(' ', 32);
                            CManODPNETParms orapar {
                                { CManODPNET::DbTypeInt32, &m_pDocument->m_mak_xx },
                                { CManODPNET::DbTypeInt32, &this->id_str },
                                { CManODPNET::DbTypeVarchar2, CManODPNET::ParameterOut, &wBoundingBox }
                            };
                            orapar.outParamsCount = 1;
                            theManODPNET.EI("begin pagina.get_boundingbox(:mak_xx,:str_xx,:bb); end;", orapar);
                            line = CStringA(tofind[i]) + " " + CW2A(wBoundingBox);
                        }
                    } else
                        line.Format("%%%%BoundingBox: %d %d %d %d", BX1, BY1, BX2, BY2);
                    break;
                    case 6: line = CStringA(tofind[i]) + (kolor == 1 ? " Black" : towrite[i]);
                        break;
                    default:
                        line = CStringA(tofind[i]) + " ";
                        if (i < 7) line.Append(towrite[i]);
                } // switch
                if (++i == 8) break;
                line.Append("\r\n");
            }
            dest->Write(line, line.GetLength());
        }
        if (!pArg->bIsPRN)
            dest->Write("%%EndPageSetup\r\n/STAN_STR save def\r\n", 36);
        dest->Flush();
    } catch (CException* e2) {
        theApp.SetErrorMessage(pArg->cBigBuf);
        ::MessageBox(pArg->pDlg->m_hWnd, CString("B³¹d przepisywania plików\n") + pArg->cBigBuf, _T("B³¹d"), MB_ICONERROR);
        e2->Delete();
        ok = FALSE;
    }

    if (ok && pArg->bIsPRN) {
        fPagina.SeekToBegin();
        ULONGLONG lLen = fPagina.GetLength();
        while (lLen > 0) {
            UINT lBR = fPagina.Read(cThreadBuf, bigSize);
            dest->Write(cThreadBuf, lBR);
            lLen -= lBR;
        }
        fPagina.Close();
    }

    // extern
    if (ok && (isDrobEPS || isUzupEPS)) {
        CDrawAdd externAdd(&m_position);
        externAdd.m_pDocument = m_pDocument;
        externAdd.typ_xx = 0;
        if (isUzupEPS) {
            CFlag dervSpace = GetReservedFlag();
            if (dervSpace.IsSet()) { //ustal posx,posy,sizex,sizey
                int i, j;
                for (i = szpalt_x*szpalt_y - 1; i >= 0; i--)
                    if (dervSpace.GetBit(i) > 0) break;
                if (i >= 0) {
                    externAdd.posy = szpalt_y - i / szpalt_x;
                    for (j = 0; j <= i; j++)
                        if (dervSpace.GetBit(j) > 0) break;
                    externAdd.sizey = szpalt_y - j / szpalt_x - externAdd.posy + 1;
                    int endy = externAdd.posy + externAdd.sizey - 1;
                    for (i = 1; i <= szpalt_x; i++)
                        for (j = externAdd.posy; j <= endy; j++)
                            if (dervSpace.GetBit(szpalt_x*(szpalt_y - j + 1) - i) > 0) goto foundposx;
foundposx:
                    externAdd.posx = i;
                    for (i = szpalt_x; i > 0; i--)
                        for (j = externAdd.posy; j <= endy; j++)
                            if (dervSpace.GetBit(szpalt_x*(szpalt_y - j + 1) - i) > 0) goto foundsizex;
foundsizex:
                    externAdd.sizex = i - externAdd.posx + 1;
                    externAdd.szpalt_x = szpalt_x;
                    externAdd.szpalt_y = szpalt_y;
                    externAdd.wersja = DERV_TMPL_WER;
                    externAdd.nazwa = uzupEpsPath;
                    ok = externAdd.RewriteEps(pArg, dest);
                }
            }
        }
        if (isDrobEPS) {
            externAdd.txtposx = std::find_if(std::cbegin(m_pDocument->m_pages), std::cend(m_pDocument->m_pages), [](const CDrawPage* p) {return p->nr & c_rzym; }) != std::cend(m_pDocument->m_pages) ? nr >> 3 : m_pDocument->GetIPage(this);
            if (externAdd.txtposx == 0) externAdd.txtposx = (int)m_pDocument->m_pages.size();
            externAdd.posx = externAdd.posy = 1;
            externAdd.sizex = externAdd.szpalt_x = pszpalt_x;
            externAdd.sizey = externAdd.szpalt_y = pszpalt_y;
            externAdd.nazwa = drobneEpsPath;
            ok = externAdd.RewriteDrob(pArg, dest);
        }
    }

    auto itAdd = m_adds.cbegin();
    while (ok && itAdd != m_adds.cend() && !pArg->pDlg->cancelGenEPS) {
        BOOL bAddOK = (*itAdd++)->RewriteEps(pArg, dest);
        if (!bAddOK && pArg->bIsPRN == 1 && pArg->bDoKorekty == 0) {
            if (!theApp.isOpiMode) {
                CString sPath(dest->GetFilePath());
                dest->Close();
                ::DeleteFile(sPath);
            }
            return FALSE;
        }
    }

    UINT br;
    try {
        while (ok && (br = fManamEps.Read(cThreadBuf, n_size)) > 0)
            dest->Write(cThreadBuf, br);
        dest->Write("\r\n", 2);
    } catch (CFileException* e2) {
        theApp.SetErrorMessage(pArg->cBigBuf);
        ::MessageBox(pArg->pDlg->m_hWnd, CString("B³¹d przepisywania plików 2\n") + pArg->cBigBuf, _T("B³¹d"), MB_ICONERROR);
        e2->Delete();
        ok = FALSE;
    }

    if (ok && pArg->bIsPreview)
        Preview(pArg, dest, BX1, BY1, BX2, BY2);

    if (fManamEps.m_hFile != CFile::hFileNull) fManamEps.Close();

    if (ok && theApp.isOpiMode)
        ok &= PostPageToWorkflowServer(pArg, (CMemFile*)dest);

    dest->Close();

    return ok;
}

BOOL CDrawPage::PostPageToWorkflowServer(PGENEPSARG pArg, CMemFile *pOpiFile) const
{
    CStringA sOpiSeparator;
    if (!sOpiSeparator.LoadString(IDS_OPISEPAR))
        throw CManPDFExc(_T("Nie odnaleziono separatora OPI"));

    CStringA sFormBody, sFormData; /* rfc2046 */
    const CString sAppId(APP_NAME + theApp.m_app_version);
    const CStringA sOpiLine = "--" + sOpiSeparator + "--\r\n";
    sFormData.Format("--%s\r\nContent-Disposition: form-data; name=\"", sOpiSeparator);
    sFormBody.AppendFormat("%suser\"\r\n\r\n%s\r\n", sFormData, CStringA(theManODPNET.m_userName));
    sFormBody.AppendFormat("%sdaydir\"\r\n\r\n%s\r\n", sFormData, CStringA(m_pDocument->dayws));
    sFormBody.AppendFormat("%sftype\"\r\n\r\nPAGE\r\n", sFormData);
    sFormBody.AppendFormat("%sfname\"\r\n\r\n%s\r\n", sFormData, CStringA(pOpiFile->GetFileName()));
    sFormBody.AppendFormat("%sfsize\"\r\n\r\n%llu\r\n", sFormData, pOpiFile->GetLength());
    sFormBody.AppendFormat("%sversion\"\r\n\r\n%s\r\n", sFormData, CStringA(sAppId));
    sFormBody.AppendFormat("%sfile\"; filename=\"%s\"\r\nContent-Type: application/postscript\r\n\r\n", sFormData, CStringA(pOpiFile->GetFileName()));

    INTERNET_PORT nPort;
    DWORD dwServiceType;
    CString strServerName, strObject;
    if (!AfxParseURL(m_pDocument->sOpiServerUrl, dwServiceType, strServerName, strObject, nPort)) {
        ::MessageBox(pArg->pDlg->m_hWnd, _T("Nieprawid³owy adres OPI: ") + m_pDocument->sOpiServerUrl, _T("B³¹d"), MB_ICONEXCLAMATION);
        return false;
    } else {
        theApp.BeginWaitCursor();
        CInternetSession ses(sAppId, PRE_CONFIG_INTERNET_ACCESS);
        std::unique_ptr<CHttpConnection> pSrv(ses.GetHttpConnection(strServerName, nPort));
        std::unique_ptr<CHttpFile> pFile(pSrv->OpenRequest(CHttpConnection::HTTP_VERB_POST, strObject, NULL, 1, NULL, NULL, INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_NO_AUTO_REDIRECT));
        pFile->AddRequestHeaders(_T("Content-Type: multipart/form-data; boundary=") + CString(sOpiSeparator));

        int iReqLen = (int)pOpiFile->GetLength() + sFormBody.GetLength() + sOpiSeparator.GetLength() + 4; /* cztery minusy wokol ostatniego sOpiSeparator */
        try {
            pFile->SendRequestEx(iReqLen);
            pFile->Write(sFormBody, sFormBody.GetLength());
            //wklej plik
            pOpiFile->SeekToBegin();
            ULONGLONG lLen = pOpiFile->GetLength();
            while (lLen > 0) {
                UINT lBR = pOpiFile->Read(pArg->cBigBuf, bigSize);
                pFile->Write(pArg->cBigBuf, lBR);
                lLen -= lBR;
            }
            pFile->Write(sOpiLine, sOpiLine.GetLength());
            pFile->EndRequest();
            pFile->ReadString(pArg->cBigBuf, bigSize);
        } catch (CInternetException* iex) {
            iex->GetErrorMessage(pArg->cBigBuf, DLGMSG_MAX_LEN);
            ::StringCchCat(pArg->cBigBuf, n_size, m_pDocument->sOpiServerUrl);
            iex->Delete();
        }

        theApp.EndWaitCursor();
        if (pFile != nullptr) pFile->Close();
        if (pSrv != nullptr) pSrv->Close();
        ses.Close();

        if (*reinterpret_cast<short*>(pArg->cBigBuf) != 0x4b4f) { // "OK"
            ::MessageBox(pArg->pDlg->m_hWnd, CString(reinterpret_cast<char*>(pArg->cBigBuf)), _T("OPI Error"), MB_ICONERROR | MB_OK);
            return false;
        }
    }

    return true;
}

void CDrawPage::Preview(PGENEPSARG pArg, CFile *dest, int bx1, int by1, int bx2, int by2)
{ //GN     //x,y dx dy w 0.1 mm
    DWORD header[7];
    header[0] = 0xC6D3D0C5L; // eps z preview
    header[1] = (DWORD)preview_offset; // poczatek eps'a 
    header[2] = (DWORD)(dest->GetLength() - preview_offset);  //dl eps'a
    header[3] = (DWORD)0;
    header[4] = (DWORD)0;
    header[5] = (DWORD)dest->GetLength(); // pozycja tifa
    int x = (int)ceil(bx1 / pkt_10m - 0.5);
    int y = (int)ceil(by1 / pkt_10m - 0.5);
    int dx = (int)ceil((bx2 - bx1) / pkt_10m - 0.5);
    int dy = 2 + (int)ceil((by2 - by1 - podpisH) / pkt_10m - 0.5);
    div_t szer_t = div(dx, 8);
    int szer = szer_t.quot + min(szer_t.rem, 1);
    int p_size = (int)(dy*szer * sizeof(BYTE));

    if (p_size > n_size) {
        const TCHAR* errMsg = _T("Zbyt gêsta krata. Proszê wy³¹czyæ opcjê generowania preview");
        if (p_size < 10 * n_size) {
            pArg->cBigBuf = (TCHAR*)LocalReAlloc(pArg->cBigBuf, p_size, LMEM_MOVEABLE);
            if (pArg->cBigBuf == nullptr) {
                ::MessageBox(pArg->pDlg->m_hWnd, errMsg, _T("Brak pamiêci"), MB_OK | MB_ICONERROR);
                return;
            }
        } else {
            ::MessageBox(pArg->pDlg->m_hWnd, errMsg, _T("B³¹d"), MB_OK | MB_ICONINFORMATION);
            dest->SetLength(0);
            return;
        }
    }

    memset(pArg->cBigBuf, 0, p_size);
    for (const auto& pAdd : m_adds)
        pAdd->Preview(pArg, x, y, dy, szer);

    // CManTiff::TiffHeader(dest, dx, dy, szer);
    dest->Write(pArg->cBigBuf, p_size);

    header[6] = (DWORD)dest->GetLength() - header[5];
    dest->SeekToBegin();
    dest->Write(header, sizeof(header));
    WORD w = (WORD)0xFFFF;
    dest->Write(&w, sizeof(WORD));
}

BOOL CDrawPage::GenPDF(PGENEPSARG pArg)
{
    CString dstName, num = GetNrPaginy();

    CManPDF pdf(pArg);
    dstName.Format(_T("%02i"), nr >> 3);

    if (!CheckRozmKrat(pArg)) {
        ::MessageBox(pArg->pDlg->m_hWnd, "Brak wymiarów dla wszystkich krat strony: " + num, _T("B³¹d"), MB_ICONERROR | MB_OK);
        return FALSE;
    }

    BOOL fileWarn = GetDestName(pArg, num, dstName);
    pArg->pDlg->StrInfo(pArg->iChannelId, CString("Strona ") + num + "\ndo pliku " + dstName);

    // czy plik istnieje
    if (fileWarn) {
        if (IDNO == ::MessageBox(pArg->pDlg->m_hWnd, _T("Docelowy plik ju¿ istnieje. Czy chcesz zast¹piæ wszystkie istniej¹ce pliki?"), _T("GenPdf"), MB_YESNO | MB_ICONQUESTION))
            return FALSE;
        m_pDocument->ovEPS = TRUE;
    }

    pdf.CreatePDF(this, dstName);

    return TRUE;
}
