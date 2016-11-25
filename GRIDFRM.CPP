
#include "stdafx.h"
#include "drawvw.h"
#include "drawdoc.h"
#include "drawadd.h"
#include "mainfrm.h"
#include "gridfrm.h"
#include "addlistctrl.h"

/////////////////////////////////////////////////////////////////////////////
// CGridFrm

extern CDrawApp NEAR theApp ;

TCHAR CGridFrm::studioStats[][8] = {_T("Brak"), _T("Jest"), _T("Akcept."), _T("Nowy"), _T("O.K."), _T("Wys³any"), _T("Filtr"), _T("Err")};

IMPLEMENT_DYNCREATE(CGridFrm, CFormView)

BOOL CGridFrm::bSortAsc = TRUE;
GridSortCol CGridFrm::eSortCol = GridSortCol::lp;
GridSortCol CGridFrm::eLastOrder = GridSortCol::lp;

CGridFrm::CGridFrm() : CFormView(CGridFrm::IDD), lcPubList(this)
{
	EnableAutomation();
	//{{AFX_DATA_INIT(CGridFrm)
	//}}AFX_DATA_INIT
	lcPubList.EnableMultipleSort(FALSE);
	lcPubList.EnableMarkSortedColumn();
	showLastAdnoUsed = m_bInitialized = m_bEventLockout = FALSE;
}

void CGridFrm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGridFrm)
	DDX_Control(pDX, IDC_PUBLIST, lcPubList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGridFrm, CFormView)
	//{{AFX_MSG_MAP(CGridFrm)
	ON_COMMAND_RANGE(ID_SORT_LP, ID_SORT_WARLOG, OnSort)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SORT_LP, ID_SORT_WARLOG, OnUpdateSort)
	ON_COMMAND(IDM_SHOWREPT, OnShowrept)
	ON_UPDATE_COMMAND_UI(IDM_SHOWREPT, OnUpdateShowrept)
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_NOTIFY(NM_DBLCLK, IDC_PUBLIST, OnDbClick)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PUBLIST, OnChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridFrm diagnostics

#ifdef _DEBUG
void CGridFrm::AssertValid() const
{
	CFormView::AssertValid();
}

void CGridFrm::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGridFrm message handlers

BOOL CGridFrm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	BOOL ret = CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
	OnInitialUpdate();
	return ret;
}

void CGridFrm::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (m_bEventLockout || (pHint != NULL && !dynamic_cast<CDrawAdd*>(pHint)))
		return;

	m_bEventLockout = TRUE;
	CDrawDoc* pDoc = GetDocument();
	ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CDrawDoc)));

	switch (lHint)
	{
	case HINT_UPDATE_WINDOW:    // redraw entire window
		if (!pSender) SetDlgItemText(IDC_GAZETA, GetDocument()->gazeta + " " + GetDocument()->data);
		InvalAll(); 
		break;

	case HINT_SAVEAS_DELETE_SELECTION: // gdy jest save as i trzeba usunac ogl z makiety
		// to usuwa ogloszenia z m_selection
		// dla gridu robi to to samo co updatewindow
		InvalAll();
		break;

	case HINT_UPDATE_DRAWOBJ:   // a single object has changed
		InvalObj((CDrawAdd*)pHint, -1);
		break;

	case HINT_UPDATE_SELECTION: // an entire selection has changed   
	case HINT_UPDATE_DRAWVIEW:
	case HINT_UPDATE_COMBOBOXY:
	case HINT_DELETE_SELECTION: // an entire selection has been removed ale dla view
	case HINT_UPDATE_OLE_ITEMS:
		break;

	case HINT_DELETE_FROM_GRID: // tylko dla nas
		InvalAll();
		break;

	case HINT_UPDATE_GRID:
		Select((CDrawAdd*)pHint, -1);
		break;

	case HINT_EDIT_PASTE: // an entire selection has changed
		InvalAll();
		Select((CDrawAdd*)pHint, -1);
		break;

	default:
		ASSERT(FALSE);
		break;
	}
	m_bEventLockout = FALSE;
}

void CGridFrm::InvalAll()
{
	auto pDoc = (CDrawDoc*) GetDocument();
	ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CDrawDoc)));

	lcPubList.SetRedraw(FALSE);
	int nRow = 0, ile = pDoc->AddsCount();
	while (lcPubList.GetItemCount() > ile)
		lcPubList.DeleteItem(0);

	for (const auto& pObj : pDoc->m_objects) {
		CDrawAdd *pAdd = dynamic_cast<CDrawAdd*>(pObj);
		if (pAdd)
			RefreshRow(nRow++, pAdd);
	}

	CSize sTotalSize = GetTotalSize();
	sTotalSize.cy = ROWHEIGHT*(3 + lcPubList.GetItemCount());
	SetScrollSizes(MM_TEXT, sTotalSize);
	lcPubList.SetRedraw(TRUE);
}

