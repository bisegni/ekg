#ifndef EKG_CONTROLLER_NODE_WORKER_GETCOMMANDWORKER_H_
#define EKG_CONTROLLER_NODE_WORKER_GETCOMMANDWORKER_H_

#include <ekg/controller/node/worker/CommandWorker.h>
#include <ekg/service/epics/EpicsServiceManager.h>

namespace ekg::controller::node::worker {

class GetCommandWorker : public CommandWorker {
    ekg::service::epics_impl::EpicsServiceManagerShrdPtr epics_service_manager;

public:
    GetCommandWorker(std::shared_ptr<BS::thread_pool> shared_worker_processing,
                     ekg::service::epics_impl::EpicsServiceManagerShrdPtr epics_service_manager);
    virtual ~GetCommandWorker() = default;
    void submitCommand(ekg::controller::command::CommandConstShrdPtr command);
};

} // namespace ekg::controller::node::worker

#endif // EKG_CONTROLLER_NODE_WORKER_GETCOMMANDWORKER_H_