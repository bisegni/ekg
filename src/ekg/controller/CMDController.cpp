#include <ekg/controller/CMDController.h>

#include <ekg/service/log/ILogger.h>
#include <ekg/service/ServiceResolver.h>
#include <ekg/service/pubsub/pubsub.h>

#include <boost/json.hpp>

using namespace ekg::service;
using namespace ekg::controller;
using namespace ekg::service::log;
using namespace ekg::service::pubsub;
using namespace ekg::service::pubsub::impl::kafka;

using namespace boost::json;



CMDController::CMDController(
    CMDControllerConfigUPtr configuration,
    CMDControllerCommandHandler cmd_handler)
    : configuration(std::move(configuration)),
      cmd_handler(cmd_handler),
      logger(ServiceResolver<ILogger>::resolve()),
      subscriber(std::make_unique<RDKafkaSubscriber>(configuration->message_bus_address)) {}

void CMDController::consume()
{
    SubscriberInterfaceElementVector received_message;
    while (run)
    {
        // fetch message
        subscriber->getMsg(received_message, 1, 250);
        if (received_message.size())
        {
            CommandConstShrdPtrVec result_vec;
            std::for_each(received_message.begin(), received_message.end(),
                          [&logger = logger, &result_vec = result_vec](auto message)
                          {
                              if (!message->data_len)
                                  return;
                              error_code ec;
                              boost::json::string_view value_str = boost::json::string_view(message->data.get(), message->data_len);
                              object command_description = boost::json::parse(value_str, ec).as_object();
                              if (ec)
                              {
                                  logger->logMessage("Error:" + ec.message() + " parsing command: " + std::string(message->data.get(), message->data_len));
                                  return;
                              }
                              // parse the command and call the handler
                              if(auto v = MapToCommand::parse(command_description)) {
                                    result_vec.push_back(v);
                              }
                          });
            cmd_handler(result_vec);
            received_message.clear();
        }
        // wait
        std::this_thread::sleep_for(std::chrono::microseconds(250));
    }
}

void CMDController::start()
{
    logger->logMessage("Starting command controller");
    logger->logMessage("Connecting to: " + configuration->message_bus_address);
    logger->logMessage("Receive command message from: " + configuration->topic_in);
    subscriber->init();
    subscriber->setQueue({configuration->topic_in});
    run = true;
    t_subscriber = std::thread(&CMDController::consume, this);
}

void CMDController::stop()
{
    if (t_subscriber.joinable())
    {
        run = false;
        t_subscriber.join();
    }
    logger->logMessage("Stopping command controller");
    subscriber->deinit();
}