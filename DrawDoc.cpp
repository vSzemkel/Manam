
#include "StdAfx.h"
#include "AtexKrat.h"
#include "DrawAdd.h"
#include "DrawDoc.h"
#include "DrawOpis.h"
#include "DrawPage.h"
#include "DrawView.h"
#include "KolDlg.h"
#include "MainFrm.h"
#include "Manam.h"
#include "QueView.h"

extern BOOL drawErrorBoxes;
extern BOOL disableMenu;

const TCHAR* CDrawDoc::asDocTypeExt[3] = { _T(".DB"), _T(".LIB"), _T(".GRB") }; // rozszerzenie poszczeglnych typów dokumentow opisanych w eDocType
std::vector<UINT> CDrawDoc::spoty;
std::vector<CString> CDrawDoc::kolory;
std::vector<CBrush*> CDrawDoc::brushe;

/////////////////////////////////////////////////////////////////////////////
// CDrawDoc
IMPLEMENT_DYNCREATE(CDrawDoc, COleDocument)

BEGIN_MESSAGE_MAP(CDrawDoc, COleDocument)
    //{{AFX_MSG_MAP(CDrawDoc)
    ON_COMMAND(ID_FILE_SAVE_AS, &CDrawDoc::OnFileSaveAs)
    ON_COMMAND(ID_VU_MAKIETOWANIE, &CDrawDoc::OnVuMakietowanie)
    ON_COMMAND(ID_VU_CK_MAKIETOWANIE, &CDrawDoc::OnVuCkMakietowanie)
    ON_COMMAND(ID_ADD_FIND, &CDrawDoc::OnAddFind)
    ON_COMMAND(ID_ADD_SYNCHRONIZE, &CDrawDoc::OnAddSynchronize)
    ON_COMMAND(IDM_EDIT_FIND_NEXT, &CDrawDoc::OnEditFindNext)
    ON_UPDATE_COMMAND_UI(IDM_EDIT_FIND_NEXT, &CDrawDoc::OnUpdateEditFindNext)
    ON_COMMAND(ID_FILE_SAVE, &CDrawDoc::OnFileSave)
    ON_UPDATE_COMMAND_UI(ID_FILE_DBSAVE, &CDrawDoc::OnDisableMenuRO)
    ON_COMMAND(IDM_SYNCPOW, &CDrawDoc::OnSyncpow)
    ON_UPDATE_COMMAND_UI(IDM_SYNCPOW, &CDrawDoc::OnUpdateSyncpow)
    ON_COMMAND(IDM_DELREMARKS, &CDrawDoc::OnDelremarks)
    ON_COMMAND(IDM_CHECKREP, &CDrawDoc::OnCheckrep)
    ON_UPDATE_COMMAND_UI(IDM_CHECKREP, &CDrawDoc::OnDisableDB)
    ON_COMMAND(IDM_EPSDATA, &CDrawDoc::OnEpsdate)
    ON_UPDATE_COMMAND_UI(IDM_EPSDATA, &CDrawDoc::OnUpdateEpsdata)
    ON_COMMAND(IDM_PAGEDERV, &CDrawDoc::OnPagederv)
    ON_COMMAND(IDM_INSERTGRZBIET, &CDrawDoc::OnInsertGrzbiet)
    ON_UPDATE_COMMAND_UI(IDM_INSERTGRZBIET, &CDrawDoc::OnDisableGrbNotSaved)
    ON_COMMAND(IDM_SYNCDRV, &CDrawDoc::OnSyncDrv)
    ON_COMMAND(IDM_SETPAGINA, &CDrawDoc::OnSetPagina)
    ON_COMMAND(IDM_SETDEA, &CDrawDoc::OnSetDea)
    ON_UPDATE_COMMAND_UI(IDM_SETDEA, &CDrawDoc::OnUpdateSetDea)
    ON_COMMAND(ID_ADD_EXPORT, &CDrawDoc::OnExport)
    ON_COMMAND(ID_ADD_IMPORTMINUS, &CDrawDoc::OnFileImportMinus)
    ON_COMMAND(ID_ADD_DBIMPORTMINUS, &CDrawDoc::OnDBImportMinus)
    ON_COMMAND(ID_DRAW_NUMEROWANIE, &CDrawDoc::OnNumberPages)
    ON_COMMAND(ID_DRAW_PAGE, &CDrawDoc::OnAdd4Pages)
    ON_COMMAND(ID_FILE_INFO, &CDrawDoc::OnFileInfo)
    ON_COMMAND(ID_FILE_DRZEWO, &CDrawDoc::OnFileDrzewo)
    ON_COMMAND(ID_FILE_DBSAVE, &CDrawDoc::OnDBSave)
    ON_COMMAND(ID_FILE_DBSAVE_AS, &CDrawDoc::OnDBSaveAs)
    ON_COMMAND(ID_FILE_DBDELETE, &CDrawDoc::OnDBDelete)
    ON_UPDATE_COMMAND_UI(ID_FILE_DBSAVE_AS, &CDrawDoc::OnDisableDB)
    ON_UPDATE_COMMAND_UI(ID_FILE_DBDELETE, &CDrawDoc::OnDisableMenuRO)
    ON_UPDATE_COMMAND_UI(IDM_PAGEDERV, &CDrawDoc::OnDisableMenuRO)
    ON_UPDATE_COMMAND_UI(IDM_SYNCDRV, &CDrawDoc::OnDisableMenuRO)
    ON_COMMAND(IDM_SHOWTIME, &CDrawDoc::OnShowTime)
    ON_COMMAND(IDM_ACDEAD, &CDrawDoc::OnShowAcDeadline)
    ON_UPDATE_COMMAND_UI(IDM_SHOWTIME, &CDrawDoc::OnUpdateShowTime)
    ON_UPDATE_COMMAND_UI(IDM_ACDEAD, &CDrawDoc::OnUpdateShowAcDeadline)
    ON_COMMAND(ID_NARZ_KALKULATORKRAT, &CDrawDoc::OnKratCalc)
    ON_UPDATE_COMMAND_UI(ID_NARZ_KALKULATORKRAT, &CDrawDoc::OnDisableDB)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, COleDocument::OnUpdatePasteMenu)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_LINK, COleDocument::OnUpdatePasteLinkMenu)
    ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS, COleDocument::OnUpdateEditLinksMenu)
    ON_COMMAND(ID_OLE_EDIT_LINKS, COleDocument::OnEditLinks)
    ON_UPDATE_COMMAND_UI(ID_OLE_VERB_FIRST, COleDocument::OnUpdateObjectVerbMenu)
    ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_CONVERT, COleDocument::OnUpdateObjectVerbMenu)
    ON_COMMAND(ID_OLE_EDIT_CONVERT, COleDocument::OnEditConvert)
    ON_COMMAND(IDM_ACCGRB, &CDrawDoc::OnAccGrb)
    ON_UPDATE_COMMAND_UI(IDM_ACCGRB, &CDrawDoc::OnDisableGrbNotSaved)
    ON_COMMAND(ID_WIDOK_CHCAP, &CDrawDoc::OnChangeCaptions)
    ON_UPDATE_COMMAND_UI(ID_WIDOK_CHCAP, &CDrawDoc::OnDisableDB)
    ON_COMMAND(ID_WIDOK_COLPERROW, &CDrawDoc::OnChangeColsPerRow)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CDrawDoc::OnDisableMenuRO(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!isRO && !disableMenu && theApp.isRDBMS);
}

void CDrawDoc::OnDisableDB(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!disableMenu && theApp.isRDBMS);
}

void CDrawDoc::OnUpdateEpsdata(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!disableMenu && theApp.isRDBMS && theApp.grupa&(UserRole::mas | UserRole::stu));
}

void CDrawDoc::OnDisableGrbNotSaved(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!disableMenu && theApp.activeDoc->m_mak_xx > 0);
}
/////////////////////////////////////////////////////////////////////////////
// CDrawDoc construction/destruction

CDrawDoc::CDrawDoc() :
    isRO(0), isSIG(0), isGRB(0), isLIB(0), isACD(0),
    data(CTime::GetCurrentTime().Format(c_ctimeData))
{
    iPagesInRow = 10 + 2 * theApp.GetProfileInt(_T("General"), _T("PagesInRow"), 0);
    // font
    LOGFONT lf { 0 };
    lf.lfHeight = theApp.GetProfileInt(_T("Settings"), _T("PageFontSize"), -11);  //~8pt
    lf.lfPitchAndFamily = VARIABLE_PITCH;
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = EASTEUROPE_CHARSET;
    lf.lfOutPrecision = OUT_DEVICE_PRECIS;
    lf.lfClipPrecision = CLIP_LH_ANGLES;
    ::StringCchCopy(lf.lfFaceName, 32, theApp.GetProfileString(_T("Settings"), _T("PageFontFace"), _T("Arial Narrow")));
    if (!m_pagefont.CreateFontIndirect(&lf))
        m_pagefont.CreateStockObject(SYSTEM_FONT);

    lf.lfHeight = theApp.GetProfileInt(_T("Settings"), _T("AddFontSize"), -5);  //~4pt
    lf.lfPitchAndFamily = FF_SWISS | VARIABLE_PITCH;
    ::StringCchCopy(lf.lfFaceName, 32, theApp.GetProfileString(_T("Settings"), _T("AddFontFace"), _T("Small Fonts")));
    if (!m_addfont.CreateFontIndirect(&lf))
        m_addfont.CreateStockObject(SYSTEM_FONT);
    m_grzbietMenu.LoadMenu(IDR_MENUGRZBIET);
    ComputeCanvasSize();
}

CDrawDoc::~CDrawDoc()
{
    for (const auto& o : m_objects)
        delete o;

    for (const auto& p : m_pages)
        delete p;

    for (const auto& a : m_addsque)
        delete a;

    m_pagefont.DeleteObject();
    m_addfont.DeleteObject();
}

BOOL CDrawDoc::OnNewDocument()
{
    if (!COleDocument::OnNewDocument()) return FALSE;

    CString bf;
    const int n = theApp.GetProfileInt(_T("General"), _T("IloscKolumn"), 44);
    bf.Format(_T("%i"), n < 4 ? 44 : (n & 0xFC));

    if (!theApp.isRDBMS) return Add4Pages();
    if (theApp.isOpen) return DBOpenDoc();
    return AddDrz4Pages(bf);
}

void CDrawDoc::OnCloseDocument()
{
    if (!isRO && m_mak_xx > 0)
        theManODPNET.RmSysLock(this);

    COleDocument::OnCloseDocument();

    if (!((CMDIFrameWnd*)AfxGetMainWnd())->MDIGetActive())
        ((CMainFrame*)AfxGetMainWnd())->SetOpenStatus(_T(""));
}

BOOL CDrawDoc::SaveModified()
{
    /* vu :nadpisuje metodê domyœln¹ tak, aby uwzglêdniaæ zapis do bazy */

    if (IsModified() && ((m_mak_xx > 1 && !isGRB) || isLIB || isGRB)) {
        if (!isRO)
            switch (AfxMessageBox(_T("Czy chcesz zachowaæ makietê w bazie danych"), MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE)) {
                case IDCANCEL:
                    return FALSE;
                case IDYES:
                    OnDBSave();
                    break;
                case IDNO:
                    break;
            }
        return TRUE;
    }

    return CDocument::SaveModified();
}

