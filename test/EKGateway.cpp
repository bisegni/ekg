#include <gtest/gtest.h>
#include <ekg/EKGateway.h>
#include <ekg/common/ProgramOptions.h>

#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>

using namespace ekg;

TEST(EKGateway, Default)
{
    int argc = 1;
    const char *argv[1] = {"epics-ekg-test"};
    const char *LOG_FILE_TEST_NAME = "/workspace/test/log-file-test.log";
    // remove possible old file
    std::filesystem::remove(LOG_FILE_TEST_NAME);
    // set environment variable for test
    clearenv();
    setenv(std::string("EPICS_ekg_").append(LOG_ON_CONSOLE).c_str(), "false", 1);
    setenv(std::string("EPICS_ekg_").append(LOG_ON_FILE).c_str(), "true", 1);
    setenv(std::string("EPICS_ekg_").append(LOG_FILE_NAME).c_str(), LOG_FILE_TEST_NAME, 1);
    auto ekg = std::make_unique<EKGateway>();
    ekg->run(argc, argv);

    // read file for test
    std::ifstream ifs; // input file stream
    std::string str;
    std::string full_str;
    ifs.open(LOG_FILE_TEST_NAME, std::ios::in);
    if (ifs)
    {
        while (!ifs.eof())
        {
            std::getline(ifs, str);
            full_str.append(str);
        }
        ifs.close();
    }
    EXPECT_NE(full_str.find("Gateway"), std::string::npos);
}