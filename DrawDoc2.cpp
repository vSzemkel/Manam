
#include "StdAfx.h"
#include "DrawAdd.h"
#include "DrawDoc.h"
#include "DrawPage.h"
#include "DrawView.h"
#include "KolDlg.h"
#include "MainFrm.h"
#include "Manam.h"

extern BOOL drawErrorBoxes;
extern BOOL disableMenu;

/////////////// DB MAKIETA z DB
BOOL CDrawDoc::DBOpenDoc(TCHAR *sMakieta)
{
    if (!theApp.isRDBMS && !theApp.ConnecttoDB())
        return FALSE;

    CDBOpenDlg dlg;
    if (sMakieta) {
        TCHAR *mnam, *sep, *sep2;
        mnam = _tcsrchr(sMakieta, _T('\\'));
        if (!mnam) return FALSE;
        sep = _tcschr(++mnam, _T('_'));
        if (!sep) return FALSE;
        *sep++ = 0;
        sep2 = _tcschr(mnam, _T(' '));
        if (sep2) { // okreœlono mutacjê
            *sep2++ = 0;
            dlg.m_mutacja = sep2;
        } else
            dlg.m_mutacja = _T(" ");
        dlg.m_tytul = mnam;
        sep2 = _tcschr(sep, _T('.'));
        if (!sep2) return FALSE;
        *sep2 = 0;
        if (!isLIB) //.DB
            sep[2] = sep[5] = '/';
        dlg.m_dt = sep;
        dlg.m_doctype = static_cast<int>(iDocType);
    } else {
        if (dlg.DoModal() != IDOK) return FALSE;
        iDocType = (DocType)dlg.m_doctype;
        isLIB = iDocType == DocType::makieta_lib;
        isGRB = iDocType == DocType::grzbiet_drukowany;
        auto cnt = dlg.m_arrDaty.size();
        if (cnt > 0) {
            if (cnt > 52) {
                AfxMessageBox(_T("Jednym poleceniem mo¿na otworzyæ co najwy¿ej 52 makiety"));
                cnt = 52;
            }

            for (size_t i = 0; i < cnt; i++)
                theApp.OpenDocumentFile(dlg.m_arrDaty[i] + CDrawDoc::asDocTypeExt[(int)iDocType]);

            return FALSE;
        }
    }
    theApp.default_title = dlg.m_tytul;
    theApp.default_mut = dlg.m_mutacja;

    // moze byc juz otwarte
    auto frame = reinterpret_cast<CMainFrame*>(AfxGetMainWnd());
    auto pos = m_pDocTemplate->GetFirstDocPosition();
    while (pos != NULL) {
        auto openDoc = reinterpret_cast<CDrawDoc*>(m_pDocTemplate->GetNextDoc(pos));
        if (openDoc == this) continue;
        if (openDoc->data == dlg.m_dt && openDoc->gazeta == dlg.m_tytul + (dlg.m_mutacja == " " ? "" : " " + dlg.m_mutacja) && static_cast<int>(openDoc->iDocType) == dlg.m_doctype) {
            frame->MDIActivate(openDoc->GetFirstFrame());
            return FALSE;
        }
    }

    if (dlg.m_tytul.IsEmpty() || dlg.m_dt.IsEmpty()) {
        MessageBox(NULL, (LPCTSTR)_T("Proszê podaæ trzyliterowy kod tytu³u i datê emisji lub wersjê"), (LPCTSTR)_T("Brak danych"), MB_ICONSTOP);
        return FALSE;
    }

    frame->SetStatusBarInfo((LPCTSTR)_T("Proszê czekaæ. Trwa otwieranie makiety z bazy danych . . ."));
    theApp.BeginWaitCursor();

    data = dlg.m_dt;
    gazeta.Format(_T("%s %s"), dlg.m_tytul, dlg.m_mutacja);
    dayws = data.Mid(6, 4) + data.Mid(3, 2) + data.Mid(0, 2);
    daydir = dayws + _T("\\");
    disableMenu = TRUE;
    if (!theManODPNET.OpenManamDoc(this)) {
        if (m_mak_xx != 0) {
            theManODPNET.RmSysLock(this);
            if (!isGRB) AfxGetMainWnd()->MessageBox(_T("B³¹d czytania stron makiety"), gazeta + _T(" ") + data, MB_OK);
        }
        frame->SetStatusBarInfo(_T(""));
        return disableMenu = FALSE;
    }

    ComputeCanvasSize();
    ArrangeQue();
    if (isGRB) NumberPages();
    UpdateAllViews(NULL);

    TCHAR filedate[11];
    if (!isLIB) {
        _tcscpy_s(filedate, 11, data);
        filedate[2] = filedate[5] = '-';
    }
    if (theApp.showAcDeadline > 0) {
        theApp.showAcDeadline = 0;
        OnShowAcDeadline();
    }

    SetTitleAndMru();
    frame->SetStatusBarInfo("Gotowy - Makieta " + gazeta + " " + data);
    theApp.EndWaitCursor();
    SetModifiedFlag(FALSE);
    disableMenu = FALSE;
    return TRUE;
}

void CDrawDoc::OnDBSave()
{
    isGRB ? theManODPNET.GrbSaveMutczas(this) : DBSaveAs(m_mak_xx == -1);
}

void CDrawDoc::OnDBSaveAs()
{
    for (const auto& pPage : m_pages)
        pPage->dirty = TRUE;
    for (const auto& pObj : m_objects)
        pObj->dirty = TRUE;
    DBSaveAs(TRUE);
}

