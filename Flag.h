/*********************************************************************************
**                                                                              **
**	(c)	Marcin Buchwald										wrzesieñ 1999       **
**                                                                              **
**	CFlag jest rozszerzeniem typu long wykorzystywanego jako flaga 32bitowa.    **
**	Pole danych obiektu CFlag jest 8*size bitowe.                               **
**	Je¿eli rozmiar flagi jest niewiêkszy od CRITICAL_SIZE pamiêæ alokowana		**
**	jest statycznie, w przeciwnym przypadku alokujemy dynamicznie.              **
**                                                                              **
*********************************************************************************/

#pragma once

class CFlag final
{
public:
    CFlag(const CFlag& f);                              // copy constructor
    CFlag(CFlag&& f) noexcept;                          // move constructor
    explicit CFlag(size_t s = CRITICAL_SIZE);           // default constructor, how many bytes to store
    CFlag(int sx, int sy, int szpalt_x, int szpalt_y);  // SetSpace constructor, how many modules to store
    CFlag(const char *raw);                             // Oracle RAW hexadecimal format
    CFlag(const wchar_t *raw) : CFlag(CStringA(raw)){}; // Oracle RAW hexadecimal format unicode encoded
    CFlag(CByteArray& bArr);
    ~CFlag();
    const CFlag& operator= (const CFlag& f);
    const CFlag& operator= (CFlag&& f) noexcept;        // move assignment
    CFlag& operator>>=(size_t shift) noexcept;
    CFlag& operator<<=(size_t shift) noexcept;
    CFlag  operator >> (size_t shift) const noexcept;
    CFlag  operator<< (size_t shift) const noexcept;
    CFlag operator| (const CFlag& f) const noexcept;
    CFlag operator& (const CFlag& f) const noexcept;
    CFlag  operator^ (const CFlag& f) const noexcept;
    uintptr_t operator|| (const CFlag& f) const noexcept;
    uintptr_t operator&& (const CFlag& f) const noexcept;
    uintptr_t operator& (uintptr_t mask) const noexcept;
    uintptr_t operator! () const noexcept;
    void operator|= (const CFlag& f) noexcept;
    void operator|= (uintptr_t mask) noexcept;
    void   operator&= (const CFlag& f) noexcept;
    void   operator&= (uintptr_t mask) noexcept;
    void   operator^= (const CFlag& f) noexcept;
    void   operator^= (uintptr_t mask) noexcept;
    int operator== (const CFlag& f) const noexcept;
    int operator!= (const CFlag& f) const noexcept;
    bool   IsZero() const noexcept;
    bool   IsSet() const noexcept;
    size_t GetSize() const noexcept;
    CFlag& Invert() noexcept;
    void   SetZero() noexcept;
    void   Reverse(size_t len) noexcept;
    void   CopyFlag(CByteArray *bArr);
    void   Serialize(CArchive& ar);
    CString Print() const;
    CString ToRaw() const;
    int GetBitCnt(unsigned char val) const noexcept;
    unsigned char GetBit(size_t pos) const noexcept; // bity numerowane w porz¹dku [8*size-1..0]
    void SetBit(size_t pos, unsigned char bit = 1) noexcept;

protected:
    void SetSize(size_t s);                                 // okreœla rozmiar flagi, zarz¹dza pamiêci¹
    char* GetRawFlag() const noexcept;                      // wskaŸnik do buforu z danymi flagi - do wewnêtrznych manipulacji

private:
    static const size_t DBRAW_BLOCK = 4;                    // wielkoœc logicznej paczki bajtów wymienianych z baz¹ danych
    static const size_t RAW_MOD_SIZE = 2 * DBRAW_BLOCK;     // d³ugoœc napisu RAW definiuj¹cego atomow¹ paczkê bajtów z bazy
    static const size_t CRITICAL_SIZE = sizeof(uintptr_t);  // do ilu bajtów operacje na fladze mo¿na robiæ wprost na rejestrach

    size_t size;                                            // flag size in bytes
    union
    {
        uintptr_t flag;                                     // if (size > CRITICAL_SIZE) uintptr_t->char*
        void     *flagblob_ptr;                             // ASSERT(size % CRITICAL_SIZE == 0)
    };
};
