#include <ekg/common/ProgramOptions.h>

#include <string>
#include <ostream>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <exception>

using namespace ekg::common;
using namespace ekg::service::log;
using namespace ekg::controller;
namespace po = boost::program_options;

ProgramOptions::ProgramOptions()
{
    options.add_options()("help", "Show all the options")
    (CONF_FILE, po::value<bool>()->default_value(false), "Specify if we need to load configuration from file")
    (CONF_FILE_NAME, po::value<std::string>()->default_value(""), "Specify te file with the configuration")
    (LOG_ON_CONSOLE, po::value<bool>()->default_value(true), "Specify when the logger print in console")
    (LOG_ON_FILE, po::value<bool>()->default_value(false), "Specify when the logger print in file")
    (LOG_FILE_NAME, po::value<std::string>(), "Specify the log file path")
    (LOG_FILE_MAX_SIZE, po::value<int>()->default_value(1), "Specify the maximum log file size in mbyte")
    (LOG_ON_SYSLOG, po::value<bool>()->default_value(false), "Specify when the logger print in syslog server")
    (SYSLOG_SERVER, po::value<std::string>(), "Specify syslog hotsname")
    (SYSLOG_PORT, po::value<int>()->default_value(514), "Specify syslog server port")
    (LOG_LEVEL, po::value<std::string>()->default_value("info"), "Specify the level of the log using the value [debug, info, notice, warning, fatal]")
    (MESSAGE_BUS_ADDRESS, po::value<std::string>(), "Specify the messages address")
    (CMD_INPUT_TOPIC, po::value<std::string>(), "Specify the messages bus's queue where the ekg receive the configuration command");
}

void ProgramOptions::parse(int argc, const char *argv[])
{
    try
    {
        po::store(
            po::command_line_parser(argc, argv)
                .options(options)
                .allow_unregistered()
                .run(),
            vm);

        po::store(
            po::parse_environment(
                options,
                "EPICS_ekg_"),
            vm);
        po::notify(vm);

        // check if we need to load further option from file
        if(vm[CONF_FILE].as<bool>()) {
            const std::string conf_file_name = vm[CONF_FILE_NAME].as<std::string>();
            if(conf_file_name.empty()) {
                throw std::runtime_error("configuration file has nott been specifyed");
            }
            //load from file
            std::ifstream option_file_stream;
            option_file_stream.open(conf_file_name.c_str(), std::ifstream::in);
            if (!option_file_stream) {
                throw std::runtime_error("Error opening configuration file");
            }

            po::store(po::parse_config_file(option_file_stream, options), vm);
            po::notify(vm);
        }
    }
    catch (po::too_many_positional_options_error &e)
    {
        // A positional argument like `opt2=option_value_2` was given
        std::cerr << e.what() << std::endl;
        throw std::runtime_error(e.what());
    }
    catch (po::error_with_option_name &e)
    {
        // Another usage error occurred
        std::cerr << e.what() << std::endl;
        throw std::runtime_error(e.what());
    }
}

bool ProgramOptions::optionConfigure(const std::string &name)
{
    return vm.count(name) > 0;
}

#define GET_OPTION(opt, type, def) \
    optionConfigure(opt) ? getOption<type>(opt) : def

std::shared_ptr<const LogConfiguration> 
ProgramOptions::getloggerConfiguration()
{
    return std::make_shared<const LogConfiguration>(
        LogConfiguration{
            .log_on_console = GET_OPTION(LOG_ON_CONSOLE, bool, false),
            .log_on_file = GET_OPTION(LOG_ON_FILE, bool, false),
            .log_file_name = GET_OPTION(LOG_FILE_NAME, std::string, ""),
            .log_file_max_size_mb = GET_OPTION(LOG_FILE_MAX_SIZE, int, 1),
            .log_on_syslog = GET_OPTION(LOG_ON_SYSLOG, bool, false),
            .log_syslog_srv = GET_OPTION(SYSLOG_SERVER, std::string, ""),
            .log_syslog_srv_port = GET_OPTION(SYSLOG_PORT, int, 514)});
}

CMDControllerConfigUPtr 
ProgramOptions::getCMDControllerConfiguration() {
    return std::make_unique<const CMDControllerConfig>(
        CMDControllerConfig{
            .message_bus_address = GET_OPTION(MESSAGE_BUS_ADDRESS, std::string, ""),
            .topic_in = GET_OPTION(CMD_INPUT_TOPIC, std::string, "")
        });
}