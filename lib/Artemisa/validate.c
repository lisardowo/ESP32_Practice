
#include "validate.h"

bool is_valid_payload(int size)
{
 
    return size >= payloadHeaderSize;
  
}
bool validate_management_frame(int type)
{
    return type == managementFrame;
}
bool validate_beacon_subtype(int subtype)
{   

    return subtype == beacon;

}

