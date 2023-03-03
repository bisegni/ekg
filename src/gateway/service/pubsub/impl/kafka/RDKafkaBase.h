#ifndef RDKafkaBase_H
#define RDKafkaBase_H

#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include <librdkafka/rdkafkacpp.h>
// macro utility
#define RDK_LOG_AND_THROW(msg) \
    throw std::runtime_error("Error creating kafka producer (" + errstr + ")");

#define RDK_CONF_SET(conf, prop, value)                               \
    {                                                                 \
        std::string errstr;                                           \
        if (conf->set(prop, value, errstr) != RdKafka::Conf::CONF_OK) \
        {                                                             \
            RDK_LOG_AND_THROW(errstr)                                 \
        }                                                             \
    }

namespace gateway::service::pubsub::impl::kafka
{
    class RDKafkaBase
    {
    protected:
        std::unique_ptr<RdKafka::Conf> conf;
        std::unique_ptr<RdKafka::Conf> t_conf;

    public:
        RDKafkaBase();
        ~RDKafkaBase();
        int setOption(const std::string &key, const std::string &value);
    };
}

#endif