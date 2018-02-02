
/*
**	CFlag jest rozszerzeniem typu int_ptr wykorzystywanego jako flaga bitowa
**	Pole danych obiektu CFlag jest 8*size bitowe
*/

#include "StdAfx.h"
#include "Flag.h"

CFlag::CFlag(const CFlag& f)
{
    size = f.size;
    if (size > CRITICAL_SIZE) {
        flagblob_ptr = malloc(size);
        memcpy(flagblob_ptr, f.flagblob_ptr, size);
    } else
        flag = f.flag;
}

CFlag::CFlag(CFlag&& f) noexcept
{
    size = f.size;
    flag = f.flag;
    if (f.size > CRITICAL_SIZE)
        f.size = CRITICAL_SIZE;
}

CFlag::CFlag(size_t s)
{
    if (s & (CRITICAL_SIZE - 1))
        s = (s & (UINT_MAX - CRITICAL_SIZE + 1)) + CRITICAL_SIZE;
    size = s;
    if (size > CRITICAL_SIZE)
        flagblob_ptr = malloc(size);
    SetZero();
}

CFlag::CFlag(int sx, int sy, int szpalt_x, int szpalt_y)
{
    ASSERT(0 <= sx && sx <= szpalt_x && 0 <= sy && sy <= szpalt_y);
    int i = szpalt_x * szpalt_y;
#ifdef _WIN64
    size = CRITICAL_SIZE * ((i >> 6) + ((i & 0x3F) > 0 ? 1 : 0));
#else
    size = CRITICAL_SIZE * ((i >> 5) + ((i & 0x1F) > 0 ? 1 : 0));
#endif

    if (size > CRITICAL_SIZE)
        flagblob_ptr = malloc(size);

    SetZero();
    if (size <= CRITICAL_SIZE) {
        uintptr_t tmp = (1 << sx) - 1;
        for (i = 0; i < sy; ++i)
            flag = (flag << szpalt_x) + tmp;
    } else {
        for (i = 0; i < sx; ++i)
            SetBit(i);
        CFlag tmp(*this);
        for (i = 1; i < sy; ++i)
            *this = (*this << szpalt_x) | tmp;
    }
}

CFlag::CFlag(const char *raw) : size(CRITICAL_SIZE)
{
    size_t len = strlen(raw);
    if (len == 0)
        return;

    if (!(len & 7) && strspn(raw, "0123456789ABCDEFabcdef") == len) { // ciag 4n dwuznakowych batow
        size = len >> 1;
#ifdef _WIN64
        if (size & 0x04)
            size += 4;
#endif
        if (size > CRITICAL_SIZE) {
            int32_t l;
            char buf[RAW_MOD_SIZE + 1];
            flagblob_ptr = malloc(size);
            buf[RAW_MOD_SIZE] = '\0';
            for (size_t i = 0; DBRAW_BLOCK * i < size; ++i) {
                memcpy(buf, &raw[len - RAW_MOD_SIZE * (i + 1)], RAW_MOD_SIZE);
                sscanf_s(buf, "%lx", &l);
                memcpy((char *)flagblob_ptr + DBRAW_BLOCK * i, (const char *)&l, DBRAW_BLOCK);
            }
        } else
            sscanf_s(raw, "%Ix", &flag);
    } else { // ciag binarny
        SetSize(len);
        ::StringCchCopyA(GetRawFlag(), size, raw);
    }
}

CFlag::CFlag(CByteArray& bArr)
{
    size = bArr.GetSize();
    if (size > CRITICAL_SIZE) {
        flagblob_ptr = malloc(size);
        memcpy(flagblob_ptr, bArr.GetData(), size);
    } else {
        flag = 0;
        auto buf = reinterpret_cast<BYTE*>(&flag);
        for (size_t i = 0; i < size; i++)
            buf[i] = bArr[i];
    }
}

CFlag::~CFlag()
{
    if (size > CRITICAL_SIZE)
        free(flagblob_ptr);
}

uintptr_t CFlag::operator! () const noexcept
{ // the same as IsZero()
    if (size > CRITICAL_SIZE) {
        int iszero = TRUE;
        auto src = (uintptr_t*)flag;
        const size_t blocks = size / CRITICAL_SIZE;
        for (size_t i = 0; i < blocks; ++i)
            iszero &= !src[i];
        return iszero;
    } else
        return !flag;
}

