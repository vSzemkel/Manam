
#include "stdafx.h"
#include "manam.h"
#include "koldlg.h"
#include "drawdoc.h"
#include "drawadd.h"
#include "drawopis.h"
#include "spacer.h"
#include "genepsinfodlg.h"
#include "mainfrm.h"
#include <vcclr.h>

#using <System.dll>
#using <System.Data.dll>
#using <System.Xml.dll>
#using <System.Windows.Forms.dll>
#using <Oracle.ManagedDataAccess.dll>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Data;
using namespace System::Windows::Forms;
using namespace System::IO;
using namespace Oracle::ManagedDataAccess::Client;
using namespace Oracle::ManagedDataAccess::Types;

extern CDrawApp NEAR theApp;

gcroot<String^> g_ConnectionString;

CManODPNET theManODPNET;
CManODPNETParms CManODPNET::emptyParm({});

// ODP.NET datatypes for unmanaged code
const int CManODPNET::DbTypeInt32 = static_cast<int>(OracleDbType::Int32);
const int CManODPNET::DbTypeDouble = static_cast<int>(OracleDbType::Double);
const int CManODPNET::DbTypeVarchar2 = static_cast<int>(OracleDbType::Varchar2);
const int CManODPNET::DbTypeRefCursor = static_cast<int>(OracleDbType::RefCursor);

// ODP.NET parameter directions for unmanaged code
const int CManODPNET::ParameterIn = static_cast<int>(ParameterDirection::Input);			// 1
const int CManODPNET::ParameterOut = static_cast<int>(ParameterDirection::Output);			// 2
const int CManODPNET::ParameterInOut = static_cast<int>(ParameterDirection::InputOutput);	// 3
const int CManODPNET::ReturnValue = static_cast<int>(ParameterDirection::ReturnValue);		// 6

struct OdpHelper final
{
    static const gcroot<String^> DbNull;

    // reverse operation is: static String^ PtrToStringUni(IntPtr ptr), but simple assignment will do
    inline static void __clrcall String2WChar(System::String^ s, wchar_t* os, size_t noe)
    {
        pin_ptr<const wchar_t> wch = PtrToStringChars(s);
        wcscpy_s(os, noe, wch);
    }

    inline static void __clrcall String2CString(System::String^ s, CString& cs)
    {
        int noe = s->Length;
        auto buf = cs.GetBufferSetLength(s->Length);
        pin_ptr<const wchar_t> wch = PtrToStringChars(s);
        ::CopyMemory(buf, wch, 2 * noe);
    }

    inline static CString __clrcall ReadOdrString(OracleDataReader^ odr, int index)
    {
        if (odr->IsDBNull(index)) return CString();
        pin_ptr<const wchar_t> wch = PtrToStringChars(odr->GetString(index));
        return CString(wch);
    }

    inline static void __clrcall ShowErrMsgDlg(String^ msg)
    {
        int pos = msg->IndexOf(": ");
        if (pos > 0) msg = msg->Substring(pos + 2);
        pos = msg->IndexOf("\n");
        if (pos > 0) msg = msg->Substring(0, pos);
        MessageBox::Show(nullptr, msg, L"B³¹d", MessageBoxButtons::OK, MessageBoxIcon::Error);
    }

    inline static long long __clrcall IntegerValue(OracleParameter^ op)
    {
        auto d = static_cast<OracleDecimal>(op->Value);
        return d.IsNull ? 0 : static_cast<long long>(d);
    }

    static System::Object^ __clrcall PrepareOdpParam(const CManODPNETParm& p)
    {
        if (p.m_value == nullptr)
            return System::DBNull::Value;

        switch (p.m_odptype) {
            case CManODPNET::DbTypeInt32:
                return *reinterpret_cast<int*>(p.m_value);
            case CManODPNET::DbTypeDouble:
                return *reinterpret_cast<double*>(p.m_value);
            case CManODPNET::DbTypeVarchar2:
                return gcnew String(reinterpret_cast<CString*>(p.m_value)->GetBuffer());
            default:
                return System::DBNull::Value;
        }
    }

    static void __clrcall RetrieveOdpParam(CManODPNETParm *p, OracleParameter^ op)
    {
        switch (p->m_odptype) {
            case CManODPNET::DbTypeInt32:
                *reinterpret_cast<int*>(p->m_value) = op->Value->GetType() == OracleDecimal::typeid
                    ? static_cast<int>(IntegerValue(op))
                    : Convert::ToInt32(op->Value);
                break;
            case CManODPNET::DbTypeDouble:
                *reinterpret_cast<double*>(p->m_value) = op->Value->GetType() == OracleDecimal::typeid
                    ? static_cast<OracleDecimal^>(op->Value)->ToDouble()
                    : static_cast<double>(IntegerValue(op));
                break;
            case CManODPNET::DbTypeVarchar2:
                String^ ret = Convert::ToString(op->Value);
                if (!String::CompareOrdinal(ret, DbNull))
                    ret = String::Empty;
                String2CString(ret, *reinterpret_cast<CString*>(p->m_value));
                break;
        }
    }

    static void __clrcall AddParams(OracleCommand^ cmd, CManODPNETParms& ps)
    {
        for (const auto& p : ps.params) {
            auto dir = static_cast<ParameterDirection>(p.m_direction);
            switch (dir) {
                case ParameterDirection::Output:
                    if (p.m_odptype == CManODPNET::DbTypeVarchar2) {
                        cmd->Parameters->Add(String::Empty, static_cast<OracleDbType>(p.m_odptype), reinterpret_cast<CString*>(p.m_value)->GetLength(), nullptr, dir)->Value = PrepareOdpParam(p);
                        break;
                    } /* else do not break */
                case ParameterDirection::Input:
                case ParameterDirection::InputOutput:
                    cmd->Parameters->Add(String::Empty, static_cast<OracleDbType>(p.m_odptype), nullptr, dir)->Value = PrepareOdpParam(p);
                    break;
                default: /* ParameterDirection::ReturnValue */
                    ps.hasReturnValue = true;
                    break;
            }
        }
    }

    static void __clrcall RetrieveOutParams(OracleCommand^ cmd, CManODPNETParms& ps)
    {
        if (ps.outParamsCount > 0)
            for (int i = 0; i < static_cast<int>(ps.params.size()); ++i)
                if (ps.params[i].m_direction != CManODPNET::ParameterIn) {
                    RetrieveOdpParam(&ps.params[i], cmd->Parameters[i]);
                    if (--ps.outParamsCount == 0) break;
                }
    }

    static void __clrcall ExecuteOpenedCommand(OracleCommand^ cmd, CManODPNETParms& ps)
    {
        AddParams(cmd, ps);

        if (ps.hasReturnValue) {
            auto op = gcnew OracleParameter();
            op->Value = cmd->ExecuteScalar();
            RetrieveOdpParam(&ps.params[0], op);
        } else {
            cmd->ExecuteNonQuery();
            RetrieveOutParams(cmd, ps);
        }
    }

    static BOOL __clrcall InitRozmInternal(OracleCommand^ cmd, std::vector<CRozm>* roz)
    {
        auto odr = cmd->ExecuteReader(CommandBehavior::SingleResult);
        BOOL found = odr->HasRows;
        while (odr->Read()) {
            int i1 = odr->GetInt32(0);
            int i2 = odr->GetInt32(1);
            int i3 = odr->GetInt32(2);
            int i4 = odr->GetInt32(3);
            int i5 = odr->GetInt32(4);
            int i6 = odr->GetInt32(5);
            int i7 = odr->GetInt32(6);
            int i8 = odr->GetInt32(7);
            roz->emplace_back(i1, i2, i3, i4, i5, i6, i7, i8);
        }

        odr->Close();
        return found;
    }
};

const gcroot<String^> OdpHelper::DbNull = "null";

#pragma region CDrawDocDbReader
class CDrawDocDbReader
{
public:
    CDrawDocDbReader(CDrawDoc* doc) : m_doc(doc), m_objetosc(0) {};
    BOOL OpenManamDoc();	// pobiera z bazy podstawowe informacje o makiecie lub grzbiecie
private:
    CDrawDoc* m_doc;
    int m_objetosc;
    BOOL OpenDocContent();	// pobiera obiekty stanowice treœæ dokumentu (strony, og³oszenia, opisy)
    void OpenSpot();
    void OpenOpis(OracleDataReader^ opiCur);
    void OpenQued(OracleDataReader^ queCur);
    void OpenMultiKraty(OracleCommand^ cmd, CFlag* multiKraty);
    BOOL OpenPage(OracleDataReader^ strCur, OracleDataReader^ pubCur, CFlag* multiKraty, CPoint& pos);
};

