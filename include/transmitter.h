#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <stdio.h>
#include <string.h>

#define MESSAGE_SIZE 280

typedef int id;

typedef struct{ // change struct name to something more explicit

    id transmitter_id;
    id receiver_id;
    char message[MESSAGE_SIZE];
    
    //look for implemetntaion of enums in structures 
}  device ;


int sendMessage(int receiver_id, int emmisorID,char *message, device *debugSender, device *debugReceiver);
int getReceiver(int receiver_id);
int validateConnection(int receiver_id, int emmisorID,char *message, device *debugSender, device *debugReceiver);
int setMessage(char *message,device *debugReceiver );
char* getMessage(char *message);




enum metadata{
    
    DATE,
    FLYTIME

};

#endif
