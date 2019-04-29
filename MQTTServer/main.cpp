#include <QCoreApplication>

#include <iostream>

#include "globalcontroller.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    GlobalController *gc = new GlobalController();
    if(!gc->Startup())
    {
        std::cout << "Something failed to start up!" << std::endl;
        return 1;
    }
    std::cout << "Everything is running!" << std::endl;

    return a.exec();
}