BOOL CDrawDocDbReader::OpenManamDoc()
{
    String ^openStatus, ^dataZamkniecia;
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    auto par = cmd->Parameters;
    cmd->CommandText = String::Format("begin {0}{1}(:tytul,:mutacja,:kiedy,:isro,:retCur); end;", m_doc->isGRB ? "grb.open_grzbiet" : "open_makieta", m_doc->isLIB ? "_lib" : "");
    par->Add("tytul", OracleDbType::Varchar2, 3)->Value = gcnew String(m_doc->gazeta.Left(3));
    par->Add("mutacja", OracleDbType::Varchar2, 2)->Value = gcnew String(m_doc->gazeta.Mid(4));
    par->Add("kiedy", OracleDbType::Char, 10)->Value = gcnew String(m_doc->data);
    par->Add("isro", OracleDbType::Byte, nullptr, ParameterDirection::InputOutput)->Value = theApp.OPENRO;
    par->Add("retCur", OracleDbType::RefCursor, nullptr, ParameterDirection::Output);
    try {
        conn->Open();
        auto odr = cmd->ExecuteReader(CommandBehavior::SingleRow);
        if (!odr->Read()) return FALSE;
        m_doc->m_mak_xx = odr->GetInt32(0);
        m_doc->id_drw = odr->GetInt32(1);
        m_objetosc = odr->GetInt32(2);
        openStatus = odr->IsDBNull(3) ? String::Empty : odr->GetString(3);
        dataZamkniecia = odr->IsDBNull(4) ? String::Empty : odr->GetString(4);
        m_doc->opis = OdpHelper::ReadOdrString(odr, m_doc->iDocType == DocType::makieta ? 5 : 4);
        odr->Close();
    } catch (OracleException^ oex) {
        OdpHelper::ShowErrMsgDlg(oex->Message);
        m_doc->m_mak_xx = 0;
        return FALSE;
    } finally {
        conn->Close();
    }

    m_doc->m_pages.resize(m_objetosc);
    auto ro = static_cast<int8_t>(OdpHelper::IntegerValue(par[3]));
    m_doc->isRO = ((m_doc->isGRB && ro == -1) ? 0 : ro);
    if (!m_doc->isRO && !String::IsNullOrEmpty(openStatus)) {
        if (DialogResult::Cancel == MessageBox::Show(nullptr, openStatus + L".\nCzy chcesz otworzyæ j¹ w trybie tylko do odczytu?", L"Manam", MessageBoxButtons::OKCancel, MessageBoxIcon::Information)) {
            ((CMainFrame*)AfxGetMainWnd())->SetStatusBarInfo((LPCTSTR)_T("Spróbuj wys³aæ wiadomoœæ (F10) lub zadzwoniæ do u¿ytkownika, który blokuje makietê . . ."));
            m_doc->m_mak_xx = 0;
            return FALSE;
        }
        m_doc->isRO = TRUE;
    }

    if (m_doc->isGRB && ro == -1) m_doc->m_mak_xx *= -1;

    if (m_doc->iDocType == DocType::makieta) { // data_zm
        int d, m, r, g, min;
        OdpHelper::String2WChar(dataZamkniecia, (wchar_t*)theApp.bigBuf, (dataZamkniecia->Length + 1) * 2);
        if (swscanf_s((wchar_t*)theApp.bigBuf, c_formatCzasu, &d, &m, &r, &g, &min) == 5)
            m_doc->dataZamkniecia = CTime(r, m, d, g, min, 0);
    }

    if (!m_doc->isLIB) {
        par->Clear();
        par->Add("mak_xx", OracleDbType::Int32)->Value = m_doc->m_mak_xx;
        cmd->CommandText = String::Format("select p1,kto_prowadzi,opiekun,wydawca,is_sig from {0}info where xx=abs(:mak_xx)", m_doc->isGRB ? "grb" : "mak");
        try {
            conn->Open();
            auto odr = cmd->ExecuteReader(CommandBehavior::SingleRow);
            if (odr->Read()) {
                m_doc->prowadzacy1 = OdpHelper::ReadOdrString(odr, 0);
                m_doc->prowadzacy2 = OdpHelper::ReadOdrString(odr, 1);
                m_doc->sekretarz = OdpHelper::ReadOdrString(odr, 2);
                m_doc->symWydawcy = OdpHelper::ReadOdrString(odr, 3);
                m_doc->isSIG = odr->GetByte(4);
            }
            odr->Close();
        } catch (OracleException^ oex) {
            OdpHelper::ShowErrMsgDlg(oex->Message);
            return FALSE;
        } finally {
            conn->Close();
        }
    }

    CRect rct(0, 0, 0, 0);
    for (int i = 0; i < m_objetosc; i++) {
        auto pNewPage = new CDrawPage(&rct);
        pNewPage->m_pDocument = m_doc;
        m_doc->m_pages[i] = pNewPage;
        m_doc->SetPageRectFromOrd(pNewPage, i);
    }

    if (m_doc->gazeta.GetAt(0) == _T('Z')) { // spoty ju¿ tylko dla zewnêtrznych
        m_doc->ZmianaSpotow(m_objetosc);
        if (!m_doc->isGRB) OpenSpot();
    }

    return OpenDocContent();
}

void CDrawDocDbReader::OpenSpot()
{
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->CommandText = String::Format("select nvl(spo_xx,-1) from spot_makiety{0} where mak_xx=:mak_xx order by spot", m_doc->isLIB ? gcnew String("_lib") : String::Empty);
    cmd->Parameters->Add("mak_xx", OracleDbType::Int32)->Value = m_doc->m_mak_xx;
    try {
        conn->Open();
        auto odr = cmd->ExecuteReader(CommandBehavior::SingleResult);
        while (odr->Read())
            m_doc->m_spot_makiety.push_back(((UINT)((CMainFrame*)AfxGetMainWnd())->GetIdxfromSpotID(odr->GetInt32(0))));
        odr->Close();
    } catch (OracleException^ oex) {
        OdpHelper::ShowErrMsgDlg(oex->Message);
    } finally {
        conn->Close();
    }
}

void CDrawDocDbReader::OpenOpis(OracleDataReader^ opiCur)
{
    int l, r, t, b;
    while (opiCur->Read()) {
        t = opiCur->GetInt32(1);
        l = opiCur->GetInt32(2);
        b = opiCur->GetInt32(3);
        r = opiCur->GetInt32(4);
        CRect rc(l*vscale, t*vscale, r*vscale, b*vscale);
        auto pOpis = new CDrawOpis(&rc, OdpHelper::ReadOdrString(opiCur, 5));
        pOpis->m_opi_xx = opiCur->GetInt32(0);
        pOpis->m_Scale = opiCur->GetFloat(6);
        pOpis->kolor = opiCur->GetInt32(7);
        pOpis->dirty = FALSE;
        m_doc->Add(pOpis);
    }
}

void CDrawDocDbReader::OpenQued(OracleDataReader^ queCur)
{
    while (queCur->Read()) {
        CRect r(0, 0, 1, 1);
        auto pAdd = new CDrawAdd(&r);
        m_doc->AddQue(pAdd);
        pAdd->flags.reserv = 0;
        pAdd->m_pub_xx = queCur->GetInt32(0); // xx
        pAdd->m_add_xx = queCur->GetInt32(1); // add_xx
        pAdd->typ_xx = queCur->GetInt32(2); // add_xx
        pAdd->szpalt_x = queCur->GetInt32(3); // szpalt_x
        pAdd->szpalt_y = queCur->GetInt32(4); // szpalt_y
        pAdd->SetSpaceSize(queCur->GetInt32(5), queCur->GetInt32(6)); // sizex, sizey
        pAdd->nreps = queCur->GetInt32(13); // adno
        pAdd->nazwa = OdpHelper::ReadOdrString(queCur, 7); // sciezka
        pAdd->remarks = OdpHelper::ReadOdrString(queCur, 8); // uwagi
        pAdd->wersja = OdpHelper::ReadOdrString(queCur, 9); // wersja
        pAdd->czaskto = OdpHelper::ReadOdrString(queCur, 12); // czaskto
        pAdd->kolor = ((UINT)((CMainFrame*)AfxGetMainWnd())->GetIdxfromSpotID(queCur->GetInt32(11))); // spo_xx
        pAdd->kolor = (UINT)(((pAdd->kolor) << 3) + queCur->GetInt32(10));
        pAdd->SetClean();
        pAdd->UpdateInfo();
    }
}

