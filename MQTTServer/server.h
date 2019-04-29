#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QTcpSocket>
#include <QtMqtt/QMqttClient>

#include <thread>
#include <iostream>

#include "datastore.h"

#include "container_helpers.hh"


class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);

    bool Startup();

signals:

public slots:

    /// connects to broker (127.0.0.1 / 1883)
    void StartClient();

    void StateChanged();

    /// we should try to re-connect
    void BrokerDisconnected();

    void DataRecieved(const QByteArray &message, const QMqttTopicName &topic);

    void HandlePing();


private:
    QMqttClient *_client;
    DataStore *_dataStore;
    QTimer _pingTimer;

    std::chrono::milliseconds pingTime = std::chrono::seconds(1);

    bool _firstMessage = true;

    void Connect();

    QString findNextString(SensorType type);
};

#endif // SERVER_H
