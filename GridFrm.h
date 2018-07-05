
#pragma once

#include "Manam.h"
#include "AddListCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CGridFrm form view

#define		PORTROWSPERPAGE		54
#define		PORTPAGEHEIGHT		914
#define		LANDROWSPERPAGE		36
#define		LANDPAGEHEIGHT		630
#define		HEADERHEIGHT		180
#define		ROWHEIGHT			17
#define		MARGINTOP			43

#define		IMG_ERR			1
#define		IMG_NEW_BRAK	8
#define		IMG_NEW_JEST	0
#define		IMG_POWT_BRAK	4
#define		IMG_POWT_JEST	5
#define		IMG_BRAK		8
#define		IMG_FILE_LOCAL	2
#define		IMG_FILE_REMOTE	3

enum class GridSortCol : uint8_t
{
    lp = ID_SORT_LP,            // -
    strona = ID_SORT_STR,       // i
    zamowienie = ID_SORT_NREPS, // i
    nazwa = ID_SORT_NAZWA,      // s
    rozmiar = ID_SORT_ROZMIAR,  // s
    logiczna = ID_SORT_WARLOG,  // s
};

class CDrawAdd;
class CDrawDoc;

class CGridFrm : public CFormView
{
protected:
    CGridFrm(); // protected constructor used by dynamic creation
    DECLARE_DYNCREATE(CGridFrm)

    // Form Data
private:
    BOOL isLandscape;
    BOOL m_bEventLockout;
    BOOL m_bInitialized;
    CAddListCtrl lcPubList;
    void RefreshRow(int nRow, CDrawAdd* vAdd);
    int FindRow(DWORD_PTR key) const;
    void Select(CDrawAdd* pObj, int i);
    void InvalObj(CDrawAdd* pObj, int idx);
    void InvalAll();

    static bool bSortAsc;
    static GridSortCol eSortCol;
public:
    //{{AFX_DATA(CGridFrm)
    enum { IDD = IDD_GRIDFORM };
    //}}AFX_DATA

// Attributes
public:
    bool showLastAdnoUsed;
    static TCHAR studioStats[][8];
    static GridSortCol eLastOrder;
    // Operations
public:
    auto GetDocument() const { return reinterpret_cast<CDrawDoc*>(m_pDocument); }

public:
    BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = nullptr) override;
protected:
    void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
    BOOL OnPreparePrinting(CPrintInfo* pInfo) override;
    void OnPrint(CDC* pDC, CPrintInfo* pInfo) override;
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
    //}}AFX_VIRTUAL

// Implementation
protected:
    ~CGridFrm() override = default;
#ifdef _DEBUG
    void AssertValid() const override;
    void Dump(CDumpContext& dc) const override;
#endif

    // Generated message map functions
    //{{AFX_MSG(CGridFrm)
    afx_msg void OnFilePrintPreview();
    afx_msg void OnSort(UINT col);
    afx_msg void OnUpdateSort(CCmdUI* pCmdUI);
    afx_msg void OnShowrept();
    afx_msg void OnUpdateShowrept(CCmdUI* pCmdUI);
    afx_msg void OnDbClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnChanged(NMHDR* pNMHDR, LRESULT* pResult);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
