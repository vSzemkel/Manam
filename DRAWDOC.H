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

#include "drawpage.h"

#pragma once

#define A4 (5 * theApp.activeDoc->iPagesInRow)
#define A3 (10 * theApp.activeDoc->iPagesInRow)

class CDrawView;
class CDrawObj;
class CDrawOpis;
class CQueView;

enum class DocType : int {						// typ wyliczeniowy rodzajów dokumentów
	makieta, 
	makieta_lib,
	grzbiet_drukowany
};

class CDrawDoc final : public COleDocument
{
protected: // create from serialization only
	CDrawDoc();
	DECLARE_DYNCREATE(CDrawDoc)

	friend class CManODPNET;
	friend class CDrawDocDbReader;
	friend class CDrawDocDbWriter;
// Attributes
private:
	float drobneH;								// wysokoœæ drobnych na stronie
	int findNextInd ;
	CMenu m_grzbietMenu ;
	CTime dataZamkniecia ;
	long lastSearchNrAtex ;
	long lastSearchNrSpacer ;
	CString lastSearchNazwa ;

	void AdvanceAsidePos(CPoint& p) const;			// wylicza polozenie kolejnego nastepnego ogloszenia z boku
	float PowAdd2Mod(BOOL bQueStat) const;
	bool MoveOpisAfterPage(const CRect& rFrom, const CRect& rTo);
	void ModCount(UINT *m_modogl, UINT *m_modred, UINT *m_modrez, UINT *m_modwol) const;

public:
	static const TCHAR* asDocTypeExt[3];
	static int ValidKolor(TCHAR *st);
	static void DrawPageCross(CDC* pDC) ;
	static CString ErrMess(int cause);
	static CString XmlReadText(IXmlReader* reader);

	virtual ~CDrawDoc();
	virtual HMENU GetDefaultMenu();				// dla grzbietu zmien menu
	virtual void Serialize(CArchive& ar);		// overridden for document i/o
	virtual BOOL CDrawDoc::SaveModified();		// plik do pliku , baza do bazy
	virtual BOOL OnOpenDocument(LPCTSTR pszPathName);

	template<class T> T* GetPanelView() const;
	const CSize& GetSize() const { return m_size; }
	float GetDrobneH();
	void ComputeCanvasSize();
	void OnCloseDocument();
	int AddsCount() const;
	int GetIPage(int n) const;
	int GetIPage(CDrawPage *pPage) const;
	int Nr2NrPorz(const TCHAR* s) const;		// dla numeru na paginie liczy numer porz¹dkowy

// Operations
public:
//dotyczy CDrawObj - obiektow i add ogloszen
	void OnAsideAdds();
	void AsideAdds();							// ustawia z boku ogloszenia bez okreslonej strony
	CPoint GetAsideAddPos(BOOL opening) const; // wylicza lewy gorny rog kolejnego ogloszenia z boku makiety
	void ArrangeQue();							// ustawie og³oszenia na rysunku kolejki
	void MakietujStrone(CDrawPage *pPage); 		// makietowanie dla wskazanej strony
	void AddFind(long nrAtex, long nrSpacer, LPCTSTR nazwa);	// wyszukiwanie og³oszeñ
	void DerivePages(CDrawPage *pPage);			// dziedziczenie stron

	void Draw(CDC* pDC, CDrawView* pView);
	void DrawQue(CDC* pDC, CQueView* pView);
	void Print(CDC* pDC);
	void PrintPage(CDrawPage* pPage, CDC* pDC);
	void Add(CDrawObj* pObj);
	void AddQue(CDrawAdd* pObj);
	void RemoveQue(CDrawAdd* pObj);
	void RemoveFromHead(int n);
	void RemoveFromTail(int n);
	void Remove(CDrawObj* pObj);
	int GetAddPosition(CDrawAdd* pAdd) const;
	CDrawObj* ObjectAt(const CPoint& point) const;
	CDrawAdd* ObjectAtQue(const CPoint& point) const;
	CDrawAdd* FindAddAt(int i) const; // szuka ogloszenia na pozycji i wsrod ogl - opisy sie nie numeruje

	void SelectAdd(CDrawAdd* pObj, BOOL multiselect = FALSE) const;

	int ComputePageOrderNr(CDrawPage* pObj) const;				// oblicza numer porz¹dkowy strony na podstawie po³o¿enia
	void SetPageRectFromOrd(CDrawPage* pObj, size_t iOrd) const;		// pozycjonuje stronê na podstawie numeru porz¹dkowego
	void MoveBlockOfPages(int iSrcOrd, int iDstOrd, int iCnt);	// przesuwa ci¹g³y blok stron w obrêbie makiety

