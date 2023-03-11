#include <gtest/gtest.h>
#include <ekg/EKGateway.h>
#include <ekg/common/ProgramOptions.h>

#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>

using namespace ekg;
namespace fs = std::filesystem;

TEST(EKGateway, Default)
{
    int argc = 1;
    const char *argv[1] = {"epics-ekg-test"};
    const std::string log_file_path = fs::path(fs::current_path()) / "log-file-test.log";
    const std::string storage_db_file = fs::path(fs::current_path()) / "ekg.sqlite";

    // set environment variable for test
    clearenv();
    setenv(std::string("EPICS_ekg_").append(LOG_ON_CONSOLE).c_str(), "false", 1);
    setenv(std::string("EPICS_ekg_").append(LOG_ON_FILE).c_str(), "true", 1);
    setenv(std::string("EPICS_ekg_").append(LOG_FILE_NAME).c_str(), log_file_path.c_str(), 1);

    setenv(std::string("EPICS_ekg_").append(CMD_INPUT_TOPIC).c_str(), "cmd_topic_in", 1);
    setenv(std::string("EPICS_ekg_").append(PUB_SERVER_ADDRESS).c_str(), "kafka:9092", 1);
    setenv(std::string("EPICS_ekg_").append(SUB_SERVER_ADDRESS).c_str(), "kafka:9092", 1);
    setenv(std::string("EPICS_ekg_").append(STORAGE_PATH).c_str(), storage_db_file.c_str(), 1);

    // remove possible old file
    std::filesystem::remove(log_file_path);
    auto ekg = std::make_unique<EKGateway>();
     std::thread t([&ekg = ekg, &argc = argc, &argv = argv](){
         int exit_code = ekg->run(argc, argv);
         EXPECT_EQ(ekg->isStopRequested(), true);
         EXPECT_EQ(ekg->isTerminated(), true);
         EXPECT_EQ(exit_code, EXIT_SUCCESS);
    });
    sleep(2);
    ekg->stop();
    t.join();

    // read file for test
    std::ifstream ifs; // input file stream
    std::string str;
    std::string full_str;
    ifs.open(log_file_path, std::ios::in);
    if (ifs)
    {
        while (!ifs.eof())
        {
            std::getline(ifs, str);
            full_str.append(str);
        }
        ifs.close();
    }
    EXPECT_NE(full_str.find("Shoutdown compelted"), std::string::npos);
}