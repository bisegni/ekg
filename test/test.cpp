#include <gtest/gtest.h>
#include <sciarc.h>
int main(int argc, char** argv)
{
    //init library
    init();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
