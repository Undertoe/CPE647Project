#include "imusensor.h"

IMUSensor::IMUSensor(const QString &source, const QString &topic) :
    AbstractSensor()
{
    data = new SensorData<XYZ>();
    type = SensorType::IMU;
    id = source;
    subscriptionTopic = topic;
}

IMUSensor::~IMUSensor()
{
    delete data;
}

void IMUSensor::IngestData(const QByteArray &input)
{
    using namespace std::chrono;
    QString dataString(input);
    if(input == "*")
    {
        data->NewDataStream();
//        notificationHandler.AddData({NotificationType::StreamStarted});
    }
    else if(input == "@")
    {
        std::cout << "End of data stream receieved.  Finalizing: " << std::endl;
        data->Finalize();
        data->ClearOldData();
        std::cout << "Data count: " << data->data.size() << std::endl;
        if(data->activeStream == nullptr)
        {
            std:: cout << "ACTIVE STREAM WAS ACTUALLY DELETED" << std::endl;
        }
        else
        {
            std::cout << "Duration: "
                      << duration_cast<microseconds>(data->activeStream->end -
                                                     data->activeStream->start).count()
                      << "uSec\n";
            dataCompleted.Post();
//            notificationHandler.AddData({NotificationType::StreamStopped});
        }
    }
    else
    {
        if(data->data.size() == 0)
        {
            std::cout << "IGNORING DATA: WE DO NOT HAVE AN INITIALIZED STREAM" << std::endl;
            return;
        }
        std::cout << "Data Points being inserted to IMU: " << dataString.toStdString() << std::endl;
        auto DataPoints = dataString.split(",");
        std::cout << "Point count: " << DataPoints.size() << std::endl;
        XYZ point(DataPoints);
        data->PushData(point);
    }
}

void IMUSensor::ProcessData()
{
    /// don't process the first 15 samples
    if(data->activeStream->fullData.size() < 15)
    {
        return;
    }

    /// acquire last second of data
    std::vector<XYZ> testedData(data->activeStream->fullData.end() - 10, data->activeStream->fullData.end());
    XYZ back = testedData.back();
    auto runTimestamp = back.timestamp;
    /// test fall
    float avg = 0.0;
    float largest = 0.0;
    float mag;
    for(const auto &x : testedData)
    {
        mag = x.mag();
        if(mag > largest)
        {
            largest = mag;
        }
        avg += mag;
    }
    avg /= 10.0F;

    std::cout << "---------------------------------------------------" << std::endl;
    std::cout << "TESTING AVERAGE" << std::endl;
    std::cout << avg << std::endl;
    std::cout << back.mag() << std::endl;
    std::cout << "---------------------------------------------------" << std::endl;

    /// detecting a landing
    if((back.mag() > (avg * 6)) && (back.mag() > 10.0))
    {
        if(fallNumbers.size() > 0)
        {
            if(runTimestamp < fallNumbers.back() + 1000)
            {
                return;
            }
        }
        std::cout << "\n===================================================\n================FALL DETECTED======================\n===================================================\n" << std::endl;
        QString header = "FALL ON RUN: " + QString::number(runTimestamp);
//        notificationType.push_back(NotificationType::ProcessedEvent);
        notificationData.push_back({back, ProcessedEvent::Fall});
//        notificationHandler.AddData({header, ProcessedEvent::Fall});
        notificationHandler.AddData({NotificationType::ProcessedEvent});
        notificationHandler.AddData({NotificationType::PostSensorEvent});
        fallNumbers.push_back(runTimestamp);
    }
}

void IMUSensor::Wait()
{
    data->newDataNotification.Wait();
}

bool IMUSensor::Closing()
{
    return disconnecting;
}


std::tuple<QString, QString> IMUSensor::GetNotificaition(NotificationType type)
{
    switch(type)
    {
    case NotificationType::SensorDisconnected:
    {

        return {"Sensor Disconnected", id + " has Disconnected"};
    }
    case NotificationType::StreamStarted:
    {
        return {"Data Stream Started", QTime::currentTime().toString("HH::mm::ss.zzz") + " data stream started"};
    }
    case NotificationType::StreamStopped:
    {
        return {"Data Stream Stopped", QTime::currentTime().toString("HH::mm::ss.zzz") + " data stream stopped"};
    }
    case NotificationType::ProcessedEvent:
    {
        auto [point, type] = notificationData.front();
        notificationData.pop_front();

        QString header = "";
        QString body = "";
        switch(type)
        {
            case ProcessedEvent::Fall:
            {
                float mag = point.mag();
                header = "Fall Detected!  Sensor: ";
                header += id;
                body = "Total Gravity magnitude: " + QString::number(mag);
                body += " Detected at: " + QTime::currentTime().toString("HH::mm::ss.zzz");
                break;
            }
            default:
            {
                break;
            }
        }

        return {header, body};
    }
    case NotificationType::PostSensorEvent:
    {
        return {"Fall Detected!", ""};
    }
    default:
        std::cout << "\nType: " << NotificationTypeToString(type) << "\n";
        return {"Unknown Event", "Unknown Event"};
    }

}
