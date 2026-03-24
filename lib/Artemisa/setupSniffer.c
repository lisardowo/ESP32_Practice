
#include "networkStruct.h"
#include "setupSniffer.h"
#include "validate.h"
#include "extract.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "extract.h"

void wifi_stack_init()
{
    static bool init = false;
    if (init)
    {
        return;
    }
    esp_err_t err = esp_netif_init();

    if (err != ESP_OK ) 
    {
        ESP_ERROR_CHECK(err);
    }

     err = esp_netif_init();
    if (err != ESP_OK ) 
    {
        ESP_ERROR_CHECK(err);
    }
}

void channel_swapping(void *parametersTopass)
{
    while(1){
    for(uint_least8_t i = 1; i <= maxChannels; i++) 
    {
        printf("The value is %" PRIu8 "\n", i);
        ESP_ERROR_CHECK(esp_wifi_set_channel(i, WIFI_SECOND_CHAN_NONE));
        vTaskDelay(pdMS_TO_TICKS(3000));
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

   
    printf("starting Promiscuous Setup\n");
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_wifi_init(&config);
    if (err != ESP_OK) {
        printf("Error configurating Wi-Fi: %d\n", err);
        return;
    }

    wifi_mode_t mode = WIFI_MODE_NULL;
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

    printf("promiscuous setup completed \n");
    return;
}

void sniffed_packets_handler(void* buf, wifi_promiscuous_pkt_type_t type){
    

   wifi_promiscuous_pkt_t *packet = (wifi_promiscuous_pkt_t *)buf;

    unsigned char *payload = (unsigned char *)packet->payload;
    uint16_t payloadSize = packet->rx_ctrl.sig_len;
    uint8_t rssi = packet->rx_ctrl.rssi;
    //TODO debug
    if (validate_network(extract_subtype(payload)))
    { 
        uint_least8_t flagsBoolean = 0x00 ;  
    
        extract_toDs(payload, &flagsBoolean);
        extract_fromDs(payload, &flagsBoolean);
    
    //TODO following this comment youll find the LAMEST patch ever created
    //im way too tired for this shit
    //instead of reusing the functions I already worked my ass off ill just re do it here cuz otherwise wont work
    //ill check that later
    
    
        unsigned char *srcMac = extract_mac_addres(payload, &flagsBoolean);

        identified_network *network = find_network(srcMac);



        if(network != NULL)
        {
            update_network(network, rssi, get_time_ms());
        }
        else
        {
            create_new_network(srcMac, rssi, (unsigned char*)"PlaceHolder name..", 0 , 11);
            network = head; 
        }
        //TODO still a lot of stuff to correct
        payload_data_walker(payload, payloadSize,network);
    }
    
}

/*void payload_interpreter(unsigned char *payload, uint16_t payloadSize){

    payload_header_extractor(payload , payloadSize);
    payload_data_walker(payload , payloadSize);

}*/

void sniffer_init(){

    memory_initializer();
    wifi_stack_init();
    set_promiscuous();

    static uint_least8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;
    
    xTaskCreate( channel_swapping , "SWAPPING", 4097, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle);

    esp_err_t callback = esp_wifi_set_promiscuous_rx_cb(&sniffed_packets_handler);
     if (callback != ESP_OK) {
        printf("Error in callback of Sniffed Handler func: %d\n", callback);
        return;
    }
}


