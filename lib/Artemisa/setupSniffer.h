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



void memory_initializer();
void set_promiscuous();
void channel_swapping(void *parametersTopass);
void sniffed_packets_handler(void* buf, wifi_promiscuous_pkt_type_t type);
void sniffer_init();

#endif