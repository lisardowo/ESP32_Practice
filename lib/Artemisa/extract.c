
#include <stdio.h>
#include <string.h>
#include "fillStruct.h"
#include "validate.h"
#include "extract.h"
#include "addressing.h"

#define startTags       36

#define SSID            0x00
#define DSParameter     0x03
#define TIM             0x05
#define BSSLOAD         0x0B
#define RSN             0x30
#define mobility        0x36
#define WPALEGACY       0xDD

//TODO refactorize 2 header files, properly allocate code based in what it does



void extract_protocol(unsigned char *payload, uint_least8_t *flagsBoolean)
{


    unsigned char frameControl = payload[0]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x03; 

    unsigned char protocol = frameControl & mask;

    printf("Protocol : %X\n", protocol);

    //TODO -> DEBUG print, delete 4 production

}

void extract_type(unsigned char *payload, uint_least8_t *flagsBoolean, uint16_t payloadSize)
{

    
    unsigned char frameControlFragment = payload[0]; //Frame control is from two BYTES (so two fragments)
    unsigned char typeMask = 0x0C; 

    unsigned char frameType = (frameControlFragment & typeMask) >> 2;

    printf("type : %X\n", frameType);
    
    frame_type_interpreter(&frameType, payload, payloadSize);
   //TODO debugg
   //TODO if possible id like to use the interpreter OUTSIDE this function
                                                             // , but also wanna avoid returns an stuff due to memory reasons
  
}

void extract_subtype(unsigned char *payload, uint_least8_t *flagsBoolean)
{

    unsigned char frameControlFragment = payload[0]; //Frame control is from two BYTES (so two fragments)
    unsigned char typeMask = 0xF0; 

    unsigned char subtype = (frameControlFragment & typeMask) >> 4;
    
    printf("subtype : %X\n", subtype);

    if (validate_network(&subtype))
    { 
        //extract_network_name(payload);
    }
    

}

void extract_toDs(unsigned char *payload, uint_least8_t *flagsBoolean)
{

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x01; 

    unsigned char toDs = (frameControlFragment & mask) >> normalizeFlagToDs;
    if(toDs){
        *flagsBoolean |= flagToDs;
    }

    printf("toDs : %X\n", toDs);

    
}
void extract_fromDs(unsigned char *payload, uint_least8_t *flagsBoolean)
{

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x02; 

    unsigned char fromDs = (frameControlFragment & mask) >> normalizeFlagFromDs;

    if(fromDs)
    {
        *flagsBoolean |= flagFromDs;
    }
    printf("fromDs : %X\n", fromDs);

}

void extract_retry(unsigned char *payload, uint_least8_t *flagsBoolean)
{

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x08; 

    unsigned char retry = (frameControlFragment & mask) >> normalizeFlagRetry;
    
    if (retry)
    {
        *flagsBoolean |= flagRetry;
    }

    printf("extract_retry : %X\n", retry);
    
}

void extract_powerManagement(unsigned char *payload, uint_least8_t *flagsBoolean)
{

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x10; 

    unsigned char powerManagement = (frameControlFragment & mask) >> normalizeFlagPowerMgmt;

    if (powerManagement)
    {
        *flagsBoolean |= flagPowerManagement;
    }
    printf("power Management : %X\n", powerManagement);

}

void extract_wep(unsigned char *payload, uint_least8_t *flagsBoolean)
{

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x40; 

    unsigned char wep = (frameControlFragment & mask) >> normalizeFlagWep;
    if(wep)
    {
        *flagsBoolean |= flagWep;
    }
    
    
    printf("WEP : %X\n", wep);    
    
}

void extract_order(unsigned char *payload, uint_least8_t *flagsBoolean)
{
    
    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x80; 

    unsigned char order = (frameControlFragment & mask) >> normalizeFlagOrder;
    if(order)
    {
        *flagsBoolean |= flagOrder;
    }
    printf("Order : %X\n", order);
    
}


void extract_more_frag(unsigned char *payload, uint_least8_t *flagsBoolean)
{
    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x04; 

    unsigned char moreFrag = (frameControlFragment & mask) >> normalizeFlagMoreFrag;

    if(moreFrag)
    {
        *flagsBoolean |= flagMoreFrag;
    }

    printf("moreFrag : %X\n", moreFrag);
    
}

void extract_more_data(unsigned char *payload, uint_least8_t *flagsBoolean)
{
    
    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x20; 

    unsigned char moreData = (frameControlFragment & mask) >> normalizeFlagMoreData;

    if(moreData)
    {
        *flagsBoolean |= flagMoreData;
    }
    printf("moreData : %X\n", moreData);
    
}

void extract_addrs1(unsigned char *payload, const char *type)
{
    
    unsigned char destinationAddress[addresesSize];
    memcpy(destinationAddress, &payload[10], 6);
    printf("%s : %02X:%02X:%02X:%02X:%02X:%02X\n", type, destinationAddress[0], destinationAddress[1], destinationAddress[2],destinationAddress[3], destinationAddress[4] ,destinationAddress[5]);
    
}

