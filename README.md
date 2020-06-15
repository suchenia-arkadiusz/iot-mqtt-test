# IoT MQTT Test

This project was created for the MQTT connection test between a server that was written in the Kotlin Language
and a device that is a NodeMCU board with the Arduino library support.

C++ code for the Device you can find [here](https://github.com/suchenia-arkadiusz/iot-mqtt-test/blob/master/mqtt-test-device/src/main.cpp)

Kotlin code for the server you can find [here](https://github.com/suchenia-arkadiusz/iot-mqtt-test/tree/master/mqtt-test-server/src/main/kotlin/pl/arusoftware/mqtttest)

## Code explanation

### Device code

```cpp
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include <Ticker.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define SERIAL_NUMBER "NMCU_1589714856299"

Adafruit_BME280 bme;
WiFiClient net;
MQTTClient client;
Ticker ticker;

String publishTopic = "adevice/" + String(SERIAL_NUMBER) + "/data";

float temperature, humidity, pressure, altitude;
String json;
```
In the beginning, we can find all imports that are used in this example. After that, we define a default pressure on the
sea level and the device's serial number, prepare modules for work and define some variables that we'll use later.
```cpp
void readData() {
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
}

void generateJson() {
  StaticJsonDocument<200> object;
  object["temperature"] = temperature;
  object["humidity"] = humidity;
  object["pressure"] = pressure;
  object["altitude"] = altitude;

  json = "";
  serializeJson(object, json);
}

void readDataAndSendMessage() {
  readData();
  generateJson();
  Serial.println("Publish message: " + json);
  client.publish(publishTopic.c_str(), json.c_str());
}

void receiveMessage(String &topic, String &message) {
  Serial.println("Incoming:" + topic + " - " + message);
}
```
A `readData()` function reads data from the sensor and pass the values on the variables.

A `generateJson()` function generates JSON string from the read values

A `readDataAndSendMessage()` function read data from the sensors, generates JSON string and put it to the MQTT queue
with a topic that we defined earlier

A `receiveMessage()` function reads the message from the topic and print it on the debug console
```cpp
void connectWithMQTT() {
  Serial.print("Connect with MQTT broker");
  while(!client.connect(SERIAL_NUMBER)) {
    Serial.print(".");
    delay(500);
  }
  client.subscribe("adevice/NMCU_1589714856299/settings", 0);
}

void configureWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.hostname(SERIAL_NUMBER);
  WiFi.begin("dom");

  Serial.print("Connecting with wifi");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");

  Serial.print(SERIAL_NUMBER);
  Serial.println("'s IP Address = " + WiFi.localIP().toString());
}

void setup() {
  Serial.begin(115200);
  delay(100);

  bme.begin(0x76);
  delay(100);

  Serial.println();

  configureWiFi();
  delay(100);

  client.begin("192.168.8.185", 1883, net);
  client.onMessage(reveiveMessage);
  connectWithMQTT();
  
  ticker.attach_ms(10000, readDataAndsendMessage);
}
```
This code setup all device to work with the MQTT broker, subscribe to the topic and connect with the WiFi
```cpp
void loop() {
  if(!client.connected()) {
    connectWithMQTT();
    Serial.println("Reconnected");
  }

  client.loop();
}
```
In the `loop()` function device checks if it's connected with the MQTT - if not it tries to connect and call the MQTT
client's `loop()` function.

### Server code

_Connection.kt_
```kotlin
class Connection private constructor(val publisherId: String, val uri: String, private val mqttClient: IMqttClient){
    companion object {
        fun create(uri: String, port: Int = 1883, publisherId: String = UUID.randomUUID().toString()): Connection {

            val mqttClient = MqttClient("tcp://$uri:$port", publisherId)

            val connectOptions = MqttConnectOptions()
            connectOptions.isAutomaticReconnect = true
            connectOptions.isCleanSession = true
            connectOptions.connectionTimeout = 10
            mqttClient.connect(connectOptions)

            return Connection(publisherId, uri, mqttClient)
        }
    }

    private val usedTopics = mutableListOf<String>()

    fun subscribeTopic(topic: String, listener: (topic: String, message: MqttMessage) -> Unit) {
        usedTopics.add(topic)
        mqttClient.subscribe(topic, listener)
    }

    fun publishMessage(topic: String, message: String) {
        val mqttMessage = MqttMessage(message.toByteArray())
        println("Publish message: $message")
        mqttClient.publish(topic, mqttMessage)
    }

    fun unsubscribe(topic: String) {
        mqttClient.unsubscribe(topic)
    }

    fun unsubscribeAll() {
        mqttClient.unsubscribe(usedTopics.toTypedArray())
    }
}
```
This class creates connection with the MQTT broker (in this case with [Mosquitto](https://mosquitto.org/)). To create
this objecte we have to use the static `create()` function and we have to provide only the broker's address - We can
change the port and the publisherId if we want to, if not we can use the default _1883_ port and the randomly generated
_UUID_ as a publisherId.

A `subscribeTopic()` function add topic to the usedTopics list and subscribe this topic with the MQTT broker. The
second argument is a lambda function that we have to provide to work with the topic's response.

A `publishMessage()` function creates a MqttMessage object and put it to the queue.

A `unsubscribe()` and `unsubscribeAll()` functions unsubscribe provided topic or all topics from the broker.

_MQTTTest.kt_
```kotlin
fun main() {
    val mqttConnection = Connection.create("localhost")
    mqttConnection.subscribeTopic("adevice/+/data") { topic, message ->
        println("$topic = ${String(message.payload)}")
    }
    while (true) {
        mqttConnection.publishMessage("adevice/NMCU_1589714856299/settings", "test message")
        Thread.sleep(1000)
    }
}
```
A `main()` function creates MQTT connection using the _Connection.kt_ class, subscribes to the topic - print the
result on the console, and publish message to the queue every 1 second.