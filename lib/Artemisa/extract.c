
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


int extract_type(unsigned char *payload)
{
    
    unsigned char frameControlFragment = payload[0]; //Frame control is from two BYTES (so two fragments)
    unsigned char typeMask = 0x0C; 

    return((frameControlFragment & typeMask) >> 2);
    
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
                    break;
            }

            case DSParameter:
                {
                    uint8_t channel = payload[position + 2];
                    fill_channel(network, &channel);
                   
                    break;
                }


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

}