BOOL CDrawDocDbReader::OpenPage(OracleDataReader^ strCur, OracleDataReader^ pubCur, CFlag* multiKraty, CPoint& pos)
{
    BOOL ogloszenieNaNastepnaStrone = FALSE;
    CString op_zew, sekcja, op_sek, pl, op_pl, poz_na_str;
    int ind, ivar, ivx, ivy, szpalt_x, szpalt_y, s_x, s_y, d, m, r, g, min;
    m_doc->isRED = ((CDrawApp*)AfxGetApp())->GetProfileInt(_T("General"), _T("Captions"), 1) != 0;
    while (strCur->Read()) {
        ind = strCur->GetInt32(1);
        if (ind >= m_objetosc) return FALSE;
        auto pPage = m_doc->m_pages[ind];
        pPage->id_str = strCur->GetInt32(0);
        pPage->szpalt_x = s_x = strCur->GetInt32(9);
        pPage->szpalt_y = s_y = strCur->GetInt32(10);
        ivar = strCur->GetInt32(2); // nr
        if (ivar < 0) pPage->niemakietuj = TRUE;
        pPage->nr = (abs(ivar)) << 3;
        ivar = strCur->GetInt32(3); // num_xx
        if (ivar < 0) {
            ivar *= -1;
            multiKraty->SetBit(ind);
        }
        pPage->nr += ivar;
        ivar = strCur->GetInt32(4); // ile_kol
        pPage->kolor = ivar;
        if (pPage->kolor == c_spot) {
            ivar = strCur->GetInt32(5); // spot
            pPage->kolor += ((ivar - 1) << 3);
        }
        pPage->name = OdpHelper::ReadOdrString(strCur, 6);
        ivar = m_doc->isRED ? 8 : 7;
        pPage->caption = OdpHelper::ReadOdrString(strCur, ivar);
        ivar = 15 - ivar; // 7 => 8; 8 => 7
        pPage->caption_alt = OdpHelper::ReadOdrString(strCur, ivar); // naglowek : strlog
        pin_ptr<const wchar_t> p = PtrToStringChars(strCur->GetString(11));
        pPage->space = CFlag(p); // space
        p = PtrToStringChars(strCur->GetString(12));
        pPage->space_red = CFlag(p); // space_red
        p = PtrToStringChars(strCur->GetString(13));
        pPage->space_locked = CFlag(p); // space_locked
        pPage->space |= (pPage->space_locked | pPage->space_red);
        pPage->prn_mak_xx = strCur->IsDBNull(14) ? 0 : strCur->GetInt32(14); // pagina
        pPage->m_drukarnie = strCur->IsDBNull(15) ? 0 : strCur->GetInt32(15); // drukarnie
        p = PtrToStringChars(strCur->GetString(16));
        if (swscanf_s(p, c_formatCzasu, &d, &m, &r, &g, &min) == 5 && r > 1900)
            pPage->m_deadline = CTime(r, m, d, g, min, 0);
        pPage->m_dervlvl = strCur->IsDBNull(17) ? 0 : strCur->GetInt32(17); // dervlvl
        pPage->m_dervinfo = OdpHelper::ReadOdrString(strCur, 18);
        pPage->mutred = OdpHelper::ReadOdrString(strCur, 19);
        // 20 - pap_xx, obsolete
        pPage->m_typ_pary = strCur->GetByte(21); // rozkladowka
        pPage->wyd_xx = static_cast<UINT>(strCur->GetInt32(22));
        if (m_doc->isGRB) {
            pPage->m_mutczas = strCur->GetInt32(23); // mutczas
            StringCchPrintf(theApp.bigBuf, bigSize, _T("%s (mutacja %i)"), pPage->m_dervinfo, pPage->m_mutczas);
            pPage->m_dervinfo = theApp.bigBuf;
        }

        if (!m_doc->isLIB) // w bibliotecznych nie ma og³oszeñ
            while (ogloszenieNaNastepnaStrone || pubCur->Read()) {
                ivar = pubCur->GetInt32(3); // str_xx
                if (ogloszenieNaNastepnaStrone = ivar != pPage->id_str)
                    break;

                ivx = pubCur->GetInt32(10); // sizex
                ivy = pubCur->GetInt32(11); // sizey
                szpalt_x = pubCur->GetInt32(4); // szpalt_x
                szpalt_y = pubCur->GetInt32(5); // szpalt_y
                CRect rt(pos, CSize((int)(ivx*modulx), (int)(ivy*(-1)*moduly)));
                auto pAdd = new CDrawAdd(&rt);
                pAdd->m_pub_xx = pubCur->GetInt32(0); // xx
                pAdd->m_add_xx = pubCur->GetInt32(1); // add_xx
                pAdd->posx = pubCur->GetInt32(6); // posx
                pAdd->posy = pubCur->GetInt32(7); // posy
                pAdd->szpalt_x = szpalt_x;
                pAdd->szpalt_y = szpalt_y;
                pAdd->SetSpaceSize(ivx, ivy);
                pAdd->flags.locked = pubCur->GetByte(8); // blokada
                pAdd->flags.reserv = pubCur->GetByte(9); // flaga_rezerw
                pAdd->nazwa = OdpHelper::ReadOdrString(pubCur, 12); // nazwa
                pAdd->nreps = pubCur->GetInt32(13); // adno
                pAdd->wersja = OdpHelper::ReadOdrString(pubCur, 14);
                pAdd->remarks = OdpHelper::ReadOdrString(pubCur, 15);
                pAdd->remarks_atex = OdpHelper::ReadOdrString(pubCur, 36);
                ivar = pubCur->GetInt32(17); // spo_xx
                pAdd->kolor = 8 * ((UINT)((CMainFrame*)AfxGetMainWnd())->GetIdxfromSpotID(ivar));
                pAdd->kolor += pubCur->GetInt32(16); // ile_kol
                pAdd->txtposx = pubCur->GetInt32(26); // txtposx
                pAdd->txtposy = pubCur->GetInt32(27); // txtposy
                pAdd->typ_xx = pubCur->GetInt32(28); // typ_xx
                pAdd->czaskto = OdpHelper::ReadOdrString(pubCur, 29); // czaskto
                ivar = pubCur->GetInt32(30); // isok
                pAdd->flags.isok = ivar < 0 ? pAdd->flags.zagroz = 1, -ivar : ivar;
                ivar = pubCur->GetInt32(31); // eps_present
                pAdd->flags.showeps = (pAdd->flags.epsok = ivar) & 1;
                ivar = pubCur->GetInt32(32); // powtorka
                pAdd->powtorka = ivar ? POWTSEED_1 + ivar * ONEDAY : 0;
                pAdd->oldAdno = pubCur->GetInt32(33); // powtórka z numeru
                pAdd->flags.studio = pubCur->GetByte(34); // studio
                pAdd->flags.derived = pubCur->GetByte(35); // derived
                pAdd->spad_flag = pubCur->GetByte(37); // krawedzie na spad
                pAdd->flags.reksbtl = pubCur->GetByte(41); // podpis REKLAMA
                pAdd->flags.digital = pubCur->GetByte(42); // czy cyfrowe
                ivx = pubCur->GetInt32(21); // nr_w_sekcji
                ivy = pubCur->GetInt32(24); // nr_pl
                op_zew = OdpHelper::ReadOdrString(pubCur, 18); // op_zew
                sekcja = OdpHelper::ReadOdrString(pubCur, 19); // sekcja
                op_sek = OdpHelper::ReadOdrString(pubCur, 20); // op_sek
                pl = OdpHelper::ReadOdrString(pubCur, 22); // pl
                op_pl = OdpHelper::ReadOdrString(pubCur, 23); // op_pl
                poz_na_str = OdpHelper::ReadOdrString(pubCur, 25); // poz_na_str
                pAdd->SetLogpage(op_zew, sekcja, op_sek, ivx, pl, op_pl, ivy, poz_na_str);
                if (!pubCur->IsDBNull(38)) { // precel
                    p = PtrToStringChars(pubCur->GetString(38));
                    pAdd->InitPrecel(CString(p));
                }
                pAdd->kodModulu = OdpHelper::ReadOdrString(pubCur, 39); // nazwa pliku
                pAdd->nag_xx = pubCur->GetInt32(40); // nag_xx
                m_doc->Add(pAdd);
                if (pAdd->posx > 0) {
                    pPage->AddAdd(pAdd);
                    CRect vRect(pPage->m_position.left + (int)((pAdd->posx - 1)*modulx), pPage->m_position.bottom + (int)((szpalt_y - pAdd->posy + 1)*moduly),
                        pPage->m_position.left + (int)((pAdd->posx + pAdd->sizex - 1)*modulx), pPage->m_position.bottom + (int)((szpalt_y - pAdd->posy - pAdd->sizey + 1)*moduly));
                    pAdd->m_position = vRect;
                } else
                    m_doc->AdvanceAsidePos(pos);

                pAdd->SetClean();
                pAdd->UpdateInfo();
                pAdd->dirty = FALSE;
            }
        pPage->SetBaseKrata(s_x, s_y, FALSE);
        pPage->dirty = FALSE;

        pPage->UpdateInfo();
    }

    return TRUE;
}

void CDrawDocDbReader::OpenMultiKraty(OracleCommand^ cmd, CFlag* multiKraty)
{
    // :mak_xx
    auto par = cmd->Parameters;
    par->Insert(1, gcnew OracleParameter("str_xx", OracleDbType::Int32));
    while (par->Count > 3) par->RemoveAt(3);
    cmd->CommandText = String::Format("begin {0}open_kra_str{1}(:mak_xx,:str_xx,:retCur); end;", m_doc->isGRB ? "grb." : String::Empty, m_doc->isLIB ? "_lib" : String::Empty);

    for (int i = 0; i < (int)m_objetosc; ++i) {
        if (!multiKraty->GetBit(i)) continue;
        auto pPage = m_doc->m_pages[i];
        int s_x = pPage->szpalt_x;
        int s_y = pPage->szpalt_y;

        par[1]->Value = pPage->id_str;
        auto odr = cmd->ExecuteReader(CommandBehavior::SingleResult);
        while (odr->Read()) {
            int ivx = odr->GetInt32(0); // szpalt_x
            int ivy = odr->GetInt32(1); // szpalt_y
            pPage->SetBaseKrata(ivx, ivy, FALSE);
            pin_ptr<const wchar_t> p = PtrToStringChars(odr->GetString(2));
            pPage->space = CFlag(p);
            p = PtrToStringChars(odr->GetString(3));
            pPage->space_red = CFlag(p);
            p = PtrToStringChars(odr->GetString(4));
            pPage->space_locked = CFlag(p);
            pPage->space |= (pPage->space_locked | pPage->space_red);
        }
        odr->Close();
        pPage->SetBaseKrata(s_x, s_y, FALSE);
    }
}

BOOL CDrawDocDbReader::OpenDocContent()
{
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    auto par = cmd->Parameters;
    cmd->CommandText = String::Format("begin {0}open_spo{1}; end;", m_doc->isGRB ? "grb." : String::Empty, m_doc->isLIB ? "_lib(:mak_xx,:retCur,:retCur2,:retCur3)" : "q(:mak_xx,:retCur,:retCur2,:retCur3,:retCur4)");
    par->Add("mak_xx", OracleDbType::Int32)->Value = m_doc->m_mak_xx;
    par->Add("refCur", OracleDbType::RefCursor)->Direction = ParameterDirection::Output;
    par->Add("refCur2", OracleDbType::RefCursor)->Direction = ParameterDirection::Output;
    par->Add("refCur3", OracleDbType::RefCursor)->Direction = ParameterDirection::Output;
    if (!m_doc->isLIB)
        par->Add("refCur4", OracleDbType::RefCursor)->Direction = ParameterDirection::Output;
    OracleDataReader ^strCur, ^pubCur, ^opiCur, ^queCur;
    CFlag multiKraty((int)ceil(m_doc->m_pages.size() / 8.0));
    try {
        conn->Open();
        cmd->ExecuteNonQuery();
        strCur = static_cast<OracleRefCursor^>(par[1]->Value)->GetDataReader();
        pubCur = static_cast<OracleRefCursor^>(par[2]->Value)->GetDataReader();
        opiCur = static_cast<OracleRefCursor^>(par[3]->Value)->GetDataReader();

        if (!OpenPage(strCur, pubCur, &multiKraty, m_doc->GetAsideAddPos(TRUE)))
            return FALSE;

        OpenOpis(opiCur);

        if (!m_doc->isLIB) {
            queCur = static_cast<OracleRefCursor^>(par[4]->Value)->GetDataReader();
            OpenQued(queCur);
        }

        if (multiKraty.IsSet())
            OpenMultiKraty(cmd, &multiKraty);
    } catch (OracleException^ oex) {
        OdpHelper::ShowErrMsgDlg(oex->Message);
        return FALSE;
    } finally {
        if (strCur != nullptr) strCur->Close();
        if (pubCur != nullptr) pubCur->Close();
        if (opiCur != nullptr) opiCur->Close();
        if (queCur != nullptr) queCur->Close();
        conn->Close();
    }

    return TRUE;
}
#pragma endregion

#pragma region CDrawDocDbWriter
class CDrawDocDbWriter
{
public:
    CDrawDocDbWriter(CDrawDoc* doc);
    BOOL SaveManamDoc(BOOL isSaveAs, BOOL doSaveAdds);
private:
    CDrawDoc* m_doc;
    int m_objetosc;
    void RemoveDeletedObj(OracleConnection^ conn, BOOL delAdds);
    void SaveOpisyMakiety(OracleConnection^ conn, BOOL isSaveAs);
    void SaveSpotyMakiety(OracleConnection^ conn);
    void SaveStrony(OracleConnection^ conn, BOOL isSaveAs, BOOL doSaveAdds);
    void SaveOgloszenie(OracleCommand^ conn, CDrawAdd* pAdd);
};

CDrawDocDbWriter::CDrawDocDbWriter(CDrawDoc* doc) : m_doc(doc)
{
    m_objetosc = (int)doc->m_pages.size();
}

