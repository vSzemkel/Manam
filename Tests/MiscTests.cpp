
#include "../StdAfx.h"
#include "../ManPDF.h"

TEST(MiscTests, RomanNumerals) {
    EXPECT_STREQ(_T("I"), CDrawObj::Rzymska(1));
    EXPECT_STREQ(_T("II"), CDrawObj::Rzymska(2));
    EXPECT_STREQ(_T("III"), CDrawObj::Rzymska(3));
    EXPECT_STREQ(_T("IV"), CDrawObj::Rzymska(4));
    EXPECT_STREQ(_T("V"), CDrawObj::Rzymska(5));
    EXPECT_STREQ(_T("VI"), CDrawObj::Rzymska(6));
    EXPECT_STREQ(_T("VII"), CDrawObj::Rzymska(7));
    EXPECT_STREQ(_T("VIII"), CDrawObj::Rzymska(8));
    EXPECT_STREQ(_T("IX"), CDrawObj::Rzymska(9));

    EXPECT_STREQ(_T("XXXIX"), CDrawObj::Rzymska(39));
    EXPECT_STREQ(_T("LXXXVIII"), CDrawObj::Rzymska(88));
    EXPECT_STREQ(_T("CLVII"), CDrawObj::Rzymska(157));
    EXPECT_STREQ(_T("DCXV"), CDrawObj::Rzymska(615));
    EXPECT_STREQ(_T("MCDXI"), CDrawObj::Rzymska(1411));

    EXPECT_STREQ(_T("MMCMXCIX"), CDrawObj::Rzymska(2999));
}

TEST(MiscTests, MemStr) {
    const char text[n_size] = "zaqwsxcderfv543\xa6\00bnmjhgfrtcsKOTEKequdo825dgqjx92jdDCSIvfds9*%c3@^((b%^((*gswtysirkkwpfceoioGt7j";
    const char pat1[] = "KOTEK";
    const char pat2[] = "KO£EK";

    EXPECT_NE(CManPDF::memstr(text, pat1, sizeof(pat1) - 1), nullptr);
    EXPECT_EQ(CManPDF::memstr(text, pat2, sizeof(pat2) - 1), nullptr);
}