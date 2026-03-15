/* 
  HERMES MODULE - Artemisa
  Wifi-Sniffer - library
 */
#include "esp_log.h"

static const char *TAG = "Artemisa"; 
      
#define scanTimeMs               300     
#define radarThresholdNear      -45     
#define radarThresholdFar       -85  



#include "setupSniffer.h"


void app_main(void)
{
    
    ESP_LOGI(TAG, "ESP32 | Wifi Sniffer");

    memory_initializer();

    set_promiscuous();

    static uint_least8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;
    
    xTaskCreate( channel_swapping , "SWAPPING", 4097, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle);
    


}




