#include "gcinterface.h"

GCInterface::GCInterface(QObject *parent) : QObject(parent)
{
    _dataStore = &GlobalDataStore::Instance();
    _gCloudata = &GlobalGCloudData::Instance();

    _networkManager = new QNetworkAccessManager();

    connect(_networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleNetworkData(QNetworkReply*)));
    connect(_networkManager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(handleSSLErrors(QNetworkReply*)));
    connect(this, &GCInterface::EmitRequest, this, &GCInterface::SendRequest, Qt::QueuedConnection);

    SendPhoneKeysRequest();

    connect(&refreshPhones, &QTimer::timeout, this, &GCInterface::SendPhoneKeysRequest);


    refreshPhones.start(std::chrono::seconds(10));
}

bool GCInterface::Init()
{
    if(!_threadRunning)
    {
        _threadRunning = true;
        _newDataStreamListener = std::thread(&GCInterface::NewDataStreamListenerRun, this);
        return true;
    }
    return false;
}


void GCInterface::handleNetworkData(QNetworkReply *data)
{
    std::cout << "Reply receieved: ";
    const auto &headers = data->rawHeaderList();
    for(auto header : headers)
    {
        std::cout << QString::fromUtf8(header).toStdString() << "\n";
    }
    QByteArray d = data->readAll();
    std::cout << "\nPrinting ReadAll: " << d.toStdString();
    std::cout << "\nAttribute: " << data->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString().toStdString();
    std::cout << "\nNetwork Error: " << data->error() << std::endl;

    auto s = data->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();

    std::cout << "\nReply URL: " << data->request().url().toString().toStdString() << "\n";

    if(data->request().url().toString() == _gCloudata->PhoneRequest)
    {
//        QByteArray d = data->readAll();
        QString dataReply(d);
        QJsonDocument doc = QJsonDocument::fromJson(d);
        QJsonObject obj = doc.object();
        QStringList phones = obj.keys();

        std::cout << "Got databack.\n";
        for(const auto &phone : phones)
        {
            _gCloudata->AvailableDevices[phone] = obj.value(phone).toBool();
            std::cout << "\nPhone: " << phone.toStdString() << "\nSet to: " << std::boolalpha << obj.value(phone).toBool() << std::endl;
        }
    }
}

void GCInterface::handleSSLErrors(QNetworkReply *data)
{
    std::cout << "SSL Errors receieved: ";
    const auto &headers = data->rawHeaderList();
    for(auto header : headers)
    {
        std::cout << QString::fromUtf8(header).toStdString();
    }
}

void GCInterface::SendRequest(QNetworkRequest request, QByteArray data)
{

    std::cout << "\nSendingRequest with data: " << data.toStdString() << "\n";

    QNetworkReply *reply = _networkManager->post(request, data);
}


void GCInterface::SendPhoneKeysRequest()
{
    GCNetworkPostBuilder phoneRequest(0);   /// we dont need a payload

    auto mMessage = phoneRequest.BuildPhoneRequest();
    if(!mMessage)
    {
        return;
    }

    auto [request, data] = mMessage.value_or(std::make_tuple(QNetworkRequest(), QByteArray()));

    QByteArray dummyLoad = "";
    QNetworkReply *reply = _networkManager->get(request);

}


void GCInterface::NewDataStreamListenerRun()
{
    while(_threadRunning)
    {
        auto mTag = _dataStore->newDataStreamsNotifications.PopData();
        if(!mTag)
        {
            return;
        }
        auto tag = mTag.value_or("");
        std::cout << "Starting up for tag: " << tag.toStdString() << std::endl;

        _notificationThreads.emplace_back(&GCInterface::RunNotificationThread, this, tag);
    }
}

void GCInterface::Debug()
{

}

void GCInterface::FireNotificationTest()
{
    GCNetworkPostBuilder<Notification> newNotification({"This is a test.",
                                                        "This is a test body."});

    auto [request, data] = newNotification.BuildMessage();
            emit EmitRequest(request, data);
}



void GCInterface::RunNotificationThread(const QString &tag)
{
    AbstractSensor *sensor = _dataStore->getSensor(tag);

    while(_threadRunning && sensor != nullptr)
    {
        auto chuck = sensor->notificationHandler.PopData();
        if(sensor == nullptr || sensor->Closing())
            return;

        if(!chuck)
        {
            continue;
        }
        auto process = chuck.value_or(NotificationType::DEFAULT);

        auto [title, body] = sensor->GetNotificaition(process);
        if(process == NotificationType::PostSensorEvent)
        {
            std::cout << "POSTING AN EVENT\n";
            GCNetworkPostBuilder<PublishData> postData({QTime::currentTime().toString("hh:mm:ss.zzz"), sensor->id, title});
            auto [request, data] = postData.BuildDataBaseNotificationPost();
            emit EmitRequest(request, data);
        }
        else
        {
            std::cout << "SENDING NOTIFICATION\n";
            GCNetworkPostBuilder<Notification> newNotification({title, body});

            auto [request, data] = newNotification.BuildMessage();
            emit EmitRequest(request, data);
        }
    }
}




