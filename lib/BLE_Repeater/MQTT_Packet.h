#ifndef _MQTT_PACKET_H
#define _MQTT_PACKET_H

class MQTT_Packet {
public:
    MQTT_Packet(const char* clientUUID, const char* command, const char* value);
    char*           ClientUUID;
    char*           Command;
    char*           Value;
};

#endif