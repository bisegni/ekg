#ifndef ekg_H_
#define ekg_H_

#include <ekg/common/ProgramOptions.h>
#include <ekg/controller/command/CMDCommand.h>
#include <ekg/controller/node/NodeController.h>

#include <memory>
#include <mutex>
#include <condition_variable>

namespace ekg
{
    // Main class
    class EKGateway
    {   
        bool quit;
        bool terminated;
        std::mutex m;
        std::condition_variable cv;

        ekg::common::ProgramOptionsUPtr po;
        ekg::controller::command::CMDControllerUPtr cmd_controller;
        ekg::controller::node::NodeControllerUPtr node_controller;
        
        int setup(int argc, const char *argv[]);
        void commandReceived(ekg::controller::command::CommandConstShrdPtrVec received_command);
    public:
        EKGateway();
        ~EKGateway() = default;
        int run(int argc, const char *argv[]);
        void stop();
        const bool isTerminated();
        const bool isStopRequested();
    };
}

#endif // ekg_H_