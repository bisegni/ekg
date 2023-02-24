#include <gateway/service/log/impl/BoostLogger.h>

namespace sources = boost::log::sources;
namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace attrs = boost::log::attributes;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
using namespace gateway::service::log;
using namespace gateway::service::log::impl;

#define BASE_LOG_FORMAT "[%TimeStamp%][%Severity%]: %_%"
#define EXTENDEND_LOG_FORMAT "[%TimeStamp%][%Severity%][%ProcessID%][%ThreadID%]: %_%"


BoostLogger::BoostLogger(std::shared_ptr<const LogConfiguration> configuration) : ILogger(configuration)
{
    logging::add_common_attributes();
    boost::shared_ptr<logging::core> logger = boost::log::core::get();
    // logger->set_filter(expr::attr<level::LogSeverityLevel>("Severity") >= logLevel);
    if (configuration->log_on_console)
    {
        console_sink = logging::add_console_log(std::clog, logging::keywords::format = EXTENDEND_LOG_FORMAT);
    }

    if (configuration->log_on_file)
    {
        file_sink = logging::add_file_log(keywords::file_name = configuration->log_file_name,                                     // file name pattern
                                          keywords::rotation_size = configuration->log_file_max_size_mb * 1024 * 1024,            // rotate files every 10 MiB...
                                          keywords::time_based_rotation = logging::sinks::file::rotation_at_time_point(0, 0, 0), // ...or at midnight
                                          keywords::format = EXTENDEND_LOG_FORMAT,
                                          keywords::auto_flush = true);
    }

    if (configuration->log_on_syslog)
    {
        // Creating a syslog sink.
        syslog_sink.reset(new sinks::synchronous_sink<sinks::syslog_backend>(keywords::use_impl = sinks::syslog::udp_socket_based,
                                                                             keywords::format = EXTENDEND_LOG_FORMAT));
        // Setting the remote address to sent syslog messages to.
        syslog_sink->locked_backend()->set_target_address(configuration->log_syslog_srv, configuration->log_syslog_srv_port);
        // Adding the sink to the core.b
        logger->add_sink(syslog_sink);
    }

    // enable the log in case of needs
    logger->set_logging_enabled(
        configuration->log_on_console ||
        configuration->log_on_file ||
        configuration->log_on_syslog);
}

BoostLogger::~BoostLogger()
{
    boost::shared_ptr<logging::core> logger = boost::log::core::get();
    if (console_sink.get())
    {
        logger->remove_sink(console_sink);
        console_sink.reset();
    }
    if (file_sink.get())
    {
        logger->remove_sink(file_sink);
        file_sink.reset();
    }
    if (syslog_sink.get())
    {
        logger->remove_sink(syslog_sink);
        syslog_sink.reset();
    }
}

logging::trivial::severity_level BoostLogger::getLevel(LogLevel level)
{
    logging::trivial::severity_level boost_level = logging::trivial::info;

    switch (level)
    {
    case LogLevel::INFO:
        boost_level = logging::trivial::info;
        break;
    case LogLevel::DEBUG:
        boost_level = logging::trivial::debug;
        break;
    case LogLevel::FATAL:
        boost_level = logging::trivial::fatal;
        break;

    default:
        boost_level = logging::trivial::info;
    }
    return boost_level;
}
void BoostLogger::setLevel(LogLevel level)
{
    // set log
    logging::core::get()->set_filter(
        logging::trivial::severity >= getLevel(level));
}
void BoostLogger::logMessage(const std::string &message, LogLevel level)
{
    logging::record rec = logger_mt.open_record(keywords::severity = getLevel(level));
    if (rec)
    {
        logging::record_ostream strm(rec);
        strm << message;
        strm.flush();
        logger_mt.push_record(boost::move(rec));
    }
}