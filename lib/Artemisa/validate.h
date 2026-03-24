#ifndef VALIDATE_H
#define VALIDATE_H

#include <stdbool.h>

#define managementFrame          0
#define beacon                   8
#define payloadHeaderSize        24

bool validate_management_frame(int type);
bool validate_beacon_subtype(int subtype);


#endif