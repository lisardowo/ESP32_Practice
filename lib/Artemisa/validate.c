
#include "validate.h"

bool validate_management_frame(int type)
{
    return type == managementFrame;
}
bool validate_beacon_subtype(int subtype)
{   

    return subtype == beacon;

}

