#ifndef IPUBLISHER_H
#define IPUBLISHER_H

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <functional>

#pragma once
namespace gateway::service::pubsub
    {
        /*
        * Message publish interface *implementation need to internally manage the implmenetaion instance
        */
        class PublishMessage
        {
        public:
            virtual ~PublishMessage() {}
            virtual char *getBufferPtr() = 0;
            virtual size_t getBufferSize() = 0;
            virtual const std::string &getQueue() = 0;
            virtual const std::string &getDistributionKey() = 0;
            virtual const std::string &getReqType() = 0;
        };

        typedef std::unique_ptr<PublishMessage> PublishMessageUniquePtr;

        typedef std::vector<PublishMessageUniquePtr> PublisherMessageVector;

        typedef enum EventType
        {
            OnDelivery,
            OnSent,
            OnError
        } EventType;

        // Callback called after the message has been sent
        typedef std::function<void(EventType, PublishMessage *const)> EventCallback;
        typedef std::map<std::string, EventCallback> MapEvtHndlrForReqType;
        typedef std::pair<std::string, EventCallback> MapEvtHndlrForReqTypePair;

        class IPublisher {
        protected:
            MapEvtHndlrForReqType eventCallbackForReqType;
        public:
            IPublisher() = default;
            virtual ~IPublisher() = default;
            virtual void init() = 0;
            virtual void deinit() = 0;
            virtual void setAutoPoll(bool autopoll) = 0;
            //! PublisherInterface initialization
            virtual int setCallBackForReqType(const std::string req_type, EventCallback eventCallback);
            virtual int createQueue(const std::string &queue) = 0;
            virtual int flush(const int timeo) = 0;
            virtual int pushMessage(PublishMessageUniquePtr message) = 0;
            virtual int pushMessages(PublisherMessageVector &messages) = 0;
            virtual size_t getQueueMessageSize() = 0;
        };

    }

#endif