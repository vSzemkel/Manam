
/*
**	CFlag jest rozszerzeniem typu int_ptr wykorzystywanego jako flaga bitowa
**	Pole danych obiektu CFlag jest 8*size bitowe
*/

#include "stdafx.h"
#include "flag.h"

CFlag::CFlag(const CFlag& f) {
	size = f.size ;
	if (size > CRITICAL_SIZE) {
		flagblob_ptr = malloc(size) ;
		memcpy(flagblob_ptr, f.flagblob_ptr, size) ;
	} else 
		flag = f.flag;
}

CFlag::CFlag(CFlag&& f) noexcept {
	size = f.size;
	flag = f.flag;
	if (f.size > CRITICAL_SIZE)
		f.size = CRITICAL_SIZE;
}

CFlag::CFlag(size_t s) {
	if (s & (CRITICAL_SIZE-1))
		s = (s & (UINT_MAX - CRITICAL_SIZE + 1)) + CRITICAL_SIZE;
	size = s ;
	if (size > CRITICAL_SIZE)
		flagblob_ptr = malloc(size) ;
	SetZero();
}

CFlag::CFlag(int sx, int sy, int szpalt_x, int szpalt_y) {
	int i = szpalt_x * szpalt_y;
	size = DBRAW_BLOCK * ((i >> 5) + ((i & 0x1F) > 0 ? 1 : 0));
	if (size > CRITICAL_SIZE)
		flagblob_ptr = malloc(size) ;
	SetZero();

	if (size <= CRITICAL_SIZE) {
		if (sx) flag = 1;
		for (i = 1; i < sx; i++)
			flag = (flag << 1) + 1;
		uintptr_t tmp = flag;
		for (i = 1; i < sy; i++)
			flag = (flag << szpalt_x) + tmp;
	} else {
		for (i = 0; i < sx; i++)
			SetBit(i);
		CFlag tmp(*this);
		for (i = 1; i < sy; i++)
			*this = (*this << szpalt_x) | tmp;
	}
}

CFlag::CFlag(const char* raw) {
	size_t len = strlen(raw) ;
	if (len == 0) {
		size = DBRAW_BLOCK;
		return;
	}
	if (!(len & 7) && strspn(raw, "0123456789ABCDEFabcdef") == len) { // ci¹g 4n dwuznakowych batów
		size = len >> 1 ;
		if (size > CRITICAL_SIZE) {
			int32_t l;
			char buf[RAW_MOD_SIZE + 1];
			flagblob_ptr = malloc(size) ;
			buf[RAW_MOD_SIZE] = '\0';
			for (size_t i = 0; DBRAW_BLOCK*i < size; i++) {
				memcpy(buf, &raw[len - RAW_MOD_SIZE*(i + 1)], RAW_MOD_SIZE);
				sscanf_s(buf, "%lx", &l);
				memcpy((char*)flagblob_ptr + DBRAW_BLOCK*i, (const char*)&l, DBRAW_BLOCK);
			}
		} else sscanf_s(raw, "%Ix", &flag);
	} else { // ci¹g binarny
		SetSize(len);
		::StringCchCopyA(GetRawFlag(), size, raw);
	}
}

CFlag::CFlag(CByteArray& bArr) {
	size = bArr.GetSize() ;
	if (size > CRITICAL_SIZE) {
		flagblob_ptr = malloc(size) ;
		memcpy(flagblob_ptr, bArr.GetData(), size);
	} else {
		flag = 0;
		auto buf = reinterpret_cast<BYTE*>(&flag);
		for (size_t i = 0; i < size; i++) 
			buf[i] = bArr[i];
	}
}

CFlag::~CFlag() {
	if (size > CRITICAL_SIZE)
		free(flagblob_ptr);
}

uintptr_t CFlag::operator! () const noexcept { // the same as IsZero()
	if (size > CRITICAL_SIZE) {
		int iszero = TRUE ;
		char *str = (char*)flagblob_ptr;
		for (size_t i = 0; i < size; i++)
			iszero &= !str[i] ;
		return iszero ;
	} else 
		return !flag ;
}

