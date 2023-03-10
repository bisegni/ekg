#ifndef EpicsServiceManager_H
#define EpicsServiceManager_H
#include <ekg/common/types.h>
#include <ekg/common/broadcaster.h>
#include <ekg/service/epics/EpicsChannel.h>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace ekg::service::epics_impl {

DEFINE_MAP_FOR_TYPE(std::string, EpicsChannelShrdPtr, EpicsChannelMap)
typedef const MonitorEventVecShrdPtr& EpicsServiceManagerHandlerParamterType;
typedef std::function<void(EpicsServiceManagerHandlerParamterType)> EpicsServiceManagerHandler;

class EpicsServiceManager {
    std::mutex channel_map_mutex;
    std::map<std::string, std::shared_ptr<EpicsChannel>> channel_map;
    std::unique_ptr<std::thread> scheduler_thread;
    ekg::common::broadcaster<EpicsServiceManagerHandlerParamterType> handler_broadcaster;
    bool run = false;
    void task();
    void processIterator(const std::shared_ptr<EpicsChannel>& epics_channel);

public:
    explicit EpicsServiceManager();
    ~EpicsServiceManager();
    void addChannel(const std::string& channel_name, const std::string& protocoll = "pva");
    void removeChannel(const std::string& channel_name);
    void monitorChannel(const std::string& channel_name, bool activate);
    size_t getChannelMonitoredSize();
    /**
     * Register an event handler and return a token. Unitl this token is alive
     * the handler receives the events. The internal broadcaster dispose all handler
     * which token is invalid
    */
    ekg::common::BroadcastToken addHandler(EpicsServiceManagerHandler new_handler);
    size_t getHandlerSize();
    ekg::common::StringVector getMonitoredChannels();
};

DEFINE_PTR_TYPES(EpicsServiceManager)

} // namespace ekg::service::epics_impl
#endif