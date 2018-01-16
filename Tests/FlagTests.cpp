
#include "stdafx.h"

TEST(FlagTests, DefaultLattice) {
    CFlag flag{ 1, 1, 5, 6 };

    ASSERT_EQ((size_t)sizeof(intptr_t), flag.GetSize()) << L"Nieprawidłowy rozmiar kraty bazowej";
    flag <<= 10;
    EXPECT_TRUE(flag.GetBit(9) == 0) << L"Nieprawidłowy bit";
    EXPECT_FALSE(flag.GetBit(10) == 0) << L"Nieprawidłowy bit";
    EXPECT_TRUE(flag.GetBit(11) == 0) << L"Nieprawidłowy bit";
    EXPECT_STREQ(L"00000400", (LPCTSTR)flag.ToRaw()) << L"Zaburzona flaga";
}
