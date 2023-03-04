#ifndef ekg_H_
#define ekg_H_

#include <ekg/common/ProgramOptions.h>
#include <memory>

namespace ekg
{
    // Main class
    class EKGateway
    {
        std::unique_ptr<ekg::common::ProgramOptions> po;
        int setup(int argc, const char *argv[]);
    public:
        EKGateway();
        ~EKGateway() = default;
        int run(int argc, const char *argv[]);
    };
}

#endif // ekg_H_