void CDrawDoc::DBSaveAs(BOOL isSaveAs)
{
    if (!theApp.isRDBMS) return;

    if (isSaveAs) {
        CDBSaveAsDlg dlg;
        dlg.m_tytmut = gazeta;
        dlg.m_lib = isLIB;
        if (dlg.DoModal() != IDOK) return;
        if ((isLIB = dlg.m_lib) > 0) {
            data.Format(_T("%03Iu%s"), m_pages.size(), static_cast<LPCTSTR>(dlg.m_dt));
            iDocType = DocType::makieta_lib;
        } else {
            data = dlg.m_dt;
            iDocType = DocType::makieta;
        }
    }

    if (!theManODPNET.CkAccess(gazeta, isLIB ? _T("WD") : _T("W"))) return;

    // sprawdz, czy s¹ og³oszenia poza makieta
    for (const auto& pObj : m_objects) {
        auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
        if (pAdd && !pAdd->fizpage) {
            if (AfxMessageBox(_T("Na makiecie s¹ og³oszenia, które nie zosta³y przyporz¹dkowane do ¿adnej kolumny. Czy mimo to chcesz zapisaæ makietê"), MB_YESNO) == IDNO) {
                disableMenu = FALSE;
                EndWaitCursor();
                return;
            }
            break;
        }
    }

    disableMenu = TRUE;
    BeginWaitCursor();

    BOOL doSaveAdds = (!isLIB && (m_mak_xx == -1 || !isSaveAs));
    ((CMainFrame*)AfxGetMainWnd())->SetStatusBarInfo((LPCTSTR)_T("Trwa zapis makiety ") + gazeta + _T(" ") + data);
    if (theManODPNET.SaveManamDoc(this, isSaveAs, doSaveAdds)) {
        if (!doSaveAdds) {
            auto pos = std::remove_if(m_objects.begin(), m_objects.end(), [](CDrawObj*& pObj) {
                if (dynamic_cast<CDrawAdd*>(pObj)) { delete pObj; return true; } else return false;
            });
            m_objects.erase(pos, m_objects.end());
        }
        ((CMainFrame*)AfxGetMainWnd())->SetStatusBarInfo((LPCTSTR)_T("Zachowywanie zakoñczone pomyœlnie."));
        SetModifiedFlag(FALSE);

        SetTitleAndMru();
        if (!doSaveAdds)
            UpdateAllViews(NULL, HINT_SAVEAS_DELETE_SELECTION, NULL);
    } else {
        if (isSaveAs) m_mak_xx = -1;
        ((CMainFrame*)AfxGetMainWnd())->SetStatusBarInfo((LPCTSTR)_T("Poczas zachowywania wyst¹pi³ b³¹d. Zadzwoñ: 56158 lub napisz: velvet@agora.pl"));
    }

    EndWaitCursor();
    disableMenu = FALSE;
}

void CDrawDoc::OnDBDelete()
{
    if (!theApp.isRDBMS) return;

    if (iDocType == DocType::makieta) // sprawdz czy s¹ og³oszenia poza makieta
        for (const auto& pObj : m_objects)
            if (dynamic_cast<CDrawAdd*>(pObj) != nullptr) {
                AfxMessageBox(_T("Przed usuniêciem makiety proszê rêcznie usun¹æ wszystkie og³oszenia, które na niej stoj¹."));
                return;
            }

    if (AfxMessageBox(_T("Czy na pewno chcesz usunaæ ") + CString(isGRB ? "ten grzbiet" : "tê makietê"), MB_OKCANCEL | MB_ICONQUESTION) != IDOK) return;

    char* sql = reinterpret_cast<char*>(theApp.bigBuf);
    ::StringCchPrintfA(sql, bigSize, "begin %s%s(:mak_xx); end;", isGRB ? "grb.delete_grzbiet" : "delete_makieta", isLIB ? "_lib" : "");
    CManODPNETParms orapar { CManODPNET::DbTypeInt32, &m_mak_xx };
    if (theManODPNET.EI(sql, orapar)) {
        m_mak_xx = -1;
        SetModifiedFlag(FALSE);
        OnCloseDocument();
    }
}

void CDrawDoc::ZmianaSpotow(int n)
{
    // gdy dodaje strony byc moze mam wiecej mozliwosci spotu, pod warunkiem ze mam wybrane drzewo
    if (id_drw == -1) return;
    int m = DBReadSpot(n);
    if (m_spot_makiety.size() == m) return;
    for (const auto& p : m_pages) {
        const int e1 = p->kolor & c_spot;
        const int e2 = p->kolor >> 3;
        if ((e1 == c_spot) && (e2 >= m))
            p->ChangeKolor(c_brak);
    }
    ((CMainFrame*)AfxGetMainWnd())->InsComboNrSpotow(m);
    m_spot_makiety.resize(m);
}

int CDrawDoc::DBReadSpot(int n)
{
    if (!theApp.isRDBMS) return 0;

    int ile_spotow = -1;
    CManODPNETParms orapar {
        { CManODPNET::DbTypeInt32, &id_drw },
        { CManODPNET::DbTypeInt32, &n },
        { CManODPNET::DbTypeInt32, CManODPNET::ParameterOut, &ile_spotow } };
    orapar.outParamsCount = 1;

    theManODPNET.EI("begin select_spot(:drw_xx,:obj,:ile); end;", orapar);

    return ile_spotow;
}

void CDrawDoc::ZmianaCaptions(int old_id_drw, int new_id_drw)
{
    //gdy dodaje strony byc moze mam wiecej mozliwosci spotu, pod warunkiem ze mam wybrane drzewo
    if (theApp.isRDBMS && ((CDrawApp*)AfxGetApp())->GetProfileInt(_T("General"), _T("Captions"), 1) == 0)
        for (const auto& p : m_pages)
            p->DBChangeName(new_id_drw);
    ((CMainFrame*)AfxGetMainWnd())->IniCaptionBox(old_id_drw, new_id_drw);
}

#ifdef _DEBUG
void CDrawDoc::AssertValid() const
{
    COleDocument::AssertValid();
}

void CDrawDoc::Dump(CDumpContext& dc) const
{
    COleDocument::Dump(dc);
}
#endif //_DEBUG

