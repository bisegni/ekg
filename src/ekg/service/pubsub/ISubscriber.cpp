#include <ekg/service/pubsub/ISubscriber.h>

using namespace ekg::service::pubsub;

ISubscriber::ISubscriber(ConstSubscriberConfigurationUPtr configuration):configuration(std::move(configuration)){}