#include "CppUTest/TestHarness.h"

TEST_GROUP(dummy_test) {
    void setup() {
    }

    void teardown() {
    }
};

TEST(dummy_test, pass_me) {
    // void *data = (void *)malloc(100);
    // CHECK(data != 0);
    CHECK_EQUAL(1, 1);
}
