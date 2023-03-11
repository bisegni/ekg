#include <ekg/controller/command/CMDController.h>

#include <ekg/service/ServiceResolver.h>


#include <boost/json.hpp>

using namespace ekg::service;
using namespace ekg::controller::command;
using namespace ekg::service::log;
using namespace ekg::service::pubsub;


using namespace boost::json;

CMDController::CMDController(ConstCMDControllerConfigUPtr configuration,
                             CMDControllerCommandHandler cmd_handler)
    : configuration(std::move(configuration))
    , cmd_handler(cmd_handler)
    , logger(ServiceResolver<ILogger>::resolve())
    , subscriber(ServiceResolver<ISubscriber>::resolve()) {
    start();
}

CMDController::~CMDController() { stop(); }

void CMDController::consume() {
    SubscriberInterfaceElementVector received_message;
    while (run) {
        // fetch message
        subscriber->getMsg(received_message, configuration->max_message_to_fetch, configuration->fetch_time_out);
        if (received_message.size()) {
            CommandConstShrdPtrVec result_vec;
            std::for_each(received_message.begin(),
                          received_message.end(),
                          [&logger = logger, &result_vec = result_vec](auto message) {
                              if (!message->data_len) return;
                              error_code ec;
                              object command_description;
                              boost::json::string_view value_str =
                                  boost::json::string_view(message->data.get(), message->data_len);
                              try {
                                  command_description = boost::json::parse(value_str, ec).as_object();

                                  if (ec) {
                                      logger->logMessage("Error: '" + ec.message() + "' parsing command: "
                                                             + std::string(message->data.get(), message->data_len),
                                                         LogLevel::ERROR);
                                      return;
                                  }
                                  // parse the command and call the handler
                                  if (auto v = MapToCommand::parse(command_description)) {
                                      result_vec.push_back(v);
                                  }
                              } catch (std::exception& ex) {
                                  logger->logMessage("Error: '" + std::string(ex.what()) + "' parsing command: "
                                                         + std::string(message->data.get(), message->data_len),
                                                     LogLevel::ERROR);
                              }
                          });
            try {
                // dispatch the received command
                if(result_vec.size()) {cmd_handler(result_vec);}
                // at this point we can commit, in sync mode,  the offset becaus all
                // mesage has been maaged
                subscriber->commit();
            } catch (...) {
                logger->logMessage("Error occured during command processing", LogLevel::ERROR);
            }
            received_message.clear();
        }
        // wait
        std::this_thread::sleep_for(std::chrono::microseconds(250));
    }
}

void CMDController::start() {
    logger->logMessage("Starting command controller");
    logger->logMessage("Receive command message from: " + configuration->topic_in);
    subscriber->setQueue({configuration->topic_in});
    run = true;
    t_subscriber = std::thread(&CMDController::consume, this);
}

void CMDController::stop() {
    if (t_subscriber.joinable()) {
        run = false;
        t_subscriber.join();
    }
    logger->logMessage("Stopping command controller");
}