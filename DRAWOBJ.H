
#pragma once

#include "flag.h"

class CDrawView;
class CDrawDoc;

using delobj_t = std::pair<int, short>;

/////////////////////////////////////////////////////////////////////////////
// CDrawObj - base class for all 'drawable objects'

class CDrawObj : public CObject
{
protected:
    DECLARE_SERIAL(CDrawObj);
    CDrawObj();
    static CString RzCyfra(int i, const CString *znaki); //vu
    int GetVertPrintShift() const;

    // Constructors
public:
    CDrawObj(const CRect *position);
    virtual ~CDrawObj() {};
    virtual void Draw(CDC *pDC);
    virtual void Print(CDC *pDC) { Draw(pDC); }
    virtual void UpdateInfo() {}
    virtual void DrawKolor(CDC *pDC, const CRect& pos) const;
    virtual void MoveTo(const CRect& positon, CDrawView *pView = nullptr);
    virtual void Serialize(CArchive& ar) override;
    virtual BOOL OnOpen(CDrawView *pView);
    virtual CDrawObj* Clone(CDrawDoc *pDoc = nullptr) const;

    static double modx(double x);
    static double mody(double y);
    static CString Rzymska(int i);
    static int Arabska(LPCTSTR rz);
    static void DrawNapis(CDC *pDC, LPCTSTR napis, int cnt, LPRECT r, UINT format, int bkMode);

    enum TrackerState { normal, selected, active };
    // Attributes
    CRect m_position;
    CDrawDoc *m_pDocument;
    CString info;
    UINT kolor;
    BOOL dirty;
    //3 bity na to ile kolorow czyli 100=4 full; 010=2 spot; 001 =1 brak
    // wyzsze bity dla ogloszenia oznaczaja id_spotu 
    // dla strony wyzsze bity oznaczaja nr spotu spotI spotII itp

    int GetHandleCount() const;
    CPoint GetHandle(int nHandle) const;
    CRect GetHandleRect(int nHandleID, CDrawView *pView) const;
    HCURSOR GetHandleCursor(int nHandle) const;
    int HitTest(const CPoint& point, CDrawView *pView, BOOL bSelected) const;
    void ChangeKolor(UINT new_kolor);
    void SetDirty();
    void SetClean() { dirty = FALSE; }
    BOOL IsClean() const { return (!dirty); }


    // Operations
    void Remove();
    void Invalidate();
    void DrawTracker(CDC *pDC, TrackerState state) const;
    void MoveHandleTo(int nHandle, const CPoint& point, CDrawView *pView = nullptr);

    CRect GetPrintRect() const;
    BOOL Intersects(const CRect& rect) const;
    BOOL Contains(const CPoint& point) const;
#ifdef _DEBUG
    void AssertValid();
#endif
};