void CDrawDoc::OnVuMakietowanie()
{
    /* vu:  Funkcja stara sie przypisac strony wszystkim ogloszeniom przypisanym do dokumentu,
            ktore jeszcze nie maja okreslonego numeru strony fizycznej. Ogloszenia sa wstepnie
            sortowane po zlozonosci warunku logicznego. Dalej dla kazdego ogloszenia ustalana
            jest lista stron, na ktorych mozna je postawic. Z uzyskanej listy wybiera sie te,
            na ktorej jest najwiecej wolnego miejsca. Po skojarzeniu ogloszenia ze strona wolana
            jest procedura makietujaca ogloszenia znajdujace sie na kazdej ze stron. Nastepnie
            wywoluje polecenie odswiezenia wszystkich widokow biezacego dokumentu. Algorytm nie
            postawi og³oszenia na stronie, gdy ich kraty nie s¹ zgodne					end vu */

    CObArray vAdds;
    CDrawPage *vTmpPage, *vPage = NULL;
    CDrawAdd *castval;
    CRect vRect;

    theApp.BeginWaitCursor();

    //przygotuj liste ogloszen niezablokowanych, ktore nie maja jeszcze numeru strony
    //oraz tych, które stoj¹ na stronach makietowanych
    for (const auto& pObj : m_objects) {
        castval = dynamic_cast<CDrawAdd*>(pObj);
        if (castval) {
            if (castval->flags.locked || castval->flags.derived)
                continue;
            if (!castval->fizpage)
                vAdds.Add(castval);
            else if (theApp.makietujAll) {
                vPage = GetPage(castval->fizpage);
                if (!vPage->niemakietuj) {
                    castval->fizpage = 0;
                    vAdds.Add(castval);
                    vPage->RemoveAdd(castval); //zwolnij miejsce na stronie
                }
            }
        }
    }

    int i, j, k, maxLen, maxWeight, vAddsCount = (int)vAdds.GetSize();
    //posortuj tablice po dlugosci warunku logicznego, malejaco
    for (i = 0; i < vAddsCount - 1; ++i) {
        k = -1;
        auto viAdd = (CDrawAdd*)vAdds.GetAt(i);
        maxLen = viAdd->logpage.GetLength();
        maxWeight = viAdd->sizex*viAdd->sizey;
        for (j = i + 1; j < vAddsCount; j++) {
            auto vjAdd = (CDrawAdd*)vAdds.GetAt(j);
            if (vjAdd->logpage.GetLength() > maxLen) {
                maxLen = vjAdd->logpage.GetLength();
                k = j;
            }
            if (vjAdd->logpage.GetLength() == maxLen) { //warunek z '=' ma wiekszy prirytet
                if (_tcsstr(vjAdd->logpage, _T("=")) && !_tcsstr(viAdd->logpage, _T("=")))
                    k = j;
                else if (maxWeight < vjAdd->sizex*vjAdd->sizey) {
                    maxWeight = vjAdd->sizex*vjAdd->sizey;
                    k = j;
                }
            }
        }
        if (k > -1) {
            castval = (CDrawAdd*)vAdds.GetAt(i);
            vAdds.SetAt(i, vAdds.GetAt(k));
            vAdds.SetAt(k, castval);
        }
    }

    //dla kazdego ogloszenia ustal liste stron fizycznych, na ktore moze trafic
    BOOL hashed = FALSE;
    for (i = 0; i < vAddsCount; ++i) {
        CString sOrgLogpage;
        BOOL bSecondRun = FALSE;
aSecondRun:
        int nr_sek, nr_pl, nr_off, vnr_sek = 0, pc = (int)m_pages.size();
        TCHAR op_zew[2], op_sekcji[2], op_pl[2], sekcja[30], pl[2];
        castval = (CDrawAdd*)vAdds.GetAt(i);
        castval->logpage.Trim().MakeUpper();
        //parsuj napis i ustaw zmienne 
        castval->ParseLogpage(op_zew, sekcja, op_sekcji, &nr_sek, pl, op_pl, &nr_pl);
        //przygotuj liste stron, pamietaj jaki numer ma strona w danej sekcji 
        CObArray vPages;
        for (j = 1; j <= pc; j++) {
            vPage = m_pages[j % pc];
            //interesuj¹ nas tylko strony makietowane
            if (vPage->niemakietuj) continue;
            CString vStrLog = CString(vPage->name);
            vStrLog.MakeUpper();
            //zliczaj strony z odpowiedni¹ sekcj¹
            if (_tcsstr(vStrLog, sekcja))
                vnr_sek++;
            //ró¿ne kratki
            if (castval->szpalt_x != vPage->szpalt_x || castval->szpalt_y != vPage->szpalt_y) continue;
            //nie zgodne kolory - kolor strony 2 oznacza spot anonimowy
            if ((castval->kolor & 7) > (vPage->kolor & 7) ||
                ((castval->kolor&c_spot) && (vPage->kolor&c_spot) && (castval->kolor >> 3) != m_spot_makiety[(vPage->kolor >> 3)] && m_spot_makiety[(vPage->kolor >> 3)])) continue;
            //nie na sciezce
            if (sekcja[0]) {
                CString sSekOgl("/");
                sSekOgl.Append(sekcja);							//nie dopasowujemy czêœciowych wêz³ów œcie¿ki
                int iPos = vStrLog.Find(sSekOgl);				//pierwsze dopasowanie wyszukuje podci¹g
                if (iPos == -1) continue;						//drugi dok³adnie, na koñcu œcie¿ki
                while (vStrLog.Find(sSekOgl, iPos + 1) > 0)
                    iPos = vStrLog.Find(sSekOgl, iPos + 1);
                if (bSecondRun && iPos + sSekOgl.GetLength() != vStrLog.GetLength()) continue;
            }
            //lewa - nieparzysty numer
            if (!_tcscmp(pl, _T("L")) && (vPage->pagina % 2)) continue;
            //prawa - parzysty numer
            if (!_tcscmp(pl, _T("P")) && !(vPage->pagina % 2)) continue;
            //polozenie pl
            if (pl[0] && op_pl[0]) {
                //brak numeru
                if (nr_pl < 1) continue;
                if ((vPage->pagina - vnr_sek) % 2)	//czy sekcja zaczyna sie na stronie o parzstym numerze
                    nr_off = (sekcja[0] ? ((vPage->pagina % 2) ? vnr_sek - 1 : vnr_sek + 1) : vPage->pagina); //czy numer odnosi sie do sekcji czy do gazety
                else
                    nr_off = (sekcja[0] ? vnr_sek : vPage->pagina); //czy numer odnosi sie do sekcji czy do gazety
                //inna strona prawa/lewa niz zadano
                if (!_tcscmp(op_pl, _T("=")) && !_tcscmp(pl, _T("P")) && nr_off != 2 * nr_pl - 1) continue;
                if (!_tcscmp(op_pl, _T("=")) && !_tcscmp(pl, _T("L")) && nr_off != 2 * nr_pl) continue;
                //strona prawa/lewa o mniejszym numerze niz zadano
                if (!_tcscmp(op_pl, _T(">")) && !_tcscmp(pl, _T("P")) && nr_off <= 2 * nr_pl - 1) continue;
                if (!_tcscmp(op_pl, _T(">")) && !_tcscmp(pl, _T("L")) && nr_off <= 2 * nr_pl) continue;
                //strona prawa/lewa o wiekszym numerze niz zadano
                if (!_tcscmp(op_pl, _T("<")) && !_tcscmp(pl, _T("P")) && nr_off >= 2 * nr_pl - 1) continue;
                if (!_tcscmp(op_pl, _T("<")) && !_tcscmp(pl, _T("L")) && nr_off >= 2 * nr_pl) continue;
            }
            //polozenie w sekcji
            if (sekcja[0] && op_sekcji[0]) {
                //brak numeru
                if (nr_sek < 1) continue;
                //inna strona w sekcji niz zadano
                if (!_tcscmp(op_sekcji, _T("=")) && vnr_sek != nr_sek) continue;
                //strona o mniejszym numerze w sekcji niz zadano
                if (!_tcscmp(op_sekcji, _T(">")) && vnr_sek <= nr_sek) continue;
                //strona o wiekszym numerze w sekcji niz zadano
                if (!_tcscmp(op_sekcji, _T("<")) && vnr_sek >= nr_sek) continue;
            }
            //polozenie na stronie
            hashed = castval->logpage.ReverseFind('#') >= 0;
            if (hashed) {
                if (!castval->SetPagePosition(NULL, vPage))
                    continue;
            } else {
                k = 1;
                if (!vPage->FindSpace(castval, &k, &k, castval->sizex, castval->sizey))
                    continue;
            }
            //ta strona spe³nia wszystkie warunki
            vPages.Add(vPage);
        }

        k = (int)vPages.GetSize();
        if (!k && castval->logpage.GetLength() == 5) { // nie uda³o siê dopasowaæ wertykalu np. BIZIR
            bSecondRun = TRUE;
            sOrgLogpage = castval->logpage;
            castval->logpage = castval->logpage.Mid(3); // spróbujemy dopasowaæ do strony IR
            goto aSecondRun;
        } else if (bSecondRun)
            castval->logpage = sOrgLogpage;
        if (!k)
            continue; // nie znaleziono zadnej strony dla tego ogloszenia

        // wybierz najmniej lub najbardziej zajeta strone z listy - makietujDoKupy
        maxWeight = (int)1E4;
        for (j = 0; j < k; j++) {
            vTmpPage = (CDrawPage*)vPages.GetAt(j);
            int m = 0, ile_mod = vTmpPage->szpalt_x*vTmpPage->szpalt_y;
            for (int l = 0; l < ile_mod; l++)
                m += (int)((vTmpPage->space >> l) & 1);
            if (theApp.makietujDoKupy)
                m = ile_mod - m;
            if (m < maxWeight) {
                maxWeight = m;
                vPage = vTmpPage;
            }
            if (!maxWeight) break;
        }

        if (hashed) // ustaw na zadanym miejscu na stronie
            castval->SetPagePosition(&vRect, vPage);
        else if (vPage) //ustaw ogloszenie w prawym dolnym rogu strony
            vRect = CRect(vPage->m_position.right - (int)((castval->sizex)*CDrawObj::modx(castval->szpalt_x)), vPage->m_position.bottom + (int)(castval->sizey*CDrawObj::mody(castval->szpalt_y)), vPage->m_position.right, vPage->m_position.bottom);

        castval->SetPosition(&vRect, vPage);
        castval->m_position = vRect;
        castval->Invalidate();
    } // end of dla kazdego ogloszenia ustal liste stron fizycznych, na ktore moze trafic

    // Makietuj kazda strone z osobna
    for (const auto& p : m_pages)
        MakietujStrone(p);

    // ustaw z boku te, ktorych sie nie zmakietowalo automatycznie
    AsideAdds();

    UpdateAllViews(NULL);

    theApp.EndWaitCursor();
}

