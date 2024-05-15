#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"

#define WIFI_SSID           "Wokwi-GUEST"
#define WIFI_PASSWORD       ""
#define TOKEN               "FwrpQlb5o1UMvH5zfzYI"
#define THINGSBOARD_SERVER  "thingsboard.cloud"
#define SERIAL_DEBUG_BAUD   115200
#define DHT_PIN             15

DHTesp dhtSensor;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void connectToMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT Server...");
    if (mqttClient.connect("ESPDevice", TOKEN, NULL)) {
      Serial.println("Connected to MQTT Server");
    } else {
      Serial.print("Failed, rc=");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(SERIAL_DEBUG_BAUD);
  connectToWiFi();
  mqttClient.setServer(THINGSBOARD_SERVER, 1883);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
}

void loop() {
  delay(1000);

  if (!mqttClient.connected()) {
    connectToMQTT();
  }

  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  if (!isnan(data.temperature) && !isnan(data.humidity)) {
    String telemetry = String("{\"temperature\":") + data.temperature + ",\"humidity\":" + data.humidity + "}";
    mqttClient.publish("v1/devices/me/telemetry", telemetry.c_str());
    Serial.print("Temperature: ");
    Serial.print(data.temperature);
    Serial.print(" Humidity: ");
    Serial.println(data.humidity);
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }

  mqttClient.loop();
}