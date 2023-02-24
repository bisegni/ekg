#include <gateway/service/pubsub/impl/kafka/RDKafkaPublisher.h>


using namespace gateway::pubsub::impl::kafka;

RDKafkaPublisher::RDKafkaPublisher()
    : IPublisher(), RDKafkaBase(), _stop_inner_thread(false), _auto_poll(false) {}

RDKafkaPublisher::~RDKafkaPublisher() {}

void RDKafkaPublisher::setAutoPoll(bool autopoll) {
  _auto_poll = autopoll;
}

void RDKafkaPublisher::dr_cb(RdKafka::Message &message) {
  // re-gain ownership of the allcoated message release in push methods
  PublishMessageUniquePtr message_managed(static_cast<PublishMessage *>(message.msg_opaque()));
  if (!message_managed) return;
  // check if we have a callback
  EventCallback cb_handler = eventCallbackForReqType[message_managed->getReqType()];
  if (!cb_handler) return;
  switch (message.status()) {
    case RdKafka::Message::MSG_STATUS_NOT_PERSISTED:
    case RdKafka::Message::MSG_STATUS_POSSIBLY_PERSISTED:
      cb_handler(OnError, message_managed.get());
      break;
    case RdKafka::Message::MSG_STATUS_PERSISTED:
      cb_handler(OnSent, message_managed.get());
      break;
    default:
      break;
  }
  if (message.status() != RdKafka::Message::MSG_STATUS_PERSISTED) {
    // RDK_PUB_DBG_ << "Message delivery for (" << message.len() << " bytes): "
                // << ": " << message.errstr();
    if (message.key()) {
      // RDK_PUB_DBG_ << "Key: " << *(message.key()) << ";";
    }
  }
}

void RDKafkaPublisher::init(const std::string &bootstrap_server) {
  std::string                       errstr;

  // RDK_CONF_SET(conf, "debug", "cgrp,topic,fetch,protocol", RDK_PUB_ERR_)
  RDK_CONF_SET(conf, "bootstrap.servers", bootstrap_server.c_str())
  RDK_CONF_SET(conf, "dr_cb", this);
  producer.reset(RdKafka::Producer::create(conf.get(), errstr));
  if (!producer) {
    //RDK_PUB_ERR_ << "Failed to create producer: " << errstr << std::endl;
    throw std::runtime_error("Error creating kafka producer (" + errstr + ")");
  }
  // start polling thread
  if (_auto_poll) {
    auto_poll_thread = std::thread(&RDKafkaPublisher::autoPoll, this);
  }
}

void RDKafkaPublisher::autoPoll() {
  while (!this->_stop_inner_thread) {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    flush(10);
  }
}

void RDKafkaPublisher::deinit() {
  if (_auto_poll) {
    _stop_inner_thread = true;
    auto_poll_thread.join();
  }
}

int RDKafkaPublisher::flush(const int timeo) {
  //RDK_PUB_DBG_ << "Flushing... ";
  while (producer->outq_len() > 0) {
    producer->poll(timeo);
  }
  //RDK_PUB_DBG_ << "Flushing...done ";
  return 0;
}

int RDKafkaPublisher::createQueue(const std::string &queue) {
  std::string     errstr;
  RdKafka::Topic *topic = nullptr;
  if (!queue.empty()) {
    topic = RdKafka::Topic::create(producer.get(), queue, t_conf.get(), errstr);
    if (!topic) {
      //RDK_PUB_ERR_ << "Failed to create topic: " << errstr;
      return -1;
    }
  }
  return 0;
}

int RDKafkaPublisher::pushMessage(PublishMessageUniquePtr message) {
  RdKafka::ErrorCode resp             = RdKafka::ERR_NO_ERROR;
  RdKafka::Headers  *headers          = RdKafka::Headers::create();
  const std::string  distribution_key = message->getDistributionKey();
  // headers->add("packet_num", std::to_string(idx));
  headers->add("custom_chaos_header", "test");
  resp = producer->produce(
      message->getQueue(),
      RdKafka::Topic::PARTITION_UA,
      0 /* zero copy management */,
      /* Value */
      (void *)message->getBufferPtr(),
      message->getBufferSize(),
      /* Key */
      distribution_key.c_str(),
      distribution_key.size(),
      /* Timestamp (defaults to now) */
      0,
      /* Message headers, if any */
      headers,
      /* pass PublishMessage instance to opaque information */
      message.get());
  if (resp != RdKafka::ERR_NO_ERROR) {
    //RDK_PUB_ERR_ << "Producer failed: " << RdKafka::err2str(resp);
    delete headers; /* Headers are automatically deleted on produce() success. */
    return -1;
  } else {
    // whe need to release the message memory becaus is not more owned by this instance
    message.release();
    return 0;
  }
}

int RDKafkaPublisher::pushMessages(PublisherMessageVector &messages) {
  int  err = 0;
  auto message = messages.begin();

  while (message != messages.end()) {
    if (!(err = pushMessage(std::move(*message))))
      message = messages.erase(message);
    else
      break;
  }
  return !err ? 0 : -1;
}

size_t RDKafkaPublisher::getQueueMessageSize() {
  return 0;
}
