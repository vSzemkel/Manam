// *****************************************************
// Copyright (C) 1999-2014 Marcin Buchwald dla Agora SA
// All rights reserved.
// *****************************************************

#pragma once

#include "ManConst.h"

class CMainFrame : public CMDIFrameWndEx
{
    DECLARE_DYNAMIC(CMainFrame)
public:
    CMainFrame();
    ~CMainFrame() override;
    BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr) override;
#ifdef _DEBUG
    void AssertValid() const override;
    void Dump(CDumpContext& dc) const override;
#endif
private:
    void StoreComboHandlers();
    void SwapToolbarImages(int iCmd1, int iCmd2);
public:
    void SetStatusBarInfo(LPCTSTR tx);
    int GetKolor(int ile_spotow) const;   //zwraca kolor z (-) gdy konkretny spot kolor
    CString GetCaption() const;
    CString GetCaption(int i) const;
    CString GetCapStrFromData(DWORD_PTR captionId) const;
    int GetCaptionBoxSize() const;
    void SetToolbarBitmap(ToolbarMode bPrevMode, ToolbarMode bNewMode);

    int GetKolorInd(LPCTSTR text) const noexcept;
    DWORD GetCaptionDataItem(int ind) const;
    CString GetKolorText() const noexcept;
    void InsKolorBox();
    void InsComboNrSpotow(int new_i);
    void LoadKolorCombo();

    void IniCaptionBox(int id_drw, int new_id_drw);
    void IniCaptionCombo(bool iscaption);
    BOOL DBIniCaptionCombo(bool iscaption, int id_drw);

    CPen pen;
    CBrush cyjan, magenta, yellow, rzym, robgcolor;

    int lastCapToolBar{0};      // 0==combo zawiera nag³ówki_eps; >0==id_drw
    bool show_spacelocks{true}; // domyœlnie pokazujemy zablokowane modu³y
    bool lastColToolBar{false}; // czy combo zawiera kolory
// Implementation
public:
    void SetLogonStatus(LPCTSTR t);
    void SetOpenStatus(LPCTSTR t);
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
    bool CreateManamToolBar();
    LRESULT OnToolBarReset(WPARAM wp, LPARAM /*unused*/);
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
