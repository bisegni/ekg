#include <ekg/controller/node/worker/GetCommandWorker.h>

using namespace ekg::controller::node::worker;
using namespace ekg::service::epics_impl;

GetCommandWorker::GetCommandWorker(
    std::shared_ptr<BS::thread_pool> shared_worker_processing,
    EpicsServiceManagerShrdPtr epics_service_manager)
    : CommandWorker(shared_worker_processing), epics_service_manager(epics_service_manager){}

void GetCommandWorker::submitCommand(ekg::controller::command::CommandConstShrdPtr command) {

}