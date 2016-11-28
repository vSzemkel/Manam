// *****************************************************
// Copyright (C) 1999-2015 Marcin Buchwald dla Agora SA
// All rights reserved.
// *****************************************************

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH 
#endif

#pragma once

#include "resource.h"       // main symbols
#include "ManSock.h"

#define vscale		   theApp.m_vscale
#define phight		   theApp.m_phight
#define pwidth		   theApp.m_pwidth
#define pmodulx		   theApp.m_pmodulx
#define pmoduly		   theApp.m_pmoduly
#define MAX_STUDIO_PATH (_MAX_PATH - 12)

#define modulx		   CDrawObj::modx(szpalt_x)
#define moduly		   CDrawObj::mody(szpalt_y)
#define B2MB(x) (int)floor((((double)x) / 1048576) + 0.5)

static const int CLIENT_SCALE   = 20;
static const int PRINT_VOFFSET  = -10;
static const int MIN_COLSPERROW = 10;
static const int pszpalt_x      = 5;	// liczba kolumn w kracie domyœlnej
static const int pszpalt_y      = 6;	// liczba wierszy w kracie domyœlnej
static const int pmodcnt		= 30;   // pszpalt_x * pszpalt_y
static const int DLGMSG_MAX_LEN = 3000;	// timeout na uzyskanie po³¹czenia z puli wyra¿ony w milisekundach
static const int MIN_VALID_ADNO	= 10'000'000;	// minimalny prawid³owy numer ADNO
static const int MANAM_DEFAULT_PORT	= 2501; // domyœlny numer portu u¿ywany przez aplikacjê
static const TCHAR* c_formatDaty = _T("%02d/%02d/%04d");
static const TCHAR* c_formatCzasu = _T("%02d/%02d/%04d %02d:%02d");
static const TCHAR* c_ctimeData = _T("%d/%m/%Y");
static const TCHAR* c_ctimeCzas = _T("%d/%m/%Y %H:%M");
static const __time64_t ONEDAY = 86400; // liczba sekund w dobie
static const __time64_t POWTSEED_0 = 946594800; // 31/12/1999 00:00 data, w stosunku do której s¹ obliczane powtórki
static const __time64_t POWTSEED_1 = 946598400; // 31/12/1999 01:00 data, w stosunku do której s¹ obliczane powtórki plus godzina
// kolory
static const int c_brak = 1;
static const int c_spot = 2;
static const int c_full = 4; 
static const TCHAR* BRAK = _T("(brak)");
static const TCHAR* FULL = _T("(full)");
static const COLORREF BIALY = RGB(255, 255, 255);
// numeracja - odpowiada PK s³ownika TYP_NUMERACJI z bazy
static const int c_normal = 1;
static const int c_rzym	  = 2;
// typy obiektow
static const int c_page		= 0;
static const int c_add		= 1;
static const int c_opis		= 2; 
static const int c_addque	= 3;
static const int c_page_lib	= 4;
static const int c_opis_lib	= 5;
// grupy
static const int R_DEA = 1;
static const int R_RED = 2;
static const int R_STU = 4;
static const int R_KIE = 8;
static const int R_ADM = 16;
static const int R_MAS = 32;
// status studia
static const int STUDIO_BRAK  = 0;
static const int STUDIO_JEST  = 1;
static const int STUDIO_NOWY  = 2;
static const int STUDIO_ACC   = 3;
static const int STUDIO_OK    = 4;
static const int STUDIO_SEND  = 5;
static const int STUDIO_FILTR = 6;
static const int STUDIO_MSG   = 7;
// gen eps
static const double mm2pkt  = 0.2835;		// 1mm = 2.835pt
static const double pkt2mm	= 0.3527;		// 1pt = 0.3527mm
static const double pkt_10m	= 2.835;
static const int podpisH = 6;				// 2.1162 [mm]
static const int preview_offset = 30;
// format materialu
static const int F_EPS = 0;
static const int F_PS  = 1;
static const int F_PDF = 2;
// dziedziczenie
static const int DERV_NONE = 0;
static const int DERV_ADDS = 1;
static const int DERV_TMPL = 2;
static const int DERV_FIXD = 3;
static const int DERV_PROH = 4;
static const int DERV_DRUK = 5;
static const int DERV_COLO = 6;
#define DERV_TMPL_WER	"$c"
#define OPI_TAG			"%%MANAM-OPI "
#define APP_NAME		_T("Manam")

static const int bigSize = 0x8000;	// 32kB
static const size_t n_size = 0x7FFF;	// (bigSize-1)

class CDrawDoc;

