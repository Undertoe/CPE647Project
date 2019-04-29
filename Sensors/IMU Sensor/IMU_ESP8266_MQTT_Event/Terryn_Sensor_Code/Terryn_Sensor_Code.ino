#include <ESP8266WiFi.h> // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h> // Allows us to connect to, and publish to the MQTT broker
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#include <string>

#define DEBUG
#define IMU1
//#define IMU2


class Topic
{
private:
  bool _initialized = false;
  std::string _topicString = "";
  std::string _serverPingRequest = "";
  std::string _serverPongResponse = "";
  std::string _sensorPingRequest = "";
  std::string _sensorPongResponse = "";
  
  std::string ServerPingTopic = "server/ping/";
  std::string ServerPongTopic = "server/pong/";
  std::string SensorPingTopic = "sensor/ping/";
  std::string SensorPongTopic = "sensor/pong/";
  std::string BaseTopic = "sensors/";

public:
  bool Initialized() const { return _initialized; }
  void Initialize(const std::string &topic) 
  {
    _initialized = true;
    _topicString = BaseTopic + topic;
    _serverPingRequest = ServerPingTopic + topic;
    _serverPongResponse = ServerPongTopic + topic;
    _sensorPingRequest = SensorPingTopic + topic;
    _sensorPongResponse = SensorPongTopic + topic;
  }

  std::string GetTopic()
  {
    if(!_initialized)
    {
      return "";
    }
    return _topicString;
  }

  std::string GetServerPing()
  {
    if(!_initialized)
    {
      return "";
    }
    return _serverPingRequest;
  }
  std::string GetServerPong()
  {
    if(!_initialized)
    {
      return "";
    }
    return _serverPongResponse;
  }
  std::string GetSensorPing()
  {
    if(!_initialized)
    {
      return "";
    }
    return _sensorPingRequest;
  }
  std::string GetSensorPong()
  {
    if(!_initialized)
    {
      return "";
    }
    return _sensorPongResponse;
  }

  void Disconnect() { _initialized = false; }
};

//#ifdef IMU1
//  std::string MyID = "IMU_1";
//#elif IMU2
//  std::string MyID = "IMU_2";
//#endif
std::string MyID = "IMU";
//std::string IMUSensorTitle = "IMU";
//std::string TopicRequestPayload = IMUSensorTitle + "|" + MyID;

const char* SensorRequestTopic = "server/TopicRequest";
const char* SensorRequestResponseTopic = "server/TopicResponse";

// WiFi
// Make sure to update this for your own WiFi network!
const std::string ssid = "PiFi";
const std::string wifi_password = "password";

// MQTT
// Make sure to update this for your own MQTT Broker!
const std::string mqtt_server = "192.168.1.204";
//const std::string mqtt_server = "192.168.1.150";

std::string clientID = "";

//Activity Thresold value
#define ACTIVITY_THRESOLD 15

char message_buff[100];
float baro_filtered;
Adafruit_BNO055 bno = Adafruit_BNO055();

#define BUF_SIZE 64
#define BIG_BUF_SIZE 6400
#define TEST_BUF_SIZE 100
long int slider = 0;
char buf[BUF_SIZE];
char big_buf[BIG_BUF_SIZE];
char test_buf[TEST_BUF_SIZE];

char msgAX[7];
char msgAY[7];
char msgAZ[7];
char msgGX[50];
char msgGY[50];
char msgGZ[50];
char msgMX[50];
char msgMY[50];
char msgMZ[50];
char quatW[7];
char quatX[7];
char quatY[7];
char quatZ[7];
char countN[7];
char s_num[8];
long int num = 0;
char sample_num[50];
int value = 0;
int send_data = 0;
int previous_time=0;
float aaa_mag=0;
int activity = 0;
int count_time = 0;
const int ledPin = 5;
unsigned int endTime = 0;
unsigned int nextPing = 0;
unsigned int timeoutPing = 0;
unsigned int sampleCount = 0;
bool FirstMessage = true;


Topic myTopic;


int TopicCompare(char* incomingTopic);

#define UNKNOWN_TOPIC -1
#define Sensor_Request_Response_Topic 1
#define Server_Ping_Topic 2
#define Sensor_Pong_Topic 3

int TopicCompare(char* incomingTopic)
{
  if(strcmp(incomingTopic, SensorRequestResponseTopic) == 0)
//  if(msgString.compare(SensorRequestTopic) == 0)
    return Sensor_Request_Response_Topic;

    /// server Ping Request
  else if(strcmp(incomingTopic, myTopic.GetServerPing().c_str()) == 0)
//  if(msgString.compare(myTopic.GetServerPing()) == 0)
    return Server_Ping_Topic;

    /// server Pong Response
  else if(strcmp(incomingTopic, myTopic.GetSensorPong().c_str()) == 0)
//  if(msgString.compare(myTopic.GetSensorPong()) == 0)
  {
    return Sensor_Pong_Topic;
  }
  else
  {
    return UNKNOWN_TOPIC;
  }
}

