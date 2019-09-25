
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CSpacerDlg dialog

class CSpacerDlg : public CDialog
{
  public:
    CSpacerDlg(CDrawAdd* vAdd, CWnd* pParent = nullptr);   // standard constructor
    static BOOL Deal(CDrawAdd* vAdd);

    //{{AFX_DATA(CSpacerDlg)
    enum { IDD = IDD_SPACER };
    std::vector<CString> m_typ_precel_arr;
    CDateTimeCtrl m_lastemisionctl;
    CWordArray m_typ_ogl_arr;
    CByteArray m_typ_sizex_arr;
    CByteArray m_typ_sizey_arr;
    CComboBox m_typ_ogl_combo;
    CComboBox m_kolorcombo;
    CListBox m_emisjelist;
    CListBox m_ollist;
    CString m_nazwa;
    CString m_uwagi;
    CString m_wersja;
    CTime   m_lastemision;
    UINT    m_kolor;
    int     m_posx;
    int     m_posy;
    int     m_sizex;
    int     m_sizey;
    int     m_spacer;
    int     m_typ_xx;
    int     m_dd, m_mm, m_rrrr;
    BOOL    m_onclose_refresh;
    BOOL    m_exactplace;
    BOOL    m_niekratowe;
    BOOL    m_pagelayout;
    BOOL    m_pageparity;
    BOOL    m_dealappend;
    BOOL    m_que_deal;
    BOOL    m_blokada;
    BOOL    m_wsekcji;
    BOOL    m_sekcja;
    //}}AFX_DATA
  private:
    static constexpr int narrowcx = 333;
    static constexpr int widecx = 475;
    static constexpr int olcx = 535;
    static constexpr int normalcy = 380;
    static constexpr int qfExact = 33;
    static constexpr int qfSectionExact = 58;
    static constexpr int qfSecParExact = 62;
    CDrawAdd* pub;
    int m_mak_xx;
    int m_add_xx;
    int	m_quepub_xx;
    int m_first_emision_pub_xx;
    int m_str_xx;
    int m_szpalt_x, m_szpalt_y;
    bool m_firstSearch{true};
    bool m_olSelected{false};

    int GetQueryFlag() const; // oblicza flagê warunków zamówienia co do miejsca
    void SetBlokadaState();   // ustawia status kontrolki do blokady miejsca odpowiednio do flagi warunków zamówienia
    void EnableMultiCond(BOOL flag) const;
    void InsertRequestNoDup(CString& kiedy, CString& mut);

protected:
    //{{AFX_VIRTUAL(CSpacerDlg)
    void OnOK() override;
    void OnCancel() override;
    BOOL DestroyWindow() override;
    BOOL OnInitDialog() override;
    void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
    //}}AFX_VIRTUAL

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
