#include <sqlite_orm/sqlite_orm.h>

namespace gateway::service::data
{

    class DataRepository
    {
        sqlite_orm:;
        storage database;

    public:
        DataRepository() = default;
        ~DataRepository() = default;
    }

}