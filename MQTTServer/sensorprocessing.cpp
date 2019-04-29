#include "sensorprocessing.h"

SensorProcessing::SensorProcessing()
{
    _dataStore = &GlobalDataStore::Instance();
}


void SensorProcessing::ProcessTempData()
{

}

void SensorProcessing::ProcessAccelData()
{

}

std::vector<std::function<void()>> SensorProcessing::GetAllSensorProcessors()
{
      return {
             std::bind(&SensorProcessing::ProcessAccelData, this),
             std::bind(&SensorProcessing::ProcessTempData, this)
             };
}
