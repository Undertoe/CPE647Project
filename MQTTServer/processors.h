#ifndef PROCESSORS_H
#define PROCESSORS_H

#include <QObject>
#include <QDateTime>


#include <thread>
#include <vector>
#include <iostream>
#include <fstream>

#include "datastore.h"
#include "sensorprocessing.h"


/// Runs and manages processing threads on the new sensor data.
/// this needs to be optimized to run fast, but that comes later.
///
/// If an event is found, this is passed to the GCInterface
///
/// All processor functions are declared in sensorprocessing
class Processors : public QObject
{
    Q_OBJECT
public:
    explicit Processors(QObject *parent = nullptr);

    bool Init();

signals:

public slots:

protected:
    void NewDataStreamListenerRun();

    void RTProcessingThreadRun(const QString &tag);
    void EndOfThreadDSListenerRun(const QString &tag);

private:
    DataStore *_dataStore;
    SensorProcessing *_sensorProcessors;
    bool _threadRunning = false;

    std::thread _newDataStreamListener;

    std::vector<std::thread> _endOfThreadDSListener;
    std::vector<std::thread> _rTProcessingThreads;

};

#endif // PROCESSORS_H
