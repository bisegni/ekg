#ifndef EpicsChannel_H
#define EpicsChannel_H

#include <cadef.h>
#include <string>
#include <memory>
#include <pva/client.h>
#include <pv/configuration.h>
#include <pv/createRequest.h>

namespace gateway {
namespace epics_impl {

enum  MonitorType{
    Fail,
    Cancel,
    Disconnec,
    Data
};

typedef struct {
    MonitorType type;
    const std::string channel_name;
    const std::string message;
    epics::pvData::PVStructure::shared_pointer data;
} MonitorEvent;

typedef std::vector<std::shared_ptr<MonitorEvent>> MonitorEventVec;
typedef std::shared_ptr<MonitorEventVec> MonitorEventVecShrdPtr;

class EpicsChannel {
    const std::string channel_name;
    const std::string address;
    epics::pvData::PVStructure::shared_pointer pvReq = epics::pvData::createRequest("field()");
    epics::pvAccess::Configuration::shared_pointer conf = epics::pvAccess::ConfigurationBuilder()
                                                            .push_env()
                                                            .build();
    std::unique_ptr<pvac::ClientProvider> provider;
    std::unique_ptr<pvac::ClientChannel> channel;
    pvac::MonitorSync mon;
public:
explicit EpicsChannel(
    const std::string& provider_name,
    const std::string& channel_name,
    const std::string& address = std::string()
    );
~EpicsChannel() = default;
static void init();
static void deinit();
void connect();
epics::pvData::PVStructure::const_shared_pointer getData() const;
template<typename T >
void putData(const std::string& name, T new_value) const {
    channel->put().set(name, new_value).exec();
}
void putData(const std::string& name, const epics::pvData::AnyScalar& value) const;
void startMonitor();
MonitorEventVecShrdPtr monitor();
void stopMonitor();
};
}
}
#endif
