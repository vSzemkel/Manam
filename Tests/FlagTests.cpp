
#include "stdafx.h"

TEST(FlagTests, DefaultLattice) {
    CFlag flag{ 1, 1, 5, 6 };

    ASSERT_EQ((size_t)sizeof(intptr_t), flag.GetSize()) << L"Nieprawid這wy rozmiar kraty bazowej";
    flag <<= 10;
    EXPECT_FALSE(flag[9]) << L"Nieprawid這wy bit";
    EXPECT_TRUE(flag[10]) << L"Nieprawid這wy bit";
    EXPECT_FALSE(flag[11]) << L"Nieprawid這wy bit";
    EXPECT_STREQ(L"00000400", (LPCTSTR)flag.ToRaw()) << L"Zaburzona flaga";
}

TEST(FlagTests, BitMaskInvert) {
    CFlag flag{ 1, 1, 10, 30 };

    ASSERT_EQ(40, flag.GetSize()) << L"Nieprawid這wy rozmiar kraty bazowej";
    flag |= 0x33333333;
    auto p = flag.Print();
    EXPECT_STREQ(p.Right(8), CString("00110011"));
    CFlag flag2{ flag };
    flag2 ^= 0xCCCCCCCC; // "11001100"
    p = flag2.Print();
    EXPECT_STREQ(p.Right(8), CString("11111111"));
    flag2.Invert();
    ASSERT_TRUE(flag2.IsZero());
}

TEST(FlagTests, BitCount) {
    CFlag flag{"A580BD51C580BDAF"};

    auto zero_cnt = flag.GetBitCnt(0);
    auto one_cnt = flag.GetBitCnt(1);
    ASSERT_EQ(zero_cnt + one_cnt, 8 * flag.GetSize()) << L"New kind of bit invented!";

    std::wstring p{flag.Print()};
    auto char0_cnt = std::count(p.begin(), p.end(), '0');
    auto char1_cnt = std::count(p.begin(), p.end(), '1');
    EXPECT_EQ(char0_cnt, zero_cnt);
    EXPECT_EQ(char1_cnt, one_cnt);
}

TEST(FlagTests, MaskShift) {
    CFlag flag{100};
    const auto block_cnt = flag.GetSize() / sizeof(intptr_t);

    flag |= 1;
    ASSERT_EQ(flag.GetBitCnt(1), block_cnt) << L"New kind of bit invented!";
    flag <<= 6;
    EXPECT_EQ(flag.GetBitCnt(1), block_cnt);

    flag.SetZero();
    flag.SetBit(0);
    flag <<= 300;
    EXPECT_EQ(flag.GetBitCnt(1), 1);
}
