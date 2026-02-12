#include <stdio.h>
#include <stdbool.h>
#include <string.h>
//#include <sx128x.h>

#define MESSAGE_SIZE 280

typedef int id;


id receiver_id;
id transmitter_id;
char message[MESSAGE_SIZE];

typedef struct{ // change struct name to something more explicit

    id transmitter_id;
    id receiver_id;
    char message[MESSAGE_SIZE];
    
    //look for implemetntaion of enums in structures 
}  device ;

device debugReceiver;
device debugSender;

int sendMessage(int receiver_id, int emmisorID,char *message, device *debugSender, device *debugReceiver);
int getReceiver();
int validateConnection(int receiver_id, int emmisorID,char *message, device *debugSender, device *debugReceiver);
int setMessage(char *message,device *debugReceiver );
char* getMessage();

enum metadata{
    
    DATE,
    FLYTIME

};

int main(){


    transmitter_id = 4;
    receiver_id = getReceiver(); 
    printf("Provided ID is : %d \n", receiver_id);

    char* message = getMessage();
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

int getReceiver(){

    int receiver_id;
    printf("insert an ID\n");
    scanf("%d", &receiver_id);
    printf("Receiver %d\n", receiver_id);

    return receiver_id;

}

char* getMessage(){
    
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