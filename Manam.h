// *****************************************************
// Copyright (C) 1999-2018 Marcin Buchwald dla Agora SA
// All rights reserved.
// *****************************************************

#pragma once

#include "ManConst.h"
#include "ManSock.h"

#define vscale theApp.m_vscale
#define phight theApp.m_phight
#define pwidth theApp.m_pwidth
#define pmodulx theApp.m_pmodulx
#define pmoduly theApp.m_pmoduly

#define modulx CDrawObj::modx(szpalt_x)
#define moduly CDrawObj::mody(szpalt_y)

class CDrawDoc;

class CDrawApp : public CWinAppEx
{
    DECLARE_DYNAMIC(CDrawApp)
  public:
    CDrawApp();
    CDrawDoc* activeDoc;
    CString default_title, default_mut;
    ToolbarMode swCZV : 2;          // switch wskazuje typ widoku (zwyk�y==0,czasu obowi�zywania==1,studio==2)
    ToolbarMode initCZV : 2;        // switch u�ywany do okre�lania typu widoku przy otwieraniu
    BYTE isOpiMode : 1;             // generowanie plik�w w trybie OPI
    BYTE isRDBMS : 1;               // flaga wskazuje, czy program wsp�pracuje z baz� czy jest offline
    BYTE isOpen : 1;                // flaga ustawiana na otwarcie dokumentu
    BYTE OPENRO : 1;                // flaga u�ywana do otwierania pierwszej makiety w trybie RO
    BYTE makietujDoKupy : 1;        // flaga makietowania skomasowanego
    BYTE makietujAll : 1;           // czy makietowa� tylko og�oszenia poza makiet� czy wszystkie
    BYTE includeKratka : 1;         // do importu z .txt
    BYTE unQueing : 1;              // do przenoszenia og�osze� z kolejki na makiet�
    BYTE m_view_top : 5;            // podpis na g�rze
    BYTE m_view_bottom : 5;         // podpis na dole
    BYTE colsPerPage : 2;           // ile kolumn drukuje si� na stronie (1==50; 2==100)
    BYTE autoMark : 1;              // automatycznie zaznacz znalezione EPSy w gridzie
    BYTE showDeadline : 1;          // rysowanie deadline'u na stronach
    BYTE showAcDeadline : 1;        // rysowanie deadline'�w dniowych dla Czasopism na stronach
    BYTE isParalellGen : 1;         // r�wnoleg�e sprawdzanie/generowanie postscriptu
    BYTE ribbonStyle : 1;           // je�li =0 to klasyczne menu, wst��ka w przeiwnym przypadku
    BYTE grupa;                     // flaga z�o�ona z masek UserRole::dea,UserRole::red itd.
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
