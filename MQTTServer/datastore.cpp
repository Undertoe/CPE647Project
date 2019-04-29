#include "datastore.h"

DataStore::DataStore()
{
}


void DataStore::Subscribe(const QString &tag, SensorType type)
{
    using namespace std::chrono_literals;
    switch(type)
    {
    case SensorType::IMU:
        QString topic = mqttStrings.SensorBase + tag;
        activeSensors.push_back(std::make_unique<IMUSensor>(tag, topic));
        sensorTags[type].push_back(tag);
        tagTopicMap[tag] = topic;
        lastPongs[tag] = 0s;
        tags.push_back(tag);
        ServerPingRequestStrings[tag] = mqttStrings.ServerPingBase + tag;
        ServerPongResponseStrings[tag] = mqttStrings.ServerPongBase + tag;
        SensorPingRequestStrings[tag] = mqttStrings.SensorPingBase + tag;
        SensorPongResponseStrings[tag] = mqttStrings.SensorPongBase + tag;

        /// alert the processor of a new datastream
        newDataStreamsProcessing.AddData(tag);
        newDataStreamsNotifications.AddData(tag);
        break;
//    default:
//        return;
    }
}

void DataStore::Unsubscribe(const QString &tag, SensorType type)
{
//    std::cout << "Unsubscribing tag: " << tag.toStdString() << std::endl;

    auto &registerdTags = sensorTags[type];
    if(Terryn::Contains(registerdTags, tag))
    {
//        std::cout << "Tag found in registered tags.  Removing sensor" << std::endl;
        auto obj = std::find_if(activeSensors.begin(), activeSensors.end(),
                                [tag](auto &obj)
        {
            return obj->id == tag;
        });

        /// we are going to wait to actually disconnect this
        obj->get()->Disconnect();

        /// dont do this because you invalidate the HandlePing() iterator
        /// remove from lastPong
//        lastPongs.erase(tag);

        /// remove tag
        tags.erase(std::remove(tags.begin(), tags.end(), tag));

        /// erase from tag map
        tagTopicMap.erase(tag);


        ServerPingRequestStrings.erase(tag);
        ServerPongResponseStrings.erase(tag);
        SensorPingRequestStrings.erase(tag);
        SensorPongResponseStrings.erase(tag);

//        /// remove from sensortags LAST
        auto tagList = sensorTags[type];

        tagList.erase(std::remove(tagList.begin(), tagList.end(), tag), tagList.end());

        /// disconnect the sensor last
        activeSensors.erase(std::remove(activeSensors.begin(), activeSensors.end(), *obj));

        std::cout << "Erased" << std::endl;
    }
}

AbstractSensor* DataStore::getSensor(const QString &tag)
{
    return std::find_if(activeSensors.begin(), activeSensors.end(),
                            [tag](auto &obj)
    {
        return obj->id == tag;
    })->get();
}

