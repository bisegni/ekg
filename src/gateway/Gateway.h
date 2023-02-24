#ifndef GATEWAY_H_
#define GATEWAY_H_

#include <gateway/common/ProgramOptions.h>

namespace gateway
{
    // Main class
    class Gateway
    {
        std::unique_ptr<gateway::common::ProgramOptions> po;
        int setup(int argc, const char *argv[]);
    public:
        Gateway();
        ~Gateway() = default;
        int run(int argc, const char *argv[]);
    };
}

#endif // GATEWAY_H_