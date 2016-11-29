
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
	virtual ~CAtexKrat();
// Implementation
public:
	BOOL isValid;	// flaga poprawnosci parsowania napisu w konstruktorze
	BOOL isToSmall;	// gêstoœæ obliczonej kraty przekracza DIM_LIMIT
	BOOL Compute(int* sizex, int* sizey, int* s_x, int* s_y);	// liczy rozmiary ogloszenia i krate strony
	static const int DIM_LIMIT;		// jak gestej kraty podstawowej szukamy
	static const int ADJUST_LEVEL;	// przez max ile mnozymy rozmiar podstawowy
	static const int KRATA_PASKOWA;	// od takiego wymiaru wysokoœci szerokoœæ jest ujednolicana na 1 szpalte
private:
	static const float TOLERANCE;	// wzgledna tolerancja rozmiaru
	float dy;			// wysokosc strony
	float xcol;			// ile szpalt na stronie zajmuje ogloszenie
	float szpalt_x;		// ile jest szpalt na stronie
	float ymm;			// wysokosc ogloszenia
	float xmm;			// szerokosc ogloszenia
	CDrawDoc *doc;		// aktywny dokument

	BOOL CompX(int* sizex, int* s_x) const;	// liczy szerokosc ogloszenia i liczbe kolumn strony
	BOOL CompY(int* sizey, int* s_y) const;	// liczy wysokosc ogloszenia i liczbe wierszy strony
};

// CKratCalc dialog

class CKratCalc : public CDialog
{
	DECLARE_DYNAMIC(CKratCalc)

public:
	CKratCalc(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CKratCalc();

// Dialog Data
	enum { IDD = IDD_KRATCALC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnDeltaposSpinx(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpiny(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelchangeKratka();
	afx_msg void OnDefineModelid();
private:
	BOOL bCalcMode;					//tryb kalkulatora czy rêczny
	static const double TOLERANCE;	//wzgledna tolerancja rozmiaru
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
//{{AFX_INSERT_LOCATION}}
