
#pragma once
// AtexKrat.h : header file
//
class CDrawDoc;
/////////////////////////////////////////////////////////////////////////////
// CAtexKrat window

class CAtexKrat
{
    // Construction
public:
    CAtexKrat(const TCHAR* atexKratInfo, CDrawDoc* vDoc);
    virtual ~CAtexKrat() = default;
    // Implementation
public:
    static constexpr int DIM_LIMIT = 16;      // jak gestej kraty podstawowej szukamy
    bool isValid;   // flaga poprawnosci parsowania napisu w konstruktorze
    bool isToSmall; // gêstoœæ obliczonej kraty przekracza DIM_LIMIT
    bool Compute(int* sizex, int* sizey, int* s_x, int* s_y); // liczy rozmiary ogloszenia i krate strony
private:
    static constexpr int ADJUST_LEVEL = 6;    // przez max ile mnozymy rozmiar podstawowy
    static constexpr int KRATA_PASKOWA = 10;  // od takiego wymiaru wysokoœci szerokoœæ jest ujednolicana na 1 szpalte
    static constexpr float TOLERANCE = 0.05F; // wzgledna tolerancja rozmiaru
    float dy;			// wysokosc strony
    float xcol;			// ile szpalt na stronie zajmuje ogloszenie
    float szpalt_x;		// ile jest szpalt na stronie
    float ymm;			// wysokosc ogloszenia
    float xmm;			// szerokosc ogloszenia
    CDrawDoc* doc;		// aktywny dokument

    bool CompX(int* sizex, int* s_x) const noexcept; // liczy szerokosc ogloszenia i liczbe kolumn strony
    bool CompY(int* sizey, int* s_y) const noexcept; // liczy wysokosc ogloszenia i liczbe wierszy strony
};

// CKratCalc dialog

class CKratCalc : public CDialog
{
    DECLARE_DYNAMIC(CKratCalc)

public:
    CKratCalc(CWnd* pParent = nullptr); // standard constructor
    ~CKratCalc() override = default;

    // Dialog Data
    enum { IDD = IDD_KRATCALC };

protected:
    void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    BOOL OnInitDialog() override;

    afx_msg void OnDeltaposSpin(UINT spinCtrlId, NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnCbnSelchangeKratka();
    afx_msg void OnDefineModelid();
    afx_msg void OnModeSwitch();
private:
    static constexpr double TOLERANCE = 0.011F; // wzgledna tolerancja rozmiaru
    bool m_autocalcMode;                        // tryb kalkulatora czy rêczny
    int m_x;
    int m_y;
    int m_lightx;
    int m_lighty;
    int m_userdef_sizex;
    int m_userdef_sizey;
    double m_stronax;
    double m_stronay;
    CString m_sizex;
    CString m_sizey;
    CString m_modelid;
    CString m_kra_sym;
    CEdit m_wynik;
    CComboBox m_kratycombo;

    void Calculate();
};