void CDrawDoc::MakietujStrone(CDrawPage *pPage)
{
    /* vu: 	Jedynym argumentem jest referencja do strony, ktora zostanie przemakietowana.
            Przemakietowanie polega na przeniesieni ogloszen ze strony do tabeli lokalnej,
            posortowaniu jej i ponownego postawienia na stronie jej ogloszen z tym, ze tym
            razem wedlug porzadku ustalonego w algorytmie sortujacym. Uwaga: funkcje nie
            odswieza widoku - musi to zrobic funkcja wywolujaca. Uwzglêdnia flagê niemakietuj. end vu */

    if (pPage->niemakietuj) return;

    CObArray vAdds;
    int i, j, k;

    // utworz liste ogloszen na stronie 
    pPage->m_adds.erase(std::remove_if(pPage->m_adds.begin(), pPage->m_adds.end(), [=, &vAdds](CDrawAdd *pAdd) -> BOOL {
        if (!pAdd->flags.locked && !pAdd->flags.derived) {
            vAdds.Add(pAdd);
            pAdd->fizpage = 0;
            pPage->RemoveAdd(pAdd, FALSE);
            return TRUE;
        }
        return FALSE;
    }), pPage->m_adds.end());

    // posortuj liste pod wzgledem kryteriow rozmieszczenia ogloszenia - {skomplikowane == poczatek listy}
    const int vAddsCount = (int)vAdds.GetSize();
    for (i = 0; i < vAddsCount - 1; ++i) {
        k = -1;
        auto viAdd = (CDrawAdd*)vAdds.GetAt(i);
        BOOL hashedi = viAdd->logpage.ReverseFind('#') >= 0;
        // pasek ma najwyzszy priorytet
        if (hashedi && (viAdd->sizex == viAdd->szpalt_x && viAdd->sizey == 1) || (viAdd->sizey == viAdd->szpalt_y && viAdd->sizex == 1)) continue;
        int maxWeight = viAdd->sizex*viAdd->sizey;
        for (j = i + 1; j < vAddsCount; j++) {
            auto vjAdd = (CDrawAdd*)vAdds.GetAt(j);
            BOOL hashedj = vjAdd->logpage.ReverseFind('#') >= 0;
            if (hashedi && !hashedj) continue;
            if (!hashedi && hashedj) { // po znalezieniu pierwszego hashowanego sprawdzaj tylko hashowane
                hashedi = TRUE;
                maxWeight = vjAdd->sizex * vjAdd->sizey;
                vAdds.SetAt(i, vjAdd);
                vAdds.SetAt(j, viAdd);
                viAdd = vjAdd;
                k = -1;
                continue;
            }
            if (hashedi && hashedj) { // d³u¿szy warunek po '#' ma wiêkszy priorytet
                if (viAdd->logpage.GetLength() - viAdd->logpage.ReverseFind('#') < vjAdd->logpage.GetLength() - vjAdd->logpage.ReverseFind('#')) {
                    vAdds.SetAt(i, vjAdd);
                    vAdds.SetAt(j, viAdd);
                    viAdd = vjAdd;
                    k = -1;
                    continue;
                }
                maxWeight = __max(maxWeight, vjAdd->sizex * vjAdd->sizey);
            }
            if ((viAdd->sizex == viAdd->szpalt_x && viAdd->sizey == 1) || (viAdd->sizey == viAdd->szpalt_y && viAdd->sizex == 1)) continue;
            if ((vjAdd->sizex == viAdd->szpalt_x && vjAdd->sizey == 1) || (vjAdd->sizey == viAdd->szpalt_y && vjAdd->sizex == 1)) { //paski
                maxWeight = viAdd->szpalt_x * viAdd->szpalt_y;
                k = j;
                break;
            }
            if (vjAdd->sizex*vjAdd->sizey > maxWeight) { // waga
                maxWeight = vjAdd->sizex * vjAdd->sizey;
                k = j;
                continue;
            }
            if (k >= 0 && vjAdd->sizex * vjAdd->sizey == maxWeight) { // ogloszenie o wiekszej przekatnej ma wiekszy priorytet
                auto vkAdd = (CDrawAdd*)vAdds.GetAt(k);
                if (__max(vjAdd->sizex, vjAdd->sizey) > __max(vkAdd->sizex, vkAdd->sizey)) k = j;
            }
        }
        if (k > -1) {
            auto castval = (CDrawAdd*)vAdds.GetAt(i);
            vAdds.SetAt(i, vAdds.GetAt(k));
            vAdds.SetAt(k, castval);
        }
    }

    // Powrzucaj ogloszenia na strone w odpowiednim porzadku
    for (i = 0; i < vAddsCount; ++i) {
        auto castval = (CDrawAdd*)vAdds.GetAt(i);
        int hashpos = castval->logpage.ReverseFind('#');
        CRect vRect;
        if (hashpos >= 0) {
            k = 0;
            TCHAR tail[20];
            for (j = hashpos + 1; j < castval->logpage.GetLength(); j++)
                if (isupper((int)castval->logpage[j])) tail[k++] = castval->logpage[j];
            tail[k] = '\0';
            if (_tcsstr(tail, _T("DL")))
                castval->SetEstPagePos(_T("DL"), &vRect, pPage);
            else if (_tcsstr(tail, _T("DP")))
                castval->SetEstPagePos(_T("DP"), &vRect, pPage);
            else if (_tcsstr(tail, _T("D")))
                castval->SetEstPagePos(_T("D"), &vRect, pPage);
            else if (_tcsstr(tail, _T("GP")))
                castval->SetEstPagePos(_T("GP"), &vRect, pPage);
            else if (_tcsstr(tail, _T("GL")))
                castval->SetEstPagePos(_T("GL"), &vRect, pPage);
            else if (_tcsstr(tail, _T("G")))
                castval->SetEstPagePos(_T("G"), &vRect, pPage);
            else if (_tcsstr(tail, _T("L")))
                castval->SetEstPagePos(_T("L"), &vRect, pPage);
            else if (_tcsstr(tail, _T("P")))
                castval->SetEstPagePos(_T("P"), &vRect, pPage);
            else { // póki co rób to, co nizej
                vRect = CRect(pPage->m_position.right - (castval->sizex)*pmodulx, pPage->m_position.bottom + castval->sizey*pmoduly, pPage->m_position.right, pPage->m_position.bottom);
                castval->SetPosition(&vRect, pPage);
            }
        } else {
            // ustaw ogloszenie bez opisu na prawej stronie w prawym dolnym rogu strony, na lewej - w lewym dolnym
            castval->SetEstPagePos((pPage->pagina & 1 ? _T("DP") : _T("DL")), &vRect, pPage);
        }
        castval->m_position = vRect;
        castval->Invalidate();
        castval->UpdateInfo();
    }
}

