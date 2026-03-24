
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
