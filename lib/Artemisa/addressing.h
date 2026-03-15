#ifndef ADDRESSING_H
#define ADDRESSING_H

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#define extractToAndFromMask     0x03

#define dtlFrames               0x00
#define dsToClient              0x01
#define clientToDS              0x02
#define bridge                  0x03

#define managementFrame         0
#define controlFrame            1
#define dataFrame               2


void frame_type_interpreter(unsigned char *frameType ,unsigned char *payload,uint_least8_t *flagsBoolean);
void type_of_addressing(uint_least8_t booleanFlags, unsigned char *payload);


#endif