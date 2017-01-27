// ManPDF.cpp : implementation file
//

#include "StdAfx.h"
#include "manam.h"
#include "ManPDF.h"
#include "drawdoc.h"
#include "drawadd.h"
#include "koldlg.h"
#include "genepsinfodlg.h"

#define ZLIB_WINAPI
#include "zlib.h"

const int CManPDF::orgX = 80;
const int CManPDF::orgY = 1100;
const int CManPDF::maxTokenSize = 256;
const long CManPDF::estTailLen = 30L;
const long CManPDF::pakBufLen = 10'000'000L;
const BOOL CManPDF::compressContent = TRUE;
const char CManPDF::sepline[] = {(char)10, (char)13, (char)0};
const char CManPDF::septok[] = {(char)9, (char)10, (char)13, (char)32, (char)0};
const char CManPDF::seppdf[] = {'<', '>', '/', '[', ']'};
const unsigned long CManPDF::ulNotFound = -1L;
extern CDrawApp NEAR theApp ;
/////////////////////////////////////////////////////////////////////////////
// CManPDF

CManPDF::CManPDF(PGENEPSARG pArg) {
	dlg = pArg->pDlg;
	bSearchInPdfToken = false;
	cStore = reinterpret_cast<char*>(pArg->cBigBuf);
	iPdfTokenSlot = 0;
	xrefOffset = 0L;
	nextObjNr = 7;
}

CManPDF::~CManPDF() {}

/////////////////////////////////////////////////////////////////////////////
// CManPDF message handlers

// rozszerza mo¿liwoœci funkcji strtok o wydzielanie tokenów PDF nierozdzielonych od siebie
char* CManPDF::pdftok(char *str) {
	if (str)  // wywo³anie, które nie jest kontynuacj¹
		cNextPdfToken[0] = '\0';
	if (cNextPdfToken[0]) {// rozpocznij analizê fragmentu tokeny wczeœniej zwróconego przez strtok
		char buf[256];
		::StringCchCopyA(buf, 256, cNextPdfToken);
		bSearchInPdfToken = true;
		iPdfTokenSlot++;
		iPdfTokenSlot %= 5;
		::StringCchCopyA(cPdfToken[iPdfTokenSlot], 256, pdftok(buf));
		bSearchInPdfToken = false;
	} else {
		char* p = bSearchInPdfToken ? str : strtok(str,septok);
		if (!p) return p;
		// na której pozycji p wystêpuje najwczeœniejszy znak spoœród seppdf[]
		char* e = p;
		int foundIndex = -1;
		while (e[0] && foundIndex<0) {
			for (int i = 0; i < 5; i++)
				if (e[0] == seppdf[i] && (e != p || i != 2)) { // znaku / na pocz¹tku nie traktuj jako specjalny
					foundIndex = i;
					break;
				}
			if (foundIndex<0) e++;
		}
		if (e[0] == '\0') // nie znaleziono znaków specjalnych
			return p;
		if (e == p) { // znaleziono znak specjalny na pocz¹tku p
			if (foundIndex < 2 && e[0] == e[1]) 
				e++;
			if (e[1] == '\0') // znaleziono token specjalny
				return p;
			else {
				e++;
				goto nextToken;
			}
		} else // znalezono znak specjalny wewn¹trz p
nextToken:
			iPdfTokenSlot++;
			iPdfTokenSlot %= 5;
			::StringCchCopyA(cNextPdfToken, 256, e);
			::StringCchCopyNA(cPdfToken[iPdfTokenSlot], maxTokenSize, p, e - p);
	}

	return cPdfToken[iPdfTokenSlot];
}

const char* CManPDF::memstr(const char *str, const char *sub, size_t len) {
  size_t l = 0;
  char *p;

  if (!str) return nullptr;
  if (strlen(str) == len) return strstr(str, sub);
  while(l<len)
  {
     if(!*(str+l)) {
       l++;
       continue;
     }
     if((p = strstr((char*)str+l, sub)) != nullptr) return (const char*)p;
     l += strlen(str + l) + 1;
  }
  return nullptr;
} //memstr

unsigned long CManPDF::SearchPattern(CFile &f, const char *pat) const {
	char *p;
	unsigned long filepos;
	size_t res = 1;
	int backLen = (int)strlen(pat);

	while (res > 0) { // read block from the file
		filepos = (unsigned long)f.GetPosition();
		res = f.Read(cStore, n_size);    // check if anything was read and no error occurred
		if (res < 1) break;
		if (res == n_size) 
			 f.Seek(-backLen, CFile::current); // buffer has to end with zero char to be treated as string
		cStore[res] = 0;

		if((p = (char*)memstr(cStore, pat, res)) != nullptr) {    // search for substring and get it's position
			 long offset = filepos + (long)(p - cStore);
			 f.Seek(offset - 1, CFile::begin);
			 f.Read(cStore, 1);
			 if (cStore[0] >= '0' && cStore[0] <= '9') 
				f.Seek(offset + 1, CFile::begin);
			 else
				return offset;
		}
	}
	return CManPDF::ulNotFound;
} //SearchPattern

