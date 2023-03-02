#include <gateway/service/data/DataStorage.h>

using namespace gateway::service::data;
using namespace gateway::service::data::repository;

using namespace sqlite_orm;

#define PUBLIC_SHARED_ENABLER(x)                                                      \
    struct x##SharedEnabler : public x                                                \
    {                                                                                 \
        x##SharedEnabler(DataStorage &data_storage) : ChannelRepository(data_storage) \
        {                                                                             \
        }                                                                             \
    };

DataStorage::DataStorage(const std::string &path)
{
    storage = std::make_shared<Storage>(initStorage(path));
    if (!storage)
    {
        throw std::runtime_error("Error creating database");
    }
    storage->sync_schema();
}

std::weak_ptr<repository::ChannelRepository> DataStorage::getChannelRepository()
{
    PUBLIC_SHARED_ENABLER(ChannelRepository)
    std::unique_lock<std::mutex> m(repository_access_mutex);
    if (!channel_repository_instance)
    {
        channel_repository_instance = std::make_shared<ChannelRepositorySharedEnabler>(*this);
    }
    return channel_repository_instance;
}

StorageLockedRef DataStorage::getLockedStorage()
{
    //create unique lock without owning the lock
    return std::move(StorageLockedRef(
            std::move(std::unique_lock<std::recursive_mutex>(storage_mutex, std::defer_lock)),
            storage
            ));
}