
#pragma once

const int NONEMPTY_QUEUE_HEIGHT = 70;

class CSplitFrame : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CSplitFrame)
private:
	BOOL bInitialized;
protected:
	CSplitFrame();           // protected constructor used by dynamic creation

// Attributes
public:
	CSplitterWndEx    m_Splitter;
	CSplitterWndEx    m_Splitter2;

// Operations
public:
	void ResizeQueView();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSplitFrame)
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSplitFrame();
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) override;

	// Generated message map functions
	//{{AFX_MSG(CSplitFrame)
	afx_msg void OnShowDtls();
	afx_msg void OnUpdateShowDtls(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
