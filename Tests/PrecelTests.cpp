
#include "../StdAfx.h"

TEST(PrecelTests, Podium) {
    constexpr char* bits = "00003BFF";
    CFlag flag{ bits };

    /*
    0XXX0
    XXXXX
    XXXXX
    */

    EXPECT_TRUE(flag[0]);
    EXPECT_TRUE(flag[1]);
    EXPECT_TRUE(flag[2]);
    EXPECT_TRUE(flag[3]);
    EXPECT_TRUE(flag[4]);
    EXPECT_TRUE(flag[5]);
    EXPECT_TRUE(flag[6]);
    EXPECT_TRUE(flag[7]);
    EXPECT_TRUE(flag[8]);
    EXPECT_TRUE(flag[9]);
    EXPECT_FALSE(flag[10]);
    EXPECT_TRUE(flag[11]);
    EXPECT_TRUE(flag[12]);
    EXPECT_TRUE(flag[13]);
    EXPECT_FALSE(flag[14]);

    EXPECT_STREQ(CString(bits), flag.ToRaw().Right(8).MakeUpper()) << "Irreversible construction";
}
