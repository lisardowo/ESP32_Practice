
//gcc transmiterDemo.c ../src/transmiter.c -I../include -o ../testplayground/transmiter && ../testplayground/transmiter

#include "transmitter.h"
#include <stdio.h>
#include <string.h>

device debugReceiver;
device debugSender;

id transmitter_id ;
id receiver_id;

char message[MESSAGE_SIZE];

int main(){


    transmitter_id = 4;
    receiver_id = getReceiver(receiver_id); 
    printf("Provided ID is : %d \n", receiver_id);

    getMessage(message);
    printf("message is : %s\n", message);
  
    // initialization of transmitter
    
    debugSender.transmitter_id = transmitter_id;
    debugSender.receiver_id = receiver_id;
    strncpy(debugSender.message, message, MESSAGE_SIZE);

    // initialization of receiver
    
    debugReceiver.transmitter_id = transmitter_id;
    debugReceiver.receiver_id = receiver_id;

    
    int succes = sendMessage(receiver_id, transmitter_id, message, &debugSender, &debugReceiver);

    if(succes == 0){
        printf("OK\n");
    }
    else{
        printf("%d\n", succes);
    }

    return 0;
}