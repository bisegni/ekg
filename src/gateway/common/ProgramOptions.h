#ifndef __PROGRAMOPTIONS_H__
#define __PROGRAMOPTIONS_H__

#include <boost/program_options.hpp>
#include <gateway/service/log/ILogger.h>

namespace po = boost::program_options;

static const char* const CONF_FILE = "conf-file";
static const char* const CONF_FILE_NAME = "conf-file-name";
static const char* const LOG_ON_CONSOLE = "log-on-console";
static const char* const LOG_ON_FILE = "log-on-file";
static const char* const LOG_FILE_NAME = "log-file-name";
static const char* const LOG_FILE_MAX_SIZE = "log-file-max-size";
static const char* const LOG_ON_SYSLOG = "log-on-syslog";
static const char* const SYSLOG_SERVER = "syslog-server";
static const char* const SYSLOG_PORT = "syslog-port";

static const char* const CMD_TOPIC_NAME = "cmd-input-topic";

static const char* const LOG_LEVEL = "log-level";

namespace gateway
{
    namespace common
    {
        /**
         * Options management
         */
        class ProgramOptions
        {
            po::variables_map vm;
            po::options_description options{"Epics Gateway"};

        public:
            ProgramOptions() = default;
            ~ProgramOptions() = default;
            void init();
            void parse(int argc, const char *argv[]);

            std::shared_ptr<const gateway::service::log::LogConfiguration> getloggerConfiguration();

            bool optionConfigure(const std::string &name);

            template <class T>
            const T &getOption(const std::string &name)
            {
                return vm[name].as<T>();
            }
        };

    } // namespace common

} // namespace gateway

#endif // __PROGRAMOPTIONS_H__