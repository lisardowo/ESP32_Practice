/* 
  HERMES MODULE - Artemisa
  Wifi-Sniffer - library
 */

static const char *TAG = "Artemisa"; 

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

#define flagToDs                 0x01
#define flagFromDs               0x02
#define flagMoreFrag             0x04
#define flagRetry                0x08
#define flagPowerManagement      0x10
#define flagMoreData             0x20
#define flagWep                  0x40
#define flagOrder                0x80

#define extractToAndFromMask     0x03

#define dtlFrames               0x00
#define dsToClient              0x01
#define clientToDS              0x02
#define bridge                  0x03

#define normalizeFlagToDs        0
#define normalizeFlagFromDs      1
#define normalizeFlagMoreFrag    2
#define normalizeFlagRetry       3
#define normalizeFlagPowerMgmt   4
#define normalizeFlagMoreData    5
#define normalizeFlagWep         6
#define normalizeFlagOrder       7

#define managementFrame         0
#define controlFrame            1
#define dataFrame               2

#define DEBUG_Size 32 
#define addresesSize 6

#define payloadHeaderSize        24
#define maxChannels              13      
#define scanTimeMs               300     
#define radarThresholdNear      -45     
#define radarThresholdFar       -85   
#define networkNameMaxLenght     33 // 32char + null terminator
#define nameLengthbite           37
#define nameStartBite            38
#define beacon                   8

unsigned int duration;
unsigned int destinationAddress;
unsigned int sourceAddress;
unsigned int bssidSource;
unsigned int bssidDestination;

typedef struct __attribute__((packed)) {

    unsigned char version; //Protocol Version
    unsigned char Type; //funion of frame (management control or data)
    unsigned char Subtype; // indicates frame purpose (?) 0000 for association , 1000 for beacon
    uint_least8_t flagsBoolean;

}  frameControl;


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
void extract_network_name(unsigned char *payload);
void extract_addrs1(unsigned char *payload, const char *type);
void extract_addrs2(unsigned char *payload, const char *type);
void extract_addrs3(unsigned char *payload, const char *type);
void extract_addrs4(unsigned char *payload, const char *type);

bool validate_network(unsigned char *subtype);
bool is_valid_payload(int size);

void type_of_addressing(uint_least8_t booleanFlags, unsigned char *payload);
void frame_type_interpreter(unsigned char *frameType ,unsigned char *payload,uint_least8_t *flagsBoolean);

void memory_initializer();
int set_promiscuous();
void channel_swapping();
void sniffed_packets_handler();

void app_main(void)
{
    
    ESP_LOGI(TAG, "ESP32 | Wifi Sniffer");

    memory_initializer();

    esp_err_t set_prom = set_promiscuous();

    if(set_prom == ESP_OK )
    {
        printf("Promiscuous : OK\n");
    }
    else if (set_prom == ESP_ERR_WIFI_NOT_INIT || set_prom == ESP_ERR_INVALID_ARG )
    {
        printf("Error %d", set_prom);
    }
    static uint_least8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;
    
    xTaskCreate( channel_swapping , "SWAPPING", 4097, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle);
    


   while(1) 
   {

        ESP_LOGI(TAG, "OS. Free heap: %d bytes", esp_get_free_heap_size());
        vTaskDelay(pdMS_TO_TICKS(10000)); 
    
    }
    
}

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

