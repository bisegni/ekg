#include <gtest/gtest.h>
#include <epics/EpicsChannel.h>
#include <epics/EpicsChannelMonitor.h>
#include <thread>
#include <chrono>
TEST(EpicsTest, ChannelFault) {
    std::unique_ptr<EpicsChannel> pc;
    EXPECT_NO_THROW(pc = std::make_unique<EpicsChannel>("ca","bacd_channel_name"));
    EXPECT_NO_THROW(pc->connect());
    EXPECT_ANY_THROW(pc->getData());
}

TEST(EpicsTest, ChannleOK) {
    std::unique_ptr<EpicsChannel> pc;
    epics::pvData::PVStructure::const_shared_pointer val;
    EXPECT_NO_THROW(pc = std::make_unique<EpicsChannel>("pva", "variable:sum"););
    EXPECT_NO_THROW(pc->connect());
    EXPECT_NO_THROW(val = pc->getData(););
    EXPECT_NE(val, nullptr);
}

bool retry_eq(const EpicsChannel& channel, 
                const std::string & name, 
                double value, 
                int mseconds, 
                int retry_times ) {
    for (
        int times = retry_times;
    times!=0;
    times--){
        auto val = channel.getData();
        if(val->getSubField<epics::pvData::PVDouble>(name)->get()==value){
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(mseconds));
    }
    return false;
}

TEST(EpicsTest, ChannelGetSetTemplatedGet) {
    std::unique_ptr<EpicsChannel> pc_sum;
    std::unique_ptr<EpicsChannel> pc_a;
    std::unique_ptr<EpicsChannel> pc_b;
    epics::pvData::PVStructure::const_shared_pointer val;
    EXPECT_NO_THROW(pc_sum = std::make_unique<EpicsChannel>("pva", "variable:sum"););
    EXPECT_NO_THROW(pc_a = std::make_unique<EpicsChannel>("pva", "variable:a"););
    EXPECT_NO_THROW(pc_b = std::make_unique<EpicsChannel>("pva", "variable:b"););
    EXPECT_NO_THROW(pc_sum->connect());
    EXPECT_NO_THROW(pc_a->connect());
    EXPECT_NO_THROW(pc_b->connect());
    EXPECT_NO_THROW(pc_a->putData<int32_t>("value", 0););
    EXPECT_NO_THROW(pc_b->putData<int32_t>("value", 0););
    EXPECT_EQ(retry_eq(*pc_sum, "value", 0, 500, 3), true);
    EXPECT_NO_THROW(pc_a->putData<int32_t>("value", 5););
    EXPECT_NO_THROW(pc_b->putData<int32_t>("value", 5););
    EXPECT_EQ(retry_eq(*pc_sum, "value", 10, 500, 3), true);
}

TEST(EpicsTest, ChannelGetSetPVDatadGet) {
    std::unique_ptr<EpicsChannel> pc_sum;
    std::unique_ptr<EpicsChannel> pc_a;
    std::unique_ptr<EpicsChannel> pc_b;
    epics::pvData::PVStructure::const_shared_pointer val;
    EXPECT_NO_THROW(pc_sum = std::make_unique<EpicsChannel>("pva", "variable:sum"););
    EXPECT_NO_THROW(pc_a = std::make_unique<EpicsChannel>("pva", "variable:a"););
    EXPECT_NO_THROW(pc_b = std::make_unique<EpicsChannel>("pva", "variable:b"););
    EXPECT_NO_THROW(pc_sum->connect());
    EXPECT_NO_THROW(pc_a->connect());
    EXPECT_NO_THROW(pc_b->connect());
    EXPECT_NO_THROW(pc_a->putData("value", epics::pvData::AnyScalar(0)););
    EXPECT_NO_THROW(pc_b->putData("value", epics::pvData::AnyScalar(0)););
    EXPECT_EQ(retry_eq(*pc_sum, "value", 0, 500, 3), true);
    EXPECT_NO_THROW(pc_a->putData("value", epics::pvData::AnyScalar(5)););
    EXPECT_NO_THROW(pc_b->putData("value", epics::pvData::AnyScalar(5)););
    EXPECT_EQ(retry_eq(*pc_sum, "value", 10, 500, 3), true);
}

TEST(EpicsTest, ChannelMonitor) {
    std::unique_ptr<EpicsChannel> pc_a;
    epics::pvData::PVStructure::const_shared_pointer val;
    EXPECT_NO_THROW(pc_a = std::make_unique<EpicsChannel>("pva", "variable:a"););
    EXPECT_NO_THROW(pc_a->connect());
        //enable monitor
    EXPECT_NO_THROW(pc_a->putData("value", epics::pvData::AnyScalar(0)););
    EXPECT_EQ(retry_eq(*pc_a, "value", 0, 500, 3), true);

    EXPECT_NO_THROW(pc_a->startMonitor(););
    MonitorEventVecShrdPtr fetched = pc_a->monitor();
    EXPECT_EQ(fetched->size(), 1);
    EXPECT_EQ(fetched->at(0)->type, MonitorType::Data);
    EXPECT_EQ(fetched->at(0)->data->getSubField<epics::pvData::PVDouble>("value")->get(), 0);

    EXPECT_NO_THROW(pc_a->putData("value", epics::pvData::AnyScalar(1)););
    EXPECT_NO_THROW(pc_a->putData("value", epics::pvData::AnyScalar(2)););

    fetched = pc_a->monitor();
    EXPECT_EQ(fetched->size(), 2);
    EXPECT_EQ(fetched->at(0)->type, MonitorType::Data);
    EXPECT_EQ(fetched->at(0)->data->getSubField<epics::pvData::PVDouble>("value")->get(), 1);
    EXPECT_EQ(fetched->at(1)->type, MonitorType::Data);
    EXPECT_EQ(fetched->at(1)->data->getSubField<epics::pvData::PVDouble>("value")->get(), 2);
    EXPECT_NO_THROW(pc_a->stopMonitor(););
}

MonitorEventVec allEvent;
std::mutex check_result_mutex;
void handler(const MonitorEventVecShrdPtr& event_data) {
    std::lock_guard guard(check_result_mutex);
    allEvent.insert(allEvent.end(), (*event_data).begin(), (*event_data).end());
}

TEST(EpicsTest, EpicsChannelMonitor) {
    std::unique_ptr<EpicsChannelMonitor> monitor = std::make_unique<EpicsChannelMonitor>();

    EXPECT_NO_THROW(monitor->setHandler(std::bind(handler, std::placeholders::_1)););
    EXPECT_NO_THROW(monitor->addChannel("variable:sum"););
    EXPECT_NO_THROW(monitor->start(););
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
    EXPECT_NO_THROW(monitor->stop(););
    EXPECT_EQ(allEvent.size() > 0, true);
}