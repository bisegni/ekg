#include <ekg/controller/CMDCommand.h>
#include <ekg/service/ServiceResolver.h>
#include <ekg/service/log/ILogger.h>

using namespace ekg::controller;

using namespace ekg::service;
using namespace ekg::service::log;
using namespace boost::json;

CommandType MapToCommand::getCMDType(const object &obj)
{
    if (auto v = obj.if_contains(KEY_COMMAND))
    {
        const auto cmd = v->as_string();
        if (cmd.compare("monitor") == 0)
            return CommandType::monitor;
        else if (cmd.compare("get") == 0)
            return CommandType::get;
        else if (cmd.compare("put") == 0)
            return CommandType::put;
        else if (cmd.compare("info") == 0)
            return CommandType::info;
    }
    return CommandType::unknown;
}

FieldValuesMapUPtr MapToCommand::checkFields(const object &obj, const std::vector<std::tuple<std::string, kind>> &fields)
{
    FieldValuesMapUPtr result = std::make_unique<FieldValuesMap>();
    std::for_each(begin(fields), end(fields), [&obj = obj, &result = result](const auto field)
                  { 
                        if( obj.if_contains(std::get<0>(field)) != nullptr){
                            auto v = obj.find(std::get<0>(field))->value();
                            if(v.kind()==std::get<1>(field)) {
                                switch(v.kind()) {
                                    case kind::int64:
                                    result->insert(FieldValuesMapPair(std::get<0>(field), value_to<int64_t>(v)));
                                    break;
                                    case kind::bool_:
                                    result->insert(FieldValuesMapPair(std::get<0>(field), value_to<bool>(v)));
                                    break;
                                     case kind::uint64:
                                    result->insert(FieldValuesMapPair(std::get<0>(field), value_to<uint64_t>(v)));
                                    break;
                                    case kind::double_:
                                    result->insert(FieldValuesMapPair(std::get<0>(field), value_to<double>(v)));
                                    break;
                                    case kind::string:
                                    result->insert(FieldValuesMapPair(std::get<0>(field), value_to<std::string>(v)));
                                    break;                                    
                                }
                                
                            }
                        } });

    return result;
}

CommandConstShrdPtr MapToCommand::parse(const object &obj)
{
#ifdef __DEBUG__
    ServiceResolver<ILogger>::resolve()->logMessage("Received command: " + serialize(obj), LogLevel::DEBUG);
#endif
    CommandConstShrdPtr result = nullptr;
    switch (getCMDType(obj))
    {
    case CommandType::monitor:
        if (auto fields = checkFields(obj,
                                      {{KEY_ACTIVATE, kind::bool_},
                                       {KEY_PROTOCOL, kind::string},
                                       {KEY_CHANNEL_NAME, kind::string},
                                       {KEY_DEST_TOPIC, kind::string}});
            fields != nullptr)
        {
            result = std::make_shared<AquireCommand>(AquireCommand{
                CommandType::monitor,
                std::any_cast<std::string>(fields->find(KEY_PROTOCOL)->second),
                std::any_cast<std::string>(fields->find(KEY_CHANNEL_NAME)->second),
                std::any_cast<bool>(fields->find(KEY_ACTIVATE)->second),
                std::any_cast<std::string>(fields->find(KEY_DEST_TOPIC)->second)});
            break;
        }
    case CommandType::get:
        if (auto fields = checkFields(obj,
                                      {{KEY_PROTOCOL, kind::string},
                                       {KEY_CHANNEL_NAME, kind::string},
                                       {KEY_DEST_TOPIC, kind::string}});
            fields != nullptr)
        {
            result = std::make_shared<GetCommand>(GetCommand{
                CommandType::get,
                std::any_cast<std::string>(fields->find(KEY_PROTOCOL)->second),
                std::any_cast<std::string>(fields->find(KEY_CHANNEL_NAME)->second),
                std::any_cast<std::string>(fields->find(KEY_DEST_TOPIC)->second)});
            break;
        }
    case CommandType::put:
        if (auto fields = checkFields(obj,
                                      {{KEY_PROTOCOL, kind::string},
                                       {KEY_CHANNEL_NAME, kind::string},
                                       {KEY_VALUE, kind::string}});
            fields != nullptr)
        {
            result = std::make_shared<PutCommand>(PutCommand{
                CommandType::put,
                std::any_cast<std::string>(fields->find(KEY_PROTOCOL)->second),
                std::any_cast<std::string>(fields->find(KEY_CHANNEL_NAME)->second),
                std::any_cast<std::string>(fields->find(KEY_VALUE)->second)});
            break;
        }
    case CommandType::info:
        if (auto fields = checkFields(obj,
                                      {{KEY_PROTOCOL, kind::string},
                                       {KEY_CHANNEL_NAME, kind::string},
                                       {KEY_DEST_TOPIC, kind::string}});
            fields != nullptr)
        {
            result = std::make_shared<InfoCommand>(InfoCommand{
                CommandType::info,
                std::any_cast<std::string>(fields->find(KEY_PROTOCOL)->second),
                std::any_cast<std::string>(fields->find(KEY_CHANNEL_NAME)->second),
                std::any_cast<std::string>(fields->find(KEY_DEST_TOPIC)->second)});
            break;
        }
    case CommandType::unknown:
        ServiceResolver<ILogger>::resolve()->logMessage("Command not well formed:" + serialize(obj), LogLevel::ERROR);
        break;
    }
    return result;
}