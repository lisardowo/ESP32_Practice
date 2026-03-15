
#include "setupSniffer.h"
#include "validate.h"
#include "extract.h"

void channel_swapping()
{
    while(1){
    for(uint_least8_t i = 1; i <= maxChannels; i++) 
    {
        printf("The value is %" PRIu8 "\n", i);
        ESP_ERROR_CHECK(esp_wifi_set_channel(i, WIFI_SECOND_CHAN_NONE));
        vTaskDelay(3000);
    }
    }
}

void memory_initializer(){
    
    //Inizialating the memory --> Checking for possible errors: if found, trying again and verifying
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

}

void set_promiscuous(){

    vTaskDelay(pdMS_TO_TICKS(10000)); //TODO debug time -> Reduce or eliminar
    printf("starting Promiscuous Setup\n");
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_wifi_init(&config);
    if (err != ESP_OK) {
        printf("Error configurating Wi-Fi: %d\n", err);
        return;
    }

    wifi_mode_t mode = WIFI_MODE_APSTA;
    err = esp_wifi_set_mode(mode);
    if (err != ESP_OK) {
        printf("Error selecting Wi-Fi  mode: %d\n", err);
        return;
    }

    esp_err_t start = esp_wifi_start();
    if (start != ESP_OK) {
        printf("Error starting Wi-Fi: %d\n", err);
        return;
    }
    esp_err_t setProm = esp_wifi_set_promiscuous(true);
    if (setProm != ESP_OK) {
        printf("Error activating Prom Mode Wi-Fi: %d\n", err);
        return;
    }



    esp_err_t callback = esp_wifi_set_promiscuous_rx_cb(&sniffed_packets_handler);
     if (callback != ESP_OK) {
        printf("Error in callback of Sniffed Handler func: %d\n", err);
        return;
    }

    printf("promiscuous setup completed \n");
    return;
}

void sniffed_packets_handler(void* buf, wifi_promiscuous_pkt_type_t type){
    
   wifi_promiscuous_pkt_t *packet = (wifi_promiscuous_pkt_t *)buf;

    unsigned char *payload = (unsigned char *)packet->payload;
    uint16_t payloadSize = packet->rx_ctrl.sig_len;

    if(is_valid_payload(payloadSize))
    {          
        payload_header_extractor(payload);
    }
}