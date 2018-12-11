
#pragma once

#include "DrawObj.h"

class CFlag;
class CDrawAdd;
class CDrawPage;
class CGenEpsInfoDlg;

struct GENEPSARG
{
    CManFormat format;
    bool bSignAll;
    bool bIsPreview;
    bool bDoKorekty;
    WORD iChannelId;
    TCHAR* cBigBuf;
    CDrawPage* pPage;
    CGenEpsInfoDlg* pDlg;
    HANDLE hCompletedEvent;
};
using PGENEPSARG = GENEPSARG *;

struct CKrataNiebazowa final
{
    explicit CKrataNiebazowa(int s_x, int s_y, CFlag&& space, CFlag&& locked, CFlag&& red) noexcept
        : m_szpalt_x(s_x), m_szpalt_y(s_y), m_space(std::forward<CFlag>(space)), m_space_locked(std::forward<CFlag>(locked)), m_space_red(std::forward<CFlag>(red))
    {
    };
    int m_szpalt_x;
    int m_szpalt_y;
    CFlag m_space;
    CFlag m_space_locked;
    CFlag m_space_red;
};

class CDrawPage final : public CDrawObj
{
    DECLARE_SERIAL(CDrawPage);

// public: declared in macro
    CDrawPage() noexcept {}; // can't be =default
    CDrawPage(const CRect& position) noexcept;
    ~CDrawPage() override;

    // Overrides
    void Draw(CDC* pDC) override;
    void Print(CDC* pDC) override;
    void Serialize(CArchive& ar) override;
    void UpdateInfo() override;
    void DrawKolor(CDC* pDC, const CRect& pos) const override;
    void MoveTo(const CRect& position, CDrawView* pView = nullptr) override;
    BOOL OnOpen(CDrawView* pView) override;
    CDrawObj* Clone(CDrawDoc* pDoc) const override;

    // Members
    int id_str;
    int szpalt_x;
    int szpalt_y;
    union {
        struct {
            short pagina_type;  // typ numeracji normalna:1, rzymska:2
            short pagina;       // numer strony na paginie
        };
        int nr;    // numer strony
    };
    int prn_mak_xx;     // identyfikator makiety prn
    int wyd_xx;         // identyfikator wydawcy strony
    BYTE m_typ_pary;    // czy strona wchodzi do sztucznej rozkladowki
    CString caption;    // widoczny naglowek, redakcyjny lub cennikowy
    CString caption_alt;// alternatywny naglowek, cennikowy lub redakcyjny
    CString name;       // logiczna strony z atexa ==sciezka ale zmieniamy  str_log np RED/PRG/PON user poprwia tylko pon
    CString mutred;     // alternatywne mutacje redakcyjne strony
    CFlag space;        // bitowa maska zajêtosci
    CFlag space_locked; // bitowa maska blokady
    CFlag space_red;    // bitowa maska powierzchni redakcyjnej
    BOOL niemakietuj;   // blokada makietowania

    std::vector<CDrawAdd*> m_adds;	// og³oszenia przypisane do danej strony
    std::vector<CKrataNiebazowa> m_kraty_niebazowe;	// kraty niebazowe

    int m_mutczas;			// numer mutacji czasowej (tylko grzbiet)
    long m_drukarnie;		// flaga bitowa drukarni
    CTime m_deadline;		// najwczesniejszy deadline
    CString m_dervinfo;		// informacja o dziedziczeniu
    CString f5_errInfo;		// komunikat o b³êdzie dotycz¹cym materia³u, wygenerowany przez funkcjê F5
    CStringA sBoundingBox;	// postscriptowy BB
    DervType m_dervlvl;		// poziom dziedziczenia

    CTime m_ac_red;			// deadline redakcyjny dla czasopism
    CTime m_ac_fot;			// deadline na zdjêcia dla czasopism
    CTime m_ac_kol;			// deadline na kolumnê dla czasopism

    // Implementation
    void DrawGrid(CDC* pDC);
    void DrawReserved(CDC* pDC);
    void DrawDeadline(CDC* pDC, const CRect& pos) const;
    void DrawAcDeadline(CDC* pDC, const CRect& pos) const;

    void SetNr(int i);
    void AddAdd(CDrawAdd* pAdd);
    void RemoveAdd(CDrawAdd* pAdd, bool removeFromAdds = true);
    void SetSpace(const CDrawAdd* pObj);
    void SetSpotKolor(UINT spot_kolor);
    void SetBaseKrata(int s_x, int s_y, bool refresh = true);
    void RealizeSpace(const CDrawAdd* pObj);
    void ChangeMark(size_t module, SpaceMode mode);
    void ChangeCaption(bool iscaption, const CString& cap);
    void DBChangeName(int id_drw);
    std::vector<int> CleanKraty(bool dbSave);

    bool FindSpace(CDrawAdd* pObj, int* px, int* py, int sx, int sy) const;
    bool CheckSpace(const CDrawAdd* pObj, int px, int py) const;
    bool CheckSpaceDiffKraty(const CDrawAdd* pObj, int x, int y) const;

    bool CheckRozmKrat();
    bool GenPDF(PGENEPSARG pArg);
    bool CheckSrcFile(PGENEPSARG pArg);
    bool GenEPS(PGENEPSARG pArg);
    bool GetDestName(PGENEPSARG pArg, const CString& sNum, CString& destName);
    CRect GetNormalizedModuleRect(size_t module) const;         // Prostok¹t ograniczaj¹cy modu³ o numerze porz¹dkowym module zakresu 0..sx*sy-1
    CString GetNrPaginy() const;

private:
    static CStringW GenerateGUIDString();                       // Konwertuje wygenerowany GIUD do stringu
    static void MoveMemFileContent(CFile& dst, CMemFile&& src); // Przenosi zawartoœæ src od poczatku do aktualnej pozycji

    CFlag GetReservedFlag();                                    // Pobiera flagê szarych modu³ów
    int  TiffHeader(CFile& dest, int dx, int dy, int bytesPerScanline) const noexcept;
    void BoundingBox(int* bx1, int* by1, int* bx2, int* by2) const noexcept;
    void Preview(PGENEPSARG pArg, CFile& dest, int bx1, int by1, int bx2, int by2) const noexcept;
    bool StaleElementy(PGENEPSARG pArg, CFile& handle);
    bool MovePageToOpiServer(PGENEPSARG pArg, CMemFile&& pOpiFile) const;
};

// CRozm
struct CRozm final
{
    CRozm() noexcept : CRozm(468, 575, 40, 34, 5, 6, 0, false) {}
    CRozm(int m_w, int m_h, int m_sw, int m_sh, BYTE m_szpalt_x, BYTE m_szpalt_y, int m_typ_xx, bool m_scale_it) noexcept :
        w(m_w),
        h(m_h),
        sw(m_sw),
        sh(m_sh),
        szpalt_x(m_szpalt_x),
        szpalt_y(m_szpalt_y),
        scale_it(m_scale_it),
        typ_xx(m_typ_xx)
    {
    }
    int typ_xx;
    WORD w;
    WORD h;
    WORD sw;
    WORD sh;
    BYTE szpalt_x;
    BYTE szpalt_y;
    bool scale_it;
};