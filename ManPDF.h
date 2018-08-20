
#pragma once

#include "DrawPage.h"

using uintmap = std::map<unsigned int, int, std::less<>>;

class CDrawAdd;

class CManPDF
{
  public:
    static constexpr unsigned long ulNotFound = -1L;
    static constexpr char seppdf[]  = {'<', '>', '/', '[', ']'};
    static constexpr char sepline[] = {(char)10, (char)13, (char)0};
    static constexpr char septok[]  = {(char)9, (char)10, (char)13, (char)32, (char)0};
    static const char* memstr(const char* buf, const char* pat, size_t patlen);

    CManPDF(PGENEPSARG pArg);
    virtual ~CManPDF() = default;
    unsigned long GetMediaBox(const TCHAR* fpath, float* x1, float* y1, float* x2, float* y2, HANDLE hFile = nullptr);
    unsigned long SearchPattern(CFile& f, const char* pat) const;
    bool CreatePDF(CDrawPage* page, const TCHAR* trgName);

  private:
    static constexpr unsigned long pakBufLen = 10'000'000L;
    static constexpr long estTailLen = 30L;
    static constexpr int orgX = 80;
    static constexpr int orgY = 1100;
    static constexpr int maxTokenSize = 256;
    static constexpr bool compressContent = true;

    unsigned int nextObjNr;
    unsigned long xrefOffset;
    float bbx1, bby1, bbx2, bby2;

    CFile trg;
    CFileException fileErr;
    CGenEpsInfoDlg* dlg;

    uintmap renumMap;         // numerowi obiektu z pliku Ÿród³owego przyporz¹dkowuje nowy numer z pliku wynikowego

    int   iPdfTokenSlot;      // numer aktywnego buforu, w którym zwraca siê wynik funkcji pdftok
    bool  bSearchInPdfToken;  // wskaŸnik trybu dzia³ania funkcji pdftok - gdy ustawiony, to nie wywo³uje strtok
    char  cNextPdfToken[256]; // tablica, do której pdftok przepisuje kontynuacjê znalezionego tokenu, ma rozmiar maxTokenSize
    char  cPdfToken[5][256];  // tablica, do której pdftok przepisuje znaleziony token, ma rozmiar maxTokenSize, jest ich 5 by utworzy³y bufor cykliczny, nastêpne 4 wywo³ania nie zamazuj¹ wyniku
    char* pdftok(char* str);  // zachowuje siê podobnie jak strtok, ale rozpoznaje tokeny PDF nie rozdzielone niczym - przepisuje je do bufora cPdfToken w takim przypadku

    char* cStore;
    bool GenProlog();
    bool EmbedPDF(CFile& src, unsigned int objNr, const CDrawAdd& pAdd);
    void GenPDFTail(unsigned int howMany = 0);
    void EmbedContents(CFile& src, unsigned long offset);
    void EmbedRef(CFile& src, unsigned int srcObjNr);
    void EmbedKey(const char* buf, const char* key);
    void EmbedSection(const char* str, bool innerOnly = false);
    inline void EmbedTextRight(const char* font, unsigned int fsize, float px, float py, const char* text, CStringA& buf);
    unsigned int GetRefNr(char* buf);
    unsigned long GetMainXref(CFile& f);
    unsigned long FindObjEntry(CFile& f, unsigned int obj);
    unsigned long EmbedStream(CFile& src, unsigned long offset, bool decorate);
    unsigned long EmbedPakStream(CFile& src, unsigned long offset, unsigned char* pak, unsigned char* niepak, unsigned char* tmp, unsigned long niepakoff);
    unsigned long GetPrevXref(CFile& f, unsigned long currentxrefOffset);
};

class CManPDFExc
{
  public:
    CManPDFExc(const TCHAR* msg) { errMsg = CString(msg); }
    ~CManPDFExc() = default;
    const TCHAR* ShowReason() const { return errMsg; }
  private:
    CString errMsg;
};
