#include "arduino_secrets.h"

#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiS3.h>
#include <WiFiSSLClient.h>
#include <ArduinoHttpClient.h>


#define ONE_WIRE_BUS  2 // DATA
#define SENSER_BIT    1 // 精度

WiFiSSLClient wifiClient;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup_wifi() {
  WiFi.begin(WIFISSID, PASSWORD);
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
  if (f > 40) {
    Serial.println("⚠️高温注意⚠️");
    if(WiFi.status() == WL_CONNECTED){
      if (!wifiClient.connect("hooks.slack.com", 443)) {
        Serial.println("Slack Connection failed");
        return;
      }
      Serial.println("Connected to server");
      wifiClient.println("POST " SLACK_WEBHOOK_PATH " HTTP/1.1");
      wifiClient.println("Host: hooks.slack.com");
      wifiClient.println("Content-Type: application/json");
      String message = "{\"text\":\"Too hot! " + String(f) + "℃\"}";
      wifiClient.println("Content-Length: " + String(message.length())); // Content-Length が一致しないと送れない
      wifiClient.println();
      wifiClient.println(message);
      Serial.println("Message sent to slack.");
      wifiClient.stop();

      // ここからSwitchBotへのリクエスト処理
      if (!wifiClient.connect("api.switch-bot.com", 443)) {
        Serial.println("Connection failed");
        return;
      }

     Serial.println("Connected to server");
     wifiClient.println("POST /v1.0/devices/"DEVICE_ID"/commands HTTP/1.1");
     wifiClient.println("Host: api.switch-bot.com");
     wifiClient.println("Content-Type: application/json");
     wifiClient.println("Authorization: Bearer "SWICTH_BOT_TOKEN);
     String messageS = "{\"command\":\"turnOff\",\"parameter\":\"default\",\"commandType\":\"command\"}";
     wifiClient.println("Content-Length: " + String(messageS.length()));
     wifiClient.println();
     wifiClient.println(messageS);
     Serial.println("Message sent to switch bot.");

      while (wifiClient.connected()) {
       String line = wifiClient.readStringUntil('\n');

        if (line == "\r") {
        Serial.println("Headers received");
        break;
        }
     } 

  // Read and print the response
  String line = wifiClient.readStringUntil('\n');
  Serial.println("Response: ");
  Serial.println(line);
      wifiClient.stop();
    } else {
      Serial.println("WiFi is not connected");
    }
  }
  delay(1000);
}