/* 
  HERMES MODULE - Artemisa
  Wifi-Sniffer
 */

#include <stdio.h>              
#include <string.h>             
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"  
#include "freertos/task.h"       
#include "freertos/queue.h"     

#include "esp_wifi.h"
#include "nvs_flash.h"          
#include "esp_wifi.h"           
#include "esp_event.h"          
#include "esp_log.h"            
#include "esp_system.h"         

#define DEBUG_Size 32 // Tamaño ajustado para evitar desbordamientos

static const char *TAG = "Artemisa"; 

#define MAX_CHANNELS        13      // WI-FI in 2.4GHz
#define SCAN_TIME_MS        300     
#define RADAR_THRESHOLD_NEAR -45     
#define RADAR_THRESHOLD_FAR  -85   

unsigned int duration;
unsigned int destinationAddress;
unsigned int sourceAddress;
unsigned int bssidSource;
unsigned int bssidDestination;

void frame_interpreter(unsigned char *payload);// Crea la structura -> la llena -> regresa la estructura llena
char extract_protocol(unsigned char *payload);
char extract_type(unsigned char *payload);
char extract_subtype(unsigned char *payload);
bool extract_toDs(unsigned char *payload);
bool extract_fromDs(unsigned char *payload);
bool extract_retry(unsigned char *payload);
bool extract_powerManagement(unsigned char *payload);
char extract_wep(unsigned char *payload);
bool extract_order(unsigned char *payload);

typedef struct __attribute__((packed)) {

    int16_t frame_control_field;
    int16_t duration; //TODO Is this interesting 4 us ? 
    uint8_t destination_addres;
    uint8_t source_addres;
    int16_t sequence_control;
}  DETECTED_WIFI;


typedef struct __attribute__((packed)) {

    unsigned char version; //Protocol Version
    unsigned char Type; //funion of frame (management control or data)
    unsigned char Subtype; // indicates frame purpose (?) 0000 for association , 1000 for beacon
    bool toDs; // indicates destination frame
    bool fromDs; // indicates if it comes from DS
    bool moreFragments; // if there are any following fragments
    bool retry; // indicates if is a retranssmission TODO -> Is it important? like security-relevant I guess
    bool powerManagement; // true -> indicates if sender goes into power-save mode 
    unsigned char webEncriptyionProtocolVersion; 
    bool order; //if true, franmes must be proccessed in strict order


}  frameControl;

unsigned char extract_sourceAddress();
unsigned char *extract_destinationAddress(unsigned char *payload);
unsigned char extract_SSID();
unsigned char extract_senderSSID();
void memory_Initializer();
int set_Promiscuous();
void Channel_Swapping();
void sniffed_Packets_Handler();

void app_main(void)
{
    
    ESP_LOGI(TAG, "ESP32 | Wifi Sniffer");

    memory_Initializer();

    esp_err_t set_prom = set_Promiscuous();

    if(set_prom == ESP_OK ){
        printf("Promiscuous : OK\n");
    }
    else if (set_prom == ESP_ERR_WIFI_NOT_INIT || set_prom == ESP_ERR_INVALID_ARG ){
        printf("Error %d", set_prom);
    }
    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;
    
    xTaskCreate( Channel_Swapping , "SWAPPING", 4097, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle);
    


   while(1) {
        ESP_LOGI(TAG, "OS. Free heap: %d bytes", esp_get_free_heap_size());
        vTaskDelay(pdMS_TO_TICKS(10000)); 
    }
    
}

void Channel_Swapping()
{
    while(1){
    for(uint8_t i = 1; i <= MAX_CHANNELS; i++) // Cambiar i > 13 a i < MAX_CHANNELS
    {
        printf("The value is %" PRIu8 "\n", i);
        ESP_ERROR_CHECK(esp_wifi_set_channel(i, WIFI_SECOND_CHAN_NONE));
        vTaskDelay(3000);
    }
    }
}

void memory_Initializer(){
    
    //Inizialating the memory --> Checking for possible errors: if found, trying again and verifying
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

}

