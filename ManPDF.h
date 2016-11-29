#pragma once
#pragma warning(disable:4786)

#include "drawpage.h"
#include <map>

typedef std::map<int, int, std::less<int> > uintmap;

class CDrawAdd;

class CManPDF {
	public:
		static const char septok[];
		static const char sepline[];
		static const char seppdf[];
		static const unsigned long ulNotFound;
		static const char* memstr(const char *str, const char *sub, size_t len);
		CManPDF(PGENEPSARG pArg);
		virtual ~CManPDF();
		unsigned long GetMediaBox(const TCHAR* fpath, float* x1, float* y1, float* x2, float* y2, HANDLE hFile = 0);
		unsigned long SearchPattern(CFile &f, const char *pat) const;
		BOOL CreatePDF(CDrawPage *page, const TCHAR* trgName);
	private:
		static const long estTailLen;
		static const long pakBufLen;
		static const int orgX,orgY;
		static const int maxTokenSize;
		static const BOOL compressContent;

		unsigned int nextObjNr;
		unsigned long xrefOffset;
		float bbx1,bby1,bbx2,bby2;

		CFile trg;
		CFileException fileErr;
		CGenEpsInfoDlg *dlg;

		uintmap renumMap;		// numerowi obiektu z pliku Ÿród³owego przyporz¹dkowuje nowy numer z pliku wynikowego

		int	  iPdfTokenSlot;		// numer aktywnego buforu, w którym zwraca siê wynik funkcji pdftok
		bool  bSearchInPdfToken;	// wskaŸnik trybu dzia³ania funkcji pdftok - gdy ustawiony, to nie wywo³uje strtok
		char  cNextPdfToken[256];	// tablica, do której pdftok przepisuje kontynuacjê znalezionego tokenu, ma rozmiar maxTokenSize
		char  cPdfToken[5][256];	// tablica, do której pdftok przepisuje znaleziony token, ma rozmiar maxTokenSize, jest ich 5 by utworzy³y bufor cykliczny, nastêpne 4 wywo³ania nie zamazuj¹ wyniku
		char* pdftok(char *str);   // zachowuje siê podobnie jak strtok, ale rozpoznaje tokeny PDF nie rozdzielone niczym - przepisuje je do bufora cPdfToken w takim przypadku

		char* cStore;
		BOOL GenProlog();
		BOOL EmbedPDF(CFile &src, unsigned int objNr, CDrawAdd &pAdd);
		void GenPDFTail(unsigned int howMany = 0);
		void EmbedContents(CFile &src, unsigned long offset);
		void EmbedRef(CFile &src, unsigned int srcObjNr);
		void EmbedKey(const char* buf, char* key);
		void EmbedSection(const char* lbound, BOOL innerOnly = FALSE);
		inline void EmbedTextRight(const char* font, unsigned int fsize, float px, float py, const char* text, CStringA& buf);
		unsigned int GetRefNr(char* buf);
		unsigned long GetMainXref(CFile &f);
		unsigned long FindObjEntry(CFile &f, unsigned int obj);
		unsigned long EmbedStream(CFile &src, unsigned long offset, BOOL decorate);
		unsigned long EmbedPakStream(CFile &src, unsigned long offset, unsigned char *pak, unsigned char *niepak, unsigned char *tmp, const unsigned long niepakoff);
		unsigned long GetPrevXref(CFile &f, unsigned long currentxrefOffset);
};

class CManPDFExc {
	public:
		CManPDFExc (const TCHAR* msg) { errMsg = CString(msg); }
		~CManPDFExc() {};
		const TCHAR *ShowReason() const { return errMsg; }
	private:
		CString errMsg;
};
