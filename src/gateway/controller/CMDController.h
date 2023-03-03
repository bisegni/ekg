#ifndef GATEWAY_SERVICE_CONTROLLER_CMDCONTROLLER_H_
#define GATEWAY_SERVICE_CONTROLLER_CMDCONTROLLER_H_

#include <memory>
#include <string>
#include <thread>
#include <gateway/service/pubsub/ISubscriber.h>
#include <gateway/controller/CMDCommand.h>
namespace gateway::service::log
{
    // forward decalration
    class ILogger;
}

namespace gateway::controller
{
    // configuration
    struct CMDControllerConfig
    {
        // address for the message bus
        const std::string message_bus_address;
        ;
        // the name of the message buss queue where listen for command
        const std::string topic_in;
    };

    typedef std::unique_ptr<const CMDControllerConfig> CMDControllerConfigUPtr;
    typedef std::function<void(CommandConstShrdPtrVec)> CMDControllerCommandHandler;

    /**
     * receive command and dispatch to other layer
     */
    class CMDController
    {
        CMDControllerCommandHandler cmd_handler;
        std::shared_ptr<gateway::service::log::ILogger> logger;
        std::unique_ptr<gateway::service::pubsub::ISubscriber> subscriber;
        std::thread t_subscriber;
        bool run;
        void consume();
    public:
        const CMDControllerConfigUPtr configuration;
        CMDController(
            CMDControllerConfigUPtr configuration,
            CMDControllerCommandHandler cmd_handler);
        CMDController() = delete;
        ~CMDController() = default;
        void start();
        void stop();
    };
}

#endif // GATEWAY_SERVICE_CONTROLLER_CMDCONTROLLER_H_