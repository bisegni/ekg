#include <gtest/gtest.h>

#include <ekg/common/ProgramOptions.h>
#include <ekg/controller/command/CMDController.h>
#include <ekg/service/ServiceResolver.h>
#include <ekg/service/log/ILogger.h>
#include <ekg/service/log/impl/BoostLogger.h>

#include <ekg/service/pubsub/pubsub.h>

#include <ekg/common/uuid.h>
#include <filesystem>

#include <boost/json.hpp>
#include <tuple>

using namespace ekg::common;

using namespace ekg::controller::command;

using namespace ekg::service;
using namespace ekg::service::log;
using namespace ekg::service::log::impl;
using namespace ekg::service::pubsub;
using namespace ekg::service::pubsub::impl::kafka;

using namespace boost::json;

#define KAFKA_ADDR "kafka:9092"
#define CMD_QUEUE "ekg_cmd_in"

class CMDMessage : public PublishMessage {
    const std::string request_type;
    const std::string distribution_key;
    const std::string queue;
    //! the message data
    const std::string message;

public:
    CMDMessage(const std::string& queue, const std::string& message)
        : request_type("command")
        , distribution_key(UUID::generateUUIDLite())
        , queue(queue)
        , message(message) {}
    virtual ~CMDMessage() = default;

    char* getBufferPtr() { return const_cast<char*>(message.c_str()); }
    size_t getBufferSize() { return message.size(); }
    const std::string& getQueue() { return queue; }
    const std::string& getDistributionKey() { return distribution_key; }
    const std::string& getReqType() { return request_type; }
};

TEST(CMDController, CheckConfiguration) {
    int argc = 1;
    const char* argv[1] = {"epics-ekg-test"};
    CMDControllerCommandHandler handler = [](CommandConstShrdPtrVec received_command) {
    };
    // set environment variable for test
    clearenv();
    setenv("EPICS_ekg_log-on-console", "false", 1);
    setenv("EPICS_ekg_sub-server-address", KAFKA_ADDR, 1);
    setenv("EPICS_ekg_cmd-input-topic", CMD_QUEUE, 1);
    std::unique_ptr<ProgramOptions> opt = std::make_unique<ProgramOptions>();
    ASSERT_NO_THROW(opt->parse(argc, argv));
    ServiceResolver<ILogger>::registerService(std::make_shared<BoostLogger>(opt->getloggerConfiguration()));
    std::unique_ptr<CMDController> cmd_controller = std::make_unique<CMDController>(
        opt->getCMDControllerConfiguration(),
        opt->getSubscriberConfiguration(),
        handler);
    EXPECT_STREQ(cmd_controller->configuration->topic_in.c_str(), CMD_QUEUE);
}

TEST(CMDController, StartStop) {
    int argc = 1;
    const char* argv[1] = {"epics-ekg-test"};
    CMDControllerCommandHandler handler = [](CommandConstShrdPtrVec received_command) {
    };
    // set environment variable for test
    clearenv();
    setenv("EPICS_ekg_log-on-console", "false", 1);
    setenv("EPICS_ekg_cmd-input-topic", CMD_QUEUE, 1);
    setenv("EPICS_ekg_sub-server-address", KAFKA_ADDR, 1);
    std::unique_ptr<ProgramOptions> opt = std::make_unique<ProgramOptions>();
    ASSERT_NO_THROW(opt->parse(argc, argv));
    ServiceResolver<ILogger>::registerService(std::make_shared<BoostLogger>(opt->getloggerConfiguration()));
    std::unique_ptr<CMDController> cmd_controller = std::make_unique<CMDController>(
        opt->getCMDControllerConfiguration(),
        opt->getSubscriberConfiguration(),
        handler);
    ASSERT_NO_THROW(cmd_controller.reset(););
}

class CMDControllerCommandTestParametrized
    : public ::testing::TestWithParam<std::tuple<CMDControllerCommandHandler, boost::json::value>> {
    int argc = 1;
    const char* argv[1] = {"epics-ekg-test"};
    std::unique_ptr<CMDController> cmd_controller;

public:
    void SetUp() {
        CMDControllerCommandHandler handler = std::get<0>(GetParam());
        clearenv();
        setenv("EPICS_ekg_log-on-console", "false", 1);
        setenv("EPICS_ekg_cmd-input-topic", CMD_QUEUE, 1);
        setenv("EPICS_ekg_cmd-max-fecth-element", "100", 1);
        setenv("EPICS_ekg_cmd-max-fecth-time-out", "100", 1);
        setenv("EPICS_ekg_sub-server-address", KAFKA_ADDR, 1);
        std::unique_ptr<ProgramOptions> opt = std::make_unique<ProgramOptions>();
        ASSERT_NO_THROW(opt->parse(argc, argv));
        ServiceResolver<ILogger>::registerService(std::make_shared<BoostLogger>(opt->getloggerConfiguration()));
        ASSERT_NO_THROW(
            cmd_controller = std::make_unique<CMDController>(
                opt->getCMDControllerConfiguration(),
                opt->getSubscriberConfiguration(),
                handler););
    }

    void TearDown() { cmd_controller.reset(); }
};

