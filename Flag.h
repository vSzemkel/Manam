/*********************************************************************************
**                                                                              **
**	(c)	Marcin Buchwald										wrzesie� 1999       **
**                                                                              **
**	CFlag jest rozszerzeniem typu long wykorzystywanego jako flaga 32bitowa.    **
**	Pole danych obiektu CFlag jest 8*size bitowe.                               **
**	Je�eli rozmiar flagi jest niewi�kszy od CRITICAL_SIZE pami�� alokowana		**
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
    explicit CFlag(const char *raw);                             // Oracle RAW hexadecimal format
    explicit CFlag(const wchar_t *raw) : CFlag(CStringA(raw)){}; // Oracle RAW hexadecimal format unicode encoded
    explicit CFlag(const CByteArray& bArr);
    ~CFlag();
    const CFlag& operator= (const CFlag& f);
    const CFlag& operator= (CFlag&& f) noexcept;        // move assignment
    CFlag& operator>>=(size_t shift) noexcept;
    CFlag& operator<<=(size_t shift) noexcept;
    CFlag  operator>> (size_t shift) const noexcept;
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
    bool operator[](size_t pos) const noexcept;
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
    int GetBitCnt(bool val) const noexcept;
    void SetBit(size_t pos, bool val = true) noexcept;

protected:
    void SetSize(size_t s);                                 // okre�la rozmiar flagi, zarz�dza pami�ci�
    char* GetRawFlag() const noexcept;                      // wska�nik do buforu z danymi flagi - do wewn�trznych manipulacji

private:
    static const size_t DBRAW_BLOCK = 4;                    // wielko�c logicznej paczki bajt�w wymienianych z baz� danych
    static const size_t RAW_MOD_SIZE = 2 * DBRAW_BLOCK;     // d�ugo�c napisu RAW definiuj�cego atomow� paczk� bajt�w z bazy
    static const size_t CRITICAL_SIZE = sizeof(uintptr_t);  // do ilu bajt�w operacje na fladze mo�na robi� wprost na rejestrach

    size_t size;                                            // flag size in bytes
    union
    {
        uintptr_t flag;                                     // if (size > CRITICAL_SIZE) uintptr_t->char*
        void     *flagblob_ptr;                             // ASSERT(size % CRITICAL_SIZE == 0)
    };
};