unsigned long CManPDF::GetMainXref(CFile &f) {
	if (xrefOffset != 0L) 
		return xrefOffset;
	f.Seek(-estTailLen, CFile::end);
	f.Read(cStore,estTailLen);
	char *p = strstr(cStore, "startxref");
	if (!p) 
		throw CManPDFExc(_T("GetMainXref: nie ma znacznika startxref w ostatnich 30 bajtach"));
	strtok(p, septok);
	return xrefOffset = atol(strtok(nullptr, septok));
} //GetMainXref

// podaje poprzedni¹, póŸniej nadpisan¹ wartoœc offsetu do spisu xref
unsigned long CManPDF::GetPrevXref(CFile &f, unsigned long currentxrefOffset) {
	f.Seek(currentxrefOffset, CFile::begin);
	f.Read(cStore, bigSize);
	char *a = strstr(cStore, "/Prev");
	if (!a) return 0L;
	strtok(a, septok);
	return atol(strtok(nullptr, septok));
} //GetPrevXref

// podaje offset w pliku f pocz¹tku definicji obiektu numer obj
unsigned long CManPDF::FindObjEntry(CFile &f, unsigned int obj) {
	unsigned long currentxrefOffset = GetMainXref(f);
	while (currentxrefOffset != 0L) {
		//szukaj xref z obj
		f.Seek(currentxrefOffset,CFile::begin);
		f.Read(cStore,bigSize);
		unsigned int xrefstart,xreflen;
		char *a = strtok(cStore,sepline);
		xrefstart = atoi((a = strtok(nullptr, septok)));
		xreflen = atoi((a = strtok(nullptr, septok)));
		while (xrefstart>obj || obj>=xrefstart+xreflen) {
			if ((a - cStore) + (2 + xreflen)*20 > bigSize) {
				a = strtok(nullptr, sepline);
				f.Seek(currentxrefOffset + (a - cStore) + xreflen*20,CFile::begin);
				currentxrefOffset = (unsigned long)f.GetPosition();
				f.Read(cStore,bigSize);
				a = strtok(cStore,septok);
			} else {
				for (unsigned int j=0; j<xreflen; j++)
					a = strtok(nullptr, sepline);
				a = strtok(nullptr, septok);
			}
			if (!strncmp(a, "trailer",7)) {
				currentxrefOffset = GetPrevXref(f,currentxrefOffset);
				xrefstart = bigSize;
				break;
			}
			xrefstart = atoi(a);
			xreflen = atoi(strtok(nullptr, septok));
		}
		if (xrefstart == bigSize) continue;
		//znaleziono
		if ((a - cStore) + (2 + obj - xrefstart)*20 > bigSize) {
			a = strtok(nullptr, sepline);
			f.Seek(currentxrefOffset + (a - cStore) + (obj - xrefstart)*20,CFile::begin);
			f.Read(cStore,bigSize);
			return atol(strtok(cStore,septok));
		}
		for (unsigned int i=0; i < obj - xrefstart; i++) 
			a = strtok(nullptr, sepline);
		return atol(strtok(nullptr, septok));
	}
	return 0L;
} //FindObjEntry

// podaje najbli¿szy numer objNr z sekwencji "objNr 0 R"
unsigned int CManPDF::GetRefNr(char *buf) {
	std::vector<char*> a;
	char *p = pdftok(buf);
	do 
		a.push_back(p = pdftok(nullptr));
	while (p && strcmp(p, "R"));
	if (!p || a.size() < 3) 
		throw CManPDFExc(_T("GetRefNr: Nie odnaleziono prawidlowej referencji"));
	char* objNr = a[a.size() - 3];
	int i = 0;
	while (!isdigit((int)objNr[i]) && objNr[i]!='\0') 
		objNr[i++] = ' ';
	return atoi(objNr);
} //GetRefNr

// przepisuje s³ownik podmianiaj¹c numery obiektów Ÿród³owych na docelowe, 
// innerOnly okreœla, czy maj¹ zostaæ przepisane znaczniki pocz¹tku i koñca sekcji
// str jest tokenem uzyskanym przez strtok i kolejne wywo³anie da nastêpny w kolejnoœci token
void CManPDF::EmbedSection(const char *str, BOOL innerOnly) {
	//ostatnie wywolanie strtok dalo token, który jest pocz¹tkiem sekcji
	//utworz liste tokenow ze slownika i podmien referencje
	char *p;
	int i, pocz, posInCstore = 0, depth = 1;
	std::vector<char*> a;

	char lbound[3], rbound[3] = "";
	::StringCchCopyA(lbound, 3, strlen(str) <= 2 ? str : pdftok((char*)str)); // str jest otwarciem sekcji lub zlepkiem
	if (!strncmp(str, "<<", 2)) ::StringCchCopyA(rbound, 3, ">>");
	else if (!strncmp(str, "[", 1)) ::StringCchCopyA(rbound, 3, "]");

	a.push_back(lbound);
	do {
		p = pdftok(nullptr);
		a.push_back(p);
		if (!strcmp(p, "R")) {
			unsigned int srcObjNr = atoi(a[a.size() - 3]);
			auto it = renumMap.find(srcObjNr);
			if (it == renumMap.end()) {
				renumMap[srcObjNr] = nextObjNr;
				StringCchPrintfA(&cStore[posInCstore], bigSize, "%u", nextObjNr++);
			} else 
				StringCchPrintfA(&cStore[posInCstore], bigSize, "%i", abs((*it).second));
			a[a.size() - 3] = &cStore[posInCstore];
			posInCstore += 20;
		} 
		if (!strcmp(p, lbound)) depth++;
		if (!strcmp(p, rbound)) depth--;
	} while (p && depth>0);
	//przepisz slownik z nowymi numerami referencji
	pocz = 0;
	depth = (int)a.size();
	p = cStore + posInCstore;
	if (innerOnly) { depth--; pocz++; }
	for (i = pocz; i < depth; i++) {
		StringCchPrintfA(p, bigSize, "%s%s", a[i], i == depth - 1 ? "\x0a" : " ");
		trg.Write(p, (UINT)strlen(p));
	}
} //EmbedSection

