#ifndef IMUSENSOR_H
#define IMUSENSOR_H

#include <QString>

#include "sensorinfo.h"


struct XYZ
{
    float AccelX = 0.0;
    float AccelY = 0.0;
    float AccelZ = 0.0;
    float QuatW = 0.0;
    float QuatX = 0.0;
    float QuatY = 0.0;
    float QuatZ = 0.0;
    uint32_t frameCount = 0;
    uint32_t timestamp = 0;

    XYZ() = default;

    XYZ(const QStringList &list)
    {
        AccelX = list[0].toFloat();
        AccelY = list[1].toFloat();
        AccelZ = list[2].toFloat();
        QuatW = list[3].toFloat();
        QuatX = list[4].toFloat();
        QuatY = list[5].toFloat();
        QuatZ = list[6].toFloat();
        frameCount = list[7].toUInt();
        timestamp = list[8].toUInt();
    }

    void Header(std::ostream &os)
    {
        os << std::setw(8) << "AccelX" << std::setw(8) << "AccelY"
           << std::setw(8) << "AccelZ" << std::setw(8) << "QuatW"
           << std::setw(8) << "QuatX" << std::setw(8) << "QuatY"
           << std::setw(8) << "QuatZ" << std::setw(10) << "frameCount"
           << std::setw(10) << "timestamp"
           << std::endl;
    }

    void Add(const XYZ &xyz)
    {
        AccelX += (xyz.AccelX);
        AccelY += (xyz.AccelY);
        AccelZ += (xyz.AccelZ);
        QuatW += (xyz.QuatW);
        QuatX += (xyz.QuatX);
        QuatY += (xyz.QuatY);
        QuatZ += (xyz.QuatZ);
        frameCount ++;
        timestamp += xyz.timestamp;
    }

    void SQRT()
    {
        AccelX = std::sqrt(AccelX);
        AccelY = std::sqrt(AccelY);
        AccelZ = std::sqrt(AccelZ);
        QuatW = std::sqrt(QuatW);
        QuatX = std::sqrt(QuatX);
        QuatY = std::sqrt(QuatY);
        QuatZ = std::sqrt(QuatZ);
    }

    void DivideBy(uint32_t val)
    {
        AccelX /= val;
        AccelY /= val;
        AccelZ /= val;
        QuatW /= val;
        QuatX /= val;
        QuatY /= val;
        QuatZ /= val;
        timestamp /= val;
    }

    float mag() const
    {
        return sqrt((AccelX * AccelX) + (AccelY * AccelY)  + (AccelZ * AccelZ) );
    }

    bool DetectFall()
    {
        return (mag()) > 100;
    }

    void OutputForCSV(std::ostream &os)
    {
        os << AccelX << "," << AccelY << "," << AccelZ << ","
               << QuatW << "," << QuatX << "," << QuatY << ","
                   << QuatZ << "," << frameCount << "," << timestamp;
    }
};

static inline std::ostream& operator<<(std::ostream& os, const XYZ& dt)
{
    os << std::setprecision(3) << std::setw(8)
       << dt.AccelX << std::setw(8) << dt.AccelY << std::setw(8)
       << std::setw(8) << dt.AccelZ << std::setw(8) << dt.QuatW
       << std::setw(8) << dt.QuatX << std::setw(8)
       << std::setw(8) << dt.QuatY << std::setw(8) << dt.QuatZ
       << std::setw(10) << dt.frameCount
       << std::setw(8) << dt.timestamp;
    return os;
}




class IMUSensor : public AbstractSensor
{
public:
    IMUSensor(const QString &source, const QString &topic);
    ~IMUSensor() override;

    void IngestData(const QByteArray &input) override;
    void ProcessData() override;
    void Wait() override;
    bool Closing() override;
    std::tuple<QString, QString> GetNotificaition(NotificationType type) override;

    SensorData<XYZ> *data;
    std::deque<uint32_t> fallNumbers;
    std::deque<std::pair<XYZ, ProcessedEvent>> notificationData;
};

#endif // IMUSENSOR_H
