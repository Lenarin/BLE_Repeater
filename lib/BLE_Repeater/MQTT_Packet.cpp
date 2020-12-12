#include <MQTT_Packet.h>

MQTT_Packet::MQTT_Packet(const char* clientUUID, const char* command, const char* value) {
    MQTT_Packet::ClientUUID = (char*) clientUUID;
    MQTT_Packet::Command = (char*) command;
    MQTT_Packet::Value = (char*) value;
}