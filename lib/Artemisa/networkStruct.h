#ifndef NETWORKSTRUCT_H
#define NETWORKSTRUCT_H

#include <stdbool.h>
#include "hashing.h"
#include "esp_timer.h"

#define macMaxSize               6
#define ssidMaxSizeStruct        24

#define contentTag               2//TODO move repetitive tags to a global define header
#define suiteSelectorsSize       4
#define suiteType                3
#define normalizeSplitTag        8
#define defaultWpaMode           2
#define normalizePmf             0x0080
#define networkTimeout           60000

typedef struct identified_network {

    struct identified_network *next;
    struct identified_network *previous;
    struct identified_network *hashNext;
  
    
    //relevant pointers 4 hashing

    uint32_t lastSeen;       
    uint16_t packetCount;  

    char ssid[ssidMaxSizeStruct]; // tried to use unsigned char jst to keep all the same but strcpy a biiiitch            
    unsigned char mac[macMaxSize];            
    int8_t rssi;              
    uint8_t channel;          

    struct {
        uint8_t wpsActive : 1;
        uint8_t authMode  : 3; 
        uint8_t pmfRequired: 1; 
        uint8_t isRogue   : 1; 
        uint8_t reserved   : 2;
    } securityFlags;

} identified_network;

unsigned char* fill_mac(identified_network *identified_network, unsigned char *mac);
void fill_ssid(identified_network *identified_network, unsigned char *networkName,uint8_t tagLength);
void fill_rssi(identified_network *identified_network, uint8_t *rssi);
void fill_channel(identified_network *identified_network, uint8_t *channel);
void fill_lastSeen(identified_network *identified_network, uint32_t *lastSeen);
void fill_packetCount(identified_network *identified_network, uint16_t *packetCount);
void fill_wpsActive(identified_network *identified_network, unsigned char *wpsActive);
void fill_authMode(identified_network *identified_network, unsigned char *authMode);
void fill_pmfRequired(identified_network *identified_network, unsigned char *pmfRequired);
void fill_isRogue(identified_network *identified_network, unsigned char *isRogue);
void fill_reserved(identified_network *identified_network, unsigned char *reserved);
void DEBUGSHOWSTRUCT(identified_network *identified_network);

void create_new_network(unsigned char *mac , uint8_t rssi, unsigned char *ssid, unsigned char channel, uint8_t tagLength);
identified_network* find_network(unsigned char *mac);
void delete_old_networks();
void update_network(identified_network *network, uint8_t rssi, uint32_t timestamp);
void delete_network(identified_network *network);

uint32_t get_time_ms(); //TODO not sure if this belongs here, i may move it


identified_network* hashTable[hashSize] = {NULL};
identified_network* head = NULL; 
identified_network* tail = NULL;
uint8_t networkCount = 0;

#endif