
#pragma once

#include "Manam.h"
#include "AddListCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CGridFrm form view

constexpr auto PORTROWSPERPAGE = 54;
constexpr auto PORTPAGEHEIGHT = 914;
constexpr auto LANDROWSPERPAGE = 36;
constexpr auto LANDPAGEHEIGHT = 630;
constexpr auto HEADERHEIGHT = 180;
constexpr auto ROWHEIGHT = 17;
constexpr auto MARGINTOP = 43;

enum class GridImgType : uint8_t {
    err = 1,
    new_brak = 8,
    new_jest = 0,
    powt_brak = 4,
    powt_jest = 5,
    brak = 8,
    file_local = 2,
    file_remote = 3
};

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
    CAddListCtrl lcPubList;
    BOOL isLandscape;
    BOOL m_bEventLockout;
    BOOL m_bInitialized;

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
    static GridSortCol eLastOrder;
    static constexpr TCHAR studioStats[][8] = {_T("Brak"), _T("Jest"), _T("Akcept."), _T("Nowy"), _T("O.K."), _T("Wys³any"), _T("Filtr"), _T("Err")};
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