void CDrawDoc::SetTitleAndMru(bool addRecentFiles)
{
    CString makieta;
    makieta.AppendFormat(_T("%s %s %s"), static_cast<LPCTSTR>(gazeta), static_cast<LPCTSTR>(opis), static_cast<LPCTSTR>(data));
    const auto len = makieta.GetLength();
    if (!isLIB) {
        ASSERT(data.GetLength() == 10);
        makieta.SetAt(len - 5, _T('-'));
        makieta.SetAt(len - 8, _T('-'));
    }

    SetTitle(CString(isGRB ? "Grzbiet drukowany " : "Makieta ") + makieta + (isRO ? ((theApp.grupa&UserRole::dea) ? " [tylko do rezerwacji]" : " [tylko do odczytu]") : ""));
    if (!addRecentFiles) return;

    ASSERT(gazeta.GetLength() == 6);
    const auto posDate = makieta.ReverseFind(_T(' '));
    const auto makBuf = makieta.GetBuffer();
    memcpy(static_cast<void*>(6 + makBuf), static_cast<void*>(makBuf + posDate), (len - posDate + 1) * sizeof(TCHAR));
    makieta.SetAt(6, _T('_'));
    makieta.GetBufferSetLength(len - posDate + 6);
    makieta.Append(asDocTypeExt[static_cast<int>(iDocType)]);
    theApp.AddToRecentFileList(makieta);
}

void CDrawDoc::Serialize(CArchive& ar)
{
    size_t i;
    LOGFONT lf;
    if (ar.IsStoring()) {
        m_pagefont.GetObject(sizeof(LOGFONT), &lf);
        ar.Write(&lf, sizeof(LOGFONT));
        m_addfont.GetObject(sizeof(LOGFONT), &lf);
        ar.Write(&lf, sizeof(LOGFONT));
        ar << (WORD)id_drw;
        ar << gazeta;
        ar << data;
        ar << prowadzacy1;
        ar << prowadzacy2;
        ar << sekretarz;
        ar << symWydawcy;
        //serializacja spotow
        ar << (WORD)m_spot_makiety.size();
        for (const auto sp : m_spot_makiety)
            ar << sp;
        ar << (WORD)m_pages.size();
        for (const auto& pPage : m_pages) {
            pPage->CleanKraty(FALSE);
            pPage->Serialize(ar);
        }
        CObList tmpList;
        for (const auto& pObj : m_objects)
            tmpList.AddTail(pObj);
        tmpList.Serialize(ar);
    } else {
        ar.Read(&lf, sizeof(LOGFONT));
        if (m_pagefont.m_hObject != nullptr) m_pagefont.DeleteObject();
        m_pagefont.CreateFontIndirect(&lf);
        ar.Read(&lf, sizeof(LOGFONT));
        if (m_addfont.m_hObject != nullptr) m_addfont.DeleteObject();
        m_addfont.CreateFontIndirect(&lf);

        WORD wTemp;
        ar >> wTemp; id_drw = wTemp;
        ar >> gazeta;
        ar >> data;
        ar >> prowadzacy1;
        ar >> prowadzacy2;
        ar >> sekretarz;
        ar >> symWydawcy;
        // serializacja spotow
        ar >> wTemp; m_spot_makiety.resize(wTemp);
        DWORD tm;
        for (i = 0; i < wTemp; ++i) {
            ar >> tm; m_spot_makiety[i] = (UINT)tm;
        }
        // serializacja stron
        ar >> wTemp;
        m_pages.clear();
        while (wTemp-- > 0) {
            auto pPage = new CDrawPage();
            pPage->Serialize(ar);
            m_pages.push_back(pPage);
        }
        // serializacja og³oszeñ i opisów
        CObList tmpList;
        tmpList.Serialize(ar);
        POSITION pos = tmpList.GetHeadPosition();
        while (pos)
            m_objects.push_back((CDrawObj*)tmpList.GetNext(pos));
    }
    COleDocument::Serialize(ar);
}


///////////////RYSOWANIE I DRUKOWANIE ////////////////////////////////////////////////////////
// CDrawDoc implementation
void CDrawDoc::Draw(CDC* pDC, CDrawView* pView)
{
    if (isRO) {
        CRect r;
        pView->GetClientRect(r);
        pDC->DPtoLP(r);
        pDC->FillRect(r, &((CMainFrame*)AfxGetMainWnd())->robgcolor);
    }

    for (const auto& p : m_pages) {
        p->Draw(pDC);
        if (!pDC->IsPrinting() && pView->IsSelected(p))
            p->DrawTracker(pDC, CDrawObj::selected);
    }

    const auto psize = m_pages.size();
    if (psize > 0 && ((psize & 1) == 0)) { // makieta ma rozkladowke
        CDrawPage* pPage = m_pages[psize / 2];
        const CRect& pRozkl = pPage->m_position;
        CRect r(pRozkl.right - vscale, pRozkl.top - pmoduly - vscale, pRozkl.right + vscale, pRozkl.bottom + vscale);
        pDC->FillRect(r, pPage->pagina_type == PaginaType::roman ? (CBrush*)pDC->SelectStockObject(WHITE_BRUSH) : &((CMainFrame*)AfxGetMainWnd())->rzym);
    }

    for (const auto& pObj : m_objects) {
        pObj->Draw(pDC);
        if (!pDC->IsPrinting() && pView->IsSelected(pObj))
            pObj->DrawTracker(pDC, CDrawObj::selected);
    }

    if (theApp.showDeadline)
        for (const auto& p : m_pages)
            p->DrawDeadline(pDC, &p->m_position);
    if (theApp.showAcDeadline)
        for (const auto& p : m_pages)
            p->DrawAcDeadline(pDC, &p->m_position);
}

void CDrawDoc::DrawQue(CDC* pDC, CQueView *pView)
{
    for (const auto& a : m_addsque) {
        a->Draw(pDC);
        if (a == CQueView::selected_add) a->DrawTracker(pDC, CDrawObj::selected);
    }

    if (theApp.unQueing && CQueView::selected_add) {
        const CRect& pos = CQueView::selected_add->m_position;
        pDC->MoveTo(pos.left, pos.top);
        pDC->LineTo(pos.right, pos.bottom);
        pDC->MoveTo(pos.right, pos.top);
        pDC->LineTo(pos.left, pos.bottom);
    }
}

void CDrawDoc::DrawPageCross(CDC* pDC)
{
    CPoint p0(0, 0);
    CPoint p(pDC->GetDeviceCaps(HORZRES), pDC->GetDeviceCaps(VERTRES));
    pDC->DPtoLP(&p0);
    pDC->DPtoLP(&p);
    pDC->MoveTo(p0.x, p0.y);
    pDC->LineTo(p.x, p.y);
    pDC->MoveTo(p.x, p0.y);
    pDC->LineTo(p0.x, p.y);
}

void CDrawDoc::Print(CDC* pDC)
{
    for (const auto& p : m_pages)
        p->Print(pDC);

    for (const auto& pObj : m_objects) { // og³oszenia
        auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
        if (pAdd) pAdd->Print(pDC);
    }

    for (const auto& pObj : m_objects) { // opisy maj¹ przykrywaæ og³oszenia
        auto pOpi = dynamic_cast<CDrawOpis*>(pObj);
        if (pOpi) pOpi->Print(pDC);
    }

    for (const auto& pObj : m_objects) { // a jeszcze na wierzchu nazwa i eps ogl
        const auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
        if (pAdd) {
            const CRect& rect = pAdd->GetPrintRect();
            pAdd->DrawDesc(pDC, &rect);
        }
    }

    if (!isSIG && theApp.isRDBMS)
        DrawPageCross(pDC);

    if (theApp.showDeadline)
        for (const auto& p : m_pages)
            p->DrawDeadline(pDC, &p->GetPrintRect());
}

void CDrawDoc::PrintPage(CDC* pDC, CDrawPage* pPage)
{
    pPage->Print(pDC);

    for (const auto& pAdd : pPage->m_adds)
        pAdd->Print(pDC);

    for (const auto& pObj : m_objects) { // dodatkowy opis zeby opis byl na wierzchu przed ogl
        auto pOpi = dynamic_cast<CDrawOpis*>(pObj);
        if (pOpi && pPage->Intersects(pOpi->m_position)) pOpi->Print(pDC);
    }

    const CPoint p = pDC->GetWindowOrg();
    const CSize s = pDC->ScaleViewportExt(1, 2, 1, 2);

    for (const auto& pAdd : pPage->m_adds) {
        pDC->SetWindowOrg(2 * (pPage->m_position.left / CLIENT_SCALE - 5 * vscale) - pAdd->m_position.left / CLIENT_SCALE,
            2 * (pPage->m_position.top / CLIENT_SCALE - 18 * vscale + pmoduly) - pAdd->m_position.top / CLIENT_SCALE + 2 * (pPage->pagina / 51) * pmoduly);
        const auto& rect = pAdd->GetPrintRect();
        pAdd->DrawDesc(pDC, rect);
    }

    pDC->SetViewportExt(s);
    pDC->SetWindowOrg(p);
    if (theApp.showDeadline)
        pPage->DrawDeadline(pDC, &pPage->GetPrintRect());
    pDC->SetViewportOrg(0, 0);
    pDC->SetWindowOrg(0, 0);
    if (!isSIG && theApp.isRDBMS)
        DrawPageCross(pDC);
}

////////// CDRAWOBJ _M_OBJECTS LIST ?/////////////////////////////       
void CDrawDoc::Add(CDrawObj* pObj)
{
    pObj->m_pDocument = this;
    m_objects.push_back(pObj);
    SetModifiedFlag();
}

void CDrawDoc::AddQue(CDrawAdd* pObj)
{
    pObj->fizpage = 0;
    pObj->m_pDocument = this;
    m_addsque.push_back(pObj);
    SetModifiedFlag();
}

void CDrawDoc::RemoveQue(CDrawAdd* pObj)
{
    const auto pos = std::find(m_addsque.cbegin(), m_addsque.cend(), pObj);
    if (pos != m_addsque.cend()) m_addsque.erase(pos);
    SetModifiedFlag();
}

void CDrawDoc::Remove(CDrawObj* pObj)
{
    auto pPage = dynamic_cast<CDrawPage*>(pObj);
    if (pPage)
        RemovePage(pPage);
    else {
        const auto pos = std::find(m_objects.begin(), m_objects.end(), pObj);
        if (pos != m_objects.end()) {
            auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
            if (pAdd) {
                if (pAdd->m_pub_xx != -1 && pObj != CQueView::selected_add)
                    m_del_obj.emplace_back(EntityType::add, pAdd->m_pub_xx);
            } else {
                auto pOpis = dynamic_cast<CDrawOpis*>(pObj);
                if (pOpis && pOpis->m_opi_xx != -1)
                    m_del_obj.emplace_back(isLIB ? EntityType::opis_lib : EntityType::opis, pOpis->m_opi_xx);
            }
            m_objects.erase(pos);
        }
    }

    SetModifiedFlag();
    // call remove for each view so that the view can remove from m_selection
    auto pView = GetPanelView<CDrawView>();
    if (pView) pView->Remove(pObj);
}

CDrawObj* CDrawDoc::ObjectAt(const CPoint& point) const
{
    auto coll = const_cast<std::vector<CDrawObj*>*>(&m_objects); // use span<CDrawObj*> in C++ 20
check:
    auto found = std::find_if(std::rbegin(*coll), std::rend(*coll), [&point](const auto& p) {return p->Contains(point); });
    if (found != std::rend(*coll))
        return *found;

    if ((intptr_t)coll != (intptr_t)&m_pages) {
        coll = reinterpret_cast<decltype(coll)>(const_cast<std::vector<CDrawPage*>*>(&m_pages));
        goto check;
    }

    return nullptr;
}

CDrawAdd* CDrawDoc::ObjectAtQue(const CPoint& point) const
{
    for (const auto& a : m_addsque)
        if (a->Contains(point))
            return a;

    return nullptr;
}

