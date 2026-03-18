#ifndef FILLSTRUCT_H
#define FILLSTRUCT_H

#define macMaxSize 6
#define ssidMaxSize 12

typedef struct __attribute__((packed)) {

    uint8_t mac[macMaxSize];           
    char ssid[ssidMaxSize];            
    int8_t rssi;              
    uint8_t channel;          
    uint32_t lastSeen;       
    uint16_t packetCount;    
    
    struct {
        uint8_t wpsActive : 1;
        uint8_t authMode  : 3; 
        uint8_t pmfRequired: 1; 
        uint8_t isRogue   : 1; 
        uint8_t reserved   : 2;
    } securityFlags;

} identified_network;

void fill_mac(identified_network *identified_network, unsigned char *mac);

#endif