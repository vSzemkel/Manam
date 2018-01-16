
#include "stdafx.h"

// prerequisite: check "Test Adapter For Google Test" in VS Installer
// docs: https://github.com/google/googletest/blob/master/googletest/docs/Primer.md
// docs: https://github.com/google/googletest/blob/master/googletest/docs/AdvancedGuide.md

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
