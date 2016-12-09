#pragma once

class CAddDlg;
class CDrawDoc;
class CSpacerDlg;
class CMainFrame;

struct CRozm;
struct CManODPNETParms;

class CManODPNET {
public:
	#pragma region managed constants forward
	static const int DbTypeInt32;
	static const int DbTypeDouble;
	static const int DbTypeVarchar2;
	static const int DbTypeRefCursor;

	static const int ParameterIn;
	static const int ParameterOut;
	static const int ParameterInOut;
	static const int ReturnValue;
	#pragma endregion

	CString m_lastErrorMsg;
	CString m_userName;
	CString m_databaseName;
	BOOL PrepareConnectionString(const CString& user, const CString& pass, const CString& database, BOOL parallel = FALSE);
	BOOL EI(LPCSTR s, CManODPNETParms& ps = CManODPNET::emptyParm);
	BOOL CkAccess(LPCTSTR tytul, LPCTSTR mutacja, LPCTSTR rights, BOOL szczekaj = TRUE);
	BOOL CkAccess(LPCTSTR gazeta, LPCTSTR rights);

	BOOL GetManamEps();				// pobiera z bazy plik z definicj¹ postscriptu
	BOOL FillArr(std::vector<CString>* arr, LPCSTR sql, CManODPNETParms& ps, BOOL comboArray = FALSE);
	BOOL FillListArr(CListBox* list, LPCSTR sql, CManODPNETParms& ps, BOOL comboArray = FALSE);
	BOOL FillList(CListBox* list, LPCSTR sql, CManODPNETParms& ps, int indexPos = -1);
	BOOL FillCombo(CComboBox* combo, LPCSTR sql, CManODPNETParms& ps, int indexPos = -1);
	BOOL FillNiekratowe(CSpacerDlg* dlg, int szpalt_x, int szpalt_y);
	BOOL FillNiekratowe(CAddDlg* dlg);
	BOOL IniKolorTable(CMainFrame* mf);
	BOOL OpenManamDoc(CDrawDoc* doc);	// pobiera z bazy podstawowe informacje o makiecie lub grzbiecie
	BOOL SaveManamDoc(CDrawDoc* doc, BOOL isSaveAs, BOOL doSaveAdds);	// zachowuje zmiany w makiecie
	BOOL RmSysLock(CDrawDoc* doc);
	BOOL GrbSaveMutczas(CDrawDoc* doc);
	BOOL LoadMakietaDirs(int idm);
	BOOL F4(CDrawDoc* doc, CListCtrl* list, BOOL initialize);
	BOOL ReadAcDeadlines(CDrawDoc* doc);
	BOOL InitRozm(CDrawDoc* doc);
	CRozm* AddRozmTypu(std::vector<CRozm>* roz, int typ_xx);
	BOOL GetAcceptStatus(int grb_xx, CString& ldrz, CString& cdrz, CString& org);
	BOOL SpacerMulti(const std::vector<int>& mak_xxArr, std::vector<CString>& arr, CManODPNETParms& ps);
	BOOL Zapora(std::vector<int>* pub_xxArr);
	BOOL Deploy(const CString& filepath);
	CString GetProdInfo(int pub_xx, LPTSTR kolor, int *ileMat);
	CString AdnoDlaZajawki(int* p, int* o);
	CString GetHttpSource(const CString& gazeta, const CString& kiedy, int* s);
	static CManODPNETParms emptyParm;
private:
	BOOL FillNiekratoweInternal(int szpalt_x, int szpalt_y, int typ, CComboBox* m_typ_ogl_combo, CWordArray* m_typ_ogl_arr, CByteArray* m_typ_sizex_arr, CByteArray* m_typ_sizey_arr, std::vector<CString>* m_typ_precel_arr);
};

struct CManODPNETParm {
	CManODPNETParm(int odptype, void* val) : m_value(val), m_odptype(odptype), m_direction(CManODPNET::ParameterIn) {}
	CManODPNETParm(int odptype, int direction, void* val) : m_value(val), m_odptype(odptype), m_direction(direction) {}

	void* m_value;
	int m_odptype;
	int m_direction;
};

struct CManODPNETParms {
	CManODPNETParms(int odptype, void* val) : CManODPNETParms({ { odptype, CManODPNET::ParameterIn, val } }) {}
	CManODPNETParms(int odptype, int direction, void* val) : CManODPNETParms({ { odptype, direction, val } }) {}
	CManODPNETParms(const std::initializer_list<CManODPNETParm>& pl) : outParamsCount(0), params(pl), hasReturnValue(false) {}

	friend struct OdpHelper;
	friend BOOL CManODPNET::SpacerMulti(const std::vector<int>& mak_xxArr, std::vector<CString>& arr, CManODPNETParms& ps);

	int outParamsCount;						// liczba parametrów typu Out lub InOut nie licz¹c RefCursorów
private:
	std::vector<CManODPNETParm> params;		// kolekcja parametrów
	bool hasReturnValue;					// wskazanie do eykonania ExecuteScalar
};

extern CManODPNET theManODPNET;
