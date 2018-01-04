// *****************************************************
// Copyright (C) 1999-2017 Marcin Buchwald dla Agora SA
// All rights reserved.
// *****************************************************

#pragma once

#include "ManSock.h"

#define vscale theApp.m_vscale
#define phight theApp.m_phight
#define pwidth theApp.m_pwidth
#define pmodulx theApp.m_pmodulx
#define pmoduly theApp.m_pmoduly

#define modulx CDrawObj::modx(szpalt_x)
#define moduly CDrawObj::mody(szpalt_y)
#define B2MB(x) (int)nearbyintf((float)x / 1048576)

constexpr int CLIENT_SCALE = 20;
constexpr int PRINT_VOFFSET = -10;
constexpr int MIN_COLSPERROW = 10;
constexpr int pszpalt_x = 5;               // liczba kolumn w kracie domy�lnej
constexpr int pszpalt_y = 6;               // liczba wierszy w kracie domy�lnej
constexpr int pmodcnt = 30;                // pszpalt_x * pszpalt_y
constexpr int DLGMSG_MAX_LEN = 3000;       // timeout na uzyskanie po��czenia z puli wyra�ony w milisekundach
constexpr int MIN_VALID_ADNO = 10'000'000; // minimalny prawid�owy numer ADNO
constexpr int MANAM_DEFAULT_PORT = 2501;   // domy�lny numer portu u�ywany przez aplikacj�
constexpr auto MAX_STUDIO_PATH = (_MAX_PATH - 12);
constexpr auto c_formatDaty = _T("%02d/%02d/%04d");
constexpr auto c_formatCzasu = _T("%02d/%02d/%04d %02d:%02d");
constexpr auto c_ctimeDataWs = _T("%Y%m%d"); // domy�lny format dla parametr�w web serwis�w
constexpr auto c_ctimeData = _T("%d/%m/%Y");
constexpr auto c_ctimeCzas = _T("%d/%m/%Y %H:%M");
constexpr __time64_t ONEDAY = 86400;         // liczba sekund w dobie
constexpr __time64_t POWTSEED_0 = 946594800; // 31/12/1999 00:00 data, w stosunku do kt�rej s� obliczane powt�rki
constexpr __time64_t POWTSEED_1 = 946598400; // 31/12/1999 01:00 data, w stosunku do kt�rej s� obliczane powt�rki plus godzina
// kolory
constexpr int c_brak = 1;
constexpr int c_spot = 2;
constexpr int c_full = 4;
constexpr auto BRAK = _T("(brak)");
constexpr auto FULL = _T("(full)");
constexpr COLORREF BIALY = RGB(255, 255, 255);
// numeracja - odpowiada PK s�ownika TYP_NUMERACJI z bazy
constexpr int c_normal = 1;
constexpr int c_rzym = 2;
// typy obiektow
constexpr int c_page = 0;
constexpr int c_add = 1;
constexpr int c_opis = 2;
constexpr int c_addque = 3;
constexpr int c_page_lib = 4;
constexpr int c_opis_lib = 5;
// grupy
constexpr int R_DEA = 1;
constexpr int R_RED = 2;
constexpr int R_STU = 4;
constexpr int R_KIE = 8;
constexpr int R_ADM = 16;
constexpr int R_MAS = 32;
// status studia
constexpr int STUDIO_BRAK = 0;
constexpr int STUDIO_JEST = 1;
constexpr int STUDIO_NOWY = 2;
constexpr int STUDIO_ACC = 3;
constexpr int STUDIO_OK = 4;
constexpr int STUDIO_SEND = 5;
constexpr int STUDIO_FILTR = 6;
constexpr int STUDIO_MSG = 7;
// gen eps
constexpr float mm2pkt = 0.2835f; // 1mm = 2.835pt
constexpr float pkt2mm = 0.3527f; // 1pt = 0.3527mm
constexpr float pkt_10m = 2.835f;
constexpr int podpisH = 6; // 2.1162 [mm]
constexpr int preview_offset = 30;
// format materialu
constexpr int F_EPS = 0;
constexpr int F_PS = 1;
constexpr int F_PDF = 2;
// dziedziczenie
constexpr int DERV_NONE = 0;
constexpr int DERV_ADDS = 1;
constexpr int DERV_TMPL = 2;
constexpr int DERV_FIXD = 3;
constexpr int DERV_PROH = 4;
constexpr int DERV_DRUK = 5;
constexpr int DERV_COLO = 6;
constexpr auto DERV_TMPL_WER = "$c";
constexpr auto OPI_TAG = "%%MANAM-OPI ";
constexpr auto APP_NAME = _T("Manam");

constexpr int bigSize = 0x8000;   // 32kB
constexpr size_t n_size = 0x7FFF; // (bigSize-1)

class CDrawDoc;

