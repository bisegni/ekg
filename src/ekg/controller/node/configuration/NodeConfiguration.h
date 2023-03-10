#ifndef EKG_CONTROLLER_NODE_CONFIGURATION_NODECONFIGURATION_H_
#define EKG_CONTROLLER_NODE_CONFIGURATION_NODECONFIGURATION_H_

#include <ekg/service/data/DataStorage.h>

#include <memory>

namespace ekg::controller::node::configuration {
    typedef std::vector<ekg::service::data::repository::ChannelMonitorType> ChannelMonitorTypeConstVector;
/**
 * Abstract the node configuration
 */
class NodeConfiguration {
    const ekg::service::data::DataStorageUPtr data_storage;

public:
    NodeConfiguration(ekg::service::data::DataStorageUPtr data_storage);
    NodeConfiguration() = delete;
    NodeConfiguration(const NodeConfiguration&) = delete;
    NodeConfiguration& operator=(const NodeConfiguration&) = delete;
    ~NodeConfiguration() = default;
    /**
     * Add a monitor configuration for a determinated channel for a destination topic
     */
    void addChannelMonitor(const ChannelMonitorTypeConstVector& channel_descriptions);
    void removeChannelMonitor(const ChannelMonitorTypeConstVector& channel_descriptions);
    void iterateAllChannelMonitor(ekg::service::data::repository::ChannelMonitorTypeProcessHandler handle);
};

typedef std::unique_ptr<NodeConfiguration> NodeConfigurationUPtr;
} // namespace ekg::controller::node::configuration

#endif // EKG_CONTROLLER_NODE_CONFIGURATION_NODECONFIGURATION_H_