const CFlag& CFlag::operator= (const CFlag& f)
{
    SetSize(f.size);
    if (size > CRITICAL_SIZE)
        memcpy(flagblob_ptr, f.flagblob_ptr, size);
    else
        flag = f.flag;

    return *this;
}

const CFlag& CFlag::operator= (CFlag&& f) noexcept
{
    if (this != &f) {
        if (size > CRITICAL_SIZE)
            free(flagblob_ptr);
        size = f.size;
        flag = f.flag;
        if (f.size > CRITICAL_SIZE)
            f.size = CRITICAL_SIZE;
    }

    return *this;
}

CFlag CFlag::operator& (const CFlag& f) const noexcept
{
    ASSERT(size == f.size);
    CFlag ret{size};
    if (size > CRITICAL_SIZE) {
        auto src1 = (uintptr_t*)flag;
        auto src2 = (uintptr_t*)f.flag;
        auto dst = (uintptr_t*)ret.flag;
        const size_t blocks = size / CRITICAL_SIZE;
        for (size_t i = 0; i < blocks; ++i)
            dst[i] = src1[i] & src2[i];
    } else
        ret.flag = flag & f.flag;

    return ret;
}

CFlag CFlag::operator| (const CFlag& f) const noexcept
{
    ASSERT(size == f.size);
    CFlag ret{size};
    if (size > CRITICAL_SIZE) {
        auto src1 = (uintptr_t*)flag;
        auto src2 = (uintptr_t*)f.flag;
        auto dst = (uintptr_t*)ret.flag;
        const size_t blocks = size / CRITICAL_SIZE;
        for (size_t i = 0; i < blocks; ++i)
            dst[i] = src1[i] | src2[i];
    } else
        ret.flag = flag | f.flag;

    return ret;
}

CFlag CFlag::operator^ (const CFlag& f) const noexcept
{
    ASSERT(size == f.size);
    CFlag ret{size};
    if (size > CRITICAL_SIZE) {
        auto src1 = (uintptr_t*)flag;
        auto src2 = (uintptr_t*)f.flag;
        auto dst = (uintptr_t*)ret.flag;
        const size_t blocks = size / CRITICAL_SIZE;
        for (size_t i = 0; i < blocks; ++i)
            dst[i] = src1[i] ^ src2[i];
    } else
        ret.flag = flag ^ f.flag;

    return ret;
}

CFlag& CFlag::Invert() noexcept
{
    if (size > CRITICAL_SIZE) {
        auto src = (uintptr_t*)flag;
        const size_t blocks = size / CRITICAL_SIZE;
        for (size_t i = 0; i < blocks; ++i)
            src[i] ^= static_cast<intptr_t>(-1);
    } else
        flag ^= static_cast<intptr_t>(-1);

    return *this;
}

void CFlag::operator|= (const CFlag& f) noexcept
{
    ASSERT(size == f.size);
    if (size > CRITICAL_SIZE) {
        auto src = (uintptr_t*)f.flag;
        auto dst = (uintptr_t*)flag;
        const size_t blocks = size / CRITICAL_SIZE;
        for (size_t i = 0; i < blocks; ++i)
            dst[i] |= src[i];
    } else
        flag |= f.flag;
}

void CFlag::operator|= (uintptr_t mask) noexcept
{
    if (size > CRITICAL_SIZE) {
        auto dst = (uintptr_t*)flag;
        const size_t blocks = size / CRITICAL_SIZE;
        for (size_t i = 0; i < blocks; ++i)
            dst[i] |= mask;
    } else
        flag |= mask;
}

uintptr_t CFlag::operator& (uintptr_t mask) const noexcept
{
    if (size > CRITICAL_SIZE) {
        auto dst = (uintptr_t*)flag;
        return dst[0] & mask;
    } return flag & mask;
}

void CFlag::operator&= (uintptr_t mask) noexcept
{
    if (size > CRITICAL_SIZE) {
        auto dst = (uintptr_t*)flag;
        const size_t blocks = size / CRITICAL_SIZE;
        for (size_t i = 0; i < blocks; ++i)
            dst[i] &= mask;
    } else
        flag &= mask;
}

