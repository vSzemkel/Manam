
#pragma once

class CDrawAdd;

class CQueView : public CScrollView
{
    DECLARE_DYNCREATE(CQueView)
protected:
    CQueView();			// protected constructor used by dynamic creation

// Attributes
public:
    static CDrawAdd *selected_add;
private:
    CPoint vPoint;
    CRect vPos;
    BOOL moving;
    // Operations 
public:
    CRect* GetStoredPosition() { return &vPos; }
    static CDrawAdd* GetSelectedAdd() { return selected_add; }
    // Implementation
protected:
    virtual ~CQueView() = default;
    virtual void OnDraw(CDC *pDC) override; // overridden to draw this view
    virtual void OnPrepareDC(CDC *pDC, CPrintInfo *pInfo) override;
    virtual void OnUpdate(CView *pSender, LPARAM lHint, CObject *pHint) override;
    virtual void OnInitialUpdate() override; // first time after construct
    auto GetDocument() const { return reinterpret_cast<CDrawDoc*>(m_pDocument); }
    void DocToClient(CRect *rect);
    void ClientToDoc(CRect *rect);
    void ClientToDoc(CPoint *point);
    void DocToClient(CPoint *point);
    void RepaintRect(CRect *rect);

    // Generated message map functions
    //{{AFX_MSG(CQueView)
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnEditClear();
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};