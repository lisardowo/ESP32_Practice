
#include "addressing.h"
#include "extract.h"

void type_of_addressing(uint_least8_t booleanFlags, unsigned char *payload) 
{


    uint_least8_t directionAndAddressing = booleanFlags & extractToAndFromMask; 
        
    //Following is a switch case 
    //that decides what type of comm is to succesfull extract all the information
    //direction addressing is decided following the table found in : unnamedFolder/addresing.svg
                                        
    switch(directionAndAddressing)
    {

        case dtlFrames:

            printf("dtlFrames\n");
            extract_addrs1(payload, "Dest MAC");
            extract_addrs2(payload, "src Mac");
            extract_addrs3(payload, "BSSID");

            break;

        case dsToClient:

            printf("dsToClient\n");
            extract_addrs1(payload, "Dest MAC");
            extract_addrs2(payload,"BSSID");
            extract_addrs3(payload,"Src Mac");

            break;

        case clientToDS:

            printf("clientToDs\n");
            extract_addrs1(payload, "BBSID");
            extract_addrs2(payload, "Src Mac");
            extract_addrs3(payload, "Dest Mac");

            break;

        case bridge: 

            printf("bridge\n");
            extract_addrs1(payload, "Dest Radio");
            extract_addrs2(payload, "Src Radio");
            extract_addrs3(payload, "Dest Mac");
            extract_addrs4(payload, "Src Mac");


            break;

        default:

            printf("unknown address type");
            break;
        

    }
}

void frame_type_interpreter(uint_least8_t *frameType, unsigned char *payload, uint16_t payloadSize)
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
            /*
            Relations Map -> Analize MAC from origin and destiny to identify 
            what devices are connected to what routers
            
            IoT Heartbeats -> look for patrons of small but constant information 
            to identify cameras and sensors
            
            */
            break;
        default:
            printf("not valid data Type");
            break;
        
    }
}

