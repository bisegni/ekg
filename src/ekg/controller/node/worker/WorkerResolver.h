#ifndef EKG_CONTROLLER_NODE_WORKER_WORKERRESOLVER_H_
#define EKG_CONTROLLER_NODE_WORKER_WORKERRESOLVER_H_

#include <map>
#include <string>
#include <memory>
#include <ekg/controller/command/CMDCommand.h>
namespace ekg::controller::node::worker
{
    template <typename T>
    class WorkerResolver
    {
        std::map<ekg::controller::command::CommandType, std::shared_ptr<T>> typed_instances;
    public:
        void registerService(const ekg::controller::command::CommandType type, std::shared_ptr<T> object)
        {
            typed_instances.insert(std::pair<ekg::controller::command::CommandType, std::shared_ptr<T>>(type, object));
        }
        std::shared_ptr<T> resolve(const ekg::controller::command::CommandType type)
        {
            return typed_instances[type];
        }
    };
}

#endif // EKG_CONTROLLER_NODE_WORKER_WORKERRESOLVER_H_