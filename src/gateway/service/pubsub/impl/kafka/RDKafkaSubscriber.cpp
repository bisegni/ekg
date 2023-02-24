#include <gateway/service/pubsub/impl/kafka/RDKafkaSubscriber.h>
#include <gateway/common/uuid.h>

#include <stdexcept>
#include <chrono>

using namespace gateway::common;
using namespace gateway::pubsub::impl::kafka;

RDKafkaSubscriber::RDKafkaSubscriber()
    : ISubscriber(), RDKafkaBase() {}

RDKafkaSubscriber::~RDKafkaSubscriber() {}

void RDKafkaSubscriber::init(
    const std::string &bootstrap_server,
    const std::string &group_id)
{
    std::string errstr;
    // setting properties
    RDK_CONF_SET(conf, "enable.partition.eof", "false")
    // RDK_CONF_SET(conf, "debug", "cgrp,topic,fetch,protocol")
    RDK_CONF_SET(conf, "bootstrap.servers", bootstrap_server)
    RDK_CONF_SET(conf, "group.id", group_id.empty()?UUID::generateUUIDLite():group_id)
    RDK_CONF_SET(conf, "client.id", "epics_gateway_consumer_" + UUID::generateUUIDLite())
    RDK_CONF_SET(conf, "enable.auto.commit", "true")
    RDK_CONF_SET(conf, "auto.offset.reset", "latest")
    RDK_CONF_SET(conf, "default_topic_conf", t_conf.get())

    // RDK_CONF_SET(conf, "batch.size", "2000", errstr);
    consumer.reset(RdKafka::KafkaConsumer::create(conf.get(), errstr));
    if (!consumer)
    {
        // RDK_CONS_ERR_ << "Failed to create consumer: " << errstr;
        throw std::runtime_error("Error creating kafka producer (" + errstr + ")");
    }
}
void RDKafkaSubscriber::deinit()
{
    consumer->close();
}

int RDKafkaSubscriber::setQueue(const gateway::common::StringVector &queue)
{
    if (!consumer)
    {
        throw std::runtime_error("Subscriber has not been initialized");
    }
    RdKafka::ErrorCode err = consumer->subscribe(queue);
    if (err != RdKafka::ERR_NO_ERROR)
    {
        // RDK_CONS_ERR_ << "Failed to subscribe " << RdKafka::err2str(err);
        return -1;
    }
    else
    {
        return 0;
    }
}

int RDKafkaSubscriber::getMsg(SubscriberInterfaceElementVector &messages, int m_num, int timeo)
{
    RdKafka::ErrorCode err = RdKafka::ERR_NO_ERROR;
    // RDK_CONS_APP_ << "Entering getMsg";
    bool looping = true;

    int timeout_ms = timeo;
    auto end = std::chrono::system_clock::now() + std::chrono::milliseconds(timeo);
    while (messages.size() < m_num && looping)
    {
        std::unique_ptr<RdKafka::Message> msg(consumer->consume(timeout_ms));
        switch (msg->err())
        {
        case RdKafka::ERR__PARTITION_EOF:
        {
            // If partition EOF and have consumed messages, retry with timeout 1
            // This allows getting ready messages, while not waiting for new ones
            if (messages.size() > 0)
            {
                timeout_ms = 1;
            }
            msg.reset();
            // RDK_CONS_APP_ << "Retry on EOF partition";
            break;
        }
        case RdKafka::ERR_NO_ERROR:
        {
            // RDK_CONS_APP_ << "Message recevied";
            if (internalConsume(std::move(msg), messages) != 0)
            {
                // RDK_CONS_ERR_ << "Error consuming received message";
            }
            break;
        }
        case RdKafka::ERR__TIMED_OUT:
        case RdKafka::ERR__TIMED_OUT_QUEUE:
            // Break of the loop if we timed out
            err = RdKafka::ERR_NO_ERROR;
            looping = false;
            msg.reset();
            break;
        default:
            // Set the error for any other errors and break
            if (messages.size() != 0)
            {
                err = msg->err();
            }
            msg.reset();
            looping = false;
            break;
        }
        auto _now = std::chrono::system_clock::now();
        timeout_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - _now).count();

        if (timeout_ms < 0)
        {
            break;
        }
    }
    // RDK_CONS_APP_ << "Exit getMsg";
    if (err != RdKafka::ERR_NO_ERROR)
    {
        // RDK_CONS_ERR_ << RdKafka::err2str(err);
        return -1;
    }
    else
    {
        return 0;
    }
}

int RDKafkaSubscriber::internalConsume(std::unique_ptr<RdKafka::Message> message, SubscriberInterfaceElementVector &messages)
{
    size_t len = message->len();
    std::unique_ptr<char[]> buffer(new char[len]);

    // copy message
    std::memcpy(buffer.get(), message->payload(), len);

    messages.push_back(
        std::make_shared<SubscriberInterfaceElement>(
            SubscriberInterfaceElement{
                *message->key(),
                len,
                std::move(buffer)}));
    return 0;
}
