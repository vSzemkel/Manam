
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
    bool isValid;   // flaga poprawnosci parsowania napisu w konstruktorze
    bool isToSmall; // gêstoœæ obliczonej kraty przekracza DIM_LIMIT
    bool Compute(int* sizex, int* sizey, int* s_x, int* s_y); // liczy rozmiary ogloszenia i krate strony
    static const int DIM_LIMIT;     // jak gestej kraty podstawowej szukamy
    static const int ADJUST_LEVEL;  // przez max ile mnozymy rozmiar podstawowy
    static const int KRATA_PASKOWA; // od takiego wymiaru wysokoœci szerokoœæ jest ujednolicana na 1 szpalte
private:
    static const float TOLERANCE;	// wzgledna tolerancja rozmiaru
    float dy;			// wysokosc strony
    float xcol;			// ile szpalt na stronie zajmuje ogloszenie
    float szpalt_x;		// ile jest szpalt na stronie
    float ymm;			// wysokosc ogloszenia
    float xmm;			// szerokosc ogloszenia
    CDrawDoc* doc;		// aktywny dokument

    bool CompX(int* sizex, int* s_x) const;	// liczy szerokosc ogloszenia i liczbe kolumn strony
    bool CompY(int* sizey, int* s_y) const;	// liczy wysokosc ogloszenia i liczbe wierszy strony
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
private:
    BOOL bCalcMode;                //tryb kalkulatora czy rêczny
    static const double TOLERANCE; // wzgledna tolerancja rozmiaru
    CComboBox m_kratycombo;
    CString m_sizex;
    CString m_sizey;
    CString m_modelid;
    CString m_kra_sym;
    int m_x;
    int m_y;
    int m_lightx;
    int m_lighty;
    int m_userdef_sizex;
    int m_userdef_sizey;
    CEdit m_wynik;
    double stronax, stronay;
    void Calculate();
    afx_msg void OnModeSwitch();
};
