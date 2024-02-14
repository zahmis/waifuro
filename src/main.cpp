#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <WiFiS3.h>
#include <WiFiSSLClient.h>

#include "Arduino_LED_Matrix.h"
#include "arduino_secrets.h"
#include "fonts.h"

#define ONE_WIRE_BUS 2  // DATA
#define SENSER_BIT 1    // 精度

WiFiSSLClient wifiClient;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
ArduinoLEDMatrix matrix;
uint8_t frame[8][12] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

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
  matrix.begin();
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
  wifiClient.connect("api.switch-bot.com", 443);
  wifiClient.println("GET /v1.0/devices/" DEVICE_ID "/status HTTP/1.1");
  wifiClient.println("Host: api.switch-bot.com");
  wifiClient.println("Authorization : Bearer " SWICTH_BOT_TOKEN);
  wifiClient.println();

  // Skip HTTP headers　bodyも読み込む
  while (wifiClient.connected()) {
    String line = wifiClient.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }

  // Read the body
  String jsonResponse = wifiClient.readStringUntil('\n');
  wifiClient.stop();
  Serial.println("Response from SwitchBot API:");
  Serial.println(jsonResponse);

  // JSONをパース
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, jsonResponse);
  // デバイスの状態を取得
  String status = doc["body"]["power"];
  return status;
}

void clear_frame() {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 12; col++) {
      frame[row][col] = 0;
    }
  }
}

void display_frame() { matrix.renderBitmap(frame, 8, 12); }

void add_to_frame(int index, int pos) {
  for (int row = 0; row < 8; row++) {
    uint32_t temp = fonts[index][row] << (7 - pos);
    for (int col = 0; col < 12; col++) {
      frame[row][col] |= (temp >> (11 - col)) & 1;
    }
  }
}

void loop() {
  sensors.requestTemperatures();
  float f = sensors.getTempCByIndex(0);

  int th, tz;
  th = f / 10;
  tz = (int)f % 10;

  clear_frame();
  add_to_frame(th, -1);
  add_to_frame(tz, 3);
  add_to_frame(10, 6);
  // add_to_frame(11, 7);

  display_frame();

  Serial.println(String(f));
  Serial.println(String(th));
  if (f > 50) {
    Serial.println("高温注意");
    String status = getDeviceStatus();
    Serial.println(status);
    if (WiFi.status() == WL_CONNECTED) {
      sendToSlack("{\"text\":\"現在温度: " + String(f) + "℃\"}");

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