// przepisuje definicjê klucza key wystêpuj¹cego najbli¿ej w ci¹gu buf z podmian¹ numerów referencji
void CManPDF::EmbedKey(const char *buf, char *key) {
	char *p,*de;

	de = strstr((char*)buf, ">>");
	if (!de) 
		throw CManPDFExc(_T("EmbedKey: Nie odnaleziono zakonczenia slownika ") + CString(key));
	p = strstr((char*)buf, key);
	if (!p)	return;

	trg.Write(key, (UINT)strlen(key)); trg.Write(" ", 1);
	p = pdftok(p);
	de = pdftok(nullptr);
	if (!strcmp(de, "<<") || !strcmp(de, "[")) 
		EmbedSection(de);
	else { // prosta referencja
		unsigned int refNr = atoi(de);
		pdftok(nullptr);
		if (strcmp(pdftok(nullptr), "R")) 
			throw CManPDFExc(_T("EmbedKey: Spodziewano sie prostej referencji ") + CString(key));
		uintmap::iterator it = renumMap.find(refNr);
		if (it == renumMap.end()) {
			renumMap[refNr] = nextObjNr;
			StringCchPrintfA(cStore, bigSize, "%u 0 R\x0a", nextObjNr++);
		} else
			StringCchPrintfA(cStore, bigSize, "%i 0 R\x0a", abs((*it).second));
		trg.Write(cStore, (UINT)strlen(cStore));
	}
} //EmbedKey

// analizuje obiekt wystêpuj¹cy po offset. Jeœli wystêpuje w nim stream to przepisuje go do pliku docelowego
unsigned long CManPDF::EmbedStream(CFile &src, unsigned long offset, BOOL decorate) {
	src.Seek(offset, CFile::begin);
	src.Read(cStore, bigSize);
	char *end = strstr(cStore, "endobj");
	char *str = strstr(cStore, "stream");
	if (!str || (end && end<str)) return 0L; // w przetwarzanym obiekcie nie wystêpuje stream

	char *p = strstr(cStore, "/Length");
	if (!p) 
		throw CManPDFExc(_T("EmbedStream: Nie odnaleziono tagu /Length"));

	pdftok(p);
	p = pdftok(nullptr);
	pdftok(nullptr);
	end = pdftok(nullptr);

	unsigned long len,ret;
	if (!strcmp(end, "R")) {
		src.Seek(FindObjEntry(src, atoi(p)), CFile::begin);
		src.Read(cStore, bigSize);
		strtok(cStore, sepline);
		ret = len = atol(pdftok(nullptr));
	} else
		ret = len = atol(p);
	if (decorate) len += 8;
	else str += 8;

	src.Seek(offset + (str - cStore), CFile::begin);
	unsigned int rB,wB = 1;
	while (len>0) {
		rB = src.Read(cStore, bigSize);
		if (rB < len && rB < bigSize) 
			throw CManPDFExc(_T("EmbedStream: nieoczekiwany koniec stream'a"));
		wB = len > rB ? rB : len;
		trg.Write(cStore, wB);
		len -= wB;
	}
	if (cStore[wB-1] > 13) trg.Write("\x0a", 1);
	if (decorate) trg.Write("endstream\x0a", 10);
	return ret;
} //EmbedStream

