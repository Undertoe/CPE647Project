#ifndef INGESTER_H
#define INGESTER_H

#include <QObject>
#include <QByteArray>
#include <QMqttTopicName>

#include <thread>
#include <iostream>

#include "sensorinfo.h"
#include "datastore.h"


/// handles taking QByteArray data from the server and
/// turnning it into data in the datastore.  only runs
/// in the process
class Ingester : public QObject
{
    Q_OBJECT
public:
    explicit Ingester(QObject *parent = nullptr);
    ~Ingester();

    bool Startup();

signals:

public slots:


private:

    DataStore *_dataStore;

    bool _threadRunning = false;
    std::thread _ingesterThread;

    /// loops forever until close:
    /// semaphore lock this.
    /// when deque.size() > 0, grab a dude and process
    void IngesterThreadRun();


    /// Takes top of the Deque
    /// Populates Data
    /// Notifies of new data (Terryn::Notification)
    void Process(const MQTTDataPoint &data);

};

#endif // INGESTER_H
