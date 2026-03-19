#ifndef SETUPSNIFFER_H
#define SETUPSNIFFER_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"  
#include "freertos/task.h"       
#include "freertos/queue.h"     

#include "esp_wifi.h"
#include "nvs_flash.h"          
#include "esp_wifi.h"           
#include "esp_event.h"                    
#include "esp_system.h"         

#define maxChannels              11

//TODO 
//IMPORTANT!! ->
/*For some countries, only channels 1 to 11 are valid
In most european countries, chanels up to 13 are allowed
channel 14 is jst for japan. Change based in ur location but is not that nedeed, it will still work just fine

*/

typedef struct __attribute__((packed)) {

    unsigned char version; //Protocol Version
    unsigned char Type; //funion of frame (management control or data)
    unsigned char Subtype; // indicates frame purpose (?) 0000 for association , 1000 for beacon
    uint_least8_t flagsBoolean;

}  frameControl;

void payload_interpreter(unsigned char *payload, uint16_t payloadSize);
void memory_initializer();
void set_promiscuous();
void channel_swapping();
void sniffed_packets_handler();
void sniffer_init();

#endif