CDrawAdd* CDrawDoc::FindAddAt(int i) const
{
    int n = 0;

    for (const auto& pObj : m_objects) {
        auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
        if (pAdd && ++n == i) return pAdd;
    }

    return nullptr;
}

int CDrawDoc::GetAdPosition(const CDrawAdd* pAdd) const
{
    int n = 0;
    const auto count_position = [&](auto p) noexcept->bool {
        if (dynamic_cast<CDrawAdd*>(p) == nullptr) return false;
        ++n;
        return pAdd == p;
    };

    const auto pos = std::find_if(m_objects.cbegin(), m_objects.cend(), count_position);
    return pos == m_objects.cend() ? -1 : n;
}

void CDrawDoc::SelectAdd(CDrawAdd* pObj, bool multiselect) const
{
    auto pView = GetPanelView<CDrawView>();
    if (pView) pView->Select(pObj, multiselect);
}

int CDrawDoc::AddsCount() const noexcept
{
    const auto is_ad = [](auto p) noexcept { return dynamic_cast<CDrawAdd*>(p) != nullptr; };
    return (int)std::count_if(std::cbegin(m_objects), std::cend(m_objects), is_ad);
}

//import i import++
void CDrawDoc::RemoveFromHead(int n)
{
    for (int i = 0; i < n; ++i)
        delete m_objects[i];
    m_objects.erase(m_objects.begin(), m_objects.begin() + n);
}

void CDrawDoc::RemoveFromTail(int n)
{
    for (int i = 0; i < n; ++i) {
        delete m_objects.back();
        m_objects.pop_back();
    }
}

/////////////////////////////////////////////////////////////////////////////
//CDrawPage
void CDrawDoc::AddPageAt(size_t idx, CDrawPage* pObj)
{
    m_pages.insert(m_pages.cbegin() + idx, pObj);
    pObj->m_pDocument = this;
    SetModifiedFlag();
}

size_t CDrawDoc::AddPage(CDrawPage* pObj)
{
    size_t iNewOrd = m_pages.size();
    m_pages.push_back(pObj);
    pObj->m_pDocument = this;
    SetPageRectFromOrd(pObj, iNewOrd);
    ComputeCanvasSize();
    if (!(++iNewOrd & 3))
        ZmianaSpotow((int)iNewOrd);
    NumberPages();
    SetModifiedFlag();
    return iNewOrd;
}

void CDrawDoc::RemovePage(CDrawPage* pObj)
{
    int i = GetIPage(pObj);
    if (i == -1)
        return;

    if (i == 0 && !isLIB)  // usuwamy ostatnia strone, na ktorej moga byc virtualne ogloszenia
        for (const auto& a : m_objects) {
            auto pAdd = dynamic_cast<CDrawAdd*>(a);
            if (pAdd && !pAdd->fizpage) pAdd->SetDirty();
        }

    // przesuwam strony na obrazku by nie zostala dziura
    for (size_t ii = i + 1; ii <= m_pages.size() - 1; ++ii)
        MoveOpisAfterPage(&m_pages[ii]->m_position, &m_pages[ii - 1]->m_position);
    for (size_t ii = m_pages.size() - 1; ii > (size_t)i; --ii)
        m_pages[ii]->MoveTo(&m_pages[ii - 1]->m_position);

    if (pObj->id_str != -1)
        m_del_obj.emplace_back(isLIB ? EntityType::page_lib : EntityType::page, pObj->id_str);
    m_pages.erase(m_pages.cbegin() + i);
    SetModifiedFlag();

    // call remove for each view so that the view can remove from m_selection
    auto pView = GetPanelView<CDrawView>();
    if (pView) pView->Remove(pObj);

    //zmiana spotow
    if (!(m_pages.size() & 3))
        ZmianaSpotow((int)m_pages.size());
}

bool CDrawDoc::MoveOpisAfterPage(const CRect& rFrom, const CRect& rTo)
{
    bool found = false;
    CRect rFromN = rFrom;
    rFromN.NormalizeRect();
    for (const auto& pObj : m_objects)
        if (dynamic_cast<CDrawOpis*>(pObj) && rFromN.PtInRect(pObj->m_position.CenterPoint())) {
            found = true;
            pObj->m_position += CPoint(rTo.left - rFrom.left, rTo.top - rFrom.top);
            pObj->SetDirty();
        }

    return found;
}

void CDrawDoc::SetPageRectFromOrd(CDrawPage* pObj, size_t iOrd) const
{
    ASSERT(0 <= iOrd && iOrd < m_pages.size());

    const auto x = (int)(pmodulx*(1 + floor(fmod((float)iOrd, (float)iPagesInRow) / 2) + (pszpalt_x*fmod((float)iOrd, (float)iPagesInRow))));
    const auto y = (int)((-7)*pmoduly*floor((float)iOrd / iPagesInRow) + (-1)*(pmoduly*(floor((float)iOrd / iPagesInRow) + 1)));

    pObj->m_position.SetRect(x, y, x + pszpalt_x*pmodulx, y - 7 * pmoduly);
    pObj->MoveTo(&pObj->m_position);
}

int CDrawDoc::ComputePageOrderNr(const CRect& position) const
{
    int i = 0, j = 0;
    while (pmoduly*(i + 1)*(-8) >= position.top)
        i++;
    while (j < iPagesInRow && pmodulx*((int)(1 + floor((float)j / 2) + pszpalt_x*(j + 0.5))) < position.left)
        j++;
    return min((int)m_pages.size() - 1, i * iPagesInRow + j);
}

void CDrawDoc::MoveBlockOfPages(int iSrcOrd, int iDstOrd, int iCnt)
{
    ASSERT(iCnt > 0);
    const int iPocz = min(iSrcOrd, iDstOrd);
    const int iKon = min(max(iSrcOrd, iDstOrd) + iCnt, (int)m_pages.size());
    ASSERT(0 <= iPocz && iKon <= (int)m_pages.size());

    CPoint pNowhere(INT_MAX >> 2, 0);
    int i, iOldOrd, iBufSize = iKon - iPocz;
    auto aNoPagePos = (CRect*)theApp.bigBuf;

    // przywróæ pozycjê przesuwanym stronom
    for (i = 0; i < iCnt; ++i)
        SetPageRectFromOrd(m_pages[iSrcOrd + i], iSrcOrd + i);
    // przenieœ opisy stron z zakresu rotacji
    for (i = iPocz; i < iKon; ++i) {
        aNoPagePos[i] = (m_pages[i])->m_position - pNowhere;
        if (!MoveOpisAfterPage(&m_pages[i]->m_position, &aNoPagePos[i]))
            aNoPagePos[i] = nullptr;
    }
    // rotacja stron, Pages[i] ==> Pages[iPocz+(i-iPocz+iDstOrd-iSrcOrd)%iBufSize]
    int iShift = iBufSize + iSrcOrd - iDstOrd;
    std::rotate(begin(m_pages) + iPocz, begin(m_pages) + iPocz + iShift % iBufSize, begin(m_pages) + iKon);
    // przesuñ opisy
    for (i = iPocz; i < iKon; ++i) {
        SetPageRectFromOrd(m_pages[i], i);
        iOldOrd = iPocz + (i - iPocz + iShift) % iBufSize;
        if (aNoPagePos[iOldOrd] != nullptr)
            MoveOpisAfterPage(&aNoPagePos[iOldOrd], &(m_pages[i])->m_position);
        m_pages[i]->SetDirty();
    }
    // og³oszenia poza makiet¹ musz¹ byæ zmienione w bazie
    if (iPocz == 0 && !isLIB)
        for (const auto& pObj : m_objects) {
            auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
            if (pAdd && !pAdd->fizpage) pAdd->SetDirty();
        }

    SetModifiedFlag();
    UpdateAllViews(nullptr);
}

int CDrawDoc::GetIdxfromSpotID(UINT spot_id) noexcept
{
    auto pos = std::find(begin(spoty), end(spoty), spot_id);
    return pos == end(spoty) ? 0 : (int)std::distance(begin(spoty), pos);
}

CDrawPage* CDrawDoc::GetPage(int n) const
{
    for (const auto& p : m_pages)
        if (p->nr == n)
            return p;
    return nullptr;
}

CDrawPage* CDrawDoc::PageAt(const CPoint& point) const
{
    for (const auto& p : m_pages)
        if (p->Contains(point))
            return p;
    return nullptr;
}

void CDrawDoc::ComputeCanvasSize()
{
    CClientDC dc{nullptr};
    auto r = div((int)m_pages.size(), iPagesInRow);
    if (r.quot < 3) r.quot = 3;
    else if (r.rem > 0) r.quot++;
    const CSize new_size(dc.GetDeviceCaps(HORZRES), m_pages.empty() ? dc.GetDeviceCaps(VERTRES) : (int)(pmoduly / vscale * (1 + 8 * r.quot)));

    // if size changed then iterate over views and reset
    if (new_size != m_size) {
        m_size = new_size;
        auto *pView = GetPanelView<CDrawView>();
        if (pView) pView->SetPageSize();
    }
}

BOOL CDrawDoc::OnOpenDocument(LPCTSTR pszPathName)
{
    if (disableMenu) return FALSE;
    if (_tcsstr(pszPathName, _T(".LIB\0"))) {
        isLIB = TRUE;
        iDocType = DocType::makieta_lib;
    } else if (_tcsstr(pszPathName, _T(".GRB\0"))) {
        isGRB = TRUE;
        iDocType = DocType::grzbiet_drukowany;
    }
    if (isGRB || isLIB || _tcsstr(pszPathName, _T(".DB\0"))) {
        TCHAR makieta[MAX_PATH];
        swCZV = theApp.initCZV;
        ::StringCchCopy(makieta, MAX_PATH, pszPathName);
        return DBOpenDoc(makieta);
    }

    if (CDocument::OnOpenDocument(pszPathName)) {
        ComputeCanvasSize();
        return TRUE;
    }

    return FALSE;
}
//////////////////////////////////////////////////////////////////////
////////// SAVE AS

void CDrawDoc::OnFileSave()
{
    m_mak_xx > 0 ? OnFileSaveAs() : (void)CDocument::DoSave(GetPathName());
}

void CDrawDoc::OnFileSaveAs()
{
    CString fname = GetPathName();
    if (fname.IsEmpty())
        fname = _T("*");
    else {
        const int pos = fname.ReverseFind('.');
        if (pos > -1) fname.Truncate(pos);
    }

    CFileDialog dlg(FALSE, nullptr, fname, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("manam (*.man)|*.man|Wszystkie pliki (*.*)|*.*||"));
    if (dlg.DoModal() != IDOK)
        return;

    if (m_mak_xx > 0 && this->isRO == 0)
        theManODPNET.RmSysLock(this);
    m_mak_xx = -1;
    CDocument::DoSave(CString(dlg.m_ofn.lpstrFile) + _T(".man"));
}

//////////////////////////////////////////////////////////////////////
////////// IMPORT 

void CDrawDoc::OnImport(bool fromDB)
{
    const auto n = (int)m_objects.size();
    if ((fromDB) ? DBImport() : Import(true)) {
        auto pView = GetPanelView<CDrawView>();
        if (pView && pView->m_bActive) pView->Select(nullptr, FALSE);
        RemoveFromHead(n);
        UpdateAllViews(nullptr, HINT_UPDATE_WINDOW, nullptr);
    } else
        RemoveFromTail((int)m_objects.size() - n);
}

void CDrawDoc::OnFileImportPlus()
{
    OnImportPlus(false);
}

void CDrawDoc::OnFileImportMinus()
{
    Import(true);
    UpdateAllViews(nullptr, HINT_UPDATE_WINDOW, nullptr);
}

