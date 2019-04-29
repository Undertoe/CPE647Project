#ifndef GCINTERFACE_H
#define GCINTERFACE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QDateTime>

#include <thread>
#include <iostream>


#include "datastore.h"
#include "gclouddata.h"
#include "gcnetworkpostbuilder.h"
#include "publishdata.h"
#include "landingnotification.h"


class GCInterface : public QObject
{
    Q_OBJECT
public:
    explicit GCInterface(QObject *parent = nullptr);

    bool Init();

signals:

    void EmitRequest(QNetworkRequest, QByteArray);

public slots:
    void handleNetworkData(QNetworkReply *data);
    void handleSSLErrors(QNetworkReply *data);

    void SendRequest(QNetworkRequest request, QByteArray data);

    void SendPhoneKeysRequest();
    void FireNotificationTest();
    void Debug();


protected:
    void NewDataStreamListenerRun();
    void RunNotificationThread(const QString &tag);


private:

    DataStore *_dataStore;
    GoogleCloudData *_gCloudata;

    QNetworkAccessManager *_networkManager;
    bool _threadRunning = false;

    QTimer refreshPhones;
    QTimer SingleNotificationTimer;
//    QTimer singleStartupPhoneInit;

    std::thread _newDataStreamListener;
    std::vector<std::thread> _notificationThreads;

};

#endif // GCINTERFACE_H
