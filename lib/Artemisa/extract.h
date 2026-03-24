#ifndef EXTRACT_H
#define EXTRACT_H

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>


#define contentTag 2        //TODO move repetitive tags to a global define header
#define suiteSelectorsSize 4
#define suiteType       3
#define normalizeSplitTag        8
#define defaultWpaMode           2
#define normalizePmf             0x0080

#define vendorTagPosition1  0x00
#define vendorTagPosition2  0x50
#define vendorTagPosition3  0xF2
#define vendorTagPosition4  0x04

#define lengthTag                1
#define ContentTag               2

#define normalizeFlagToDs        0
#define normalizeFlagFromDs      1
#define normalizeFlagMoreFrag    2
#define normalizeFlagRetry       3
#define normalizeFlagPowerMgmt   4
#define normalizeFlagMoreData    5
#define normalizeFlagWep         6
#define normalizeFlagOrder       7
#define normalizeSplitTag        8

#define flagToDs                 0x01
#define flagFromDs               0x02
#define flagMoreFrag             0x04
#define flagRetry                0x08
#define flagPowerManagement      0x10
#define flagMoreData             0x20
#define flagWep                  0x40
#define flagOrder                0x80

#define addresesSize             6
#define nameLengthbite           37
#define nameStartBite            38
#define networkNameMaxLenght     33 // 32char + null terminator

unsigned char* extract_mac_addres(unsigned char *payload, uint_least8_t *flagsBoolean);
int extract_type(unsigned char *payload);
int extract_subtype(unsigned char *payload);
void extract_toDs(unsigned char *payload, uint_least8_t *flagsBoolean);
void extract_fromDs(unsigned char *payload, uint_least8_t *flagsBoolean);

void payload_data_walker(unsigned char *payload, uint16_t totalLenght, identified_network* network);



#endif