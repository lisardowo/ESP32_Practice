#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "networkStruct.h"
#include <inttypes.h>
#include "hashing.h"
int compare_network_rssi(const void *a, const void *b);

identified_network* hashTable[hashSize] = {NULL};
identified_network* head = NULL; 
identified_network* tail = NULL;



unsigned char* fill_mac(identified_network *identified_network, unsigned char *mac)
{
    return memcpy(identified_network->mac, mac ,macMaxSize);

}
void fill_ssid(identified_network *identified_network, unsigned char *networkName, uint8_t tagLength)
{   
    memset(identified_network->ssid, 0 , ssidMaxSizeStruct);
    if (tagLength == 0)
    {
        strcpy(identified_network->ssid, "Hiden network");
        return;
    }
    uint8_t copyString = (tagLength < ssidMaxSizeStruct -1) ? tagLength : ssidMaxSizeStruct -1;
    memcpy(identified_network->ssid, networkName ,copyString );//leaves one char for null terminator
    identified_network->ssid[ssidMaxSizeStruct-1] = 0; //fill last char with null terminator
}

void fill_rssi(identified_network *identified_network, int8_t *rssi)
{
     identified_network->rssi = *rssi;
}
void fill_channel(identified_network *identified_network, uint8_t *channel)
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

void create_new_network(unsigned char *mac , int8_t rssi, unsigned char *ssid, unsigned char channel, uint8_t tagLength)
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
    fill_lastSeen(newNetwork, &now);

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
void update_network(identified_network *network, int8_t rssi, uint32_t timestamp)
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
void display_networks()
{
    uint16_t networkCount = 0;
    identified_network *currentNetwork = head;
    while(currentNetwork)
    {
        networkCount ++;
        currentNetwork = currentNetwork->next;
    }

    if (networkCount == 0)
    {
        printf("no devices detected");
        return;
    }

    identified_network **sortedNetworks = malloc(sizeof(identified_network*) * networkCount);
    if (!sortedNetworks) 
    {
        return;
    }

    currentNetwork = head;
    for (int i = 0 ; i < networkCount ; i++)
    {
        sortedNetworks[i] = currentNetwork;
        currentNetwork = currentNetwork->next;
    }

    qsort(sortedNetworks, networkCount , sizeof(identified_network*), compare_network_rssi);

    printf(" === Founded Networks === \n");

    printf("%-18s | %-15s | %-4s | %-5s | %-6s | %-4s |\n ", "BSSID", "SSID", "CH", "RSSI", "AUTH", "WPS");

    printf(" ======================== \n");
    
    for (int i = 0 ; i < networkCount ; i++)
    {
        identified_network *displayNetwork = sortedNetworks[i];

        const char* wpaVersionString = (displayNetwork->securityFlags.authMode == 3) ? "WPA3" : (displayNetwork->securityFlags.authMode == 2) ? "WPA2" : "NO WPA";  

        printf(" %02x:%02x:%02x:%02x:%02x:%02x | %-15s | %-4u | %-5d | %-6s | %-4s\n", displayNetwork->mac[0] , displayNetwork->mac[1] ,
                                                                                       displayNetwork->mac[2] , displayNetwork->mac[3] , displayNetwork->mac[4] , displayNetwork->mac[5] ,
                                                                                       displayNetwork->ssid,displayNetwork->channel,displayNetwork->rssi, wpaVersionString, displayNetwork->securityFlags.wpsActive ? "WPS active" : "WPS not found"
                                                                                    );
        printf("total founded networks : %d", networkCount);

    }
    free(sortedNetworks);
}

int compare_network_rssi(const void *a, const void *b)
{
    identified_network *networkA = *(identified_network **)a;
    identified_network *networkB = *(identified_network **)b;
    if (networkA->rssi < networkB->rssi)
    {
        return 1;
    } 
    if (networkA->rssi > networkB->rssi)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

uint32_t get_time_ms()
{
    return(uint32_t)(esp_timer_get_time() / 1000);
}