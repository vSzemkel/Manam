
#include "drawobj.h"

#pragma once

class CFlag ;
class CDrawAdd ;
class CDrawPage ;
class CGenEpsInfoDlg ;

enum class SpaceMode : BYTE {
	avail,
	redlock,
	spacelock,
};

typedef struct _GENEPSARG {
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

class CKrataNiebazowa final
{
public:
	explicit CKrataNiebazowa(int s_x, int s_y, CFlag&& space, CFlag&& locked, CFlag&& red) 
		: m_szpalt_x(s_x), m_szpalt_y(s_y), m_space(std::move(space)), m_space_locked(std::move(locked)), m_space_red(std::move(red))
	{};
	int m_szpalt_x;
	int m_szpalt_y;
	CFlag m_space;
	CFlag m_space_locked;
	CFlag m_space_red;
};

class CDrawPage : public CDrawObj
{
protected:
	DECLARE_SERIAL(CDrawPage);
	CDrawPage() {};

public:
	CDrawPage(const CRect* position);
	virtual ~CDrawPage();
// Overrides
	virtual void Draw(CDC* pDC) override;
	virtual void Print(CDC* pDC) override;
	virtual void Serialize(CArchive& ar) override;
	virtual void UpdateInfo() override;
	virtual void DrawKolor(CDC* pDC, const CRect& pos) const override;
	virtual void MoveTo(const CRect& positon, CDrawView* pView = nullptr) override;
	virtual BOOL OnOpen(CDrawView* pView) override;
	virtual CDrawObj* Clone(CDrawDoc* pDoc) const override;
public:

	//Members
	int id_str;
	int szpalt_x;
	int szpalt_y;
	int nr;				// nr strony, dolne 3 bity typ numeracji 0 normalna, 1 rzymska, wyzsze nr
	int prn_mak_xx ;	// identyfikator makiety prn
	int wyd_xx ;		// identyfikator wydawcy strony
	BYTE m_typ_pary;	// czy strona wchodzi do sztucznej rozkladowki
	CString caption;	// widoczny naglowek, redakcyjny lub cennikowy
	CString caption_alt;// alternatywny naglowek, cennikowy lub redakcyjny
	CString name;		// logiczna strony z atexa ==sciezka ale zmieniamy  str_log np RED/PRG/PON user poprwia tylko pon
	CString mutred;		// alternatywne mutacje redakcyjne strony
	CFlag space;		// bitowa maska zajêtosci
	CFlag space_locked; // bitowa maska blokady
	CFlag space_red;	// bitowa maska powierzchni redakcyjnej
	BOOL niemakietuj;	// blokada makietowania

	std::vector<CDrawAdd*> m_adds;	// og³oszenia przypisane do danej strony
	std::vector<CKrataNiebazowa> m_kraty_niebazowe;	// kraty niebazowe

	int m_dervlvl;			// poziom dziedziczenia
	int m_mutczas;			// numer mutacji czasowej (tylko grzbiet)
	long m_drukarnie;		// flaga bitowa drukarni
	CTime m_deadline;		// najwczesniejszy deadline
	CString m_dervinfo;		// informacja o dziedziczeniu
	CString f5_errInfo;		// komunikat o b³êdzie dotycz¹cym materia³u, wygenerowany przez funkcjê F5
	CStringA sBoundingBox;	// postscriptowy BB

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
	void RemoveAdd(CDrawAdd* pAdd, BOOL bRemodeFromAdds = TRUE);
	void SetSpace(CDrawAdd* pObj);
	void SetSpotKolor(UINT spot_kolor);
	void SetBaseKrata(int s_x, int s_y, BOOL refresh = TRUE);
	void RealizeSpace(CDrawAdd* pObj);
	void ChangeMark(size_t module, SpaceMode mode);
	void ChangeCaption(BOOL iscaption, const CString& cap);
	void DBChangeName(int id_drw);
	std::vector<int> CleanKraty(BOOL dbSave);

	BOOL FindSpace(CDrawAdd* pObj, int *px, int *py, int sx, int sy) const;
	BOOL CheckSpace(CDrawAdd* pObj, int px, int py) const;
	BOOL CheckSpaceDiffKraty(CDrawAdd* pObj, int x, int y, int sx, int sy) const;

	void BoundingBox(PGENEPSARG pArg, int *bx1, int *by1, int *bx2, int *by2);
	void Preview(PGENEPSARG pArg, CFile *dest, int bx1, int by1, int bx2, int by2);
	BOOL CheckRozmKrat(PGENEPSARG pArg);
	BOOL GenPDF(PGENEPSARG pArg);
	BOOL CheckSrcFile(PGENEPSARG pArg);
	BOOL StaleElementy(PGENEPSARG pArg, CFile *handle);
	BOOL GenEPS(PGENEPSARG pArg);
	BOOL GetDestName(PGENEPSARG pArg, CString& sNum, CString& destName);
	BOOL PostPageToWorkflowServer(PGENEPSARG pArg, CMemFile *pOpiFile) const;
	CString GetNrPaginy() const;

protected:
	friend class CRectTool;
	friend class CKolorTool;
	friend class CSpaceTool;

private:
	CFlag GetReservedFlag();							// Pobiera flagê szarych modu³ów
	CRect GetNormalizedModuleRect(size_t module) const;		// Prostok¹t ograniczaj¹cy modu³ o numerze porz¹dkowym module zakresu 0..sx*sy-1
	static CString GenerateGUIDString();				// Konwertuje wygenerowany GIUD do stringu
};

//CRozm
class CRozm {
public:
	CRozm();
	CRozm(int m_w, int m_h, int m_sw,int m_sh, BYTE m_szpalt_x, BYTE m_szpalt_y, int m_typ_xx, BOOL m_scale_it);
	WORD w;
	WORD h;
	WORD sw;
	WORD sh;
	BYTE szpalt_x;
	BYTE szpalt_y;
	BOOL scale_it;
	int typ_xx;
};