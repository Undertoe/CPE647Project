#ifndef PUBLISHDATA_H
#define PUBLISHDATA_H

#include <QString>
#include <QJsonDocument>
#include <QVariant>

#include <optional>

#include "gclouddata.h"
struct PublishData
{
    PublishData(const QString &timeString, const QString &sensorName, const QString &eventName) :
        _timeString(timeString),
        _sensorName(sensorName),
        _eventName(eventName)
    {
        _gCloudata = &GlobalGCloudData::Instance();
    }

    QByteArray BuildJSON()
    {
        QVariantMap information;
        information.insert("SensorName", _sensorName);
        information.insert("EventName", _eventName);

        QVariantMap jsonMap;
//        jsonMap.insert(_timeString, information);

        jsonMap.insert("TIME", _timeString);
        jsonMap.insert("data", information);

//        QJsonDocument doc = QJsonDocument::fromVariant(QVariant(jsonMap));
//        return doc.toJson();
        return QJsonDocument::fromVariant(QVariant(jsonMap)).toJson();
    }

    GoogleCloudData *_gCloudata;
    QString _timeString;
    QString _sensorName;
    QString _eventName;
};

#endif // PUBLISHDATA_H