const CFlag& CFlag::operator= (const CFlag& f) {
	SetSize(f.size);
	if (size > CRITICAL_SIZE)
		memcpy(flagblob_ptr, f.flagblob_ptr, size);
	else
		flag = f.flag;

	return *this;
}

const CFlag& CFlag::operator= (CFlag&& f) noexcept {
	if (this != &f) {
		if (size > CRITICAL_SIZE) 
			free(flagblob_ptr);
		size = f.size;
		flag = f.flag;
		if (f.size > CRITICAL_SIZE)
			f.size = CRITICAL_SIZE;
	}

	return *this ;
}

CFlag CFlag::operator& (const CFlag& f) const noexcept {
	if (size != f.size) return *this;
	CFlag ret(size) ;
	if (size > CRITICAL_SIZE) {
		auto src1 = (uintptr_t*)flag;
		auto src2 = (uintptr_t*)f.flag;
		auto dst  = (uintptr_t*)ret.flag;
		size_t blocks = size / CRITICAL_SIZE;
		for (size_t i = 0; i < blocks; ++i)
			dst[i] = src1[i] & src2[i] ;
	} else 
		ret.flag = flag & f.flag ;

	return ret ;
}

CFlag CFlag::operator| (const CFlag& f) const noexcept {
	if (size != f.size) return *this;
	CFlag ret(size) ;
	if (size > CRITICAL_SIZE) {
		auto src1 = (uintptr_t*)flag;
		auto src2 = (uintptr_t*)f.flag;
		auto dst  = (uintptr_t*)ret.flag;
		size_t blocks = size / CRITICAL_SIZE;
		for (size_t i = 0; i < blocks; ++i)
			dst[i] = src1[i] | src2[i] ;
	} else 
		ret.flag = flag | f.flag ;

	return ret ;
}

CFlag CFlag::operator^ (const CFlag& f) const noexcept {
	if (size != f.size) return *this;
	CFlag ret(size) ;
	if (size > CRITICAL_SIZE) {
		auto src1 = (uintptr_t*)flag;
		auto src2 = (uintptr_t*)f.flag;
		auto dst  = (uintptr_t*)ret.flag;
		size_t blocks = size / CRITICAL_SIZE;
		for (size_t i = 0; i < blocks; ++i)
			dst[i] = src1[i] ^ src2[i] ;
	} else 
		ret.flag = flag ^ f.flag ;

	return ret ;
}

CFlag& CFlag::Invert() noexcept {
	if (size > CRITICAL_SIZE) {
		auto src = (char*)flag;
		size_t blocks = size / CRITICAL_SIZE;
		for (size_t i = 0; i < blocks; ++i)
			src[i] ^= static_cast<intptr_t>(-1);
	} else 
		flag ^= static_cast<intptr_t>(-1);

	return *this ;
}

void CFlag::operator|= (const CFlag& f) noexcept {
	if (size != f.size) return;
	if (size > CRITICAL_SIZE) {
		auto src = (uintptr_t*)f.flag;
		auto dst = (uintptr_t*)flag;
		size_t blocks = size / CRITICAL_SIZE;
		for (size_t i = 0; i < blocks; ++i)
			dst[i] |= src[i];
	} else 
		flag |= f.flag ;
}

void CFlag::operator|= (uintptr_t mask) noexcept {
	if (size > CRITICAL_SIZE) {
		auto dst = (uintptr_t*)flag;
		size_t blocks = size / CRITICAL_SIZE;
		for (size_t i = 0; i < blocks; ++i)
			dst[i] |= mask;
	} else 
		flag |= mask ;
}

uintptr_t CFlag::operator& (uintptr_t mask) const noexcept {
	if (size > CRITICAL_SIZE) {
		auto dst = (uintptr_t*)flag;
		return dst[0] & mask;
	} return flag & mask ;
}

