#ifndef ekg_COMMON_UTILITY_H_
#define ekg_COMMON_UTILITY_H_

#include <memory>
namespace ekg
{
    namespace common
    {
        template <typename T>
        std::shared_ptr<T> toShared(std::weak_ptr<T> w)
        {
            std::shared_ptr<T> s = w.lock();
            if (!s)
                throw std::runtime_error("Error getting shared ptr for : " + std::string(typeid(w).name()));
            return s;
        }
    }
}
#endif // ekg_COMMON_UTILITY_H_
