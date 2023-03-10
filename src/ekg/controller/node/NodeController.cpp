#include <ekg/controller/node/NodeController.h>

//------------ command include ----------
#include <ekg/controller/node/worker/AcquireCommandWorker.h>
#include <ekg/controller/node/worker/GetCommandWorker.h>
#include <ekg/controller/node/worker/WorkerResolver.h>
#include <ekg/service/ServiceResolver.h>

using namespace ekg::controller::node;
using namespace ekg::controller::node::worker;
using namespace ekg::controller::node::configuration;

using namespace ekg::controller::command;


using namespace ekg::service;
using namespace ekg::service::data;
using namespace ekg::service::data::repository;
using namespace ekg::service::log;
using namespace ekg::service::epics_impl;

NodeController::NodeController(DataStorageUPtr data_storage)
    : node_configuration(std::make_unique<NodeConfiguration>(std::move(data_storage)))
    , processing_pool(std::make_shared<BS::thread_pool>()) {
    // set logger
    logger = ServiceResolver<ILogger>::resolve();

    // register worker for command type
    worker_resolver.registerService(
        CommandType::monitor,
        std::make_shared<AcquireCommandWorker>(processing_pool, ServiceResolver<EpicsServiceManager>::resolve()));
    worker_resolver.registerService(
        CommandType::get,
        std::make_shared<GetCommandWorker>(processing_pool, ServiceResolver<EpicsServiceManager>::resolve()));
}

NodeController::~NodeController() { processing_pool->wait_for_tasks(); }

void NodeController::submitCommand(ekg::controller::command::CommandConstShrdPtrVec commands) {
    // scann and process al command
    for (auto& c: commands) {
        switch (c->type) {
        case CommandType::monitor: {
            std::shared_ptr<const AquireCommand> acquire_command_shrd = static_pointer_cast<const AquireCommand>(c);
            if (acquire_command_shrd->activate) {
                // start monitoring
                node_configuration->addChannelMonitor(
                    {ChannelMonitorType{.channel_name = acquire_command_shrd->channel_name,
                                        .channel_protocol = acquire_command_shrd->protocol,
                                        .channel_destination = acquire_command_shrd->destination_topic}});
            } else {
                // stop monitoring
                node_configuration->removeChannelMonitor(
                    {ChannelMonitorType{.channel_name = acquire_command_shrd->channel_name,
                                        .channel_protocol = acquire_command_shrd->protocol,
                                        .channel_destination = acquire_command_shrd->destination_topic}});
            }
            break;
        }
        default:
            break;
        }

        logger->logMessage("Process command => " + to_json_string(c));
        // submit command to appropiate worker
        if (auto worker = worker_resolver.resolve(c->type); worker != nullptr) {
            auto type_str = command_type_to_string(c->type);
            worker->submitCommand(c);
        } else {
            logger->logMessage("No worker found for command type '"+std::string(command_type_to_string(c->type))+"'");
        }
    }
}