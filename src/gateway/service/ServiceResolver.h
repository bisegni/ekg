#ifndef __SERVICERESOLVER_H__
#define __SERVICERESOLVER_H__

#pragma once
#include <map>
#include <string>
#include <memory>
#include <typeinfo>
namespace gateway
{
    namespace service
    {
        template<typename T>
        class ServiceResolver
        {
            static std::shared_ptr<T> registered_instance;
        public:
            static void registerService(std::shared_ptr<T> object) {
                registered_instance = object;
            }
            static T resolve() {
                return registered_instance;
            }
        };

        template <typename T>
        std::shared_ptr<T> ServiceResolver<T>::registered_instance;
    }
}

#endif // __SERVICERESOLVER_H__