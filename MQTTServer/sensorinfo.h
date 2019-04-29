#ifndef SENSORINFO_H
#define SENSORINFO_H

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QTime>

#include <deque>
#include <vector>
#include <map>
#include <chrono>
#include <optional>
#include <iostream>
#include <ostream>
#include <fstream>
#include <iomanip>

#include <cmath>

#include "semaphorefix.h"
#include "circularlockedqueue.h"


enum class SensorType
{
    IMU,
    Other,
};

static QString SensorTypeString(SensorType type)
{
    switch(type)
    {
    case SensorType::IMU:
        return "IMU";
    case SensorType::Other:
        return "Other";
    }
}

static SensorType SensorTypeFromString(const QString &str)
{
    if(str.contains("IMU"))
    {
        return SensorType::IMU;
    }


    return SensorType::Other;
}

enum class NotificationType
{
    ProcessedEvent,
    StreamStarted,
    StreamStopped,
    SensorDisconnected,
    PostSensorEvent,
    DEFAULT,
};

static std::string NotificationTypeToString(NotificationType t)
{
    switch (t)
    {
    case NotificationType::ProcessedEvent:
        return "ProcessedEvent";
    case NotificationType::StreamStarted:
        return "StreamStarted";
    case NotificationType::StreamStopped:
        return "StreamStopped";
    case NotificationType::SensorDisconnected:
        return "SensorDisconnected";
    case NotificationType::PostSensorEvent:
        return "PostSensorEvent";
    case NotificationType::DEFAULT:
        return "DEFAULT";
    }
}

enum class ProcessedEvent
{
    Fall,
    FlippedOver,
};

static QString EventText(ProcessedEvent e)
{
    switch(e)
    {
    case ProcessedEvent::FlippedOver:
        return "Flipped Over";
    case ProcessedEvent::Fall:
        return "Fell down";
    default:
        return "UNKNOWN";
    }
}


template<typename DataType>
struct SensorDataStream
{
    std::deque<DataType> fullData;
    DataType recentData;

    std::chrono::steady_clock::time_point start;
    std::chrono::steady_clock::time_point end;

    SensorDataStream() = default;

    void PushData(const DataType &d)
    {
        if(fullData.size() == 0)
        {
            start = std::chrono::steady_clock::now();
        }

        recentData = d;
        fullData.push_back(d);
    }

    void Finalize()
    {
        end = std::chrono::steady_clock::now();
    }
};

struct NotificationText
{
    QString title = "";
    QString text = "";

    NotificationText() = default;
    NotificationText(const QString &t, ProcessedEvent event) :
        title(t)
    {
        text = EventText(event);
    }
};


struct AbstractSensor
{
    QString id;
    QString subscriptionTopic;


    CircularLockedQueue<NotificationType> notificationHandler;
    CircularLockedQueue<std::tuple<QString, QString>> postEvent;

    std::deque<NotificationText> text;
//    Terryn::Notification dataCompleted;
    SemaFix dataCompleted;
    SensorType type;
    bool disconnecting = false;

    AbstractSensor()
    {
        dataCompleted.Init(0);
    }

    virtual ~AbstractSensor()
    {
        dataCompleted.Post();
    }

    virtual void IngestData(const QByteArray &input) = 0;
    virtual void ProcessData() = 0;
    virtual void Wait() = 0;
    virtual bool Closing() = 0;
    virtual std::tuple<QString, QString> GetNotificaition(NotificationType type) = 0;

    void Disconnect()
    {
        disconnecting = true;
        dataCompleted.Post();
    }
};


template<typename DataType>
struct SensorData
{
    std::deque<SensorDataStream<DataType>> data;
    SensorDataStream<DataType> *activeStream = nullptr;

    SemaFix newDataNotification;

    SensorData()
    {
//        activeStream = &data.front();
        newDataNotification.Init(0);
    }

    ~SensorData()
    {
    }

    void PushData(const DataType &d)
    {
        activeStream->PushData(d);
        newDataNotification.Post();
    }

    void NewDataStream()
    {
        data.emplace_back();
        activeStream = &data.back();
    }

    SensorDataStream<DataType> LatestStream()
    {
        return data.back();
    }

    /// clears out all old data
    void ClearOldData()
    {
        while(data.size() > 1)
        {
            data.pop_front();
        }
    }

    void Finalize()
    {
        if(activeStream)
            activeStream->Finalize();
    }

    std::optional<std::deque<DataType>> GetData()
    {
        if(activeStream == nullptr)
        {
            return std::nullopt;
        }
        return activeStream->fullData;

    }
};



#endif // SENSORINFO_H
