#ifndef EKG_CONTROLLER_NODE_WORKER_COMMANDWORKER_H_
#define EKG_CONTROLLER_NODE_WORKER_COMMANDWORKER_H_

#include <ekg/common/BS_thread_pool.hpp>
#include <ekg/controller/command/CMDCommand.h>
#include <ekg/controller/node/worker/CommandWorker.h>

namespace ekg::controller::node::worker {

class CommandWorker {
protected:
    std::shared_ptr<BS::thread_pool> shared_worker_processing;

public:
    CommandWorker(std::shared_ptr<BS::thread_pool> shared_worker_processing)
        : shared_worker_processing(shared_worker_processing){};
    CommandWorker() = delete;
    CommandWorker(const CommandWorker&) = delete;
    CommandWorker& operator=(const CommandWorker&) = delete;
    ~CommandWorker() = default;
    virtual void submitCommand(ekg::controller::command::CommandConstShrdPtr command) = 0;
};

} // namespace ekg::controller::node::worker

#endif // EKG_CONTROLLER_NODE_WORKER_COMMANDWORKER_H_