BOOL CDrawDocDbWriter::SaveManamDoc(BOOL isSaveAs, BOOL doSaveAdds)
{
    OracleTransaction^ otr = nullptr;
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    auto par = cmd->Parameters;
    cmd->CommandText = String::Format("begin {0}_makieta{1}(:drw_xx,:kiedy,:obj,:mak_xx); end;", isSaveAs ? "insert" : "update", m_doc->isLIB ? "_lib" : "");
    par->Add("drw_xx", OracleDbType::Int32)->Value = m_doc->id_drw;
    par->Add("kiedy", OracleDbType::Varchar2)->Value = gcnew String(m_doc->data);
    par->Add("obj", OracleDbType::Int32)->Value = m_objetosc;
    auto m = par->Add("mak_xx", OracleDbType::Int32, ParameterDirection::InputOutput);
    m->Value = m_doc->m_mak_xx;

    try {
        conn->Open();
        otr = conn->BeginTransaction();
        cmd->ExecuteNonQuery();
        m_doc->m_mak_xx = (int)static_cast<OracleDecimal>(m->Value);

        if (!isSaveAs && !m_doc->m_del_obj.empty()) RemoveDeletedObj(conn, TRUE);

        SaveStrony(conn, isSaveAs, doSaveAdds);

        if (!isSaveAs && !m_doc->m_del_obj.empty()) RemoveDeletedObj(conn, FALSE);

        SaveOpisyMakiety(conn, isSaveAs);

        SaveSpotyMakiety(conn);

        if (isSaveAs) {
            par->Clear();
            par->Add("mak_xx", OracleDbType::Int32)->Value = m_doc->m_mak_xx;
            cmd->CommandText = String::Format("begin update_drukarnie{0}(:mak_xx); end;", m_doc->isLIB ? "_lib" : "");
            cmd->ExecuteNonQuery();
        }

        otr->Commit();

        m_doc->m_del_obj.clear();
    } catch (OracleException^ oex) {
        if (otr != nullptr)
            otr->Rollback();
        OdpHelper::ShowErrMsgDlg(oex->Message);
        return FALSE;
    } finally {
        conn->Close();
    }

    return TRUE;
}

void CDrawDocDbWriter::RemoveDeletedObj(OracleConnection^ conn, BOOL delAdds)
{	// uruchamiane dwa razy, najpierw delAdd=TRUE, potem FALSE
    auto cmd = conn->CreateCommand();
    cmd->CommandText = "begin delete_spacer_object(:mak_xx,:typ,:xx); end;";
    auto par = cmd->Parameters;
    par->Add("mak_xx", OracleDbType::Int32)->Value = m_doc->m_mak_xx;
    par->Add("typ", OracleDbType::Int16);
    par->Add("obj", OracleDbType::Int32);

    for (const auto& del : m_doc->m_del_obj) {
        if ((delAdds && del.second != c_add) || (!delAdds && del.second == c_add)) continue;
        par[1]->Value = del.second;
        par[2]->Value = del.first;
        cmd->ExecuteNonQuery();
    }
}

void CDrawDocDbWriter::SaveOpisyMakiety(OracleConnection^ conn, BOOL isSaveAs)
{
    auto cmd = conn->CreateCommand();
    auto par = cmd->Parameters;
    auto op = par->Add("opi_xx", OracleDbType::Int32, ParameterDirection::InputOutput);
    par->Add("mak_xx", OracleDbType::Int32)->Value = m_doc->m_mak_xx;
    par->Add("top", OracleDbType::Int16);
    par->Add("left", OracleDbType::Int16);
    par->Add("bottom", OracleDbType::Int16);
    par->Add("right", OracleDbType::Int16);
    par->Add("text", OracleDbType::Varchar2);
    par->Add("skala", OracleDbType::Double);
    par->Add("kolor", OracleDbType::Int16);

    for (const auto& pObj : m_doc->m_objects) {
        auto pOpis = dynamic_cast<CDrawOpis*>(pObj);
        if (pOpis && pOpis->dirty) {
            bool isInsert = isSaveAs || pOpis->m_opi_xx == -1;
            cmd->CommandText = String::Format("begin {0}_spacer_opis{1}(:opi_xx,:mak_xx,:top,:left,:bottom,:right,:tekst,:skala,:kolor); end;", isInsert ? "insert" : "update", m_doc->isLIB ? "_lib" : "");
            op->Value = pOpis->m_opi_xx;
            par[2]->Value = pOpis->m_position.top / vscale;
            par[3]->Value = pOpis->m_position.left / vscale;
            par[4]->Value = pOpis->m_position.bottom / vscale;
            par[5]->Value = pOpis->m_position.right / vscale;
            par[6]->Value = gcnew String(pOpis->info);
            par[7]->Value = pOpis->m_Scale;
            par[8]->Value = pOpis->kolor;

            cmd->ExecuteNonQuery();

            pOpis->SetClean();
            if (isInsert)
                pOpis->m_opi_xx = (int)static_cast<OracleDecimal>(op->Value);
        }
    }
}

void CDrawDocDbWriter::SaveSpotyMakiety(OracleConnection^ conn)
{
    auto spot_makiety = m_doc->m_spot_makiety;
    auto cnt = spot_makiety.size();
    if (cnt == 0) return;

    auto cmd = conn->CreateCommand();
    cmd->CommandText = String::Format("begin save_spot{0}(:mak_xx,:nr,:spot); end;", m_doc->isLIB ? "_lib" : "");
    auto par = cmd->Parameters;
    par->Add("mak_xx", OracleDbType::Int32)->Value = m_doc->m_mak_xx;
    par->Add("nr", OracleDbType::Int32);
    par->Add("spot", OracleDbType::Int32);

    for (size_t i = 0; i < cnt; ++i) // update spotow;
        if (spot_makiety[i] != 0) {
            par[1]->Value = i + 1;
            par[2]->Value = ((CMainFrame*)AfxGetMainWnd())->Spot_ID[(spot_makiety[i])];
            cmd->ExecuteNonQuery();
        }
}

void CDrawDocDbWriter::SaveStrony(OracleConnection^ conn, BOOL isSaveAs, BOOL doSaveAdds)
{
    // parametry do zapisu og³oszeñ
    auto cmdAdd = conn->CreateCommand();
    auto par = cmdAdd->Parameters;
    par->Add("pub_xx", OracleDbType::Int32, ParameterDirection::InputOutput);
    par->Add("add_xx", OracleDbType::Int32);
    par->Add("mak_xx", OracleDbType::Int32)->Value = m_doc->m_mak_xx;
    par->Add("str_xx", OracleDbType::Int32);
    par->Add("szpalt_x", OracleDbType::Int32);
    par->Add("szpalt_y", OracleDbType::Int32);
    par->Add("x", OracleDbType::Int32);
    par->Add("y", OracleDbType::Int32);
    par->Add("blokada", OracleDbType::Int32);
    par->Add("sizex", OracleDbType::Int32);
    par->Add("sizey", OracleDbType::Int32);
    par->Add("nazwa", OracleDbType::Varchar2);
    par->Add("adno", OracleDbType::Int32);
    par->Add("wersja", OracleDbType::Varchar2);
    par->Add("uwagi", OracleDbType::Varchar2);
    par->Add("ile_kolorow", OracleDbType::Int32);
    par->Add("nr_spotu", OracleDbType::Int32);
    par->Add("op_zew", OracleDbType::Varchar2);
    par->Add("sekcja", OracleDbType::Varchar2);
    par->Add("op_sekcji", OracleDbType::Varchar2);
    par->Add("nr_w_sekcji", OracleDbType::Int32);
    par->Add("pl", OracleDbType::Varchar2);
    par->Add("op_pl", OracleDbType::Varchar2);
    par->Add("nr_pl", OracleDbType::Int32);
    par->Add("poz_na_str", OracleDbType::Varchar2);
    par->Add("txtposx", OracleDbType::Int32);
    par->Add("txtposy", OracleDbType::Int32);
    par->Add("typ_xx", OracleDbType::Int32);
    par->Add("czaskto", OracleDbType::Varchar2);
    par->Add("powtorka", OracleDbType::Int32);
    par->Add("old_adno", OracleDbType::Int32);
    par->Add("studio", OracleDbType::Int32);
    par->Add("uwagi_atex", OracleDbType::Varchar2);
    par->Add("eps_present", OracleDbType::Int32);
    par->Add("spad", OracleDbType::Int32);
    par->Add("nag_xx", OracleDbType::Int32);
    par->Add("is_digital", OracleDbType::Int32);

    String^ pageSql = "begin {0}(:mak_xx,:str_xx,:szpalt_x,:szpalt_y,:nr_porz,:nr,:typ_num,:ile_kolorow,:nr_spotu,:sciezka,:str_log,:O,:B,:R,:drukarnie,:deadline,:mutred,:pap_xx,:is_rozk,:wyd_xx); end;";
    auto cmdPage = conn->CreateCommand();
    par = cmdPage->Parameters;
    par->Add("mak_xx", OracleDbType::Int32)->Value = m_doc->m_mak_xx;
    par->Add("str_xx", OracleDbType::Int32, ParameterDirection::InputOutput);
    par->Add("szpalt_x", OracleDbType::Int32);
    par->Add("szpalt_y", OracleDbType::Int32);
    par->Add("nr_porz", OracleDbType::Int32);
    par->Add("nr", OracleDbType::Int32);
    par->Add("typ_num", OracleDbType::Int32);
    par->Add("ile_kolorow", OracleDbType::Int32);
    par->Add("nr_spotu", OracleDbType::Int32);
    par->Add("sciezka", OracleDbType::Varchar2);
    par->Add("str_log", OracleDbType::Varchar2);
    par->Add("O", OracleDbType::Varchar2);
    par->Add("B", OracleDbType::Varchar2);
    par->Add("R", OracleDbType::Varchar2);
    par->Add("drukarnie", OracleDbType::Int32);
    par->Add("deadline", OracleDbType::Varchar2);
    par->Add("mutred", OracleDbType::Varchar2);
    par->Add("pap_xx", OracleDbType::Int32);
    par->Add("is_rozk", OracleDbType::Int32);
    par->Add("wyd_xx", OracleDbType::Int32);

    size_t i;
    for (i = 0; i < m_doc->m_pages.size(); i++) {
        String^ plsqlProc;

        auto pPage = m_doc->m_pages[i];
        ((CMainFrame*)AfxGetMainWnd())->SetStatusBarInfo((LPCTSTR)_T("Zachowanie strony : ") + pPage->info);

        if (!doSaveAdds) {
            for (const auto& pAdd : pPage->m_adds)
                pPage->RealizeSpace(pAdd);
            pPage->m_adds.clear();
        }

        if (!pPage->dirty) goto save_adds;

        if (isSaveAs || (pPage->id_str == -1))
            plsqlProc = "insert_strona";
        else if (AfxGetApp()->GetProfileInt(_T("General"), _T("Captions"), 1) == 0 || m_doc->isLIB)
            plsqlProc = "update_strona";
        else
            plsqlProc = "update_strona_cap";

        if (m_doc->isLIB) plsqlProc += "_lib";

        cmdPage->CommandText = String::Format(pageSql, plsqlProc);

        { // fake block to validate goto save_adds stmt.
            auto kraty = pPage->CleanKraty(TRUE);
            if (!kraty.empty()) {
                auto cmdKrt = conn->CreateCommand();
                cmdKrt->CommandText = "begin delete_str_krata(:mak_xx,:str_xx,:szpalt_x,:szpalt_y); end;";
                auto par2 = cmdKrt->Parameters;
                par2->Add("mak_xx", OracleDbType::Int32)->Value = m_doc->m_mak_xx;
                par2->Add("str_xx", OracleDbType::Int32)->Value = pPage->id_str;
                par2->Add("szpalt_x", OracleDbType::Int32);
                par2->Add("szpalt_y", OracleDbType::Int32);
                for (const int& k : kraty) {
                    par2[2]->Value = k >> 16;
                    par2[3]->Value = k & 0xFFFF;
                    cmdKrt->ExecuteNonQuery();
                }
            }
        }

        par[1]->Value = pPage->id_str;
        par[2]->Value = pPage->szpalt_x;
        par[3]->Value = pPage->szpalt_y;
        par[4]->Value = i;
        par[5]->Value = (pPage->nr >> 3) * (pPage->niemakietuj ? -1 : 1);
        par[6]->Value = pPage->nr & 0x07;
        par[7]->Value = pPage->kolor & 0x07;
        par[8]->Value = (int)(pPage->kolor&c_spot ? 1 + (pPage->kolor >> 3) : 0);
        par[9]->Value = gcnew String(pPage->name);
        par[10]->Value = gcnew String(pPage->caption);
        par[11]->Value = gcnew String(pPage->space.ToRaw());
        par[12]->Value = gcnew String(pPage->space_locked.ToRaw());
        par[13]->Value = gcnew String(pPage->space_red.ToRaw());
        par[14]->Value = pPage->m_drukarnie;
        par[15]->Value = gcnew String(pPage->m_deadline.Format(c_ctimeCzas));
        par[16]->Value = gcnew String(pPage->mutred);
        par[17]->Value = nullptr; // obsolete
        par[18]->Value = pPage->m_typ_pary;
        par[19]->Value = pPage->wyd_xx;

        cmdPage->ExecuteNonQuery();
        pPage->id_str = (int)static_cast<OracleDecimal>(par[1]->Value);
        pPage->sBoundingBox.Empty();

        for (const auto& kn : pPage->m_kraty_niebazowe) {
            auto cmdKrat = conn->CreateCommand();
            cmdKrat->CommandText = String::Format("begin save_str_krata{0}(:mak_xx,:str_xx,:szpalt_x,:szpalt_y,:O,:B,:R); end;", m_doc->isLIB ? "_lib" : "");
            auto par2 = cmdKrat->Parameters;
            par2->Add("mak_xx", OracleDbType::Int32)->Value = m_doc->m_mak_xx;
            par2->Add("str_xx", OracleDbType::Int32)->Value = pPage->id_str;
            par2->Add("szpalt_x", OracleDbType::Int32)->Value = kn.m_szpalt_x;
            par2->Add("szpalt_y", OracleDbType::Int32)->Value = kn.m_szpalt_y;
            par2->Add("O", OracleDbType::Varchar2)->Value = gcnew String(kn.m_space.ToRaw());
            par2->Add("B", OracleDbType::Varchar2)->Value = gcnew String(kn.m_space_locked.ToRaw());
            par2->Add("R", OracleDbType::Varchar2)->Value = gcnew String(kn.m_space_red.ToRaw());

            cmdKrat->ExecuteNonQuery();
        }

        pPage->SetClean();
save_adds:
        // zapisujemy og³oszenia
        for (const auto& pAdd : pPage->m_adds) {
            cmdAdd->Parameters[3]->Value = pPage->id_str;
            SaveOgloszenie(cmdAdd, pAdd);
        }
    }

    // zachowaj og³oszenia, które nie stoj¹ na ¿adnej stronie
    if (!m_doc->isLIB && i > 0) {
        cmdAdd->Parameters[3]->Value = m_doc->m_pages[0]->id_str;
        for (const auto& pObj : m_doc->m_objects) {
            auto pAdd = dynamic_cast<CDrawAdd*>(pObj);
            if (pAdd && !pAdd->fizpage) {
                pAdd->posx = 0;
                pAdd->posy = 0;
                SaveOgloszenie(cmdAdd, pAdd);
            }
        };
    }
}

