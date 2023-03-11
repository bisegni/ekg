#include <cassert>
#include <ekg/controller/node/worker/AcquireCommandWorker.h>
#include <ekg/service/ServiceResolver.h>
#include <functional>
#include <sstream>

using namespace ekg::controller::node::worker;
using namespace ekg::controller::command;

using namespace ekg::service;
using namespace ekg::service::log;

using namespace ekg::service::epics_impl;

using namespace ekg::service::pubsub;

namespace pvd = epics::pvData;
namespace pva = epics::pvAccess;

#pragma region MonitorMessage
MonitorMessage::MonitorMessage(const std::string& distribution_key,
                               const std::string& queue,
                               const std::string& message)
    : request_type("monitoring")
    , distribution_key(distribution_key)
    , queue(queue)
    , message(message) {}

char* MonitorMessage::getBufferPtr() { return const_cast<char*>(message.c_str()); }
size_t MonitorMessage::getBufferSize() { return message.size(); }
const std::string& MonitorMessage::getQueue() { return queue; }
const std::string& MonitorMessage::getDistributionKey() { return distribution_key; }
const std::string& MonitorMessage::getReqType() { return request_type; }
#pragma endregion MonitorMessage

#pragma region AcquireCommandWorker
AcquireCommandWorker::AcquireCommandWorker(std::shared_ptr<BS::thread_pool> shared_worker_processing,
                                           EpicsServiceManagerShrdPtr epics_service_manager)
    : CommandWorker(shared_worker_processing)
    , logger(ServiceResolver<ILogger>::resolve())
    , epics_service_manager(epics_service_manager) {
    handler_token = epics_service_manager->addHandler(
        std::bind(&AcquireCommandWorker::epicsMonitorEvent, this, std::placeholders::_1));
    publisher = ServiceResolver<IPublisher>::resolve();
}

void AcquireCommandWorker::submitCommand(CommandConstShrdPtr command) {
    return shared_worker_processing->push_task(&AcquireCommandWorker::acquireManagement, this, command);
}

void AcquireCommandWorker::acquireManagement(ekg::controller::command::CommandConstShrdPtr command) {
    assert(command->type == CommandType::monitor);
    bool activate = false;
    ConstAquireCommandShrdPtr a_ptr = static_pointer_cast<const AquireCommand>(command);
    // lock the vector for write
    {
        auto& vec_ref = channel_topics_map[a_ptr->channel_name];
        if (a_ptr->activate) {
            // add topic to channel
            // check if the topic is already present[fault tollerant check]
            if (std::find(std::begin(vec_ref), std::end(vec_ref), a_ptr->destination_topic) == std::end(vec_ref)) {
                logger->logMessage("Activate monitor on: " + a_ptr->channel_name
                                       + " for topic: " + a_ptr->destination_topic,
                                   LogLevel::INFO);
                std::unique_lock lock(channel_map_mtx);
                channel_topics_map[a_ptr->channel_name].push_back(a_ptr->destination_topic);
            }
        } else {
            // remove topic to channel
            auto itr = std::find(std::begin(vec_ref), std::end(vec_ref), a_ptr->destination_topic);
            if (itr != std::end(vec_ref)) {
                logger->logMessage("Deactivate monitor on: " + a_ptr->channel_name
                                       + " for topic: " + a_ptr->destination_topic,
                                   LogLevel::INFO);
                std::unique_lock lock(channel_map_mtx);
                vec_ref.erase(itr);
            }
        }
        activate = vec_ref.size();
    }
    // if the vec_ref has size > 0 mean that someone is still needed the channel data in monitor way
    epics_service_manager->monitorChannel(a_ptr->channel_name, activate);
}

void AcquireCommandWorker::epicsMonitorEvent(const MonitorEventVecShrdPtr& event_data) {
#ifdef __DEBUG__
    logger->logMessage("Received epics monitor event of size" + std::to_string(event_data->size()), LogLevel::DEBUG);
#endif
    std::shared_lock slock(channel_map_mtx);
    for (auto& e: *event_data) {
        // publisher
        if (e->type != MonitorType::Data) continue;
        std::stringstream svalue;
        pvd::PVField::const_shared_pointer fld(e->data->getSubField("value"));
        svalue << fld;
        for (auto& topic: channel_topics_map[e->channel_name]) {
            publisher->pushMessage(std::make_unique<MonitorMessage>(e->channel_name, topic, svalue.str()));
        }
    }
}
#pragma endregion MonitorMessage