void callback(char* topic, byte* payload, unsigned int length);
void init_sensors();
void run_sensors();
void Ping();
void InitializeClient();

WiFiClient wifiClient;
PubSubClient client(mqtt_server.c_str(), 1883, callback, wifiClient); // 1883 is the listener port for the Broker

// handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length){

    if(FirstMessage)
    {
      FirstMessage = false;
      return;
    }
    int i = 0;
    // create character buffer with ending null terminator (string)
    for (i = 0; i < length; i++) {
        message_buff[i] = payload[i];
    }
    
    message_buff[i] = '\0';
    String msgString = String(message_buff);
    char * pEnd;
    num = strtol(message_buff, &pEnd, 10);

#ifdef DEBUG
    Serial.println("Message: " + msgString);
    Serial.print("Topic: ");
    Serial.print(topic);
    Serial.print("\n");
#endif

    int topicFound = TopicCompare(topic);
    if(topicFound == UNKNOWN_TOPIC)
    {
      Serial.println("Unknown topic found!");
      return;
    }
    if(topicFound == Sensor_Request_Response_Topic)
    {
#ifdef DEBUG
        Serial.println("topic found == sensor request response topic");
#endif
        if(myTopic.Initialized())
        {
#ifdef DEBUG
          Serial.println("Already initialized.  Discarding");
#endif
          return;
        }
        std::string top(message_buff);
        myTopic.Initialize(top);
        client.subscribe(myTopic.GetServerPing().c_str());
        client.subscribe(myTopic.GetSensorPong().c_str());
        timeoutPing = millis() + 20000;
        
#ifdef DEBUG
        std::string debug = "initalized to: " + myTopic.GetTopic();
        Serial.print(debug.c_str());
#endif
    }
    else if(topicFound == Server_Ping_Topic)
    {
      if(myTopic.Initialized())
      {
        nextPing = millis() + 10000;
        timeoutPing = nextPing + 10000;
        client.publish(myTopic.GetServerPong().c_str(), MyID.c_str());
        
#ifdef DEBUG
        std::string debug = "Publishing data to: " + myTopic.GetServerPong() + " Data: " + MyID;
        Serial.print(debug.c_str());
#endif
      }
    }
    else if(topicFound == Sensor_Pong_Topic)
    {
      if(myTopic.Initialized())
      {
        nextPing = millis() + 10000;  /// 10 seconds from now
        timeoutPing = nextPing + 10000; /// 20 seconds from now
#ifdef DEBUG
        std::string debug = "ping reset to: " +  nextPing;
        Serial.print(debug.c_str());
#endif
      }
    }
}

void Ping()
{
  unsigned int mil = millis();
  if(mil > timeoutPing)
  {
    myTopic.Disconnect();
    return;
  }
  if(mil > nextPing)
  {
//    Serial.println("Trying to ping");
//    Serial.println(myTopic.GetSensorPing().c_str());
    nextPing = mil + 5000; /// wait a second to do this again
    client.publish(myTopic.GetSensorPing().c_str(), MyID.c_str());
  }
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(clientID.c_str())) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            //client.publish("outTopic","hello world");
            // ... and resubscribe
            client.subscribe(SensorRequestResponseTopic);
//            client.publish(SensorRequestTopic, MyID.data(), MyID.size());
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
    
}

void InitializeClient()
{
   Serial.println("Topic needs to be initialized");
//    std::string welcomeMessage = MyID;
//    client.publish(myTopic.GetTopic().c_str(), welcomeMessage.data(), welcomeMessage.size());
//    Serial.println(MyID.c_str());
   client.publish(SensorRequestTopic, MyID.c_str());
}

void setup() {
    
    pinMode(ledPin, OUTPUT);
    // Switch the on-board LED off to start with
    digitalWrite(ledPin, HIGH);
    Serial.begin(115200);
    Serial.print("Connecting to ");
    Serial.println(ssid.c_str());
    // Connect to the WiFi
    WiFi.begin(ssid.c_str(), wifi_password.c_str());
    // Wait until the connection has been confirmed before continuing
    while (WiFi.status() != WL_CONNECTED) {
        // light should be off
        digitalWrite(ledPin, LOW);
        delay(500);
        Serial.print(".");
    }
    
    // Debugging - Output the IP Address of the ESP8266
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    // light should be on
    digitalWrite(ledPin, HIGH);
    
    init_sensors();
}

void loop() {
    
    if (!client.connected()) {
        reconnect();
    }

//    myTopic.Initialize("IMU");

    
    if(myTopic.Initialized())
    {
      if(millis() > previous_time + 32)
      {
        previous_time = millis();
        run_sensors();
      }
      Ping();
    }
    else
    {
      InitializeClient();
      delay(1000);
      
    }
    
    client.loop();
}

