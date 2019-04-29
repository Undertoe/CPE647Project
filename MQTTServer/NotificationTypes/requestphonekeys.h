#ifndef REQUESTPHONEKEYS_H
#define REQUESTPHONEKEYS_H

#include <QString>
#include <QJsonDocument>
#include <QVariant>

#include <optional>

#include "gclouddata.h"

struct RequestPhoneKeys
{
    RequestPhoneKeys()
    {
        _gCloudata = &GlobalGCloudData::Instance();
    }

    std::optional<QByteArray> BuildJSON()
    {

    }

    GoogleCloudData *_gCloudata;

};


#endif // REQUESTPHONEKEYS_H
