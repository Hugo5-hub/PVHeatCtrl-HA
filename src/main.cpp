/*
  PVHeatCtrl-HA - ESP32 firmware

  The ES32C14 uses UART0 for its onboard RS485 interface:
  GPIO1 (TX), GPIO3 (RX), GPIO22 (DE/RE).
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <ModbusMaster.h>

#if __has_include("secrets.h")
#include "secrets.h"
#else
const char* WIFI_SSID = "YOUR_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASS";
const char* MQTT_HOST = "mqtt.example.local";
const uint16_t MQTT_PORT = 1883;
const char* MQTT_USER = "";
const char* MQTT_PASS = "";
#endif

const char* TOPIC_CONFIG_MAX = "pvheatctrl/config/max_power_w";
const char* TOPIC_CMD_TARGET  = "pvheatctrl/cmd/target_w";
const char* TOPIC_STATE_PREFIX = "pvheatctrl/state/"; // suffixe: target_w, output_v, temp1_c, temp2_c, status, max_power_w
const char* TOPIC_AVAILABILITY = "pvheatctrl/state/status";
const char* TOPIC_DISCOVERY_PREFIX = "homeassistant";

const int DEFAULT_MAX_POWER_W = 3000;

// --- Hardware / Modbus ---
#define RS485_RX_PIN 3
#define RS485_TX_PIN 1
#define RS485_DE_PIN 22

// PTDNC04 slave and registers (aus Manual)
const uint8_t MODBUS_SLAVE = 1;   // dein Wert
const uint16_t REG_TEMPS_START = 0x0000; // CH0..CH7, CH0@0x0000, CH1@0x0001

// ES32C14 Vo1: onboard 0-10 V output, ESP32 DAC1 on GPIO25.
const int OUTPUT_DAC_PIN = 25;

// --- Globals ---
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Preferences prefs;
ModbusMaster node; // single PTDNC04 on bus

int maxPowerW = DEFAULT_MAX_POWER_W;
int currentTargetW = 0;
float currentOutputV = 0.0f;

unsigned long lastModbus = 0;
unsigned long lastPublish = 0;

// --- Helper: publish state ---
void publishState(const char* suffix, const String &payload, bool retained=false) {
  String topic = String(TOPIC_STATE_PREFIX) + suffix;
  mqttClient.publish(topic.c_str(), payload.c_str(), retained);
}

void publishDiscovery(const char* component, const char* objectId, const String &payload) {
  String topic = String(TOPIC_DISCOVERY_PREFIX) + "/" + component + "/pvheatctrl/" + objectId + "/config";
  mqttClient.publish(topic.c_str(), payload.c_str(), true);
}

void publishHomeAssistantDiscovery() {
  const String device = "\"dev\":{\"ids\":[\"pvheatctrl\"],\"name\":\"PV Heat Controller\",\"mf\":\"PVHeatCtrl\",\"mdl\":\"ESP32\"}";
  publishDiscovery("number", "target_power", "{\"name\":\"Target power\",\"uniq_id\":\"pvheatctrl_target_power\",\"cmd_t\":\"" + String(TOPIC_CMD_TARGET) + "\",\"stat_t\":\"" + String(TOPIC_STATE_PREFIX) + "target_w\",\"avty_t\":\"" + String(TOPIC_AVAILABILITY) + "\",\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\",\"unit_of_meas\":\"W\",\"min\":0,\"max\":10000,\"step\":1,\"mode\":\"box\"," + device + "}");
  publishDiscovery("number", "max_power", "{\"name\":\"Maximum power\",\"uniq_id\":\"pvheatctrl_max_power\",\"cmd_t\":\"" + String(TOPIC_CONFIG_MAX) + "\",\"stat_t\":\"" + String(TOPIC_STATE_PREFIX) + "max_power_w\",\"avty_t\":\"" + String(TOPIC_AVAILABILITY) + "\",\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\",\"unit_of_meas\":\"W\",\"min\":0,\"max\":10000,\"step\":1,\"mode\":\"box\"," + device + "}");
  publishDiscovery("sensor", "output_voltage", "{\"name\":\"Output voltage\",\"uniq_id\":\"pvheatctrl_output_voltage\",\"stat_t\":\"" + String(TOPIC_STATE_PREFIX) + "output_v\",\"avty_t\":\"" + String(TOPIC_AVAILABILITY) + "\",\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\",\"unit_of_meas\":\"V\",\"device_class\":\"voltage\",\"state_class\":\"measurement\"," + device + "}");
  publishDiscovery("sensor", "temperature_1", "{\"name\":\"Temperature 1\",\"uniq_id\":\"pvheatctrl_temperature_1\",\"stat_t\":\"" + String(TOPIC_STATE_PREFIX) + "temp1_c\",\"avty_t\":\"" + String(TOPIC_AVAILABILITY) + "\",\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\",\"unit_of_meas\":\"°C\",\"device_class\":\"temperature\",\"state_class\":\"measurement\"," + device + "}");
  publishDiscovery("sensor", "temperature_2", "{\"name\":\"Temperature 2\",\"uniq_id\":\"pvheatctrl_temperature_2\",\"stat_t\":\"" + String(TOPIC_STATE_PREFIX) + "temp2_c\",\"avty_t\":\"" + String(TOPIC_AVAILABILITY) + "\",\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\",\"unit_of_meas\":\"°C\",\"device_class\":\"temperature\",\"state_class\":\"measurement\"," + device + "}");
  publishDiscovery("sensor", "status", "{\"name\":\"Status\",\"uniq_id\":\"pvheatctrl_status\",\"stat_t\":\"" + String(TOPIC_STATE_PREFIX) + "status\",\"avty_t\":\"" + String(TOPIC_AVAILABILITY) + "\",\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\"," + device + "}");
}

// --- Output control ---
void setOutputVoltage(float v) {
  if (v < 0.0f) v = 0.0f;
  if (v > 10.0f) v = 10.0f;
  // The ES32C14 scales the ESP32 DAC range 0..255 to approximately 0..10 V.
  uint8_t dacValue = (uint8_t) round((v / 10.0f) * 255.0f);
  dacWrite(OUTPUT_DAC_PIN, dacValue);
  currentOutputV = v;
}

void modbusPreTransmission() {
  digitalWrite(RS485_DE_PIN, HIGH);
  delayMicroseconds(100);
}

void modbusPostTransmission() {
  Serial.flush();
  digitalWrite(RS485_DE_PIN, LOW);
}

// --- Handle incoming target (W) ---
void handleTargetW(int target) {
  if (target < 0) target = 0;
  if (target > maxPowerW) target = maxPowerW;
  currentTargetW = target;
  float outV = (maxPowerW > 0) ? ((float)target / (float)maxPowerW * 10.0f) : 0.0f;
  setOutputVoltage(outV);
  publishState("target_w", String(currentTargetW));
  publishState("output_v", String(currentOutputV));
}

// --- Modbus helpers ---
// Read a single signed 16-bit register and return true on success
bool modbusReadRegisterInt16(uint8_t slaveId, uint16_t regAddr, int16_t &outValue) {
  node.begin(slaveId, Serial);
  uint8_t result = node.readHoldingRegisters(regAddr, 1);

  if (result == node.ku8MBSuccess) {
    uint16_t raw = node.getResponseBuffer(0);
    outValue = (int16_t)raw; // interpret as signed 16-bit
    return true;
  }
  return false;
}

// Read two temps (CH0, CH1) from slave and publish
void pollAndPublishTemps() {
  int16_t raw0, raw1;
  bool ok0 = modbusReadRegisterInt16(MODBUS_SLAVE, REG_TEMPS_START + 0, raw0);
  delay(20);
  bool ok1 = modbusReadRegisterInt16(MODBUS_SLAVE, REG_TEMPS_START + 1, raw1);

  if (ok0) {
    float t0 = ((float)raw0) / 10.0f;
    publishState("temp1_c", String(t0));
  } else {
    publishState("status", "modbus_err_temp1");
  }

  if (ok1) {
    float t1 = ((float)raw1) / 10.0f;
    publishState("temp2_c", String(t1));
  } else {
    publishState("status", "modbus_err_temp2");
  }
}

// --- MQTT callback ---
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);
  String msg;
  for (unsigned int i=0;i<length;i++) msg += (char)payload[i];

  if (t == String(TOPIC_CONFIG_MAX)) {
    int v = msg.toInt();
    if (v >= 0) {
      maxPowerW = v;
      prefs.putInt("max_power_w", v);
      publishState("max_power_w", String(maxPowerW), true);
      // re-evaluate current target
      handleTargetW(currentTargetW);
    }
  } else if (t == String(TOPIC_CMD_TARGET)) {
    int target = msg.toInt();
    handleTargetW(target);
  }
}

// --- WiFi / MQTT connect helpers ---
void connectWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial2.print("Connecting WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(250);
    Serial2.print(".");
  }
  Serial2.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial2.print("WiFi connected, IP: ");
    Serial2.println(WiFi.localIP());
  } else {
    Serial2.println("WiFi connect failed");
  }
}

void connectMQTT() {
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(1024);
  if (mqttClient.connected()) return;

  Serial2.print("Connecting MQTT...");
  String clientId = "pvheatctrl_" + String((uint32_t)ESP.getEfuseMac());

  bool ok = false;
  if (strlen(MQTT_USER) > 0) {
    ok = mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASS,
                            TOPIC_AVAILABILITY, 0, true, "offline");
  } else {
    ok = mqttClient.connect(clientId.c_str(), TOPIC_AVAILABILITY, 0, true,
                            "offline");
  }

  if (ok) {
    Serial2.println(" connected");
    publishHomeAssistantDiscovery();
    // publish online/status (retained)
    publishState("status", "online", true);
    mqttClient.subscribe(TOPIC_CONFIG_MAX);
    mqttClient.subscribe(TOPIC_CMD_TARGET);
  } else {
    Serial2.println(" failed");
  }
}

void setup() {
  // UART0 is connected to the onboard RS485 transceiver on ES32C14.
  Serial.begin(9600, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  delay(500);

  prefs.begin("pvheat", false);
  maxPowerW = prefs.getInt("max_power_w", DEFAULT_MAX_POWER_W);
  Serial2.printf("Loaded maxPowerW=%d\n", maxPowerW);

  // Onboard 0-10 V output Vo1 (DAC1 / GPIO25)
  setOutputVoltage(0.0f);

  // RS485 pins
  pinMode(RS485_DE_PIN, OUTPUT);
  digitalWrite(RS485_DE_PIN, LOW); // default RX

  node.begin(MODBUS_SLAVE, Serial);
  node.preTransmission(modbusPreTransmission);
  node.postTransmission(modbusPostTransmission);

  // WiFi + MQTT
  connectWifi();
  connectMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWifi();
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  unsigned long now = millis();

  if (now - lastModbus > 5000) {
    lastModbus = now;
    pollAndPublishTemps();
  }

  if (now - lastPublish > 10000) {
    lastPublish = now;
    publishState("target_w", String(currentTargetW));
    publishState("output_v", String(currentOutputV));
    publishState("max_power_w", String(maxPowerW), true);
  }
}
