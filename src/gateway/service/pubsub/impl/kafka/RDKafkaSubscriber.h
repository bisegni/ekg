#ifndef RDKAFKASUBSCRIBER_H
#define RDKAFKASUBSCRIBER_H

#pragma once

#include <memory>
#include <librdkafka/rdkafkacpp.h>

#include <gateway/common/types.h>
#include <gateway/service/pubsub/ISubscriber.h>
#include <gateway/service/pubsub/impl/kafka/RDKafkaBase.h>
namespace gateway::pubsub::impl::kafka
{
    class RDKafkaSubscriber : public ISubscriber, RDKafkaBase
    {

        std::unique_ptr<RdKafka::KafkaConsumer> consumer;

    protected:
        int internalConsume(std::unique_ptr<RdKafka::Message> message, SubscriberInterfaceElementVector &dataVector);

    public:
        RDKafkaSubscriber();
        virtual ~RDKafkaSubscriber();
        virtual void init(            
            const std::string& bootstrap_server,
            const std::string& group_id = std::string()
            );
        virtual void deinit();
        virtual int setQueue(const gateway::common::StringVector &queue);
        virtual int getMsg(SubscriberInterfaceElementVector &dataVector, int m_num, int timeo = 10);
    };

}
#endif