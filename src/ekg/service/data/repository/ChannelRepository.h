#ifndef ekg_SERVICE_DATA_MODEL_CHANNEL_H_
#define ekg_SERVICE_DATA_MODEL_CHANNEL_H_
#include <ekg/controller/command/CMDCommand.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ekg::service::data {
class DataStorage;
namespace repository {
struct ChannelMonitorType {
    int id = -1;
    std::string channel_name;
    std::string channel_protocol;
    std::string channel_destination;
};

inline ChannelMonitorType toChannelMonitor(const ekg::controller::command::AquireCommand& acquire_command) {
    return ChannelMonitorType {
        .channel_name = acquire_command.channel_name, 
        .channel_protocol = acquire_command.protocol,
        .channel_destination = acquire_command.destination_topic
    };
}

typedef std::unique_ptr<ChannelMonitorType> ChannelMonitorTypeUPtr;
typedef std::function<void(uint32_t index, const ChannelMonitorType&)> ChannelMonitorTypeProcessHandler;
typedef std::vector<std::tuple<std::string, std::string>> ChannelMonitorDistinctResultType;
class ChannelRepository {
    friend class ekg::service::data::DataStorage;
    DataStorage& data_storage;
    ChannelRepository(DataStorage& data_storage);

public:
    ~ChannelRepository() = default;
    void insert(const ChannelMonitorType& channel_description);
    void remove(const ChannelMonitorType& channel_description);
    bool isPresent(const ChannelMonitorType& new_cannel) const;
    std::optional<ChannelMonitorTypeUPtr> getChannelMonitor(const ChannelMonitorType& channel_descirption) const;
    ChannelMonitorDistinctResultType getDistinctByNameProtocol() const;
    void processAllChannelMonitor(const std::string& channel_name,
                                  const std::string& channel_protocol,
                                  ChannelMonitorTypeProcessHandler handler) const;
    void removeAll();
};
} // namespace repository
} // namespace ekg::service::data

#endif // ekg_SERVICE_DATA_MODEL_CHANNEL_H_
