#include "server.h"


Server::Server(QObject *parent) :
    QObject(parent)
{
    _dataStore = &GlobalDataStore::Instance();
    _client = new QMqttClient(this);
//    _client->setHostname("127.0.0.1");
//    _client->setHostname("192.168.1.204");
    _client->setHostname("192.168.1.150");
    _client->setPort(1883);

    connect(_client, &QMqttClient::stateChanged, this, &Server::StateChanged);
    connect(_client, &QMqttClient::disconnected, this, &Server::BrokerDisconnected);
    connect(_client, &QMqttClient::messageReceived, this, &Server::DataRecieved);
    connect(_client, &QMqttClient::pingResponseReceived, this, []()
    {
//        std::cout << "Ping Recieved!" << std::endl;

    });

    connect(&_pingTimer, &QTimer::timeout, this, &Server::HandlePing);
}


bool Server::Startup()
{
    using namespace std::chrono_literals;
    using namespace std::chrono;


    Connect();


    _pingTimer.start(pingTime);

    QTimer single;
    single.singleShot(3000, [this](){
        if(!_client->subscribe(_dataStore->mqttStrings.SensorRequestTopic))
        {
            std::cout << "Sensor request sub failed" << std::endl;
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));
//    StartClient();

    return true;
}

void Server::StartClient()
{
}

void Server::StateChanged()
{
    std::cout << "[" << QDateTime::currentDateTime().toString().toStdString()
              << "] State Changed: " << std::to_string(_client->state()) << std::endl;
}

void Server::BrokerDisconnected()
{
    std::cout << "Disconnected from broker" << std::endl;
}

void Server::DataRecieved(const QByteArray &message, const QMqttTopicName &topic)
{
    using namespace std::chrono_literals;

    std::cout << "******************************************" << std::endl;
    std::cout << "Data Receieved, checking" << std::endl;
    std::cout << "Message: " << message.toStdString() << "\nTopic: " << topic.name().toStdString() << std::endl;



    if(_firstMessage)
    {
        _firstMessage = false;
        std::cout << "First Message, discarding" << std::endl;
        return;
    }

    QString tag = topic.name().split('/').back();

    /// handle Pong response
    if(topic.name() == _dataStore->ServerPongResponseStrings[tag])
    {
        for(auto &[pongID, time] : _dataStore->lastPongs)
        {
            if(pongID == tag)
            {
                time = 0s;
                std::cout << "PONG RESPONSE from : " << tag.toStdString() << "found. Resetting" << std::endl;
                std::cout << "******************************************\n";
            }
        }
    }
    /// handle sensor ping request
    else if(topic.name() == _dataStore->SensorPingRequestStrings[tag])
    {
        for(auto &[pingID, time] : _dataStore->lastPongs)
        {
            if(pingID == tag)
            {
                _client->publish(_dataStore->SensorPongResponseStrings[tag], "*");
                std::cout << "Pong sent to sensor: " << tag.toStdString() << " Resetting." << std::endl;
                std::cout << "******************************************\n";
                time = 0s;
            }
        }

//        if(Terryn::Contains(_dataStore->tags, tag))
//        {
//            _client->publish(_dataStore->SensorPongResponseStrings[tag], "*");
//            std::cout << "Pong sent to sensor: " << tag.toStdString() << std::endl;
//            _dataStore->lastPongs[tag] = 0s;
//        }
    }
    /// handle sensor requesting a new ID
    else if (topic.name() == _dataStore->mqttStrings.SensorRequestTopic)
    {
        auto sensorType = QString::fromLatin1(message);
        auto sensor = SensorTypeFromString(sensorType);
        QString response;
        QString newTag = findNextString(sensor);
        QString newTopic = _dataStore->mqttStrings.SensorBase +  newTag;
        _client->subscribe(newTopic);
        _dataStore->Subscribe(newTag, sensor);
        std::cout << "Topic Recieved" << newTag.toStdString() << std::endl;
        std::cout << "Subscribed on " << newTopic.toStdString() << std::endl;
        std::cout << "Publishing to " << _dataStore->mqttStrings.SensorRequestTopicResponse.toStdString() << std::endl;
        _client->publish(_dataStore->mqttStrings.SensorRequestTopicResponse, newTag.toLatin1());

        _client->subscribe(_dataStore->SensorPingRequestStrings[newTag]);
        _client->subscribe(_dataStore->ServerPongResponseStrings[newTag]);
    }
    else
    {
        std::cout << "Data store requested" << std::endl;
        if(_dataStore->tagTopicMap[tag] == topic.name())
        {
            std::cout << "TOPIC FOUND" << std::endl;
            _dataStore->queuedData.AddData({tag, message, SensorTypeFromString(tag)});
            std::cout << "Data found for Sensor: " << tag.toStdString() << ": " << message.toStdString() << std::endl;


            _dataStore->lastPongs[tag] = 0s;
        }
    }
}

void Server::HandlePing()
{
    using namespace std::chrono_literals;
    using namespace std::chrono;


    if(!_client->requestPing())
    {
        std::cout << "PING REQUEST FAILED" << std::endl;
    }

//    std::cout << "PING THREAD: " << std::endl;
    std::vector<QString> tagsToErase;
    for(auto &[tag, lastPing] : _dataStore->lastPongs)
    {
        if(lastPing > (pingTime * 7))
        {
            std::cout << "Sensor has timed out.  Disconnecting" << std::endl;
            std::cout << "Disconnecting: " << tag.toStdString() << std::endl;
            if(Terryn::Contains(_dataStore->tags, tag))
            {
                _client->unsubscribe(_dataStore->tagTopicMap[tag]);
                _dataStore->Unsubscribe(tag, SensorTypeFromString(tag));
                tagsToErase.push_back(tag);
            }
            std::cout << "Sensor Disconnected" << std::endl;
        }
        else if(lastPing > (pingTime * 3))
        {
            std::cout << "Pinging sensor: " << tag.toStdString() << std::endl;
            _client->publish(_dataStore->ServerPingRequestStrings[tag], "*");
        }
        lastPing += duration_cast<seconds>(pingTime);
        std::cout << "\nLast ping for tag: " << tag.toStdString() << ": "
                  << duration_cast<seconds>(lastPing).count()
                  << std::endl;
    }

    for(auto &t : tagsToErase)
    {
        _dataStore->lastPongs.erase(t);
        std::cout << t.toStdString() << " has been removed" << std::endl;
    }
//    std::cout << "Exiting Ping" << std::endl;
}


void Server::Connect()
{
    _client->connectToHost();
}

QString Server::findNextString(SensorType type)
{
    auto &strings = _dataStore->sensorTags[type];
    QString typeBase = SensorTypeString(type) + "_";
    bool nextSlot = false;
    int i = 0;
    while(1)
    {
        QString check = typeBase + QString::number(i++);
        if(!Terryn::Contains(strings, check))
        {
            return check;
        }
    }
}