void CFlag::operator&= (uintptr_t mask) noexcept {
	if (size > CRITICAL_SIZE) {
		auto dst = (uintptr_t*)flag;
		size_t blocks = size / CRITICAL_SIZE;
		for (size_t i = 0; i < blocks; ++i)
			dst[i] &= mask;
	} else 
		flag &= mask ;
}

void CFlag::operator&= (const CFlag& f) noexcept {
	if (size != f.size) return;
	if (size > CRITICAL_SIZE) {
		auto src = (uintptr_t*)f.flag;
		auto dst  = (uintptr_t*)flag;
		size_t blocks = size / CRITICAL_SIZE;
		for (size_t i = 0; i < blocks; ++i)
			dst[i] &= src[i];
	} else 
		flag &= f.flag;
}

void CFlag::operator^= (const CFlag& f) noexcept {
	if (size != f.size) return;
	if (size > CRITICAL_SIZE) {
		auto src = (uintptr_t*)f.flag;
		auto dst = (uintptr_t*)flag;
		size_t blocks = size / CRITICAL_SIZE;
		for (size_t i = 0; i < blocks; ++i)
			dst[i] ^= src[i] ;
	} else 
		flag ^= f.flag ;
}

void CFlag::operator^= (uintptr_t mask) noexcept {
	if (size > CRITICAL_SIZE) {
		auto dst = (uintptr_t*)flag;
		size_t blocks = size / CRITICAL_SIZE;
		for (size_t i = 0; i < blocks; ++i)
			dst[i] ^= mask;
	} else 
		flag ^= mask;
}

CFlag CFlag::operator>> (size_t shift) const noexcept {
	CFlag ret(*this);
	if (size > CRITICAL_SIZE) {
		if (shift >= 8 * size) ret.SetZero();
		else {
			size_t i;
			for (i = 0; i < 8*size - shift; i++)
				ret.SetBit(i, GetBit(i + shift)) ;
			for (i = 0; i < shift; i++)
				ret.SetBit(8*size - 1 - i, 0) ;
		}
	} else 
		ret.flag = (flag >> shift);

	return ret ;
}

CFlag CFlag::operator<< (size_t shift) const noexcept {
	CFlag ret(*this);
	if (size > CRITICAL_SIZE) {
		if (shift >= 8*size) ret.SetZero() ;
		else {
			size_t i;
			for (i = 8*size - 1 - shift; i != (size_t)-1; i--)
				ret.SetBit(i + shift, GetBit(i)) ;
			for (i = 0; i < shift; i++)
				ret.SetBit(i, 0) ;
		}
	} else 
		ret.flag = (flag << shift);

	return ret ;
}

CFlag& CFlag::operator>>= (size_t shift) noexcept {
	if (size > CRITICAL_SIZE) {
		if (shift >= 8 * size) SetZero();
		else {
			size_t i;
			for (i = 0; i < 8 * size - shift; i++)
				SetBit(i, GetBit(i + shift));
			for (i = 0; i < shift; i++)
				SetBit(8 * size - 1 - i, 0);
		}
	} else 
		flag >>= shift;

	return *this ;
}

CFlag& CFlag::operator<<= (size_t shift) noexcept {
	if (size > CRITICAL_SIZE) {
		if (shift >= 8 * size) SetZero();
		else {
			size_t i;
			for (i = 8 * size - 1 - shift; i != (size_t)-1; i--)
				SetBit(i + shift, GetBit(i));
			for (i = 0; i < shift; i++)
				SetBit(i, 0);
		}
	} else 
		flag <<= shift;

	return *this ;
}

uintptr_t CFlag::operator|| (const CFlag& f) const noexcept {
	if (size > CRITICAL_SIZE) {
		return (*this | f).IsSet() ;
	} return flag || f.flag ;
}

uintptr_t CFlag::operator&& (const CFlag& f) const noexcept {
	if (size > CRITICAL_SIZE) {
		return (*this & f).IsSet() ;
	} return flag && f.flag ;
}