void CDrawDocDbWriter::SaveOgloszenie(OracleCommand^ cmd, CDrawAdd* pAdd)
{	//:mak_xx,:str_xx ustalone
    if (!pAdd->dirty) return;

    String^ oper = "update";
    BOOL bInsert = pAdd->m_pub_xx == -1;
    if (bInsert) {
        pAdd->m_pub_xx = 1;
        oper = "insert";
    }

    cmd->CommandText = String::Format("begin {0}_spacer_pub{1}(:pub_xx,:add_xx,:mak_xx,:str_xx,:szpalt_x,:szpalt_y,:x,:y,:blokada,:sizex,:sizey,:nazwa,:adno,:wersja,:uwagi,:ile_kolorow,:nr_spotu,:op_zew,:sekcja,:op_sekcji,:nr_w_sekcji,:pl,:op_pl,:nr_pl,:poz_na_str,:txtposx,:txtposy,:typ_xx,:czaskto,:powtorka,:old_adno,:studio,:uwagi_atex,:eps_present,:spad,:nag_xx,:is_digital); end;", oper, pAdd->flags.derived ? "stub" : "");
    auto par = cmd->Parameters;
    par[0]->Value = pAdd->flags.reserv ? -pAdd->m_pub_xx : pAdd->m_pub_xx;
    par[1]->Value = pAdd->m_add_xx;
    par[4]->Value = pAdd->szpalt_x;
    par[5]->Value = pAdd->szpalt_y;
    par[6]->Value = pAdd->posx;
    par[7]->Value = pAdd->posy;
    par[8]->Value = pAdd->flags.locked;
    par[9]->Value = pAdd->sizex;
    par[10]->Value = pAdd->sizey;
    par[11]->Value = gcnew String(pAdd->nazwa);
    par[12]->Value = pAdd->nreps;
    par[13]->Value = gcnew String(pAdd->wersja);
    par[14]->Value = gcnew String(pAdd->remarks);
    par[15]->Value = pAdd->kolor & 0x07;
    par[16]->Value = ((CMainFrame*)AfxGetMainWnd())->Spot_ID[pAdd->kolor >> 3];
    TCHAR op_zew[2], op_sekcji[2], op_pl[2], sekcja[30], pl[2], poz_na_str[10];
    int nr_w_sekcji = 0, nr_pl = 0;
    pAdd->ParseLogpage(op_zew, sekcja, op_sekcji, &nr_w_sekcji, pl, op_pl, &nr_pl, poz_na_str);
    par[17]->Value = gcnew String(op_zew);
    par[18]->Value = gcnew String(sekcja);
    par[19]->Value = gcnew String(op_sekcji);
    par[20]->Value = nr_w_sekcji;
    par[21]->Value = gcnew String(pl);
    par[22]->Value = gcnew String(op_pl);
    par[23]->Value = nr_pl;
    par[24]->Value = gcnew String(poz_na_str);
    par[25]->Value = pAdd->txtposx;
    par[26]->Value = pAdd->txtposy;
    par[27]->Value = pAdd->typ_xx;
    par[28]->Value = gcnew String(pAdd->czaskto);
    par[29]->Value = pAdd->powtorka == 0 ? 0 : (pAdd->powtorka - CTime(POWTSEED_0)).GetDays();
    par[30]->Value = pAdd->oldAdno;
    par[31]->Value = pAdd->flags.studio;
    par[32]->Value = gcnew String(pAdd->remarks_atex);
    par[33]->Value = pAdd->flags.epsok;
    par[34]->Value = pAdd->spad_flag;
    par[35]->Value = pAdd->nag_xx;
    par[36]->Value = pAdd->flags.digital;

    cmd->ExecuteNonQuery();

    if (bInsert) {
        pAdd->m_pub_xx = (int)static_cast<OracleDecimal>(par[0]->Value);
        if (pAdd->bank.insid > 0) {
            auto cmdBank = cmd->Connection->CreateCommand();
            cmdBank->CommandText = "begin wstaw_z_banku(:pub_xx,:bo_insid,:bo_n,:bo_k); end;";
            auto par2 = cmdBank->Parameters;
            par2->Add("pub_xx", OracleDbType::Int32)->Value = pAdd->m_pub_xx;
            par2->Add("bo_insid", OracleDbType::Int32)->Value = pAdd->bank.insid;
            par2->Add("bo_n", OracleDbType::Int32)->Value = pAdd->bank.n;
            par2->Add("bo_k", OracleDbType::Int32)->Value = pAdd->bank.k;
            cmdBank->ExecuteNonQuery();
        }
    }

    pAdd->SetClean();
}
#pragma endregion

#pragma region CManODPNET
BOOL CManODPNET::PrepareConnectionString(const CString& user, const CString& pass, const CString& database, BOOL parallel)
{
    g_ConnectionString = String::Format("max pool size={0};data source={1};user id={2};password={3}", parallel ? CGenEpsInfoDlg::GetCpuCnt() : 1, gcnew String(database), gcnew String(user), gcnew String(pass));

    auto conn = gcnew OracleConnection(g_ConnectionString);
    try {
        conn->Open();
        OdpHelper::String2CString(conn->DatabaseName, m_databaseName);
        m_databaseName.MakeUpper();
        m_userName = user;
        theApp.isRDBMS = TRUE;
        m_lastErrorMsg.Empty();
    } catch (OracleException^ oex) {
        theApp.isRDBMS = FALSE;
        m_lastErrorMsg = oex->Message;
    } finally {
        conn->Close();
    }

    return m_lastErrorMsg.IsEmpty();
}

BOOL CManODPNET::CkAccess(LPCTSTR tytul, LPCTSTR mutacja, LPCTSTR rights, BOOL szczekaj)
{
    if (!_tcscmp(rights, _T("S")) && (theApp.grupa & R_DEA))
        return TRUE;

    /* vu : Funkcja sprawdza poziom dostêpu do tytu³u i mutacji obecnie
            zalogowanego u¿ytkownika							end vu */
    BOOL ok = FALSE;
    String^ dostep;
    String^ loginname;
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->CommandText = "select dostep,loginname from upraw where tytul=:tyt and mutacja=:mut";
    cmd->Parameters->Add("tyt", OracleDbType::Varchar2, 3)->Value = gcnew String(tytul);
    cmd->Parameters->Add("mut", OracleDbType::Varchar2, 2)->Value = gcnew String(mutacja);
    try {
        conn->Open();
        auto odr = cmd->ExecuteReader(CommandBehavior::SingleRow);
        if (!odr->Read()) return FALSE;
        dostep = odr->GetString(0);
        loginname = odr->GetString(1);
        odr->Close();
        m_lastErrorMsg.Empty();
        ok = TRUE;
    } catch (OracleException^) {
        OdpHelper::ShowErrMsgDlg(L"Odczyt uprawnieñ z bazy nie powiód³ siê");
    } finally {
        conn->Close();
    }

    if (dostep->IndexOf(gcnew String(rights)) < 0 && (CDrawApp::ShortDateToCTime(theApp.activeDoc->data) < CTime::GetCurrentTime() && dostep->IndexOf("P") < 0)) {
        if (szczekaj)
            MessageBox::Show(nullptr, String::Format("Uprawnienia u¿ytkownika {0} do tytu³u {1} \ns¹ zbyt ma³e aby wykonaæ tê operacjê.", loginname, gcnew String(tytul)), L"Niedostateczne uprawnienia", MessageBoxButtons::OK, MessageBoxIcon::Stop);
        ok = FALSE;
    }

    return ok;
}

