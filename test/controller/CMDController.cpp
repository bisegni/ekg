#include <gtest/gtest.h>

#include <gateway/common/ProgramOptions.h>
#include <gateway/controller/CMDController.h>
#include <gateway/service/ServiceResolver.h>
#include <gateway/service/log/ILogger.h>
#include <gateway/service/log/impl/BoostLogger.h>

#include <gateway/service/pubsub/pubsub.h>

#include <gateway/common/uuid.h>
#include <filesystem>

#include <boost/json.hpp>
#include <tuple>

using namespace gateway::common;
using namespace gateway::controller;
using namespace gateway::service;
using namespace gateway::service::log;
using namespace gateway::service::log::impl;
using namespace gateway::service::pubsub;
using namespace gateway::service::pubsub::impl::kafka;

using namespace boost::json;

#define KAFKA_ADDR "kafka:9092"
#define CMD_QUEUE "gateway_cmd_in"

class CMDMessage : public PublishMessage
{
    const std::string request_type;
    const std::string distribution_key;
    const std::string queue;
    //! the message data
    const std::string message;

public:
    CMDMessage(const std::string &queue, const std::string &message) : request_type("command"),
                                                                       distribution_key(UUID::generateUUIDLite()),
                                                                       queue(queue),
                                                                       message(message) {}
    virtual ~CMDMessage() = default;

    char *getBufferPtr() { return const_cast<char *>(message.c_str()); }
    size_t getBufferSize() { return message.size(); }
    const std::string &getQueue() { return queue; }
    const std::string &getDistributionKey() { return distribution_key; }
    const std::string &getReqType() { return request_type; }
};

TEST(CMDController, CheckConfiguration)
{
    int argc = 1;
    const char *argv[1] = {"epics-gateway-test"};
    CMDControllerCommandHandler handler = [](CommandConstShrdPtrVec received_command) {};
    // set environment variable for test
    setenv("EPICS_GATEWAY_log-on-console", "false", 1);
    setenv("EPICS_GATEWAY_message-bus-address", KAFKA_ADDR, 1);
    setenv("EPICS_GATEWAY_cmd-input-topic", CMD_QUEUE, 1);
    std::unique_ptr<ProgramOptions> opt = std::make_unique<ProgramOptions>();
    ASSERT_NO_THROW(opt->parse(argc, argv));
    ServiceResolver<ILogger>::registerService(std::make_shared<BoostLogger>(opt->getloggerConfiguration()));
    std::unique_ptr<CMDController> cmd_controller = std::make_unique<CMDController>(opt->getCMDControllerConfiguration(), handler);
    EXPECT_STREQ(cmd_controller->configuration->message_bus_address.c_str(), KAFKA_ADDR);
    EXPECT_STREQ(cmd_controller->configuration->topic_in.c_str(), CMD_QUEUE);
}

TEST(CMDController, StartStop)
{
    int argc = 1;
    const char *argv[1] = {"epics-gateway-test"};
    CMDControllerCommandHandler handler = [](CommandConstShrdPtrVec received_command) {};
    // set environment variable for test
    setenv("EPICS_GATEWAY_log-on-console", "false", 1);
    setenv("EPICS_GATEWAY_message-bus-address", KAFKA_ADDR, 1);
    setenv("EPICS_GATEWAY_cmd-input-topic", CMD_QUEUE, 1);
    std::unique_ptr<ProgramOptions> opt = std::make_unique<ProgramOptions>();
    ASSERT_NO_THROW(opt->parse(argc, argv));
    ServiceResolver<ILogger>::registerService(std::make_shared<BoostLogger>(opt->getloggerConfiguration()));
    std::unique_ptr<CMDController> cmd_controller = std::make_unique<CMDController>(opt->getCMDControllerConfiguration(), handler);
    ASSERT_NO_THROW(cmd_controller->start(););
    ASSERT_NO_THROW(cmd_controller->stop(););
}

class CMDControllerCommandTestParametrized : public ::testing::TestWithParam<std::tuple<CMDControllerCommandHandler, boost::json::value>>
{
    int argc = 1;
    const char *argv[1] = {"epics-gateway-test"};
    std::unique_ptr<CMDController> cmd_controller;

public:
    void SetUp()
    {
        CMDControllerCommandHandler handler = std::get<0>(GetParam());
        setenv("EPICS_GATEWAY_log-on-console", "false", 1);
        setenv("EPICS_GATEWAY_message-bus-address", KAFKA_ADDR, 1);
        setenv("EPICS_GATEWAY_cmd-input-topic", CMD_QUEUE, 1);
        std::unique_ptr<ProgramOptions> opt = std::make_unique<ProgramOptions>();
        ASSERT_NO_THROW(opt->parse(argc, argv));
        ServiceResolver<ILogger>::registerService(std::make_shared<BoostLogger>(opt->getloggerConfiguration()));
        ASSERT_NO_THROW(cmd_controller = std::make_unique<CMDController>(opt->getCMDControllerConfiguration(), handler););
        ASSERT_NO_THROW(cmd_controller->start(););
    }

