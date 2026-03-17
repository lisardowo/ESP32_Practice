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

    sniffer_init();

   
    


}

/*
            Overall checklist of management Frame
            Identify beacons and search for Tag 48 looking for wpa2(secure) or
            uses obsolete prtocols

            Search for tag 221 to identify networks vulnerable to brute force

            Look For subtype 12(0x0C -> deauth atack)

            Look for phantoms (probe request) extracting SSID of mobiles that visit 
            */


/*
#define DSParameter     0x03
#define TIM             0x05
#define BSSLOAD         0x0B
#define RSN             0x30
#define mobility        0x36
#define WPALEGACY       0xDD

void payload_data_walker(unsigned char *payload, uint16_t totalLenght)
{
    uint8_t position = 36;

    while (position < totalLenght)
    {
        uint8_t tag_id = payload[tag_id];
        uint8_t tag_lenght = payload[tag_id + 1];

        switch (tag_id)
        {
            case DSParameter:
                printf("DSPARAMETER");
                break;

            case TIM:
                printf("TIM");
                break;

            case BSSLOAD:
                printf("BSS");
                break;

            case RSN:
                printf("RSN");
                break;

            case mobility:
                printf("MOB");
                break;

            case WPALEGACY:
                printf("WPA");
                break;
        }

        position += 2 + tag_lenght;
    }

}
*/