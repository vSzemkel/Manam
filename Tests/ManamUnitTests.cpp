
#include "pch.h"

// prerequisite: check "Test Adapter For Google Test" in VS Installer
// docs: https://github.com/google/googletest/blob/master/googletest/docs/Primer.md
// docs: https://github.com/google/googletest/blob/master/googletest/docs/AdvancedGuide.md

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // Get a checkpoint of the memory after Google Test has been initialized.
    _CrtMemState memoryState = { 0 };
    _CrtMemCheckpoint(&memoryState);
    int retval = RUN_ALL_TESTS();

    // Check for leaks after tests have run
    _CrtMemDumpAllObjectsSince(&memoryState);
    return retval;
}
