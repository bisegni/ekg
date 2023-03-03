
#include <sqlite_orm/sqlite_orm.h>
#include <gateway/service/data/repository/ChannelRepository.h>

#include <memory>
#include <mutex>
namespace gateway::service::data
{
    /**
     *
     */
    inline auto initStorage(const std::string &path)
    {
        using namespace sqlite_orm;
        return make_storage(path,
                            make_table("channel",
                                       make_column("id", &repository::ChannelType::id, primary_key().autoincrement()),
                                       make_column("channel_name", &repository::ChannelType::channel_name),
                                       make_column("channel_protocol", &repository::ChannelType::channel_protocol)));
    }

    using Storage = decltype(initStorage(""));

    /**
     *
     */
    inline std::shared_ptr<Storage> getStorageFromWPtr(std::weak_ptr<Storage> ws)
    {
        auto ss = ws.lock();
        if (!ss)
            throw std::runtime_error("Error locking storage: " + std::string(typeid(ss).name()));
        return ss;
    }

    /**
     * Contains lockable access to storage for multithreading operation
     */
    struct StorageLockedRef
    {
        StorageLockedRef(
            std::unique_lock<std::recursive_mutex>&& s_lock,
            std::weak_ptr<Storage>&&ss_wptr):s_lock(std::move(s_lock)),ss_wptr(std::move(ss_wptr)) {}
        std::weak_ptr<Storage> ss_wptr;
        /*
        * thelock is owned on the first call of this method
        * and is release when the StorageLockedRef instance
        * is destroyed
        */
        std::shared_ptr<Storage> get() {
            //cquire lock
            s_lock.lock();
            return std::move(getStorageFromWPtr(ss_wptr));
        }
        private:
        std::unique_lock<std::recursive_mutex> s_lock;
    };

    /**
     *
     */
    class DataStorage
    {
        std::mutex repository_access_mutex;
        std::recursive_mutex storage_mutex;
        std::shared_ptr<Storage> storage;
        std::shared_ptr<repository::ChannelRepository> channel_repository_instance;

    public:
        DataStorage(const std::string &path);
        ~DataStorage() = default;
        std::weak_ptr<repository::ChannelRepository> getChannelRepository();
        /**
         * The result @StorageLockedRef reference should be used for single operation
        */
        StorageLockedRef getLockedStorage();
    };

}