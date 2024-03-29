// koldlg.h : header file

#pragma once

#include "afxwin.h"
#include "atltime.h"
#include "afxcmn.h"
#include "afxdtctl.h"

// podpisy og�osze� maj� wsp�rz�dne z zakresu [-TXTSHIFT, TXTSHIFT]
constexpr const int TXTSHIFT = 20;

class CDrawDoc;
class CDrawAdd;

/////////////////////////////////////////////////////////////////////////////
// CConnDlg dialog

class CConnDlg final : public CDialog
{
    // Construction
public:
    CConnDlg(CWnd* pParent = nullptr); // standard constructor

// Dialog Data
    //{{AFX_DATA(CConnDlg)
    enum { IDD = IDD_CONNDLG };
    CString m_loginname;
    CString m_passwd;
    int     m_dbtest;
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CConnDlg)
protected:
    void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CConnDlg)
    BOOL OnInitDialog() override;
    void OnOK() override;
    //}}AFX_MSG
    afx_msg void OnChangePasswd();
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CKolDlg dialog

class CKolDlg final : public CDialog
{
    // Construction
public:
    CKolDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
    //{{AFX_DATA(CKolDlg)
    enum { IDD = IDD_KOLUMNY };
    CComboBox	m_combo_box;
    CString		m_ile_kolumn;
    //}}AFX_DATA

// Implementation
protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CKolDlg)
    BOOL OnInitDialog() override;
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CPageDlg dialog
class CPageDlg final : public CDialog
{
  public:
    CPageDlg(CWnd* pParent = nullptr); // standard constructor
    //{{AFX_DATA(CPageDlg)
    enum { IDD = IDD_PAGE };
    CFlag m_prn_flag;
    CString m_mutred;
    CString m_docmutred;
    CString m_caption;
    CString m_name;
    CString m_dervinfo;
    CTime m_deadline;
    CTime m_deadlineday;
    CTime m_red;
    CTime m_fot;
    CTime m_kol;
    long m_drukarnie;
    int m_id_str;
    int m_szpalt_x;
    int m_szpalt_y;
    int m_prn_mak_xx;
    int m_wyd_xx; // wydawca strony
    int m_mak_xx;
    int m_nr;
    int m_topage;
    int m_typ_pary;
    UINT m_kolor;
    UINT m_ile_spotow;
    BOOL m_sztywna_kratka;
    BOOL m_iscaption;
    BOOL m_rzymska;
    BOOL m_niemakietuj;
    DervType m_dervlvl;
    //}}AFX_DATA
  protected:
    BOOL OnInitDialog() override;
    void OnOK() override;
    void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

    //{{AFX_MSG(CPageDlg)
    afx_msg void GiveOutStrLog();
    afx_msg void OnSelchangePrnMak();
    afx_msg void OnBnClickedAcdead();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
  private:
    CListBox m_mutredcombo;
    CListBox m_drukarniecombo;
    CListBox m_prn_fun;
    CComboBox m_prn_mak;
    CComboBox m_kratkacombo;
    CComboBox m_kolorcombo;
    CComboBox m_namecombo;
    CComboBox m_captioncombo;
    CComboBox m_wydawcycombo; // lista potencjalnych wydawc�w strony
    void SetFunListBox(bool setDefaults);
};
/////////////////////////////////////////////////////////////////////////////
// CAddDlg dialog

