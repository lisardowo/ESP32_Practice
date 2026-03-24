
#include <stdio.h>
#include <string.h>
#include "networkStruct.h"
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
    
    //frame_type_interpreter(&frameType, payload, payloadSize);
   //TODO debugg
   //TODO if possible id like to use the interpreter OUTSIDE this function
                                                             // , but also wanna avoid returns an stuff due to memory reasons
  
}

int extract_subtype(unsigned char *payload)
{

    unsigned char frameControlFragment = payload[0]; //Frame control is from two BYTES (so two fragments)
    unsigned char typeMask = 0xF0; 

    return( (frameControlFragment & typeMask) >> 4);


    

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

unsigned char* extract_mac_addres(unsigned char *payload, uint_least8_t *flagsBoolean)
{
    
    uint_least8_t direction = *flagsBoolean & extractToAndFromMask;
    return type_of_addressing(direction, payload);

}



void payload_data_walker(unsigned char *payload, uint16_t totalLenght, identified_network* network)
{
    uint16_t position = startTags;

    while (position + 2 < totalLenght)
    {

        uint8_t tagId = payload[position];
        uint8_t tagLenght = payload[position + 1];
        
        if ((uint32_t)position + 2u + tagLenght > totalLenght) 
        {
            break; 
        }

        switch(tagId)
        {
            
            case SSID: 
            {
                    
                    fill_ssid(network , &payload[position + 2], tagLenght);
                    
                    printf("\n");
                    break;
            }

            case DSParameter:
                {
                    uint8_t channel = payload[position + 2];
                    fill_channel(network, &channel);
                    //printf("DS param : ");
                    break;
                }
          /* TODO is case tim really needed? 
          case TIM:
               { 
                    unsigned char tim[tag_lenght] ;
                    memcpy(tim, &payload[position + 2], tag_lenght);
    
                    printf("\n");
                    break;
                }*/

            case BSSLOAD:
                {
                    uint16_t packetCount = payload[position + ContentTag] | (payload[position + (ContentTag + 1)]) << normalizeSplitTag ;
                    fill_packetCount(network, &packetCount);
                    break;
                }
                
            case RSN:
                {
                       uint8_t *rsnData = &payload[position + contentTag];
                        int jumpVersion = 6;
                        uint16_t pairwiseCipherCount = rsnData[jumpVersion] | (rsnData[jumpVersion + 1] << 8);
                        uint8_t currentPosition = jumpVersion + contentTag + (pairwiseCipherCount * suiteSelectorsSize);
                        uint16_t akmCount = rsnData[currentPosition] | ( rsnData[currentPosition + 1] << normalizeSplitTag);
                        currentPosition += contentTag;
                        uint8_t *firstAkm = &rsnData[currentPosition];
                        uint8_t mode = defaultWpaMode;

                        if (firstAkm[suiteType] == 8 || firstAkm[suiteType] == 18)
                        {
                            mode = 3;
                        }
                        fill_authMode(network, &mode);

                        currentPosition += (akmCount * suiteSelectorsSize);
                        uint16_t rsnCaps = rsnData[currentPosition] | (rsnData[currentPosition + 1] << 8 );
                        uint8_t pmf = (rsnCaps & normalizePmf) >> 7 ;

                        fill_pmfRequired(network, &pmf);
                        break;
                }

            /*TODO i think that mobility is also not neccesary
            case mobility:
                {   
                    unsigned char mob[tagLenght];
                    memcpy(mob, &payload[position + 2], tagLenght);
                    printf("Mobility : ");
                    for(int i = 0 ; i < tagLenght ; i++)
                    {
                        printf("%04X ", mob[i]);
                    }
                   printf("\n");
                    break;
                }*/

            case WPALEGACY:
                {
                    uint8_t *vendor = &payload[position + contentTag];
                    if (tagLenght >= 4 && vendor[0] == vendorTagPosition1 && vendor[1] == vendorTagPosition2 && vendor[2] == vendorTagPosition3 && vendor[3] == vendorTagPosition4)
                    {
                        uint8_t wps = 1;
                        fill_wpsActive(network,&wps);
                    }
                    break;
                }
        }
        
        position += 2 + tagLenght;
    }
    DEBUGSHOWSTRUCT(network);
}