void extract_addrs2(unsigned char *payload, const char *type)
{
    
    unsigned char destinationAddress[addresesSize];
    memcpy(destinationAddress, &payload[4], 6);
    printf("%s : %02X:%02X:%02X:%02X:%02X:%02X\n",type, destinationAddress[0], destinationAddress[1], destinationAddress[2],destinationAddress[3], destinationAddress[4] ,destinationAddress[5]);
    
}

void extract_addrs3(unsigned char *payload, const char *type) 
{
    
    unsigned char BSSID[addresesSize];
    memcpy(BSSID, &payload[16], 6);
    printf("%s : %02X:%02X:%02X:%02X:%02X:%02X\n", type,BSSID[0], BSSID[1], BSSID[2], BSSID[3], BSSID[4] , BSSID[5]);
    
}

void extract_addrs4(unsigned char *payload, const char *type)
{

    unsigned char address4[6];
    memcpy(address4, &payload[28], 6);
    printf("%s : %02X:%02X:%02X:%02X:%02X:%02X\n",type, address4[0], address4[1], address4[2],address4[3], address4[4] ,address4[5]);
    
}
/*
void extract_network_name(unsigned char *payload)
{


    uint16_t nameLenght = payload[nameLengthbite];
    
    for(uint8_t i = 0 ; i < nameLenght; i++)
    {
        if(i < networkNameMaxLenght) // TODO trying to avoid buffer overflows -> Prob need some work
        {
            printf("%c", payload[nameStartBite + i]);
        } 
    }
    printf("\n");
    

}
TODO working to delete extract_network_name*/

//TODO debug struct
identified_network testNetwork;

void payload_header_extractor(unsigned char *payload, uint16_t payloadSize){ 
    

    printf(" ===== NEW NETWORK =====\n");
    //TODO debug
    uint_least8_t flagsBoolean = 0x00 ;  
    
    extract_type(payload, &flagsBoolean, payloadSize);
    //payload_data_walker(payload, payloadSize);
    //extract_network_name(payload);
    extract_subtype(payload, &flagsBoolean);
    extract_protocol(payload, &flagsBoolean);
    extract_toDs(payload, &flagsBoolean);
    extract_fromDs(payload, &flagsBoolean);
   
    //TODO debuggin sum stuffff

    type_of_addressing(flagsBoolean, payload);
    extract_retry(payload, &flagsBoolean);
    extract_powerManagement(payload, &flagsBoolean);
    extract_wep(payload, &flagsBoolean);
    extract_order(payload, &flagsBoolean);
    fill_mac(&testNetwork, &payload[16]);//TODO -> this is working but im not 100% sure why, check later
    printf("\n===== END OF NETWORK ======\n");//TODO -- debug

    flagsBoolean = 0x00; 


}



void payload_data_walker(unsigned char *payload, uint16_t totalLenght)
{
    uint16_t position = startTags;

    while (position < totalLenght)
    {

        uint8_t tag_id = payload[position];
        uint8_t tag_lenght = payload[position + 1];

        switch(tag_id)
        {
            
            case SSID: 
            {
                unsigned char ssid[tag_lenght];
                    memcpy(ssid, &payload[position + 2], tag_lenght);
                    for(int i = 0 ; i < tag_lenght ; i++)
                    {
                        printf("%c", ssid[i]);
                    }
                    printf("\n");
                    break;
            }

            case DSParameter:
                {
                    unsigned char ds[tag_lenght];
                    memcpy(ds, &payload[position + 2], tag_lenght);
                    for(int i = 0 ; i < tag_lenght ; i++)
                    {
                        printf("%u ", ds[i]);
                    }
                    printf("\n");
                    break;
                }
            case TIM:
               { 
                    unsigned char tim[tag_lenght] ;
                    memcpy(tim, &payload[position + 2], tag_lenght);
                    for(int i = 0 ; i < tag_lenght ; i++)
                    {
                        printf("%u ", tim[i]);
                    }
                    printf("\n");
                    break;
                }

            case BSSLOAD:
                {
                    unsigned char bss[tag_lenght] ;
                    memcpy(bss, &payload[position + 2], tag_lenght);
                    for(int i = 0 ; i < tag_lenght ; i++)
                    {
                        printf("%u ", bss[i]);
                        
                    }
                    printf("\n");
                    break;
                }
            case RSN:
                {
                    unsigned char rsn[tag_lenght];
                    memcpy(rsn, &payload[position + 2], tag_lenght);
                    for(int i = 0 ; i < tag_lenght ; i++)
                    {
                        printf("%u ", rsn[i]);
                    }
                    printf("\n");
                    break;
                }

            case mobility:
                {   
                    unsigned char mob[tag_lenght];
                    memcpy(mob, &payload[position + 2], tag_lenght);
                    for(int i = 0 ; i < tag_lenght ; i++)
                    {
                        printf("%04X ", mob[i]);
                    }
                   printf("\n");
                    break;
                }

            case WPALEGACY:
                {
                    unsigned char wpa[tag_lenght];
                    memcpy(wpa, &payload[position + 2], tag_lenght);
                    for(int i = 0 ; i < tag_lenght ; i++)
                    {
                        printf("%02X ", wpa[i]);
                    }
                    printf("\n");
                    break;
                }
        }

        position += 2 + tag_lenght;
    }

}