class CAddDlg final : public CDialog
{
    // Construction
public:
    friend class CManODPNET;
    CAddDlg(CWnd* pParent = nullptr);	// standard constructor
    void OnOK() override;
    // Dialog Data
    //{{AFX_DATA(CAddDlg)
    enum { IDD = IDD_ADD };
    CDateTimeCtrl	m_powtctrl;
    CListBox	m_emisjelist;
    CComboBox	m_typ_ogl_combo;
    CButton	m_kratowe;
    CComboBox	m_kratkacombo;
    CEdit	m_posyedit;
    CEdit	m_posxedit;
    CEdit	m_fizpageedit;
    CButton	m_checkbox;
    CComboBox m_kolorcombo;
    CComboBox m_zajawkacombo;
    UINT	m_kolor;
    int 	m_add_xx;
    int 	m_typ_xx;
    int 	m_nag_xx;
    int 	m_pub_xx;
    int 	m_fizpage;
    int 	m_sizex;
    int 	m_sizey;
    int 	m_posx;
    int 	m_posy;
    int 	m_txtposx;
    int 	m_txtposy;
    int 	m_studio;
    int		m_spad;
    int 	m_szpalt_x;
    int 	m_szpalt_y;
    CString	m_logpage;
    CString	m_nazwa;
    CString	m_nreps;
    CString	m_remarks;
    CString	m_sprzedal;
    CString	m_wersja;
    CString m_precel_flag;
    CString m_kod_modulu;
    CString	m_uwagi_atex;
    BOOL	m_locked;
    BOOL	m_rzymnum;
    BOOL	m_flaga_rezerw;
    BOOL	m_zagroz;
    BOOL	m_digital;
    BOOL	m_always;
    BOOL	m_epsok;
    BOOL	m_fromQue;
    BOOL	m_derived;
    CTime	m_data_czob;
    CTime	m_godz_czob;
    CTime	m_powt;
    long	m_oldadno;
    //}}AFX_DATA
private:
    static BOOL matchingOldAdnoUpdate;
    BOOL m_fullView;
    BOOL m_wymiarowe;
    BOOL m_spad_top;
    BOOL m_spad_bottom;
    BOOL m_spad_left;
    BOOL m_spad_right;
    CString m_initAdno;
    CString m_candidateAdnoDate;
    std::vector<CString> m_candidateAdno;
    CWordArray m_typ_ogl_arr;
    CByteArray m_typ_sizex_arr;
    CByteArray m_typ_sizey_arr;
    std::vector<CString> m_typ_precel_arr;
    int c_height, c_narrow, c_wide;
    void RefreshNiekrat();
    BOOL AtexVerify(LPCTSTR adno);
    BOOL UniqueAdno(LPCTSTR adno);
    CDrawAdd* dupNoSpacer;
    CDrawDoc* vActiveDoc;
    // Implementation
protected:
    BOOL OnInitDialog() override;
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CAddDlg)
    afx_msg void OnLocked();
    afx_msg void OnKratowe();
    afx_msg void OnSelchangeKratka();
    afx_msg void OnAlways();
    afx_msg void OnEmisje();
    afx_msg void OnDelsel();
    afx_msg void OnDelall();
    afx_msg void OnAtex();
    afx_msg void OnCbpowt();
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnBnClickedOdblokuj();
    afx_msg void OnEnUpdateOldadno();
    afx_msg void OnEnChangeWersja();
    afx_msg void OnCbnSelchangeZajawka();
    afx_msg void OnDtnDatetimechangePowt(NMHDR* pNMHDR, LRESULT* pResult);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CInfoDlg dialog

class CInfoDlg final : public CDialog
{
    // Construction
public:
    CInfoDlg(CWnd* pParent = nullptr);	// standard constructor

    enum { IDD = IDD_INFO };
    CString m_wydawca;
    CString m_wydawcared;
    CString m_korekta;
    CString m_data;
    CString	m_gazeta;
    CString	m_prowadz1;
    CString	m_prowadz2;
    CString	m_sekretarz;
    CString m_kto_makietuje;
    CString m_cena;
    CString m_cena2;
    CString	m_sign_text;
    CString	m_grzbiet;
    CString m_uwagi;
    CString m_opis_papieru;
    CString m_wydaw_str;
    CString m_typ_dodatku;
    long    m_drukarnie;
    long    m_naklad;
    int     m_objetosc;
    int     m_numer;
    int     m_numerrok;
    UINT    m_modogl;
    UINT    m_modred;
    UINT    m_modrez;
    UINT    m_modwol;
    float   m_modredp;
    float   m_modrezp;
    float   m_modwolp;
    float   m_modoglp;
    float   m_modcnt;
    float   m_quecnt;
    CTime   m_data_wykupu;
    CTime   m_data_studio;
    CTime   m_data_zamkniecia;
    CTime   m_data_deadline;
    CTime   m_godz_wykupu;
    CTime   m_godz_studio;
    CTime   m_godz_zamkniecia;
    CTime   m_godz_deadline;
    BOOL    m_set_deadline;
    BOOL    m_set_kraty;
    BOOL    m_set_papier;
    BOOL    m_szyj;
    BOOL    m_isRO;
    CListBox m_drukarniecombo;
    CComboBox m_wydawcycombo;
    CComboBox m_wydawcyredcombo;
    CComboBox m_korektacombo;
    //}}AFX_DATA
  protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CInfoDlg)
    BOOL OnInitDialog() override;
    afx_msg void OnShowPageDeadlines();
    afx_msg void OnSign();
    afx_msg void OnOK() override;
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// COpisDlg dialog

