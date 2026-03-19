#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "networkStruct.h"
#include <inttypes.h>


void fill_mac(identified_network *identified_network, unsigned char *mac)
{
    memcpy(identified_network->mac, mac ,macMaxSize);
    

}
void fill_ssid(identified_network *identified_network, unsigned char *networkName)
{
    memcpy(identified_network->ssid, networkName ,ssidMaxSizeStruct-1 );
    identified_network->ssid[ssidMaxSizeStruct] = 0;
}
void fill_rssi(identified_network *identified_network, int8_t *rssi)
{
     identified_network->rssi = *rssi;
}
void fill_channel(identified_network *identified_network, unsigned char *channel)
{
     identified_network->channel = *channel;
}
void fill_lastSeen(identified_network *identified_network, uint32_t *lastSeen)
{
    identified_network->lastSeen = *lastSeen ;
}
void fill_packetCount(identified_network *identified_network, uint16_t *packetCount)
{
     identified_network->packetCount = *packetCount;
}
void fill_wpsActive(identified_network *identified_network, unsigned char *wpsActive)
{
    identified_network->securityFlags.wpsActive = *wpsActive;
}
void fill_authMode(identified_network *identified_network, unsigned char *authMode)
{
     identified_network->securityFlags.authMode = *authMode ;
}
void fill_pmfRequired(identified_network *identified_network, unsigned char *pmfRequired)
{
    identified_network->securityFlags.pmfRequired = *pmfRequired;
}
void fill_isRogue(identified_network *identified_network, unsigned char *isRogue)
{
    identified_network->securityFlags.isRogue = *isRogue;
}
void fill_reserved(identified_network *identified_network, unsigned char *reserved)
{
    identified_network->securityFlags.reserved = *reserved;
}

void DEBUGSHOWSTRUCT(identified_network *identified_network)
{//TODO eliminar -> debug function, delete
    printf("filled MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
       identified_network->mac[0], identified_network->mac[1],
       identified_network->mac[2], identified_network->mac[3],
       identified_network->mac[4], identified_network->mac[5]);
    printf("networkName : %s",identified_network->ssid);
    printf("rssi : %"PRIu8 "\n", identified_network->rssi);
    printf("Channel : %u", identified_network->channel);
    printf("lastSeen : %"PRIu32 "\n", identified_network->lastSeen);
    printf("packetCount : %"PRIu16 "\n", identified_network->packetCount);
    printf("wpsActive : %u", identified_network->securityFlags.wpsActive);
    printf("authMode : %u", identified_network->securityFlags.authMode);
    printf("pmfRequired : %u", identified_network->securityFlags.pmfRequired);
    printf("isRogue : %u", identified_network->securityFlags.isRogue); 
    printf("reserved : %u", identified_network->securityFlags.reserved);
}

void find_networkStruct()
{
    //TODO implement a hash table for quick search
}