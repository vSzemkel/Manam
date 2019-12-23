
#include "stdafx.h"

TEST(FlagTests, DefaultLattice) {
    CFlag flag{ 1, 1, 5, 6 };

    ASSERT_EQ((size_t)sizeof(intptr_t), flag.GetSize()) << "Incorrect base lattice size";
    flag <<= 10;
    EXPECT_FALSE(flag[9]) << "Wrong bit value";
    EXPECT_TRUE(flag[10]) << "Wrong bit value";
    EXPECT_FALSE(flag[11]) << "Wrong bit value";
    EXPECT_STREQ(L"00000400", (LPCTSTR)(flag.ToRaw().Right(8))) << "Flag clobbered";
}

TEST(FlagTests, BitMaskInvert) {
    CFlag flag{ 1, 1, 10, 30 };

    ASSERT_EQ(40, flag.GetSize()) << "Incorrect base lattice size";
    flag |= 0x33333333;
    auto p = flag.Print();
    EXPECT_STREQ(p.Right(8), CString("00110011"));
    CFlag flag2{ flag };
    flag2 ^= 0xCCCCCCCC; // "11001100"
    p = flag2.Print();
    EXPECT_STREQ(p.Right(8), CString("11111111"));
    flag2.Invert();
    EXPECT_STREQ(L"00000000", (LPCTSTR)(flag2.ToRaw().Right(8))) << "Flag clobbered";
}

TEST(FlagTests, BitCount) {
    CFlag flag{ "A580BD51C580BDAF" };

    auto zero_cnt = flag.GetBitCnt(0);
    auto one_cnt = flag.GetBitCnt(1);
    ASSERT_EQ(zero_cnt + one_cnt, 8 * flag.GetSize()) << "New kind of bit invented!";

    std::wstring p{ flag.Print() };
    auto char0_cnt = std::count(p.begin(), p.end(), '0');
    auto char1_cnt = std::count(p.begin(), p.end(), '1');
    EXPECT_EQ(char0_cnt, zero_cnt);
    EXPECT_EQ(char1_cnt, one_cnt);

    CFlag space{ 5, 6, 5, 6 };
    auto ile_mod = space.GetBitCnt(true);
    EXPECT_EQ(ile_mod, 30);
    CFlag pasek{ 5, 1, 5, 6 };
    space ^= pasek;
    ile_mod = space.GetBitCnt(true);
    EXPECT_EQ(ile_mod, 25);
    ile_mod = space.GetBitCnt(false);
    EXPECT_EQ(ile_mod, space.GetSize() * 8 - 25);
}

TEST(FlagTests, MaskShift) {
    CFlag flag{ 100 };
    const auto block_cnt = flag.GetSize() / sizeof(intptr_t);

    flag |= 1;
    ASSERT_EQ(flag.GetBitCnt(1), block_cnt) << "New kind of bit invented!";
    flag <<= 6;
    EXPECT_EQ(flag.GetBitCnt(1), block_cnt);

    flag.SetZero();
    flag.SetBit(0);
    flag <<= 300;
    EXPECT_EQ(flag.GetBitCnt(1), 1);
}

TEST(FlagTests, SizeRoundup) {
    CFlag f1{ 1 };
    EXPECT_EQ(f1.GetSize(), sizeof(uintptr_t));
    CFlag f2{ sizeof(uintptr_t) / 2 };
    EXPECT_EQ(f2.GetSize(), sizeof(uintptr_t));
    CFlag f3{ sizeof(uintptr_t) };
    EXPECT_EQ(f3.GetSize(), sizeof(uintptr_t));
    CFlag f4{ sizeof(uintptr_t) + 1 };
    EXPECT_EQ(f4.GetSize(), 2 * sizeof(uintptr_t));
    CFlag f5{ 2 * sizeof(uintptr_t) - 2 };
    EXPECT_EQ(f5.GetSize(), 2 * sizeof(uintptr_t));
    CFlag f10{ 9 * sizeof(uintptr_t) + 1 };
    EXPECT_EQ(f10.GetSize(), 10 * sizeof(uintptr_t));
}

TEST(FlagTests, Invert) {
    CFlag flag{ "A580BD51C580BDAFBAADF00D" };
    const int ones = flag.GetBitCnt(true);
    flag.Invert();
    const int zeros = flag.GetBitCnt(false);
    EXPECT_EQ(zeros, ones) << "Long case";
    CFlag flag2{ 2, 4, 5, 6 };
    const int ones2 = flag2.GetBitCnt(true);
    flag2.Invert();
    const int zeros2 = flag2.GetBitCnt(false);
    EXPECT_EQ(zeros2, ones2) << "Short case";
}

TEST(FlagTests, Reverse) {
    const TCHAR* raw = _T("A580BD51C580BDAFBAADF00D");
    CFlag flag{ raw };
    flag.Reverse(71);
    flag.Reverse(20);
    flag.Reverse(20);
    flag.Reverse(71);
    EXPECT_STREQ(raw, (LPCTSTR)(flag.ToRaw().Right(24).MakeUpper())) << "Flag clobbered";
}

TEST(FlagTests, Equality) {
    CFlag flag1{ "A580BD51C580BDAE" };
    CFlag flag2{ "A580BD52C580BDAF" };
    EXPECT_FALSE(flag1 == flag2) << "Equality failed";
    EXPECT_TRUE(flag1 != flag2) << "Inequality failed";
}
