
#include <gtest/gtest.h>
#include <stdlib.h>
#include <string>
#include <gateway/common/ProgramOptions.h>

using namespace gateway::common;

TEST(ProgramOptions, CPPStandardOptions)
{
    int argc = 1;
    const char *argv[1] = {"epics-gateway-test"};
    // set environment variable for test
    setenv("EPICS_GATEWAY_log-level", "debug", 1);
    std::unique_ptr<ProgramOptions> opt = std::make_unique<ProgramOptions>();
    opt->init();
    opt->parse(argc, argv);
    auto log_level = opt->getOption<std::string>("log-level");
    EXPECT_STREQ(log_level.c_str(), "debug");
}

#define VAR_NAME(a, b) a # b
TEST(ProgramOptions, LogConfiguration)
{
    int argc = 1;
    const char *argv[1] = {"epics-gateway-test"};
    // set environment variable for test
    setenv(std::string("EPICS_GATEWAY_").append(LOG_ON_CONSOLE).c_str(), "true", 1);
    setenv(std::string("EPICS_GATEWAY_").append(LOG_ON_FILE).c_str(), "true", 1);
    setenv(std::string("EPICS_GATEWAY_").append(LOG_FILE_NAME).c_str(), "log-file-name", 1);
    setenv(std::string("EPICS_GATEWAY_").append(LOG_FILE_MAX_SIZE).c_str(), "1234", 1);
    setenv(std::string("EPICS_GATEWAY_").append(LOG_ON_SYSLOG).c_str(), "true", 1);
    setenv(std::string("EPICS_GATEWAY_").append(SYSLOG_SERVER).c_str(), "syslog-server", 1);
    setenv(std::string("EPICS_GATEWAY_").append(SYSLOG_PORT).c_str(), "5678", 1);
    std::unique_ptr<ProgramOptions> opt = std::make_unique<ProgramOptions>();
    opt->init();
    opt->parse(argc, argv);
    auto logger_configuration = opt->getloggerConfiguration();
    EXPECT_EQ(logger_configuration->log_on_console, true);
    EXPECT_EQ(logger_configuration->log_on_file, true);
    EXPECT_STREQ(logger_configuration->log_file_name.c_str(), "log-file-name");
    EXPECT_EQ(logger_configuration->log_file_max_size_mb, 1234);
    EXPECT_EQ(logger_configuration->log_on_syslog, true);
    EXPECT_STREQ(logger_configuration->log_syslog_srv.c_str(), "syslog-server");
    EXPECT_EQ(logger_configuration->log_syslog_srv_port, 5678);
}