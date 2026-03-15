#ifndef VALIDATE_H
#define VALIDATE_H

#include <stdbool.h>

#define beacon                   8
#define payloadHeaderSize        24

bool validate_network(unsigned char *subtype);
bool is_valid_payload(int size);


#endif