unsigned long CManPDF::EmbedPakStream(CFile &src, unsigned long offset, unsigned char *pak, unsigned char *niepak, unsigned char *tmp, const unsigned long niepakoff) {
	src.Seek(offset, CFile::begin);
	src.Read(cStore,bigSize);
	char *end = strstr(cStore, "endobj");
	char *str = strstr(cStore, "stream");
	if (!str || (end && end<str)) return 0L;
	if (!strstr(cStore, "/FlateDecode")) 
		throw CManPDFExc(_T("EmbedPakStream: spodziewano sie /FlateDecode"));
	char *p = strstr(cStore, "/Length");
	if (!p) 
		throw CManPDFExc(_T("EmbedPakStream: Nie odnaleziono tagu /Length"));
	pdftok(p);
	p = pdftok(nullptr);
	pdftok(nullptr);
	end = pdftok(nullptr);

	unsigned long paklen,niepaklen;
	if (!strcmp(end, "R")) {
		src.Seek(FindObjEntry(src, atoi(p)), CFile::begin);
		src.Read(cStore, bigSize);
		strtok(cStore, sepline);
		paklen = atol(pdftok(nullptr));
	} else
		paklen = atol(p);
	str += 8;

	src.Seek(offset + (str - cStore), CFile::begin);
	if (paklen > pakBufLen / 3) 
		throw CManPDFExc(_T("EmbedPakStream: Zbyt dlugi fragment contentu"));
	unsigned int pieceLen,pieceOff = 0;
	while (paklen) {
		pieceLen = (paklen > INT_MAX) ? INT_MAX : paklen;
		if (src.Read(&tmp[pieceOff],pieceLen) < pieceLen) 
			throw CManPDFExc(_T("EmbedPakStream: nieoczekiwany koniec stream'a"));
		pieceOff += pieceLen;
		paklen -= pieceLen;
	}

	niepaklen = pakBufLen;
#ifndef _WIN64
	if (uncompress(pak, &niepaklen, tmp, pieceOff)) 
		throw CManPDFExc(_T("EmbedPakStream: blad dekompresji"));
#endif

	memcpy(&niepak[niepakoff], pak, niepaklen);

	return niepakoff + niepaklen;
} //EmbedPakStream
		
// przebisuje obiekt srcObjNr z pliku Ÿród³owego pod nowy numer do pliku docelowego
void CManPDF::EmbedRef(CFile &src, unsigned int srcObjNr) {
	CString cs;
	unsigned long offset = FindObjEntry(src, srcObjNr);
	if (offset == 0L) {
		cs.Format(_T("%u"), srcObjNr);
		throw CManPDFExc("EmbedRef: Nie odnaleziono obiektu " + cs);
	}

	StringCchPrintfA(cStore, bigSize, "%i 0 obj %% old# %u\x0a", renumMap[srcObjNr], srcObjNr);
	trg.Write(cStore, (UINT)strlen(cStore));
	src.Seek(offset, CFile::begin);
	src.Read(cStore,bigSize);

	char *p,*end = strstr(cStore, "endobj");
	char *de = strstr(cStore, "<<");
	char *ae = strstr(cStore, "[");
	if (end && (!de || end<de) && (!ae || end<ae)) {
		p = strtok(cStore,sepline);
		p += strlen(cStore) + 1;
		trg.Write(p, (UINT)(end - p));
	} else {
		if (!de || (ae && ae<de)) de = ae;
		EmbedSection(de);
		EmbedStream(src, offset, TRUE);
	}
	trg.Write("endobj\x0a", 7);
} // EmbedRef

void CManPDF::EmbedContents(CFile &src, unsigned long offset) {
	CString cs;
	src.Seek(offset, CFile::begin);
	src.Read(cStore,bigSize);
	char* p = strstr(cStore, "/Contents");
	if (!p) 
		throw CManPDFExc(_T("EmbedContents: nie odnaleziono /Contents"));
	// goto /Contents
	BOOL multi = FALSE;
	unsigned int lenObjNr;
	char* de = strstr(p, "[");
	char* t = strstr(p, "R");
	if (de && t && de<t) {
		t = strstr(t + 1, "R");
		de = strstr(de, "]");
		multi = (t && de && t<de);
	}
	
	unsigned long loffset = FindObjEntry(src, GetRefNr(p));
	src.Seek(loffset, CFile::begin);
	src.Read(cStore,bigSize);
	p = strstr(cStore, "<<");
	if (!p) 
		throw CManPDFExc(_T("EmbedContents: nie odnaleziono pocz¹tku s³ownika <<"));
		
	if (multi) {
		p = pdftok(p); // skip token "<<"
		p = pdftok(nullptr);
		while (p && strcmp(p, "/Length")) {
			trg.Write(p, (UINT)strlen(p));
			trg.Write(" ", 1);
			p = pdftok(nullptr);
		} 
		if (!p) 
			throw CManPDFExc(_T("EmbedContents: nie odnaleziono /Length pierwszej czesci"));
		StringCchPrintfA(cStore, bigSize, "/Length %u 0 R\x0a", lenObjNr = nextObjNr++);
		trg.Write(cStore, (UINT)strlen(cStore));
		p = pdftok(nullptr);
		t = pdftok(nullptr);
		de = pdftok(nullptr);
		if (strcmp(de, "R")) {
			if (strcmp(t, ">>")) {
				StringCchPrintfA(cStore, bigSize, "%s %s ", t, de);
				trg.Write(cStore, (UINT)strlen(cStore));
			} else
				trg.Write(">>\x0a", 3);
		}
		p = pdftok(nullptr);
		while (p && strcmp(p, ">>")) {
			trg.Write(p, (UINT)strlen(p));
			trg.Write(" ", 1);
			p = pdftok(nullptr);
		}
		trg.Write("\x0a",1);
	} else
		EmbedSection(p, TRUE);
	trg.Write(">>\x0astream\x0a",10);
	unsigned long b = (unsigned long)trg.GetPosition();
	unsigned int lastR = 0;
	if (!multi)
		EmbedStream(src, loffset, FALSE);
	else {
		auto tmp = std::make_unique<unsigned char []>(pakBufLen / 3);
		auto pak = std::make_unique<unsigned char []>(pakBufLen + 1);
		auto niepak = std::make_unique<unsigned char []>(pakBufLen);
		if (!tmp || !pak || !niepak) 
			throw CManPDFExc(_T("EmbedContents: brak pamieci"));
		unsigned long pakLen, niepakOff = EmbedPakStream(src, loffset, pak.get(), niepak.get(), tmp.get(), 0L);
		while (multi) {
			src.Seek(offset, CFile::begin);
			src.Read(cStore, bigSize);
			p = strstr(cStore, "/Contents");
			if (p) {
				t = strstr(p + lastR + 1, "R");
				if (t) lastR = (unsigned int)(t - p);
				de = strstr(p, "]");
				if (de - t < 5) break;
				loffset = FindObjEntry(src, GetRefNr(t));
				niepakOff = EmbedPakStream(src, loffset, pak.get(), niepak.get(), tmp.get(), niepakOff);
			}
		}

		pakLen = pakBufLen;
#ifndef _WIN64
		if (compress(pak.get(), &pakLen, niepak.get(), niepakOff)) 
			throw CManPDFExc("EmbedContents: b³¹d kompresji: " + cs);
#endif

		unsigned int pieceLen, pieceOff = 0;
		if (pakLen && pak[pakLen-1] > '\x0a') 
			pak[pakLen++] = '\x0a';
		while (pakLen) {
			pieceLen = (pakLen > INT_MAX) ? INT_MAX : pakLen;
			trg.Write(&pak[pieceOff], pieceLen);
			pieceOff += pieceLen;
			pakLen -= pieceLen;
		}
	}
	b = (unsigned long)trg.GetPosition() - b ;
	trg.Write("endstream\x0a", 10);
	if (multi) {
		StringCchPrintfA(cStore, bigSize, "endobj\x0a%u 0 obj\n%lu \x0a", lenObjNr, b);
		trg.Write(cStore, (UINT)strlen(cStore));
	}
} //EmbedContents