void CGridFrm::InvalObj(CDrawAdd *pObj, int idx)
{
	if (dynamic_cast<CDrawAdd*>(pObj) == nullptr)
		return;
	if (idx == -1)
		idx = FindRow(reinterpret_cast<DWORD_PTR>(pObj));

	lcPubList.SetRedraw(FALSE);
	RefreshRow(idx, pObj);
	lcPubList.SetRedraw(TRUE);
}

void CGridFrm::Select(CDrawAdd *pObj, int i)
{
	if (!pObj)
		return;
	i = FindRow(reinterpret_cast<DWORD_PTR>(pObj));

	lcPubList.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);

	// przeskroluj, a jeslinowe og³oszenie, byc moze potrzebny jest resize gridu
	if (i == lcPubList.GetItemCount())
		InvalAll();

	CRect r;
	GetClientRect(&r);
	int iSelRowY = i*ROWHEIGHT + MARGINTOP;
	int iTopRowY = GetScrollPosition().y;
	int iBottomRowY = iTopRowY + r.Height() - ROWHEIGHT;
	if (iSelRowY < iTopRowY || iBottomRowY < iSelRowY)
		ScrollToPosition(CPoint(0, iSelRowY - r.Height()/2));
}

int CGridFrm::FindRow(DWORD_PTR key)
{
	int rc = lcPubList.GetItemCount();
	for (int n = 0; n < rc; n++)
		if (lcPubList.GetItemData(n) == key)
			return n;

	return rc;
}

void CGridFrm::RefreshRow(int nRow, CDrawAdd *vAdd)
{
	TCHAR bf[64];

	int	nImage = 8;
	if (showLastAdnoUsed) {
		if (vAdd->skad_ol.GetLength() == 2)
			nImage = (vAdd->skad_ol != theApp.activeDoc->symWydawcy) ? IMG_FILE_REMOTE : IMG_FILE_LOCAL;
	} else {
		if (vAdd->flags.studio >= STUDIO_MSG)
			nImage = IMG_ERR;
		else if(vAdd->powtorka == 0) 
			nImage = (vAdd->flags.studio == STUDIO_JEST) ? IMG_NEW_JEST : IMG_NEW_BRAK ;
		else		
			nImage = (vAdd->flags.studio == STUDIO_JEST) ? IMG_POWT_JEST : IMG_POWT_BRAK ;
	}
	
	LVITEM lvi = {0};
	lvi.mask = LVIF_IMAGE | LVIF_PARAM;
	lvi.iItem = nRow;
	lvi.iImage = nImage;
	lvi.lParam = reinterpret_cast<LPARAM>(vAdd);
	if (nRow == lcPubList.GetItemCount())
		lcPubList.InsertItem(&lvi);
	else
		lcPubList.SetItem(&lvi);

	::StringCchPrintf(bf, 64, _T("%i."), nRow + 1);
	lcPubList.SetItemText(nRow, lp, bf);
	::StringCchPrintf(bf, 64, _T("%li"), vAdd->nreps);
	lcPubList.SetItemText(nRow, zamowienie, _tcsncmp(bf, _T("-1"), 2) ? bf : _T(""));
	lcPubList.SetItemText(nRow, kod, vAdd->kodModulu);
	lcPubList.SetItemText(nRow, logiczna, vAdd->logpage);
	::StringCchPrintf(bf, 64, _T("%iX%i "), vAdd->sizex, vAdd->sizey);
	lcPubList.SetItemText(nRow, rozmiar, bf);
	lcPubList.SetItemText(nRow, nazwa, vAdd->nazwa);
	_itot_s(((vAdd->fizpage)>>3), bf, 64, 10);
	if (vAdd->fizpage & c_rzym) 
		::StringCchCat(bf, 64, _T("rom"));
	lcPubList.SetItemText(nRow, strona, bf);
	if (vAdd->kolor != c_brak) {
		if (vAdd->kolor == c_full) 
			::StringCchCopy(bf, 64, FULL); 
		else 
			::StringCchCopy(bf, 64, ((CMainFrame*)theApp.GetMainWnd())->Spot_Kolor[vAdd->kolor>>3]);
	} else ::StringCchCopy(bf, 64, BRAK); 
	lcPubList.SetItemText(nRow, kolorek,bf);
	lcPubList.SetItemText(nRow, uwagi, vAdd->remarks.IsEmpty() ? vAdd->remarks_atex : vAdd->remarks);
	lcPubList.SetItemText(nRow, powtorka, showLastAdnoUsed ? vAdd->lastAdnoUsed : (vAdd->powtorka == 0 ? "" : vAdd->powtorka.Format(c_ctimeData)));
	if (showLastAdnoUsed) {
		lcPubList.SetItemText(nRow, oldadno, vAdd->skad_ol);
		vAdd->SetDotM(vAdd->powtorka == 0 && !vAdd->skad_ol.IsEmpty() && vAdd->skad_ol != theApp.activeDoc->symWydawcy);
	} else if (vAdd->oldAdno > 0) {
		::StringCchPrintf(bf, 64, _T("%li"), vAdd->oldAdno);
		lcPubList.SetItemText(nRow, oldadno, bf);
	} else lcPubList.SetItemText(nRow, oldadno, _T(""));

	LPCTSTR sStudio;
	switch (vAdd->flags.studio) {
		case STUDIO_JEST:
			if (vAdd->f5_errInfo.IsEmpty())
				sStudio = (LPCTSTR)studioStats[vAdd->flags.studio]; 
			else {
				if (_istdigit(vAdd->f5_errInfo[0])) 
					vAdd->f5_errInfo = vAdd->f5_errInfo.Mid(9, vAdd->f5_errInfo.GetLength() - 11);
				sStudio = (LPCTSTR)vAdd->f5_errInfo;
			}
			break;
		default:
			sStudio = (LPCTSTR)studioStats[vAdd->flags.studio]; break;
	}
	lcPubList.SetItemText(nRow, studio, sStudio);
}

