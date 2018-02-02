
#pragma once

// Hints for UpdateAllViews/OnUpdate
#define HINT_UPDATE_WINDOW				0
#define HINT_UPDATE_DRAWOBJ				1
#define HINT_UPDATE_SELECTION			2
#define HINT_DELETE_SELECTION			3
#define HINT_UPDATE_OLE_ITEMS			4
#define HINT_DELETE_FROM_GRID			5
#define HINT_UPDATE_GRID				6
#define HINT_EDIT_PASTE					7
#define HINT_UPDATE_COMBOBOXY			8
#define HINT_SAVEAS_DELETE_SELECTION	9
#define HINT_UPDATE_DRAWVIEW			10

// Print formats
#define		PRINT_PAGE		0
#define		PRINT_2PAGES	1
#define		PRINT_DOC		2
#define		PRINT_ALL		3
#define		PRINT_NULL		4	// nie wybrano co si� drukuje

class CDrawObj;
class CDrawDoc;

class CDrawView : public CScrollView
{
  protected: 
    DECLARE_DYNCREATE(CDrawView)

    // Operations
    static DWORD WINAPI DelegateGenEPS(LPVOID pArg);

    virtual ~CDrawView() noexcept {};
    virtual void OnDraw(CDC* pDC) override; // overridden to draw this view
    virtual void OnActivateView(BOOL bActivate, CView* pActiveView, CView* pDeactiveView) override;
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
    virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) override;
    virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll) override;
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs) override;
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    virtual BOOL IsSelected(const CObject* pDocItem) const override;
#ifdef _DEBUG
    virtual void AssertValid() const override;
    virtual void Dump(CDumpContext& dc) const override;
#endif

    auto GetDocument() const noexcept { return reinterpret_cast<CDrawDoc*>(m_pDocument); }
    void SetPageSize();
    void DocToClient(CRect& rect);
    void DocToClient(CPoint& point);
    void ClientToDoc(CPoint& point);
    void ClientToDoc(CRect& rect);
    void Select(CDrawObj* pObj, BOOL bAdd = FALSE);
    void SelectWithinRect(CRect rect, BOOL bAdd = FALSE);
    void OpenSelected();
    void Deselect(CDrawObj* pObj);
    void CloneSelection();
    void UpdateActiveItem();
    void InvalObj(CDrawObj* pObj);
    void Remove(CDrawObj* pObj);
    void Paste(COleDataObject& dataObject);
    void CheckPrintEps(BOOL isprint); //GN
    void OnChooseFont(CFont& m_font, BOOL IsPageFont);
    int GetZoomFactor() const noexcept { return m_zoomNum.cx; }
    unsigned char GetPagesPrinted() const noexcept { return m_pagesPrinted; }
    BOOL SetZoomFactor(CSize zoomNum, CSize zoomDenom);
    CRect GetInitialPosition();

    // Attributes
    static CLIPFORMAT m_cfDraw; // custom clipboard format
    std::vector<CDrawObj*> m_selection;
    BOOL m_bActive; // is the view active?

  protected:
    CDrawView() noexcept; // create from serialization only

    virtual void OnInitialUpdate() override; // called first time after construct
    // Printing support
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo) override;
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) override;
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) override;
    virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo) override;

    void OnChoosePageFont();
    void OnChooseAddFont();
    LPDEVMODE SetLandscape();

    // zoom factor
    CSize m_zoomNum;
    CSize m_zoomDenom;

    // Generated message map functions
    //{{AFX_MSG(CDrawView)
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnUpdateDrawTool(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDrawAdd(CCmdUI* pCmdUI);
    afx_msg void OnUpdateSingleSelect(CCmdUI* pCmdUI);
    afx_msg void OnUpdateAnySelect(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewZoomN(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewSpaceLocks(CCmdUI* pCmdUI);
    afx_msg void OnUpdateImportOpcje(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditProperties(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDrawMakStrony(CCmdUI* pCmdUI);
    afx_msg void OnDisableMenu(CCmdUI* pCmdUI);
    afx_msg void OnDisableMenuRDBMS(CCmdUI* pCmdUI);
    afx_msg void OnDisableMenuLIB(CCmdUI* pCmdUI);
    afx_msg void OnDisableMenuSTU(CCmdUI* pCmdUI);
    afx_msg void OnDisableMenuDEAL(CCmdUI* pCmdUI);
    afx_msg void OnDisableMenuAdSel(CCmdUI* pCmdUI);
    afx_msg void OnDisableMenuAdSelSTU(CCmdUI* pCmdUI);
    afx_msg void OnDisableMenuAdSelOPI(CCmdUI* pCmdUI);
    afx_msg void OnDisableMenuAdPageSel(CCmdUI* pCmdUI);
    afx_msg void OnUpdateVuCkMakietowanie(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewCzasobow(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewStudio(CCmdUI* pCmdUI);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnCancelEdit();
    afx_msg void OnDrawTool(UINT tool);
    afx_msg void OnViewZoomN(UINT nID);
    afx_msg void OnEditSelectAll();
    afx_msg void OnEditClear();
    afx_msg void OnViewSpaceLocks();
    afx_msg void OnImportOpcje();
    afx_msg void OnObjectMoveBack();
    afx_msg void OnObjectMoveForward();
    afx_msg void OnObjectMoveToBack();
    afx_msg void OnObjectMoveToFront();
    afx_msg void OnEditCopy();
    afx_msg void OnEditCut();
    afx_msg void OnEditPaste();
    afx_msg void OnEditProperties();
    afx_msg void OnDestroy();
    afx_msg void OnViewZoomCustom();
    afx_msg void OnViewLupkaPlus();
    afx_msg void OnViewLupkaMinus();
    afx_msg void OnDrawMakStrony();
    afx_msg void OnViewAdddesc();
    afx_msg void OnFilePrintPreview();
    afx_msg void OnFilePrintPreview1p();
    afx_msg void OnFilePrintPreviewAll();
    afx_msg void OnFilePrint();
    afx_msg void OnFilePrint1p();
    afx_msg void OnFilePrintAll();
    afx_msg void OnDrawOpcje();
    afx_msg void OnViewCzasobow();
    afx_msg void OnPrintEps(); //GN
    afx_msg void OnCheckEps(); //GN
    afx_msg void OnAsideAdds();
    afx_msg void OnViewStudio();
    afx_msg void OnIrfan();
    afx_msg void OnPrevPdf();
    afx_msg void OnPrevDig();
    afx_msg void OnPrevKor();
    afx_msg void OnAtexSyg();
    afx_msg void OnChangeView();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
  private:
    static int OnDisableMenuInt(CCmdUI* pCmdUI);
    BOOL ModifyMutczas(int n);
    void OnPrevKolumnaDruk(); // pokazuje preview kolumny przygotowanej do druku

    unsigned char m_pagesPrinted;
};