#include <gtest/gtest.h>
#include <iostream>

class PrintTestIO : public ::testing::Test {
};

// Single test with print statement
TEST_F(PrintTestIO, BasicOutputIO) {
    std::cout << "this is a print statement from inputoutput" << std::endl;
}