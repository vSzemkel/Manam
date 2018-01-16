
#include "stdafx.h"

TEST(PrecelTests, Podium) {
    constexpr char* bits = "00003BFF";
    CFlag flag{ bits };

    /*
    0XXX0
    XXXXX
    XXXXX
    */

    EXPECT_FALSE(flag.GetBit(0) == 0);
    EXPECT_FALSE(flag.GetBit(1) == 0);
    EXPECT_FALSE(flag.GetBit(2) == 0);
    EXPECT_FALSE(flag.GetBit(3) == 0);
    EXPECT_FALSE(flag.GetBit(4) == 0);
    EXPECT_FALSE(flag.GetBit(5) == 0);
    EXPECT_FALSE(flag.GetBit(6) == 0);
    EXPECT_FALSE(flag.GetBit(7) == 0);
    EXPECT_FALSE(flag.GetBit(8) == 0);
    EXPECT_FALSE(flag.GetBit(9) == 0);
    EXPECT_TRUE(flag.GetBit(10) == 0);
    EXPECT_FALSE(flag.GetBit(11) == 0);
    EXPECT_FALSE(flag.GetBit(12) == 0);
    EXPECT_FALSE(flag.GetBit(13) == 0);
    EXPECT_TRUE(flag.GetBit(14) == 0);

    EXPECT_STREQ(CString(bits), flag.ToRaw().MakeUpper()) << L"Konstrukcja nieodwracalna";
}
