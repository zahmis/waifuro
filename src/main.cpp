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

// HTTPリクエストの共通部分を関数化
void sendHTTPRequest(String host, String request) {
  wifiClient.println(request);
  wifiClient.println("Host: " + host);
  wifiClient.println("Content-Type: application/json; charset=utf-8");
  wifiClient.println("Authorization: Bearer " SWICTH_BOT_TOKEN);
  wifiClient.println();  // ヘッダーの終了を示す空行
}

void sendToSlack(String message) {
  if (!wifiClient.connect("hooks.slack.com", 443)) {
    Serial.println("Slack Connection failed");
    return;
  }

  Serial.println("Connected to slack server");
  sendHTTPRequest("hooks.slack.com", "POST " SLACK_WEBHOOK_PATH " HTTP/1.1");
  wifiClient.println("Content-Length: " + String(message.length()));
  wifiClient.println();
  wifiClient.println(message);
  Serial.println("Message sent to slack.");
  wifiClient.stop();
}

String getDeviceStatus() {
  sendHTTPRequest("api.switch-bot.com",
                  "GET /v1.0/devices/" DEVICE_ID "/status HTTP/1.1");

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

void sendToSwitchBot() {
  if (!wifiClient.connect("api.switch-bot.com", 443)) {
    Serial.println("Connection failed");
    return;
  }
  Serial.println("Connected to server");
  String status = getDeviceStatus();

  if (status == "off") {
    Serial.println("Already turned off.");
  } else {
    sendHTTPRequest("api.switch-bot.com",
                    "POST /v1.0/devices/" DEVICE_ID "/commands HTTP/1.1");
    String messageS =
        "{\"command\":\"turnOff\",\"parameter\":\"default\",\"commandType\":"
        "\"command\"}";
    wifiClient.println("Content-Length: " + String(messageS.length()));
    wifiClient.println();
    wifiClient.println(messageS);
    Serial.println("Message sent to switch bot.");
    wifiClient.stop();
  }
}

void loop() {
  sensors.requestTemperatures();
  float f = sensors.getTempCByIndex(0);
  Serial.println(String(f));
  if (f > 50) {
    Serial.println("高温注意");
    if (WiFi.status() == WL_CONNECTED) {
      sendToSlack("{\"text\":\"現在温度: " + String(f) + "℃\"}");
      sendToSwitchBot();
      sendToSlack("{\"text\":\":warning: 追い焚きを中止しました:warning: \"}");
    } else {
      Serial.println("WiFi is not connected");
    }
  }
  delay(1000);
}