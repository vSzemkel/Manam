// *****************************************************
// Copyright (C) 1999-2014 Marcin Buchwald dla Agora SA
// All rights reserved.
// *****************************************************

#pragma once

enum class ToolbarMode : BYTE
{
	normal = 0,
	czas_obow,
	tryb_studia,
};

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();
	virtual ~CMainFrame();
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr) override;
#ifdef _DEBUG
	virtual void AssertValid() const override;
	virtual void Dump(CDumpContext& dc) const override;
#endif
private:
	void StoreComboHandlers();
	void SwapToolbarImages(int iCmd1, int iCmd2);
public:
	void SetStatusBarInfo(LPCTSTR tx);
	UINT GetKolor(int ile_spotow) const;   //zwraca kolor z (-) gdy konkretny spot kolor
	CBrush* GetSpotBrush(int i) const;
	CString GetCaption() const;
	CString GetCaption(int i) const;
	CString GetCapStrFromData( DWORD w) const;
	int GetIdxfromSpotID(UINT spot_id) const;
	int GetCaptionBoxSize() const;
	void SetToolbarBitmap(ToolbarMode bPrevMode, ToolbarMode bNewMode);

	int GetKolorInd(CString text) const;
	DWORD GetCaptionDataItem(int ind) const;
	CString GetKolorText() const;
	void InsKolorBox();
	void InsComboNrSpotow(int new_i);
	void IniKolorTable();
	void LoadKolorCombo();

	void IniCaptionBox(int id_drw, int new_id_drw);
	void IniCaptionCombo(BOOL iscaption);
	BOOL DBIniCaptionCombo(BOOL iscaption, int id_drw);

	//naglowki i strlog sa tylko w comboboxie - bo lokalne dla drzewa

	std::vector<UINT> Spot_ID;
	std::vector<CString> Spot_Kolor;
	std::vector<CBrush*> Spot_Brush;

	CPen pen;
	CBrush cyjan, magenta, yellow, rzym, robgcolor;
	BOOL show_spacelocks;

	int lastCapToolBar ;	// 0==combo zawiera nag³ówki_eps; >0==id_drw
	int lastColToolBar ;	// 0==combo zawiera wersje_eps; >0==id_drw
// Implementation
public:
	void SetLogonStatus( LPCTSTR t);
	void SetOpenStatus( LPCTSTR t);
	void SetRoleStatus();

protected:
// control bar embedded members
	//CMFCRibbonBar	m_wndRibbonBar;
	//CMFCRibbonApplicationButton m_MainButton;
	CMFCStatusBar m_wndStatusBar;
	CMFCMenuBar m_wndMenuBar;
	CMFCToolBar m_wndToolBar;
	CComboBox* m_KolorBox;
	CComboBox* m_CaptionBox;
	CString m_LastSessionKolor;
	CEdit* m_CaptionEditBox;
	BOOL CreateManamToolBar();
	LRESULT OnToolBarReset(WPARAM wp, LPARAM);
	//void InitRibbon ();
// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg void OnFullScreen();
	afx_msg void OnToolBarCustomize();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
