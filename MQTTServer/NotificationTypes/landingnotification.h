#ifndef LANDINGNOTIFICATION_H
#define LANDINGNOTIFICATION_H

#include <QString>
#include <QJsonDocument>
#include <QVariant>

#include <optional>

#include "gclouddata.h"

struct Notification
{
    Notification(const QString &title, const QString &body) :
        _title(title),
        _body(body)
    {
        _gCloudata = &GlobalGCloudData::Instance();
    }

    QByteArray BuildJSON()
    {
        QVariantList regIds;
        for(const auto &[id, enabled] : _gCloudata->AvailableDevices)
        {
            if(enabled)
            {
                regIds << id;
            }
        }
        if(_gCloudata->AvailableDevices.size() == 0)
        {
            return QByteArray();
        }

        QVariantMap notifications;
        notifications.insert("title", _title);
        notifications.insert("body", _body);

        QVariantMap jsonMap;
        jsonMap.insert("registration_ids", regIds);
        jsonMap.insert("priority", "high");
        jsonMap.insert("notification", notifications);

        return QJsonDocument::fromVariant(QVariant(jsonMap)).toJson();

    }


    GoogleCloudData *_gCloudata;
    QString _title;
    QString _body;
};

#endif // LANDINGNOTIFICATION_H