class CDrawApp : public CWinAppEx
{
    DECLARE_DYNAMIC(CDrawApp)
  public:
    CDrawApp();
    CDrawDoc* activeDoc;
    CString default_title, default_mut;
    BYTE isOpiMode : 1;             // generowanie plik�w w trybie OPI
    BYTE isRDBMS : 1;               // flaga wskazuje, czy program wsp�pracuje z baz� czy jest offline
    BYTE isOpen : 1;                // flaga ustawiana na otwarcie dokumentu
    BYTE OPENRO : 1;                // flaga u�ywana do otwierania pierwszej makiety w trybie RO
    BYTE makietujDoKupy : 1;        // flaga makietowania skomasowanego
    BYTE makietujAll : 1;           // czy makietowa� tylko og�oszenia poza makiet� czy wszystkie
    BYTE includeKratka : 1;         // do importu z .txt
    BYTE unQueing : 1;              // do przenoszenia og�osze� z kolejki na makiet�
    BYTE swCZV : 2;                 // switch wskazuje typ widoku (zwyk�y==0,czasu obowi�zywania==1,studio==2)
    BYTE OPENCZV : 2;               // switch u�ywany do okre�lania typu widoku przy otwieraniu
    BYTE m_view_top : 5;            // podpis na g�rze
    BYTE m_view_bottom : 5;         // podpis na dole
    BYTE colsPerPage : 2;           // ile kolumn drukuje si� na stronie (1==50; 2==100)
    BYTE autoMark : 1;              // automatycznie zaznacz znalezione EPSy w gridzie
    BYTE showDeadline : 1;          // rysowanie deadline'u na stronach
    BYTE showAcDeadline : 1;        // rysowanie deadline'�w dniowych dla Czasopism na stronach
    BYTE isParalellGen : 1;         // r�wnoleg�e sprawdzanie/generowanie postscriptu
    BYTE ribbonStyle : 1;           // je�li =0 to klasyczne menu, wst��ka w przeiwnym przypadku
    BYTE grupa;                     // flaga z�o�ona z masek R_DEA,R_RED itd.
    CString sManamEpsName;          // nazwa pliku z definicj� szablonu postscriptowego do produkcji stron
    CString m_tnsname;              // identyfikator bazy danych u�ywany przez connection pooling
    CString m_login;                // login po��czenia do bazy danych u�ywane przez connection pooling
    CString m_passwd;               // has�o po��czenia do bazy danych u�ywane przez connection pooling
    CString m_app_version;          // wersja programu w postaci "Manam x.x.x.x"
    CString m_local_ip;             // lokalny adres IP s�u��cy do autoryzacji wywo�a� web serwis�w
    std::vector<CString> drukarnie; // lista drukarni do dialogu strony
    std::vector<CString> wydawcy;   // lista wydawc�w do dialogu strony w formacie XXXwydawca
    std::vector<CString> zsylajacy; // lista zsy�aj�cych do dialogu metryka makiety
    std::vector<CString> kraty;
    std::vector<int> szpalt_xarr;
    std::vector<int> szpalt_yarr;
    CInternetSession m_InternetSession; // sesja umo�liwiaj�ca interakcj� z serwisami HTTP

    TCHAR* bigBuf; // bufor znakowy do przepisywania plik�w
    int m_vscale;
    int m_phight;
    int m_pwidth;
    int m_pmodulx;
    int m_pmoduly;
    int m_initZoom; // pocz�tkowy zoom widoku
    UINT m_manam_port;
    CManSock m_sock;
    // Overrides
    void AddToRecentFileList(LPCTSTR lpszPathName) override;
    BOOL InitInstance() override;
    int ExitInstance() override;

    // Implementation
    static CTime ShortDateToCTime(const CString& sData);              // konwertuje dat� w formacie dd/mm/rrrr do CTime
    static void CTimeToShortDate(const CTime& tData, CString& sData); // konwertuje dat� przekazan� jako CTime do formatu dd/mm/rrrr
    static BOOL OpenWebBrowser(const TCHAR* sUrl);                    // otwiera przegladarke Internet Explorer i przekazuje jej sUrl
    static void OpenWebBrowser(size_t service, const TCHAR* sUrl);    // otwiera przegladarke Internet Explorer i przekazuje jej sUrl
    static void SetErrorMessage(LPTSTR lpBuffer);

    BOOL ConnecttoDB();
    BOOL TryUpgradeImage() const; // proba upgrade'u wersji programu na podstawie obrazu zapisanego w bazie
    void FromIniFile();
    void SetScale(int scale);
    void InitKratyDrukarnie();
    void FileRefresh(CDrawDoc* refreshDoc = nullptr); // odswieza jeden lub wszystkie otwarte jesli nullptr
    void FillKrataCombo(CComboBox& combo, int szpalt_x = pszpalt_x, int szpalt_y = pszpalt_y);
    std::unique_ptr<CHttpFile> OpenURL(size_t service, const CString& sUrl); // pobiera tre�� z URL, kt�ry powinien by� typu http
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

extern CDrawApp NEAR theApp;
