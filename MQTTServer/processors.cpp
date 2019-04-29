#include "processors.h"

Processors::Processors(QObject *parent) : QObject(parent)
{
    _dataStore = &GlobalDataStore::Instance();
    _sensorProcessors = new SensorProcessing();
}

bool Processors::Init()
{
    if(!_threadRunning)
    {
        _threadRunning = true;
        _newDataStreamListener = std::thread(&Processors::NewDataStreamListenerRun, this);
        return true;
    }
    return false;

}


void Processors::NewDataStreamListenerRun()
{
    while(_threadRunning)
    {
        auto mTag = _dataStore->newDataStreamsProcessing.PopData();
        if(!mTag)
        {
            return;
        }
        auto tag = mTag.value_or("");

        _rTProcessingThreads.emplace_back(&Processors::RTProcessingThreadRun, this, tag);
        _endOfThreadDSListener.emplace_back(&Processors::EndOfThreadDSListenerRun, this, tag);
    }
}

void Processors::RTProcessingThreadRun(const QString &tag)
{
    AbstractSensor *sensor = _dataStore->getSensor(tag);
    while(_threadRunning && sensor != nullptr)
    {
        sensor->Wait();
        if(sensor == nullptr || sensor->Closing())
            return;
        sensor->ProcessData();
    }

}

void Processors::EndOfThreadDSListenerRun(const QString &tag)
{
    AbstractSensor *sensor = _dataStore->getSensor(tag);
    switch(sensor->type)
    {
    case SensorType::IMU:
    {
        IMUSensor* imu = dynamic_cast<IMUSensor*>(sensor);
        while(_threadRunning)
        {
            imu->dataCompleted.Wait();
            if(!imu)
            {
                return;
            }
            if(imu->disconnecting)
            {
                std::cout << "DATA COMPLETED DETECTED AS CLOSED" << std::endl;
                return;
            }
            auto mData = imu->data->GetData();
            if(!mData)
            {
                return;
            }
            auto timestampedData = imu->fallNumbers;
            std::cout << "PRINTING DATA" << std::endl;
            auto data = mData.value_or(std::deque<XYZ>());
            std::cout << "DATA SIZE: " << data.size() << std::endl;
            QString now = QDateTime::currentDateTime().toString("MM.d.yy, hh.mm.ss.zzz");

            std::ofstream output;
            output.open(_dataStore->Directory.toStdString() + now.toStdString() + "_" +  imu->id.toStdString()  + ".txt");
            if(!output.is_open())
            {
                std::cout << "OUTPUT FILE FAILED TO OPEN!!" << std::endl;
                return;
            }
            if(data.size() > 0)
            {
                data.at(0).Header(output);
                for(const auto & point : data)
                {
                    output << point;
                    if(timestampedData.size() > 0)
                    {
                        if(timestampedData.front() == point.timestamp)
                        {
                            output << "\tFALL OCCURED HERE";
                            timestampedData.pop_front();
                        }
                    }
                    output << std::endl;
                }
            }

            std::ofstream csv;
            csv.open(_dataStore->Directory.toStdString() + now.toStdString() + "_" +  imu->id.toStdString()  + ".csv");
            if(!csv.is_open())
            {
                std::cout << "OUTPUT FILE FAILED TO OPEN!!" << std::endl;
                return;
            }
            if(data.size() > 0)
            {
                data.at(0).Header(output);
                for(auto & point : data)
                {
                    point.OutputForCSV(csv);
                    csv << std::endl;
                }
            }

            std::cout << "FILE PRINTED" << std::endl;

        }
        return;
    }
    default:
        return;
    }

}

/// thhis was an old idea that
//bool Processors::Init()
//{
//    auto functionList = _sensorProcessors->GetAllSensorProcessors();

//    for(auto &func : functionList)
//    {
//        _threadPool.emplace_back(func);
//    }

//    return true;
//}
