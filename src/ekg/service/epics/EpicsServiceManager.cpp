#include <chrono>
#include <ekg/service/epics/EpicsServiceManager.h>
#include <ranges>

using namespace ekg::common;
using namespace ekg::service::epics_impl;

EpicsServiceManager::EpicsServiceManager() {
    run = true;
    scheduler_thread = std::make_unique<std::thread>(&EpicsServiceManager::task, this);
}
EpicsServiceManager::~EpicsServiceManager() {
    run = false;
    scheduler_thread->join();
}

void EpicsServiceManager::addChannel(const std::string& channel_name, const std::string& protocoll) {
    std::unique_lock guard(channel_map_mutex);
    if (auto search = channel_map.find(channel_name); search != channel_map.end()) {
        return;
    }
    try {
        channel_map[channel_name] = std::make_shared<EpicsChannel>(protocoll, channel_name);
        channel_map[channel_name]->connect();
        channel_map[channel_name]->startMonitor();
    } catch (std::exception& ex) {
        channel_map.erase(channel_name);
        throw ex;
    } catch (...) {
        channel_map.erase(channel_name);
        throw std::runtime_error("Unknown error during channel registration");
    }
}

StringVector EpicsServiceManager::getMonitoredChannels() {
    std::unique_lock guard(channel_map_mutex);
    auto kv = std::views::keys(channel_map);
    return {kv.begin(), kv.end()};
}

void EpicsServiceManager::removeChannel(const std::string& channel_name) {
    std::lock_guard guard(channel_map_mutex);
    channel_map.erase(channel_name);
}

void EpicsServiceManager::monitorChannel(const std::string& channel_name, bool activate) {
    if (activate) {
        addChannel(channel_name);
    } else {
        removeChannel(channel_name);
    }
}

size_t EpicsServiceManager::getChannelMonitoredSize() {
    std::lock_guard guard(channel_map_mutex);
    return channel_map.size();
}

ekg::common::BroadcastToken EpicsServiceManager::addHandler(EpicsServiceManagerHandler new_handler) {
    std::lock_guard guard(channel_map_mutex);
    return handler_broadcaster.registerHandler(new_handler);
}

size_t EpicsServiceManager::getHandlerSize() {
    std::lock_guard guard(channel_map_mutex);
    handler_broadcaster.purge();
    return handler_broadcaster.targets.size();
}
void EpicsServiceManager::processIterator(const std::shared_ptr<EpicsChannel>& epics_channel) {
    MonitorEventVecShrdPtr received_event = epics_channel->monitor();
    if (!received_event->size() || !handler_broadcaster.targets.size()) return;
    handler_broadcaster.broadcast(received_event);
}

void EpicsServiceManager::task() {
    std::shared_ptr<EpicsChannel> current_channel;
    while (run) {
        // lock and scan opened channel
        {
            std::unique_lock guard(channel_map_mutex);
            for (auto& [key, value]: channel_map) {
                processIterator(value);
            }
        }
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}