void CDrawDoc::OnVuCkMakietowanie()
{
    /* vu : w³¹cza/wy³¹cza sygnalizacjê Ÿle postawionych ogloszen
            Odswieza widoki                                end vu */

    drawErrorBoxes = !drawErrorBoxes;

    CDrawView* vView = GetPanelView<CDrawView>();
    if (vView) vView->Invalidate(FALSE);
}

void CDrawDoc::AdvanceAsidePos(CPoint& p) const
{
    if (pszpalt_y*pmoduly - p.y > m_size.cy*vscale) {
        p.y = -pmoduly;
        p.x -= pszpalt_x*pmodulx;
    } else
        p.y -= pmoduly;
}

void CDrawDoc::AsideAdds()
{
    /* vu : Ustawia ogloszenia, ktore nie maja swoich stron z boku makiety	end vu */

    CPoint vPoint(GetAsideAddPos(TRUE));

    //przegladaj liste ogloszen, ktore nie maja jeszcze numeru strony
    for (const auto& pObj : m_objects) {
        auto vAdd = dynamic_cast<CDrawAdd*>(pObj);
        if (vAdd && vAdd->fizpage == 0) {
            vAdd->m_position = CRect(vPoint, vAdd->m_position.Size());
            vAdd->posx = vAdd->posy = 0;
            vAdd->Invalidate();
            AdvanceAsidePos(vPoint);
        }
    }
}

CPoint CDrawDoc::GetAsideAddPos(BOOL opening) const
{
    const int addsAsideCnt = opening ? 1 : 1 + (int)std::count_if(cbegin(m_objects), cend(m_objects), [](CDrawObj *pObj) { auto a = dynamic_cast<CDrawAdd*>(pObj); return a && a->posx == 0; });
    const int marginPageMinX = (int)(pmodulx*(1 + iPagesInRow / 2 + (pszpalt_x*iPagesInRow)));
    const int marginPageMaxX = (int)(nearbyint((float)(m_size.cx * vscale * 100) / (theApp.m_initZoom * pmodulx)) - (pszpalt_x + 2)) * pmodulx;
    return CPoint(max(marginPageMinX, marginPageMaxX), -pmoduly * addsAsideCnt);
}

