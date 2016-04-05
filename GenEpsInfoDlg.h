#pragma once

class CGenEpsInfoDlg;

typedef struct _MESPUMPDLGARG {
	HANDLE hEvent;
	CGenEpsInfoDlg *pDlg;
} MESPUMPDLGARG, *PMESPUMPDLGARG;

class CGenEpsInfoDlg : public CDialog /* singleton */
{
	DECLARE_DYNAMIC(CGenEpsInfoDlg)

private:
	enum { IDD = IDD_WAIT };
	static int iCpuCnt;
	static CGenEpsInfoDlg m_instance;
	static const int ciMaxChannels = 2;

	HANDLE hCreationEvent;			// event u�ywany dwukrotnie do synchronizacji w�tku roboczego
	HANDLE hWorkingThread;			// uchwyt do w�tku roboczego

	CGenEpsInfoDlg();				// konstruktor prywatny singletonu
	void OnInterruptProcessing();	// obs�uga polecenia: "Przerwij prac�"

public:
	static DWORD WINAPI CreateGenEPSDialog(PMESPUMPDLGARG pArg);	// wykonywana przez w�tek roboczy, definiuje message loop
	static CGenEpsInfoDlg* GetGenEpsInfoDlg(BOOL bIsGen);			// inicjalizacja okna dialogowego
	static void ReleaseGenEpsInfoDlg(CGenEpsInfoDlg* pDlg);			// zwolnienie okna
	static int GetCpuCnt();											// ile mamy procesor�w 
	static INT_PTR CALLBACK CGenEpsInfoDlg::DialogProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);	

	BOOL cancelGenEPS;				// flaga przerwania przetwarzania
	BOOL m_bIsGen;					// flaga okre�laj�ca, czy dialog jest w trybie generowania/sprawdzania 

	virtual ~CGenEpsInfoDlg();
	void SetChannelCount(int iChannelCnt);
	void StrInfo(int iChannel, const CString& s);
	void OglInfo(int iChannel, const CString& s);
};