TEST_P(CMDControllerCommandTestParametrized, CheckCommand) {
    boost::json::value message_json = std::get<1>(GetParam());
    // start producer for send command
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::unique_ptr<IPublisher> publisher = std::make_unique<RDKafkaPublisher>(
        std::make_unique<const PublisherConfiguration>(PublisherConfiguration{.server_address = KAFKA_ADDR}));
    ASSERT_NO_THROW(publisher->init(););
    publisher->pushMessage(std::make_unique<CMDMessage>(CMD_QUEUE, serialize(message_json)));
    publisher->flush(100);
}

//------------------------------ command tests -------------------------
CMDControllerCommandHandler acquire_test = [](CommandConstShrdPtrVec received_command) {
    ASSERT_EQ(received_command.size(), 1);
    ASSERT_EQ(received_command[0]->type, CommandType::monitor);
    ASSERT_EQ(received_command[0]->protocol.compare("pv"), 0);
    ASSERT_EQ(received_command[0]->channel_name.compare("channel::a"), 0);
    ASSERT_EQ(reinterpret_cast<const AquireCommand*>(received_command[0].get())->activate, true);
    ASSERT_EQ(
        reinterpret_cast<const AquireCommand*>(received_command[0].get())->destination_topic.compare("topic-dest"), 0);
};
boost::json::value acquire_json = {{KEY_COMMAND, "monitor"},
                                   {KEY_ACTIVATE, true},
                                   {KEY_PROTOCOL, "pv"},
                                   {KEY_CHANNEL_NAME, "channel::a"},
                                   {KEY_DEST_TOPIC, "topic-dest"}};

CMDControllerCommandHandler get_test = [](CommandConstShrdPtrVec received_command) {
    ASSERT_EQ(received_command.size(), 1);
    ASSERT_EQ(received_command[0]->type, CommandType::get);
    ASSERT_EQ(received_command[0]->protocol.compare("pv"), 0);
    ASSERT_EQ(received_command[0]->channel_name.compare("channel::a"), 0);
    ASSERT_EQ(reinterpret_cast<const GetCommand*>(received_command[0].get())->destination_topic.compare("topic-dest"),
              0);
};
boost::json::value get_json = {
    {KEY_COMMAND, "get"}, {KEY_PROTOCOL, "pv"}, {KEY_CHANNEL_NAME, "channel::a"}, {KEY_DEST_TOPIC, "topic-dest"}};

CMDControllerCommandHandler put_test = [](CommandConstShrdPtrVec received_command) {
    ASSERT_EQ(received_command.size(), 1);
    ASSERT_EQ(received_command[0]->type, CommandType::put);
    ASSERT_EQ(received_command[0]->protocol.compare("pv"), 0);
    ASSERT_EQ(received_command[0]->channel_name.compare("channel::a"), 0);
    ASSERT_EQ(reinterpret_cast<const PutCommand*>(received_command[0].get())->value.compare("set-value"), 0);
};
boost::json::value put_json = {
    {KEY_COMMAND, "put"}, {KEY_PROTOCOL, "pv"}, {KEY_CHANNEL_NAME, "channel::a"}, {KEY_VALUE, "set-value"}};

CMDControllerCommandHandler info_test = [](CommandConstShrdPtrVec received_command) {
    ASSERT_EQ(received_command.size(), 1);
    ASSERT_EQ(received_command[0]->type, CommandType::info);
    ASSERT_EQ(received_command[0]->protocol.compare("pv"), 0);
    ASSERT_EQ(received_command[0]->channel_name.compare("channel::a"), 0);
    ASSERT_EQ(reinterpret_cast<const GetCommand*>(received_command[0].get())->destination_topic.compare("topic-dest"),
              0);
};
boost::json::value info_json = {
    {KEY_COMMAND, "info"}, {KEY_PROTOCOL, "pv"}, {KEY_CHANNEL_NAME, "channel::a"}, {KEY_DEST_TOPIC, "topic-dest"}};

INSTANTIATE_TEST_CASE_P(CMDControllerCommandTest,
                        CMDControllerCommandTestParametrized,
                        ::testing::Values(std::make_tuple(acquire_test, acquire_json),
                                          std::make_tuple(get_test, get_json),
                                          std::make_tuple(put_test, put_json),
                                          std::make_tuple(info_test, info_json)));