void CGridFrm::OnShowrept() 
{
	showLastAdnoUsed = !showLastAdnoUsed;
	auto& hcHeader = lcPubList.GetHeaderCtrl();
	HDITEM hi = {0};
	hi.mask = HDI_TEXT;
	if (showLastAdnoUsed) {
		::StringCchCopy(theApp.bigBuf, n_size, _T("Numer z"));
		hi.pszText = theApp.bigBuf;
		hcHeader.SetItem(powtorka, &hi);
		::StringCchCopy(theApp.bigBuf, n_size, _T("Oddzia³"));
		hi.pszText = theApp.bigBuf;
		hcHeader.SetItem(oldadno, &hi);

		long adno;
		CDrawAdd *vAdd;
		CDrawDoc *vDoc = GetDocument();
		TCHAR *ch, tytul[10], mutacja[5];
		char kiedypowt[32];
		CString sURL;

		::StringCchCopy(tytul, 10, vDoc->gazeta);
		if (ch = _tcschr(tytul, ' ')) {
			::StringCchCopy(mutacja, 5, ch + 1);
			*ch = '\0';
		} else 
			::StringCchCopy(mutacja, 5, _T("0"));

		auto line = reinterpret_cast<char*>(theApp.bigBuf);
		sURL.Format(_T("tyt=%s&mut=%s&kiedy=%s"), tytul, mutacja, vDoc->data);
		std::unique_ptr<CHttpFile> pFile = theApp.OpenURL(3, sURL);
		if (!pFile)
			AfxMessageBox(_T("Nieprawid³owy URL: ") + sURL);
		else {
			while (pFile->ReadString(theApp.bigBuf, bigSize))
				if (!strncmp(line, "&powtorki", 9)) break;
			while (pFile->ReadString(theApp.bigBuf, bigSize)) {
				if (!strncmp(line, "&end powtorki", 13)) break;
				if (!strncmp(line, "ERR", 3)) {
					AfxMessageBox(_T("ATEX nie odpowiada"));
					break;
				}
				if (sscanf_s(line, "%li,%s", &adno, kiedypowt, 32) == 2) {
					vAdd = vDoc->AddExists(adno);
					if (vAdd) {
						vAdd->skad_ol = &kiedypowt[11];
						kiedypowt[10] = '\0';
						vAdd->lastAdnoUsed = kiedypowt;
					}
				}
			}
			
			pFile->Close();
		}
	} else {
		::StringCchCopy(theApp.bigBuf, n_size, _T("Powtórka"));
		hi.pszText = theApp.bigBuf;
		hcHeader.SetItem(powtorka, &hi);
		::StringCchCopy(theApp.bigBuf, n_size, _T("z ATEX"));
		hi.pszText = theApp.bigBuf;
		hcHeader.SetItem(oldadno, &hi);
	}
	
	InvalAll();
}

void CGridFrm::OnUpdateShowrept(CCmdUI* pCmdUI) 
{
	pCmdUI->SetText(showLastAdnoUsed ? (LPCTSTR)_T("Ukryj powtórki numerów\tF7") : (LPCTSTR)_T("Poka¿ powtórki numerów\tF7")) ;
}

