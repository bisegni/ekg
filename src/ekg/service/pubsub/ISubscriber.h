#ifndef ISUBSCRIBER_H
#define ISUBSCRIBER_H

#pragma once

#include <ekg/common/types.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ekg::service::pubsub {

// publisher configuration
struct SubscriberConfiguration {
    // subscriber broker address
    std::string server_address;
    // subscriber group id
    std::string group_id;
    // custom k/v string map for implementation parameter
    ekg::common::MapStrKV custom_impl_parameter;
};
DEFINE_PTR_TYPES(SubscriberConfiguration)

typedef struct SubscriberInterfaceElement {
    const std::string key;
    // const int64_t off;
    // const int32_t par;
    const size_t data_len;
    std::unique_ptr<const char[]> data;
} SubscriberInterfaceElement;

DEFINE_VECTOR_FOR_TYPE(std::shared_ptr<const SubscriberInterfaceElement>, SubscriberInterfaceElementVector);

typedef std::function<void(SubscriberInterfaceElement&)> SubscriberInterfaceHandler;

typedef enum ConsumerInterfaceEventType { ONDELIVERY, ONARRIVE, ONERROR } ConsumerInterfaceEventType;
/*

 */
class ISubscriber {
protected:
    DEFINE_MAP_FOR_TYPE(ConsumerInterfaceEventType, SubscriberInterfaceHandler, handlers)
    const ConstSubscriberConfigurationUPtr configuration;
public:
    ISubscriber(ConstSubscriberConfigurationUPtr configuration);
    virtual ~ISubscriber() = default;
    /**
     * @brief Set the Topics where the consumer need to fetch data
     *
     * @param topics
     */
    virtual void setQueue(const ekg::common::StringVector& queue) = 0;
    virtual void addQueue(const ekg::common::StringVector& queue) = 0;
    virtual void commit(const bool& async = false) = 0;
    //! Fetch in synchronous way the message
    /**
         waith until the request number of message are not received keeping in mind the timeout
     */
    virtual int getMsg(SubscriberInterfaceElementVector& dataVector, unsigned int m_num, unsigned int timeo = 10) = 0;
};

} // namespace ekg::service::pubsub

#endif