BOOL CManODPNET::CkAccess(LPCTSTR gazeta, LPCTSTR rights)
{
    TCHAR *ch, tytul[10], mutacja[5];
    ::StringCchCopy(tytul, 10, gazeta);
    if (ch = _tcschr(tytul, _T(' '))) {
        ::StringCchCopy(mutacja, 5, ch + 1);
        *ch = TCHAR(0);
    } else
        ::StringCchCopy(mutacja, 5, _T(" "));
    return CkAccess(tytul, mutacja, rights);
}

BOOL CManODPNET::EI(LPCSTR s, CManODPNETParms& ps)
{
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->CommandText = gcnew String(s);

    try {
        conn->Open();
        OdpHelper::ExecuteOpenedCommand(cmd, ps);
        m_lastErrorMsg.Empty();
    } catch (OracleException^ oex) {
        pin_ptr<const wchar_t> p = PtrToStringChars(oex->Message);
        m_lastErrorMsg = p;
        if (m_lastErrorMsg.Find(_T("ORA-20222")) < 0) {
            // loguj powa¿ny b³¹d
            if (m_lastErrorMsg.Find(_T("ORA-20")) < 0) {
                m_lastErrorMsg = CString(s) + _T(" => ") + m_lastErrorMsg;
                m_lastErrorMsg.Replace(_T("'"), _T("`"));
                if (m_lastErrorMsg.Find(_T("dump_error")) < 0) {
                    cmd->CommandText = String::Format("begin dump_error('{0}'); end;", gcnew String(m_lastErrorMsg));
                    try {
                        cmd->ExecuteNonQuery();
                    } catch (...) {
                        OdpHelper::ShowErrMsgDlg("Brak wymaganej roli");
                    }
                }
            }
            // formatujemy pierwsza linijke komunikatu
            int pos = m_lastErrorMsg.Find(_T(": "));
            if (pos > 0) m_lastErrorMsg = m_lastErrorMsg.Mid(pos + 2);
            pos = m_lastErrorMsg.Find(_T("\n"));
            if (pos > 0) m_lastErrorMsg = m_lastErrorMsg.Left(pos);
            AfxMessageBox(m_lastErrorMsg, MB_OK | MB_ICONINFORMATION);
            // theApp.m_ODB.CommitTrans(); // insert into debug
        }
    } finally {
        conn->Close();
    }

    return m_lastErrorMsg.IsEmpty();
}

BOOL CManODPNET::FillArr(std::vector<CString>* combo, LPCSTR sql, CManODPNETParms& ps, BOOL comboArray)
{
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->CommandText = gcnew String(sql);
    OdpHelper::AddParams(cmd, ps);

    try {
        conn->Open();
        auto odr = cmd->ExecuteReader(comboArray ? CommandBehavior::Default : CommandBehavior::SingleResult);
nextcombo:
        while (odr->Read())
            combo->emplace_back(OdpHelper::ReadOdrString(odr, 0));

        if (comboArray && odr->NextResult()) { // w tym scenariuszu combo jest tablica listboxów
            combo++;
            goto nextcombo;
        }
        odr->Close();
        m_lastErrorMsg.Empty();
        return TRUE;
    } catch (OracleException^ ex) {
        OdpHelper::ShowErrMsgDlg(20000 <= ex->Number && ex->Number < 21000 ? ex->Message : L"Pobranie s³ownika z bazy nie powiod³o siê");
        return FALSE;
    } finally {
        conn->Close();
    }
}

BOOL CManODPNET::FillListArr(CListBox* combo, LPCSTR sql, CManODPNETParms& ps, BOOL comboArray)
{
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->CommandText = gcnew String(sql);
    OdpHelper::AddParams(cmd, ps);

    try {
        conn->Open();
        auto odr = cmd->ExecuteReader(comboArray ? CommandBehavior::Default : CommandBehavior::SingleResult);
nextcombo:
        while (odr->Read()) {
            OdpHelper::String2WChar(odr->GetString(0), (wchar_t*)theApp.bigBuf, 100);
            combo->AddString(theApp.bigBuf);
        }

        if (comboArray && odr->NextResult()) { // w tym scenariuszu combo jest tablica listboxów
            combo++;
            goto nextcombo;
        }
        odr->Close();
        m_lastErrorMsg.Empty();
        return TRUE;
    } catch (OracleException^) {
        OdpHelper::ShowErrMsgDlg(L"Pobranie s³ownika z bazy nie powiod³o siê");
        return FALSE;
    } finally {
        conn->Close();
    }
}

BOOL CManODPNET::FillList(CListBox* list, LPCSTR sql, CManODPNETParms& ps, int indexPos)
{	// gdy nie pobieramy indexu, to indexPos == -1, gdy pobieramy to indexPos zawiera numer porz¹dkowy kolumny, który jest 0 lub 1
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->CommandText = gcnew String(sql);
    OdpHelper::AddParams(cmd, ps);

    try {
        conn->Open();
        auto odr = cmd->ExecuteReader(CommandBehavior::SingleResult);
        OdpHelper::RetrieveOutParams(cmd, ps);
        while (odr->Read()) {
            int ind = list->AddString(OdpHelper::ReadOdrString(odr, 1 - indexPos*indexPos));
            if (indexPos >= 0)
                list->SetItemData(ind, odr->GetInt32(indexPos));
        }
        odr->Close();
        m_lastErrorMsg.Empty();
        return TRUE;
    } catch (OracleException^) {
        OdpHelper::ShowErrMsgDlg(L"Pobranie s³ownika z bazy nie powiod³o siê");
        return FALSE;
    } finally {
        conn->Close();
    }
}

BOOL CManODPNET::FillCombo(CComboBox* combo, LPCSTR sql, CManODPNETParms& ps, int indexPos)
{	// gdy nie pobieramy indexu, to indexPos == -1, gdy pobieramy to indexPos zawiera numer porz¹dkowy kolumny, który jest 0 lub 1
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->CommandText = gcnew String(sql);
    OdpHelper::AddParams(cmd, ps);

    try {
        conn->Open();
        auto odr = cmd->ExecuteReader(CommandBehavior::SingleResult);
        while (odr->Read()) {
            int ind = combo->AddString(OdpHelper::ReadOdrString(odr, 1 - indexPos*indexPos));
            if (indexPos >= 0)
                combo->SetItemData(ind, odr->GetInt32(indexPos));
        }
        odr->Close();
        m_lastErrorMsg.Empty();
        return TRUE;
    } catch (OracleException^) {
        OdpHelper::ShowErrMsgDlg(L"Pobranie s³ownika z bazy nie powiod³o siê");
        return FALSE;
    } finally {
        conn->Close();
    }
}

BOOL CManODPNET::FillNiekratoweInternal(int szpalt_x, int szpalt_y, int typ, CComboBox* m_typ_ogl_combo, CWordArray* m_typ_ogl_arr, CByteArray* m_typ_sizex_arr, CByteArray* m_typ_sizey_arr, std::vector<CString>* m_typ_precel_arr)
{
    int typ_xx;
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->CommandText = "select xx,sym,sizex,sizey,precel_flag from niekrat where szpalt_x=:sx and szpalt_y=:sy";
    cmd->Parameters->Add("sx", OracleDbType::Int32)->Value = szpalt_x;
    cmd->Parameters->Add("sy", OracleDbType::Int32)->Value = szpalt_y;

    m_typ_ogl_combo->ResetContent();
    m_typ_ogl_arr->RemoveAll();
    m_typ_sizex_arr->RemoveAll();
    m_typ_sizey_arr->RemoveAll();
    m_typ_precel_arr->clear();

    try {
        conn->Open();
        auto odr = cmd->ExecuteReader(CommandBehavior::SingleResult);
        while (odr->Read()) {
            auto sym = OdpHelper::ReadOdrString(odr, 1);
            m_typ_ogl_combo->AddString(sym);
            typ_xx = odr->GetInt32(0);
            m_typ_ogl_arr->Add(typ_xx);
            if (typ == typ_xx)
                m_typ_ogl_combo->SelectString(0, sym);
            m_typ_sizex_arr->Add(odr->GetInt32(2));
            m_typ_sizey_arr->Add(odr->GetInt32(3));
            m_typ_precel_arr->emplace_back(OdpHelper::ReadOdrString(odr, 4));
        }
        m_lastErrorMsg.Empty();
        odr->Close();
    } catch (OracleException^) {
        OdpHelper::ShowErrMsgDlg(L"Pobranie rozmiarów nie powiod³o siê");
        return FALSE;
    } finally {
        conn->Close();
    }

    return TRUE;
}

BOOL CManODPNET::FillNiekratowe(CSpacerDlg* dlg, int szpalt_x, int szpalt_y)
{
    return FillNiekratoweInternal(szpalt_x, szpalt_y, dlg->m_typ_xx, &dlg->m_typ_ogl_combo, &dlg->m_typ_ogl_arr, &dlg->m_typ_sizex_arr, &dlg->m_typ_sizey_arr, &dlg->m_typ_precel_arr);
}

BOOL CManODPNET::FillNiekratowe(CAddDlg* dlg)
{
    return FillNiekratoweInternal(dlg->m_szpalt_x, dlg->m_szpalt_y, dlg->m_typ_xx, &dlg->m_typ_ogl_combo, &dlg->m_typ_ogl_arr, &dlg->m_typ_sizex_arr, &dlg->m_typ_sizey_arr, &dlg->m_typ_precel_arr);
}

