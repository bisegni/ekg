#include <gateway/Gateway.h>

#include <gateway/service/log/ILogger.h>
#include <gateway/service/ServiceResolver.h>
#include <gateway/service/log/impl/BoostLogger.h>
#include <cstdlib>

using namespace gateway;
using namespace gateway::service;
using namespace gateway::service::log;
using namespace gateway::service::log::impl;
Gateway::Gateway() : po(std::make_unique<gateway::common::ProgramOptions>()) {}

int Gateway::setup(int argc, const char *argv[])
{
    int err = 0;
    std::shared_ptr<ILogger> logger;
    try {
        po->init();
        po->parse(argc, argv);
        // setup logger
        ServiceResolver<ILogger>::registerService(logger = std::make_shared<BoostLogger>(po->getloggerConfiguration()));
        logger->logMessage("General Purphose Gateway System");
    } catch(std::runtime_error re) {
        err = 1;
        if(logger) logger->logMessage(re.what(), LogLevel::FATAL);
    } catch(...) {
        err = 1;
        if(logger) logger->logMessage("Undeterminated error", LogLevel::FATAL);
    }

    return err;
}

int Gateway::run(int argc, const char *argv[])
{
    if (setup(argc, argv))
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}