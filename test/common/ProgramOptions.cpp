
#include <gtest/gtest.h>

#include <filesystem>
#include <stdlib.h>
#include <string>

#include <ekg/common/ProgramOptions.h>

using namespace ekg::common;
using namespace ekg::service::pubsub;
namespace fs = std::filesystem;

TEST(ProgramOptions, CPPStandardOptions) {
    int argc = 1;
    const char* argv[1] = {"epics-ekg-test"};
    // set environment variable for test
    clearenv();
    setenv("EPICS_ekg_log-level", "debug", 1);
    std::unique_ptr<ProgramOptions> opt = std::make_unique<ProgramOptions>();
    opt->parse(argc, argv);
    auto log_level = opt->getOption<std::string>("log-level");
    EXPECT_STREQ(log_level.c_str(), "debug");
}

#define VAR_NAME(a, b) a #b
TEST(ProgramOptions, LogConfiguration) {
    int argc = 1;
    const char* argv[1] = {"epics-ekg-test"};
    // set environment variable for test
    clearenv();
    setenv(std::string("EPICS_ekg_").append(LOG_ON_CONSOLE).c_str(), "true", 1);
    setenv(std::string("EPICS_ekg_").append(LOG_ON_FILE).c_str(), "true", 1);
    setenv(std::string("EPICS_ekg_").append(LOG_FILE_NAME).c_str(), "log-file-name", 1);
    setenv(std::string("EPICS_ekg_").append(LOG_FILE_MAX_SIZE).c_str(), "1234", 1);
    setenv(std::string("EPICS_ekg_").append(LOG_ON_SYSLOG).c_str(), "true", 1);
    setenv(std::string("EPICS_ekg_").append(SYSLOG_SERVER).c_str(), "syslog-server", 1);
    setenv(std::string("EPICS_ekg_").append(SYSLOG_PORT).c_str(), "5678", 1);
    std::unique_ptr<ProgramOptions> opt = std::make_unique<ProgramOptions>();
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

TEST(ProgramOptions, FileConfiguration) {
    int argc = 1;
    const char* argv[1] = {"epics-ekg-test"};
    // set environment variable for test
    const std::string location = fs::path(fs::current_path()) / "test/common/test-conf-file.conf";
    clearenv();
    setenv("EPICS_ekg_conf-file", "true", 1);
    setenv("EPICS_ekg_conf-file-name", location.c_str(), 1);
    std::unique_ptr<ProgramOptions> opt = std::make_unique<ProgramOptions>();
    EXPECT_NO_THROW(opt->parse(argc, argv););
    auto logger_configuration = opt->getloggerConfiguration();
    EXPECT_EQ(logger_configuration->log_on_console, false);
    EXPECT_EQ(logger_configuration->log_file_max_size_mb, 1234);
    EXPECT_EQ(logger_configuration->log_on_syslog, true);
    EXPECT_STREQ(logger_configuration->log_syslog_srv.c_str(), "syslog-server");
    EXPECT_EQ(logger_configuration->log_syslog_srv_port, 5678);
}

TEST(ProgramOptions, FileConfigurationNoPathSpecified) {
    int argc = 1;
    const char* argv[1] = {"epics-ekg-test"};
    // set environment variable for test
    clearenv();
    setenv("EPICS_ekg_conf-file", "true", 1);
    setenv("EPICS_ekg_conf-file-name", "", 1);
    std::unique_ptr<ProgramOptions> opt = std::make_unique<ProgramOptions>();
    ASSERT_THROW(opt->parse(argc, argv), std::runtime_error);
}

TEST(ProgramOptions, FileConfigurationBadPath) {
    int argc = 1;
    const char* argv[1] = {"epics-ekg-test"};
    // set environment variable for test
    clearenv();
    setenv("EPICS_ekg_conf-file", "true", 1);
    setenv("EPICS_ekg_conf-file-name", "/bad/file/path", 1);
    std::unique_ptr<ProgramOptions> opt = std::make_unique<ProgramOptions>();
    ASSERT_THROW(opt->parse(argc, argv), std::runtime_error);
}

TEST(ProgramOptions, PublisherConfiguration) {
    int argc = 1;
    const char* argv[1] = {"epics-ekg-test"};
    // set environment variable for test
    ConstPublisherConfigurationUPtr conf;
    clearenv();
    setenv("EPICS_ekg_pub-server-address", "pub-server", 1);
    setenv("EPICS_ekg_pub-impl-kv", "k1:v1", 1);
    std::unique_ptr<ProgramOptions> opt = std::make_unique<ProgramOptions>();
    EXPECT_NO_THROW(opt->parse(argc, argv););
    EXPECT_NO_THROW(conf = opt->getPublisherConfiguration(););
    EXPECT_STREQ(conf->server_address.c_str(), "pub-server");
    EXPECT_EQ(conf->custom_impl_parameter.size(), 1);
    EXPECT_STREQ(conf->custom_impl_parameter.at("k1").c_str(), "v1");
}

TEST(ProgramOptions, SubscriberConfigurationConfFile) {
    int argc = 1;
    const char* argv[1] = {"epics-ekg-test"};
    // set environment variable for test
    ConstSubscriberConfigurationUPtr conf;
    const std::string location = fs::path(fs::current_path()) / "test/common/test-conf-file.conf";
    clearenv();
    setenv("EPICS_ekg_conf-file", "true", 1);
    setenv("EPICS_ekg_conf-file-name", location.c_str(), 1);
    std::unique_ptr<ProgramOptions> opt = std::make_unique<ProgramOptions>();
    EXPECT_NO_THROW(opt->parse(argc, argv););
    EXPECT_NO_THROW(conf = opt->getSubscriberConfiguration(););
    EXPECT_STREQ(conf->server_address.c_str(), "sub-address");
    EXPECT_STREQ(conf->group_id.c_str(), "sub-group-id");
    EXPECT_EQ(conf->custom_impl_parameter.size(), 2);
    EXPECT_STREQ(conf->custom_impl_parameter.at("k1").c_str(), "v1");
    EXPECT_STREQ(conf->custom_impl_parameter.at("k2").c_str(), "v2");
}