void init_sensors(){
    // Initialise I2C communication as MASTER
    Wire.begin();
    
    /* Initialise the sensor */
    if(!bno.begin())
    {
        /* There was a problem detecting the BNO055 ... check your connections */
        Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
        while(1);
    }
    
    delay(1000);
    
    
    bno.setExtCrystalUse(true);
    
    ///////////////////////////////////////////////////////////////////////////////////////////////
    
    for(int k=0; k<BUF_SIZE; k++){
        buf[k] = 'N';
    }
    
    Serial.println("Void setup Done!");
}

void run_sensors(){
#ifdef DEBUG
  unsigned int start = millis();
#endif

  imu::Vector<3> acc = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  float ax = acc.x();
  float ay = acc.y();
  float az = acc.z();
  sampleCount++;

  if(activity == 0)
  {
    float mag = sqrt(ax*ax + ay*ay + az*az);
    if(mag > ACTIVITY_THRESOLD)
    {
      activity = 1;
      endTime = millis() + 15000;
      sampleCount = 0;
    }
    
  }

/// handle output here
  if(activity == 1)
  {
    if(count_time == 0)
    {
      client.publish(myTopic.GetTopic().c_str(), "*");
      nextPing = start + 2000;
      timeoutPing = nextPing + 20000;
#ifdef DEBUG
      Serial.print("*");
#endif
    }
    count_time = count_time + 1;
    imu::Quaternion quat = bno.getQuat();

    
    snprintf (msgAX, 7, "%lf", ax);
    msgAX[6] = ',';
    snprintf (msgAY, 7, "%lf", ay);
    msgAY[6] = ',';
    snprintf (msgAZ, 7, "%lf", az);
    msgAZ[6] = ',';
    
    snprintf (quatW, 7, "%lf", quat.w());
    quatW[6] = ',';
    snprintf (quatX, 7, "%lf", quat.x());
    quatX[6] = ',';
    snprintf (quatY, 7, "%lf", quat.y());
    quatY[6] = ',';
    snprintf (quatZ, 7, "%lf", quat.z());
    quatZ[6] = ',';
    
//    snprintf (baro, 7, "%lf", baro_filtered);
//    baro[6] = ',';
//    
//    snprintf (s_num, 7, "%u", start);
//    s_num[7] = ',';
    snprintf (countN, 7, "%lu", sampleCount);
    countN[6] = ',';

    
    snprintf (s_num, 8, "%ld", start);
    s_num[7] = '\n';
    
    for(int ix=0; ix<7; ix++){
        buf[ix] = msgAX[ix];
    }
    
    for(int iy=7; iy<14; iy++){
        buf[iy] = msgAY[iy-7];
    }
    
    for(int iz=14; iz<21; iz++){
        buf[iz] = msgAZ[iz-14];
    }
    
    for(int iqw=21; iqw<28; iqw++){
        buf[iqw] = quatW[iqw-21];
    }
    
    for(int iqx=28; iqx<35; iqx++){
        buf[iqx] = quatX[iqx-28];
    }
    
    for(int iqy=35; iqy<42; iqy++){
        buf[iqy] = quatY[iqy-35];
    }
    
    for(int iqz=42; iqz<49; iqz++){
        buf[iqz] = quatZ[iqz-42];
    }
    
//    for(int inum=49; inum<56; inum++){
//        buf[inum] = s_num[inum-49];
//    }

    for(int ibaro=49; ibaro<56; ibaro++){
        buf[ibaro] = countN[ibaro-49];
    }
    
    for(int inum=56; inum<64; inum++){
        buf[inum] = s_num[inum-56];
    }
    
    num = num + 10;

    String output = "";
    output += ax;
    output += ",";
    output += ay;
    output += ",";
    output += az;
    output += ",";
    output += quat.w();
    output += ",";
    output += quat.x();
    output += ",";
    output += quat.y();
    output += ",";
    output += quat.z();
    output += ",";
    output += sampleCount;
    output += ",";
    output += start;

  
//     if(activity == 1){
    nextPing = start + 2000;
    timeoutPing = nextPing + 20000;
//    client.publish(myTopic.GetTopic().c_str(), buf, BUF_SIZE);
    client.publish(myTopic.GetTopic().c_str(), output.c_str(), output.length());
//            client.publish(myTopic.GetTopic().c_str(), "@");
    std::string debug = myTopic.GetTopic() + ": " + buf + "\n";
    
#ifdef DEBUG
//    Serial.println(debug.c_str());
    Serial.println(output);
//    Serial.println("******************");
#endif
    if(millis() > endTime)
    {
      nextPing = start + 2000;
      timeoutPing = nextPing + 20000;
      activity = 0;
      count_time = 0;
      client.publish(myTopic.GetTopic().c_str(), "@");
#ifdef DEBUG
      Serial.print("@");
#endif
    }
  }

#ifdef DEBUG
  unsigned int end = millis();
  unsigned int total = end - start;
  char b[12];
  sprintf(b, "%u\n", total);
//  Serial.print(b);
#endif
}
