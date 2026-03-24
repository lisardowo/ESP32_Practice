
#include "addressing.h"
#include "extract.h"
#include "networkStruct.h"

unsigned char* type_of_addressing(uint_least8_t direction, unsigned char *payload) 
{

        
    //Following is a switch case 
    //that decides what type of comm is to succesfull extract all the information
    //direction addressing is decided following the table found in : unnamedFolder/addresing.svg
                                        
    switch(direction)
    {

        case clientToDS:
            return &payload[16];
        case bridge:
            return &payload[28];
        default:
            return &payload[10];
        

    }
}
/*
void frame_type_interpreter(uint_least8_t *frameType, unsigned char *payload, uint16_t payloadSize, identified_network *newNetwork)
{
    
    switch(*frameType)
    {  
        case managementFrame:
            //TODO - managementFrame
            
            Overall checklist of management Frame
            Identify beacons and search for Tag 48 looking for wpa2(secure) or
            uses obsolete prtocols

            Search for tag 221 to identify networks vulnerable to brute force

            Look For subtype 12(0x0C -> deauth atack)

            Look for phantoms (probe request) extracting SSID of mobiles that visit 
                       
           payload_data_walker(payload, payloadSize);
            break;
        case controlFrame:
            printf("control\n");
            //TODO - controlframe
            
            Density identifier -> A high count of Subtype 1 movement suggest high count
            of devices (human activity)
            
            Identify congestion of channel -> monitoring petitions to know how interfeered 
            will LoRa comms be
            
            break;
        case dataFrame:
            //TODO - dataFrame
            printf("dataframe\n");
            
            Relations Map -> Analize MAC from origin and destiny to identify 
            what devices are connected to what routers
            
            IoT Heartbeats -> look for patrons of small but constant information 
            to identify cameras and sensors
            
            
            break;
        default:
            printf("not valid data Type\n");
            break;
        
    }
} 
I think im not gonna use this at all
*/