void CDrawDoc::OnDBImportMinus()
{
    DBImport(false);
    UpdateAllViews(nullptr, HINT_UPDATE_WINDOW, nullptr);
}

void CDrawDoc::OnAddSynchronize()
{
    DBImport(true);
    UpdateAllViews(nullptr, HINT_UPDATE_WINDOW, nullptr);
}

void CDrawDoc::OnDBImportPlus()
{
    OnImportPlus(true);
}

void CDrawDoc::OnImportPlus(bool fromDB)
{
    const auto n = m_objects.size();
    if (!(fromDB ? DBImport() : Import(true)))
        RemoveFromTail((int)(m_objects.size() - n));
    UpdateAllViews(nullptr, HINT_UPDATE_WINDOW, nullptr);
}

bool CDrawDoc::Import(bool check_exist) // tu dodaje na koncu do m_objects najwyzej sie potem usunie 
{
    CFileDialog dlg(TRUE, _T("txt"), _T(""), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("Og³oszenia (*.txt)|*.txt| Wszystkie pliki (*.*)|*.*||"), nullptr);
    if (dlg.DoModal() != IDOK)
        return false;

    bool ok{true};
    try {
        auto& buf = theApp.bigBuf;
        CStdioFile file(dlg.m_ofn.lpstrFile, CFile::modeRead | CFile::typeUnicode);
        CPoint pos = GetAsideAddPos(false);
        while (file.ReadString(buf, n_size) != nullptr && ok)
            ok = CreateAdd(buf, '\t', pos, check_exist);

        file.Close();
    } catch (CException* ex) {
        ex->ReportError();
        ex->Delete();
    }

    if (!ok)
        AfxGetMainWnd()->MessageBox(_T("Dane w pliku s¹ uszkodzone"), _T("B³¹d"), MB_OK);
    return ok;
}

bool CDrawDoc::CreateAdd(LPCTSTR adBuf, const TCHAR sepChar, CPoint& pos, bool check_exist)
{ // tworzenie ogloszen importowanych z pliku tekstowego
    enum nazcstr : uint8_t
    {
        kratka,
        fizpag,
        pox,
        poy,
        nazw,
        nrep,
        kolo,
        logp,
        remark,
        wer,
        dodomu
    } i = theApp.includeKratka ? kratka : fizpag;

    CString p_remarks, p_nazwa, p_logpage, p_wersja;
    UINT p_kolor = 0;
    long p_nreps = 0;
    TCHAR st[21];

    int sx, sy, fizpage = 0, posx = 0, posy = 0;
    // interesuj¹ nas tylko linie które zaczynaj¹ siê od rozmiaru KxW
    if (adBuf == nullptr || _stscanf_s(adBuf, _T("%ix%i"), &sx, &sy) != 2)
        return FALSE;

    int szpalt_x = pszpalt_x;
    int szpalt_y = pszpalt_y;
    adBuf = _tcschr(adBuf, sepChar) + 1;

    while (*adBuf != _T('\n') && i != dodomu) {
        auto pdest = _tcschr(adBuf, sepChar);
        auto result = (pdest != nullptr) ? ((pdest - adBuf) > 20 ? 20 : pdest - adBuf) : 0;
        ::StringCchCopyN(st, 21, adBuf, result);
        st[result] = 0;

        if (result == 0 && pdest == nullptr) // urwana linia
            break;

        switch (i) {
            case kratka:
                _stscanf_s(st, _T("%ix%i"), &szpalt_x, &szpalt_y);
                i = fizpag;
                break;
            case fizpag: fizpage = _istdigit(st[0])
                ? (_ttoi(st) << 16) + PaginaType::arabic
                : (CDrawObj::Arabska(st) << 16) + PaginaType::roman;
                i = pox;
                break;
            case pox: posx = ((fizpage == 0) ? 0 : (result == 0 ? szpalt_x + 1 - sx : _ttoi(st))); i = poy; break;
            case poy: posy = ((fizpage == 0) ? 0 : (result == 0 ? szpalt_y + 1 - sy : _ttoi(st))); i = nazw; break;
            case nazw: p_nazwa = st; i = nrep; break;
            case nrep:
            {
                p_nreps = _ttol(st);
                if (p_nreps == 0) p_nreps = -1;
                i = kolo;
            }
            break;
            case kolo: p_kolor = (result == 0 ? ColorId::brak : ValidKolor(st)); i = logp; break;
            case logp: p_logpage = CString(st).Left(5); i = remark; break;
            case remark: p_remarks = st; i = wer; break;
            case wer: p_wersja = CString(st).Left(5); i = dodomu; break;
        }

        if (pdest == nullptr)
            i = dodomu;
        else if (i != dodomu)
            adBuf = ++pdest;
    }

    if (check_exist && AddExists(p_nreps) != nullptr) {
        CString msg;
        msg.Format(_T("W danych Ÿród³owych wystêpuje wiêcej ni¿ jedno og³oszenie o numerze %ld. Wczytane zostanie tylko pierwsze z tych og³oszeñ."), p_nreps);
        AfxMessageBox(msg, MB_ICONEXCLAMATION);
        return false;
    }

    const CRect r(pos, CSize(sx * pmodulx, -sy * pmoduly));
    auto pObj = new CDrawAdd(r);
    Add(pObj);
    pObj->nazwa = p_nazwa;
    pObj->nreps = p_nreps;
    pObj->kolor = p_kolor;
    pObj->logpage = p_logpage;
    pObj->remarks = p_remarks;
    pObj->wersja = p_wersja;
    pObj->szpalt_x = szpalt_x;
    pObj->szpalt_y = szpalt_y;
    if (fizpage && (szpalt_x != pszpalt_x || szpalt_y != pszpalt_y)) {
        CDrawPage* vPage = GetPage(fizpage);
        if (vPage) vPage->SetBaseKrata(szpalt_x, szpalt_y, TRUE);
    }
    pObj->SetPosition(fizpage, posx, posy, sx, sy);
    if (pObj->fizpage == 0) {
        pos.y -= pmoduly;
        if (pszpalt_y*pmoduly - pos.y > m_size.cy*vscale) {
            pos.y = -pmoduly; pos.x -= pszpalt_x*pmodulx;
        }
    }
    pObj->Invalidate();
    return true;
}

int CDrawDoc::ValidKolor(const CString& k) noexcept
{
    const auto len = (int)kolory.size();
    for (int i = 1; i < len; ++i)
        if (kolory[i].Find(k) == 0)
            return (i == 1) ? ColorId::full : (i << 3) + ColorId::spot;

    return ColorId::brak;
}

CBrush* CDrawDoc::GetSpotBrush(int i) noexcept
{
    return i >= (int)brushe.size() || i < 0 ? static_cast<CBrush*>(::GetStockObject(BLACK_BRUSH)) : brushe[i];
}

void CDrawDoc::SetSpotKolor(UINT idx_m_spot_makiety, UINT idx_Spot_Kolor)
{
    m_spot_makiety[idx_m_spot_makiety] = (m_spot_makiety[idx_m_spot_makiety] == idx_Spot_Kolor) ? 0 : idx_Spot_Kolor;
    for (const auto& p : m_pages)
        if (p->kolor == ((idx_m_spot_makiety << 3) + ColorId::spot))
            p->Invalidate();
    SetModifiedFlag();
}

// EXPORT

