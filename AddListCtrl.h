#pragma once

class CGridFrm;

enum kolejnosc : unsigned char {
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
private:
	virtual void InitHeader() override;
	virtual int OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn) override;
	static const int iWheelUnitY;
	CGridFrm *m_pContainer;
	CImageList m_SmallImageList;
public:
	CAddListCtrl(CGridFrm* pView);
	virtual ~CAddListCtrl() {};

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
};
