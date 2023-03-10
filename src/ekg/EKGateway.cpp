#include <ekg/EKGateway.h>

#include <ekg/service/log/ILogger.h>
#include <ekg/service/ServiceResolver.h>
#include <ekg/service/log/impl/BoostLogger.h>
#include <ekg/service/epics/EpicsServiceManager.h>
#include <ekg/service/pubsub/pubsub.h>

#include <cstdlib>

using namespace ekg;
using namespace ekg::service;
using namespace ekg::service::log;
using namespace ekg::service::log::impl;
using namespace ekg::service::epics_impl;
using namespace ekg::service::pubsub;
using namespace ekg::service::pubsub::impl::kafka;

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


        ServiceResolver<ILogger>::registerService(std::make_shared<BoostLogger>(po->getloggerConfiguration()));
        ServiceResolver<EpicsServiceManager>::registerService(std::make_shared<EpicsServiceManager>());
        //ServiceResolver<IPublisher>::registerService(std::make_shared<RDKafkaPublisher>());

        //EXPECT_NO_THROW(storage = std::make_unique<DataStorage>(fs::path(fs::current_path()) / "test.sqlite"););
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