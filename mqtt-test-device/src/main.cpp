#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <ArduinoJson.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define SERIAL_NUMBER "NMCU_1589714856299"

Adafruit_BME280 bme;
WiFiClient net;
MQTTClient client;

String publishTopic = "adevice/" + String(SERIAL_NUMBER) + "/data";

float temperature, humidity, pressure, altitude;
String json;

void readData() {
  Serial.println("------------------------------read data----------------------------");
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

void sendMessage() {
  Serial.println("Publish message: " + json);
  client.publish(publishTopic.c_str(), json.c_str());
}

void reconnect() {
  Serial.print("Connect with MQTT broker");
  while(!client.connect(SERIAL_NUMBER)) {
    Serial.print(".");
    delay(500);
  }
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
}

void loop() {
  if(!client.connected()) {
    reconnect();
  }

  readData();
  generateJson();

  client.loop();

  sendMessage();
  delay(10000);
}