#include <ekg/controller/node/configuration/NodeConfiguration.h>
#include <ekg/common/utility.h>

using namespace ekg::common;

using namespace ekg::controller::node::configuration;

using namespace ekg::service::data;
using namespace ekg::service::data::repository;

NodeConfiguration::NodeConfiguration(DataStorageUPtr data_storage)
    : data_storage(std::move(data_storage)) {}

void NodeConfiguration::addChannelMonitor(const ChannelMonitorTypeConstVector& channel_descriptions) {
    auto channel_repository = toShared(data_storage->getChannelRepository());
    
    for(auto const &desc: channel_descriptions) {
        channel_repository->insert(desc);
    }
}

void NodeConfiguration::removeChannelMonitor(const ChannelMonitorTypeConstVector& channel_descriptions) {
    auto channel_repository = toShared(data_storage->getChannelRepository());
    for(auto const &desc: channel_descriptions) {
        channel_repository->insert(desc);
    }
}

void NodeConfiguration::iterateAllChannelMonitor(ChannelMonitorTypeProcessHandler handler) {
    auto channel_repository = toShared(data_storage->getChannelRepository());
    auto distinct_name_prot = channel_repository->getDistinctByNameProtocol();
    for(auto &ch: distinct_name_prot) {
        channel_repository->processAllChannelMonitor(
            std::get<0>(ch),
            std::get<1>(ch),
            handler
        );
    }
}