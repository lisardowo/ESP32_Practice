
#include "validate.h"

bool is_valid_payload(int size)
{
 
    return size >= payloadHeaderSize;
  
}

bool validate_network(unsigned char *subtype)
{   

    if(*subtype == beacon)
    {
        return true;
    }
    return false;

}