void CFlag::operator&= (const CFlag& f) noexcept
{
    ASSERT(size == f.size);
    if (size > CRITICAL_SIZE) {
        auto src = (uintptr_t*)f.flag;
        auto dst = (uintptr_t*)flag;
        const size_t blocks = size / CRITICAL_SIZE;
        for (size_t i = 0; i < blocks; ++i)
            dst[i] &= src[i];
    } else
        flag &= f.flag;
}

void CFlag::operator^= (const CFlag& f) noexcept
{
    ASSERT(size == f.size);
    if (size > CRITICAL_SIZE) {
        auto src = (uintptr_t*)f.flag;
        auto dst = (uintptr_t*)flag;
        const size_t blocks = size / CRITICAL_SIZE;
        for (size_t i = 0; i < blocks; ++i)
            dst[i] ^= src[i];
    } else
        flag ^= f.flag;
}

void CFlag::operator^= (uintptr_t mask) noexcept
{
    if (size > CRITICAL_SIZE) {
        auto dst = (uintptr_t*)flag;
        const size_t blocks = size / CRITICAL_SIZE;
        for (size_t i = 0; i < blocks; ++i)
            dst[i] ^= mask;
    } else
        flag ^= mask;
}

CFlag CFlag::operator >> (size_t shift) const noexcept
{
    CFlag ret(*this);
    if (size > CRITICAL_SIZE) {
        if (shift >= 8 * size) ret.SetZero();
        else {
            size_t i;
            for (i = 0; i < 8 * size - shift; ++i)
                ret.SetBit(i, operator[](i + shift));
            for (i = 0; i < shift; ++i)
                ret.SetBit(8 * size - 1 - i, 0);
        }
    } else
        ret.flag = (flag >> shift);

    return ret;
}

CFlag CFlag::operator<< (size_t shift) const noexcept
{
    CFlag ret(*this);
    if (size > CRITICAL_SIZE) {
        if (shift >= 8 * size) ret.SetZero();
        else {
            size_t i;
            for (i = 8 * size - 1 - shift; i != (size_t)-1; --i)
                ret.SetBit(i + shift, operator[](i));
            for (i = 0; i < shift; i++)
                ret.SetBit(i, 0);
        }
    } else
        ret.flag = (flag << shift);

    return ret;
}

CFlag& CFlag::operator>>= (size_t shift) noexcept
{
    if (size > CRITICAL_SIZE) {
        if (shift >= 8 * size) SetZero();
        else {
            size_t i;
            for (i = 0; i < 8 * size - shift; ++i)
                SetBit(i, operator[](i + shift));
            for (i = 0; i < shift; ++i)
                SetBit(8 * size - 1 - i, 0);
        }
    } else
        flag >>= shift;

    return *this;
}

CFlag& CFlag::operator<<= (size_t shift) noexcept
{
    if (size > CRITICAL_SIZE) {
        if (shift >= 8 * size) SetZero();
        else {
            size_t i;
            for (i = 8 * size - 1 - shift; i != (size_t)-1; --i)
                SetBit(i + shift, operator[](i));
            for (i = 0; i < shift; ++i)
                SetBit(i, 0);
        }
    } else
        flag <<= shift;

    return *this;
}

uintptr_t CFlag::operator|| (const CFlag& f) const noexcept
{
    if (size > CRITICAL_SIZE) {
        return (*this | f).IsSet();
    } return flag || f.flag;
}

uintptr_t CFlag::operator&& (const CFlag& f) const noexcept
{
    if (size > CRITICAL_SIZE) {
        return (*this & f).IsSet();
    } return flag && f.flag;
}

int CFlag::operator== (const CFlag& f) const noexcept
{
    ASSERT(size == f.size);
    if (size > CRITICAL_SIZE) {
        return !memcmp(flagblob_ptr, f.flagblob_ptr, size);
    } return flag == f.flag;
}

int CFlag::operator!= (const CFlag& f) const noexcept
{
    ASSERT(size == f.size);
    if (size > CRITICAL_SIZE) {
        return memcmp(flagblob_ptr, f.flagblob_ptr, size);
    } return flag != f.flag;
}