int set_Promiscuous(){

    vTaskDelay(pdMS_TO_TICKS(10000)); //TODO debug time -> Reduce or eliminar
    printf("starting Promiscuous Setup\n");
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_wifi_init(&config);
    if (err != ESP_OK) {
        printf("Error configurating Wi-Fi: %d\n", err);
        return err;
    }

    wifi_mode_t mode = WIFI_MODE_APSTA;
    err = esp_wifi_set_mode(mode);
    if (err != ESP_OK) {
        printf("Error selecting Wi-Fi  mode: %d\n", err);
        return err;
    }

    esp_err_t start = esp_wifi_start();
    if (start != ESP_OK) {
        printf("Error starting Wi-Fi: %d\n", err);
        return err;
    }
    esp_err_t setProm = esp_wifi_set_promiscuous(true);
    if (setProm != ESP_OK) {
        printf("Error activating Prom Mode Wi-Fi: %d\n", err);
        return err;
    }



    esp_err_t callback = esp_wifi_set_promiscuous_rx_cb(&sniffed_Packets_Handler);
     if (callback != ESP_OK) {
        printf("Error in callback of Sniffed Handler func: %d\n", err);
        return err;
    }

    printf("promiscuous setup completed \n");
    return ESP_OK;
}

void sniffed_Packets_Handler(void* buf, wifi_promiscuous_pkt_type_t type){
    
   wifi_promiscuous_pkt_t *packet = (wifi_promiscuous_pkt_t *)buf;

    unsigned char *payload = (unsigned char *)packet->payload;

    for(int i = 0 ; i < sizeof(payload) + 1;i++)
    {
    printf("Byte %d: 0x%02x\n", i ,(unsigned char)payload[i]); 
    }                                      
    extract_destinationAddress(payload);
}

void frame_interpreter(unsigned char *payload){

    extract_protocol(payload);
    extract_type(payload);
    extract_subtype(payload);
    extract_toDs(payload);
    extract_retry(payload);
    extract_powerManagement(payload);
    extract_wep(payload);
    extract_order(payload);
    extract_fromDs(payload);

}


char extract_protocol(unsigned char *payload){
    return 0;
}

char extract_type(unsigned char *payload){
    return 0;
}
char extract_subtype(unsigned char *payload){
    return 0;
}
bool extract_toDs(unsigned char *payload){
    return false;
}
bool extract_fromDs(unsigned char *payload){
    return false;
}


unsigned char extract_sourceAddress(unsigned char *payload){
    
    return sourceAddress;
}
unsigned char* extract_destinationAddress(unsigned char *payload){
    
    unsigned char destinationAddress[6];
    memcpy(destinationAddress, &payload[10], 6);
    printf("Addres : %02X:%02X:%02X:%02X:%02X:%02X\n", destinationAddress[0], destinationAddress[1], destinationAddress[2],destinationAddress[3], destinationAddress[4] ,destinationAddress[5]);
    return 0;// destinationAddress;
}
unsigned char extract_SSID(unsigned char *payload){
    
    unsigned char SSID[6];
    memcpy(SSID, &payload[10], 6);
    printf("Addres : %02X:%02X:%02X:%02X:%02X:%02X\n", SSID[0], SSID[1], SSID[2],SSID[3], SSID[4] ,SSID[5]);
    
    return 0;//SSID;
}
unsigned char extract_senderSSID(unsigned char *payload){

    unsigned char senderSSID[6];
    memcpy(senderSSID, &payload[10], 6);
    printf("Addres : %02X:%02X:%02X:%02X:%02X:%02X\n", senderSSID[0], senderSSID[1], senderSSID[2],senderSSID[3], senderSSID[4] ,senderSSID[5]);
    
    return 0 ;//senderSSID;
}
bool extract_retry(unsigned char *payload){return false;}
bool extract_powerManagement(unsigned char *payload){return false;}
char extract_wep(unsigned char *payload){return 0;}
bool extract_order(unsigned char *payload){return false;}