void CDrawDoc::ArrangeQue()
{
    CPoint vPoint(pmodulx, -pmoduly);
    const int iPageWidth = (int)(pszpalt_x * CDrawObj::modx(pszpalt_x));

    for (const auto& a : m_addsque) {
        a->m_position.SetRect(vPoint.x, vPoint.y, vPoint.x + (int)(a->sizex * CDrawObj::modx(a->szpalt_x)), (int)(vPoint.y - a->sizey * CDrawObj::mody(a->szpalt_y)));
        vPoint.x += a->m_position.Width();
        if (vPoint.x > m_size.cx * vscale - iPageWidth) {
            vPoint.x = pmodulx;
            vPoint.y -= pszpalt_y * pmoduly;
        }
    }
    UpdateAllViews(NULL);
}

void CDrawDoc::AddFind(long nrAtex, long nrSpacer, LPCTSTR nazwa)
{
    CDrawAdd* vAdd = nullptr;
    if (nrAtex != LONG_MIN)  // szukanie po numerze ATEX
        vAdd = AddExists(nrAtex);
    else { // szukanie po nazwie lub spacerze
        int i = 0;
        for (const auto& pObj : m_objects) {
            vAdd = dynamic_cast<CDrawAdd*>(pObj);
            if (vAdd && i > findNextInd) {
                if ((long)vAdd->m_add_xx == nrSpacer) {
                    findNextInd = i;
                    break;
                }
                CString addNazwa = vAdd->nazwa;
                addNazwa.MakeUpper();
                if (addNazwa.Find(nazwa) >= 0) {
                    findNextInd = i;
                    break;
                }
                vAdd = nullptr;
            }
            i++;
        }
    }

    if (vAdd)
        SelectAdd(vAdd, 0);
    else {
        if (nrAtex == LONG_MIN) nrAtex = nrSpacer;
        CString snr;
        if (nrAtex > 0) snr.Format(_T("%ld"), nrAtex);
        AfxMessageBox(CString(_T("Nie odnaleziono og³oszenia: ")) + (snr.IsEmpty() ? nazwa : snr));
        findNextInd = -1;
    }
}

void CDrawDoc::OnAddFind()
{
    findNextInd = -1;
    CAddFindDlg dlg;
    dlg.m_pObList = &m_objects;
    if (dlg.DoModal() != IDOK) return;
    dlg.m_nazwa.MakeUpper();
    lastSearchNrAtex = (dlg.m_nreps == 0L ? LONG_MIN : dlg.m_nreps);
    lastSearchNrSpacer = (dlg.m_spacer == 0L ? LONG_MIN : dlg.m_spacer);
    lastSearchNazwa = (dlg.m_nazwa.IsEmpty() ? _T("“") : (LPCTSTR)dlg.m_nazwa);
    AddFind(lastSearchNrAtex, lastSearchNrSpacer, lastSearchNazwa);
}

void CDrawDoc::OnEditFindNext()
{
    AddFind(lastSearchNrAtex, lastSearchNrSpacer, lastSearchNazwa);
}

void CDrawDoc::OnUpdateEditFindNext(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(!disableMenu && findNextInd > -1);
}

void CDrawDoc::OnDrawOpcje()
{
    CConfDlg dlg;
    theApp.SetRegistryBase(_T("GenEPS"));
    dlg.m_contnum = theApp.GetProfileInt(_T("General"), _T("CiaglaNumeracja"), 0);
    dlg.m_strcnt = theApp.GetProfileInt(_T("General"), _T("IloscKolumn"), 44);
    dlg.m_iPagesInRow = theApp.GetProfileInt(_T("General"), _T("PagesInRow"), 0);
    dlg.m_deadline = theApp.GetProfileString(_T("General"), _T("Deadline"), _T("18:30"));
    dlg.m_format = theApp.colsPerPage - 1;
    dlg.m_makietujDoKupy = theApp.makietujDoKupy;
    dlg.m_makietujAll = theApp.makietujAll;
    dlg.m_opi_mode = (BOOL)theApp.isOpiMode;
    dlg.m_epsold = theApp.GetString(_T("EpsOld"), _T(""));
    dlg.m_epssrc = theApp.GetString(_T("EpsSrc"), _T(""));
    dlg.m_epsirf = theApp.GetString(_T("EpsIrfan"), _T(""));
    dlg.m_hashing = theApp.GetInt(_T("Hashing"), 2);
    dlg.m_subdir = theApp.GetInt(_T("SubDirSearch"), 1);
    dlg.m_daydirs = theApp.GetInt(_T("DayDirs"), 0);
    dlg.m_copyold = theApp.GetInt(_T("CopyOldEPS"), 1);
    dlg.m_podwaly = theApp.GetInt(_T("Podwaly"), 0);
    dlg.m_podwal_subdir = theApp.GetInt(_T("PodwalySubDir"), 0);
    dlg.m_epskok = theApp.GetString(_T("KorektaDobre"), _T(""));

    theManODPNET.LoadMakietaDirs(m_mak_xx);
    dlg.m_epsdst = theApp.GetString(_T("EpsDst"), _T(""));
    dlg.m_psdst = theApp.GetString(_T("PsDst"), _T(""));
    dlg.m_epszaj = theApp.GetString(_T("EpsZajawki"), _T(""));
    dlg.m_epspod = theApp.GetString(_T("EpsPodwaly"), _T(""));
    dlg.m_epsdro = theApp.GetString(_T("EpsDrobne"), _T(""));
    dlg.m_epsuzu = theApp.GetString(_T("EpsUzupel"), _T(""));

    if (dlg.DoModal() != IDOK) return;
    theApp.WriteProfileInt(_T("General"), _T("CiaglaNumeracja"), dlg.m_contnum);
    theApp.WriteProfileInt(_T("General"), _T("IloscKolumn"), dlg.m_strcnt);
    theApp.WriteProfileInt(_T("General"), _T("PagesInRow"), dlg.m_iPagesInRow);
    theApp.WriteProfileString(_T("General"), _T("Deadline"), dlg.m_deadline);
    theApp.colsPerPage = dlg.m_format + 1;
    theApp.makietujDoKupy = dlg.m_makietujDoKupy;
    theApp.makietujAll = dlg.m_makietujAll;
    theApp.isOpiMode = dlg.m_opi_mode;

    if (dlg.m_save_dirs) {
        CManODPNETParms orapar {
            { CManODPNET::DbTypeInt32, &m_mak_xx },
            { CManODPNET::DbTypeVarchar2, &dlg.m_epsdst },
            { CManODPNET::DbTypeVarchar2, &dlg.m_psdst },
            { CManODPNET::DbTypeVarchar2, &dlg.m_epszaj },
            { CManODPNET::DbTypeVarchar2, &dlg.m_epspod },
            { CManODPNET::DbTypeVarchar2, &dlg.m_epsdro },
            { CManODPNET::DbTypeVarchar2, &dlg.m_epsuzu }
        };

        theManODPNET.EI("begin dir.makieta_save(:mak_xx,:wynik_eps,:wynik_ps,:zajawki,:podwaly,:drobne,:uzupel); end;", orapar);
    } else {
        theApp.WriteString(_T("EpsDst"), dlg.m_epsdst);
        theApp.WriteString(_T("PsDst"), dlg.m_psdst);
        theApp.WriteString(_T("EpsZajawki"), dlg.m_epszaj);
        theApp.WriteString(_T("EpsDrobne"), dlg.m_epsdro);
        theApp.WriteString(_T("EpsPodwaly"), dlg.m_epspod);
        theApp.WriteString(_T("EpsUzupel"), dlg.m_epsuzu);
    }
    theApp.WriteString(_T("EpsOld"), dlg.m_epsold);
    theApp.WriteString(_T("EpsSrc"), dlg.m_epssrc);
    theApp.WriteString(_T("EpsIrfan"), dlg.m_epsirf);
    theApp.WriteString(_T("KorektaDobre"), dlg.m_epskok);
    theApp.WriteInt(_T("Hashing"), dlg.m_hashing);
    theApp.WriteInt(_T("SubDirSearch"), dlg.m_subdir);
    theApp.WriteInt(_T("DayDirs"), dlg.m_daydirs);
    theApp.WriteInt(_T("CopyOldEPS"), dlg.m_copyold);
    theApp.WriteInt(_T("Podwaly"), dlg.m_podwaly);
    theApp.WriteInt(_T("PodwalySubDir"), dlg.m_podwal_subdir);
}

