#ifndef EpicsChannelMonitor_H
#define EpicsChannelMonitor_H
#include <map>
#include <thread>
#include <mutex>
#include <memory>
#include <functional>

#include <ekg/service/epics/EpicsChannel.h>

namespace ekg {
namespace epics_impl {

typedef std::map<std::string, std::shared_ptr<EpicsChannel>> EpicsChannelMap;
typedef std::map<std::string, std::shared_ptr<EpicsChannel>>::iterator EpicsChannelMapIterator;
typedef std::function<void(const ekg::epics_impl::MonitorEventVecShrdPtr& event_data)> EpicsChannelMonitorHandler;

class EpicsChannelMonitor {
    std::mutex channel_map_mutex;
    std::map<std::string, std::shared_ptr<EpicsChannel>> channel_map;
    std::unique_ptr<std::thread> scheduler_thread;
    EpicsChannelMonitorHandler data_handler;
    bool run = false;
    void task();
    void processIterator(const std::shared_ptr<EpicsChannel>& epics_channel);
public:
explicit EpicsChannelMonitor() = default;
~EpicsChannelMonitor() = default;
void start();
void stop();
void addChannel(const std::string& channel_name);
void removeChannel(const std::string& channel_name);
void setHandler(EpicsChannelMonitorHandler new_handler);
};
}
}
#endif