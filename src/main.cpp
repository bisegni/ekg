#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <gateway/Gateway.h>

int main(int argc, char *argv[])
{
    gateway::Gateway g;
    return g.run(argc, argv);
}