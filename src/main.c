/* 
  HERMES MODULE - Artemisa
  Wifi-Sniffer - library
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

#define FLAG_TODS                0x01
#define FLAG_FROMDS              0x02
#define FLAG_MORE_FRAG           0x04
#define FLAG_RETRY               0x08
#define FLAG_POWER_MGMT          0x10
#define FLAG_MORE_DATA           0x20
#define FLAG_WEP                 0x40
#define FLAG_ORDER               0x80
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

#define DEBUG_Size 32 
#define addresesSize 6

static const char *TAG = "Artemisa"; 

#define payloadHeaderSize       24
#define MAX_CHANNELS            13      
#define SCAN_TIME_MS            300     
#define RADAR_THRESHOLD_NEAR    -45     
#define RADAR_THRESHOLD_FAR     -85   
#define networkNameMaxLenght     33 // 32char + null terminator
#define nameLengthbite           37
#define nameStartBite            38

unsigned int duration;
unsigned int destinationAddress;
unsigned int sourceAddress;
unsigned int bssidSource;
unsigned int bssidDestination;

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

bool validate_beacon(unsigned char *subtype);
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
    uint_least8_t flagsBoolean;

}  frameControl;

void extract_addrs1(unsigned char *payload, const char *type);
void extract_addrs2(unsigned char *payload);
void extract_addrs3(unsigned char *payload);
void extract_addrs4(unsigned char *payload);

void type_of_addressing(uint_least8_t booleanFlags, unsigned char *payload);

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
    static uint_least8_t ucParameterToPass;
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
    for(uint_least8_t i = 1; i <= MAX_CHANNELS; i++) 
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
    uint16_t payloadSize = packet->rx_ctrl.sig_len;
    //for(int i = 0 ; i < payloadSize;i++) //TODO Debug (delete 4 production) shits bugged too, payloadSize depeneds from packet lenght (header + body) instead of only header (24 bytes)
    //{                                      //causes task watchdog (operation to slow 4 the cuantity of info, blocking cpu)
    //printf("Byte %d: 0x%02x\n", i ,(unsigned char)payload[i]); 
    //}
    //extract_network_name(payload);                            
    if(is_valid_payload(payloadSize)){          
        payload_header_extractor(payload);
    }
}

void payload_header_extractor(unsigned char *payload){ 
    

    printf(" ===== NEW NETWORK =====\n");//TODO debug
    uint_least8_t flagsBoolean = 0x00 ;  
    
    extract_protocol(payload, &flagsBoolean);
    extract_type(payload, &flagsBoolean);
    extract_subtype(payload, &flagsBoolean);
    
    extract_toDs(payload, &flagsBoolean);
    extract_fromDs(payload, &flagsBoolean);
    type_of_addressing(flagsBoolean, payload);

    extract_retry(payload, &flagsBoolean);
    extract_powerManagement(payload, &flagsBoolean);
    extract_wep(payload, &flagsBoolean);
    extract_order(payload, &flagsBoolean);
   
   
    
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

    unsigned char type = (frameControlFragment & typeMask)>> 2;

    printf("type : %X\n", type);

  
}

void extract_subtype(unsigned char *payload, uint_least8_t *flagsBoolean){

    unsigned char frameControlFragment = payload[0]; //Frame control is from two BYTES (so two fragments)
    unsigned char typeMask = 0xF0; 

    unsigned char subtype = (frameControlFragment & typeMask) >> 4;
    if (validate_beacon(&subtype)){ //TODO debuggin filter -> extract to function and refactorize
        extract_network_name(payload);
    }
    printf("subtype : %X\n", subtype);

}

void extract_toDs(unsigned char *payload, uint_least8_t *flagsBoolean){

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x01; 

    unsigned char toDs = (frameControlFragment & mask) >> normalizeFlagToDs;

    if (toDs){
        *flagsBoolean |= FLAG_TODS;
    }
    

    printf("toDs : %X\n", toDs);

    
}
void extract_fromDs(unsigned char *payload, uint_least8_t *flagsBoolean){

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x02; 

    unsigned char fromDs = (frameControlFragment & mask) >> normalizeFlagFromDs;

    if (fromDs){
        *flagsBoolean |= FLAG_FROMDS;
    }

    printf("fromDs : %X\n", fromDs);

}

void extract_retry(unsigned char *payload, uint_least8_t *flagsBoolean)
{

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x08; 

    unsigned char retry = (frameControlFragment & mask) >> normalizeFlagRetry;
    
    if (retry){
        *flagsBoolean |= FLAG_RETRY;
    }

    printf("exetract_retry : %X\n", retry);
    
}

void extract_powerManagement(unsigned char *payload, uint_least8_t *flagsBoolean)
{

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x10; 

    unsigned char powerManagement = (frameControlFragment & mask) >> normalizeFlagPowerMgmt;

    if (powerManagement){
        *flagsBoolean |= FLAG_POWER_MGMT;
    }

    printf("power Management : %X\n", powerManagement);

}

void extract_wep(unsigned char *payload, uint_least8_t *flagsBoolean)
{

    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x40; 

    unsigned char wep = (frameControlFragment & mask) >> normalizeFlagWep;

    if (wep){
        *flagsBoolean |= FLAG_WEP;
    }

    printf("WEP : %X\n", wep);    
    
}

void extract_order(unsigned char *payload, uint_least8_t *flagsBoolean)
{
    
    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x80; 

    unsigned char order = (frameControlFragment & mask) >> normalizeFlagOrder;

    if (order){
        *flagsBoolean |= FLAG_ORDER;
    }

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

    if (moreFrag){
        *flagsBoolean |= FLAG_MORE_FRAG;
    }

    printf("moreFrag : %X\n", moreFrag);
    
}

void extract_more_data(unsigned char *payload, uint_least8_t *flagsBoolean)
{
    
    unsigned char frameControlFragment = payload[1]; //Frame control is from two BYTES (so two fragments)
    unsigned char mask = 0x20; 

    unsigned char moreData = (frameControlFragment & mask) >> normalizeFlagMoreData;

    if (moreData){
        *flagsBoolean |= FLAG_MORE_DATA;
    }

    printf("moreData : %X\n", moreData);
    
}

void extract_addrs1(unsigned char *payload, const char *type)
{
    
    unsigned char destinationAddress[addresesSize];
    memcpy(destinationAddress, &payload[10], 6);
    printf("%s : %02X:%02X:%02X:%02X:%02X:%02X\n", type, destinationAddress[0], destinationAddress[1], destinationAddress[2],destinationAddress[3], destinationAddress[4] ,destinationAddress[5]);
    
}

void extract_addrs2(unsigned char *payload)
{
    
    unsigned char destinationAddress[addresesSize];
    memcpy(destinationAddress, &payload[4], 6);
    printf("Addres : %02X:%02X:%02X:%02X:%02X:%02X\n", destinationAddress[0], destinationAddress[1], destinationAddress[2],destinationAddress[3], destinationAddress[4] ,destinationAddress[5]);
    
}

void extract_addrs3(unsigned char *payload) 
{
    
    unsigned char BSSID[addresesSize];
    memcpy(BSSID, &payload[16], 6);
    printf("Addres : %02X:%02X:%02X:%02X:%02X:%02X\n", BSSID[0], BSSID[1], BSSID[2], BSSID[3], BSSID[4] , BSSID[5]);
    
    
}

void extract_addrs4(unsigned char *payload) //TODO this is a provitional name. IEE 808.11 has 4 addresess, even tho fourth is barely seen
                                            //if seen should be proccessed so function must exist
                                            //BUT idk what it really repressents so I cant assign it a propper name yet, name shall be corrected
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
        } // add filter to only pass beacon request (1000 bin or 8 hex)

        else
        {
            return;
        }

    }
    printf("\n");
    

}

void type_of_addressing(uint_least8_t booleanFlags, unsigned char *payload) //will take a copy of boolean flags and operate over it 
{
    uint_least8_t directionAndAddressing = booleanFlags & extractToAndFromMask; 
                                        //Following is a switch case 
                                        //that decides what type of comm is to succesfull extract all the information
                                        //direction addressing is decided following the table found in : unnamedFolder/addresing.svg
                                        
    switch(directionAndAddressing){

        case dtlFrames:

            extract_addrs1(payload, "Dest MAC");
            extract_addrs2(payload);
            extract_addrs3(payload);

            break;

        case dsToClient:

            extract_addrs1(payload, "Dest MAC");
            extract_addrs2(payload);
            extract_addrs3(payload);

            break;

        case clientToDS:

            extract_addrs1(payload, "BBSID");
            extract_addrs2(payload);
            extract_addrs3(payload);

            break;

        case bridge: 

            extract_addrs1(payload, "Dest Radio");
            extract_addrs2(payload);
            extract_addrs3(payload);
            extract_addrs4(payload);


            break;

        default:

            printf("unknown address type");
            break;
        

    }
}

bool validate_beacon(unsigned char *subtype)
{   

    if(*subtype == 8){
        return true;
    }
    return false;
}