#include <gtest/gtest.h>
#include <ekg/common/utility.h>
#include <ekg/service/data/DataStorage.h>

#include <filesystem>
#include <vector>
#include <thread>
#include <latch>

namespace fs = std::filesystem;
using namespace ekg::common;
using namespace ekg::service::data;
using namespace ekg::service::data::repository;

TEST(DataStorage, Default)
{
    int new_id = 0;
    std::unique_ptr<DataStorage> storage;
    EXPECT_NO_THROW(storage = std::make_unique<DataStorage>(fs::path(fs::current_path()) / "test.sqlite"););
    EXPECT_NO_THROW(toShared(storage->getChannelRepository())->removeAll(););
    EXPECT_NO_THROW(new_id = toShared(storage->getChannelRepository())->insert({.channel_name = "channel::a", .channel_protocol = "pv"}););
    auto found_channel = toShared(storage->getChannelRepository())->getChannel("channel::a");

    EXPECT_EQ(found_channel.has_value(), true);
    EXPECT_EQ(found_channel->get()->id, new_id);
    EXPECT_STREQ(found_channel->get()->channel_name.c_str(), "channel::a");
    EXPECT_STREQ(found_channel->get()->channel_protocol.c_str(), "pv");
}

TEST(DataStorage, MultiThreading)
{
    const int test_element = 10;
    std::shared_ptr<DataStorage> storage;
    std::vector<std::thread> thread_vec;
    std::latch test_done(test_element);
    EXPECT_NO_THROW(storage = std::make_shared<DataStorage>(fs::path(fs::current_path()) / "test.sqlite"););
    EXPECT_NO_THROW(toShared(storage->getChannelRepository())->removeAll(););
    SCOPED_TRACE("Start thread");
    for (int idx = 0; idx < test_element; idx++)
    {
        thread_vec.push_back(
            std::move(
                std::thread([ t_id = idx, &t_storage = storage, &t_latch = test_done]
                            {   
                                int new_id = 0;
                                std::string channel_name = "channel::" + std::to_string(t_id);
                                EXPECT_NO_THROW(
                                    new_id = toShared(t_storage->getChannelRepository())->insert({.channel_name = channel_name, .channel_protocol = "pv"});
                                    ) << channel_name << "[" << new_id << "]";
                                auto found_channel = toShared(t_storage->getChannelRepository())->getChannel(channel_name);

                                EXPECT_EQ(found_channel.has_value(), true);
                                EXPECT_EQ(found_channel->get()->id, new_id);
                                EXPECT_STREQ(found_channel->get()->channel_name.c_str(), channel_name.c_str());
                                EXPECT_STREQ(found_channel->get()->channel_protocol.c_str(), "pv");
                                t_latch.count_down(); })));
    }
    SCOPED_TRACE("Wait thread");
    test_done.wait();
    std::for_each(thread_vec.begin(), thread_vec.end(), [](auto &t){ t.join(); });
    thread_vec.clear();
}