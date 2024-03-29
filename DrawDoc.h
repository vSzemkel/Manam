// drawdoc.h : interface of the CDrawDoc class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1993 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#pragma once

#define A4 (5 * theApp.activeDoc->m_pagerow_size)
#define A3 (10 * theApp.activeDoc->m_pagerow_size)

class CDrawAdd;
class CDrawObj;
class CDrawOpis;
class CDrawPage;
class CDrawView;
class CQueView;

struct CRozm;

class CDrawDoc final : public COleDocument
{
    DECLARE_DYNCREATE(CDrawDoc)

    // public: declared in macro
    CDrawDoc();

    friend class CManODPNET;
    friend class CDrawDocDbReader;
    friend class CDrawDocDbWriter;

    static std::vector<UINT> spoty;         // numer na li�cie kolor�w -> bazodanowy identyfikator koloru
    static std::vector<CString> kolory;     // dopuszczalne kolory og�osze� i stron
    static std::vector<CBrush*> brushe;     // cache p�dzli u�ywanych w widoku makiety

    static void IniKolorTable();
    static void DrawPageCross(CDC* pDC);
    static int ValidKolor(const CString& k) noexcept;
    static int GetIdxfromSpotID(UINT spot_id) noexcept;
    static CBrush* GetSpotBrush(int i) noexcept;
    static CString XmlReadText(IXmlReader* reader);
    static inline const TCHAR asDocTypeExt[][5] = {_T(".DB"), _T(".LIB"), _T(".GRB")}; // rozszerzenie poszczeglnych typ�w dokumentow opisanych w eDocType

    ~CDrawDoc() override;
    BOOL OnNewDocument() override;
    BOOL OnOpenDocument(LPCTSTR pszPathName) override;
    BOOL SaveModified() override;
    HMENU GetDefaultMenu() override;        // dla grzbietu zmien menu
    void OnCloseDocument() override;
    void Serialize(CArchive& ar) override;
    void SetModifiedFlag(BOOL modified = TRUE) override;

    template<class T = CDrawView> T* GetPanelView() const noexcept;
    const CSize& GetSize() const noexcept { return m_size; }
    float GetDrobneH();
    void ComputeCanvasSize();
    int AddsCount() const noexcept;
    int GetIPage(int n) const noexcept;
    int GetIPage(CDrawPage* pPage) const noexcept;
    int Nr2NrPorz(const TCHAR* s) const noexcept;   // dla numeru na paginie liczy numer porz�dkowy

// Operations
    //dotyczy CDrawObj - obiektow i add ogloszen
    void OnAsideAdds();
    void AsideAdds();                          // ustawia z boku ogloszenia bez okreslonej strony
    CPoint GetAsideAddPos(bool opening) const; // wylicza lewy gorny rog kolejnego ogloszenia z boku makiety
    void ArrangeQue();                         // ustawie og�oszenia na rysunku kolejki
    void MakietujStrone(CDrawPage* pPage);     // makietowanie dla wskazanej strony
    void AddFind(long nrAtex, long nrSpacer, LPCTSTR nazwa); // wyszukiwanie og�osze�
    void DerivePages(CDrawPage* pPage);        // dziedziczenie stron

    void Draw(CDC* pDC, CDrawView* pView);
    void DrawQue(CDC* pDC);
    void Print(CDC* pDC);
    void PrintPage(CDC* pDC, CDrawPage* pPage);
    void Add(CDrawObj* pObj);
    void AddQue(CDrawAdd* pObj);
    void RemoveQue(CDrawAdd* pObj);
    void RemoveFromHead(int n);
    void RemoveFromTail(int n);
    void Remove(CDrawObj* pObj);
    int GetAdPosition(const CDrawAdd* pAdd) const;
    CDrawObj* ObjectAt(const CPoint& point) const;
    CDrawAdd* ObjectAtQue(const CPoint& point) const;
    CDrawAdd* FindAddAt(int i) const; // szuka ogloszenia na pozycji i wsrod ogl - opisy sie nie numeruje

    void SelectAdd(CDrawAdd* pObj, bool multiselect = false) const;

