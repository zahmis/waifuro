#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <WiFiS3.h>
#include <WiFiSSLClient.h>

#include "arduino_secrets.h"
#define ONE_WIRE_BUS 2  // DATA
#define SENSER_BIT 1    // 精度

WiFiSSLClient wifiClient;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void connectToWiFi() {
  WiFi.begin(WIFISSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to the WiFi network");
}

void setup() {
  Serial.begin(9600);
  connectToWiFi();
  sensors.setResolution(SENSER_BIT);
}

void sendToSlack(String message) {
  if (!wifiClient.connect("hooks.slack.com", 443)) {
    Serial.println("Slack Connection failed");
    return;
  }
  Serial.println("Connected to slack.");
  wifiClient.println("POST " SLACK_WEBHOOK_PATH " HTTP/1.1");
  wifiClient.println("Host: hooks.slack.com");
  wifiClient.println("Content-Type: application/json");
  wifiClient.println("Content-Length: " + String(message.length()));
  wifiClient.println();
  wifiClient.println(message);
  Serial.println("Message sent to slack.");
  wifiClient.stop();
}

void sendToSwitchBot() {
  if (!wifiClient.connect("api.switch-bot.com", 443)) {
    Serial.println("Switch bot Connection failed");
    return;
  }
  Serial.println("Connected to switch bot.");

  wifiClient.println("POST /v1.0/devices/" DEVICE_ID "/commands HTTP/1.1");
  wifiClient.println("Host: api.switch-bot.com");
  wifiClient.println("Content-Type: application/json");
  wifiClient.println("Authorization: Bearer " SWICTH_BOT_TOKEN);
  String messageS =
      "{\"command\":\"turnOff\",\"parameter\":\"default\",\"commandType\":"
      "\"command\"}";
  wifiClient.println("Content-Length: " + String(messageS.length()));
  wifiClient.println();
  wifiClient.println(messageS);
  wifiClient.stop();

  Serial.println("Message sent to switch bot.");
}

String getDeviceStatus() {
  while (wifiClient.connected()) {
    String line = wifiClient.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  String jsonResponse = wifiClient.readStringUntil('\n');
  Serial.println(jsonResponse);

  // JSONをパース
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, jsonResponse);

  // デバイスの状態を取得（デバイスの状態はdeviceStateというキーに格納されていると仮定）
  String status = doc["body"]["power"];
  return status;
}

void loop() {
  sensors.requestTemperatures();
  float f = sensors.getTempCByIndex(0);
  Serial.println(String(f));
  if (f > 50) {
    Serial.println("高温注意");
    if (WiFi.status() == WL_CONNECTED) {
      sendToSlack("{\"text\":\"現在温度: " + String(f) + "℃\"}");

      String status = getDeviceStatus();

      if (status == "on") {
        sendToSwitchBot();
        sendToSlack(
            "{\"text\":\":warning: 追い焚きを中止しました:warning: \"}");
      } else {
        Serial.println("Already turned off.");
      }
    } else {
      Serial.println("WiFi is not connected");
    }
  }
  delay(1000);
}