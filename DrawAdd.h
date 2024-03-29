
#pragma once

class CDrawPage;
class CGenEpsInfoDlg;

struct GENEPSARG;

class CDrawAdd final : public CDrawObj
{
    DECLARE_SERIAL(CDrawAdd);

    // public: declared in macro
    CDrawAdd() noexcept;
    CDrawAdd(const CRect& position) noexcept;

    ~CDrawAdd() override;
    void Draw(CDC* pDC) override;
    void Print(CDC* pDC) override;
    void UpdateInfo() override;
    void Serialize(CArchive& ar) override;
    BOOL OnOpen(CDrawView* pView) override;

    CString nazwa;        // nazwa ogl
    CString logpage;      // logiczna strona - powiedzmy ze string war logicznych
    CString remarks;      // uwagi
    CString remarks_atex; // uwagi_atex
    CString wersja;       // scan,eps,el,klisze,odz
    CString czaskto;      // zawiera sformatowany czas obowi�zywania i login sprzedawcy
    CString kodModulu;    // zawiera nazw� pliku z materia�em �r�d�owym do produkcji, bez �cie�ki i rozszerzenia
    CString f5_errInfo;   // komunikat o b��dzie dotycz�cym materia�u, wygenerowany przez funkcj� F5
    CString lastAdnoUsed; // data ostatniego uzycia tego numeru zamowienia
    CString skad_ol;      // kod oddzia�y przyjmuj�cego, uzyskany przy F7
    CFlag space;          // odpowiada ksztaltowi
    CTime powtorka;       // data powt�rki
    CTime epsDate;        // data pliku
    long nreps;           // nr eps==adno z atexa
    long oldAdno;         // adno z kt�rego jest powt�rka
    int m_pub_xx{-1};
    int m_add_xx;         // FK_SPACER_AD
    int szpalt_x;
    int szpalt_y;
    int posx;
    int posy;
    int sizex;
    int sizey;
    int typ_xx;             // FK_TYP_OGLOSZENIA
    int nag_xx{1};          // FK_NAGLOWEK_OGLOSZENIA, identyfikator g�rnego nag��wka redakcyjnego
    int fizpage;            // fizpage & PaginaType::roman == PaginaType::roman to jest rzymska paginacja
    int txtposx;
    int txtposy;
    int precelWertexCnt{0}; // ilo�� wierzcho�k�w precla, 0 jesli precelWertexCnt == 4
    int precelRingCnt{0};   // ilo�� obwodnic precla
    int iFileId{0};         // identyfikator materia�u graficznego wykorzytywany przy F4
    struct
    {
        BYTE epsok : 2;          // akceptacja sprzeda�y (0-NIE,1-TAK,2-NIE WIEM)
        BYTE showeps : 1;        // kolor gridu (F5 - bialy, zolty)
        BYTE locked : 1;         // blokada miejsca
        BYTE reserv : 1;         // flaga rezerwacji
        BYTE weryf : 1;          // weryfikacja produkcyjna
        BYTE zagroz : 2;         // zagro�ona emisja
        BYTE isok : 2;           // spacer-status
        BYTE derived : 1;        // flaga dziedziczenia
        BYTE reksbtl : 1;        // czy pod og�oszeniem w podpisie wydrukowa� napis REKLAMA
        BYTE digital : 1;        // czy og�oszenie ma tre�� cyfrow�
        StudioStatus studio : 3; // status dla studia
    } flags;
    struct
    {               // BANK OG�OSZE�
        WORD insid; // insertionid
        WORD n;     // ile opublikowa�
        WORD k;     // w ilu wydaniach
    } bank;
    BYTE spad_flag : 4; // flaga mapuj�ca kraw�dzie do wylewu na spad

    static void PrintPadlock(CDC* pDC, const CRect& rect);
    static bool BBoxFromFile(GENEPSARG* pArg, CFile& handle, float* x1, float* y1, float* x2, float* y2);
    static bool LocatePreview(CFile& fEps, unsigned long* lOffset, unsigned long* lSize);
    static bool CopyNewFile(const CString& srcPath, const CString& dstPath);
    static bool EpsFromATEX(const CString& num, const CString& dstPath);
    static bool EmbedEpsFile(GENEPSARG* pArg, CFile& dstFile, const CString& srcPath);

