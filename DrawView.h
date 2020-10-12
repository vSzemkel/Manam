
#pragma once

// Hints for UpdateAllViews/OnUpdate
constexpr auto HINT_UPDATE_WINDOW = 0;
constexpr auto HINT_UPDATE_DRAWOBJ = 1;
constexpr auto HINT_UPDATE_SELECTION = 2;
constexpr auto HINT_DELETE_SELECTION = 3;
constexpr auto HINT_UPDATE_OLE_ITEMS = 4;
constexpr auto HINT_DELETE_FROM_GRID = 5;
constexpr auto HINT_UPDATE_GRID = 6;
constexpr auto HINT_EDIT_PASTE = 7;
constexpr auto HINT_UPDATE_COMBOBOXY = 8;
constexpr auto HINT_SAVEAS_DELETE_SELECTION = 9;
constexpr auto HINT_UPDATE_DRAWVIEW = 10;

class CDrawObj;
class CDrawDoc;

class CDrawView : public CScrollView
{
  protected: 
    DECLARE_DYNCREATE(CDrawView)

    // Operations
    static void CALLBACK DelegateGenEPS(PTP_CALLBACK_INSTANCE /*unused*/, PVOID parameter, PTP_WORK work);

    ~CDrawView() noexcept override = default;
    void OnDraw(CDC* pDC) override; // overridden to draw this view
    void OnActivateView(BOOL bActivate, CView* pActiveView, CView* pDeactiveView) override;
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
    void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) override;
    BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll) override;
    BOOL PreCreateWindow(CREATESTRUCT& cs) override;
    BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    BOOL IsSelected(const CObject* pDocItem) const override;
#ifdef _DEBUG
    void AssertValid() const override;
    void Dump(CDumpContext& dc) const override;
#endif

    auto GetDocument() const noexcept { return reinterpret_cast<CDrawDoc*>(m_pDocument); }
    void SetPageSize();
    void DocToClient(CRect& rect);
    void DocToClient(CPoint& point);
    void ClientToDoc(CPoint& point);
    void ClientToDoc(CRect& rect);
    void Select(CDrawObj* pObj, SelectUpdateMode mode = SelectUpdateMode::replace);
    void SelectWithinRect(CRect rect, bool bAdd = false);
    void OpenSelected();
    void Deselect(CDrawObj* pObj);
    void CloneSelection();
    void UpdateActiveItem();
    void InvalObj(CDrawObj* pObj);
    void Remove(CDrawObj* pObj);
    void Paste(COleDataObject& dataObject);
    void CheckPrintEps(BOOL isprint); //GN
    void OnChooseFont(CFont& m_font, bool IsPageFont);
    int GetZoomFactor() const noexcept { return m_zoomNum.cx; }
    PrintFormat GetPagesPrinted() const noexcept { return m_pagesPrinted; }
    BOOL SetZoomFactor(CSize zoomNum, CSize zoomDenom);

    // Attributes
    static CLIPFORMAT m_cfDraw; // custom clipboard format
    std::vector<CDrawObj*> m_selection;
    BOOL m_bActive{FALSE}; // is the view active?

  protected:
    CDrawView() noexcept; // create from serialization only

    void OnInitialUpdate() override; // called first time after construct
    // Printing support
    BOOL OnPreparePrinting(CPrintInfo* pInfo) override;
    void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) override;
    void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) override;
    void OnPrint(CDC* pDC, CPrintInfo* pInfo) override;

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
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
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
    PrintFormat m_pagesPrinted{PrintFormat::doc};

    BOOL ModifyMutczas(int n);
    void OnPrevKolumnaDruk();                // pokazuje preview kolumny przygotowanej do druku
    void UpdateToolbar(ToolbarMode newMode); // wyswietla obrazki na toolbarze adekwatnie do trybu pracy
};