void CFlag::SetZero() noexcept
{
    if (size > CRITICAL_SIZE)
        memset(flagblob_ptr, 0, size);
    else
        flag = 0;
}

bool CFlag::IsZero() const noexcept
{
    if (size > CRITICAL_SIZE) {
        bool iszero = true;
        const auto src = (uintptr_t*)flag;
        const size_t blocks = size / CRITICAL_SIZE;
        for (size_t i = 0; i < blocks; ++i)
            iszero &= !src[i];
        return iszero;
    } return flag == 0;
}

bool CFlag::IsSet() const noexcept
{
    if (size > CRITICAL_SIZE) {
        bool iszero = true;
        const auto src = (uintptr_t*)flag;
        const size_t blocks = size / CRITICAL_SIZE;
        for (size_t i = 0; i < blocks; ++i)
            iszero &= !src[i];
        return !iszero;
    } return flag != 0;
}

size_t CFlag::GetSize() const noexcept
{
    return size;
}

void CFlag::SetSize(size_t s)
{
    if (size != s) {
        if (size > CRITICAL_SIZE) free(flagblob_ptr);
        if (s > CRITICAL_SIZE) flagblob_ptr = malloc(s);
        size = s;
    }
}

void CFlag::CopyFlag(CByteArray *bArr)
{
    bArr->SetSize(size);
    char *str = GetRawFlag();
    for (size_t i = 0; i < size; ++i, str++)
        bArr->SetAt(i, *str);
}

void CFlag::Reverse(size_t len) noexcept
{
    if (len <= 8 * size)
        for (size_t i = 0; i < len / 2; ++i) {
            auto bit = operator[](i);
            SetBit(i, operator[](len - i - 1));
            SetBit(len - i - 1, bit);
        }
}

void CFlag::Serialize(CArchive& ar)
{
    if (ar.IsStoring()) {
        ar << (WORD)size;
        ar.Write(GetRawFlag(), (UINT)size);
    } else {
        WORD storedSize;
        ar >> storedSize;
        const auto sizeToRead = (UINT)min(size, storedSize); // truncate read
        ar.Read(GetRawFlag(), sizeToRead);
        const auto pad = storedSize - (int)sizeToRead;
        if (pad > 0) { // compensate read
            QWORD devnull;
            ar.Read(&devnull, (UINT)pad);
        }
    }
}

int CFlag::GetBitCnt(bool val) const noexcept
{
    int iIle = 0;
    const auto bit_cnt = 8 * size;
    for (size_t i = 0; i < bit_cnt; ++i)
        if (operator[](i) == val)
            iIle++;
    return iIle;
}

bool CFlag::operator[](size_t pos) const noexcept
{
    ASSERT(0 <= pos && pos < 8 * size);
    const unsigned char mask = (1 << (pos % 8));
    char *str = GetRawFlag();
    str += (pos / 8);
    return *str & mask;
}

void CFlag::SetBit(size_t pos, bool val) noexcept
{
    const unsigned char mask = (1 << (pos % 8));
    char *str = GetRawFlag();
    str += (pos / 8);
    *str = (val ? *str | mask : *str & (0xff - mask));
}

CString CFlag::Print() const
{
    const int bit_cnt = (int)size * 8;
    CString display{'0', bit_cnt};

    for (auto i = 0; i < bit_cnt; ++i)
        if (operator[](i))
            display.SetAt(i, '1');

    return display.MakeReverse();
}

CString CFlag::ToRaw() const
{
    TCHAR buf[9];
    CString raw("");
    if (size > CRITICAL_SIZE) {
        int32_t l;
        char *str = (char*)flag;
        for (size_t i = 0; i < size; i += DBRAW_BLOCK) {
            memcpy((char*)&l, &str[i], DBRAW_BLOCK);
            ::StringCchPrintf(buf, 9, _T("%08lx"), l);
            raw = buf + raw;
        }
    } else {
        ::StringCchPrintf(buf, 9, _T("%%0%IiIx"), 2 * size);
        raw.Format(buf, flag);
    }
    return raw;
}

char* CFlag::GetRawFlag() const noexcept
{
    return size > CRITICAL_SIZE ? (char*)flagblob_ptr : (char*)&flag;
}
