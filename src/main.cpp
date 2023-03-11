#include <cstdlib>
#include <ekg/EKGateway.h>
#include <iomanip>
#include <iostream>

ekg::EKGateway g;

void event_handler(int signum) {
    if ((signum == SIGABRT) || (signum == SIGSEGV)) {
        std::cerr << "INTERNAL ERROR, please provide log, Catch SIGNAL: " << signum;
    } else {
        g.stop();
    }
}

int main(int argc, char* argv[]) {
    if (signal((int)SIGINT, event_handler) == SIG_ERR) {
        std::cerr << "SIGINT Signal handler registration error";
        return EXIT_FAILURE;
    }

    if (signal((int)SIGTERM, event_handler) == SIG_ERR) {
        std::cerr << "SIGTERM Signal handler registration error";
        return EXIT_FAILURE;
    }

    return g.run(argc, const_cast<const char**>(argv));
}