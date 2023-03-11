#include <ekg/EKGateway.h>

#include <ekg/service/ServiceResolver.h>
#include <ekg/service/data/DataStorage.h>
#include <ekg/service/epics/EpicsServiceManager.h>
#include <ekg/service/log/ILogger.h>
#include <ekg/service/log/impl/BoostLogger.h>
#include <ekg/service/pubsub/pubsub.h>

#include <cstdlib>

using namespace ekg;
using namespace ekg::service;

using namespace ekg::service::log;
using namespace ekg::service::log::impl;

using namespace ekg::service::epics_impl;

using namespace ekg::service::data;

using namespace ekg::service::pubsub;
using namespace ekg::service::pubsub::impl::kafka;

using namespace ekg::controller::node;
using namespace ekg::controller::command;

EKGateway::EKGateway()
    : po(std::make_unique<ekg::common::ProgramOptions>())
    , quit(false)
    , terminated(false) {}

int EKGateway::setup(int argc, const char* argv[]) {
    int err = 0;
    std::shared_ptr<ILogger> logger;
    std::unique_lock lk(m);
    try {
        po->parse(argc, argv);
        if(po->hasOption(HELP)) {
            std::cout << po->getHelpDescription() << std::endl;
            return err;
        }

        // setup logger
        ServiceResolver<ILogger>::registerService(logger = std::make_shared<BoostLogger>(po->getloggerConfiguration()));
        logger->logMessage("EPICS to Kafka Gateway");
        logger->logMessage("Start EPICS service");
        ServiceResolver<EpicsServiceManager>::registerService(std::make_shared<EpicsServiceManager>());
        logger->logMessage("Start publisher service");
        ServiceResolver<IPublisher>::registerService(std::make_shared<RDKafkaPublisher>(po->getPublisherConfiguration()));
        logger->logMessage("Start subscriber service");
        ServiceResolver<ISubscriber>::registerService(std::make_shared<RDKafkaSubscriber>(po->getSubscriberConfiguration()));
        // ServiceResolver<DataStorage>::registerService(std::make_shared<DataStorage>(po->getStoragePath()));
        logger->logMessage("Start node controller");
        node_controller = std::make_unique<NodeController>(std::make_unique<DataStorage>(po->getStoragePath()));

        logger->logMessage("Start command controller");
        cmd_controller =
            std::make_unique<CMDController>(po->getCMDControllerConfiguration(),
                                            std::bind(&EKGateway::commandReceived, this, std::placeholders::_1));



        cv.wait(lk, [this] { return this->quit; });

        // deallocation
        logger->logMessage("Stop command controller");
        cmd_controller.reset();
        logger->logMessage("Stop node controller");
        node_controller.reset();
        logger->logMessage("Stop subscriber service");
        ServiceResolver<ISubscriber>::resolve().reset();
        logger->logMessage("Stop publihser service");
        ServiceResolver<IPublisher>::resolve().reset();
        logger->logMessage("Stop EPICS service");
        ServiceResolver<EpicsServiceManager>::resolve().reset();
        logger->logMessage("Shoutdown compelted");
        ServiceResolver<ILogger>::resolve().reset();
        terminated = true;
    } catch (std::runtime_error re) {
        err = 1;
        if (logger) logger->logMessage(re.what(), LogLevel::FATAL);
    } catch (...) {
        err = 1;
        if (logger) logger->logMessage("Undeterminated error", LogLevel::FATAL);
    }
    
    return err;
}

int EKGateway::run(int argc, const char* argv[]) {
    if (setup(argc, argv)) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void EKGateway::stop() {
    std::lock_guard lk(m);
    quit = true;
    cv.notify_one();
}

const bool EKGateway::isStopRequested() {return quit;}
const bool EKGateway::isTerminated() {return terminated;}

void EKGateway::commandReceived(ekg::controller::command::CommandConstShrdPtrVec received_command) {
    node_controller->submitCommand(received_command);
}