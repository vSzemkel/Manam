
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CSpacerDlg dialog

class CSpacerDlg : public CDialog
{
    // Construction
public:
    CSpacerDlg(CDrawAdd *vAdd, CWnd *pParent = nullptr);   // standard constructor
    static BOOL Deal(CDrawAdd *vAdd);

    // Dialog Data
        //{{AFX_DATA(CSpacerDlg)
    enum { IDD = IDD_SPACER };
    CDateTimeCtrl	m_lastemisionctl;
    CComboBox	m_typ_ogl_combo;
    CComboBox	m_kolorcombo;
    CListBox	m_emisjelist;
    CListBox m_ollist;
    int 	m_posx;
    int 	m_posy;
    int 	m_sizex;
    int 	m_sizey;
    int 	m_spacer;
    int 	m_typ_xx;
    int 	dd, mm, rrrr;
    BOOL	m_exactplace;
    BOOL	m_niekratowe;
    BOOL	m_blokada;
    BOOL	m_pagelayout;
    BOOL	m_pageparity;
    BOOL	m_sekcja;
    BOOL	m_wsekcji;
    BOOL	m_dealappend;
    BOOL	queDeal;
    BOOL	refreshOnClose;
    CString	m_nazwa;
    CString	m_uwagi;
    CString	m_wersja;
    CTime	m_lastemision;
    UINT	m_kolor;
    //}}AFX_DATA
    CWordArray m_typ_ogl_arr;
    CByteArray m_typ_sizex_arr;
    CByteArray m_typ_sizey_arr;
    std::vector<CString> m_typ_precel_arr;
private:
    static const int narrowcx, widecx, olcx, normalcy;
    static const int qfExact, qfSectionExact, qfSecParExact;
    BOOL	firstSearch;
    BOOL	olSelected;
    int m_mak_xx;
    int m_add_xx;
    int	m_quepub_xx;
    int m_first_emision_pub_xx;
    int m_str_xx;
    int m_szpalt_x, m_szpalt_y;
    CDrawAdd* pub;

    int GetQueryFlag() const;						//oblicza flagê warunków zamówienia co do miejsca
    void SetBlokadaState();					//ustawia status kontrolki do blokady miejsca odpowiednio do flagi warunków zamówienia
    void EnableMultiCond(BOOL flag) const;
    void InsertRequestNoDup(CString& kiedy, CString& mut);
    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSpacerDlg)
protected:
    virtual void OnOK() override;
    virtual void OnCancel() override;
    virtual BOOL DestroyWindow() override;
    virtual BOOL OnInitDialog() override;
    virtual void DoDataExchange(CDataExchange *pDX) override;    // DDX/DDV support
    //}}AFX_VIRTUAL

    // Generated message map functions
    //{{AFX_MSG(CSpacerDlg)
    afx_msg void OnLocked();
    afx_msg void OnKratowe();
    afx_msg void OnDealappend();
    afx_msg void OnCloseupLastemision(NMHDR *pNMHDR, LRESULT* pResult);
    afx_msg void OnEmisje();
    afx_msg void OnSelchangeKrok();
    afx_msg void OnExactplace();
    afx_msg void OnSekcja();
    afx_msg void OnWsekcji();
    afx_msg void OnPageparity();
    afx_msg void OnPagelayout();
    afx_msg void OnDelsel();
    afx_msg void OnDelall();
    afx_msg void OnQue();
    afx_msg void OnZDnia();
    afx_msg void OnBnClickedOl();
    afx_msg void OnBnClickedMutacje();
    afx_msg void OnBnClickedMalaSiec();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