//osadza w generowanym pliku og³oszenie pAdd, do którego materia³ jest w pliku src i przypisuje mu numer objNr
BOOL CManPDF::EmbedPDF(CFile &src, unsigned int objNr, CDrawAdd &pAdd) {
	StringCchPrintfA(cStore, bigSize, "%u 0 obj\x0a<<\x0a/Type /XObject\x0a/Subtype /Form\x0a/FormType 1\x0a", objNr);
	trg.Write(cStore, (UINT)strlen(cStore));
	// goto /Root
	xrefOffset = 0L;
	renumMap.clear();

	unsigned long offset = GetMediaBox(NULL, &bbx1, &bby1, &bbx2, &bby2, (HANDLE)src.m_hFile);
	if (offset == 0L) return FALSE;
	StringCchPrintfA(cStore, bigSize, "/BBox [ %.2f %.2f %.2f %.2f ]\x0a", bbx1, bby1, bbx2, bby2);
	trg.Write(cStore, (UINT)strlen(cStore));

	src.Seek(offset, CFile::begin);
	src.Read(cStore, bigSize);

	//ustal maciez dla ogloszenia
	auto pRozm = pAdd.m_pDocument->GetCRozm(pAdd.szpalt_x,pAdd.szpalt_y,pAdd.typ_xx);
	if (!pRozm) return FALSE;
	float px = (float)(mm2pkt * (pAdd.posx - 1) * (pRozm->w + pRozm->sw));
	float py = (float)(-1 * mm2pkt * (pAdd.posy + pAdd.sizey - 1) * (pRozm->h + pRozm->sh));
	float dimx = (float)((pAdd.sizex*(pRozm->w+pRozm->sw) - pRozm->sw)*mm2pkt); 
	float dimy = (float)((pAdd.sizey*(pRozm->h+pRozm->sh) - pRozm->sh)*mm2pkt);
	float sclx = (bbx2==bbx1 || pAdd.typ_xx!=0) ? 1 : dimx/(bbx2-bbx1);
	float scly = (bby2==bby1 || pAdd.typ_xx!=0) ? 1 : dimy/(bby2-bby1);

	if (pAdd.wersja.Find(_T("c")) >= 0) { //centrujemy
		px += (float)(0.5 * (dimx - bbx2 + bbx1));
		py += (float)(0.5 * (dimy - bby2 + bby1));
		StringCchPrintfA(cStore, bigSize, "/Matrix [ 1 0 0 1 %.2f %.2f ]\x0a", px, py);
	} else
		StringCchPrintfA(cStore, bigSize, "/Matrix [ %f 0 0 %f %.2f %.2f ]\x0a", sclx, scly, px, py);
	trg.Write(cStore, (UINT)strlen(cStore));

	src.Seek(offset, CFile::begin);
	src.Read(cStore, bigSize);
	EmbedKey(cStore, "/Resources");
	EmbedContents(src, offset);
	trg.Write("endobj\x0a", 7);
	for (auto it : renumMap) {
		EmbedRef(src, it.first);
		it.second *= -1;
	}
	int cc;
	do { // tyle obrotow jaka glebokosc referencji
		cc = 0;
		for (auto it : renumMap)
			if (it.second > 0) {
				EmbedRef(src, it.first);
				it.second *= -1;
				cc++;
			}
	} while (cc > 0);
	return TRUE;
} //EmbedPDF

