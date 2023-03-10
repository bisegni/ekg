#ifndef EKG_CONTROLLER_NODE_WORKER_ACQUIRECOMMANDWORKER_H_
#define EKG_CONTROLLER_NODE_WORKER_ACQUIRECOMMANDWORKER_H_

#include <ekg/common/types.h>
#include <ekg/controller/node/worker/CommandWorker.h>
#include <ekg/service/epics/EpicsServiceManager.h>
#include <ekg/service/log/ILogger.h>
#include <ekg/service/pubsub/IPublisher.h>

#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>
namespace ekg::controller::node::worker {

// Message structure for publisher
class MonitorMessage : public ekg::service::pubsub::PublishMessage {
    const std::string request_type;
    const std::string distribution_key;
    const std::string queue;
    //! the message data
    const std::string message;

public:
    MonitorMessage(const std::string& distribution_key, const std::string& queue, const std::string& message);
    virtual ~MonitorMessage() = default;
    char* getBufferPtr();
    size_t getBufferSize();
    const std::string& getQueue();
    const std::string& getDistributionKey();
    const std::string& getReqType();
};
// define the base ptr types
DEFINE_PTR_TYPES(MonitorMessage)

// map a channel to the topics where it need to be published
DEFINE_MAP_FOR_TYPE(std::string, std::vector<std::string>, ChannelTopicsMap);

//
// ss the command handler for the management of the AcquireCommand
//
class AcquireCommandWorker : public CommandWorker {
    mutable std::shared_mutex channel_map_mtx;
    ChannelTopicsMap channel_topics_map;
    ekg::service::log::ILoggerShrdPtr logger;
    ekg::service::pubsub::IPublisherShrdPtr publisher;
    ekg::service::epics_impl::EpicsServiceManagerShrdPtr epics_service_manager;
    // Handler's liveness token
    ekg::common::BroadcastToken handler_token;

    void acquireManagement(ekg::controller::command::CommandConstShrdPtr command);
    void epicsMonitorEvent(const ekg::service::epics_impl::MonitorEventVecShrdPtr& event_data);

public:
    AcquireCommandWorker(std::shared_ptr<BS::thread_pool> shared_worker_processing,
                         ekg::service::epics_impl::EpicsServiceManagerShrdPtr epics_service_manager);
    virtual ~AcquireCommandWorker() = default;
    void submitCommand(ekg::controller::command::CommandConstShrdPtr command);
};

} // namespace ekg::controller::node::worker

#endif // EKG_CONTROLLER_NODE_WORKER_ACQUIRECOMMANDWORKER_H_