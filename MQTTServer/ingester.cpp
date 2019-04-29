#include "ingester.h"

Ingester::Ingester(QObject *parent) : QObject(parent)
{
    _dataStore = &GlobalDataStore::Instance();

}


Ingester::~Ingester()
{
}

bool Ingester::Startup()
{
    if(!_threadRunning)
    {
        _threadRunning = true;
        _ingesterThread = std::thread(&Ingester::IngesterThreadRun, this);
        return true;
    }
    return false;
}


void Ingester::IngesterThreadRun()
{
    std::cout << "[Ingester] Thread started" << std::endl;
    while(!_dataStore->Quitting)
    {
        auto newData = _dataStore->queuedData.PopData();
        if(newData)
        {
            std::cout << "[Ingester] NEW DATA" << newData.value_or(MQTTDataPoint())._data.toStdString() << std::endl;
            Process(newData.value_or(MQTTDataPoint()));
        }
        else {
            std::cout << "[Ingester] FALSE FLAG" << std::endl;
        }
    }
    std::cout << "[Ingester] Thread ENDED" << std::endl;
}


void Ingester::Process(const MQTTDataPoint &data)
{
    auto &[mqttSourceString, mqttData, sensorType] = data;
    auto tag = mqttSourceString.split('/').back();

    auto sensor = std::find_if(_dataStore->activeSensors.begin(),
            _dataStore->activeSensors.end(),
            [tag](auto &sensor)
    {
        return sensor->id == tag;
    });

    sensor->get()->IngestData(mqttData);
}