int CFlag::operator== (const CFlag& f) const noexcept {
	if (size != f.size) return 0;
	if (size > CRITICAL_SIZE) {
		return !memcmp(flagblob_ptr, f.flagblob_ptr, size) ;
	} return flag == f.flag ;
}

int CFlag::operator!= (const CFlag& f) const noexcept {
	if (size != f.size) return 1;
	if (size > CRITICAL_SIZE) {
		return memcmp(flagblob_ptr, f.flagblob_ptr, size) ;
	} return flag != f.flag ;
}

void CFlag::SetZero() noexcept {
	if (size > CRITICAL_SIZE) 
		memset(flagblob_ptr, 0, size);
	else 
		flag = 0;
}

bool CFlag::IsZero() const noexcept {
	if (size > CRITICAL_SIZE) {
		bool iszero = true ;
		auto src = (uintptr_t*)flag;
		size_t blocks = size / CRITICAL_SIZE;
		for (size_t i = 0; i < blocks; ++i)
			iszero &= !src[i] ;
		return iszero ;
	} return flag == 0 ;
}

bool CFlag::IsSet() const noexcept {
	if (size > CRITICAL_SIZE) {
		bool iszero = true;
		auto src = (uintptr_t*)flag;
		size_t blocks = size / CRITICAL_SIZE;
		for (size_t i = 0; i < blocks; ++i)
			iszero &= !src[i];
		return !iszero ;
	} return flag != 0 ;
}

size_t CFlag::GetSize() const noexcept {
	return size ;
}

void CFlag::SetSize(size_t s) {
	if (size != s) {
		if (size > CRITICAL_SIZE) free(flagblob_ptr) ;
		if (s > CRITICAL_SIZE) flagblob_ptr = malloc(s) ;
		size = s;
	}
}

void CFlag::CopyFlag(CByteArray *bArr) {
	bArr->SetSize(size);
	char *str = GetRawFlag();
	for (size_t i = 0; i < size; i++, str++)
		bArr->SetAt(i, *str);
}

void CFlag::Reverse(size_t len) noexcept {
	if (len <= 8 * size)
		for (size_t i = 0; i < len / 2; i++) {
			unsigned char bit = GetBit(i);
			SetBit(i, GetBit(len - i - 1));
			SetBit(len - i - 1, bit);
		}
}

void CFlag::Serialize(CArchive& ar) {
	if (ar.IsStoring())	{
		ar << (WORD)size ;
		ar.Write(GetRawFlag(), (UINT)size);
	} else {
		WORD w ;
		ar >> w; 
		SetSize(w) ;
		ar.Read(GetRawFlag(), (UINT)size);
	}
}

int CFlag::GetBitCnt(unsigned char val) const noexcept {
	int iIle = 0;
	for (size_t i = 0; i < 8 * size; ++i)
		if ((GetBit(i) > 0) == (val > 0))
			iIle++;
	return iIle;
}

unsigned char CFlag::GetBit(size_t pos) const noexcept {
	unsigned char mask = (1 << (pos % 8));
	char *str = GetRawFlag();
	str += (pos / 8) ;
	return *str & mask ;
}

void CFlag::SetBit(size_t pos, unsigned char bit) noexcept {
	unsigned char mask = (1 << (pos % 8));
	char *str = GetRawFlag();
	str += (pos / 8);
	*str = ( bit ? *str | mask : *str & (0xff-mask) ) ;
}

CString CFlag::Print() const {
	CString display('\0', (int) size * 8);
	for (int i = 0; i < (int)size; i++)
		for (int j = 0; j < 8; j++)
			display.SetAt(8 * i + j, (GetBit(8 * i + j) ? '1' : '0'));
	display.MakeReverse() ;
	return display ;
}

CString CFlag::ToRaw() const {
	TCHAR buf[9];
	CString raw("");
	if (size > CRITICAL_SIZE) {
		int32_t l;
		char *str = (char*)flag ;
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

char* CFlag::GetRawFlag() const noexcept {
	return size > CRITICAL_SIZE ? (char*)flagblob_ptr : (char*)&flag;
}
