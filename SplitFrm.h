
#pragma once

constexpr int NONEMPTY_QUEUE_HEIGHT = 70;

class CSplitFrame : public CMDIChildWndEx
{
    DECLARE_DYNCREATE(CSplitFrame)
    // Attributes
  public:
    CSplitterWndEx m_Splitter;
    CSplitterWndEx m_Splitter2;

    // Operations
    void ResizeQueView();

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSplitFrame)
    //}}AFX_VIRTUAL

  protected:
    CSplitFrame(){}; // protected constructor used by dynamic creation
    ~CSplitFrame() noexcept override = default;
    BOOL OnCreateClient(LPCREATESTRUCT cs, CCreateContext* pContext) override;

    // Generated message map functions
    //{{AFX_MSG(CSplitFrame)
    afx_msg void OnShowDtls();
    afx_msg void OnUpdateShowDtls(CCmdUI* pCmdUI);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

  private:
    bool m_initialized{false};
};
