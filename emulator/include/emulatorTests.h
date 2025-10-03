//
// Created by Bernardo on 02/10/2025.
//

#ifndef EMULATORTESTS_H
#define EMULATORTESTS_H

#include <stdio.h>

#define ASSERT_EQ(actual, expected, msg) \
    if ((actual) != (expected)) { \
        printf("FAIL: %s (expected 0x%02X, got 0x%02X)\n", msg, (expected), (actual)); \
        return 1; \
    }

void test_emulator();

#endif //EMULATORTESTS_H
