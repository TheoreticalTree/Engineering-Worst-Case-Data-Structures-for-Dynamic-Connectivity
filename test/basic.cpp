#include <gtest/gtest.h>

#include "base.hpp"

// Demonstrate some basic assertions.
TEST(ExampleTests, edgeXor) {
    edge e1 = {1,2};
    edge e2 = {2,1};
    edge e3 = {3,3};
    EXPECT_EQ(e1^e2, e3);
    e1 ^= e2;
    EXPECT_EQ(e1, e3);
}