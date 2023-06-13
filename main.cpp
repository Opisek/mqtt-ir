// Library Section

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <config.h>

// Topic Section
// Configure based on your experimental results
// See line 68 for configuring your protocol

#define CODE_POWER 0xDC3
#define CODE_SPEED 0xD90
#define CODE_ROTATION 0xD84
#define CODE_MODE 0xD88
#define CODE_MIST 0xD81

#define MQTT_TOPIC_COUNT 5
#define MQTT_TOPIC_POWER "power"
#define MQTT_TOPIC_SPEED "speed"
#define MQTT_TOPIC_MODE "mode"
#define MQTT_TOPIC_ROTATION "rotation"
#define MQTT_TOPIC_MIST "mist"
const char*[MQTT_TOPIC_COUNT] mqttTopics = { MQTT_TOPIC_POWER, MQTT_TOPIC_SPEED, MQTT_TOPIC_MODE, MQTT_TOPIC_MODE, MQTT_TOPIC_MIST };

// Code Section

void mqttCallback(char* topic, byte* payload, unsigned int length);
void connectWifi();

WiFiClient wifiClient;
PubSubClient mqttClient(MQTT_ADDRESS, MQTT_PORT, mqttCallback, wifiClient);
IRsend sender(IR_PIN);

String mqttName;

struct {
  bool power;
  int speed;
  int mode;
  bool rotation;
  bool mist;
  unsigned long lastIR;
  
} state;

void setup() {
  //Serial.begin(115200);
  sender.begin();

  // Connect WIFI
  IPAddress lanAddress, lanGateway, lanSubnet;
  lanAddress.fromString(LAN_ADDRESS);
  lanGateway.fromString(LAN_GATEWAY);
  lanSubnet.fromString(LAN_SUBNET);
  WiFi.config(lanAddress, lanGateway, lanSubnet);
  WiFi.mode(WIFI_STA);

  // Connect MQTT
  mqttName = BOARD "-" MQTT_USERNAME "-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  for (int i = 0; i < 6; ++i) mqttName += String(mac[i], 16);

  // Start Parameters
  state.power = false;
  state.speed = 1;
  state.mode = 1;
  state.rotation = false;
  state.mist = false;
  state.lastIR = millis();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) { // connect wifi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) delay(100);
  } else if (!mqttClient.connected()) { // connect mqtt
    if (mqttClient.connect((char*)mqttName.c_str(), MQTT_USERNAME, MQTT_PASSWORD, MQTT_BASE "available", 0, true, "offline")) {
      for (int i = 0; i < MQTT_TOPIC_COUNT; ++i) mqttClient.subscribe(mqttTopics[i]);
      mqttClient.publish(MQTT_BASE "available", "online", true);
    }
  } else mqttClient.loop(); // loop mqtt
  delay(100);
}

void mqttInitialCallback(char* topic, byte* payload, unsigned int length) {
  if (length != 1) return;
  switch (topic) {
    case MQTT_TOPIC_POWER: state.power = payload[0]; break;
    case MQTT_TOPIC_SPEED: state.speed = payload[0]; break;
    case MQTT_TOPIC_MODE: state.mode = payload[0]; break;
    case MQTT_TOPIC_ROTATION: state.rotation = payload[0]; break;
    case MQTT_TOPIC_MIST: state.mist = payload[0]; break;
    default:
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  if (millis() < 1000) return mqttInitialCallback(topic, payload, length);

  switch (topic) {
    case MQTT_TOPIC_POWER:
      if (length == 1 && payload[0] == state.power) return;
      if (length != 1) mqttClient.publish(MQTT_BASE MQTT_TOPIC_POWER, state.power, true);

      state.power = !state.power;
      sendIR(CODE_POWER);

      if (!state.power) {
        state.mist = false;
        mqttClient.publish(MQTT_BASE MQTT_TOPIC_MIST, 0, true);
      }
      break;

    case MQTT_TOPIC_SPEED:
      if (length == 1 && payload[0] == state.speed) return;
      if (length != 1 || payload[0] > 3 || payload[0] == 0) {
        mqttClient.publish(MQTT_BASE MQTT_TOPIC_SPEED, state.speed, true);
        return;
      }

      if (!state.power) {
        mqttClient.publish(MQTT_BASE POWER, 1, true);
        state.power = true;
        delay(50);
      }

      for (; state.speed != payload[0]; state.speed = (state.speed % 3) + 1) {
        sendIR(CODE_SPEED);
        delay(50);
      }
      break;

    case MQTT_TOPIC_MODE:
      if (payload[0] == state.mode) return;
      if (!state.power || length != 1 || payload[0] > 3 || payload[0] == 0) {
        mqttClient.publish(MQTT_BASE MQTT_TOPIC_MODE, state.mode, true);
        return;
      }

      for (; state.mode != payload[0]; state.mode = (state.mode % 3) + 1) {
        sendIR(CODE_MODE);
        delay(50);
      }
      break;

    case MQTT_TOPIC_ROTATION:
      if (length == 1 && payload[0] == state.rotation) return;
      if (length != 1 || !state.power) {
        mqttClient.publish(MQTT_BASE MQTT_TOPIC_ROTATION, state.rotation, true);
        return;
      }

      state.rotation = !state.rotation;
      sendIR(CODE_ROTATION)
      break;

    case MQTT_TOPIC_MIST:
      if (length == 1 && payload[0] == state.mist) return;
      if (length != 1) mqttClient.publish(MQTT_BASE MQTT_TOPIC_MIST, state.mist, true);

      state.mist = !state.mist;
      sendIR(CODE_MIST);
      break;

    default:
  }
}

void sendIR(uint64_t data) {
  if (state.lastIR > 30 * 1000) {
    sender.sendSymphony(MQTT_TOPIC_POWER, IR_BITS, IR_REPEAT);
    delay(50);
  }
  sender.sendSymphony(data, IR_BITS, IR_REPEAT);
}