int set_promiscuous(){

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



    esp_err_t callback = esp_wifi_set_promiscuous_rx_cb(&sniffed_packets_handler);
     if (callback != ESP_OK) {
        printf("Error in callback of Sniffed Handler func: %d\n", err);
        return err;
    }

    printf("promiscuous setup completed \n");
    return ESP_OK;
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

void payload_header_extractor(unsigned char *payload){ 
    

    printf(" ===== NEW NETWORK =====\n");//TODO debug
    uint_least8_t flagsBoolean = 0x00 ;  
    
    extract_type(payload, &flagsBoolean);

    
    

    /*extract_subtype(payload, &flagsBoolean);
    extract_protocol(payload, &flagsBoolean);
    extract_toDs(payload, &flagsBoolean);
    extract_fromDs(payload, &flagsBoolean);
   
    TODO debuggin sum stuffff
    type_of_addressing(flagsBoolean, payload);
    extract_retry(payload, &flagsBoolean);
    extract_powerManagement(payload, &flagsBoolean);
    extract_wep(payload, &flagsBoolean);
    extract_order(payload, &flagsBoolean);*/
    
    printf("\n===== END OF NETWORK ======\n");//TODO -- debug

    flagsBoolean = 0x00; 


}


void extract_protocol(unsigned char *payload, uint_least8_t *flagsBoolean){

    unsigned char frameControl = payload[0]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x03; 

    unsigned char protocol = frameControl & mask;

    printf("Protocol : %X\n", protocol);

    //TODO -> DEBUG print, delete 4 production

}

void extract_type(unsigned char *payload, uint_least8_t *flagsBoolean){

    
    unsigned char frameControlFragment = payload[0]; //Frame control is from two BYTES (so two fragments)
    unsigned char typeMask = 0x0C; 

    unsigned char frameType = (frameControlFragment & typeMask)>> 2;



    printf("type : %X\n", frameType);

    frame_type_interpreter(&frameType ,payload, flagsBoolean);//TODO if possible id like to use the interpreter OUTSIDE this function
                                                             // , but also wanna avoid returns an stuff due to memory reasons
  
}

void extract_subtype(unsigned char *payload, uint_least8_t *flagsBoolean)
{

    unsigned char frameControlFragment = payload[0]; //Frame control is from two BYTES (so two fragments)
    unsigned char typeMask = 0xF0; 

    unsigned char subtype = (frameControlFragment & typeMask) >> 4;
    
    printf("subtype : %X\n", subtype);

    if (validate_network(&subtype))
    { 
        extract_network_name(payload);
    }
    

}

void extract_toDs(unsigned char *payload, uint_least8_t *flagsBoolean)
{

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x01; 

    unsigned char toDs = (frameControlFragment & mask) >> normalizeFlagToDs;

    
    *flagsBoolean |= flagToDs;
    
    

    printf("toDs : %X\n", toDs);

    
}
void extract_fromDs(unsigned char *payload, uint_least8_t *flagsBoolean)
{

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x02; 

    unsigned char fromDs = (frameControlFragment & mask) >> normalizeFlagFromDs;

    *flagsBoolean |= flagFromDs;
    
    printf("fromDs : %X\n", fromDs);

}

void extract_retry(unsigned char *payload, uint_least8_t *flagsBoolean)
{

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x08; 

    unsigned char retry = (frameControlFragment & mask) >> normalizeFlagRetry;
    
    
    *flagsBoolean |= flagRetry;
    

    printf("extract_retry : %X\n", retry);
    
}

void extract_powerManagement(unsigned char *payload, uint_least8_t *flagsBoolean)
{

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x10; 

    unsigned char powerManagement = (frameControlFragment & mask) >> normalizeFlagPowerMgmt;

    *flagsBoolean |= flagPowerManagement;
    
    printf("power Management : %X\n", powerManagement);

}

void extract_wep(unsigned char *payload, uint_least8_t *flagsBoolean)
{

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x40; 

    unsigned char wep = (frameControlFragment & mask) >> normalizeFlagWep;

    *flagsBoolean |= flagWep;
    
    printf("WEP : %X\n", wep);    
    
}

void extract_order(unsigned char *payload, uint_least8_t *flagsBoolean)
{
    
    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x80; 

    unsigned char order = (frameControlFragment & mask) >> normalizeFlagOrder;

    *flagsBoolean |= flagOrder;
    
    printf("Order : %X\n", order);
    
}

bool is_valid_payload(int size)
{
 
    return size >= payloadHeaderSize;
  
}

void extract_more_frag(unsigned char *payload, uint_least8_t *flagsBoolean)
{
    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x04; 

    unsigned char moreFrag = (frameControlFragment & mask) >> normalizeFlagMoreFrag;

    
    *flagsBoolean |= flagMoreFrag;
    

    printf("moreFrag : %X\n", moreFrag);
    
}

void extract_more_data(unsigned char *payload, uint_least8_t *flagsBoolean)
{
    
    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x20; 

    unsigned char moreData = (frameControlFragment & mask) >> normalizeFlagMoreData;

    
    *flagsBoolean |= flagMoreData;

    printf("moreData : %X\n", moreData);
    
}

void extract_addrs1(unsigned char *payload, const char *type)
{
    
    unsigned char destinationAddress[addresesSize];
    memcpy(destinationAddress, &payload[10], 6);
    printf("%s : %02X:%02X:%02X:%02X:%02X:%02X\n", type, destinationAddress[0], destinationAddress[1], destinationAddress[2],destinationAddress[3], destinationAddress[4] ,destinationAddress[5]);
    
}

void extract_addrs2(unsigned char *payload, const char *type)
{
    
    unsigned char destinationAddress[addresesSize];
    memcpy(destinationAddress, &payload[4], 6);
    printf("Addres : %02X:%02X:%02X:%02X:%02X:%02X\n", destinationAddress[0], destinationAddress[1], destinationAddress[2],destinationAddress[3], destinationAddress[4] ,destinationAddress[5]);
    
}

void extract_addrs3(unsigned char *payload, const char *type) 
{
    
    unsigned char BSSID[addresesSize];
    memcpy(BSSID, &payload[16], 6);
    printf("Addres : %02X:%02X:%02X:%02X:%02X:%02X\n", BSSID[0], BSSID[1], BSSID[2], BSSID[3], BSSID[4] , BSSID[5]);
    
}

void extract_addrs4(unsigned char *payload, const char *type)
{

    unsigned char address4[6];
    memcpy(address4, &payload[28], 6);
    printf("Addres : %02X:%02X:%02X:%02X:%02X:%02X\n", address4[0], address4[1], address4[2],address4[3], address4[4] ,address4[5]);
    
}

void extract_network_name(unsigned char *payload)
{


    uint16_t nameLenght = payload[nameLengthbite];
    
    for(uint8_t i = 0 ; i < nameLenght; i++)
    {
        if(i < networkNameMaxLenght) // TODO trying to avoid buffer overflows -> Prob need some work
        {
            printf("%c", payload[nameStartBite + i]);
        } 
        return;
    }
    printf("\n");
    

}

void type_of_addressing(uint_least8_t booleanFlags, unsigned char *payload) 
{


    uint_least8_t directionAndAddressing = booleanFlags & extractToAndFromMask; 
        
    //Following is a switch case 
    //that decides what type of comm is to succesfull extract all the information
    //direction addressing is decided following the table found in : unnamedFolder/addresing.svg
                                        
    switch(directionAndAddressing)
    {

        case dtlFrames:

            printf("dtlFrames\n");
            extract_addrs1(payload, "Dest MAC");
            extract_addrs2(payload, "src Mac");
            extract_addrs3(payload, "BSSID");

            break;

        case dsToClient:

            printf("dsToClient\n");
            extract_addrs1(payload, "Dest MAC");
            extract_addrs2(payload,"BSSID");
            extract_addrs3(payload,"Src Mac");

            break;

        case clientToDS:

            printf("clientToDs\n");
            extract_addrs1(payload, "BBSID");
            extract_addrs2(payload, "Src Mac");
            extract_addrs3(payload, "Dest Mac");

            break;

        case bridge: 

            printf("bridge\n");
            extract_addrs1(payload, "Dest Radio");
            extract_addrs2(payload, "Src Radio");
            extract_addrs3(payload, "Dest Mac");
            extract_addrs4(payload, "Src Mac");


            break;

        default:

            printf("unknown address type");
            break;
        

    }
}

void frame_type_interpreter(uint_least8_t *frameType, unsigned char *payload, uint_least8_t *flagsBoolean)
{
    
    switch(*frameType)
    {  
        case managementFrame:
            printf("%"PRIuLEAST8, *frameType);
            break;
        case controlFrame:
            printf("%"PRIuLEAST8, *frameType);
            break;
        case dataFrame:
            printf("%"PRIuLEAST8, *frameType);
            break;
        default:
            printf("not valid data Type");
            break;
        
    }
}


bool validate_network(unsigned char *subtype)
{   

    if(*subtype == beacon)
    {
        return true;
    }
    return false;

}

