#pragma once

class CAddDlg;
class CDrawDoc;
class CSpacerDlg;
class CMainFrame;

struct CRozm;
struct CManODPNETParms;

class CManODPNET
{
  public:
    CString m_lastErrorMsg;
    CString m_userName;
    CString m_databaseName;
    BOOL PrepareConnectionString(const CString& user, const CString& pass, const CString& database, BOOL parallel = FALSE);
    BOOL EI(LPCSTR s, CManODPNETParms& ps = CManODPNET::emptyParm);
    BOOL CkAccess(LPCTSTR tytul, LPCTSTR mutacja, LPCTSTR rights, BOOL szczekaj = TRUE);
    BOOL CkAccess(LPCTSTR gazeta, LPCTSTR rights);

    void IniKolorTable();
    BOOL GetManamEps();                                               // pobiera z bazy plik z definicj¹ postscriptu
    BOOL FillArr(std::vector<CString>* arr, LPCSTR sql, CManODPNETParms& ps, BOOL comboArray = FALSE);
    BOOL FillListArr(CListBox* combo, LPCSTR sql, CManODPNETParms& ps, BOOL comboArray = FALSE);
    BOOL FillList(CListBox *list, LPCSTR sql, CManODPNETParms& ps, int indexPos = -1);
    BOOL FillCombo(CComboBox *combo, LPCSTR sql, CManODPNETParms& ps, int indexPos = -1);
    BOOL FillNiekratowe(CSpacerDlg *dlg, int szpalt_x, int szpalt_y);
    BOOL FillNiekratowe(CAddDlg *dlg);
    BOOL OpenManamDoc(CDrawDoc *doc);                                 // pobiera z bazy podstawowe informacje o makiecie lub grzbiecie
    BOOL SaveManamDoc(CDrawDoc *doc, BOOL isSaveAs, BOOL doSaveAdds); // zachowuje zmiany w makiecie
    BOOL RmSysLock(CDrawDoc *doc);
    BOOL GrbSaveMutczas(CDrawDoc *doc);
    BOOL LoadMakietaDirs(int idm);
    BOOL F4(CDrawDoc *doc, CListCtrl *list, BOOL initialize);
    BOOL ReadAcDeadlines(CDrawDoc* doc);
    BOOL InitRozm(CDrawDoc* doc);
    BOOL GetAcceptStatus(int grb_xx, CString& ldrz, CString& cdrz, CString& org);
    BOOL SpacerMulti(const std::vector<int>& mak_xxArr, std::vector<CString>& arr, CManODPNETParms& ps);
    BOOL Zapora(const std::vector<int>& pub_xxArr);
    BOOL Deploy(const CString& filepath);
    const CRozm* AddRozmTypu(std::vector<CRozm>& roz, int typ_xx);
    CString GetProdInfo(int pub_xx, LPTSTR kolor, int *ileMat);
    CString AdnoDlaZajawki(int *p, int *o);
    CString GetHttpSource(const CString& gazeta, const CString& kiedy, int *s);
    static CManODPNETParms emptyParm;
private:
    BOOL FillNiekratoweInternal(int szpalt_x, int szpalt_y, int typ, CComboBox *m_typ_ogl_combo, CWordArray *m_typ_ogl_arr, CByteArray *m_typ_sizex_arr, CByteArray *m_typ_sizey_arr, std::vector<CString> *m_typ_precel_arr);
};

struct CManODPNETParm
{
    CManODPNETParm(uint8_t odptype, void* val) : m_value(val), m_odptype(odptype), m_direction(CManDbDir::ParameterIn) {}
    CManODPNETParm(uint8_t odptype, uint8_t direction, void *val) : m_value(val), m_odptype(odptype), m_direction(direction) {}

    void *m_value;
    uint8_t m_odptype;
    uint8_t m_direction;
};

struct CManODPNETParms
{
    CManODPNETParms(uint8_t odptype, void* val) : CManODPNETParms({{odptype, CManDbDir::ParameterIn, val}}) {}
    CManODPNETParms(uint8_t odptype, uint8_t direction, void* val) : CManODPNETParms({{odptype, direction, val}}) {}
    CManODPNETParms(const std::initializer_list<CManODPNETParm>& pl) : outParamsCount(0), params(pl), hasReturnValue(false) {}

    friend struct OdpHelper;
    friend BOOL CManODPNET::SpacerMulti(const std::vector<int>& mak_xxArr, std::vector<CString>& arr, CManODPNETParms& ps);

    uint8_t outParamsCount;             // liczba parametrów typu Out lub InOut nie licz¹c RefCursorów
  private:
    std::vector<CManODPNETParm> params; // kolekcja parametrów
    bool hasReturnValue;                // wskazanie do eykonania ExecuteScalar
};

extern CManODPNET theManODPNET;
