#include <gateway/service/data/repository/ChannelRepository.h>
#include <gateway/service/data/DataStorage.h>

#include <iostream>

using namespace gateway::service::data;
using namespace gateway::service::data::repository;

using namespace sqlite_orm;

ChannelRepository::ChannelRepository(DataStorage &data_storage) : data_storage(data_storage) {}

int ChannelRepository::insert(const ChannelType &new_cannel) const
{
    return data_storage.getLockedStorage().get()->insert(new_cannel);
}

void ChannelRepository::removeAll()
{
    return data_storage.getLockedStorage().get()->remove_all<ChannelType>();
}

std::optional<std::unique_ptr<ChannelType>> ChannelRepository::getChannel(const std::string &channel_name)
{
    auto result = data_storage.getLockedStorage().get()->get_all_pointer<ChannelType>(where(c(&ChannelType::channel_name) == channel_name));
    return (result.size()==0) ? std::optional<std::unique_ptr<ChannelType>>() : make_optional(std::make_unique<ChannelType>(*result[0]));
}