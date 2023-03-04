#ifndef __ILOGGER_H__
#define __ILOGGER_H__

#include <string>
#include <memory>
namespace ekg
{
    namespace service
    {
        namespace log
        {
            typedef struct LogConfiguration
            {
                bool log_on_console;
                bool log_on_file;                
                std::string log_file_name;
                int log_file_max_size_mb;
                bool log_on_syslog;
                std::string log_syslog_srv;
                int log_syslog_srv_port;
            }LogConfiguration;

            typedef enum class LogLevel
            {
                ERROR,
                INFO,
                DEBUG,
                FATAL
            } LogLevel;

            class ILogger
            {
            protected:
                std::shared_ptr<const LogConfiguration> configuration;

            public:
                ILogger(std::shared_ptr<const LogConfiguration> configuration):configuration(configuration){};
                virtual ~ILogger() = default;
                virtual void setLevel(LogLevel level) = 0;
                virtual void logMessage(const std::string &message, LogLevel level = LogLevel::INFO) = 0;
            };
        }
    }
}

#endif // __ILOGGER_H__