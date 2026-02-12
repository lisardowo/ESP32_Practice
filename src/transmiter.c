
#include <stdio.h>
#include "transmitter.h"

int getReceiver(int receiver_id){


    printf("insert an ID\n");
    scanf("%d", &receiver_id);
    printf("Receiver %d\n", receiver_id);

    return receiver_id;

}

char* getMessage(char *message){
    
    printf("provide the message : \n");
    scanf("%*c");
    fgets(message, MESSAGE_SIZE, stdin);
    // Buffer Size will be caped at 280

    return message;
} 

//errors 200 -> NOT possible Communications (user associated)
//errors 300 -> Message Couldnt be send

int sendMessage(int receiver_id, int emmisorID,char *message, device *debugSender, device *debugReceiver){

    printf("sendingMessage\n");


    if(validateConnection(receiver_id, emmisorID, message, debugSender, debugReceiver) == 0){
        printf("Sender Message = %s \n", debugSender->message);
        printf("Receiver Message = %s \n", debugReceiver->message);
        
        if(setMessage(message, debugReceiver) != 0){

            return 301;
        }
        printf("Received Message = %s \n", debugReceiver->message);
        return 0;
    }
    else{
        return validateConnection(receiver_id, emmisorID, message, debugSender, debugReceiver) ;// Will return the respective error code BUT also will duplicate Sender/receiver info #TODO check to changethat
    }

 
 


}

int setMessage(char *message, device *debugReceiver ){
    
    strncpy(debugReceiver->message, message, MESSAGE_SIZE);
    return 0;
}

int validateConnection(int receiver_id, int emmisorID,char *message, device *debugSender, device *debugReceiver){

    if (debugSender->transmitter_id != emmisorID || debugReceiver->transmitter_id != emmisorID)
    {

        printf("Emisor = %d \n", emmisorID);
        printf("Sender = %d  \n", debugSender->transmitter_id);
        printf("Receiver = %d  \n", debugReceiver->transmitter_id);

        return 201; // Error 201 -> Not a valid emmisor ID

    }
    if(debugSender->receiver_id != receiver_id || debugReceiver->receiver_id != receiver_id)
    {
        
        printf("receiver = %d \n", receiver_id);
        printf("Sender = %d  \n", debugSender->receiver_id);
        printf("Receiver = %d  \n", debugReceiver->receiver_id);

        return 202; // Error 202 -> Not a valid receiver ID
    }
    
    return 0;

} 