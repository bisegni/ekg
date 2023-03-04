#ifndef ISUBSCRIBER_H
#define ISUBSCRIBER_H

#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <functional>
#include <ekg/common/types.h>

namespace ekg::service::pubsub
{
    typedef struct SubscriberInterfaceElement
    {
        const std::string key;
        // const int64_t off;
        // const int32_t par;
        const size_t data_len;
        std::unique_ptr<const char[]> data;
    } SubscriberInterfaceElement;

    DEFINE_VECTOR_FOR_TYPE(
        std::shared_ptr<const SubscriberInterfaceElement>,
        SubscriberInterfaceElementVector);

    typedef std::function<void(SubscriberInterfaceElement &)> SubscriberInterfaceHandler;

    typedef enum ConsumerInterfaceEventType
    {
        ONDELIVERY,
        ONARRIVE,
        ONERROR
    } ConsumerInterfaceEventType;
    /*

     */
    class ISubscriber
    {
    protected:
        DEFINE_MAP_FOR_TYPE(
            ConsumerInterfaceEventType,
            SubscriberInterfaceHandler,
            handlers)
    public:
        ISubscriber() = default;
        virtual ~ISubscriber() = default;
        virtual void init() = 0;
        virtual void deinit() = 0;
        /**
         * @brief Set the Topics where the consumer need to fetch data
         *
         * @param topics
         */
        virtual int setQueue(const ekg::common::StringVector &queue) = 0;

        //! Fetch in synchronous way the message
        /**
             waith until the request number of message are not received keeping in mind the timeout
         */
        virtual int getMsg(SubscriberInterfaceElementVector &dataVector, int m_num, int timeo = 10) = 0;
    };

}

#endif