BOOL CManPDF::GenProlog() {
	CStringA mbs;
	CString cs, fname;
	if (!cs.LoadString(IDS_INITVUPDF)) 
		throw CManPDFExc(_T("GenProlog: nie odnaleziono prologu"));
TRY {
	mbs = cs;
	trg.Write(mbs, mbs.GetLength());
	cs = trg.GetFilePath();
	for (int i = 0; i < cs.GetLength(); i++)
		fname += (cs[i] == '\\' ? "\\\\" : cs.Mid(i, 1));
	cs.Format(_T("/Creator (%s)\n/CreationDate (D:%s)\n/Title (%s)\n"), static_cast<const TCHAR*>(theManODPNET.m_userName), static_cast<const TCHAR*>(CTime::GetCurrentTime().Format("%Y%m%d%H%M%S")), static_cast<const TCHAR*>(fname));

	mbs = cs;
	trg.Write(mbs, mbs.GetLength());

	if (!cs.LoadString(IDS_INITVUPDF2)) 
		throw CManPDFExc(_T("GenProlog: nie odnaleziono prologu cz. 2"));
	mbs = cs;
	trg.Write(mbs, mbs.GetLength());
} CATCH(CFileException, e) {
	e->GetErrorMessage(theApp.bigBuf, DLGMSG_MAX_LEN);
	e->Delete();
	throw CManPDFExc(CString("GenProlog: ") + theApp.bigBuf);
} END_CATCH;
	return TRUE;
} // GenProlog

void CManPDF::GenPDFTail(unsigned int howMany) {
	std::vector<CString> offsetArr;
	BOOL noLimit = howMany == 0;
		
	unsigned int objnr = 0;
	unsigned long offset;
	CString objdef;
	trg.SeekToBegin();
	while (noLimit || objnr < howMany) {
		StringCchPrintfA(cStore, bigSize, "%u 0 obj", ++objnr);
		if ((offset = SearchPattern(trg, cStore)) == -1L) {
			if (noLimit) break;
			objdef.Format(_T("%010ld 00000 f \x0a"), 0L);
		} else 
			objdef.Format(_T("%010lu 00000 n \x0a"), offset);

		trg.SeekToBegin();
		offsetArr.emplace_back(objdef);
	}
	objnr--;

	trg.SeekToBegin();
	long trailer = SearchPattern(trg, "trailer");
	long traillen = SearchPattern(trg, "startxref") - trailer;

	trg.SeekToBegin();
	unsigned long initxref = SearchPattern(trg, "xref");
	if (initxref == -1L) 
		initxref = (unsigned long)trg.SeekToEnd();
	else
		trg.Seek(initxref,CFile::begin);
	StringCchPrintfA(cStore, bigSize, "xref\n0 %u\n0000000000 65535 f \x0a", objnr + 1);
	trg.Write(cStore, (UINT)strlen(cStore));

	for (unsigned int i = 0; i < objnr; i++) 
		trg.Write(CStringA(offsetArr[i]), 20);

	if (traillen) { //popraw /Size
		offset = (unsigned long)trg.GetPosition();
		trg.Seek(trailer,CFile::begin);
		trg.Read(cStore,traillen);
		trg.Seek(offset,CFile::begin);
		cStore[traillen] = '\0';
		char *sizetag = strstr(cStore, "/Size ");
		if (!sizetag) 
			throw CManPDFExc(_T("GenPDFTail: nie odnaleziono tagu /Size"));
		sizetag[5] = '\0';
		sizetag += 6;
		trg.Write(cStore, (UINT)(sizetag - cStore - 1));
		StringCchPrintfA(cStore, bigSize, " %u\n", objnr + 1);
		trg.Write(cStore, (UINT)strlen(cStore));
		char *tail = strstr(sizetag, "/");
		if (!tail) tail = strstr(sizetag, ">>");
		if (!tail) 
			throw CManPDFExc(_T("GenPDFTail: nie odnaleziono tagu >>"));
		trg.Write(tail, (UINT)strlen(tail));
	} else {
		StringCchPrintfA(cStore, bigSize, "trailer\n<</Size %u\n/Root 1 0 R\n/Info 2 0 R\n>>\n", objnr + 1);
		trg.Write(cStore, (UINT)strlen(cStore));
	}

	StringCchPrintfA(cStore, bigSize, "startxref\n%lu\n%%%%EOF\n", initxref);
	trg.Write(cStore, (UINT)strlen(cStore));
	trg.SetLength(trg.GetPosition());
} //GenPDFTail

