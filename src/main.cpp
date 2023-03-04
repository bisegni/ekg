#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ekg/EKGateway.h>

int main(int argc, char *argv[])
{
    ekg::EKGateway g;
    return g.run(argc, const_cast<const char **>(argv));
}