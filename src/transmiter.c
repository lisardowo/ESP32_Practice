#include <stdio.h>
#include <stdbool.h>
#include <string.h>
//#include <sx128x.h>

#define MESSAGE_SIZE 108

int ID;
int receiverID;
int transmitterID;
char message[MESSAGE_SIZE];

struct device{ // change struct name to something more explicit

    int emissorID;
    int receiverID;
    char message[MESSAGE_SIZE];
    //look for implemetntaion of enums in structures


} ;



struct device debugReceiver;
struct device debugSender;

int sendMessage(int receiverID, int emmisorID,char *message,struct device *debugSender,struct device *debugReceiver);
int getreceiver();
int validateConnection(int receiverID, int emmisorID,char *message,struct device *debugSender,struct device *debugReceiver);
int setMessage(char *message,struct device *debugReceiver );
char* getMessage();

enum metadata{
    
    DATE,
    DEVICE,
    FLYTIME

};




int main(){

    char debug[100];
    int n = 10;

    int receiverID;
    transmitterID = 4;
    receiverID = getreceiver(); 
    printf("Provided ID is : %d \n", receiverID);

    char* message = getMessage();
    printf("message is : %s\n", message);
  
    // initialization of transmitter
    
    debugSender.emissorID = transmitterID;
    debugSender.receiverID = receiverID;
    strncpy(debugSender.message, message, MESSAGE_SIZE);

    // initialization of receiver
    
    debugReceiver.emissorID = transmitterID;
    debugReceiver.receiverID = receiverID;

    
    int succes = sendMessage(receiverID, transmitterID, message, &debugSender, &debugReceiver);

    if(succes == 0){
        printf("OK\n");
    }
    else{
        printf("%d\n", succes);
    }

    return 0;
}

int getreceiver(){

    int receiverID;
    printf("insert an ID\n");
    scanf("%d", &receiverID);
    printf("Receiver %d\n", receiverID);

    return receiverID;




}

char* getMessage(){
    
    printf("provide the message : \n");
    scanf("%*c");
    fgets(message, MESSAGE_SIZE, stdin);
    //TODO set buffer size to prevent bufffer overflows ==> Raise Error for long messages

    return message;
} //TODO get message via function

//errors 200 -> NOT possible Communications (user associated)
//errors 300 -> Message Couldnt be send

int sendMessage(int receiverID, int emmisorID,char *message,struct device *debugSender,struct device *debugReceiver){

    printf("sendingMessage\n");


    if(validateConnection(receiverID, emmisorID, message, debugSender, debugReceiver) == 0){
        printf("Sender Message = %s \n", debugSender->message);
        printf("Receiver Message = %s \n", debugReceiver->message);
        
        if(setMessage(message, debugReceiver) != 0){

            return 301;
        }
        printf("Received Message = %s \n", debugReceiver->message);
        return 0;
    }
    else{
        return validateConnection(receiverID, emmisorID, message, debugSender, debugReceiver) ;// Will return the respective error code BUT also will duplicate Sender/receiver info #TODO check to changethat
    }

 
 


}

int setMessage(char *message,struct device *debugReceiver ){
    
    strncpy(debugReceiver->message, message, MESSAGE_SIZE);
    return 0;
}

int validateConnection(int receiverID, int emmisorID,char *message,struct device *debugSender,struct device *debugReceiver){

    if (debugSender->emissorID != emmisorID || debugReceiver->emissorID != emmisorID)
    {

        printf("Emisor = %d \n", emmisorID);
        printf("Sender = %d  \n", debugSender->emissorID);
        printf("Receiver = %d  \n", debugReceiver->emissorID);

        return 201; // Error 201 -> Not a valid emmisor ID

    }
    if(debugSender->receiverID != receiverID || debugReceiver->receiverID != receiverID)
    {
        
        printf("receiver = %d \n", receiverID);
        printf("Sender = %d  \n", debugSender->receiverID);
        printf("Receiver = %d  \n", debugReceiver->receiverID);

        return 202;// Error 202 -> Not a valid receiver ID
    }
    
    return 0;
} //TODO refactorize the validation logic in send To validate comms to keep a cleaner code