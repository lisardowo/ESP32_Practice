#ifndef EXTRACT_H
#define EXTRACT_H

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

#define normalizeFlagToDs        0
#define normalizeFlagFromDs      1
#define normalizeFlagMoreFrag    2
#define normalizeFlagRetry       3
#define normalizeFlagPowerMgmt   4
#define normalizeFlagMoreData    5
#define normalizeFlagWep         6
#define normalizeFlagOrder       7

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

void payload_header_extractor(unsigned char *payload);
void extract_protocol(unsigned char *payload, uint_least8_t *flagsBoolean);
void extract_type(unsigned char *payload, uint_least8_t *flagsBoolean);
void extract_subtype(unsigned char *payload, uint_least8_t *flagsBoolean);
void extract_toDs(unsigned char *payload, uint_least8_t *flagsBoolean);
void extract_retry(unsigned char *payload, uint_least8_t *flagsBoolean);
void extract_powerManagement(unsigned char *payload, uint_least8_t *flagsBoolean);
void extract_wep(unsigned char *payload, uint_least8_t *flagsBoolean);
void extract_order(unsigned char *payload, uint_least8_t *flagsBoolean);
void extract_fromDs(unsigned char *payload, uint_least8_t *flagsBoolean);

void extract_addrs1(unsigned char *payload, const char *type);
void extract_addrs2(unsigned char *payload, const char *type);
void extract_addrs3(unsigned char *payload, const char *type);
void extract_addrs4(unsigned char *payload, const char *type);

void extract_network_name(unsigned char *payload);




#endif