int CDrawDoc::GetIPage(int n) const noexcept
{
    for (int i = 0; i < (int)m_pages.size(); i++)
        if (m_pages[i]->nr == n)
            return i;
    return -1;
}

int CDrawDoc::GetIPage(CDrawPage *pPage) const noexcept
{
    auto pos = std::find(std::cbegin(m_pages), std::cend(m_pages), pPage);
    return pos == std::cend(m_pages) ? -1 : static_cast<int>(pos - std::cbegin(m_pages));
}

void CDrawDoc::OnAsideAdds()
{
    CDrawPage* vPage = nullptr;
    CDrawView *pView = GetPanelView<CDrawView>();
    if (pView->m_selection.size() == 1)
        vPage = dynamic_cast<CDrawPage*>(pView->m_selection.front());

    if (vPage == nullptr) {
        const auto pcount = m_pages.size();
        for (size_t i = 0; i < pcount; i++)
            if (!(m_pages[i])->niemakietuj) {
                vPage = m_pages[i];
                vPage->m_adds.erase(std::remove_if(vPage->m_adds.begin(), vPage->m_adds.end(), [=](CDrawAdd *pAdd) -> BOOL {
                    if (!pAdd->flags.locked && !pAdd->flags.derived) {
                        pAdd->fizpage = 0;
                        pAdd->UpdateInfo();
                        m_pages[i]->RemoveAdd(pAdd, FALSE);
                        return TRUE;
                    }
                    return FALSE;
                }), vPage->m_adds.end());
            }
        AsideAdds();
    } else { // zdejmujemy og³oszenia ze wskazanej strony (wiemy, ¿e to strona makietowana)
        int iOffset = iPagesInRow - (GetIPage(vPage) % iPagesInRow);
        iOffset = iOffset*pszpalt_x*pmodulx + (iOffset / 2 + 1)*pmodulx;
        vPage->m_adds.erase(std::remove_if(vPage->m_adds.begin(), vPage->m_adds.end(), [=](CDrawAdd *pAdd) -> BOOL {
            if (!pAdd->flags.locked && !pAdd->flags.derived) {
                pAdd->fizpage = 0;
                pAdd->m_position.left += iOffset;
                pAdd->m_position.right += iOffset;
                pAdd->UpdateInfo();
                vPage->RemoveAdd(pAdd, FALSE);
                return TRUE;
            }
            return FALSE;
        }), vPage->m_adds.end());
    }

    UpdateAllViews(NULL);
    SetModifiedFlag(TRUE);
}

////////////////////////////////////////////// ini kolory

void CDrawDoc::IniKolorTable()
{
    int max_col = theApp.GetProfileInt(_T("SpotColors"), _T("Amount"), 0);
    kolory.resize(max_col + 2);
    brushe.resize(max_col + 2);
    kolory[0] = BRAK; brushe[0] = new CBrush(RGB(190, 190, 190));
    kolory[1] = FULL; brushe[1] = new CBrush(BIALY);
    CString bf;
    for (int i = 1; i <= max_col; ++i) {
        bf.Format(_T("%i"), i);
        kolory[i + 1] = theApp.GetProfileString(_T("SpotColors"), _T("Spot") + bf, _T("nie ma")).MakeUpper();
        brushe[i + 1] = new CBrush(theApp.GetProfileInt(_T("SpotColors"), _T("Color") + bf, BIALY));
    }
}

void CDrawDoc::IniRozm()
{
    m_Rozm.clear();

    if (theApp.isRDBMS)
        theManODPNET.InitRozm(this);

    if (!m_Rozm.empty()) return;
    m_Rozm.emplace_back(); // krata 5x6
    theApp.SetRegistryBase(_T("EPSKratka"));
    int i, n = theApp.GetInt(_T("Amount"), 0);
    CString bf;
    for (i = 1; i <= n; ++i) {
        bf.Format(_T("%i"), i); // tylko kraty inne niz 5x6
        m_Rozm.emplace_back(theApp.GetInt(_T("w") + bf, 0), theApp.GetInt(_T("h") + bf, 0),
            theApp.GetInt(_T("sw") + bf, 0), theApp.GetInt(_T("sh") + bf, 0),
            theApp.GetInt(_T("szpalt_x") + bf, 0), theApp.GetInt(_T("szpalt_y") + bf, 0), 0, FALSE);
    }
    n = theApp.GetProfileInt(_T("EPSNiekratowe"), _T("Amount"), 0);
    for (i = 1; i <= n; ++i) {
        bf.Format(_T("%i"), i);
        m_Rozm.emplace_back(theApp.GetProfileInt(_T("EPSNiekratowe"), _T("w") + bf, 0), theApp.GetProfileInt(_T("Niekratowe"), _T("h") + bf, 0), 0, 0, 0, 0,
            theApp.GetProfileInt(_T("EPSNiekratowe"), _T("typ_xx") + bf, 0), FALSE);
    }
} // IniRozm

