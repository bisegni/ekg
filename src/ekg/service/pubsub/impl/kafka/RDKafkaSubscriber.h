#ifndef RDKAFKASUBSCRIBER_H
#define RDKAFKASUBSCRIBER_H

#pragma once

#include <memory>
#include <string>
#include <librdkafka/rdkafkacpp.h>

#include <ekg/common/types.h>
#include <ekg/service/pubsub/ISubscriber.h>
#include <ekg/service/pubsub/impl/kafka/RDKafkaBase.h>

namespace ekg::service::pubsub::impl::kafka
{
    class RDKafkaSubscriber : public ISubscriber, RDKafkaBase
    {
        std::unique_ptr<RdKafka::KafkaConsumer> consumer;
        ekg::common::StringVector topics;
    protected:
        int internalConsume(std::unique_ptr<RdKafka::Message> message, SubscriberInterfaceElementVector &dataVector);
        virtual void init();
        virtual void deinit();
    public:
        RDKafkaSubscriber(ConstSubscriberConfigurationUPtr configuration);
        RDKafkaSubscriber() = delete;
        virtual ~RDKafkaSubscriber();
        virtual void setQueue(const ekg::common::StringVector &queue);
        virtual void addQueue(const ekg::common::StringVector &queue);
        virtual void commit(const bool& async = false);
        virtual int getMsg(SubscriberInterfaceElementVector &dataVector, unsigned int m_num, unsigned int timeo = 10);
    };

}
#endif