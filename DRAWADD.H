
#pragma once

#include "drawobj.h"
#include "drawpage.h"
#include "flag.h"

class CGenEpsInfoDlg;
class CDrawPage;

class CDrawAdd final : public CDrawObj
{
protected:
    DECLARE_SERIAL(CDrawAdd);
    CDrawAdd();
public:
    CDrawAdd(const CRect *position);
    virtual ~CDrawAdd();
    virtual void Draw(CDC *pDC) override;
    virtual void Print(CDC *pDC) override;
    virtual void UpdateInfo() override;
    virtual void Serialize(CArchive& ar) override;
    virtual BOOL OnOpen(CDrawView *pView) override;

  public:
    int m_pub_xx;
    int m_add_xx;	// FK_SPACER_AD
    int szpalt_x;
    int szpalt_y;
    int typ_xx; // FK_TYP_OGLOSZENIA
    int nag_xx; // FK_NAGLOWEK_OGLOSZENIA, identyfikator górnego nag³ówka redakcyjnego
    long  nreps; //nr eps          ==adno z atexa
    long  oldAdno; //adno z którego jest powtórka
    CString nazwa; //nazwa ogl
    CString logpage;	//logiczna strona - powiedzmy ze string war logicznych
    CString remarks;	//uwagi
    CString remarks_atex;	//uwagi_atex
    CString wersja;		//scan,eps,el,klisze,odz
    CString czaskto;	//zawiera sformatowany czas obowi¹zywania i login sprzedawcy
    CString kodModulu;	//zawiera nazwê pliku z materia³em Ÿród³owym do produkcji, bez œcie¿ki i rozszerzenia
    CString f5_errInfo; //komunikat o b³êdzie dotycz¹cym materia³u, wygenerowany przez funkcjê F5
    int sizex;
    int sizey;
    int posx;
    int posy;
    int fizpage;     //fizpage &c_rzym == c_rzym to jest rzymska paginacja
    struct
    {
        BYTE epsok : 2;  // akceptacja sprzeda¿y (0-NIE,1-TAK,2-NIE WIEM)
        BYTE showeps : 1;// kolor gridu (F5 - bialy, zolty)
        BYTE locked : 1; // blokada miejsca
        BYTE reserv : 1; // flaga rezerwacji
        BYTE weryf : 1; // weryfikacja produkcyjna
        BYTE zagroz : 2; // zagro¿ona emisja
        BYTE isok : 2; // spacer-status
        BYTE studio : 3; // status dla studia
        BYTE derived : 1; // flaga dziedziczenia
        BYTE reksbtl : 1; // czy pod og³oszeniem w podpisie wydrukowaæ napis REKLAMA	
        BYTE digital : 1; // czy og³oszenie ma treœæ cyfrow¹
    } flags;
    struct
    {				// BANK OG£OSZEÑ
        WORD insid;			// insertionid
        WORD n;				// ile opublikowaæ
        WORD k;				// w ilu wydaniach
    } bank;
    int txtposx;
    int txtposy;
    int precelWertexCnt;	// iloœæ wierzcho³ków precla, 0 jesli precelWertexCnt == 4
    int precelRingCnt;		// iloœæ obwodnic precla
    int iFileId;			// identyfikator materia³u graficznego wykorzytywany przy F4
    BYTE spad_flag : 4;		// flaga mapuj¹ca krawêdzie do wylewu na spad
    CFlag space;			// odpowiada ksztaltowi      
    CString lastAdnoUsed;
    CString skad_ol;		// kod oddzia³y przyjmuj¹cego, uzyskany przy F7
    CTime powtorka;			// data powtórki
    CTime epsDate;			// data pliku

private:
    static const int ciMaxRings; // maksymalna iloœæ obwodnic precla
    CString m_precel_flag;  // flaga precla, przechowywana po to, by niepotrzebnie nie liczyæ obwodnic po OnOpen
    std::unique_ptr<CPoint[]> aPrecelWertex;	// tablica wierzcho³ków precla
    int *aRingWertexCnt;	// tablica iloœci wierzcho³ków na poszczególnych obwornicach
    BOOL SetStrictDescPos(TCHAR *description, CRect *vRect, CDrawPage *pPage);
    BOOL PtOnRing(CPoint p) const; // stwierdza, czy krawêdŸ (px,py)->(px+1,py) jest na dotychczas znalezionej obwodnicy precla
    int FindRing(CPoint p0, BOOL bOuterRing); // znajduje obwodnicê precla rozpoczynaj¹c¹ siê od p0 w kierunku E, zwraca liczbê wierzcho³ków

public:
    static void PrintPadlock(CDC *pDC, const CRect& rect);
    static BOOL BBoxFromFile(PGENEPSARG pArg, CFile *handle, float *x1, float *y1, float *x2, float *y2);
    static BOOL LocatePreview(CFile& fEps, unsigned long *lOffset, unsigned long *lSize);
    static BOOL CopyNewFile(const CString& srcPath, const CString& dstPath);
    static BOOL EpsFromATEX(const CString& num, const CString& dstPath);

