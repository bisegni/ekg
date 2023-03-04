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
        const std::string bootstrap_server;
        const std::string group_id;
        std::unique_ptr<RdKafka::KafkaConsumer> consumer;

    protected:
        int internalConsume(std::unique_ptr<RdKafka::Message> message, SubscriberInterfaceElementVector &dataVector);

    public:
        RDKafkaSubscriber(
            const std::string &bootstrap_server,
            const std::string &group_id = std::string());
        RDKafkaSubscriber() = delete;
        virtual ~RDKafkaSubscriber();
        virtual void init();
        virtual void deinit();
        virtual int setQueue(const ekg::common::StringVector &queue);
        virtual int getMsg(SubscriberInterfaceElementVector &dataVector, int m_num, int timeo = 10);
    };

}
#endif