/*************************** SORTOWANIE **********************************/
void CGridFrm::OnSort(UINT col) 
{
	eSortCol = static_cast<GridSortCol>(col);
	bSortAsc = eSortCol != eLastOrder;
	eLastOrder = eSortCol;

	switch (eSortCol) {
		case GridSortCol::lp: lcPubList.Sort(kolejnosc::lp, bSortAsc); break;
		case GridSortCol::strona: lcPubList.Sort(kolejnosc::strona, bSortAsc); break;
		case GridSortCol::zamowienie: lcPubList.Sort(kolejnosc::zamowienie, bSortAsc); break;
		case GridSortCol::nazwa: lcPubList.Sort(kolejnosc::nazwa, bSortAsc); break;
		case GridSortCol::rozmiar: lcPubList.Sort(kolejnosc::rozmiar, bSortAsc); break;
		case GridSortCol::logiczna: lcPubList.Sort(kolejnosc::logiczna, bSortAsc); break;
	}
}

void CGridFrm::OnUpdateSort(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(eLastOrder == static_cast<GridSortCol>(pCmdUI->m_nID));
}

/***************************** DRUKOWANIE ********************************/
BOOL CGridFrm::OnPreparePrinting(CPrintInfo* pInfo) 
{
	BOOL ret = DoPreparePrinting(pInfo);
	isLandscape = pInfo->m_pPD->GetDevMode()->dmOrientation == DMORIENT_LANDSCAPE;
	pInfo->SetMaxPage((int)ceil((float)lcPubList.GetItemCount()/(isLandscape ? LANDROWSPERPAGE : PORTROWSPERPAGE))) ;
	return ret;
}

void CGridFrm::OnFilePrintPreview()
{
	AFXPrintPreview(this);
}

void CGridFrm::OnPrint(CDC* pDC, CPrintInfo* pInfo) {
	CClientDC dcScreen(NULL);
	pDC->SetMapMode(MM_ANISOTROPIC);
	// czynnik skaluj¹cy 
	pDC->SetWindowExt((int)(dcScreen.GetDeviceCaps(LOGPIXELSX)/1.295), (int)(dcScreen.GetDeviceCaps(LOGPIXELSX)/1.17));
	pDC->SetViewportExt(pDC->GetDeviceCaps(LOGPIXELSX),	pDC->GetDeviceCaps(LOGPIXELSX));
	
	CPen pen(PS_SOLID, 1, RGB(0, 0, 0));  // solid black pen
	CPen* pOldPen = pDC->SelectObject(&pen);

	if (pInfo->m_nCurPage == 1 && !pInfo->m_bPreview) { // naglowek
		pDC->SaveDC();
		CSize vp = pDC->GetWindowExt();
		vp.cx *= 11 ; vp.cy *= 11 ;
		pDC->SetWindowExt(vp);
		pDC->TextOut(2700, 0, GetDocument()->gazeta + _T(" ") + GetDocument()->data) ;
		pDC->RestoreDC(-1);
	}

	pDC->SaveDC();
	CPoint ptOffset(0, (pInfo->m_nCurPage-1)*(isLandscape ? LANDPAGEHEIGHT : PORTPAGEHEIGHT)) ;
	pDC->LPtoDP(&ptOffset);
	pDC->OffsetViewportOrg(0, -ptOffset.y+HEADERHEIGHT);
	::SendMessage(lcPubList.GetHeaderCtrl().m_hWnd, WM_PAINT, (WPARAM)pDC->m_hDC, 0L);
	::SendMessage(lcPubList.m_hWnd, WM_PAINT, (WPARAM)pDC->m_hDC, 0L);
	pDC->RestoreDC(-1);

	pDC->SelectObject(pOldPen);
}

/************************* OBS£UGA ZDARZEÑ ***************************/
void CGridFrm::OnDbClick(NMHDR * , LRESULT *pResult)
{
	if (m_bEventLockout)
		return;

	int idx = lcPubList.GetSelectionMark();
	if (idx == -1) return;

	auto pAdd = reinterpret_cast<CDrawAdd*>(lcPubList.GetItemData(idx));
	pAdd->OnOpen((CDrawView*)this);

	*pResult = 0;
}

void CGridFrm::OnChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	auto pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (!m_bEventLockout && (pNMLV->uNewState & LVIS_SELECTED) != 0) {
		auto pAdd = reinterpret_cast<CDrawAdd*>(lcPubList.GetItemData(pNMLV->iItem));
		GetDocument()->SelectAdd(pAdd);
	}

	*pResult = 0;
}
