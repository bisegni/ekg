#ifndef EKG_CONTROLLER_NODE_NODECONTROLLER_H_
#define EKG_CONTROLLER_NODE_NODECONTROLLER_H_

#include <ekg/common/BS_thread_pool.hpp>
#include <ekg/controller/command/CMDCommand.h>
#include <ekg/controller/node/configuration/NodeConfiguration.h>
#include <ekg/controller/node/worker/CommandWorker.h>
#include <ekg/controller/node/worker/WorkerResolver.h>

#include <ekg/service/log/ILogger.h>

#include<ekg/service/epics/EpicsServiceManager.h>

#include <memory>

namespace ekg::controller::node {
/**
 * Main controller class for the node operation
 */
class NodeController {
    std::shared_ptr<BS::thread_pool> processing_pool;
    worker::WorkerResolver<worker::CommandWorker> worker_resolver;
    configuration::NodeConfigurationUPtr node_configuration;

    ekg::service::log::ILoggerShrdPtr logger;
    ekg::service::epics_impl::EpicsServiceManager epics_service_manager;
public:
    NodeController(ekg::service::data::DataStorageUPtr data_storage);
    NodeController() = delete;
    NodeController(const NodeController&) = delete;
    NodeController& operator=(const NodeController&) = delete;
    ~NodeController();
    /**
     * Process an array of command
     */
    void submitCommand(ekg::controller::command::CommandConstShrdPtrVec commands);
};

} // namespace ekg::controller::node

#endif // EKG_CONTROLLER_NODE_NODECONTROLLER_H_