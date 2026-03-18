#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fillStruct.h"
#include <inttypes.h>


void fill_mac(identified_network *identified_network, unsigned char *mac)
{
    memcpy(identified_network->mac, mac ,macMaxSize);
    printf("filled MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
       identified_network->mac[0], identified_network->mac[1],
       identified_network->mac[2], identified_network->mac[3],
       identified_network->mac[4], identified_network->mac[5]);

}
void fill_ssid()
{
    
}
void fill_rssi()
{
    
}
void fill_channel()
{
    
}
void fill_lastSeen()
{
    
}
void fill_packetCount()
{
    
}
void fill_wpsActive()
{
    
}
void fill_authMode()
{
    
}
void fill_pmfRequired()
{
    
}
void fill_isRogue()
{
    
}
void fill_reserved()
{

}