BOOL CManPDF::CreatePDF(CDrawPage *page, const TCHAR *trgName) {
	CFile src;
	std::vector<CString> embAlias,filePath;
try {
	trg.Open(trgName, CFile::modeCreate | CFile::modeReadWrite | CFile::typeBinary | CFile::shareExclusive, &fileErr);
	if (fileErr.m_cause) {
		fileErr.GetErrorMessage(theApp.bigBuf, DLGMSG_MAX_LEN);
		throw CManPDFExc(CString(_T("CreatePDF: ")) + theApp.bigBuf);
	}
	
	if (!GenProlog()) return FALSE;

	CString cs, fname;
	StringCchPrintfA(cStore, bigSize, "%u 0 obj\x0a<<\x0a", nextObjNr++);
	trg.Write(cStore, (UINT)strlen(cStore));
	unsigned int kon, pocz = nextObjNr;
	kon = pocz + (unsigned int)page->m_adds.size();

	nextObjNr = kon;
	CDrawAdd *pAdd;
	// lista XObjectow
	unsigned int i,dziury = 0;
	for (i = pocz; i < kon; i++) {
		pAdd = page->m_adds[i - pocz];
		BOOL copyEps = theApp.GetProfileInt(_T("GenEPS"), _T("CopyOldEPS"), 0);
		fname = pAdd->EpsName(F_PDF, FALSE);
		filePath.emplace_back(fname);
		if (fname.Mid(1, 1) != _T(":")) {
			embAlias.emplace_back("");
			nextObjNr--;
			dziury++;
			continue;
		} 
		int pos = fname.ReverseFind('\\');
		if (pos >= 0) fname = fname.Mid(pos+1);
		pos = fname.ReverseFind('.');
		if (pos >= 0) fname = fname.Left(pos);
		embAlias.emplace_back("/" + fname);
		StringCchPrintfA(cStore, bigSize, "/%s %u 0 R\x0a", fname, i - dziury);
		trg.Write(cStore, (UINT)strlen(cStore));
	}
	trg.Write(">>\x0a", 3); trg.Write("endobj\x0a", 7);
	// osadx PDFy
	dziury = 0;
	for (i = pocz; i < kon; i++) {
		if (embAlias[i-pocz].IsEmpty()) {
			dziury++;
			continue;
		}
		pAdd = page->m_adds[i - pocz];
		dlg->OglInfo(0, "przetwarzanie pliku: " + filePath[i-pocz]);
		src.Open(filePath[i-pocz], CFile::modeReadWrite | CFile::typeBinary, &fileErr);
		if (fileErr.m_cause) {
			if (fileErr.m_cause != CFileException::fileNotFound) { //jak 
				fileErr.GetErrorMessage(theApp.bigBuf, DLGMSG_MAX_LEN);
				throw CManPDFExc(CString("CreatePDF: ") + theApp.bigBuf);
			}
			continue;
		} 
		EmbedPDF(src, i - dziury, *pAdd);
		src.Close();
	}
	// narysuj PDFy
	trg.Write("5 0 obj % Page Contents\012", 24);
	StringCchPrintfA(cStore, bigSize, "<< %s/Length %u 0 R >>\012stream\012", (compressContent ? "/Filter /FlateDecode " : ""), nextObjNr);
	trg.Write(cStore, (UINT)strlen(cStore));
	long contentLen = 0L;
	StringCchPrintfA(cStore, bigSize, "q\0121 0 0 1 %i %i cm\012", orgX, orgY);
	contentLen += (UINT)strlen(cStore);
	
	for (i = pocz; i < kon; i++) {
		pAdd = page->m_adds[i - pocz];
		auto pRozm = pAdd->m_pDocument->GetCRozm(pAdd->szpalt_x, pAdd->szpalt_y, pAdd->typ_xx);
		float px = (float)(mm2pkt * ((pAdd->posx + pAdd->sizex - 1) * (pRozm->w + pRozm->sw) - pRozm->sw));
		float py = (float)(-1 * mm2pkt * (pAdd->posy + pAdd->sizey - 1) * (pRozm->h + pRozm->sh) - podpisH);
		fname.Format(_T("%li%s"), pAdd->nreps, pAdd->wersja);	
		// pdf ogloszenia
		if (!embAlias[i - pocz].IsEmpty()) {
			cs.Format(_T("%s Do\012"), embAlias[i - pocz]);
			::StringCchCopyA(&cStore[contentLen], n_size - contentLen, CStringA(cs));
		} else {
			fname = "Nie odnaleziono pliku: " + fname;
			float w = (float)(mm2pkt * (pAdd->sizex * (pRozm->w + pRozm->sw) - pRozm->sw));
			float h = (float)(mm2pkt * (pAdd->sizey * (pRozm->h + pRozm->sh) - pRozm->sh));
			cs.Format(_T("q %.2f %.2f %.2f %.2f re S Q\012"), px, py + podpisH, -w, h);
			::StringCchCopyA(&cStore[contentLen], n_size - contentLen, CStringA(cs));
		}
		contentLen += cs.GetLength();

		// podpis
		CStringA csA(cs);
		EmbedTextRight("/FVU", 6, px, py, CStringA(fname), csA);
		::StringCchCopyA(&cStore[contentLen], n_size - contentLen, csA);
		contentLen += csA.GetLength();
	}
	::StringCchCopyA(&cStore[contentLen], n_size - contentLen, "Q");
	contentLen++;
	if (compressContent) {
		auto contentBuf = std::make_unique<unsigned char []>(bigSize);
		if (!contentBuf)
			throw CManPDFExc(_T("CreatePDF: b³¹d pamiêci przy pakowaniu contentu"));
		unsigned long initLen = contentLen,pakLen = bigSize;
#ifndef _WIN64
		if(compress(contentBuf.get(), &pakLen, (unsigned char*)cStore,contentLen))
			throw CManPDFExc(_T("CreatePDF: b³¹d pakowania contentu"));
#endif
		trg.Write(contentBuf.get(), pakLen);
	} else
		trg.Write(cStore, contentLen);
	trg.Write("\012endstream\012endobj\012", 18);
	// dodaj pagine
	//
	// dlugosc contentu
	StringCchPrintfA(cStore, bigSize, "%u 0 obj\x0a%li%cendobj\x0a", nextObjNr++, --contentLen, 10);
	trg.Write(cStore, (UINT)strlen(cStore));
	// wygeneruj tail
	GenPDFTail();
	trg.Close();
} catch(CManPDFExc &e) {
	AfxMessageBox(e.ShowReason(), MB_ICONSTOP);
	return FALSE;
}
	return TRUE;
} //CreatePDF