    void DrawTx(CDC *pDC, const CRect& rect, LPCTSTR tx, BOOL top) const;
    void DrawDesc(CDC *pDC, const CRect& rect) const; // vu : rysuje opis tekstowy og³oszenia
    void DrawPadlock(CDC *pDC, const CRect& rect) const;
    CString PrepareBuf(TCHAR *ch) const;

    BOOL Lock();
    CFlag GetPlacementFlag() const;					// mapa modu³ów zajêtych na stronie przez to og³oszenie
    CFlag GetPlacementFlag(int px, int py) const;	// mapa modu³ów zajêtych na stronie przez to og³oszenie, gdyby mia³o wspó³rzêdne (posx, posy)
    void SetPosition(CRect *m_pos, CDrawPage *pPage);
    void SetPosition(int fizp, int px, int py, int sx, int sy);
    void SetSpaceSize(int sx, int sy);				// og³oszenie w ustalonej kracie zmienia rozmiar
    void SetSpaceAndPosition(int fizp, int px, int py);
    void MoveWithPage(const CRect& position, CDrawView *pView = nullptr);
    void InitPrecel(const CString& sPrecelFlag);

    void SetLogpage(CString& m_op_zew, CString& m_sekcja, CString& m_op_sekcji, int m_nr_w_sekcji, CString& m_PL, CString& m_op_PL, int m_nr_PL, CString& m_poz_na_str); //vu
    void ParseLogpage(TCHAR *op_zew, TCHAR *sekcja, TCHAR *op_sekcji, int *nr_sek, TCHAR *pl, TCHAR *op_pl, int *nr_pl, TCHAR *poz_na_str = nullptr); // vu
    int  CkPageLocation(int vFizPage); //vu : czy ogloszenie moze byc na stronie numer vFizPage
    void SetEstPagePos(TCHAR *description, CRect *vRect, CDrawPage *pPage);
    BOOL SetPagePosition(CRect *vRect, CDrawPage *vPage);

    BOOL GetProdInfo(PGENEPSARG pArg, TCHAR *cKolor, float *bx1, float *by1, float *bx2, float *by2, int *ileMat); // szuka danych w bazie, jeœli nie ma, to otwiera i przeszukuje plik
    BOOL CheckSrcFile(PGENEPSARG pArg);
    BOOL RewriteEps(PGENEPSARG pArg, CFile *dest);
    BOOL RewriteDrob(PGENEPSARG pArg, CFile *dest);
    CString FindZajawka(CString& root, const CString& ext) const;
    CString EpsName(int format, BOOL copyOldEPS, BOOL modifTest = FALSE);
    void Preview(PGENEPSARG pArg, int x, int y, int dy, int szer) const;
    void SetDotM(BOOL setFlag);		// parsuje wersjê i ustawia lub cofa .m
};

struct SpadInfo
{
    float fOffX;	// przesuniêcie punktu zaczepienia od lewego dolnego rogu, wyra¿one liczb¹ krawêdzi {0, 0.5, 1}
    float fOffY;
    BYTE  bScaX;	// flaga skalowania dla wymiaru
    BYTE  bScaY;
};