BOOL CManODPNET::IniKolorTable(CMainFrame* mf)
{
    int i;
    for (const auto& b : mf->Spot_Brush)
        delete b;
    mf->Spot_Kolor.clear(); mf->Spot_Brush.clear(); mf->Spot_ID.clear();
    mf->Spot_Kolor.emplace_back(BRAK);  mf->Spot_Brush.push_back(new CBrush(RGB(190, 190, 190)));  mf->Spot_ID.push_back(0);
    mf->Spot_Kolor.emplace_back(FULL);  mf->Spot_Brush.push_back(new CBrush(BIALY));  mf->Spot_ID.push_back(0);

    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->CommandText = gcnew String("select xx,nazwa,to_number(rgb,'XXXXXX') from kolory_spotu order by 2");
    try {
        conn->Open();
        auto odr = cmd->ExecuteReader(CommandBehavior::SingleResult);
        while (odr->Read()) {
            i = odr->GetInt32(0);
            mf->Spot_ID.push_back(i);
            mf->Spot_Kolor.emplace_back(OdpHelper::ReadOdrString(odr, 1));
            i = odr->GetInt32(2);
            mf->Spot_Brush.push_back(new CBrush(i));
        }
        odr->Close();
        m_lastErrorMsg.Empty();
        return TRUE;
    } catch (OracleException^) {
        OdpHelper::ShowErrMsgDlg(L"Pobranie s³ownika z bazy nie powiod³o siê");
        return FALSE;
    } finally {
        conn->Close();
    }
}

BOOL CManODPNET::GetManamEps()
{
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->CommandText = "select datafile from space_reservation.manam_clob where nazwa='manam.eps'";
    try {
#pragma push_macro("GetTempPath")
#undef GetTempPath
        auto manamEpsName = Path::GetTempPath() + L"Manam.eps";
#pragma pop_macro("GetTempPath")
        conn->Open();
        auto manamEpsContent = cmd->ExecuteScalar()->ToString();

        File::WriteAllText(manamEpsName, manamEpsContent);
        OdpHelper::String2CString(manamEpsName, theApp.sManamEpsName);
        m_lastErrorMsg.Empty();
        return TRUE;
    } catch (OracleException^) {
        OdpHelper::ShowErrMsgDlg(L"Pobranie z bazy szablonu postscriptu nie powiod³o siê i produkcja kolumn nie jest mo¿liwa");
        return FALSE;
    } catch (Exception^) {
        OdpHelper::ShowErrMsgDlg(L"Zapis szablonu do katalogu tymczasowego siê nie powiód³ i produkcja kolumn nie jest mo¿liwa");
        return FALSE;
    } finally {
        conn->Close();
    }
}

BOOL CManODPNET::OpenManamDoc(CDrawDoc* doc)
{
    auto opener = CDrawDocDbReader(doc);
    return opener.OpenManamDoc();
}

BOOL CManODPNET::SaveManamDoc(CDrawDoc* doc, BOOL isSaveAs, BOOL doSaveAdds)
{
    auto opener = CDrawDocDbWriter(doc);
    return opener.SaveManamDoc(isSaveAs, doSaveAdds);
}

BOOL CManODPNET::RmSysLock(CDrawDoc* doc)
{
    if (!theApp.isRDBMS) return FALSE;

    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->CommandText = String::Format("call {0}drop_syslock{1}(:mak_xx)", doc->isGRB ? "grb." : String::Empty, doc->isLIB ? "_lib" : String::Empty);
    cmd->Parameters->Add("mak_xx", OracleDbType::Int32)->Value = doc->m_mak_xx;

    try {
        conn->Open();
        cmd->ExecuteNonQuery();
        m_lastErrorMsg.Empty();
        return TRUE;
    } catch (OracleException^) {
        OdpHelper::ShowErrMsgDlg(L"Zdjêcie blokady nie powiod³o siê");
        return FALSE;
    } finally {
        conn->Close();
    }
}

BOOL CManODPNET::GrbSaveMutczas(CDrawDoc* doc)
{
    int cc = (int)doc->m_pages.size();
    auto mutArr = gcnew array<int>(cc);
    for (int i = 0; i < cc; i++)
        mutArr[i] = doc->m_pages[i]->m_mutczas;

    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->CommandText = "begin grb.update_mutczas(:mak_xx,:cc,:mutczasArr); end;";
    cmd->Parameters->Add("mak_xx", OracleDbType::Int32, ParameterDirection::InputOutput)->Value = doc->m_mak_xx;
    cmd->Parameters->Add("cc", OracleDbType::Int32)->Value = cc;

    auto ma = cmd->Parameters->Add("mutczasArr", OracleDbType::Int32);
    ma->CollectionType = OracleCollectionType::PLSQLAssociativeArray;
    ma->Size = cc;
    ma->Value = mutArr;

    try {
        conn->Open();
        cmd->ExecuteNonQuery();
        doc->m_mak_xx = (int)static_cast<OracleDecimal>(cmd->Parameters[0]->Value);
        m_lastErrorMsg.Empty();
        return TRUE;
    } catch (OracleException^) {
        OdpHelper::ShowErrMsgDlg(L"B³¹d zapisu mutacji");
        return FALSE;
    } finally {
        conn->Close();
    }
}

BOOL CManODPNET::LoadMakietaDirs(int idm)
{
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->CommandText = "begin dir.makieta_load(:mak_xx,:retCur); end;";
    cmd->Parameters->Add("mak_xx", OracleDbType::Int32)->Value = idm;
    cmd->Parameters->Add("refCur", OracleDbType::RefCursor)->Direction = ParameterDirection::Output;

    try {
        conn->Open();
        auto odr = cmd->ExecuteReader(CommandBehavior::SingleRow);
        if (odr->Read()) {
            theApp.SetRegistryBase(_T("GenEPS"));
            theApp.WriteString(_T("EpsDst"), OdpHelper::ReadOdrString(odr, 0));
            theApp.WriteString(_T("PsDst"), OdpHelper::ReadOdrString(odr, 1));
            theApp.WriteString(_T("EpsZajawki"), OdpHelper::ReadOdrString(odr, 2));
            theApp.WriteString(_T("EpsPodwaly"), OdpHelper::ReadOdrString(odr, 3));
            theApp.WriteString(_T("EpsDrobne"), OdpHelper::ReadOdrString(odr, 4));
            theApp.WriteString(_T("EpsUzupel"), OdpHelper::ReadOdrString(odr, 5));
        }
        m_lastErrorMsg.Empty();
        odr->Close();
    } catch (Exception^) {
        OdpHelper::ShowErrMsgDlg(L"Pobranie katalogów nie powiod³o siê");
        return FALSE;
    } finally {
        conn->Close();
    }

    return TRUE;
}

BOOL CManODPNET::F4(CDrawDoc* doc, CListCtrl* list, BOOL initialize)
{
    CString cs;
    CDrawAdd *vAdd;
    int id, rc = 0;
    auto adnos = gcnew List<int>();
    auto fileIds = gcnew List<int>();
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->Parameters->Add("mak_xx", OracleDbType::Int32)->Value = doc->m_mak_xx;

    try {
        conn->Open();
        if (initialize) {
            cmd->CommandText = gcnew String("select adno,fileid,lastfileid,transmited,kto_wprowadzil,storage_date from pub_fileid where mak_xx=:mak_xx order by 1 desc");
            auto odr = cmd->ExecuteReader(CommandBehavior::SingleResult);
            while (odr->Read()) {
                id = odr->GetInt32(0);
                if (vAdd = doc->AddExists(id)) {
                    cs.Format(_T("%li"), id);
                    list->InsertItem(rc, cs, 2);
                    list->SetCheck(rc, FALSE);
                    list->SetItemData(rc, 0L);
                    cs.Format(_T("%i"), vAdd->fizpage >> 3);
                    if (vAdd->fizpage & c_rzym)
                        cs += _T("rom");
                    list->SetItemText(rc, 1, cs);
                    vAdd->iFileId = odr->GetInt32(1);
                    cs.Format(_T("%li"), vAdd->iFileId);
                    list->SetItemText(rc, 2, cs);
                    id = odr->GetInt32(2);
                    cs.Format(_T("%li"), id);
                    list->SetItemText(rc, 3, cs);
                    if (id != 0 && vAdd->iFileId != id) { // podmiana
                        list->SetItem(rc, 0, LVIF_IMAGE, _T(""), 1, LVIF_IMAGE, LVIF_IMAGE, 1);
                        list->SetItemData(rc, 1L);
                    } else if (id == 0) { // inicjalizacja lastfileid
                        adnos->Add(vAdd->nreps);
                        fileIds->Add(vAdd->iFileId);
                    }
                    id = odr->GetInt32(3);
                    if (id != 0 && vAdd->iFileId != 0 && vAdd->iFileId != id) { // transmisja w toku
                        list->SetItem(rc, 0, LVIF_IMAGE, _T(""), 0, LVIF_IMAGE, LVIF_IMAGE, 1);
                        list->SetItemData(rc, 0L);
                    }
                    cs = OdpHelper::ReadOdrString(odr, 4);
                    list->SetItemText(rc, 4, cs);
                    cs = OdpHelper::ReadOdrString(odr, 5);
                    list->SetItemText(rc++, 5, cs);
                }
            }
            odr->Close();
        } else {
            rc = list->GetItemCount();
            for (int i = 0; i < rc; i++)
                if (list->GetItemData(i) && list->GetCheck(i)) {
                    list->SetCheck(i, FALSE);
                    list->SetItem(i, 0, LVIF_IMAGE, _T(""), 2, LVIF_IMAGE, LVIF_IMAGE, 0);
                    list->GetItemText(i, 0, theApp.bigBuf, bigSize);
                    vAdd = doc->AddExists(_ttol(theApp.bigBuf));
                    adnos->Add(vAdd->nreps);
                    fileIds->Add(vAdd->iFileId);
                    ::StringCchPrintf(theApp.bigBuf, n_size, _T("%d"), vAdd->iFileId);
                    list->SetItemText(i, 3, theApp.bigBuf);
                }
        }

        if (adnos->Count > 0) {
            cmd->CommandText = gcnew String("begin update_fileid_4f4(:mak_xx,:adnos,:fileids); end;");
            auto pa = cmd->Parameters->Add("adnos", OracleDbType::Int32);
            pa->CollectionType = OracleCollectionType::PLSQLAssociativeArray;
            pa->Value = adnos->ToArray();
            auto pf = cmd->Parameters->Add("fileids", OracleDbType::Int32);
            pf->CollectionType = OracleCollectionType::PLSQLAssociativeArray;
            pf->Value = fileIds->ToArray();
            cmd->ExecuteNonQuery();
        }
    } catch (OracleException^ oex) {
        OdpHelper::ShowErrMsgDlg(oex->Message);
        return FALSE;
    } finally {
        conn->Close();
    }

    return TRUE;
}

