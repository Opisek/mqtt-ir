// Library Section

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <config.h>

// Topics

#define IR_BUTTONS 6
std::pair<const char*, int> mqttTopics[IR_BUTTONS] = {
  std::make_pair(MQTT_BASE "on", 0xDC3),
  std::make_pair(MQTT_BASE "lamp", 0xD82),
  std::make_pair(MQTT_BASE "cycle", 0xD90),
  std::make_pair(MQTT_BASE "red", 0xD88),
  std::make_pair(MQTT_BASE "green", 0xD84),
  std::make_pair(MQTT_BASE "blue", 0xD81)
};

// Code Section

void mqttCallback(char* topic, byte* payload, unsigned int length);
void connectWifi();

WiFiClient wifiClient;
PubSubClient mqttClient(MQTT_ADDRESS, MQTT_PORT, mqttCallback, wifiClient);
IRsend sender(IR_PIN);

String mqttName;

void setup() {
  //Serial.begin(115200);
  sender.begin();

  IPAddress lanAddress, lanGateway, lanSubnet;
  lanAddress.fromString(LAN_ADDRESS);
  lanGateway.fromString(LAN_GATEWAY);
  lanSubnet.fromString(LAN_SUBNET);
  WiFi.config(lanAddress, lanGateway, lanSubnet);
  WiFi.mode(WIFI_STA);

  mqttName = BOARD "-" MQTT_USERNAME "-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  for (int i = 0; i < 6; ++i) mqttName += String(mac[i], 16);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) { // connect wifi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) delay(100);
  } else if (!mqttClient.connected()) { // connect mqtt
    if (mqttClient.connect((char*)mqttName.c_str(), MQTT_USERNAME, MQTT_PASSWORD, MQTT_BASE "available", 0, true, "offline")) {
      for (int i = 0; i < IR_BUTTONS; ++i) mqttClient.subscribe(mqttTopics[i].first);
      mqttClient.publish(MQTT_BASE "available", "online", true);
    }
  } else mqttClient.loop(); // loop mqtt
  delay(100);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  for (int i = 0; i < IR_BUTTONS; ++i) {
    if (std::strcmp(topic, mqttTopics[i].first) == 0) {
      sender.sendSymphony(mqttTopics[i].second, 12, 5);
      break;
    }
  }
}