void CDrawDoc::OnExport()
{
    CFileDialog dlg(FALSE, _T("txt"), _T("add.txt"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("Og³oszenia (*.txt) |*.txt| Wszystkie pliki (*.*)|*.*||"), nullptr);
    if (dlg.DoModal() != IDOK)
        return;

    try {
        CStdioFile file(dlg.m_ofn.lpstrFile, CFile::modeWrite | CFile::typeUnicode | CFile::modeCreate);
        for (const auto& pObj : m_objects) {
            auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
            if (pAdd)
                file.WriteString(pAdd->PrepareBuf(_T("\t")));
        }

        file.Close();
    } catch (CException* ex) {
        ex->ReportError();
        ex->Delete();
    }
}

// NUMEROWANIE I DODAWANIE STRON

void CDrawDoc::NumberPages()
{
    int iFirstPageNumber = 1;
    int iFirstPageToNumber = 1;
    // dla stron arabskich dopuszczamy przenumerowanie od zadanej
    auto pView = GetPanelView<CDrawView>();
    if (pView->m_selection.size() == 1) {
        auto pSelPage = dynamic_cast<CDrawPage*>(pView->m_selection.front());
        if (pSelPage != nullptr && pSelPage->pagina_type == PaginaType::arabic) {
            iFirstPageNumber = pSelPage->pagina;
            iFirstPageToNumber = GetIPage(pSelPage);
            if (iFirstPageToNumber < 0) return; // wystêpuje podczas copy/paste grupy stron
        }
    }
    // sprawdzamy, czy nie powstan¹ duble w numeracji
    const auto pc = (int)m_pages.size();
    const int iLastPageNumber = iFirstPageNumber + pc - iFirstPageToNumber;
    for (int i = 1; i < iFirstPageToNumber; ++i) {
        const int iNrToCheck = m_pages[i]->pagina;
        if (iFirstPageNumber <= iNrToCheck && iNrToCheck <= iLastPageNumber) {
            iFirstPageNumber = iFirstPageToNumber = 1;
            break;
        }
    }
    // przenumerowujemy strony od pocz¹tku lub zaznaczonej a¿ do ostatniej
    if (theApp.GetProfileInt(_T("General"), _T("CiaglaNumeracja"), 0))
        for (int i = iFirstPageToNumber; i <= pc; ++i) {
            auto pObj = m_pages[i % pc];
            pObj->SetNr(((iFirstPageNumber++) << 16) + pObj->pagina_type);
    } else {
        int rz = 0;
        for (int i = iFirstPageToNumber; i <= pc; ++i) {
            auto pObj = m_pages[i % pc];
            if (pObj->pagina_type == PaginaType::roman) {
                pObj->SetNr(((++rz) << 16) + PaginaType::roman);
                iFirstPageNumber++;
            } else
                pObj->SetNr((((iFirstPageNumber++) - rz) << 16) + PaginaType::arabic);
        }
    }
    // przenumerowanie od ostatniej wymaga przywrócenia numeru
    if (iFirstPageToNumber == 0) {
        auto pPage = m_pages[0];
        pPage->SetNr(((--iFirstPageNumber - pc) << 16) + pPage->pagina_type);
    }

    UpdateAllViews(nullptr);
}

void CDrawDoc::OnNumberPages()
{
    NumberPages();
    SetModifiedFlag(TRUE);  //vu
}

void CDrawDoc::OnAdd4Pages()
{
    Add4Pages();
}

bool CDrawDoc::Add4Pages()
{
    CKolDlg dlg;
    if (dlg.DoModal() != IDOK)
        return false;

    const CRect r{nullptr};
    const int n = _ttoi(dlg.m_ile_kolumn);
    const size_t last = m_pages.size();
    for (size_t i = last; i < last + n; ++i) {
        auto pNewPage = new CDrawPage(r);
        if (last > 1) {
            const auto pJedynka = m_pages[1];
            pNewPage->m_drukarnie = pJedynka->m_drukarnie;
            pNewPage->m_deadline = pJedynka->m_deadline;
            pNewPage->wyd_xx = pJedynka->wyd_xx;
        }
        AddPageAt(i, pNewPage);
        SetPageRectFromOrd(pNewPage, i);
    }

    ZmianaSpotow((int)last + n);
    NumberPages();
    ComputeCanvasSize();
    UpdateAllViews(nullptr);

    return true;
}

bool CDrawDoc::AddDrz4Pages(LPCTSTR ile_kolumn)
{
    CDrzDlg dlg;
    dlg.m_ile_kolumn = ile_kolumn;
    if (dlg.DoModal() != IDOK)
        return false;

    gazeta = dlg.m_gazeta;
    id_drw = dlg.m_id_drw;

    const CRect r{nullptr};
    const int n = _ttoi(dlg.m_ile_kolumn);
    const auto last = (int)m_pages.size();
    for (int i = last; i < last + n; ++i) {
        auto pNewPage = new CDrawPage(r);
        AddPageAt(i, pNewPage);
        SetPageRectFromOrd(pNewPage, i);
    }

    ZmianaSpotow(n);
    NumberPages();
    ComputeCanvasSize();
    UpdateAllViews(nullptr);
    return true;
}

void CDrawDoc::ModCount(UINT* m_modogl, UINT* m_modred, UINT* m_modrez, UINT* m_modwol) const
{
    UINT l_modogl, l_modred, l_modrez;
    *m_modogl = *m_modred = *m_modrez = 0;
    for (const auto& vPage : m_pages) {
        if (vPage->m_dervlvl == DervType::proh)
            continue;
        l_modogl = l_modred = l_modrez = 0;
        int pmods = vPage->szpalt_x * vPage->szpalt_y;
        for (int j = 0; j < pmods; ++j) {
            if (vPage->space_red[j]) l_modred++;
            else if (vPage->space_locked[j]) l_modrez++;
            else if (vPage->space[j]) l_modogl++;
        }
        if (pmods != pmodcnt) {
            const auto norm = (float)pmodcnt / pmods;
            l_modogl = (UINT)nearbyintf((float)l_modogl * norm);
            l_modred = (UINT)nearbyintf((float)l_modred * norm);
            l_modrez = (UINT)nearbyintf((float)l_modrez * norm);
        }
        *m_modogl += l_modogl; *m_modred += l_modred; *m_modrez += l_modrez;

        // og³oszenia dla krat niebazowych
        const int init_szpalt_x = vPage->szpalt_x;
        const int init_szpalt_y = vPage->szpalt_y;
        for (const auto& kn : vPage->m_kraty_niebazowe) {
            const int sx = kn.m_szpalt_x;
            const int sy = kn.m_szpalt_y;
            vPage->SetBaseKrata(sx, sy, TRUE);
            l_modogl = l_modred = l_modrez = 0;
            pmods = sx * sy;
            for (int j = 0; j < pmods; ++j)
                if (vPage->space[j] && !vPage->space_red[j] && !vPage->space_locked[j])
                    l_modogl++;
            if (pmods != pmodcnt)
                l_modogl = (UINT)nearbyintf((float)(l_modogl * pmodcnt) / pmods);
            *m_modogl += l_modogl;
        }
        vPage->SetBaseKrata(init_szpalt_x, init_szpalt_y, TRUE);
    }

    *m_modwol = (UINT)m_pages.size()*pmodcnt - *m_modogl - *m_modred - *m_modrez;
    if (*m_modwol > (UINT)m_pages.size()*pmodcnt) *m_modwol = 0;
}

inline float CDrawDoc::PowAdd2Mod(bool bQueStat) const
{
    float pow = 0.0;
    auto mod_count = [](const CDrawAdd* a) noexcept -> float { return (float)(a->sizex * a->sizey * pmodcnt) / (a->szpalt_x * a->szpalt_y); };

    if (bQueStat) {
        for (const auto& a : m_addsque)
            pow += mod_count(a);
    } else {
        for (const auto& pObj : m_objects) {
            auto a = dynamic_cast<CDrawAdd*>(pObj);
            if (a) pow += mod_count(a);
        }
    }

    return nearbyintf(100 * pow) / 100;
}

void CDrawDoc::OnFileInfo()
{
    if (!theApp.isRDBMS || m_mak_xx < 0) {
        UINT ogl, red, rez, wol;
        ModCount(&ogl, &red, &rez, &wol);
        float oglp, redp, rezp, wolp;
        oglp = (float)ogl / pmodcnt;
        redp = (float)red / pmodcnt;
        rezp = (float)rez / pmodcnt;
        wolp = (float)wol / pmodcnt;
        ::StringCchPrintf(theApp.bigBuf, n_size, _T("Wszystkich modu³ów\n\tog³oszeniowych %u\n\tredakcyjnych %u\n\tzarezerwowanych %u\n\twolnych %u\nStron\n\tog³oszeniowych %.3f\n\tredakcyjnych %.3f\n\tzarezerwowanych %.3f\n\twolnych %.3f\n\nPowierzchnia og³oszeñ: %.2f mod."), ogl, red, rez, wol, oglp, redp, rezp, wolp, PowAdd2Mod(FALSE));
        MessageBox(nullptr, theApp.bigBuf, _T("Statystyka"), MB_OK);
        return;
    }

    CInfoDlg dlg;
    CString deadline, data_z, data_s, data_w;
    CManODPNETParm idmPar { CManDbType::DbTypeInt32, &m_mak_xx };
    if (isLIB) {
        CInfoDlgLib dlg2;
        dlg2.m_tytmut = gazeta;
        dlg2.m_wersja = data;

        dlg2.m_papier = dlg2.m_opis = CString(' ', 200);
        CManODPNETParms orapar {
            { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &dlg2.m_szycie },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &dlg2.m_papier },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &dlg2.m_opis },
            idmPar
        };
        orapar.outParamsCount = 3;

        if (!theManODPNET.EI("begin select szycie,papier,opis into :1,:2,:3 from maklibinfo where xx=:4; end;", orapar)) return;

        if (dlg2.DoModal() != IDOK) return;

        CManODPNETParms orapar2 {
            { CManDbType::DbTypeInt32, &dlg2.m_szycie },
            { CManDbType::DbTypeVarchar2, &dlg2.m_opis },
            idmPar
        };
        theManODPNET.EI("update makieta_lib set szycie=decode(:1,0,null,1),opis=:2 where xx=:3", orapar2);
    } else {
        int d, m, r, g, min;

        dlg.isRO = isRO;
        dlg.m_data = data;
        dlg.m_gazeta = gazeta;
        dlg.m_prowadz1 = prowadzacy1;
        dlg.m_prowadz2 = prowadzacy2;
        dlg.m_sekretarz = sekretarz;
        dlg.m_objetosc = (int)m_pages.size();
        dlg.m_grzbiet = dlg.m_cena = dlg.m_cena2 = dlg.m_wydawcared = dlg.m_wydawca = CString(' ', 8);
        dlg.m_sign_text = deadline = data_z = data_s = data_w = CString(' ', 16);
        dlg.m_typ_dodatku = dlg.m_kto_makietuje = dlg.m_opis_papieru = CString(' ', 128);
        dlg.m_wydaw_str = dlg.m_uwagi = CString(' ', 256);

        CManODPNETParms orapar {
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &deadline },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &data_z },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &data_s },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &data_w },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &dlg.m_kto_makietuje },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &dlg.m_wydawca },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &dlg.m_wydawcared },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &dlg.m_uwagi },
            { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &dlg.m_naklad },
            { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &dlg.m_numer },
            { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &dlg.m_numerrok },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &dlg.m_cena },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &dlg.m_cena2 },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &dlg.m_sign_text },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &dlg.m_wydaw_str },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &dlg.m_typ_dodatku },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &dlg.m_grzbiet },
            { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &dlg.m_szyj },
            { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &dlg.m_drukarnie },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &dlg.m_opis_papieru },
            idmPar
        };
        orapar.outParamsCount = 20;

        auto sql = reinterpret_cast<char*>(theApp.bigBuf);
        ::StringCchPrintfA(sql, bigSize, "begin select deadline,data_z,data_s,data_w,p2,wydawca,zsy_red,grzbietowanie,naklad,numer,numerrok,to_char(cena),to_char(cena2),podpis_red,wydaw_str,typ_dodatku,grzbiet,szycie,drukarnie,opis_papieru into :1,:2,:3,:4,:5,:6,:7,:8,:9,:10,:11,:12,:13,:14,:l5,:16,:17,:18,:19,:20 from %sinfo where rownum=1 and xx=:xx; end;", isGRB ? "grb" : "mak");
        if (!theManODPNET.EI(sql, orapar)) return;

        if (_stscanf_s(deadline, c_formatCzasu, &d, &m, &r, &g, &min) == 5)
            dlg.m_godz_deadline = dlg.m_data_deadline = CTime(r, m, d, g, min, 0);
        if (_stscanf_s(data_z, c_formatCzasu, &d, &m, &r, &g, &min) == 5)
            dlg.m_godz_zamkniecia = dlg.m_data_zamkniecia = CTime(r, m, d, g, min, 0);
        if (_stscanf_s(data_s, c_formatCzasu, &d, &m, &r, &g, &min) == 5)
            dlg.m_godz_studio = dlg.m_data_studio = CTime(r, m, d, g, min, 0);
        if (_stscanf_s(data_w, c_formatCzasu, &d, &m, &r, &g, &min) == 5)
            dlg.m_godz_wykupu = dlg.m_data_wykupu = CTime(r, m, d, g, min, 0);

        CString oldMutGrb = dlg.m_grzbiet;
        ModCount(&dlg.m_modogl, &dlg.m_modred, &dlg.m_modrez, &dlg.m_modwol);
        dlg.m_modoglp = (float)dlg.m_modogl / pmodcnt;
        dlg.m_modredp = (float)dlg.m_modred / pmodcnt;
        dlg.m_modrezp = (float)dlg.m_modrez / pmodcnt;
        dlg.m_modwolp = (float)dlg.m_modwol / pmodcnt;
        dlg.m_modcnt = PowAdd2Mod(FALSE);
        dlg.m_quecnt = PowAdd2Mod(TRUE);
        if (dlg.DoModal() != IDOK) return;
        // zmiana grzbietu
        if (iDocType == DocType::grzbiet_drukowany) {
            BOOL isOK = TRUE;
            d = _ttoi(dlg.m_sign_text);
            deadline.Format(c_formatCzasu, dlg.m_data_deadline.GetDay(), dlg.m_data_deadline.GetMonth(), dlg.m_data_deadline.GetYear(), dlg.m_godz_deadline.GetHour(), dlg.m_godz_deadline.GetMinute());
            if (oldMutGrb != dlg.m_grzbiet) {
                CManODPNETParms orapar2 {
                    idmPar,
                    { CManDbType::DbTypeVarchar2, &dlg.m_grzbiet }
                };
                isOK = theManODPNET.EI("begin grb.set_mutgrb_explicit(:grb_xx,:mutgrb); end;", orapar2);
            }

            CManODPNETParms orapar3 {
                idmPar,
                { CManDbType::DbTypeInt32, &dlg.m_szyj },
                { CManDbType::DbTypeInt32, &dlg.m_drukarnie },
                { CManDbType::DbTypeVarchar2, &deadline },
                { CManDbType::DbTypeInt32, &dlg.m_naklad },
                { CManDbType::DbTypeInt32, &d }
            };
            if (isOK) isOK = theManODPNET.EI("begin grb.update_info(:grb_xx,:przefalc,:drukarnie,:deadline,:naklad,:gramatura); end;", orapar3);

            for (const auto& p : m_pages)
                p->m_drukarnie = dlg.m_drukarnie;
            if (!isOK || oldMutGrb == dlg.m_grzbiet) return;

            CString newGazeta = gazeta.Left(4) + dlg.m_grzbiet;
            CString oldTitle = GetTitle();
            const int pos = oldTitle.Find(gazeta);
            SetTitle(oldTitle.Left(pos) + newGazeta + oldTitle.Mid(pos + gazeta.GetLength()));
            gazeta = newGazeta;
            theApp.FileRefresh(this);
            return;
        }

        data_w.Format(c_formatCzasu, dlg.m_data_wykupu.GetDay(), dlg.m_data_wykupu.GetMonth(), dlg.m_data_wykupu.GetYear(), dlg.m_godz_wykupu.GetHour(), dlg.m_godz_wykupu.GetMinute());
        dlg.m_data_studio.GetYear() < 2000 ? data_s.Empty() : data_s.Format(c_formatCzasu, dlg.m_data_studio.GetDay(), dlg.m_data_studio.GetMonth(), dlg.m_data_studio.GetYear(), dlg.m_godz_studio.GetHour(), dlg.m_godz_studio.GetMinute());
        const auto dataZamknieciaOld = dataZamkniecia;
        dataZamkniecia = CTime(dlg.m_data_zamkniecia.GetYear(), dlg.m_data_zamkniecia.GetMonth(), dlg.m_data_zamkniecia.GetDay(), dlg.m_godz_zamkniecia.GetHour(), dlg.m_godz_zamkniecia.GetMinute(), 0);
        //zmiana daty wykupu rezerwacji - czyli zamkniêcia makiety
        //dla DGW obowi¹zuje czas na podstawie daty wykupu
        if (dataZamkniecia != dataZamknieciaOld && dlg.m_data_zamkniecia > CTime::GetCurrentTime()) {
            if (gazeta.Left(3) != _T("DGW")) {
                data_z = dataZamkniecia.Format(c_ctimeCzas);
                for (const auto& pObj : m_objects) {
                    auto pMayBeAdd = dynamic_cast<CDrawAdd*>(pObj);
                    if (pMayBeAdd && pMayBeAdd->m_add_xx > 0 && pMayBeAdd->czaskto.Find(_T('#')) == -1) {
                        pMayBeAdd->czaskto = data_z + pMayBeAdd->czaskto.Mid(16);
                        pMayBeAdd->SetDirty();
                    }
                }
            }
        }

        // propagacja deadline'u
        const auto pc = m_pages.size();
        if (pc > 1 && (dlg.m_set_deadline || dlg.m_set_kraty || dlg.m_set_papier)) {
            CDrawPage* pPage = m_pages[1];
            const auto& dt = pPage->m_deadline;
            const auto druk = pPage->m_drukarnie;
            const auto s_x1 = pPage->szpalt_x;
            const auto s_y1 = pPage->szpalt_y;

            for (const auto& p : m_pages) {
                if (dlg.m_set_deadline && p->m_dervlvl != DervType::fixd) {
                    p->m_deadline = dt;
                    p->m_drukarnie = druk;
                }
                if (dlg.m_set_kraty && p->m_adds.empty())
                    p->SetBaseKrata(s_x1, s_y1);
                p->SetDirty();
            }
        }
    }

    if (!isLIB) {
        prowadzacy1 = dlg.m_prowadz1;
        prowadzacy2 = dlg.m_prowadz2;
        sekretarz = dlg.m_sekretarz;
        isSIG = !dlg.m_sign_text.IsEmpty();
        data_z = dataZamkniecia.Format(c_ctimeCzas);

        CManODPNETParms orapar4 {
            idmPar,
            { CManDbType::DbTypeVarchar2, &dlg.m_wydawca },
            { CManDbType::DbTypeVarchar2, &dlg.m_cena },
            { CManDbType::DbTypeInt32, &dlg.m_naklad },
            { CManDbType::DbTypeInt32, &dlg.m_numerrok },
            { CManDbType::DbTypeInt32, &dlg.m_numer },
            { CManDbType::DbTypeVarchar2, &sekretarz },
            { CManDbType::DbTypeVarchar2, &prowadzacy1 },
            { CManDbType::DbTypeVarchar2, &dlg.m_kto_makietuje },
            { CManDbType::DbTypeVarchar2, &data_w },
            { CManDbType::DbTypeVarchar2, &data_z },
            { CManDbType::DbTypeVarchar2, &dlg.m_sign_text },
            { CManDbType::DbTypeInt32, &dlg.m_szyj },
            { CManDbType::DbTypeVarchar2, &dlg.m_uwagi },
            { CManDbType::DbTypeVarchar2, &prowadzacy2 },
            { CManDbType::DbTypeVarchar2, &dlg.m_cena2 },
            { CManDbType::DbTypeVarchar2, &dlg.m_wydawcared },
            { CManDbType::DbTypeVarchar2, &data_s }
        };

        theManODPNET.EI("begin update_makinfo(:mak_xx,:mutwyd,:cena,:naklad,:numerrok,:numer,:sekr,:prow1,:prow2,:wykup,:zamkniecie,:podpis,:szycie,:uwagi,:kto_prowadzi,:cena2,:zsy_red,:do_studia); end;", orapar4);
    }
}

