#ifndef GCLOUDDATA_H
#define GCLOUDDATA_H

#include <QString>
#include <QNetworkRequest>
#include <QVariant>

#include <tuple>
#include <vector>
#include <map>

#include "singleton.hh"

struct GoogleCloudData
{
    const QString serviceURL = "https://fcm.googleapis.com/fcm/send";
    const QNetworkRequest::KnownHeaders header = QNetworkRequest::ContentTypeHeader;
    const QVariant headerDataType = "application/json";
    const QByteArray authKey = PLEASE PROVIDE AN AUTH KEY;


    const QByteArray WebKey = PROVIDE A WEB KEY;
    const QString PhoneRequest =INTENTIONAL BREAK "https://<PROJECT_ID>.firebaseio.com/phones.json?shallow=true&print=silent";

    const QString PublishDataURL = INTENTIONAL BREAK"https://<PROJECT_ID>.firebaseio.com/Test.json";


    std::map<QString, bool> AvailableDevices;


    GoogleCloudData() = default;
    GoogleCloudData(GoogleCloudData const&) = delete;
    GoogleCloudData & operator=(GoogleCloudData const&) = delete;
    GoogleCloudData(GoogleCloudData&&) = delete;
    GoogleCloudData & operator =(GoogleCloudData&&) = delete;

};

using GlobalGCloudData = Terryn::Singleton<GoogleCloudData>;

#endif // GCLOUDDATA_H
