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

#define maxChannels              13

typedef struct __attribute__((packed)) {

    unsigned char version; //Protocol Version
    unsigned char Type; //funion of frame (management control or data)
    unsigned char Subtype; // indicates frame purpose (?) 0000 for association , 1000 for beacon
    uint_least8_t flagsBoolean;

}  frameControl;

void memory_initializer();
void set_promiscuous();
void channel_swapping();
void sniffed_packets_handler();


#endif