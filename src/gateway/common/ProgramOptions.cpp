#include <gateway/common/ProgramOptions.h>

#include <string>
#include <ostream>
#include <regex>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <exception>

using namespace gateway::common;
using namespace gateway::service::log;
namespace po = boost::program_options;

void ProgramOptions::init()
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
    (CMD_TOPIC_NAME, po::value<std::string>(), "Specify input topic where the gateway receive the configuration command");
}

void ProgramOptions::parse(int argc, const char *argv[])
{
    const std::regex r("EPICS_GATEWAY_(.*)");
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
                "EPICS_GATEWAY_"),
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

std::shared_ptr<const LogConfiguration> ProgramOptions::getloggerConfiguration()
{
    return std::make_shared<const LogConfiguration>(
        LogConfiguration{
            GET_OPTION(LOG_ON_CONSOLE, bool, false),
            GET_OPTION(LOG_ON_FILE, bool, false),
            GET_OPTION(LOG_FILE_NAME, std::string, ""),
            GET_OPTION(LOG_FILE_MAX_SIZE, int, 1),
            GET_OPTION(LOG_ON_SYSLOG, bool, false),
            GET_OPTION(SYSLOG_SERVER, std::string, ""),
            GET_OPTION(SYSLOG_PORT, int, 514)});
}