void CDrawDoc::PrintInfo(CDC* pDC, int max_n, int wspol_na_str)  // wspol_na_str =1 lub 2- 50  lub 100 str na wydruku
{
    ASSERT_VALID(this);
    CFont m_font, m_vertfont;
    CFont *pOldFont;
    LOGFONT lf;
    m_pagefont.GetLogFont(&lf);
    lf.lfHeight = -11;
    lf.lfWidth = 0;
    lf.lfWeight = FW_BOLD;

    if (m_font.CreateFontIndirect(&lf))
        pOldFont = reinterpret_cast<CFont*>(pDC->SelectObject(&m_font));
    else
        pOldFont = reinterpret_cast<CFont*>(pDC->SelectStockObject(DEVICE_DEFAULT_FONT));

    m_addfont.GetLogFont(&lf);
    lf.lfHeight = -8;
    lf.lfOrientation = lf.lfEscapement = 900;
    const BOOL vertOK = m_vertfont.CreateFontIndirect(&lf);

    CRect rect;
    const int iHorizScale = iPagesInRow * pmodulx;
    for (int i = 1; i <= max_n; ++i) {
        rect.SetRect(pmodulx, (i - 1)*(wspol_na_str * 40 + 2)*(-pmoduly), (int)(5.5*iHorizScale), ((i - 1)*(wspol_na_str * 40 + 2) + 1)*(-pmoduly));
        pDC->FillRect(rect, (CBrush*)pDC->SelectStockObject(WHITE_BRUSH));
        CString info = (gazeta.IsEmpty() && data.IsEmpty() ? _T("") : _T("EDYCJA: ") + gazeta + _T(" ") + data);
        info += ((prowadzacy1.IsEmpty() && prowadzacy2.IsEmpty()) ? _T("") : _T("         PROWADZ¥CY: ") + (prowadzacy1.IsEmpty() ? prowadzacy2 : prowadzacy1) + ((!prowadzacy1.IsEmpty() && !prowadzacy2.IsEmpty()) ? _T("  DWÓJKA: ") + prowadzacy2 : _T("")));
        info += (sekretarz.IsEmpty() ? _T("") : _T("    ODPOWIEDZIALNY: ") + sekretarz);
        pDC->DrawText(info, info.GetLength(), rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
        info.Format(_T("str: %i/%i"), i, max_n);
        pDC->DrawText(info, info.GetLength(), rect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

        rect.SetRect((int)(4.7*iHorizScale), (i - 1)*(wspol_na_str * 40 + 2)*(-pmoduly), (int)(4.8*iHorizScale), ((i - 1)*(wspol_na_str * 40 + 2) + 1)*(-pmoduly));
        pDC->SetBkMode(TRANSPARENT);
        pDC->FillRect(rect, &(((CMainFrame*)AfxGetMainWnd())->cyjan));
        pDC->DrawText(_T("C"), 1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
        rect.SetRect((int)(4.8*iHorizScale), (i - 1)*(wspol_na_str * 40 + 2)*(-pmoduly), (int)(4.9*iHorizScale), ((i - 1)*(wspol_na_str * 40 + 2) + 1)*(-pmoduly));
        pDC->FillRect(rect, &(((CMainFrame*)AfxGetMainWnd())->magenta));
        pDC->DrawText(_T("M"), 1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
        rect.SetRect((int)(4.9*iHorizScale), (i - 1)*(wspol_na_str * 40 + 2)*(-pmoduly), (int)(5 * iHorizScale), ((i - 1)*(wspol_na_str * 40 + 2) + 1)*(-pmoduly));
        pDC->FillRect(rect, &(((CMainFrame*)AfxGetMainWnd())->yellow));
        pDC->DrawText(_T("Y"), 1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
        rect.SetRect((int)(5 * iHorizScale), (i - 1)*(wspol_na_str * 40 + 2)*(-pmoduly), (int)(5.1*iHorizScale), ((i - 1)*(wspol_na_str * 40 + 2) + 1)*(-pmoduly));
        pDC->Rectangle(rect);
        pDC->DrawText(_T("K"), 1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
        // czas wydruku
        if (vertOK) {
            auto pTempFont = reinterpret_cast<CFont*>(pDC->SelectObject(&m_vertfont));
            rect.SetRect((int)(5.8*iHorizScale), 3 * (-pmoduly), (int)(5.9*iHorizScale), 10 * (-pmoduly));
            pDC->DrawText(CTime::GetCurrentTime().Format(_T("%d/%m/%Y godz. %H:%M")), 22, rect, DT_SINGLELINE | DT_NOCLIP);
            pDC->SelectObject(pTempFont);
        }
    }

    pDC->SelectObject(pOldFont);
    if (&m_font != nullptr) m_font.DeleteObject();
    if (vertOK) m_vertfont.DeleteObject();
}

void CDrawDoc::OnFileDrzewo() // zmiana drzewa 
{
    CDrz1Dlg dlg;
    dlg.m_id_drw = id_drw;
    if (dlg.DoModal() != IDOK) return;
    BeginWaitCursor();
    gazeta = dlg.m_gazeta;
    if (id_drw != dlg.m_id_drw && AfxMessageBox(_T("Czy chcesz zmieniæ kod i nazwê produktu dla tej makiety"), MB_ICONQUESTION | MB_YESNO) == IDYES) {
        ZmianaCaptions(id_drw, dlg.m_id_drw);
        id_drw = dlg.m_id_drw;
        ZmianaSpotow((int)m_pages.size() & -4);
    }
    SetTitleAndMru(FALSE);
    EndWaitCursor();
}

CString CDrawDoc::XmlReadText(IXmlReader *reader)
{
    XmlNodeType nodeType;
    const wchar_t *tmp;
    reader->Read(&nodeType);
    reader->Read(&nodeType);
    reader->GetValue(&tmp, nullptr);
    CString ret { tmp };
    if (nodeType == XmlNodeType_Text) reader->Read(&nodeType);
    return ret;
}

////////////////// DB ///////////////////////////////////////////////
bool CDrawDoc::DBImport(bool synchronize)
{
    CDbDlg dlg;
    dlg.m_dtime = CDrawApp::ShortDateToCTime(data);
    if (dlg.DoModal() != IDOK) return false;
    BeginWaitCursor();

    long adno;
    CDrawAdd* vAdd, *vAdd2;
    bool empSet{true}, bBank{false};
    CString xRoz, xNaz, xKol, xStr, xUwa, xKrt;
    std::vector<int> syncATEX, dirtyATEX, zaporaATEX;
    std::vector<CDrawAdd*> aNewAdds, aOldAdds, aModifAdds;
    const bool bOstateczna = dataZamkniecia < CTime::GetCurrentTime();

    if (bOstateczna) synchronize = true;

second_paper:
    auto buf = reinterpret_cast<char*>(theApp.bigBuf);
    xStr.Format(_T("tyt=%s&mut=%s&kiedy=%s&str=%s&cli=%i&zone=%s&lastone=%i"), (LPCTSTR)dlg.m_pap, (LPCTSTR)dlg.m_mut, (LPCTSTR)dlg.m_dt, (LPCTSTR)dlg.m_strlog, (int)dlg.m_client, (LPCTSTR)dlg.m_zone, bOstateczna ? 1 : 0);
    auto pFile = theApp.OpenURL(0, xStr);
    if (pFile) {
        const UINT bytesRead = pFile->Read(buf, n_size);
        pFile->Close();

        auto reader = CComPtr<IXmlReader> {};
        CreateXmlReader(__uuidof(reader), reinterpret_cast<void**>(&reader), nullptr);
        auto stream = CComPtr<IStream>(SHCreateMemStream((const BYTE*)buf, bytesRead));
        reader->SetInput(stream);

        XmlNodeType nodeType;
        if (S_OK != reader->Read(&nodeType)) {
            AfxMessageBox(CString(_T("ATEX nie odpowiada\n\n")) + buf);
            return false;
        }

        CPoint pos{GetAsideAddPos(false)}, pos2;
        while (S_OK == reader->Read(&nodeType) && nodeType != XmlNodeType_EndElement) // read next ad
        {
            empSet = false;
            pos2 = pos;

            adno = _wtoi(XmlReadText(reader));
            xRoz = XmlReadText(reader);
            xNaz = XmlReadText(reader);
            xKol = XmlReadText(reader);
            xStr = XmlReadText(reader);
            xUwa = XmlReadText(reader);
            xKrt = XmlReadText(reader);

            vAdd = AddExists(adno);
            vAdd2 = DBCreateAdd(xRoz, xNaz, adno, ValidKolor(xKol), xStr, xUwa, xKrt, pos);
            if (vAdd2 == nullptr)
                continue;

            reader->Read(&nodeType);
            if (!reader->IsEmptyElement()) {
                bBank = TRUE;
                vAdd2->bank.insid = (WORD)_wtoi(XmlReadText(reader));
                vAdd2->bank.n = (WORD)_wtoi(XmlReadText(reader));
                vAdd2->bank.k = (WORD)_wtoi(XmlReadText(reader));
                reader->Read(&nodeType);
            }
            reader->Read(&nodeType); // </ad>


            vAdd2->flags.digital = vAdd2->bank.insid < 0;

            if ((bOstateczna || bBank) && vAdd == nullptr)
                aNewAdds.push_back(vAdd2);
            if (vAdd) {
                if (vAdd->logpage.IsEmpty() && vAdd->m_add_xx > 0) { // sprzedane spacerem
                    vAdd->logpage = xStr;
                    vAdd->remarks_atex = xUwa;
                    vAdd->SetDirty();
                }
                if (vAdd->remarks_atex != xUwa) {
                    vAdd->remarks_atex = xUwa;
                    vAdd->SetDirty();
                }
                if (vAdd->flags.digital != vAdd2->flags.digital) {
                    vAdd->flags.digital = vAdd2->flags.digital;
                    vAdd->SetDirty();
                }
                if ((vAdd->typ_xx == vAdd2->typ_xx || vAdd->typ_xx == 0) && vAdd->sizex == vAdd2->sizex && vAdd->sizey == vAdd2->sizey && vAdd->kolor == vAdd2->kolor && vAdd->logpage.SpanExcluding(_T("., ")) == vAdd2->logpage.SpanExcluding(_T("., "))) {
                    vAdd = vAdd2;
                    pos = pos2;
                    if (bOstateczna) {
                        Remove(vAdd);
                        delete vAdd;
                    }
                } else {
                    if (bOstateczna) {
                        aOldAdds.push_back(vAdd);
                        aModifAdds.push_back(vAdd2);
                    }
                    vAdd2->powtorka = vAdd->powtorka;
                    vAdd2->oldAdno = vAdd->oldAdno;
                    dirtyATEX.push_back(adno);
                }
                if (!bOstateczna) {
                    Remove(vAdd);
                    delete vAdd;
                }
            }
            if (synchronize) syncATEX.push_back(adno);
        }
    }

    if (!dlg.m_alttyt.IsEmpty()) {
        dlg.m_pap = dlg.m_alttyt;
        dlg.m_mut = dlg.m_altmut;
        dlg.m_alttyt.Empty();
        goto second_paper;
    }

    if (empSet && !bOstateczna)
        AfxMessageBox(_T("nie odnaleziono og³oszeñ"));

    if (synchronize) { // usuñ z makiety te og³oszenia, które maj¹ nr>0 i nie ma ich w ATEX'ie
        UpdateAllViews(nullptr, HINT_SAVEAS_DELETE_SELECTION, nullptr);
        bool spfluos{false}, rejestracjaSpfluos{false};
        int i, j = 1, ac = (int)syncATEX.size();
        std::vector<CDrawAdd*> aDelAdds;
        CString sQueMissed;
        CString adnos = _T("Stwierdzono brak nastêpuj¹cych og³oszeñ w bazie ATEX:\n\n");
        CString doRejetracjiAdnos = _T("Trzeba zarejestrowaæ lub usun¹æ z makiety, jeœli zosta³y wycmentarzowane, nastêpuj¹ce og³oszenia:\n\n");
        for (const auto& pObj : m_objects) {
            vAdd = dynamic_cast<CDrawAdd*>(pObj);
            if (vAdd && vAdd->nreps > 0) {
                adno = vAdd->nreps;
                for (i = 0; i < ac; ++i)
                    if (syncATEX[i] == adno) break;
                if (i == ac && !vAdd->flags.derived && vAdd->wersja.Find('z') == -1 && (vAdd->flags.isok == 3 || (bOstateczna && vAdd->flags.isok < 3))) { // nie ma w ATEX'ie
                    SelectAdd(vAdd, true);
                    adnos.AppendFormat(_T("%li, "), adno);
                    aDelAdds.push_back(vAdd);
                    spfluos = true;
                }
                if (i == ac && !vAdd->flags.derived && abs(vAdd->flags.isok) < 3) { // nie zarejestrowane
                    doRejetracjiAdnos.AppendFormat(_T("%li, "), adno);
                    rejestracjaSpfluos = true;
                }
                if (i < ac && abs(vAdd->flags.isok) < 3) { // ustaw zapore
                    zaporaATEX.push_back((int)vAdd->m_pub_xx);
                    vAdd->flags.isok = 3;
                    const int p = vAdd->czaskto.Find(_T("["));
                    if (p >= 0) vAdd->czaskto = _T("# ") + vAdd->czaskto.Mid(p);
                }
            }
        }

        // sprawdz powi¹zane og³oszenia z kolejki
        for (const auto& a : m_addsque)
            if (a->nreps > 0) {
                adno = a->nreps;
                for (i = 0; i < ac; ++i)
                    if (syncATEX[i] == adno) break;
                if (i == ac)
                    sQueMissed.AppendFormat(_T(", %ld"), adno);
            }

        // ostateczna synchronizacja
        if (bOstateczna) {
            if (aNewAdds.empty() && aModifAdds.empty() && aDelAdds.empty())
                MessageBox(nullptr, _T("Nie stwierdzono zmian"), _T("Ostateczna Weryfikacja z ATEX'em"), MB_OK);
            else {
                COstWer dlg2(&aNewAdds, &aOldAdds, &aModifAdds, &aDelAdds, FALSE);
                dlg2.DoModal();
            }
        } else {
            if (bBank) {
                COstWer dlg2(&aNewAdds, &aOldAdds, &aModifAdds, &aDelAdds, TRUE);
                dlg2.DoModal();
            }
            if (!spfluos && !rejestracjaSpfluos)
                adnos.Empty();
            else {
                if (spfluos) adnos += _T("\n\n");
                else adnos.Empty();
                if (rejestracjaSpfluos) adnos += doRejetracjiAdnos + _T("\n\n");
            }
            if (ac = (int)dirtyATEX.size()) {
                j = 1;
                adnos += _T("Zmieniono w ATEXie nastêpuj¹ce og³oszenia:\n\n");
                for (i = 0; i < ac; ++i)
                    adnos.AppendFormat((j++ & 3) ? _T("%li,\t") : _T("%li\n\t"), (long)dirtyATEX[i]);
                adnos += _T("\n\n");
            }
            if (!sQueMissed.IsEmpty())
                adnos += _T("W kolejce stoj¹ powi¹zane og³oszenia, ktorych brak w ATEXie:\n\n") + sQueMissed.Mid(2) + _T("\n\n");
            if (spfluos) {
                UpdateAllViews(nullptr);
                if (AfxMessageBox(adnos + _T("Czy usun¹æ nadmiarowe og³oszenia z makiety?"), MB_YESNO, AFX_IDP_ASK_TO_SAVE) == IDYES)
                    for (const auto& a : aDelAdds) {
                        Remove(a);
                        delete a;
                    }
            } else if (!adnos.IsEmpty())
                AfxMessageBox(adnos, MB_OK);

            if (!zaporaATEX.empty())
                theManODPNET.Zapora(zaporaATEX);
        }
    }

    EndWaitCursor();
    return true;
}

CDrawAdd* CDrawDoc::DBCreateAdd(const CString& roz, const CString&  nazwa, long nr, UINT kolor, CString& warunki, const CString& uwagi, const CString& krt, CPoint& pos)
{
    CString sPrecelFlag;
    int sx, sy, atex_krat, szpalt_x = 0, szpalt_y = 0, fizp = 0, posx = 0, posy = 0, typ_xx = 0;

    if (_stscanf_s(krt, _T("%iX%i"), &szpalt_x, &szpalt_y) != 2) {  // og³oszenia z PROFIT'a maj¹ tu informacjê o kracie
        szpalt_x = pszpalt_x;
        szpalt_y = pszpalt_y;
    }

    if (_stscanf_s(roz, _T("%iX%i"), &sx, &sy) != 2) {  // typ nazwany w ATEX'ie
        sPrecelFlag = CString(' ', 64);
        CManODPNETParms orapar {
            { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &szpalt_x },
            { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &szpalt_y },
            { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &sx },
            { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &sy },
            { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &typ_xx },
            { CManDbType::DbTypeInt32, CManDbDir::ParameterOut, &atex_krat },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterOut, &sPrecelFlag },
            { CManDbType::DbTypeVarchar2, const_cast<CString*>(&roz) }
        };
        orapar.outParamsCount = 7;

        theManODPNET.EI("begin select min(szpalt_x),min(szpalt_y),min(sizex),min(sizey),min(typ_xx),min(uzyj_atexkrat),min(precel) into :sx,:sy,:x,:y,:atex,:typ,:prec from typoglrozm where modelid=:sym; end;", orapar);

        if (!szpalt_x) {
            if (krt[0]) { // informacja o kracie z ATEXa
                if (m_Rozm.empty()) IniRozm();
                CAtexKrat krat(krt, this);
                if (krat.isValid && !krat.Compute(&sx, &sy, &szpalt_x, &szpalt_y)) {
                    AfxMessageBox(krat.isToSmall ? _T("Og³oszenie ") + nazwa + _T(": ") + roz + _T(" jest za ma³e do automatycznego dopasowania kraty") : _T("Nie uda³o siê dopasowaæ kraty og³oszenia: ") + roz, MB_OK);
                    return nullptr;
                }
            } else {
                AfxMessageBox(_T("Nieznany rozmiar og³oszenia ") + nazwa + _T(": ") + roz, MB_OK);
                return nullptr;
            }
        } else { // modelid zdefiniowany w s³owniku TYP_OGLOSZENIA
            if (atex_krat == 1 && krt[0]) {
                if (m_Rozm.empty()) IniRozm();
                CAtexKrat krat(krt, this);
                if (krat.isValid) {
                    int _sx, _sy, _szpalt_x, _szpalt_y;
                    if (krat.Compute(&_sx, &_sy, &_szpalt_x, &_szpalt_y)) {
                        sx = _sx; sy = _sy; szpalt_x = _szpalt_x; szpalt_y = _szpalt_y;
                    }
                }
            }
        }

        if (m_pages.size() > 1 && roz == _T("LOG")) {
            fizp = (m_pages[1])->nr; posx = 5; posy = 1; warunki += _T(" # GP");
        }
    }

    if (sx < 1 || sx > szpalt_x || sy < 1 || sy > szpalt_y) return FALSE;

    const CRect r(pos, CSize((int)(sx * modulx), (int)(sy * (-1) * moduly)));
    auto pObj = new CDrawAdd(r);
    Add(pObj);
    pObj->m_pDocument = this;
    pObj->szpalt_x = szpalt_x;
    pObj->szpalt_y = szpalt_y;
    pObj->typ_xx = typ_xx;
    pObj->nazwa = nazwa;
    pObj->nreps = nr;
    pObj->kolor = kolor;
    pObj->logpage = warunki;
    pObj->remarks_atex = uwagi;
    pObj->flags.isok = 3; // WER
    pObj->SetSpaceSize(sx, sy);
    if (fizp != 0) {  // logo albo pasek
        auto pPage = GetPage(fizp);
        if (pPage->FindSpace(pObj, &posx, &posy, sx, sy)) {
            pObj->posx = posx; pObj->posy = posy;
            pPage->AddAdd(pObj);
            pObj->m_position.SetRect(pPage->m_position.left + (posx - 1)*pmodulx, pPage->m_position.top - posy*pmoduly, pPage->m_position.left + (posx + sx - 1)*pmodulx, pPage->m_position.top - (posy + sy)*pmoduly);
        }
    } else
        AdvanceAsidePos(pos);

    if (!sPrecelFlag.IsEmpty())
        pObj->InitPrecel(sPrecelFlag);
    pObj->UpdateInfo();
    return pObj;
}

CDrawAdd* CDrawDoc::AddExists(long nr) const
{
    if (nr != -1 && nr != 0)
        for (const auto& pObj : m_objects) {
            auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
            if (pAdd && pAdd->nreps == nr)
                return pAdd;
        }

    return nullptr;
}

CDrawAdd* CDrawDoc::PubXXExists(int pub_xx) const
{
    for (const auto& pObj : m_objects) {
        auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
        if (pAdd && pAdd->m_pub_xx == pub_xx)
            return pAdd;
    }
    return nullptr;
}

void CDrawDoc::OnSyncpow()
{
    if (SaveModified()) {
        CManODPNETParms orapar { CManDbType::DbTypeInt32, &m_mak_xx };
        theManODPNET.EI("begin derv.synchronize_kraj(:mak_xx); end;", orapar);
        SetModifiedFlag(FALSE);
        theApp.FileRefresh(this);
    }
}

void CDrawDoc::OnUpdateSyncpow(CCmdUI* pCmdUI)
{
    if (gazeta.GetLength() > 4) OnDisableMenuRO(pCmdUI); else pCmdUI->Enable(FALSE);
}

void CDrawDoc::OnDelremarks()
{
    if (IDYES == AfxMessageBox(_T("Czy usun¹æ uwagi ze wszystkich og³oszeñ"), MB_ICONQUESTION | MB_YESNO)) {
        for (const auto& pObj : m_objects) {
            auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
            if (pAdd) pAdd->remarks.Empty();
        }
        UpdateAllViews(nullptr);
    }
}

void CDrawDoc::OnCheckrep()
{
    if (!SaveModified()) return;

    TCHAR* buf = theApp.bigBuf;
    std::vector<CString> badpowt;
    CManODPNETParms orapar {
        { CManDbType::DbTypeInt32, &m_mak_xx },
        { CManDbType::DbTypeRefCursor, CManDbDir::ParameterOut, nullptr }
    };

    if (!theManODPNET.FillArr(&badpowt, "begin check_repeats(:mak_xx,:retCur); end;", orapar)) return;

    CString head(badpowt.empty() ? "Nie stwierdzono b³êdnych powtórek" : "Nie ma wczeœniejszych emisji dla:\n\n");
    ::StringCchCopy(buf, n_size, head);
    buf += head.GetLength();
    for (const auto& err : badpowt) {
        ::StringCchCopy(buf, n_size, err);
        buf += err.GetLength();
    }

    AfxMessageBox(theApp.bigBuf);
}

void CDrawDoc::OnEpsdate()
{
    CEPSDateDlg dlg;
    dlg.DoModal();
}

void CDrawDoc::OnPagederv()
{
    DerivePages(nullptr);
}

HMENU CDrawDoc::GetDefaultMenu()
{
    return isGRB ? m_grzbietMenu.GetSafeHmenu() : nullptr;
}

void CDrawDoc::OnInsertGrzbiet()
{
    CGrzbDlg dlg;
    if (dlg.DoModal() == IDOK) {
        CString mutacja = _T("  ");
        int cnt = (dlg.m_incordec ? dlg.m_split_cnt : dlg.m_insert_cnt) >> 2;

        CManODPNETParms orapar {
            { CManDbType::DbTypeInt32, CManDbDir::ParameterInOut, &m_mak_xx },
            { CManDbType::DbTypeInt32, &dlg.m_drw_xx },
            { CManDbType::DbTypeInt32, &cnt },
            { CManDbType::DbTypeVarchar2, CManDbDir::ParameterInOut, &mutacja }
        };
        orapar.outParamsCount = 2;

        auto sql = reinterpret_cast<char*>(theApp.bigBuf);
        ::StringCchPrintfA(sql, bigSize, "begin grb.%s_capacity(:grb_xx,:drw_xx,:count,:mutgrb); end;", dlg.m_incordec ? (dlg.m_delete == 2 ? "split" : "shrink") : "expand");
        theApp.BeginWaitCursor();
        if (theManODPNET.EI(sql, orapar)) {
            if (mutacja.IsEmpty()) {
                AfxMessageBox(_T("W wyniku tej operacji grzbiet przesta³ istnieæ"));
                CWnd* pWnd = ((CMDIFrameWnd*)AfxGetMainWnd())->MDIGetActive();
                pWnd->PostMessage(WM_CLOSE);
            } else {
                if (gazeta.Mid(4).Compare(mutacja)) {
                    CString newGazeta = gazeta.Left(4) + mutacja;
                    CString oldTitle = GetTitle();
                    int i = oldTitle.Find(gazeta);
                    SetTitle(oldTitle.Left(i) + newGazeta + oldTitle.Mid(i + gazeta.GetLength()));
                    gazeta = newGazeta;
                    AfxMessageBox(CString("Zmiana mutacji terytorialnej.\nTen grzbiet bêdzie siê teraz nazywa³ ") + gazeta);
                }
                theApp.FileRefresh();
            }
        }
        theApp.EndWaitCursor();
    } else if (dlg.bRefreshOnClose) { // 4x4
        theApp.FileRefresh(this);
        return;
    }
}

void CDrawDoc::OnSyncDrv()
{
    if (SaveModified()) {
        CManODPNETParms orapar { CManDbType::DbTypeInt32, &m_mak_xx };
        theManODPNET.EI("begin derv.synchronize_derv(:mak_xx); end;", orapar);
    }
}

void CDrawDoc::OnSetPagina()
{
    const auto mask = PaginaType::roman | PaginaType::arabic;
    for (const auto& pPage : m_pages) {
        pPage->SetDirty();
        const auto newnum = pPage->nr ^= mask;
        for (const auto& pAdd : pPage->m_adds) {
            pAdd->fizpage = newnum;
            pAdd->SetDirty();
        }
    }
    UpdateAllViews(nullptr);
}

void CDrawDoc::OnSetDea()
{
    if (theApp.grupa & (UserRole::kie | UserRole::mas)) {
        if (!(theApp.grupa & UserRole::dea)) {
            SaveModified();
            theApp.grupa |= UserRole::dea;
        } else
            theApp.grupa ^= UserRole::dea;
        ((CMainFrame*)AfxGetMainWnd())->SetRoleStatus();
    }
}

void CDrawDoc::OnUpdateSetDea(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(theApp.isRDBMS && theApp.grupa&(UserRole::kie | UserRole::mas));
    pCmdUI->SetCheck(theApp.grupa&UserRole::dea);
}

void CDrawDoc::OnShowTime()
{
    theApp.showDeadline = !theApp.showDeadline; UpdateAllViews(nullptr);
}

void CDrawDoc::OnShowAcDeadline()
{
    theApp.showAcDeadline = !theApp.showAcDeadline;
    if (theApp.showAcDeadline > 0 && isACD == 0 && !m_pages.empty())
        if (theManODPNET.ReadAcDeadlines(this))
            isACD = 1;

    UpdateAllViews(nullptr);
}

void CDrawDoc::OnUpdateShowTime(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(theApp.showDeadline);
}

void CDrawDoc::OnUpdateShowAcDeadline(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(theApp.showAcDeadline);
}

/*BOOL CDrawDoc::DearchCompleted(int remnet_xx, BOOL isOK, CString msg) {
    for (const auto& pObj : m_objects) {
        CDrawAdd* pAdd = dynamic_cast<CDrawAdd*>(pObj);
        if (pAdd && pAdd->remnet_xx == remnet_xx) {
            ::InterlockedExchange((volatile LONG*)&pAdd->remnet_xx, 0);
            if (!isOK) {
                pAdd->flags.studio = StudioStatus::msg;
                pAdd->flags.showeps = FALSE;
                pAdd->remnetMsg = msg;
            } else {
                pAdd->flags.studio = StudioStatus::jest;
                pAdd->flags.showeps = TRUE;
                pAdd->remnetMsg = _T("");
            }
            pAdd->SetDirty();
            // popraw status og³oszenia w tabelce
            ::PostMessage(GetPanelView("CGridFrm")->m_hWnd, WM_ADDSARCH, 1, (LPARAM)pAdd);
            return TRUE;
        }
    }
    return FALSE;
}*/

void CDrawDoc::OnKratCalc()
{
    CKratCalc dlg;
    dlg.DoModal();
}

void CDrawDoc::OnAccGrb()
{
    CAccGrbDlg dlg;
    theManODPNET.GetAcceptStatus(m_mak_xx, dlg.m_print_ldrz, dlg.m_print_cdrz, dlg.m_print_org);

    if (dlg.DoModal() == IDOK) {
        uint8_t mask = 0;
        if (dlg.m_ckpldrz) mask += 4;
        if (dlg.m_ckporg)  mask += 8;
        if (dlg.m_ckpcdrz) mask += 16;

        CManODPNETParms orapar {
            { CManDbType::DbTypeInt32, &m_mak_xx },
            { CManDbType::DbTypeByte, &mask }
        };

        theManODPNET.EI("begin wyprzedzeniowe.set_accept_flag(:grb_xx,:flag); end;", orapar);
    }
}

void CDrawDoc::OnChangeCaptions()
{
    CString aux;
    isRED = 1 - isRED;
    SaveModified();
    for (const auto& pPage : m_pages) {
        aux = pPage->caption;
        pPage->caption = pPage->caption_alt;
        pPage->caption_alt = aux;
        pPage->Invalidate();
    }
}

float CDrawDoc::GetDrobneH()
{
    if (drobneH <= 0) {
        double dh;
        CManODPNETParms orapar {
            { CManDbType::DbTypeInt32, &m_mak_xx },
            { CManDbType::DbTypeDouble, CManDbDir::ParameterOut, &dh }
        };
        orapar.outParamsCount = 1;

        theManODPNET.EI("begin pagina.get_drobneh(:mak_xx,:drobneh); end;", orapar);
        drobneH = (float)dh;
    }

    return drobneH;
}
