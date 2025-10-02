//
// Created by Bernardo on 02/10/2025.
//

#ifndef EMULATORTESTS_H
#define EMULATORTESTS_H

#define ASSERT_EQ(actual, expected, msg) \
    if ((actual) != (expected)) { \
        printf("FAIL: %s (expected 0x%X, got 0x%X)\n", msg (expected), (actual)); \
        return 0; \
    }

void test_emulator();

#endif //EMULATORTESTS_H
