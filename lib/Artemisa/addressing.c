
#include "addressing.h"
#include "extract.h"
#include "networkStruct.h"

unsigned char* type_of_addressing(uint_least8_t direction, unsigned char *payload, identified_network *identifiedNetwork) 
{

        
    //Following is a switch case 
    //that decides what type of comm is to succesfull extract all the information
    //direction addressing is decided following the table found in : unnamedFolder/addresing.svg
                                        
    switch(direction)
    {

        case dtlFrames:

            printf("dtlFrames\n");
            extract_addrs1(payload, "Dest MAC");
            extract_addrs2(payload, "src Mac");
            extract_addrs3(payload, "BSSID");
            return fill_mac(identifiedNetwork, &payload[10]);//TODO -> this is working but im not 100% sure why, check later
            break;

        case dsToClient:

            printf("dsToClient\n");
            extract_addrs1(payload, "Dest MAC");
            extract_addrs2(payload,"BSSID");
            extract_addrs3(payload,"Src Mac");
            return fill_mac(identifiedNetwork, &payload[10]);
            break;

        case clientToDS:

            printf("clientToDs\n");
            extract_addrs1(payload, "BBSID");
            extract_addrs2(payload, "Src Mac");
            extract_addrs3(payload, "Dest Mac");
            return fill_mac(identifiedNetwork, &payload[16]);
            break;

        case bridge: 

            printf("bridge\n");
            extract_addrs1(payload, "Dest Radio");
            extract_addrs2(payload, "Src Radio");
            extract_addrs3(payload, "Dest Mac");
            extract_addrs4(payload, "Src Mac");
            return fill_mac(identifiedNetwork, &payload[28]);

            break;

        default:

            printf("unknown address type");
            break;
        

    }
}

void frame_type_interpreter(uint_least8_t *frameType, unsigned char *payload, uint16_t payloadSize, identified_network *newNetwork)
{
    
    switch(*frameType)
    {  
        case managementFrame:
            //TODO - managementFrame
            /*
            Overall checklist of management Frame
            Identify beacons and search for Tag 48 looking for wpa2(secure) or
            uses obsolete prtocols

            Search for tag 221 to identify networks vulnerable to brute force

            Look For subtype 12(0x0C -> deauth atack)

            Look for phantoms (probe request) extracting SSID of mobiles that visit 
            */
           
           payload_data_walker(payload, payloadSize);
            break;
        case controlFrame:
            printf("control\n");
            //TODO - controlframe
            /*
            Density identifier -> A high count of Subtype 1 movement suggest high count
            of devices (human activity)
            
            Identify congestion of channel -> monitoring petitions to know how interfeered 
            will LoRa comms be
            */
            break;
        case dataFrame:
            //TODO - dataFrame
            printf("dataframe\n");
            /*
            Relations Map -> Analize MAC from origin and destiny to identify 
            what devices are connected to what routers
            
            IoT Heartbeats -> look for patrons of small but constant information 
            to identify cameras and sensors
            
            */
            break;
        default:
            printf("not valid data Type\n");
            break;
        
    }
}

