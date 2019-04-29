#ifndef GLOBALCONTROLLER_H
#define GLOBALCONTROLLER_H


#include "server.h"
#include "processors.h"
#include "ingester.h"
#include "gcinterface.h"

class GlobalController
{
public:
    GlobalController();

    bool Startup();

private:
    Server *server;
    Processors *processor;
    Ingester *ingester;
    GCInterface *gcInterface;
};

#endif // GLOBALCONTROLLER_H
