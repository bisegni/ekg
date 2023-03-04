#ifndef RDKKAFKAPUBLISHER_H
#define RDKKAFKAPUBLISHER_H

#pragma once
#include <thread>
#include <memory>

#include <librdkafka/rdkafkacpp.h>

#include <ekg/service/pubsub/IPublisher.h>
#include <ekg/service/pubsub/impl/kafka/RDKafkaBase.h>
namespace ekg::service::pubsub::impl::kafka
{
    class RDKafkaPublisher : public IPublisher, RDKafkaBase, RdKafka::DeliveryReportCb
    {
        bool _stop_inner_thread;
        bool _auto_poll;
        std::thread auto_poll_thread;
        std::unique_ptr<RdKafka::Producer> producer;
        const std::string bootstrap_server;
    protected:
        void dr_cb(RdKafka::Message &message);
        void autoPoll();

    public:
        explicit RDKafkaPublisher(const std::string &bootstrap_server);
        virtual ~RDKafkaPublisher();
        virtual int createQueue(const std::string &queue);
        virtual void setAutoPoll(bool autopoll);
        virtual void init();
        virtual void deinit();
        virtual int flush(const int timeo = 10000);
        virtual int pushMessage(PublishMessageUniquePtr message);
        virtual int pushMessages(PublisherMessageVector &messages);
        virtual size_t getQueueMessageSize();
    };
}

#endif