    void TearDown()
    {
        ASSERT_NO_THROW(cmd_controller->stop(););
    }
};

TEST_P(CMDControllerCommandTestParametrized, CheckCommand)
{
    boost::json::value message_json = std::get<1>(GetParam());
    // start producer for send command
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::unique_ptr<IPublisher> publisher = std::make_unique<RDKafkaPublisher>(KAFKA_ADDR);
    ASSERT_NO_THROW(publisher->init(););
    publisher->pushMessage(std::make_unique<CMDMessage>(CMD_QUEUE, serialize(message_json)));
    publisher->flush(100);
}

//------------------------------ command tests -------------------------
CMDControllerCommandHandler acquire_test = [](CommandConstShrdPtrVec received_command)
{
    ASSERT_EQ(received_command.size(), 1);
    ASSERT_EQ(received_command[0]->type, CommandType::monitor);
    ASSERT_EQ(received_command[0]->protocol.compare("pv"), 0);
    ASSERT_EQ(received_command[0]->channel_name.compare("channel::a"), 0);
    ASSERT_EQ(reinterpret_cast<const AquireCommand *>(received_command[0].get())->activate, true);
    ASSERT_EQ(reinterpret_cast<const AquireCommand *>(received_command[0].get())->destination_topic.compare("topic-dest"), 0);
};
boost::json::value acquire_json = {{KEY_COMMAND, "monitor"},
                                     {KEY_ACTIVATE, true},
                                     {KEY_PROTOCOL, "pv"},
                                     {KEY_CHANNEL_NAME, "channel::a"},
                                     {KEY_DEST_TOPIC, "topic-dest"}};

CMDControllerCommandHandler get_test = [](CommandConstShrdPtrVec received_command)
{
    ASSERT_EQ(received_command.size(), 1);
    ASSERT_EQ(received_command[0]->type, CommandType::get);
    ASSERT_EQ(received_command[0]->protocol.compare("pv"), 0);
    ASSERT_EQ(received_command[0]->channel_name.compare("channel::a"), 0);
    ASSERT_EQ(reinterpret_cast<const GetCommand *>(received_command[0].get())->destination_topic.compare("topic-dest"), 0);
};
boost::json::value get_json = {{KEY_COMMAND, "get"},
                                     {KEY_PROTOCOL, "pv"},
                                     {KEY_CHANNEL_NAME, "channel::a"},
                                     {KEY_DEST_TOPIC, "topic-dest"}};

CMDControllerCommandHandler put_test = [](CommandConstShrdPtrVec received_command)
{
    ASSERT_EQ(received_command.size(), 1);
    ASSERT_EQ(received_command[0]->type, CommandType::put);
    ASSERT_EQ(received_command[0]->protocol.compare("pv"), 0);
    ASSERT_EQ(received_command[0]->channel_name.compare("channel::a"), 0);
    ASSERT_EQ(reinterpret_cast<const PutCommand *>(received_command[0].get())->value.compare("set-value"), 0);
};
boost::json::value put_json = {{KEY_COMMAND, "put"},
                                     {KEY_PROTOCOL, "pv"},
                                     {KEY_CHANNEL_NAME, "channel::a"},
                                     {KEY_VALUE, "set-value"}};

CMDControllerCommandHandler info_test = [](CommandConstShrdPtrVec received_command)
{
    ASSERT_EQ(received_command.size(), 1);
    ASSERT_EQ(received_command[0]->type, CommandType::info);
    ASSERT_EQ(received_command[0]->protocol.compare("pv"), 0);
    ASSERT_EQ(received_command[0]->channel_name.compare("channel::a"), 0);
    ASSERT_EQ(reinterpret_cast<const GetCommand *>(received_command[0].get())->destination_topic.compare("topic-dest"), 0);
};
boost::json::value info_json = {{KEY_COMMAND, "info"},
                                     {KEY_PROTOCOL, "pv"},
                                     {KEY_CHANNEL_NAME, "channel::a"},
                                     {KEY_DEST_TOPIC, "topic-dest"}};

INSTANTIATE_TEST_CASE_P(
    CMDControllerCommandTest,
    CMDControllerCommandTestParametrized,
    ::testing::Values(
        std::make_tuple(acquire_test, acquire_json),
        std::make_tuple(get_test, get_json),
        std::make_tuple(put_test, put_json),
        std::make_tuple(info_test, info_json)));
