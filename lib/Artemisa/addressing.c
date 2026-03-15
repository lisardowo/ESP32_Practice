
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

void frame_type_interpreter(uint_least8_t *frameType, unsigned char *payload, uint_least8_t *flagsBoolean)
{
    
    switch(*frameType)
    {  
        case managementFrame:
            printf("%"PRIuLEAST8, *frameType);
            break;
        case controlFrame:
            printf("%"PRIuLEAST8, *frameType);
            break;
        case dataFrame:
            printf("%"PRIuLEAST8, *frameType);
            break;
        default:
            printf("not valid data Type");
            break;
        
    }
}

