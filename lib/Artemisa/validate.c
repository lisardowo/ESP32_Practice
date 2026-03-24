
#include "validate.h"

bool is_valid_payload(int size)
{
 
    return size >= payloadHeaderSize;
  
}

bool validate_network(int subtype)
{   

    return subtype == beacon;

}