class COpisDlg final : public CDialog
{
    // Construction
public:
    COpisDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
    //{{AFX_DATA(COpisDlg)
    enum { IDD = IDD_OPIS };
    CString	m_opis;
    //}}AFX_DATA

// Implementation
protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(COpisDlg)
        // NOTE: the ClassWizard will add member functions here
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:
    BOOL m_Centruj{FALSE};
};

/////////////////////////////////////////////////////////////////////////////
// CZoomDlg dialog

class CZoomDlg final : public CDialog
{
    // Construction
public:
    CZoomDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
    //{{AFX_DATA(CZoomDlg)
    enum { IDD = IDD_CUSTZOOM };
    int		m_zoomX;
    //}}AFX_DATA

// Implementation
protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CZoomDlg)
        // NOTE: the ClassWizard will add member functions here
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CDbDlg dialog

class CDbDlg final : public CDialog
{
    // Construction
public:
    CDbDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
    //{{AFX_DATA(CDbDlg)
    enum { IDD = IDD_DB_ADD };
    CString	m_pap;
    CString	m_mut;
    CString	m_strlog;
    BOOL	m_client;
    CTime	m_dtime;
    CString	m_alttyt;
    CString	m_altmut;
    CString	m_zone;
    BOOL	m_zakres;
    //}}AFX_DATA
    CString m_dt;

    // Implementation
protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support
    void OnOK() override;
    // Generated message map functions
    //{{AFX_MSG(CDbDlg)
    BOOL OnInitDialog() override;
    afx_msg void OnDefineZone();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CDrzDlg dialog
class CDrzDlg final : public CDialog
{
    // Construction
public:
    CDrzDlg(CWnd* pParent = nullptr);	// standard constructor
// Dialog Data
    //{{AFX_DATA(CDrzDlg)
    enum { IDD = IDD_DBKOLUMNY };
    CComboBox	m_kol_combo;
    CComboBox	m_drz_combo;
    CString	m_ile_kolumn;
    CDateTimeCtrl m_kiedy;
    //}}AFX_DATA
    CString m_gazeta;
    int m_id_drw;
    // Implementation
protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support
    BOOL OnInitDialog() override;

    // Generated message map functions
    //{{AFX_MSG(CDrzDlg)
    void OnOK() override;
    afx_msg void OnKiedyChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnCbnEditupdateDrzewo();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
private:
    void SelectDrzewo(const CTime& t);
};
/////////////////////////////////////////////////////////////////////////////
// CDrz1Dlg dialog

class CDrz1Dlg final : public CDialog
{
    // Construction
public:
    CDrz1Dlg(CWnd* pParent = nullptr);	// standard constructor
// Dialog Data
    //{{AFX_DATA(CDrz1Dlg)
    enum { IDD = IDD_DRZEWO };
    CComboBox	m_drz_combo;
    //}}AFX_DATA
    CString m_gazeta;
    int m_id_drw;
    // Implementation
protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support
    BOOL OnInitDialog() override;
    void OnOK() override;

    // Generated message map functions
    //{{AFX_MSG(CDrz1Dlg)
        // NOTE: the ClassWizard will add member functions here
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CDBOpenDlg dialog

class CDBOpenDlg final : public CDialog
{
    // Construction
public:
    CDBOpenDlg(CWnd* pParent = nullptr);	// standard constructor
// Dialog Data
    //{{AFX_DATA(CDBOpenDlg)
    enum { IDD = IDD_DB_OPEN };
    int			m_doctype;
    int			m_okres_ind;
    CTime		m_dtime;
    CString		m_tytul;
    CString		m_mutacja;
    CListBox	m_date_combo;
    CListBox	m_mutacja_combo;
    CComboBox	m_wersje_lib;
    //}}AFX_DATA
    CString m_dt;
    std::vector<CString> m_arrDaty;
    // Implementation
private:
    BOOL bEnableCtrls;
    void RefreshCombo();
protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support
    void OnOK() override;
    // Generated message map functions
    //{{AFX_MSG(CDBOpenDlg)
    afx_msg void OnChangeTytul();
    afx_msg void OnChangeMutacja();
    afx_msg void OnBnClickedAllMut();
    afx_msg void OnDtnDatetimechangeDt(NMHDR* pNMHDR, LRESULT* pResult);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CDBSaveAsDlg dialog
class CDBSaveAsDlg final : public CDialog
{
    // Construction
public:
    CDBSaveAsDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
    //{{AFX_DATA(CDBSaveAsDlg)
    enum { IDD = IDD_DB_SAVEAS };
    CString	m_tytmut;
    BOOL	m_lib;
    CTime	m_dtime;
    CString	m_wersja;
    //}}AFX_DATA
    CString m_dt;

