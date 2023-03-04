#include <ekg/EKGateway.h>

#include <ekg/service/log/ILogger.h>
#include <ekg/service/ServiceResolver.h>
#include <ekg/service/log/impl/BoostLogger.h>
#include <cstdlib>

using namespace ekg;
using namespace ekg::service;
using namespace ekg::service::log;
using namespace ekg::service::log::impl;
EKGateway::EKGateway() : po(std::make_unique<ekg::common::ProgramOptions>()) {}

int EKGateway::setup(int argc, const char *argv[])
{
    int err = 0;
    std::shared_ptr<ILogger> logger;
    try {
        po->parse(argc, argv);
        // setup logger
        ServiceResolver<ILogger>::registerService(logger = std::make_shared<BoostLogger>(po->getloggerConfiguration()));
        logger->logMessage("EPICS to Kafka Gateway");
    } catch(std::runtime_error re) {
        err = 1;
        if(logger) logger->logMessage(re.what(), LogLevel::FATAL);
    } catch(...) {
        err = 1;
        if(logger) logger->logMessage("Undeterminated error", LogLevel::FATAL);
    }

    return err;
}

int EKGateway::run(int argc, const char *argv[])
{
    if (setup(argc, argv))
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}