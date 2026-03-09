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
void extract_protocol(unsigned char *payload);
char extract_type(unsigned char *payload);
char extract_subtype(unsigned char *payload);
bool extract_toDs(unsigned char *payload);
bool extract_fromDs(unsigned char *payload);
bool extract_retry(unsigned char *payload);
bool extract_powerManagement(unsigned char *payload);
char extract_wep(unsigned char *payload);
bool extract_order(unsigned char *payload);
bool is_valid_payload(int size);

typedef struct __attribute__((packed)) {

    int16_t frame_control_field;
    int16_t duration; //TODO Is this interesting 4 us ? 
    unsigned char destination_addres[6];
    unsigned char source_addres[6];
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
    bool order; //if true, franmes must be proccessed in strict order
    unsigned char webEncriptyionProtocolVersion; 

}  frameControl;

void extract_sourceAddress();
void extract_destinationAddress(unsigned char *payload);
void extract_SSID();
void extract_senderSSID();
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
    is_valid_payload(sizeof(payload));          
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


void extract_protocol(unsigned char *payload){

    unsigned char frameControl = payload[0]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0xC0; //11000000

    unsigned char protocol = frameControl & mask;

    printf("Protocol : %X", protocol);
    //TODO -> DEBUG print, delete 4 production

}

char extract_type(unsigned char *payload){

    unsigned char frameControlFragment = payload[0]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x30; //00110000

    unsigned char type = frameControlFragment & mask;

    printf("type : %X", type);

    return 0;
}
char extract_subtype(unsigned char *payload){

    unsigned char frameControlFragment = payload[0]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0xF; // 00001111

    unsigned char subtype = frameControlFragment & mask;

    printf("Protocol : %X", subtype);

    return 0;
}
bool extract_toDs(unsigned char *payload){

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x80; // 10000000

    unsigned char toDs = frameControlFragment & mask;

    printf("Protocol : %X", toDs);

    return false;
}
bool extract_fromDs(unsigned char *payload){

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x40; // 01000000

    unsigned char fromDs = frameControlFragment & mask;

    printf("Protocol : %X", fromDs);

    return false;
}
bool extract_retry(unsigned char *payload)
{
    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x10; // 00010000

    unsigned char retry = frameControlFragment & mask;

    printf("Protocol : %X", retry);

    return false;
}
bool extract_powerManagement(unsigned char *payload)
{
    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x8; // 00001000

    unsigned char powerManagement = frameControlFragment & mask;

    printf("Protocol : %X", powerManagement);

    return false;
}
char extract_wep(unsigned char *payload)
{
    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x2; // 00000001

    unsigned char wep = frameControlFragment & mask;

    printf("Protocol : %X", wep);    
    return 0;
}
bool extract_order(unsigned char *payload)
{
    
    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x1; // 00000001

    unsigned char order = frameControlFragment & mask;

    printf("Protocol : %X", order);
    return false;
}

bool is_valid_payload(int size){
    if (size < 24) 
    {
        return false;
    } 
}

void extract_more_frag(unsigned char *payload)
{
    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x20; // 00000001

    unsigned char order = frameControlFragment & mask;

    printf("Protocol : %X", order);
    return false;
}

void extract_more_data(unsigned char *payload)
{
    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x1; // 00000001

    unsigned char order = frameControlFragment & mask;

    printf("Protocol : %X", order);
    return false;
}

void extract_sourceAddress(unsigned char *payload){
    
    unsigned char destinationAddress[6];
    memcpy(destinationAddress, &payload[4], 6);
    printf("Addres : %02X:%02X:%02X:%02X:%02X:%02X\n", destinationAddress[0], destinationAddress[1], destinationAddress[2],destinationAddress[3], destinationAddress[4] ,destinationAddress[5]);
    return 0;
}
void extract_destinationAddress(unsigned char *payload){
    
    unsigned char destinationAddress[6];
    memcpy(destinationAddress, &payload[9], 6);
    printf("Addres : %02X:%02X:%02X:%02X:%02X:%02X\n", destinationAddress[0], destinationAddress[1], destinationAddress[2],destinationAddress[3], destinationAddress[4] ,destinationAddress[5]);
    return 0;// destinationAddress;
}
void extract_SSID(unsigned char *payload){
    
    unsigned char SSID[6];
    memcpy(SSID, &payload[15], 6);
    printf("Addres : %02X:%02X:%02X:%02X:%02X:%02X\n", SSID[0], SSID[1], SSID[2],SSID[3], SSID[4] ,SSID[5]);
    
    return 0;//SSID;
}
void extract_senderSSID(unsigned char *payload){

    unsigned char senderSSID[6];
    memcpy(senderSSID, &payload[16], 6);
    printf("Addres : %02X:%02X:%02X:%02X:%02X:%02X\n", senderSSID[0], senderSSID[1], senderSSID[2],senderSSID[3], senderSSID[4] ,senderSSID[5]);
    
    return 0 ;//senderSSID;
}
