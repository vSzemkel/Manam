
#include "stdafx.h"

#pragma region mock
CString RzCyfra(const int digit, const int offset) noexcept
{
    /* vu: Pierwszym argumentem jest cyfra dziesietna, drugim trzypozycyjna tablica cyfr
    rzymskich. Jej zawartosc ustalana jest na podstawie miejsca w zapisie pozycyjnym
    calej liczby cyfry bedacej pierwszym argumentem. Wywolywana przez CDrawObj::Rzymska end vu	*/

    TCHAR ret[5]{ 0 };
    const TCHAR rz_cyfry[7] = { 'I', 'V', 'X', 'L', 'C', 'D', 'M' };
    const auto grupa_cyfr = &rz_cyfry[offset];

    switch (digit) {
    case 0:
        break;
    case 1:
        ret[0] = grupa_cyfr[0];
        break;
    case 2:
        ret[0] = grupa_cyfr[0];
        ret[1] = grupa_cyfr[0];
        break;
    case 3:
        ret[0] = grupa_cyfr[0];
        ret[1] = grupa_cyfr[0];
        ret[2] = grupa_cyfr[0];
        break;
    case 4:
        ret[0] = grupa_cyfr[0];
        ret[1] = grupa_cyfr[1];
        break;
    case 5:
        ret[0] = grupa_cyfr[1];
        break;
    case 6:
        ret[0] = grupa_cyfr[1];
        ret[1] = grupa_cyfr[0];
        break;
    case 7:
        ret[0] = grupa_cyfr[1];
        ret[1] = grupa_cyfr[0];
        ret[2] = grupa_cyfr[0];
        break;
    case 8:
        ret[0] = grupa_cyfr[1];
        ret[1] = grupa_cyfr[0];
        ret[2] = grupa_cyfr[0];
        ret[3] = grupa_cyfr[0];
        break;
    case 9:
        ret[0] = grupa_cyfr[0];
        ret[1] = grupa_cyfr[2];
        break;
    default:
        return _T("err");
    }

    return ret;
}

CString Rzymska(int i)
{
    return CString('M', i / 1000) + RzCyfra((i % 1000) / 100, 4) + RzCyfra((i % 100) / 10, 2) + RzCyfra(i % 10, 0);
}

const char* memstr(const char* const buf, const char* const pat, size_t patlen)
{
    const auto bufend = buf + 100;
    const auto ret = std::search(buf, bufend, pat, pat + patlen);
    if (ret < bufend)
        return ret;

    return nullptr;
}
#pragma endregion

TEST(MiscTests, RomanNumerals) {
    EXPECT_STREQ(_T("I"), Rzymska(1));
    EXPECT_STREQ(_T("II"), Rzymska(2));
    EXPECT_STREQ(_T("III"), Rzymska(3));
    EXPECT_STREQ(_T("IV"), Rzymska(4));
    EXPECT_STREQ(_T("V"), Rzymska(5));
    EXPECT_STREQ(_T("VI"), Rzymska(6));
    EXPECT_STREQ(_T("VII"), Rzymska(7));
    EXPECT_STREQ(_T("VIII"), Rzymska(8));
    EXPECT_STREQ(_T("IX"), Rzymska(9));

    EXPECT_STREQ(_T("XXXIX"), Rzymska(39));
    EXPECT_STREQ(_T("LXXXVIII"), Rzymska(88));
    EXPECT_STREQ(_T("CLVII"), Rzymska(157));
    EXPECT_STREQ(_T("DCXV"), Rzymska(615));
    EXPECT_STREQ(_T("MCDXI"), Rzymska(1411));

    EXPECT_STREQ(_T("MMCMXCIX"), Rzymska(2999));
}

TEST(MiscTests, MemStr) {
    const char text[] = "zaqwsxcderfv543\xa6\00bnmjhgfrtcsKOTEKequdo825dgqjx92jdDCSIvfds9*%c3@^((b%^((*gswtysirkkwpfceoioGt7j";
    const char pat1[] = "KOTEK";
    const char pat2[] = "KO£EK";

    EXPECT_NE(memstr(text, pat1, sizeof(pat1) - 1), nullptr);
    EXPECT_EQ(memstr(text, pat2, sizeof(pat2) - 1), nullptr);
}