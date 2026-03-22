#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "networkStruct.h"
#include <inttypes.h>
#include "hashing.h"
/* void fill_struct(identified_network *identifiedNetwork, unsigned char *pmfRequired, uint32_t *lastSeen ,unsigned char *mac,unsigned char *networkName, unsigned char *channel  , uint8_t *rssi , unsigned char *authMode, unsigned char *reserved, uint16_t *packetCount, unsigned char *isRogue, unsigned char *wpsActive)
{
    
    identified_network debugStruct2;//TODO debug struct

    fill_authMode(&debugStruct2, authMode);
    fill_channel(&debugStruct2, channel);
    fill_isRogue(&debugStruct2,isRogue);
    fill_lastSeen(&debugStruct2,lastSeen);
    fill_mac(&debugStruct2, mac);
    fill_packetCount(&debugStruct2,packetCount);
    fill_pmfRequired(&debugStruct2,pmfRequired);
    fill_reserved(&debugStruct2,reserved); //TODO addres of operation is for debug (since the nerwokr is created in scope) so it shall be eliminated for prod 
    fill_rssi(&debugStruct2,rssi);
    fill_ssid(&debugStruct2,networkName);
    fill_wpsActive(&debugStruct2,wpsActive);
    DEBUGSHOWSTRUCT(&debugStruct2);//TODO -> DEBUG show filled debug struct
} */

unsigned char* fill_mac(identified_network *identified_network, unsigned char *mac)
{
    return memcpy(identified_network->mac, mac ,macMaxSize);

}
void fill_ssid(identified_network *identified_network, unsigned char *networkName, uint8_t tagLength)
{   
    if (tagLength == 0)
    {
        strcpy(identified_network->ssid, "Hide network");
        return;
    }
    memcpy(identified_network->ssid, networkName ,ssidMaxSizeStruct-2 );//leaves one char for null terminator
    identified_network->ssid[ssidMaxSizeStruct-1] = 0; //fill last char with null terminator
}
void fill_rssi(identified_network *identified_network, uint8_t *rssi)
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
    printf("networkName : %sc",identified_network->ssid);
    printf("rssi : %"PRIu8 "\n", identified_network->rssi);
    printf("Channel : %u", identified_network->channel);
    printf("lastSeen : %"PRIu32 "\n", identified_network->lastSeen);
    printf("packetCount : %"PRIu16 "\n", identified_network->packetCount);
    printf("wpsActive : %u\n", identified_network->securityFlags.wpsActive);
    printf("authMode : %u\n", identified_network->securityFlags.authMode);
    printf("pmfRequired : %u\n", identified_network->securityFlags.pmfRequired);
    printf("isRogue : %u\n", identified_network->securityFlags.isRogue); 
    printf("reserved : %u\n", identified_network->securityFlags.reserved);
}
void create_new_network(unsigned char *mac , uint8_t rssi, unsigned char *ssid, unsigned char channel, uint8_t tagLength)
{
    identified_network *newNetwork = (identified_network*)malloc(sizeof(identified_network));
    if(!newNetwork)
    {
        return;
    }
    fill_mac(newNetwork, mac);
    fill_ssid(newNetwork, ssid, tagLength);
    fill_rssi(newNetwork, &rssi);
    fill_channel(newNetwork, &channel);

    uint32_t now = get_time_ms();
    uint16_t initialCount = 1;
    fill_lastSeen(newNetwork, &now);
    //fill_packetCount(newNetwork, initialCount);

    uint8_t index = convert_to_hash(mac);
    newNetwork->hashNext = hashTable[index];
    hashTable[index] = newNetwork;

    newNetwork->next = head;
    newNetwork->previous = NULL;
    if(head)
    {
        head->previous = newNetwork;
    }
    head = newNetwork;
    if(!tail)
    {
        tail = newNetwork;
    }

}

identified_network* find_network(unsigned char *mac)
{
    //TODO implement a hash table for quick search
    uint8_t index = convert_to_hash(mac);
    identified_network *currentNetwork = hashTable[index];

    while(currentNetwork != NULL)
    {
        if(memcmp(currentNetwork->mac, mac, 6) == 0)
        {
            return currentNetwork;
        }
        currentNetwork = currentNetwork->hashNext;
    }
    return NULL;

}
void delete_old_networks()
{
    uint32_t now = get_time_ms();

    identified_network *current = tail;

    while (current != NULL)
    {
        if ((now - current->lastSeen) > networkTimeout)
        {
            identified_network *toDelete = current;
            current = current->previous;
            delete_network(toDelete);
        }
        else
        {
            break;
        }
    }

}
void delete_network(identified_network *network)
{
    if (network == NULL)
    {
        return;
    }

    uint8_t index = (network->mac[0] ^ network->mac[1] ^ network->mac[2] ^ network->mac[3] ^ network->mac[4] ^ network->mac[5]) % hashSize ;
    identified_network **indirect = &hashTable[index];
    while((*indirect) != NULL && (*indirect) != network)
    {
        indirect = &(*indirect)->hashNext; //takes the addres of the hashNext element of indirect
    }

    if(*indirect == network)
    {
        *indirect = network->hashNext;
    }

    if(network->previous)
    {
        network->previous->next = network->next;
    }
    else
    {
        tail = network->previous;
    }

    free(network);
    


}
void update_network(identified_network *network, uint8_t rssi, uint32_t timestamp)
{
    fill_rssi(network, &rssi);
    fill_lastSeen(network, &timestamp);

    if(network == head) 
    {
        return;
    }
    if(network->previous)
    {
        network->previous->next = network->next;
    }
    if(network->next)
    {
        network->next->previous = network->previous;
    }
    if(network == tail)
    {
        tail = network->previous;
    }

    network->next = head;
    network->previous = NULL;
    if(head)
    {
        head->previous = network;
    }
    head = network;


}

uint32_t get_time_ms()
{
    return(uint32_t)(esp_timer_get_time() / 1000);
}