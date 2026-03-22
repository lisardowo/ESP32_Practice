
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

identified_network testNetwork; //TODO debug -> delete
//TODO refactorize 2 header files, properly allocate code based in what it does



void extract_protocol(unsigned char *payload, uint_least8_t *flagsBoolean)
{


    unsigned char frameControl = payload[0]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x03; 

    unsigned char protocol = frameControl & mask;

    printf("Protocol : %X\n", protocol);

    //TODO -> DEBUG print, delete 4 production

}

void extract_type(unsigned char *payload, uint_least8_t *flagsBoolean, uint16_t payloadSize, identified_network *newNetwork)
{

    
    unsigned char frameControlFragment = payload[0]; //Frame control is from two BYTES (so two fragments)
    unsigned char typeMask = 0x0C; 

    unsigned char frameType = (frameControlFragment & typeMask) >> 2;

    printf("type : %X\n", frameType);
    
    frame_type_interpreter(&frameType, payload, payloadSize, newNetwork);
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
    
    //TODO fill mac shall be inside extract_addrs copying destination address instead of payload
    printf("%s : %02X:%02X:%02X:%02X:%02X:%02X\n", type, destinationAddress[0], destinationAddress[1], destinationAddress[2],destinationAddress[3], destinationAddress[4] ,destinationAddress[5]);
    
}

void extract_addrs2(unsigned char *payload, const char *type)
{
    
    unsigned char address2[addresesSize];
    memcpy(address2, &payload[4], 6);
    printf("%s : %02X:%02X:%02X:%02X:%02X:%02X\n",type, address2[0], address2[1], address2[2],address2[3], address2[4] ,address2[5]);
    
}

void extract_addrs3(unsigned char *payload, const char *type) 
{
    
    unsigned char addres3[addresesSize];
    memcpy(addres3, &payload[16], 6);
    printf("%s : %02X:%02X:%02X:%02X:%02X:%02X\n", type,addres3[0], addres3[1], addres3[2], addres3[3], addres3[4] , addres3[5]);
    
}

void extract_addrs4(unsigned char *payload, const char *type)
{

    unsigned char address4[addresesSize];
    memcpy(address4, &payload[28], 6);
    printf("%s : %02X:%02X:%02X:%02X:%02X:%02X\n",type, address4[0], address4[1], address4[2],address4[3], address4[4] ,address4[5]);
    
}


void payload_header_extractor(unsigned char *payload, uint16_t payloadSize, uint8_t rssi)
{ 
    

    printf(" ===== NEW NETWORK =====\n");
    //TODO debug
    uint_least8_t flagsBoolean = 0x00 ;  
    
    extract_toDs(payload, &flagsBoolean);
    extract_fromDs(payload, &flagsBoolean);
    
    //TODO following this comment youll find the LAMEST patch ever created
    //im way too tired for this shit
    //instead of reusing the functions I already worked my ass off ill just re do it here cuz otherwise wont work
    //ill check that later
    unsigned char srcMac[macMaxSize];
    uint_least8_t direction = flagsBoolean & extractToAndFromMask;
    unsigned char *macAddress = type_of_addressing(direction, payload);

    identified_network *newNetwork = find_network(macAddress);

    uint32_t now = get_time_ms();

    if(newNetwork)
    {
        update_network(newNetwork, rssi, now);
    }
    else
    {
        create_new_network(srcMac, rssi, (unsigned char*)"PlaceHolder name..", 0 , 11);
        newNetwork = head; 
    }

    extract_type(payload, &flagsBoolean, payloadSize, newNetwork);
    //TODO still a lot of stuff to correct
    flagsBoolean = 0x00; 


}



void payload_data_walker(unsigned char *payload, uint16_t totalLenght)
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
                    
                    fill_ssid(&testNetwork , &payload[position + 2], tagLenght);
                    
                    printf("\n");
                    break;
            }

            case DSParameter:
                {
                    uint8_t channel = payload[position + 2];
                    fill_channel(&testNetwork, &channel);
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
                    fill_packetCount(&testNetwork, &packetCount);
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
                        fill_authMode(&testNetwork, &mode);

                        currentPosition += (akmCount * suiteSelectorsSize);
                        uint16_t rsnCaps = rsnData[currentPosition] | (rsnData[currentPosition + 1] << 8 );
                        uint8_t pmf = (rsnCaps & normalizePmf) >> 7 ;

                        fill_pmfRequired(&testNetwork, &pmf);
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
                        fill_wpsActive(&testNetwork,&wps);
                    }
                    break;
                }
        }
        
        position += 2 + tagLenght;
    }
    DEBUGSHOWSTRUCT(&testNetwork);
}
