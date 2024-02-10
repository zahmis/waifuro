#include "arduino_secrets.h"

#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiS3.h>
#include <WiFiSSLClient.h>
#include <ArduinoHttpClient.h>


#define ONE_WIRE_BUS  2 // DATA
#define SENSER_BIT    1 // 精度

#define WIFI_SSID    SECRET_SSID
#define WIFI_PASSWORD SECRET_PASSWORD
#define SLACK_WEBHOOK_PATH  SECRET_SLACK_WEBHOOK_PATH

WiFiSSLClient wifiClient;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup_wifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to the WiFi network");
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  sensors.setResolution(SENSER_BIT);
}

void loop() {
  sensors.requestTemperatures();
  float f = sensors.getTempCByIndex(0);
  Serial.println(String(f));
  if (f > 50) {
    Serial.println("温度が高いです");
    if(WiFi.status() == WL_CONNECTED){
      if (!wifiClient.connect("hooks.slack.com", 443)) {
        Serial.println("Connection failed");
        return;
      }
      Serial.println("Connected to server");
      wifiClient.println("POST " SLACK_WEBHOOK_PATH " HTTP/1.1");
      wifiClient.println("Host: hooks.slack.com");
      wifiClient.println("Content-Type: application/json");
      wifiClient.println("Content-Length: 35");
      wifiClient.println();
      wifiClient.println("{\"text\":\"温度が高いです\"}");
      wifiClient.println();
      wifiClient.stop();

      Serial.println("Message sent");
    } else {
      Serial.println("WiFi is not connected");
    }

  }
  delay(1000);
}