const CRozm* CDrawDoc::GetCRozm(int s_x, int s_y, int typ_xx)
{
    GENEPSARG genEpsArg;
    return GetCRozm(&genEpsArg, s_x, s_y, typ_xx);
}

const CRozm* CDrawDoc::GetCRozm(PGENEPSARG pArg, int s_x, int s_y, int typ_xx)
{
    auto pR = std::find_if(m_Rozm.cbegin(), m_Rozm.cend(), [=](const CRozm& r) {
        return (typ_xx == 0 && s_x == r.szpalt_x && s_y == r.szpalt_y) || (typ_xx > 0 && typ_xx == r.typ_xx);
    });

    if (typ_xx > 0 && pR == m_Rozm.cend()) { // doczytaj wymiar niestandardowy przypisany do wszystkich produktów
        auto pRozm = theManODPNET.AddRozmTypu(m_Rozm, typ_xx);
        if (pRozm) return pRozm;
    }

    return pR == m_Rozm.cend() ? (typ_xx ? GetCRozm(pArg, s_x, s_y, 0) : nullptr) : &*pR;
}

void CDrawDoc::DerivePages(CDrawPage *pPage)
{
    int nr_porz, npages = (int)m_pages.size();
    if (!npages) return;
    if (!pPage) {
        nr_porz = npages > 1 ? 1 : 0;
        pPage = m_pages[nr_porz];
    } else
        nr_porz = GetIPage(pPage);
    if (theApp.isRDBMS && !this->isGRB && !this->isLIB && !this->isRO && this->SaveModified() && !pPage->dirty) {
        CPageDerv dlg;
        dlg.m_mak_xx = this->m_mak_xx;
        dlg.m_idervlvl = pPage->m_dervlvl;
        dlg.m_nr = nr_porz ? nr_porz : npages;
        if (dlg.DoModal() == IDOK) {
            if (dlg.m_direction == 0) {
                if (pPage->m_dervlvl == DERV_NONE && dlg.m_idervlvl == DERV_NONE)
                    return;
                if (dlg.m_idervlvl != DERV_NONE && dlg.m_idervlvl != DERV_PROH && dlg.m_idervlvl != DERV_DRUK)
                    for (int i = 0; i < dlg.m_ilekol; i++)
                        if (!(m_pages[(dlg.m_nr + i) % npages])->m_adds.empty()) {
                            AfxMessageBox(_T("Przed dziedziczeniem nale¿y zdj¹æ og³oszenia ze zmienianych stron"), MB_ICONEXCLAMATION);
                            return;
                        }
            }
            if (dlg.m_drw_xx == this->id_drw) {
                AfxMessageBox(_T("Autodziedziczenie jest niedozwolone"), MB_ICONEXCLAMATION);
                return;
            }
            if (dlg.m_nr == npages) dlg.m_nr = 0;
            if (dlg.m_nr + dlg.m_ilekol - 1 > npages) {
                AfxMessageBox(_T("Sekwencja stron dziedziczonych wychodzi poza makietê"), MB_ICONEXCLAMATION);
                return;
            }
            SetModifiedFlag(FALSE);

            CManODPNETParms orapar {
                { CManODPNET::DbTypeInt32, &this->m_mak_xx },
                { CManODPNET::DbTypeInt32, &dlg.m_nr },
                { CManODPNET::DbTypeInt32, &dlg.m_drw_xx },
                { CManODPNET::DbTypeInt32, &dlg.m_base_nr },
                { CManODPNET::DbTypeInt32, &dlg.m_idervlvl },
                { CManODPNET::DbTypeInt32, &dlg.m_ilekol }
            };

            const char* sql = "begin derv.derive_pages(:dst_mak_xx,:dst_nr_porz,:base_drw_xx,:base_nr_porz,:dervlvl,:ile_kol); end;";
            if (dlg.m_direction == 0) {
                theManODPNET.EI(sql, orapar);
                theApp.FileRefresh(this);
            } else {
                auto derv = &dlg.m_derv_add;
DerivePages_loop:
                for (const int d : *derv) {
                    dlg.m_drw_xx = d;
                    theManODPNET.EI(sql, orapar);
                }
                if (derv != addressof(dlg.m_derv_del)) {
                    dlg.m_idervlvl = 0;
                    derv = &dlg.m_derv_del;
                    goto DerivePages_loop;
                }
                theApp.FileRefresh(nullptr);
            }
        }
    }
} // DerivePages

int CDrawDoc::Nr2NrPorz(const TCHAR *s) const noexcept
{
    if (!s) return -1;
    int nr_porz = _ttoi(s);
    if (nr_porz == 0)
        if ((nr_porz = CDrawObj::Arabska(s)) == 0)
            nr_porz = -1;
        else
            nr_porz = GetIPage((nr_porz << 16) + c_rzym);
    else
        nr_porz = GetIPage((nr_porz << 16) + c_normal);
    return nr_porz;
}

void CDrawDoc::OnChangeColsPerRow()
{
    CPoint pNowhere(INT_MAX >> 2, 0);
    iPagesInRow = MIN_COLSPERROW + 2 * (((iPagesInRow - MIN_COLSPERROW) / 2 + 1) % 5);
    int i, iPC = (int)m_pages.size();
    auto aNoPagePos = (CRect*)theApp.bigBuf;
    // przenieœ opisy tekstowe na bok
    for (i = MIN_COLSPERROW; i < iPC; ++i) {
        aNoPagePos[i] = m_pages[i]->m_position - pNowhere;
        MoveOpisAfterPage(&m_pages[i]->m_position, &aNoPagePos[i]);
    }
    // repozycjonuj strony
    for (i = MIN_COLSPERROW; i < iPC; ++i)
        this->SetPageRectFromOrd(m_pages[i], i);
    // przywróæ opisy tekstowe na makietê
    for (i = MIN_COLSPERROW; i < iPC; ++i)
        MoveOpisAfterPage(&aNoPagePos[i], &m_pages[i]->m_position);

    UpdateAllViews(NULL);
}
