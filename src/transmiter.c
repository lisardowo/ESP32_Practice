#include <stdio.h>
//#include <sx128x.h>

#define MESSAGE_SIZE 65

int ID;
int recieverID;
int transmitterID;
char message[MESSAGE_SIZE];

int sendMessage(int recieverID, int emmisorID,char *message,struct sender *debugSender,struct receiver *debugReciever);
int getReciever();
int validateComms();

enum metadata{
    
    DATE,
    DEVICE,
    FLYTIME

};


struct sender{

    int ID;
    int recieverID;
    char message[MESSAGE_SIZE];
    //look for implemetntaion of enums in structures


} ;

struct receiver{
    int ID;
    int transmitterID;
    char message[MESSAGE_SIZE];

}; // will this be redundant?

struct message{

    

};




int main(){

    int recieverID;
    transmitterID = 4;
    recieverID = getReciever(); 
    printf("Provided ID is : %d \n", recieverID);

    //message
    printf("provide the message");
    scanf("%64s", message);
    printf("message is : %s", message);//TODO set buffer size to prevent bufffer overflows ==> Raise Error for long messages

    
    //construction of structs
    struct receiver debugReceiver;
    struct sender debugSender;

    // initialization of transmitter
    
    
    debugSender.ID = ID;
    debugSender.recieverID = recieverID;

    // initialization of reciever
    
    debugReceiver.ID = recieverID;
    debugReceiver.transmitterID = transmitterID;


    if(sendMessage(recieverID, transmitterID, message, &debugSender, &debugReceiver) == 0){
        printf("OK");
    }

}

int getReciever(){

    int recieverID;
    printf("insert an ID");
    scanf("%d", &recieverID);
    printf("%d\n", recieverID);

    return recieverID;




}

//char getMessage(){} //TODO get message via function

//errors 200 -> NOT possible Communications (user associated)
int sendMessage(int recieverID, int emmisorID,char *message,struct sender *debugSender,struct receiver *debugReciever){

    if (debugSender->ID || debugReciever->transmitterID != emmisorID){

        return 201; // Error 201 -> Not a valid emmisor ID

    }
    if(debugSender->recieverID || debugReciever->ID != recieverID){
        return 202;// Error 202 -> Not a valid reciever ID
    }



    return 0;


}

int validateComms(){

} //TODO refactorize the validation logic in send To validate comms to keep a cleaner code