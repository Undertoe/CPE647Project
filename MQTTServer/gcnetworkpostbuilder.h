#ifndef GCNETWORKPOSTBUILDER_H
#define GCNETWORKPOSTBUILDER_H

#include <QNetworkRequest>
#include <QString>
#include <QDebug>
#include <QJsonDocument>

#include <vector>
#include <tuple>
#include <optional>

#include "gclouddata.h"


template<typename PayloadType>
class GCNetworkPostBuilder
{
public:

    using NetworkPost = std::tuple<QNetworkRequest, QByteArray>;
    using MNetworkPost = std::optional<NetworkPost>;

    GCNetworkPostBuilder(const PayloadType &payload) :
        _payload(payload)
    {
        _gCloudata = &GlobalGCloudData::Instance();
    }

    /// should allow in line initialization
    GCNetworkPostBuilder(PayloadType &&payload) :
        _payload(std::move(payload))
    {
        _gCloudata = &GlobalGCloudData::Instance();
    }

    NetworkPost BuildMessage()
    {
        QString baseString = "";
//        auto mJsonString = _payload.BuildJSON();
//        if(!mJsonString)
//        {
//            return std::nullopt;
//        }
        QByteArray jsonString = _payload.BuildJSON();

        QByteArray postDataSize =  QByteArray::number(jsonString.size());

        QNetworkRequest request(_gCloudata->serviceURL);

        request.setHeader(_gCloudata->header, _gCloudata->headerDataType);
        request.setRawHeader("Content-Length", postDataSize);
        request.setRawHeader("Authorization", _gCloudata->authKey);

        return {request, jsonString};
    }

    MNetworkPost BuildPhoneRequest()
    {
        QNetworkRequest request(_gCloudata->PhoneRequest);

        request.setHeader(_gCloudata->header, _gCloudata->headerDataType);
        request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");


        return std::make_tuple(request, QByteArray());
    }

    NetworkPost BuildDataBaseNotificationPost()
    {
//        auto mJsonString = _payload.BuildJSON();
//        if(!mJsonString)
//        {
//            return std::nullopt;
//        }
//        QByteArray jsonString = mJsonString.value_or(QByteArray());
        QByteArray jsonString = _payload.BuildJSON();

        QByteArray postDataSize = QByteArray::number(jsonString.size());

        QUrl url(intentional breaking"https://<PROJECTID>.firebaseio.com/Notifications.json");

        QNetworkRequest request(url);

        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Content-Length", postDataSize);
//        request.setRawHeader("Content-Type", "application/json");
//        request.setRawHeader("Authorization", _gCloudata->WebKey);

        return {request, jsonString};
    }



private:
    PayloadType _payload;

    GoogleCloudData *_gCloudata;
};

#endif // GCNETWORKPOSTBUILDER_H
