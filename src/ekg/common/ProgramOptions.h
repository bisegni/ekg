#ifndef __PROGRAMOPTIONS_H__
#define __PROGRAMOPTIONS_H__

#include <boost/program_options.hpp>
#include <ekg/service/log/ILogger.h>
#include <ekg/controller/command/CMDController.h>
#include <ekg/service/pubsub/IPublisher.h>
#include <ekg/service/pubsub/ISubscriber.h>

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

static const char* const CMD_INPUT_TOPIC = "cmd-input-topic";
static const char* const CMD_MAX_FECTH_CMD = "cmd-max-fecth-element";
static const char* const CMD_MAX_FETCH_TIME_OUT = "cmd-max-fecth-time-out";

static const char* const PUB_SERVER_ADDRESS = "pub-server-address";
static const char* const PUB_IMPL_KV = "pub-impl-kv";

static const char* const SUB_SERVER_ADDRESS = "sub-server-address";
static const char* const SUB_GROUP_ID = "sub-group-id";
static const char* const SUB_IMPL_KV = "sub-impl-kv";

static const char* const LOG_LEVEL = "log-level";

namespace ekg
{
    namespace common
    {
        /**
         * Options management
         */
        class ProgramOptions
        {
            po::variables_map vm;
            po::options_description options{"Epics ekg"};

            MapStrKV parseKVCustomParam(const std::vector<std::string> &kv_vec);
        public:
            ProgramOptions();
            ~ProgramOptions() = default;
            void parse(int argc, const char *argv[]);

            ekg::service::log::ConstLogConfigurationUPtr getloggerConfiguration();
            ekg::controller::command::ConstCMDControllerConfigUPtr getCMDControllerConfiguration();
            ekg::service::pubsub::ConstPublisherConfigurationUPtr getPublisherConfiguration();
            ekg::service::pubsub::ConstSubscriberConfigurationUPtr getSubscriberConfiguration();
            bool optionConfigure(const std::string &name);

            template <class T>
            const T &getOption(const std::string &name)
            {
                return vm[name].as<T>();
            }
        };

    } // namespace common

} // namespace ekg

#endif // __PROGRAMOPTIONS_H__