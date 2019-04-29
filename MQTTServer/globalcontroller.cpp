#include "globalcontroller.h"

GlobalController::GlobalController()
{
    server = new Server();
    ingester = new Ingester();
    processor = new Processors();
    gcInterface = new GCInterface();
}


bool GlobalController::Startup()
{
    if(!server->Startup())
    {
        std::cout << "Server failed to start" << std::endl;
        return false;
    }

    if(!ingester->Startup())
    {
        std::cout << "Ingester failed to start" << std::endl;
        return false;
    }

    if(!processor->Init())
    {
        std::cout << "Processors failed to start" << std::endl;
        return false;
    }

    if(!gcInterface->Init())
    {
        std::cout << "Processors failed to start" << std::endl;
        return false;
    }

    return true;
}
