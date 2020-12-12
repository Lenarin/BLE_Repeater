#include <BLE_Repeater.h>
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define UUID_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define COMMAND_CHARACTERISTIC_UUID "bc4e2979-0d35-4d57-8754-4408a0cd8b3c"
#define VALUE_CHARACTERISTIC_UUID "c011e213-6a31-45bf-b09d-69c367bca009"
#define REGISTER_CHARACTERISTIC_UUID "5b700253-a658-4d3c-bb94-669130a8259b"
#define BATTERY_CHARACTERISTIC_UUID "8c95c505-1ad1-406b-9353-15c3a86205a6"

// Server params
const char* ssid = "LaLaLa";
const char* password = "13377332";

const char* mqtt_server = "192.168.1.67";
const char* login = "test-server";
const char* pass = "123";
const char* clientId = "test";

BLE_Repeater* ble_repeater = NULL;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.println(topic);

  Serial.print("payload: ");
  String message = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println();

  String s = String(topic);
  String uuid = s.substring(0, s.indexOf('/'));
  s.remove(0, s.indexOf('/')+1);

  MQTT_Packet* packet = new MQTT_Packet(uuid.c_str(), s.c_str(), message.c_str());
  ble_repeater->SendPacket(packet);
}

WiFiClient espClient;
PubSubClient client(espClient);


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientId, login, pass)) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void onRegisterCallback(String deviceUuid) {
  deviceUuid.concat("/connection");
  Serial.println(deviceUuid);
  client.publish(deviceUuid.c_str(), "1");
}

void onUnregisterCallback(String deviceUuid) {
  deviceUuid.concat("/connection");
  client.publish(deviceUuid.c_str(), "0");
}

void onBatteryUpdateCallback(String deviceUuid, int val) {
  deviceUuid.concat("/battery");
  client.publish(deviceUuid.c_str(), String(val).c_str());
}

void setup() {
  Serial.begin(115200);

  ble_repeater = new BLE_Repeater(
    SERVICE_UUID,
    UUID_CHARACTERISTIC_UUID,
    COMMAND_CHARACTERISTIC_UUID,
    VALUE_CHARACTERISTIC_UUID,
    REGISTER_CHARACTERISTIC_UUID,
    BATTERY_CHARACTERISTIC_UUID,
    "Test Repeater"
  );

  ble_repeater->SetBateryUpdateCallback(onBatteryUpdateCallback);
  ble_repeater->SetOnRegisterCallback(onRegisterCallback);
  ble_repeater->SetOnUnregisterCallback(onUnregisterCallback);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  // Allow the hardware to sort itself out
  delay(1500);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  ble_repeater->loop();
}