    int ComputePageOrderNr(const CRect& position) const;           // oblicza numer porz�dkowy strony na podstawie po�o�enia
    void SetPageRectFromOrd(CDrawPage* pObj, uint32_t iOrd) const; // pozycjonuje stron� na podstawie numeru porz�dkowego
    void MoveBlockOfPages(int iSrcOrd, int iDstOrd, int iCnt);     // przesuwa ci�g�y blok stron w obr�bie makiety

    uint32_t AddPage(CDrawPage* pObj);        // na koncu, zwieksza pamiec na tablice
    void AddPageAt(int idx, CDrawPage* pObj); // pod zadanym idx
    void RemovePage(CDrawPage* pObj);
    CDrawPage* PageAt(const CPoint& point) const;
    CDrawPage* GetPage(int n) const;

    // inne akcje menu -OGLOSZENIA
    void OnImport(bool fromDB);
    void OnImportPlus(bool fromDB);
    void OnFileImportPlus();
    void OnFileImportMinus();
    void OnDBImportPlus();
    void OnDBImportMinus();
    void OnDrawOpcje();
    bool Import(bool check_exist);
    bool CreateAdd(LPTSTR adBuf, TCHAR sepChar, CPoint& pos, bool check_exist);
    bool DBImport(bool synchronize = false);
    CDrawAdd* AddExists(long nr) const;
    CDrawAdd* PubXXExists(int pub_xx) const;
    CDrawAdd* DBCreateAdd(const CString& roz, const CString& nazwa, long nr, UINT kolor, CString& warunki, const CString& uwagi, const CString& krt, CPoint& pos);

    void OnExport();
    void SetSpotKolor(UINT idx_m_spot_makiety, UINT idx_Spot_Kolor);

    void OnNumberPages();
    void NumberPages();

    void OnFileInfo();
    void OnFileDrzewo();
    void PrintInfo(CDC* pDC, int max_n, int wspol_na_str);

    void OnAdd4Pages();
    bool Add4Pages();
    bool AddDrz4Pages(LPCTSTR ile_kolumn = _T("4"));

    bool DBOpenDoc(TCHAR* makieta = nullptr);
    void OnDBSave();
    void OnDBSaveAs();
    void DBSaveAs(bool isSaveAs);
    void OnDBDelete();

    int DBReadSpot(int n);
    void ZmianaSpotow(int n);
    void ChangeCaptions(int old_id_drw, int new_id_drw);

    void IniRozm();
    const CRozm* GetCRozm(int s_x, int s_y, int typ_xx = 0);

#ifdef _DEBUG
    void AssertValid() const override;
    void Dump(CDumpContext& dc) const override;
#endif

    std::vector<UINT> m_spot_makiety;  // lista uzywanych kolorow spotowych
    std::vector<CDrawObj*>  m_objects; // lista og�osze� i opis�w nale��cych do dokumentu, og�oszenia przed opisami
    std::vector<CDrawPage*> m_pages;   // uporz�dkowana kolekcja stron dokumentu; ostatnia strona ma index 0
    std::vector<CDrawAdd*>  m_addsque; // lista og�osze� czekaj�cych w kolejce makiety
    std::vector<delobj_t>   m_del_obj; // obiekty usuni�te z dokumentu, do usuni�cia w bazie
    std::vector<CRozm>      m_rozm;    // tablica rozmiar�w krat i format�w niestandardowych dokumentu
    CFont m_pagefont;      // naglowki stron
    CFont m_addfont;       // numery ogloszen
    CString gazeta;        // tytul i mutacja - ust przy wyborze dzewa
    CString data;          // ust przy save'owaniu
    CString opis;          // ustalana w Rejkodzie dzienna nazwa produktu
    CString dayws;         // do przekazywania parametr�w w web serwis�w
    CString daydir;        // do generowania ps z katalog�w dniowych
    CString docmutred;     // lista dopuszczalnych mutacji redakcyjnych
    CString sOpiServerUrl; // adres serwera OPI, do kt�rego b�d� wysy�ane kolumny
    CString prowadzacy1;   // prowadzacy w metryce makiety
    CString prowadzacy2;   // drugi prowadzacy
    CString sekretarz;     // dla makiet bibliotecznych zawiera opis
    CString symWydawcy;    // dwuliterowy symbol dzia�u zsy�aj�cego
    int m_mak_xx{-1};      // ident makiety
    int id_drw{-2};        // ident drzewa
    uint16_t m_pagerow_size;                // ile stron rysuje si� w wierszu
    DocType iDocType{DocType::makieta};     // typ dokumentu na podstawie eDocType
    ToolbarMode swCZV{ToolbarMode::normal}; // prze��cznik widoku: 0==standard; 1==czas_obow; 2==studio;
    BYTE ovEPS : 1;        // nadpisuj EPS'y przy generacji
    BYTE isRO  : 1;        // otwarty tylko do odczytu
    BYTE isSIG : 1;        // makieta podpisana przez kierownika 
    BYTE isRED : 1;        // czy pokazuj� si� nag��wki redakcyjne
    BYTE isACD : 1;        // czy wczytano ju� dane o deadlinach dla czasopism

