#ifndef SENSORPROCESSING_H
#define SENSORPROCESSING_H

#include <vector>
#include <functional>

#include "sensorinfo.h"
#include "datastore.h"

class SensorProcessing
{
public:
    SensorProcessing();


    void ProcessAccelData();
    void ProcessTempData();

    std::vector<std::function<void()>> GetAllSensorProcessors();


private:

    DataStore *_dataStore;


};

#endif // SENSORPROCESSING_H
