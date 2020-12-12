#ifndef _BLE_REPEATER_H
#define _BLE_REPEATER_H

#include "./MQTT_Packet.h"
#include <functional>
#include <Arduino.h>

class BLE_Repeater {
public:
    BLE_Repeater(char* serviceUuid,
        char* uuidCharastericticUuid,
        char* commandCharacteristicUuid,
        char* valueCharacteristicUuid,
        char* registerCharacteristicUuic,
        char* batteryCharacteristicUUid,
        char* name);
    ~BLE_Repeater();

    void SendPacket(MQTT_Packet* packet);
    void SetOnRegisterCallback(std::function<void(String)> callback);
    void SetOnUnregisterCallback(std::function<void(String)> callback);
    void SetBateryUpdateCallback(std::function<void(String, int)> callback);
    bool IsRegistered(String adressId);
    void loop();
private:

};

#endif