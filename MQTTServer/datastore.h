#ifndef DATASTORE_H
#define DATASTORE_H


#include <deque>
#include <vector>
#include <map>
#include <chrono>
#include <memory>
#include <optional>

#include <iostream>


#include <QString>

#include "singleton.hh"
#include "container_helpers.hh"

#include "circularlockedqueue.h"
#include "semaphorefix.h"
#include "sensorinfo.h"
#include "imusensor.h"

struct MQTTStrings
{
    const QString ServerPingBase = "server/ping/";
    const QString ServerPongBase = "server/pong/";
    const QString SensorPingBase = "sensor/ping/";
    const QString SensorPongBase = "sensor/pong/";
    const QString SensorBase = "sensors/";
    const QString SensorRequestTopic = "server/TopicRequest";
    const QString SensorRequestTopicResponse = "server/TopicResponse";

};

struct MQTTDataPoint
{
    QString _source;
    QByteArray _data;
    SensorType sensorType;
};



class DataStore
{
public:

    bool Quitting = false;
    CircularLockedQueue<MQTTDataPoint> queuedData;
    CircularLockedQueue<QString> newDataStreamsProcessing;
    CircularLockedQueue<QString> newDataStreamsNotifications;

    /// in all of this, a TAG is known as a subscribed sensor ID,
    /// while the topic is the actual full string.
    /// For example: we have 3 IUMs
    /// Tag for each will be: IMU_1, IMU_2, IMU_3
    /// However, the topic will be as follows:
    ///
    /// sensors/IMU_1 sensors/IMU_2 sensors/IMU_3

    /// list of active sensors
    std::vector<std::unique_ptr<AbstractSensor>> activeSensors;

    /// <sensorType ID, vector<TAG>>
    std::map<SensorType, std::vector<QString>> sensorTags;

    /// <tag , duration since last ping>
    std::map<QString, std::chrono::seconds> lastPongs;

    /// list of active tags we have available
    std::vector<QString> tags;

    /// <tags, topics>
    std::map<QString, QString> tagTopicMap;

    std::map<QString, QString> ServerPingRequestStrings;
    std::map<QString, QString> ServerPongResponseStrings;
    std::map<QString, QString> SensorPingRequestStrings;
    std::map<QString, QString> SensorPongResponseStrings;

    MQTTStrings mqttStrings;


    QString Directory = directory output;

    DataStore();

    void Subscribe(const QString &tag, SensorType type);
    void Unsubscribe(const QString &tag, SensorType type);
    AbstractSensor* getSensor(const QString &tag);


};

using GlobalDataStore = Terryn::Singleton<DataStore>;

#endif // DATASTORE_H