    // Implementation
protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support
    void OnOK() override;
    BOOL OnInitDialog() override;
    // Generated message map functions
    //{{AFX_MSG(CDBSaveAsDlg)
    afx_msg void OnLib();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CAddDesc dialog

#define	TEXT_NAZWA		0
#define	TEXT_PLIK		1
#define TEXT_WARLOG		2
#define TEXT_CZASKTO	3
#define	TEXT_UWAGI		4
#define TEXT_STUDIO		5
#define	TEXT_EPS		6
#define TEXT_BRAK		7

class CAddDesc final : public CDialog
{
    // Construction
public:
    CAddDesc(int top, int bottom, CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
    //{{AFX_DATA(CAddDesc)
    enum { IDD = IDD_ADDDESC };
    int		m_top;
    int		m_bottom;
    //}}AFX_DATA

// Implementation
protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CAddDesc)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CAddFindDlg dialog

class CAddFindDlg final : public CDialog
{
    // Construction
public:
    CAddFindDlg(CWnd* pParent = nullptr); // standard constructor

// Dialog Data
    //{{AFX_DATA(CAddFindDlg)
    CString m_nazwa;
    long    m_nreps;
    long    m_spacer;
    enum { IDD = IDD_ADD_FIND };
    //}}AFX_DATA

// Implementation
protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CAddFindDlg)
    afx_msg void OnUpdateNr();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CInfoDlgLib dialog

class CInfoDlgLib final : public CDialog
{
    // Construction
public:
    CInfoDlgLib(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
    //{{AFX_DATA(CInfoDlgLib)
    enum { IDD = IDD_INFOLIB };
    CString	m_opis;
    CString	m_tytmut;
    CString	m_wersja;
    CString m_papier;
    BOOL m_set_papier;
    BOOL m_szycie;
    //}}AFX_DATA

// Implementation
protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CInfoDlgLib)
        // NOTE: the ClassWizard will add member functions here
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CConfDlg dialog

class CConfDlg final : public CDialog
{
    // Construction
public:
    CConfDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
    //{{AFX_DATA(CConfDlg)
    enum { IDD = IDD_CONFIG };
    int		m_strcnt;
    int		m_format;
    int		m_makietujAll;
    int		m_iPagesInRow;
    BOOL	m_makietujDoKupy;
    BOOL	m_subdir;
    BOOL	m_contnum;
    BOOL	m_daydirs;
    BOOL	m_opi_mode;
    BOOL	m_copyold;
    BOOL	m_podwaly;
    BOOL	m_save_dirs;
    BOOL	m_podwal_subdir;
    UINT	m_hashing;
    CString	m_deadline;
    CString	m_epsdst;
    CString	m_epssrc;
    CString	m_epszaj;
    CString	m_epsold;
    CString	m_epsdro;
    CString	m_epspod;
    CString	m_epsirf;
    CString m_psdst;
    CString m_epsuzu;
    CString m_epskok;
    //}}AFX_DATA

// Implementation
protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CConfDlg)
    void OnOK() override;
    BOOL OnInitDialog() override;
    afx_msg void OnDaydirs();
    afx_msg void OnSubdir();
    afx_msg void OnBnClickedCopyPodwaly();
    afx_msg void OnBnClickedOpiMode();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

class CPassDlg final : public CDialog
{
    // Construction
public:
    CPassDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
    //{{AFX_DATA(CPassDlg)
    enum { IDD = IDD_PASSWD };
    CString	m_newpass;
    CString	m_newpass2;
    CString	m_oldpass;
    //}}AFX_DATA

// Implementation
protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CPassDlg)
    void OnOK() override;
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CPrnEpsDlg dialog

class CPrnEpsDlg final : public CDialog
{
    // Construction
public:
    CPrnEpsDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
    //{{AFX_DATA(CPrnEpsDlg)
    enum { IDD = IDD_PRNEPS };
    int		m_page;
    union
    {
        CManFormat m_format;
        int m_format_padded;
    };
    CString	m_od;
    CString	m_do;
    CString m_subset;
    BOOL	m_preview;
    BOOL	m_signall;
    BOOL	m_markfound;
    BOOL	m_streamed;
    BOOL	m_korekta;
    BOOL	m_exclude_emptypages;
    BOOL	m_isprint;
    CComboBox	m_cbrip;
    CComboBox	m_cbdruk;
    //}}AFX_DATA

// Implementation
    CFlag GetChoosenPages(CDrawDoc* pDoc) const noexcept;

  protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CPrnEpsDlg)
    BOOL OnInitDialog() override;
    void OnOK() override;
    afx_msg void OnAllPages();
    afx_msg void OnFmteps();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CUserDlg dialog

class CUserDlg final : public CDialog
{
    // Construction
public:
    CUserDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
    //{{AFX_DATA(CUserDlg)
    enum { IDD = IDD_NOWY_USER };
    CString	m_grupa;
    CString	m_imie;
    CString	m_loginname;
    CString m_login_nds;
    CString	m_nazwisko;
    CString	m_pass;
    CString	m_telefon;
    CString	m_uwagi;
    //}}AFX_DATA
    BOOL yesNext;
    // Implementation
protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CUserDlg)
    afx_msg void OnZaloz();
    afx_msg void OnYesnnext();
    BOOL OnInitDialog() override;
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CAccDlg dialog

class CAccDlg final : public CDialog
{
    // Construction
public:
    CAccDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
    //{{AFX_DATA(CAccDlg)
    enum { IDD = IDD_ACCESS };
    BOOL	m_alltyt;
    BOOL	m_dacc;
    CString	m_loginname;
    CString	m_mutacja;
    BOOL	m_racc;
    CString	m_tytul;
    BOOL	m_wacc;
    CString	m_senior;
    BOOL	m_sacc;
    BOOL	m_pacc;
    BOOL	m_gacc;
    //}}AFX_DATA
    BOOL	yesNext;

    // Implementation
protected:
    void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CAccDlg)
    afx_msg void OnAlltyt();
    afx_msg void OnYesnnext();
    BOOL OnInitDialog() override;
    afx_msg void OnGrantAlike();
    void OnOK() override;
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CNewTitleDlg dialog

class CNewTitleDlg final : public CDialog
{
    // Construction
public:
    CNewTitleDlg(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CNewTitleDlg)
    enum { IDD = IDD_NOWY_TYTUL };
    CString	m_mutacja;
    CString	m_opis;
    CString	m_tytul;
    int		m_kra_x;
    int		m_kra_y;
    int		m_strona_x;
    int		m_strona_y;
    int		m_sw_w;
    int		m_sw_h;
    CString	m_tytul_upraw;
    CString	m_mutacja_upraw;
    CTime m_do_kiedy;
    //}}AFX_DATA
// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CNewTitleDlg)
protected:
    void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CNewTitleDlg)
        // NOTE: the ClassWizard will add member functions here
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CEPSDateDlg dialog

class CEPSDateDlg final : public CDialog
{
    // Construction
public:
    CEPSDateDlg(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CEPSDateDlg)
    enum { IDD = IDD_EPSDATE };
    CListCtrl	m_epsdate;
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CEPSDateDlg)
protected:
    void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    CDrawDoc* vDoc;
    CImageList m_SmallImageList, m_StateImageList;
    // Generated message map functions
    //{{AFX_MSG(CEPSDateDlg)
    BOOL OnInitDialog() override;
    afx_msg void OnUznaj();
    afx_msg void OnClickEpsdate(NMHDR *pNMHDR, LRESULT* pResult);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CDirDaysDlg dialog

class CDirDaysDlg final : public CDialog
{
    // Construction
public:
    CDirDaysDlg(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CDirDaysDlg)
    enum { IDD = IDD_DIRDAYS };
    CString	m_path;
    CTime	m_odkiedy;
    CTime	m_dokiedy;
    //}}AFX_DATA
    CTimeSpan m_oneday;


    // Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CDirDaysDlg)
protected:
    void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CDirDaysDlg)
    afx_msg void OnDaydirs();
    afx_msg void OnCab();
    void OnOK() override;
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CPageDerv dialog

class CPageDerv final : public CDialog
{
    // Construction
public:
    CPageDerv(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CPageDerv)
    enum { IDD = IDD_PAGEDERV };
    CListBox m_tytmut;
    int m_direction;
    int m_ilekol;
    int m_nr;
    int m_base_nr;
    int m_mak_xx;
    int m_drw_xx;
    DervType m_idervlvl;
    //}}AFX_DATA
    std::vector<int> m_derv_add; // kt�re mutacje maja zaczac dziedziczyc z tej strony
    std::vector<int> m_derv_del; // kt�re mutacje maja przestac dziedziczyc z tej strony
    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CPageDerv)
protected:
    void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CPageDerv)
    BOOL OnInitDialog() override;
    void OnOK() override;
    afx_msg void OnSelchangeDervlvl();
    afx_msg void OnDirectionChange(UINT mode);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CGrzbDlg dialog

class CGrzbDlg final : public CDialog
{
    // Construction
public:
    long m_drw_xx;
    BOOL m_delete;
    BOOL bRefreshOnClose;				// od�wie�anie dla funkcji 4x4
    CGrzbDlg(CWnd* pParent = nullptr);

    // Dialog Data
        //{{AFX_DATA(CGrzbDlg)
    enum { IDD = IDD_GRZBIET };
    CComboBox	m_tytmut;
    int		m_insert;
    int		m_incordec;
    int		m_insert_cnt;
    int		m_delete_cnt;
    int		m_split_cnt;
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CGrzbDlg)
protected:
    void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    // Generated message map functions
    //{{AFX_MSG(CGrzbDlg)
    BOOL OnInitDialog() override;
    void OnOK() override;
    afx_msg void OnExpandGrzbiet();
    afx_msg void OnShrinkGrzbiet();
    afx_msg void OnBnClicked4x4();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
private:
    void ChangeMode(BOOL isExpand) const;
};

// CAccGrbDlg dialog
class CAccGrbDlg final : public CDialog
{
    DECLARE_DYNAMIC(CAccGrbDlg)

public:
    CAccGrbDlg(CWnd* pParent = nullptr);   // standard constructor
    ~CAccGrbDlg() override = default;

    // Dialog Data
    enum { IDD = IDD_ACCGRB };

protected:
    BOOL OnInitDialog() override;
    void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    CString m_print_ldrz;
    CString m_print_cdrz;
    CString m_print_org;
    BOOL m_ckpldrz;
    BOOL m_ckpcdrz;
    BOOL m_ckporg;
};

// COstWer dialog
class COstWer final : public CDialog
{
    DECLARE_DYNAMIC(COstWer)

public:
    COstWer(CWnd* pParent = nullptr);   // standard constructor
    COstWer(std::vector<CDrawAdd*> *aNewAdds, std::vector<CDrawAdd*> *aOldAdds, std::vector<CDrawAdd*> *aModifAdds, std::vector<CDrawAdd*> *aDelAdds, BOOL bBankOnly);
    ~COstWer() override = default;

    // Dialog Data
    enum { IDD = IDD_ATEXADDS };

protected:
    void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
    BOOL OnInitDialog() override;
    void OnCancel() override {};

    DECLARE_MESSAGE_MAP()
private:
    BOOL m_bBankOnly;
    CListCtrl m_adds;
    CImageList m_SmallImageList, m_StateImageList;
    std::vector<CDrawAdd*> *m_aNewAdds, *m_aOldAdds, *m_aModifAdds, *m_aDelAdds;
    void AppendAdd(CDrawAdd* pAdd, int status);
public:
    afx_msg void OnOK() override;
    afx_msg void OnSelectAll();
    afx_msg void OnNMClickAddlist(NMHDR* pNMHDR, LRESULT* pResult);
};

// CAcDeadDlg dialog
class CAcDeadDlg final : public CDialog
{
    DECLARE_DYNAMIC(CAcDeadDlg)

public:
    CTime m_red;
    CTime m_fot;
    CTime m_kol;

    CAcDeadDlg(CWnd* pParent = nullptr);   // standard constructor
    ~CAcDeadDlg() override = default;

    // Dialog Data
    enum { IDD = IDD_ACDEAD };

protected:
    void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
};

// CAboutDlg dialog
class CAboutDlg final : public CDialog
{
    DECLARE_DYNAMIC(CAboutDlg)
public:
    CAboutDlg();

    // Dialog Data
        //{{AFX_DATA(CAboutDlg)
    enum { IDD = IDD_ABOUTBOX };
    CString	m_client;
    CString	m_memstat;
    //}}AFX_DATA

// Implementation
protected:
    //{{AFX_MSG(CAboutDlg)
    void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
    BOOL OnInitDialog() override;
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
#pragma once