  protected:
    CSize m_size;

    // Generated message map functions
    //{{AFX_MSG(CDrawDoc)
    afx_msg void OnFileSaveAs();
    afx_msg void OnVuMakietowanie();
    afx_msg void OnVuCkMakietowanie();
    afx_msg void OnAddFind();
    afx_msg void OnAddSynchronize();
    afx_msg void OnEditFindNext();
    afx_msg void OnUpdateEditFindNext(CCmdUI* pCmdUI);
    afx_msg void OnFileSave();
    afx_msg void OnDisableMenuRO(CCmdUI* pCmdUI);
    afx_msg void OnDisableGrbNotSaved(CCmdUI* pCmdUI);
    afx_msg void OnSyncpow();
    afx_msg void OnUpdateSyncpow(CCmdUI* pCmdUI);
    afx_msg void OnDelremarks();
    afx_msg void OnCheckrep();
    afx_msg void OnDisableDB(CCmdUI* pCmdUI);
    afx_msg void OnEpsdate();
    afx_msg void OnUpdateEpsdata(CCmdUI* pCmdUI);
    afx_msg void OnPagederv();
    afx_msg void OnInsertGrzbiet();
    afx_msg void OnSyncDrv();
    afx_msg void OnSetPagina();
    afx_msg void OnSetDea();
    afx_msg void OnUpdateSetDea(CCmdUI* pCmdUI);
    afx_msg void OnShowTime();
    afx_msg void OnShowAcDeadline();
    afx_msg void OnUpdateShowTime(CCmdUI* pCmdUI);
    afx_msg void OnUpdateShowAcDeadline(CCmdUI* pCmdUI);
    afx_msg void OnKratCalc();
    afx_msg void OnAccGrb();
    afx_msg void OnChangeCaptions();
    afx_msg void OnChangeColsPerRow();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    CMenu m_grzbietMenu;
    CTime dataZamkniecia;
    CString lastSearchNazwa;
    float drobneH{0};                                // wysoko�� drobnych na stronie
    long lastSearchNrAtex;
    long lastSearchNrSpacer;
    int findNextInd;

    std::array<UINT,4> ModCount() const;             // raport ilo�ci modu��w poszczeg�lnych typ�w
    float PowAdd2Mod(bool queryQue) const;
    void AdvanceAsidePos(CPoint& p) const;           // wylicza polozenie kolejnego nastepnego ogloszenia z boku
    void SetTitleAndMru(bool addRecentFiles = true); // wywo�uje base->SetTitle przekazuj�c odpowiednie parametry i opcjonalnie dodaje do MRU
    bool MoveOpisAfterPage(const CRect& rFrom, const CRect& rTo);
    CMainFrame* GetMainWnd() const { return reinterpret_cast<CMainFrame*>(AfxGetMainWnd()); }
};

/////////////////////////////////////////////////////////////////////////////

template<class T> T* CDrawDoc::GetPanelView() const noexcept
{
    POSITION pos = GetFirstViewPosition();
    while (pos != nullptr)
        if (auto view = dynamic_cast<T*>(GetNextView(pos)))
            return view;

    return nullptr;
}
