#include <ekg/service/epics/EpicsChannelMonitor.h>
#include <chrono>

using namespace ekg::epics_impl;

void EpicsChannelMonitor::start() {
    run = true;
    scheduler_thread = std::make_unique<std::thread>(&EpicsChannelMonitor::task, this);
}

void EpicsChannelMonitor::stop() {
    run = false;
    scheduler_thread->join();
}

void EpicsChannelMonitor::addChannel(const std::string& channel_name) {
    std::lock_guard guard(channel_map_mutex);
    if (auto search = channel_map.find(channel_name); search != channel_map.end()) {
        return;
    }

    channel_map[channel_name] = std::make_shared<EpicsChannel>("pva", channel_name);
    channel_map[channel_name]->connect();
    channel_map[channel_name]->startMonitor();
}

void EpicsChannelMonitor::removeChannel(const std::string& channel_name) {
    std::lock_guard guard(channel_map_mutex);
    channel_map.erase(channel_name);
}

void EpicsChannelMonitor::setHandler(EpicsChannelMonitorHandler new_handler) {
    std::lock_guard guard(channel_map_mutex);
    data_handler = new_handler;
}

void EpicsChannelMonitor::processIterator(const std::shared_ptr<EpicsChannel>& epics_channel) {
    MonitorEventVecShrdPtr received_event = epics_channel->monitor();
    if(!received_event->size() || !data_handler) return;

    data_handler(received_event);
}

void EpicsChannelMonitor::task() {
    std::shared_ptr<EpicsChannel> current_channel;
    EpicsChannelMapIterator cur_iter = channel_map.end();
    while(run) {
        // lock and increment iterator 
        {
            std::lock_guard guard(channel_map_mutex);
            if(cur_iter == channel_map.end()) {
                cur_iter = channel_map.begin();
            } else {
                cur_iter++;
            }
            if(cur_iter != channel_map.end()) {
                current_channel = cur_iter->second;
            }
        }

        // in this case, we process this channel also if in the meanwhile someone 
        // is removing it from the map
        if(!current_channel) {
            // just in case we completed all the channel give some time to sleep
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        } else {
            processIterator(current_channel);
        }
    }
}