    void DrawTxt(CDC* pDC, const CRect& rect, LPCTSTR tx, bool top) const;
    void DrawDesc(CDC* pDC, const CRect& rect) const; // vu : rysuje opis tekstowy og�oszenia
    void DrawPadlock(CDC* pDC, const CRect& rect) const;
    CString PrepareBuf(const TCHAR* ch) const;

    CFlag GetPlacementFlag() const;               // mapa modu��w zaj�tych na stronie przez to og�oszenie
    CFlag GetPlacementFlag(int px, int py) const; // mapa modu��w zaj�tych na stronie przez to og�oszenie, gdyby mia�o wsp�rz�dne (posx, posy)
    void Lock();
    void SetPosition(CRect* m_pos, CDrawPage* pPage);
    void SetPosition(int fizp, int px, int py, int sx, int sy);
    void SetSpaceSize(int sx, int sy); // og�oszenie w ustalonej kracie zmienia rozmiar
    void SetSpaceAndPosition(int fizp, int px, int py);
    void MoveWithPage(const CRect& position, CDrawView* pView = nullptr);
    void InitPrecel(const CString& sPrecelFlag);

    void SetLogpage(const CString& m_op_zew, const CString& m_sekcja, const CString& m_op_sekcji, int m_nr_w_sekcji, const CString& m_PL, const CString& m_op_PL, int m_nr_PL, CString& m_poz_na_str); //vu
    void ParseLogpage(TCHAR* op_zew, TCHAR* sekcja, TCHAR* op_sekcji, int* nr_sek, TCHAR* pl, TCHAR* op_pl, int* nr_pl, TCHAR* poz_na_str = nullptr);                                                  // vu
    int CkPageLocation(int nr_porz);                                                                                                                                                                   // czy og�oszenie moze sta� na stronie numer nr_porz
    void SetEstPagePos(const TCHAR* description, CRect* vRect, CDrawPage* pPage);
    bool SetPagePosition(CRect* pRect, CDrawPage* pPage);

    bool GetProdInfo(GENEPSARG* pArg, TCHAR* cKolor, float* bx1, float* by1, float* bx2, float* by2, int* ileMat); // szuka danych w bazie, je�li nie ma, to otwiera i przeszukuje plik
    bool CheckSrcFile(GENEPSARG* pArg);
    bool RewriteEps(GENEPSARG* pArg, CFile& dest);
    bool RewriteDrob(GENEPSARG* pArg, CFile& dest);
    CString FindZajawka(CString& root, const CString& ext) const;
    CString EpsName(CManFormat format, bool copyOldEPS, bool bModifTest = false);
    void Preview(GENEPSARG* pArg, int x, int y, int scanlinesCount, int bytesPerScanline) const;
    void SetDotM(bool setFlag); // parsuje wersj� i ustawia lub cofa .m

  private:
    static constexpr int ciMaxRings = 4;     // maksymalna ilo�� obwodnic precla
    std::unique_ptr<CPoint[]> aPrecelWertex; // tablica wierzcho�k�w precla
    CString m_precel_flag;                   // flaga precla, przechowywana po to, by niepotrzebnie nie liczy� obwodnic po OnOpen
    int* aRingWertexCnt;                     // tablica ilo�ci wierzcho�k�w na poszczeg�lnych obwodnicach, w�asciciel pami�ci: aPrecelWertex
    bool SetStrictDescPos(LPCTSTR description, CRect* pRect, CDrawPage* pPage);
    bool PtOnRing(CPoint p) const;            // stwierdza, czy kraw�d� (px,py)->(px+1,py) jest na dotychczas znalezionej obwodnicy precla
    int FindRing(CPoint p0, bool bOuterRing); // znajduje obwodnic� precla rozpoczynaj�c� si� od p0 w kierunku E, zwraca liczb� wierzcho�k�w
};

struct SpadInfo
{
    enum SpadOffset : uint8_t { bleed_right = 0, just_center, bleed_left };
    SpadOffset adjust_x; // przesuni�cie punktu zaczepienia od lewego dolnego rogu
    SpadOffset adjust_y;
    bool scale_x; // flaga skalowania dla wymiaru
    bool scale_y;
};