class CDrawApp : public CWinAppEx
{
	DECLARE_DYNAMIC(CDrawApp)
public:
	CDrawApp();
	CDrawDoc *activeDoc;
	CString default_title, default_mut;
	BYTE isOpiMode : 1;	// generowanie plików w trybie OPI
	BYTE isRDBMS : 1;	// flaga wskazuje, czy program wspó³pracuje z baz¹ czy jest offline
	BYTE isOpen : 1;	// flaga ustawiana na otwarcie dokumentu
	BYTE OPENRO : 1;	// flaga u¿ywana do otwierania pierwszej makiety w trybie RO
	BYTE makietujDoKupy : 1;   // flaga makietowania skomasowanego
	BYTE makietujAll : 1;	// czy makietowaæ tylko og³oszenia poza makiet¹ czy wszystkie
	BYTE includeKratka : 1;	// do importu z .txt
	BYTE unQueing : 1;	// do przenoszenia og³oszeñ z kolejki na makietê
	BYTE swCZV : 2;	// switch wskazuje typ widoku (zwyk³y==0,czasu obowi¹zywania==1,studio==2)
	BYTE OPENCZV : 2;	// switch u¿ywany do okreœlania typu widoku przy otwieraniu
	BYTE m_view_top : 5;	// podpis na górze
	BYTE m_view_bottom : 5;	// podpis na dole
	BYTE colsPerPage : 2;	// ile kolumn drukuje siê na stronie (1==50; 2==100)
	BYTE autoMark : 1;	// automatycznie zaznacz znalezione EPSy w gridzie
	BYTE showDeadline : 1;	// rysowanie deadline'u na stronach
	BYTE showAcDeadline : 1;	// rysowanie deadline'ów dniowych dla Czasopism na stronach
	BYTE isParalellGen : 1;	// równoleg³e sprawdzanie/generowanie postscriptu 
	BYTE ribbonStyle : 1;   // jeœli =0 to klasyczne menu, wst¹¿ka w przeiwnym przypadku
	BYTE grupa;				// flaga z³o¿ona z masek R_DEA,R_RED itd.
	CString sManamEpsName;	// nazwa pliku z definicj¹ szablonu postscriptowego do produkcji stron
	CString m_tnsname;	// identyfikator bazy danych u¿ywany przez connection pooling
	CString m_login;	// login po³¹czenia do bazy danych u¿ywane przez connection pooling
	CString m_passwd;	// has³o po³¹czenia do bazy danych u¿ywane przez connection pooling
	CString m_app_version;	// wersja programu w postaci "Manam x.x.x.x"
	CString m_local_ip;	// lokalny adres IP s³u¿¹cy do autoryzacji wywo³añ web serwisów 
	std::vector<CString> drukarnie	;	// lista drukarni do dialogu strony
	std::vector<CString> wydawcy	;	// lista wydawców do dialogu strony w formacie XXXwydawca
	std::vector<CString> zsylajacy	;	// lista zsy³aj¹cych do dialogu metryka makiety
	std::vector<CString> kraty		;
	std::vector<int>	szpalt_xarr ;
	std::vector<int>	szpalt_yarr ;
	CInternetSession m_InternetSession; // sesja umo¿liwiaj¹ca interakcjê z serwisami HTTP

	TCHAR *bigBuf;				// bufor znakowy do przepisywania plików
	int m_vscale; 
	int m_phight;	
	int m_pwidth;	
	int m_pmodulx;
	int m_pmoduly;
	int m_initZoom;				// pocz¹tkowy zoom widoku
	UINT m_manam_port;
	CManSock m_sock;
// Overrides
	virtual void AddToRecentFileList(LPCTSTR lpszPathName) override;
	virtual BOOL InitInstance() override;
	virtual int ExitInstance() override;

// Implementation
	static CTime ShortDateToCTime(const CString& sData);				// konwertuje datê w formacie dd/mm/rrrr do CTime
	static void CTimeToShortDate(const CTime& tData, CString& sData);	// konwertuje datê przekazan¹ jako CTime do formatu dd/mm/rrrr

	BOOL ConnecttoDB();
	BOOL CDrawApp::OpenWebBrowser(const TCHAR* sUrl);		// otwiera przegladarke Internet Explorer i przekazuje jej sUrl
	BOOL TryUpgradeImage();									// proba upgrade'u wersji programu na podstawie obrazu zapisanego w bazie
	void InitKratyDrukarnie() ;
	void FromIniFile();
	void SetScale(int scale);
	void FileRefresh(CDrawDoc *refreshDoc = nullptr);		// odswieza jeden lub wszystkie otwarte jesli nullptr
	void FillKrataCombo(CComboBox& combo, int szpalt_x = pszpalt_x, int szpalt_y = pszpalt_y);
	std::unique_ptr<CHttpFile> OpenURL(size_t service, const CString& sUrl); // pobiera treœæ z URL, który powinien byæ typu http
protected:
	//{{AFX_MSG(CDrawApp)
	afx_msg void OnAppAbout();
	afx_msg void OnDBOpen();
	afx_msg void OnDisableMenu(CCmdUI* pCmdUI);
	afx_msg void OnLogin();
	afx_msg void OnUpdateLogin(CCmdUI* pCmdUI);
	afx_msg void OnDisableMenuRDBMS(CCmdUI* pCmdUI);
	afx_msg void OnFileDBOpenRO();
	afx_msg void OnFileNewBath();
	afx_msg void OnPasswd();
	afx_msg void OnAccess();
	afx_msg void OnUpdateAdmin(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStudio(CCmdUI* pCmdUI);
	afx_msg void OnNewuser();
	afx_msg void OnCaptions();
	afx_msg void OnUpdateCaptions(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolBarCombo(CCmdUI* pCmdUI);
	afx_msg void OnNewTitle();
	afx_msg void OnDaydirs();
	afx_msg void OnHelp();
	afx_msg void OnSendmsg();
	afx_msg void OnFileRefresh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	std::vector<CString> m_uriDict;
};
