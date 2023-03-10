#include <chrono>
#include <ekg/common/uuid.h>
#include <ekg/service/pubsub/pubsub.h>
#include <future>
#include <gtest/gtest.h>
#include <thread>
#include <unistd.h>

using namespace ekg::common;
using namespace ekg::service::pubsub;
using namespace ekg::service::pubsub::impl::kafka;

#define TOPIC_TEST_NAME "queue-test"

class Message : public PublishMessage {
    const std::string request_type;
    const std::string distribution_key;
    const std::string queue;
    //! the message data
    const std::string message;

public:
    Message(const std::string& queue, const std::string& message)
        : request_type("test")
        , distribution_key(UUID::generateUUIDLite())
        , queue(queue)
        , message(message) {}
    virtual ~Message() {}

    char* getBufferPtr() { return const_cast<char*>(message.c_str()); }
    size_t getBufferSize() { return message.size(); }
    const std::string& getQueue() { return queue; }
    const std::string& getDistributionKey() { return distribution_key; }
    const std::string& getReqType() { return request_type; }
};

TEST(Kafka, KafkaSimplePubSub) {
    SubscriberInterfaceElementVector messages;
    std::unique_ptr<RDKafkaPublisher> producer = std::make_unique<RDKafkaPublisher>(
        std::make_unique<const PublisherConfiguration>(PublisherConfiguration{.server_address = "kafka:9092"}));
    std::unique_ptr<RDKafkaSubscriber> consumer = std::make_unique<RDKafkaSubscriber>(
        std::make_unique<const SubscriberConfiguration>(SubscriberConfiguration{.server_address = "kafka:9092"})
    );

    std::string message_sent = "hello_" + UUID::generateUUIDLite();

    ASSERT_NO_THROW(consumer->setQueue({TOPIC_TEST_NAME}));
    ASSERT_EQ(consumer->getMsg(messages, 1, 1000), 0);
    ASSERT_NO_THROW(consumer->commit(););
    auto iotaFuture = std::async(
        std::launch::async,
        [&message_sent](std::unique_ptr<IPublisher> producer) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            ASSERT_EQ(
                producer->pushMessage(std::move(PublishMessageUniquePtr(new Message(TOPIC_TEST_NAME, message_sent)))),
                0);
            ASSERT_EQ(producer->flush(1000), 0);
        },
        std::move(producer));

    ASSERT_EQ(consumer->getMsg(messages, 1, 1000), 0);
    while (messages.size() == 0) {
        ASSERT_EQ(consumer->getMsg(messages, 1, 1000), 0);
    }

    ASSERT_EQ(messages.size(), 1);
    ASSERT_NO_THROW(consumer->commit(););

    std::string message_received(messages[0]->data.get(), messages[0]->data_len);
    ASSERT_STREQ(message_received.c_str(), message_sent.c_str());

    iotaFuture.wait();
}

TEST(Kafka, KafkaPushMultipleMessage) {
    SubscriberInterfaceElementVector tmp_received_messages;
    SubscriberInterfaceElementVector received_messages;
    std::unique_ptr<RDKafkaPublisher> producer = std::make_unique<RDKafkaPublisher>(
        std::make_unique<const PublisherConfiguration>(PublisherConfiguration{.server_address = "kafka:9092"}));
    std::unique_ptr<RDKafkaSubscriber> consumer = std::make_unique<RDKafkaSubscriber>(
        std::make_unique<const SubscriberConfiguration>(SubscriberConfiguration{.server_address = "kafka:9092"})
    );

    ASSERT_NO_THROW(consumer->setQueue({TOPIC_TEST_NAME}));
    PublisherMessageVector push_messages;
    std::vector<std::string> message_to_sent;
    for (int idx = 0; idx < 10; idx++) {
        std::string message_sent = "hello_" + UUID::generateUUIDLite();
        message_to_sent.push_back(message_sent);
        push_messages.push_back(std::move(PublishMessageUniquePtr(new Message("queue-test", message_sent))));
    }

    auto iotaFuture = std::async(
        std::launch::async,
        [&push_messages](std::unique_ptr<IPublisher> producer) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            ASSERT_EQ(producer->pushMessages(push_messages), 0);
            ASSERT_EQ(producer->flush(1000), 0);
        },
        std::move(producer));

    int to_fetch = message_to_sent.size();
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_EQ(consumer->getMsg(tmp_received_messages, to_fetch, 1000), 0);
        received_messages.insert(received_messages.end(), tmp_received_messages.begin(), tmp_received_messages.end());
        tmp_received_messages.clear();
    } while (received_messages.size() != message_to_sent.size());
    ASSERT_NO_THROW(consumer->commit(););

    ASSERT_EQ(received_messages.size(), message_to_sent.size());

    for (int idx = 0; idx < received_messages.size(); idx++) {
        ASSERT_EQ(received_messages[idx]->data_len, message_to_sent[idx].size());
        ASSERT_EQ(std::memcmp(received_messages[idx]->data.get(),
                              message_to_sent[idx].c_str(),
                              received_messages[idx]->data_len),
                  0);
    }

    iotaFuture.wait();
}
