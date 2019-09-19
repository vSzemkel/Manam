
#include "StdAfx.h"
#include "DrawDoc.h"
#include "DrawOpis.h"
#include "DrawView.h"
#include "KolDlg.h"
#include "MainFrm.h"
#include "Manam.h"

IMPLEMENT_SERIAL(CDrawOpis, CDrawObj, 0)

const CPoint CDrawOpis::drawrogi(50, 70);
const CPoint CDrawOpis::printrogi(2, 3);

CDrawOpis::CDrawOpis(const CRect& position) noexcept : CDrawOpis(position, _T(""))
{
}

CDrawOpis::CDrawOpis(const CRect& position, const TCHAR *tx) noexcept :
    CDrawObj(position), m_opi_xx(-1), m_Scale(1.0f)
{
    ASSERT_VALID(this);
    info = tx;
}

void CDrawOpis::DrawInternal(CDC* pDC, CRect& rect) const
{
    auto pOldPen = (CPen*)pDC->SelectStockObject(BLACK_PEN);
    auto pOldFont = pDC->SelectObject(&m_pDocument->m_pagefont);

    const bool coloured = kolor > 7;
    auto pOldBrush = CDrawDoc::GetSpotBrush(coloured ? kolor >> 3 : 1);
    if (coloured) {
        LOGBRUSH lb;
        pOldBrush->GetLogBrush(&lb);
        pDC->SetTextColor(lb.lbColor ^ 0xffffff);
    }
    pOldBrush = pDC->SelectObject(pOldBrush);

    pDC->RoundRect(rect, static_cast<int>(m_Scale) > 5 || pDC->IsPrinting() ? printrogi : drawrogi);

    if (m_Scale != 1) {
        const auto scale = m_Scale / vscale;
        const auto& initExt = pDC->GetWindowExt();
        auto edge = reinterpret_cast<LONG*>(&rect);
        pDC->SetWindowExt((int)(initExt.cx * scale), (int)(initExt.cy * scale));
        for (size_t i = 0; i < 4; ++i, ++edge)
            *edge = (LONG)(*edge * scale);

        rect.OffsetRect(1, -1);
        const auto oldBM = pDC->SetBkMode(TRANSPARENT);
        pDC->DrawText(info, info.GetLength(), rect, DT_WORDBREAK);
        pDC->SetWindowExt(initExt);
        pDC->SetBkMode(oldBM);
    } else if (info.Find(_T("\n")) < 0)
        DrawNapis(pDC, info, info.GetLength(), rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP, TRANSPARENT);
    else
        DrawNapis(pDC, info, info.GetLength(), CRect(rect.left + 4 * vscale, rect.top, rect.right, rect.bottom), DT_LEFT | DT_NOCLIP, TRANSPARENT);

    pDC->SelectObject(pOldBrush);
    pDC->SelectObject(pOldPen);
    pDC->SelectObject(pOldFont);
    if (coloured)
        pDC->SetTextColor(0);
}

void CDrawOpis::Draw(CDC* pDC)
{
    ASSERT_VALID(this);

    CRect rect = m_position;
    DrawInternal(pDC, rect);
}

////////////////////////////////////////////////////////
/////////////// PRINT
void CDrawOpis::Print(CDC* pDC)
{
    ASSERT_VALID(this);

    pDC->SetBkMode(TRANSPARENT);
    CRect rect = GetPrintRect();
    DrawInternal(pDC, rect);
}

void CDrawOpis::Serialize(CArchive& ar)
{
    ASSERT_VALID(this);
    CDrawObj::Serialize(ar);

    if (ar.IsLoading()) {
        m_opi_xx = -1;
        ar >> m_Scale;
    } else
        ar << m_Scale;
}

CDrawObj* CDrawOpis::Clone(CDrawDoc* pDoc) const
{
    ASSERT_VALID(this);

    auto pClone = new CDrawOpis(m_position);
    ASSERT_VALID(pClone);
    pClone->info = info;
    pClone->kolor = kolor;
    pClone->m_Scale = m_Scale;
    pClone->m_position += CSize(15 * vscale, -15 * vscale);

    if (pDoc != nullptr) {
        pDoc->Add(pClone);
        pClone->Invalidate();
    }

    return pClone;
}

BOOL CDrawOpis::OnOpen(CDrawView* /*pView*/)
{
    ASSERT_VALID(this);

    COpisDlg dlg;
    dlg.m_opis = info;

    if (dlg.DoModal() != IDOK)
        return FALSE;
    info = dlg.m_opis;
    if (dlg.m_Centruj)
        m_Scale = 1.0F;

    SetDirty(); Invalidate();
    return TRUE;
}
