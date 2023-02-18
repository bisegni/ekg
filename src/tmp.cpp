#include "sciarc.h"

#include <map>
#include <string>
#include <stdio.h>
#include <iostream>
#include <epics/EpicsChannel.h>
#include <epics/EpicsChannelMonitor.h>
#include <map>
#include <stdint.h>
#include <thread>
#include <mutex>
#include <pv/json.h>
namespace pvd = epics::pvData;

std::unique_ptr<EpicsChannelMonitor> channel_monitoring;
std::mutex channel_map_mutex;

void ACFunction() {
  char name[] = "channle_name";
  int a[5] = {1,2,3,4,5};
  goCallbackHandler(name, a, 5);
}

// event handler called by monitor
void eventHandler(const MonitorEventVecShrdPtr& event_data) {

  for(auto& iter: *event_data) {
    std::string json_str;
    std::ostringstream json;
    //epics::pvData::printJSON(json, *iter->data);
    iter->data->dumpValue(json);
    json_str = json.str();
    
    auto out = (char*)calloc(json_str.size(), sizeof(char));
    strcpy(out, json_str.c_str());
    goCallbackHandler(
      const_cast<char*>(iter->channel_name.c_str()),
      out,
      json_str.size()
    );
  }
}

int init() {
  EpicsChannel::init();

  // init moitor
  channel_monitoring = std::make_unique<EpicsChannelMonitor>();
  channel_monitoring->setHandler(std::bind(eventHandler, std::placeholders::_1));
  channel_monitoring->start();
  return 0;
}

char* getData(const char *channel_name) {
    std::ostringstream json;
    auto channel = std::make_unique<EpicsChannel>("pva", std::string(channel_name));
    channel->connect();
    auto value = channel->getData();
    if(!value) return nullptr;
    //epics::pvData::printJSON(json, value);
    value->dumpValue(json);
    std::string str(json.str());
    auto out = (char*)calloc(str.size(), sizeof(char));
    strcpy(out, str.c_str());
    return out;
}

void startMonitor(const char *channel_name) {
  try {
    channel_monitoring->addChannel(channel_name);
  } catch(...) {

  }
}

void stopMonitor(const char *channel_name) {
  try {
    channel_monitoring->removeChannel(channel_name);
  } catch(...) {

  }
}

void deinit() {
  if(channel_monitoring) {
    channel_monitoring->stop();
  }
  EpicsChannel::deinit();
}