BOOL CManODPNET::ReadAcDeadlines(CDrawDoc* doc)
{
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->Parameters->Add("mak_xx", OracleDbType::Int32)->Value = doc->m_mak_xx;
    cmd->Parameters->Add("refCur", OracleDbType::RefCursor)->Direction = ParameterDirection::Output;
    cmd->CommandText = "begin open_ac_deadlines(:mak_xx,:retCur); end;";

    try {
        conn->Open();
        auto odr = cmd->ExecuteReader(CommandBehavior::SingleResult);
        while (odr->Read()) {
            int id_str = odr->GetInt32(0);
            auto p = std::find_if(std::cbegin(doc->m_pages), std::cend(doc->m_pages), [id_str](auto p) { return p->id_str == id_str; });
            if (p != std::end(doc->m_pages)) {
                auto s = OdpHelper::ReadOdrString(odr, 1);
                (*p)->m_ac_red = CDrawApp::ShortDateToCTime(s);
                s = OdpHelper::ReadOdrString(odr, 2);
                (*p)->m_ac_fot = CDrawApp::ShortDateToCTime(s);
                s = OdpHelper::ReadOdrString(odr, 3);
                (*p)->m_ac_kol = CDrawApp::ShortDateToCTime(s);
            }
        }

        odr->Close();
    } catch (OracleException^ oex) {
        OdpHelper::ShowErrMsgDlg(oex->Message);
        return FALSE;
    } finally {
        conn->Close();
    }

    return TRUE;
}

BOOL CManODPNET::InitRozm(CDrawDoc* doc)
{
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->Parameters->Add("mak_xx", OracleDbType::Int32)->Value = doc->m_mak_xx;
    cmd->Parameters->Add("refCur", OracleDbType::RefCursor)->Direction = ParameterDirection::Output;
    cmd->CommandText = "begin rozmiary_dla_makiety(:mak_xx,:retCur); end;";

    try {
        conn->Open();
        OdpHelper::InitRozmInternal(cmd, &doc->m_Rozm);
    } catch (OracleException^ oex) {
        OdpHelper::ShowErrMsgDlg(oex->Message);
        return FALSE;
    } finally {
        conn->Close();
    }

    return TRUE;
}

CRozm* CManODPNET::AddRozmTypu(std::vector<CRozm>* roz, int typ_xx)
{
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->Parameters->Add("typ_xx", OracleDbType::Int32)->Value = typ_xx;
    cmd->CommandText = "select * from rozmogl where drw_xx=0 and typ_xx=:typ_xx";

    try {
        conn->Open();
        if (OdpHelper::InitRozmInternal(cmd, roz))
            return &roz->back();
    } catch (OracleException^ oex) {
        OdpHelper::ShowErrMsgDlg(oex->Message);
    } finally {
        conn->Close();
    }

    return nullptr;
}

BOOL CManODPNET::GetAcceptStatus(int grb_xx, CString& ldrz, CString& cdrz, CString& org)
{
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->Parameters->Add("mak_xx", OracleDbType::Int32)->Value = grb_xx;
    cmd->Parameters->Add("refCur", OracleDbType::RefCursor)->Direction = ParameterDirection::Output;
    cmd->CommandText = "begin grb.kto_zatwierdzil(:grb_xx,:retCur); end;";

    try {
        conn->Open();
        auto odr = cmd->ExecuteReader(CommandBehavior::SingleRow);
        if (odr->Read()) {
            ldrz = OdpHelper::ReadOdrString(odr, 2);
            cdrz = OdpHelper::ReadOdrString(odr, 3);
            org = OdpHelper::ReadOdrString(odr, 4);
        }

        odr->Close();
    } catch (OracleException^ oex) {
        OdpHelper::ShowErrMsgDlg(oex->Message);
        return FALSE;
    } finally {
        conn->Close();
    }

    return TRUE;
}

BOOL CManODPNET::SpacerMulti(const std::vector<int>& mak_xxArr, std::vector<CString>& arr, CManODPNETParms& ps)
{
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->CommandText = "begin spacer.reserve_mul(:mak_xxArr,:str_xx,:szpalt_x,:szpalt_y,:x,:y,:sizex,:sizey,:qf,:blokada,:nazwa,:wersja,:uwagi,:ile_kol,:spo_xx,:typ_xx,:add_xx,:pub_xx,:czaskto,:msgArr); end;";

    OdpHelper::AddParams(cmd, ps);

    int cnt = (int)mak_xxArr.size();
    auto man_mak_xxArr = gcnew array<int>(cnt);
    auto man_bindSize = gcnew array<int>(cnt);
    for (int i = 0; i < cnt; ++i) {
        man_mak_xxArr[i] = mak_xxArr[i];
        man_bindSize[i] = 255;
    }

    auto m = gcnew OracleParameter("mak_xxArr", OracleDbType::Int32, ParameterDirection::Input);
    m->Value = man_mak_xxArr;
    m->Size = cnt;
    m->CollectionType = OracleCollectionType::PLSQLAssociativeArray;
    cmd->Parameters[0] = m;

    auto s = cmd->Parameters->Add("msgArr", OracleDbType::Varchar2, ParameterDirection::Output);
    s->Value = nullptr;
    s->Size = cnt;
    s->ArrayBindSize = man_bindSize;
    s->CollectionType = OracleCollectionType::PLSQLAssociativeArray;

    try {
        conn->Open();
        cmd->ExecuteNonQuery();

        for (int i = 16; i < 19; ++i)
            OdpHelper::RetrieveOdpParam(&ps.params[i], cmd->Parameters[i]);
    } catch (OracleException^ oex) {
        OdpHelper::ShowErrMsgDlg(oex->Message);
        return FALSE;
    } finally {
        conn->Close();
    }

    CString msg;
    auto sa = static_cast<array<OracleString>^>(s->Value);
    for (int i = 0; i < cnt; ++i) {
        msg.Empty();
        if (sa[i] != nullptr)
            OdpHelper::String2CString(sa[i].ToString(), msg);
        arr.emplace_back(msg);
    }

    return TRUE;
}

BOOL CManODPNET::Zapora(std::vector<int>* pub_xxArr)
{
    int cnt = (int)pub_xxArr->size();
    auto man_pub_xxArr = gcnew array<int>(cnt);
    for (int i = 0; i < cnt; ++i)
        man_pub_xxArr[i] = (*pub_xxArr)[i];

    auto m = gcnew OracleParameter("pub_xxArr", OracleDbType::Int32, ParameterDirection::Input);
    m->Value = man_pub_xxArr;
    m->Size = cnt;
    m->CollectionType = OracleCollectionType::PLSQLAssociativeArray;

    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->CommandText = "begin spacer.zapora(:pub_xxArr); end;";
    cmd->Parameters->Add(m);

    try {
        conn->Open();
        cmd->ExecuteNonQuery();
    } catch (OracleException^ oex) {
        OdpHelper::ShowErrMsgDlg(oex->Message);
        return FALSE;
    } finally {
        conn->Close();
    }

    return TRUE;
}

BOOL CManODPNET::Deploy(const CString& filepath)
{
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    cmd->CommandText = "select pe from space_reservation.manam_exe where aktywna=1";

    try {
        conn->Open();
        auto img = dynamic_cast<array<byte>^>(cmd->ExecuteScalar());
        if (img != nullptr)
            File::WriteAllBytes(gcnew String(filepath), img);
    } catch (OracleException^ oex) {
        OdpHelper::ShowErrMsgDlg(oex->Message);
        return FALSE;
    } finally {
        conn->Close();
    }

    return TRUE;
}

CString CManODPNET::GetProdInfo(int pub_xx, LPTSTR kolor, int *ileMat)
{
    CString sBBox;
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    auto par = cmd->Parameters;
    cmd->CommandText = "begin epstest.get_prod_info(:pub_xx,:retCur); end;";
    par->Add("pub_xx", OracleDbType::Decimal)->Value = pub_xx;
    par->Add("refCur", OracleDbType::RefCursor)->Direction = ParameterDirection::Output;

    try {
        conn->Open();
        auto odr = cmd->ExecuteReader(CommandBehavior::SingleRow);
        if (odr->Read()) {
            sBBox = OdpHelper::ReadOdrString(odr, 0);
            if (kolor != nullptr)
                ::StringCchCopy(kolor, 2, OdpHelper::ReadOdrString(odr, 1));
            if (ileMat != nullptr)
                *ileMat = odr->GetInt32(2);
        } else
            sBBox = "!";
        odr->Close();
    } catch (OracleException^) {
        OdpHelper::ShowErrMsgDlg(L"Pobranie prod_info nie powiod³o siê");
        sBBox.Empty();
    } finally {
        conn->Close();
    }

    return sBBox;
}

CString CManODPNET::AdnoDlaZajawki(int* p, int* o)
{	// wejscie <zaj_xx, mak_xx>, wyjscie <oldadno, powtorka>
    CString sAdno;
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    auto par = cmd->Parameters;
    cmd->CommandText = "begin adno_dla_zajawki(:zaj_xx,:mak_xx,:retCur); end;";
    par->Add("zaj_xx", OracleDbType::Int32)->Value = *p;
    par->Add("mak_xx", OracleDbType::Int32)->Value = *o;
    par->Add("refCur", OracleDbType::RefCursor)->Direction = ParameterDirection::Output;

    try {
        conn->Open();
        auto odr = cmd->ExecuteReader(CommandBehavior::SingleRow);
        if (odr->Read()) {
            sAdno = OdpHelper::ReadOdrString(odr, 0);
            *p = odr->GetInt32(1);
            *o = odr->GetInt32(2);
        }
        odr->Close();
    } catch (OracleException^) {
        OdpHelper::ShowErrMsgDlg(L"Pobranie numeru zajawki nie powiod³o siê");
        sAdno.Empty();
    } finally {
        conn->Close();
    }

    return sAdno;
}

CString CManODPNET::GetHttpSource(const CString& gazeta, const CString& kiedy, int* s)
{	// s to adno na wejsciu i status na wyjsciu
    CString ret;
    auto conn = gcnew OracleConnection(g_ConnectionString);
    auto cmd = conn->CreateCommand();
    auto par = cmd->Parameters;
    cmd->CommandText = "begin epstest.get_httpsource(:tytul,:mutacja,:kiedy,:adno,:retCur); end;";
    par->Add("tytul", OracleDbType::Varchar2)->Value = gcnew String(gazeta.Left(3));
    par->Add("mutacja", OracleDbType::Varchar2)->Value = gcnew String(gazeta.Mid(4, 2));
    par->Add("kiedy", OracleDbType::Varchar2)->Value = gcnew String(kiedy);
    par->Add("adno", OracleDbType::Int32)->Value = *s;
    par->Add("refCur", OracleDbType::RefCursor)->Direction = ParameterDirection::Output;

    try {
        conn->Open();
        auto odr = cmd->ExecuteReader(CommandBehavior::SingleRow);
        if (odr->Read()) {
            *s = odr->GetInt32(0);
            ret = OdpHelper::ReadOdrString(odr, 1);
        }
        odr->Close();
    } catch (OracleException^) {
        OdpHelper::ShowErrMsgDlg(L"Pobranie url do preview nie powiod³o siê");
    } finally {
        conn->Close();
    }

    return ret;
}
#pragma endregion
