#include <gtest/gtest.h>
#include <gateway/Gateway.h>
#include <gateway/common/ProgramOptions.h>

#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>

using namespace gateway;

TEST(Gateway, Default)
{
    int argc = 1;
    const char *argv[1] = {"epics-gateway-test"};
    const char *LOG_FILE_TEST_NAME = "/workspace/test/log-file-test.log";
    // remove possible old file
    std::filesystem::remove(LOG_FILE_TEST_NAME);
    // set environment variable for test
    setenv(std::string("EPICS_GATEWAY_").append(LOG_ON_CONSOLE).c_str(), "false", 1);
    setenv(std::string("EPICS_GATEWAY_").append(LOG_ON_FILE).c_str(), "true", 1);
    setenv(std::string("EPICS_GATEWAY_").append(LOG_FILE_NAME).c_str(), LOG_FILE_TEST_NAME, 1);
    std::unique_ptr<Gateway> gateway = std::make_unique<Gateway>();
    gateway->run(argc, argv);

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
    EXPECT_NE(full_str.find("Epics Gateway System"), std::string::npos);
}