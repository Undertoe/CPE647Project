#include <ESP8266WiFi.h> // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h> // Allows us to connect to, and publish to the MQTT broker
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#include <string>

// WiFi
// Make sure to update this for your own WiFi network!
const char* ssid = "TempWifi";
const char* wifi_password = "password";

// MQTT
// Make sure to update this for your own MQTT Broker!
const char* mqtt_server = "192.168.1.204";

//Activity Thresold value
#define ACTIVITY_THRESOLD 15

//#define SENSOR1
#define SENSOR2

// defines and variable for sensor/control mode
#define MODE_OFF    0
#define MODE_ON     1
#define MODE_SENSE  2
int sensorMode = 0;

#if defined(SENSOR1)
const char* mqtt_topic = "esp8266/imu/data1";
const char* clientID = "Harsha_01";
const char* mqtt_data = "I'm from sensor 1!";
const char* mqtt_ack = "sensor1";
#endif

#if defined(SENSOR2)
const char* mqtt_topic = "esp8266/imu/data2";
const char* clientID = "Harsha_02";
const char* mqtt_data = "I'm from sensor 2!";
const char* mqtt_ack = "sensor2";
#endif

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

char baro[7];
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

void callback(char* topic, byte* payload, unsigned int length);
void init_sensors();
void run_sensors();

WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, callback, wifiClient); // 1883 is the listener port for the Broker

// handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length){
    
    int i = 0;
    // create character buffer with ending null terminator (string)
    for (i = 0; i < length; i++) {
        message_buff[i] = payload[i];
    }
    
    message_buff[i] = '\0';
    String msgString = String(message_buff);
    char * pEnd;
    num = strtol(message_buff, &pEnd, 10);
    
    if (msgString.equals(mqtt_ack)) {
        sensorMode = MODE_ON;
    }

    
    
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(clientID)) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            //client.publish("outTopic","hello world");
            // ... and resubscribe
            client.subscribe("esp8266/imu/start");
            client.subscribe("esp8266/imu/time");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }

    
    Serial.println("Sensor active, attempting welcome message");
    std::string welcomeMessage = "Welcome!";
    client.publish(mqtt_topic, welcomeMessage.data(), welcomeMessage.size());
}


void setup() {
    
    pinMode(ledPin, OUTPUT);
    // Switch the on-board LED off to start with
    digitalWrite(ledPin, HIGH);
    Serial.begin(115200);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    // Connect to the WiFi
    WiFi.begin(ssid, wifi_password);
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
    sensorMode = MODE_ON;
}

void loop() {
    
    if (!client.connected()) {
        reconnect();
    }

    sensorMode = MODE_ON;

    
    switch (sensorMode) {
        case MODE_OFF:
            
            break;
        case MODE_ON:
            if(millis() > previous_time + 9)
            {
                previous_time = millis();
                run_sensors();
            }
            break;
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
    
    /* Display the current temperature */
    int8_t temp = bno.getTemp();
    Serial.print("Current Temperature: ");
    Serial.print(temp);
    Serial.println(" C");
    Serial.println("");
    
    bno.setExtCrystalUse(true);
    
    ///////////////////////////////////////////////////////////////////////////////////////////////
    
    for(int k=0; k<BUF_SIZE; k++){
        buf[k] = 'N';
    }
    
    for(int i=0; i<BUF_SIZE+1; i++){
        //Serial.print(buf[i]);
    }
    Serial.println("Void setup Done!");
}

void run_sensors(){
    imu::Vector<3> acc = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
    // Quaternion data
    imu::Quaternion quat = bno.getQuat();
    
    if(activity == 0){
        float aaax = acc.x();
        float aaay = acc.y();
        float aaaz = acc.z();
        aaa_mag = sqrt(aaax*aaax + aaay*aaay + aaaz*aaaz);
        //Serial.println(aaa_mag);
    }
    
    if(aaa_mag > ACTIVITY_THRESOLD){
        activity = 1;
//        Serial.println("Activity Detected!");
    }
    
    if(activity == 1){
        count_time = count_time + 1;
        //Serial.println("Activity is Detected!....Started sending data for a minute");
    }
    
    if(count_time > 6000){
        count_time = 0;
        activity = 0;
        aaa_mag = 0;
    }
    
    snprintf (msgAX, 7, "%lf", acc.x());
    msgAX[6] = ',';
    snprintf (msgAY, 7, "%lf", acc.y());
    msgAY[6] = ',';
    snprintf (msgAZ, 7, "%lf", acc.z());
    msgAZ[6] = ',';
    
    snprintf (quatW, 7, "%lf", quat.w());
    quatW[6] = ',';
    snprintf (quatX, 7, "%lf", quat.x());
    quatX[6] = ',';
    snprintf (quatY, 7, "%lf", quat.y());
    quatY[6] = ',';
    snprintf (quatZ, 7, "%lf", quat.z());
    quatZ[6] = ',';
    
    snprintf (baro, 7, "%lf", baro_filtered);
    baro[6] = ',';
    
    snprintf (s_num, 8, "%ld", num);
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
    
    for(int ibaro=49; ibaro<56; ibaro++){
        buf[ibaro] = baro[ibaro-49];
    }
    
    for(int inum=56; inum<64; inum++){
        buf[inum] = s_num[inum-56];
    }
    
    num = num + 10;
    
    ////////////////////////////////////////////////////////////
    for(int ii=0; ii<BUF_SIZE; ii++)
    {
        big_buf[slider+ii] = buf[ii];
    }
    
    slider = slider + BUF_SIZE;
    if((slider > BIG_BUF_SIZE))
    {
        slider = 0;
        snprintf (big_buf, BIG_BUF_SIZE, "%ls", big_buf);
        if(activity == 1){
            client.publish(mqtt_topic, big_buf, BIG_BUF_SIZE);
            client.publish(mqtt_topic, "@");
        }
    }
}
