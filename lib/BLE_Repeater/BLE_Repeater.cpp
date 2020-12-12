#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "BLE_Repeater.h"
#include <Arduino.h>
#include <functional>

BLEServer* server = NULL;
BLECharacteristic* uuidCharacteristic = NULL;
BLECharacteristic* commandCharacteristic = NULL;
BLECharacteristic* valueCharacteristic = NULL;

BLECharacteristic* registerCharacteristic = NULL;
BLECharacteristic* batteryCharestiristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

std::function<void(String)> onRegisterCallback = NULL;
std::function<void(String)> onUnregisterCallback = NULL;
std::function<void(String, int)> onBatteryUpdateCallback = NULL;

String connections[32];
uint16_t lastConnectedId;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
      lastConnectedId = pServer->getConnId();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;

      onUnregisterCallback(connections[pServer->getConnId()]);

      connections[pServer->getConnId()] = "";
    }
};

class RegisterCallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        connections[lastConnectedId] = pCharacteristic->getValue().c_str();
        onRegisterCallback(pCharacteristic->getValue().c_str());
    }
};

class BatteryUpdateCallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        String temp = pCharacteristic->getValue().c_str();
        String uuid = temp.substring(0, temp.indexOf('/'));
        temp.remove(0, temp.indexOf('/') + 1);
        onBatteryUpdateCallback(uuid, temp.toInt());
    }
};

BLE_Repeater::BLE_Repeater(
        char* serviceUuid,
        char* uuidCharastericticUuid,
        char* commandCharacteristicUuid,
        char* valueCharacteristicUuid,
        char* registerCharacteristicUuic,
        char* batteryCharacteristicUUid,
        char* name)
    {
    BLEDevice::init(name);

    // Create the BLE Server
    server = BLEDevice::createServer();
    server->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *service = server->createService(serviceUuid);

    // Create a BLE Characteristic
    uuidCharacteristic = service->createCharacteristic(
        uuidCharastericticUuid,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    commandCharacteristic = service->createCharacteristic(
        commandCharacteristicUuid,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    valueCharacteristic = service->createCharacteristic(
        valueCharacteristicUuid,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    registerCharacteristic = service->createCharacteristic(
        registerCharacteristicUuic,
        BLECharacteristic::PROPERTY_WRITE
    );

    batteryCharestiristic = service->createCharacteristic(
        batteryCharacteristicUUid,
        BLECharacteristic::PROPERTY_WRITE
    );

    BLE2902* des1 = new BLE2902();
    des1->setNotifications(true);
    BLE2902* des2 = new BLE2902();
    des2->setNotifications(true);
    BLE2902* des3 = new BLE2902();
    des3->setNotifications(true);
    BLE2902* des4 = new BLE2902();
    BLE2902* des5 = new BLE2902();


    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    // Create a BLE Descriptor
    uuidCharacteristic->addDescriptor(des1);
    commandCharacteristic->addDescriptor(des2);
    valueCharacteristic->addDescriptor(des3);
    registerCharacteristic->addDescriptor(des4);
    batteryCharestiristic->addDescriptor(des5);

    registerCharacteristic->setCallbacks(new RegisterCallback());
    batteryCharestiristic->setCallbacks(new BatteryUpdateCallback());

    Serial.println("[BLE_Repeater]  BLE Name: " + *name);
    Serial.println("[BLE_Repeater]  UUID Char: " + *uuidCharastericticUuid);
    Serial.println("[BLE_Repeater]  UUID Command " + *commandCharacteristicUuid);
    Serial.println("[BLE_Repeater]  UUID Value " + *valueCharacteristicUuid);
    Serial.println("[BLE_Repeater]  UUID Service " + *serviceUuid);

    // Start the service
    service->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(serviceUuid);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("[BLE_Repeater]  Start advertising");
}

BLE_Repeater::~BLE_Repeater() {
    server = NULL;
    uuidCharacteristic = NULL;
    commandCharacteristic = NULL;
    valueCharacteristic = NULL;

    BLEDevice::deinit();
}

void BLE_Repeater::SendPacket(MQTT_Packet* packet) {
    Serial.println("[BLE_Repeater]  Sending message:");
    Serial.println("[BLE_Repeater]  " + *packet->ClientUUID);
    Serial.println("[BLE_Repeater]  " + *packet->Command);
    Serial.println("[BLE_Repeater]  " + *packet->Value);

    uuidCharacteristic->setValue(packet->ClientUUID);
    commandCharacteristic->setValue(packet->Command);
    valueCharacteristic->setValue(packet->Value);

    uuidCharacteristic->notify();
}

void BLE_Repeater::loop() {
    // notify changed value
    if (deviceConnected) {

    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        server->startAdvertising(); // restart advertising
        Serial.println("[BLE_Repeater]  Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}

void BLE_Repeater::SetBateryUpdateCallback(std::function<void(String, int)> callback) {
    onBatteryUpdateCallback = callback;
}

void BLE_Repeater::SetOnUnregisterCallback(std::function<void(String)> callback) {
    onUnregisterCallback = callback;
}

void BLE_Repeater::SetOnRegisterCallback(std::function<void(String)> callback) {
    onRegisterCallback = callback;
}

bool BLE_Repeater::IsRegistered(String adressId) {
    for (int i = 0; i < 32; i++) {
        if (connections[i] == adressId) {
            return true;
        }
    }

    return false;
};