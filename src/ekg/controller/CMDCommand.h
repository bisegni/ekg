
#ifndef ekg_CONTROLLER_CMDOPCODE_H_
#define ekg_CONTROLLER_CMDOPCODE_H_

#include <ekg/common/types.h>

#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <any>
#include <memory>
#include <boost/json.hpp>

namespace ekg::controller
{
    #define JSON_VALUE_TO(t, v) boost::json::value_to<t>(v)

    enum class CommandType
    {
        monitor,
        get,
        put,
        info,
        unknown
    };

#define KEY_COMMAND "command"
#define KEY_PROTOCOL "protocol"
#define KEY_CHANNEL_NAME "channel_name"
#define KEY_ACTIVATE "activate"
#define KEY_DEST_TOPIC "dest_topic"
#define KEY_VALUE "value"

    struct Command
    {
        const CommandType type;
        const std::string protocol;
        const std::string channel_name;
    };

/**
 *     {
        "command", "monitor",
        "activate", true|false,
        "protocol", "pv|ca",
        "channel_name", "channel::a",
        "dest_topic", "destination_topic"
        }
*/
    struct AquireCommand : public Command
    {
        const bool activate;
        const std::string destination_topic;
    };

/**
 *     {
        "command", "get",
        "protocol", "pv|ca",
        "channel_name", "channel::a",
        "dest_topic", "destination_topic"
        }
*/
    struct GetCommand : public Command
    {
        const std::string destination_topic;
    };

/**
 *     {
        "command", "put",
        "protocol", "pv|ca",
        "channel_name", "channel::a"
        "value", value"
        }
*/
    struct PutCommand : public Command
    {
        const std::string value;
    };

/**
 *     {
        "command", "info",
        "protocol", "pv|ca",
        "channel_name", "channel::a",
        "dest_topic", "destination_topic"
        }
*/
    struct InfoCommand : public Command
    {
        const std::string destination_topic;
    };

    typedef std::shared_ptr<const Command> CommandConstShrdPtr;
    typedef std::vector<CommandConstShrdPtr> CommandConstShrdPtrVec;

    DEFINE_MAP_FOR_TYPE(std::string, std::any, FieldValuesMap)
    typedef std::unique_ptr<FieldValuesMap> FieldValuesMapUPtr;

    /**
     * class that help to map the json structure to a command
    */
    class MapToCommand
    {
        /**
         * Extract the command type
        */
        static CommandType getCMDType(const boost::json::object &ob);
        /**
         * Verify the presence of all the filed within the json object
        */
        static FieldValuesMapUPtr checkFields(const boost::json::object &obj, const std::vector<std::tuple<std::string, boost::json::kind>>& fields);
    public:
        static CommandConstShrdPtr parse(const boost::json::object &obj);
    };
}

#endif // ekg_CONTROLLER_CMDOPCODE_H_
