#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define SERIAL_NUMBER "NMCU_1589714856299"

Adafruit_BME280 bme;

float temperature, humidity, pressure, altitude;

void readData() {
  Serial.println("------------------------------read data----------------------------");
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  bme.begin(0x76);

  Serial.println();
}

void loop() {
  readData();

  Serial.println("temp = " + String(temperature));
  Serial.println("hum = " + String(humidity));
  Serial.println("pres = " + String(pressure));
  Serial.println("alt = " + String(altitude));
  delay(10000);
}