unsigned long CManPDF::GetMediaBox(const TCHAR *fpath, float *x1, float *y1, float *x2, float *y2, HANDLE hFile) {
	// zwraca offset do /Page
	CFile src(hFile);
	char *p,*t,*de;
	unsigned long offset;
try {
	if(fpath && src.Open(fpath, CFile::modeReadWrite | CFile::typeBinary, &fileErr) && fileErr.m_cause) 
		throw CManPDFExc(_T("GetMediaBox: blad otwarcia pliku"));

	offset = GetMainXref(src);
	if (offset == 0L) 
		throw CManPDFExc(_T("GetMediaBox: nie odnaleziono xref"));
	src.Seek(offset, CFile::begin);
	src.Read(cStore,bigSize);
	p = strstr(cStore, "/Root");
	if (!p) 
		throw CManPDFExc(_T("GetMediaBox: nie odnaleziono /Root"));
	strtok(p, septok);
	offset = FindObjEntry(src, atoi(strtok(NULL, septok)));

	src.Seek(offset, CFile::begin);
	src.Read(cStore,bigSize);
	// goto /Pages
	p = strstr(cStore, "/Pages");
	if (!p) 
		throw CManPDFExc(_T("GetMediaBox: nie odnaleziono /Pages"));

	offset = FindObjEntry(src, GetRefNr(p));
	src.Seek(offset, CFile::begin);
	src.Read(cStore, bigSize);	
	t = strstr(cStore, "/MediaBox");
	de = strstr(cStore, ">>");
	if (t && (!de || t<de)) goto readMediaBox;

	// goto /Kids
	p = strstr(cStore, "/Kids");
	if (!p) 
		throw CManPDFExc(_T("GetMediaBox: nie odnaleziono /Kids"));
	offset = FindObjEntry(src, GetRefNr(p));
	src.Seek(offset, CFile::begin);
	src.Read(cStore, bigSize);

	t = strstr(cStore, "/MediaBox");
	de = strstr(cStore, ">>");
	if(!t || (de && de<t)) return FALSE;
readMediaBox:	
	pdftok(t);
	t = pdftok(nullptr);
	if (!strcmp(t, "["))
		*x1 = (float)atof(strtok(NULL, septok));
	else {
		int i = 0;
		while (!isdigit((int)t[i]) && t[i])
			t[i++] = ' ';
		if (!t[i]) 
			throw CManPDFExc(_T("GetMediaBox: uszkodzony tag /MediaBox"));
		*x1 = (float)atof(t);
	}
	*y1 = (float)atof(pdftok(nullptr));
	*x2 = (float)atof(pdftok(nullptr));
	*y2 = (float)atof(pdftok(nullptr));
kids:
	// in /Page object
	src.Seek(offset, CFile::begin);
	src.Read(cStore, bigSize);
	p = strstr(cStore, "/Kids");
	if (p && p<strstr(cStore, ">>")) {
		offset = FindObjEntry(src, GetRefNr(p));
		src.Seek(offset, CFile::begin);
		src.Read(cStore, bigSize);
		goto kids;	
	}
	if (fpath) src.Close();
} catch(CManPDFExc &e) {
	AfxMessageBox(e.ShowReason(), MB_ICONSTOP);
	return 0L;
}
	return offset;
} //GetMediaBox

inline void CManPDF::EmbedTextRight(const char* font, unsigned int fsize, float px, float py, const char* text, CStringA& buf) {
	buf.Format("q BT 1 g\012%s %u Tf\0121 0 0 1 %f %f cm\012-100 Tz (%s) Tj 100 Tz 0 g (%s) Tj\012ET Q\012", font, fsize, px, py, text, text);
}
