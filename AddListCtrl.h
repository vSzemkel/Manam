#pragma once

class CGridFrm;

enum kolejnosc : uint8_t
{
    lp,
    logiczna,
    zamowienie,
    kod,
    rozmiar,
    nazwa,
    strona,
    kolorek,
    uwagi,
    powtorka,
    oldadno,
    studio,
    lastused
};

class CAddListCtrl sealed : public CMFCListCtrl
{
    DECLARE_DYNAMIC(CAddListCtrl)
public:
    CAddListCtrl(CGridFrm *pView);
    virtual ~CAddListCtrl() = default;

    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
protected:
    DECLARE_MESSAGE_MAP()
private:
    virtual void InitHeader() override;
    virtual int OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn) override;
    static const int iWheelUnitY;
    CGridFrm *m_pContainer;
    CImageList m_SmallImageList;
};
