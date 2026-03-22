
#include "hashing.h"

uint8_t convert_to_hash(unsigned char *mac)
{
    return (mac[0] ^ mac[1] ^ mac[2] ^ mac[3] ^ mac[4] ^ mac[5] ) % hashSize;
}