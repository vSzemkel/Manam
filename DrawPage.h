
#pragma once

#include "DrawObj.h"

class CFlag;
class CDrawAdd;
class CDrawPage;
class CGenEpsInfoDlg;

enum class SpaceMode : uint8_t
{
    avail,
    redlock,
    spacelock,
};

typedef struct _GENEPSARG
{
    int  iChannelId;
    int  iThreadId;
    TCHAR *cBigBuf;
    BOOL bIsPRN;
    BOOL bIsPreview;
    BOOL bSignAll;
    BOOL bDoKorekty;
    BOOL bStatus;
    CDrawPage *pPage;
    CGenEpsInfoDlg *pDlg;
} GENEPSARG, *PGENEPSARG;

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
    CDrawPage() noexcept {};
    CDrawPage(const CRect& position) noexcept;
    virtual ~CDrawPage();

    // Overrides
    virtual void Draw(CDC *pDC) override;
    virtual void Print(CDC *pDC) override;
    virtual void Serialize(CArchive &ar) override;
    virtual void UpdateInfo() override;
    virtual void DrawKolor(CDC *pDC, const CRect &pos) const override;
    virtual void MoveTo(const CRect &positon, CDrawView *pView = nullptr) override;
    virtual BOOL OnOpen(CDrawView *pView) override;
    virtual CDrawObj *Clone(CDrawDoc *pDoc) const override;

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
    CFlag space;        // bitowa maska zaj�tosci
    CFlag space_locked; // bitowa maska blokady
    CFlag space_red;    // bitowa maska powierzchni redakcyjnej
    BOOL niemakietuj;   // blokada makietowania

    std::vector<CDrawAdd*> m_adds;	// og�oszenia przypisane do danej strony
    std::vector<CKrataNiebazowa> m_kraty_niebazowe;	// kraty niebazowe

    int m_dervlvl;			// poziom dziedziczenia
    int m_mutczas;			// numer mutacji czasowej (tylko grzbiet)
    long m_drukarnie;		// flaga bitowa drukarni
    CTime m_deadline;		// najwczesniejszy deadline
    CString m_dervinfo;		// informacja o dziedziczeniu
    CString f5_errInfo;		// komunikat o b��dzie dotycz�cym materia�u, wygenerowany przez funkcj� F5
    CStringA sBoundingBox;	// postscriptowy BB

    CTime m_ac_red;			// deadline redakcyjny dla czasopism
    CTime m_ac_fot;			// deadline na zdj�cia dla czasopism
    CTime m_ac_kol;			// deadline na kolumn� dla czasopism

    // Implementation
    void DrawGrid(CDC *pDC);
    void DrawReserved(CDC *pDC);
    void DrawDeadline(CDC *pDC, const CRect& pos) const;
    void DrawAcDeadline(CDC *pDC, const CRect& pos) const;

    void SetNr(int i);
    void AddAdd(CDrawAdd *pAdd);
    void RemoveAdd(CDrawAdd *pAdd, BOOL bRemodeFromAdds = TRUE);
    void SetSpace(const CDrawAdd *pObj);
    void SetSpotKolor(UINT spot_kolor);
    void SetBaseKrata(int s_x, int s_y, BOOL refresh = TRUE);
    void RealizeSpace(const CDrawAdd *pObj);
    void ChangeMark(size_t module, SpaceMode mode);
    void ChangeCaption(BOOL iscaption, const CString& cap);
    void DBChangeName(int id_drw);
    std::vector<int> CleanKraty(BOOL dbSave);

    BOOL FindSpace(CDrawAdd *pObj, int *px, int *py, const int sx, const int sy) const;
    BOOL CheckSpace(const CDrawAdd *pObj, const int px, const int py) const;
    BOOL CheckSpaceDiffKraty(const CDrawAdd *pObj, const int x, const int y) const;

    void BoundingBox(PGENEPSARG pArg, int *bx1, int *by1, int *bx2, int *by2);
    void Preview(PGENEPSARG pArg, CFile *dest, int bx1, int by1, int bx2, int by2);
    BOOL CheckRozmKrat(PGENEPSARG pArg);
    BOOL GenPDF(PGENEPSARG pArg);
    BOOL CheckSrcFile(PGENEPSARG pArg);
    BOOL StaleElementy(PGENEPSARG pArg, CFile *handle);
    BOOL GenEPS(PGENEPSARG pArg);
    BOOL GetDestName(PGENEPSARG pArg, const CString& sNum, CString& destName);
    BOOL PostPageToWorkflowServer(PGENEPSARG pArg, CMemFile *pOpiFile) const;
    CRect GetNormalizedModuleRect(size_t module) const;	// Prostok�t ograniczaj�cy modu� o numerze porz�dkowym module zakresu 0..sx*sy-1
    CString GetNrPaginy() const;

private:
    CFlag GetReservedFlag();							// Pobiera flag� szarych modu��w
    static CString GenerateGUIDString();				// Konwertuje wygenerowany GIUD do stringu
};

// CRozm
struct CRozm final
{
    CRozm() noexcept : CRozm(468, 575, 40, 34, 5, 6, 0, 0) {}
    CRozm(int m_w, int m_h, int m_sw, int m_sh, BYTE m_szpalt_x, BYTE m_szpalt_y, int m_typ_xx, BOOL m_scale_it) noexcept :
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
    WORD w;
    WORD h;
    WORD sw;
    WORD sh;
    BYTE szpalt_x;
    BYTE szpalt_y;
    BOOL scale_it;
    int typ_xx;
};