	size_t AddPage(CDrawPage* pObj);				// na koncu, zwieksza pamiec na tablice
	void AddPageAt(size_t idx, CDrawPage* pObj);	// pod zadanym idx
	void RemovePage(CDrawPage* pObj);
	CDrawPage* PageAt(const CPoint& point) const;
	CDrawPage* GetPage(int n) const;

	//..,,inne akcje menu -OGLOSZENIA
	void OnImport(BOOL fromDB); 
	void OnImportPlus(BOOL fromDB);
	void OnFileImportPlus();
	void OnFileImportMinus();
	void OnDBImportPlus();
	void OnDBImportMinus();
	void OnDrawOpcje();
	BOOL Import(BOOL check_exist);
	BOOL CreateAdd(LPCTSTR buffor, int ch, CPoint& pos, BOOL check_exist);
	BOOL DBImport(BOOL synchronize = FALSE);
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
	BOOL Add4Pages();									// dodaje strony tzn obiekty
	BOOL AddDrz4Pages(LPCTSTR ile_kolumn = _T("4"));	// dodaje strony tzn obiekty

	//open
	BOOL DBOpenDoc(TCHAR *sMakieta = nullptr);
	//save
	void OnDBSave();
	void OnDBSaveAs();
	void DBSaveAs(BOOL isSaveAs);
	//delete makieta
	void OnDBDelete();

	int DBReadSpot(int n);
	void ZmianaSpotow(int n);
	void ZmianaCaptions(int old_id_drw, int new_id_drw);

	void IniRozm();
	const CRozm* GetCRozm(int s_x, int s_y, int typ_xx = 0);
	const CRozm* GetCRozm(PGENEPSARG pArg, int s_x, int s_y, int typ_xx = 0);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual BOOL OnNewDocument();
	CSize m_size;

public:	
	int m_mak_xx;				// ident makiety
	int id_drw; 			// ident drzewa
	int iPagesInRow;		// ile stron rysyje siê w wierszu
	BYTE ovEPS : 1 ;		// nadpisuj EPS'y przy generacji
	BYTE isRO  : 1 ;		// otwarty tylko do odczytu
	BYTE isSIG : 1 ;		// makieta podpisana przez kierownika 
	BYTE isGRB : 1 ;		// grzbiet
	BYTE isLIB : 1 ;		// makieta biblioteczna
	BYTE isRED : 1 ;		// czy pokazuj¹ siê nag³ówki redakcyjne
	BYTE swCZV : 2 ;		// prze³¹cznik widoku: 0==standard; 1==czas_obow; 2==studio;
	BYTE isACD : 1 ;		// czy wczytano ju¿ dane o deadlinach dla czasopism
	CString gazeta;			// tytul i mutacja - ust przy wyborze dzewa
	CString data;			// ust przy save'owaniu
	CString opis;			// ustalana w Rejkodzie dzienna nazwa produktu
	CString dayws;			// do przekazywania parametrów w web serwisów
	CString daydir;			// do generowania ps z katalogów dniowych
	CString docmutred;		// lista dopuszczalnych mutacji redakcyjnych
	CString sOpiServerUrl;	// adres serwera OPI, do którego bêd¹ wysy³ane kolumny
	DocType iDocType;		// typ dokumentu na podstawie eDocType
	std::vector<UINT>		m_spot_makiety;

	std::vector<CDrawObj*>  m_objects; // lista og³oszeñ i opisów nale¿¹cych do dokumentu, og³oszenia przed opisami
	std::vector<CDrawPage*> m_pages;   // uporz¹dkowana kolekcja stron dokumentu; ostatnia strona ma index 0
	std::vector<CDrawAdd*>  m_addsque; // lista og³oszeñ czekaj¹cych w kolejce makiety
	std::vector<delobj_t>   m_del_obj; // obiekty usuniête z dokumentu, do usuniêcia w bazie
	std::vector<CRozm>		m_Rozm;	   // tablica rozmiarów krat i formatów niestandardowych dokumentu
	
	CFont m_pagefont;
	CFont m_addfont;
	
	CString prowadzacy1;
	CString prowadzacy2;
	CString sekretarz;		// dla makiet bibliotecznych zawiera opis
	CString symWydawcy;		// dwuliterowy symbol dzia³u zsy³aj¹cego
	
// Generated message map functions
protected:
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
};

/////////////////////////////////////////////////////////////////////////////

template<class T> T* CDrawDoc::GetPanelView() const
{
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL) {
		auto vView = dynamic_cast<T*>(GetNextView(pos));
		if (vView)
			return vView ;
	}
	return nullptr ;
}
