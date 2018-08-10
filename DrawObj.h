
#pragma once

class CDrawView;
class CDrawDoc;

using delobj_t = std::pair<EntityType, int>;

/////////////////////////////////////////////////////////////////////////////
// CDrawObj - base class for all 'drawable objects'

class CDrawObj : public CObject
{
    DECLARE_SERIAL(CDrawObj);

    // public: declared in macro

    // Constructors
    CDrawObj() noexcept;
    CDrawObj(const CRect& position) noexcept;

    ~CDrawObj() override = default;
    virtual void Draw(CDC* pDC) {};
    virtual void Print(CDC* pDC) { Draw(pDC); }
    virtual void UpdateInfo() {}
    virtual void DrawKolor(CDC* pDC, const CRect& pos) const;
    virtual void MoveTo(const CRect& position, CDrawView* pView = nullptr);
    void Serialize(CArchive& ar) override;
    virtual BOOL OnOpen(CDrawView* pView);
    virtual CDrawObj* Clone(CDrawDoc* pDoc = nullptr) const { return nullptr; };

    static double modx(double x);
    static double mody(double y);
    static CString Rzymska(int i) noexcept;
    static int Arabska(LPCTSTR rz) noexcept;
    static CString RzCyfra(int digit, int offset) noexcept;
    static void DrawNapis(CDC* pDC, LPCTSTR napis, int cnt, LPRECT r, UINT format, int bkMode);

    enum TrackerState : uint8_t { normal, selected, active };
    // Attributes
    CString info;
    CRect m_position;
    bool dirty{true};
    UINT kolor{ColorId::brak};
    CDrawDoc* m_pDocument{nullptr};
    // 3 bity na to ile kolorow czyli 100=4 full; 010=2 spot; 001 =1 brak
    // wyzsze bity dla ogloszenia oznaczaja id_spotu
    // dla strony wyzsze bity oznaczaja nr spotu spotI spotII itp

    int GetHandleCount() const;
    int GetVertPrintShift() const;
    CPoint GetHandle(int nHandle) const;
    CRect GetHandleRect(int nHandleID, CDrawView* pView) const;
    HCURSOR GetHandleCursor(int nHandle) const;
    int HitTest(const CPoint& point, CDrawView* pView, bool selected) const;
    void ChangeKolor(UINT new_kolor);
    void SetDirty();
    void SetClean() { dirty = FALSE; }
    bool IsClean() const { return (!dirty); }

    // Operations
    void Remove();
    void Invalidate();
    void DrawTracker(CDC* pDC, TrackerState state) const;
    void MoveHandleTo(int nHandle, const CPoint& point, CDrawView* pView = nullptr);

    CRect GetPrintRect() const;
    bool Intersects(const CRect& rect) const;
    bool Contains(const CPoint& point) const;
};
