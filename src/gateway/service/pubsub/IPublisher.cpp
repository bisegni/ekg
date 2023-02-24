#include <gateway/service/pubsub/IPublisher.h>

using namespace gateway::pubsub;

IPublisher::IPublisher(){}

IPublisher::~IPublisher() {}

int IPublisher::setCallBackForReqType(const std::string req_type, EventCallback eventCallback) {
    auto ret = eventCallbackForReqType.insert(MapEvtHndlrForReqTypePair(req_type, eventCallback));
    return ret.second==false?-1:1;
}