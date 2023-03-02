#ifndef GATEWAY_SERVICE_DATA_MODEL_CHANNEL_H_
#define GATEWAY_SERVICE_DATA_MODEL_CHANNEL_H_
#include <string>
#include <memory>
#include <optional>
namespace gateway::service::data
{
    class DataStorage;
    namespace repository
    {
        struct ChannelType
        {
            int id = -1;
            std::string channel_name;
            std::string channel_protocol;
        };

        class ChannelRepository
        {
            friend class gateway::service::data::DataStorage;
            DataStorage& data_storage;
            ChannelRepository(DataStorage& data_storage);
        public:
            ~ChannelRepository() = default;
            int insert(const ChannelType& new_cannel) const;
            void removeAll();
            std::optional<std::unique_ptr<ChannelType>> getChannel(const std::string& channel_name);
        };
    }
}

#endif // GATEWAY_SERVICE_DATA_MODEL_CHANNEL_H_
