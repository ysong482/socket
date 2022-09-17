#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iomanip>
#include <stdio.h>

using namespace std;
#define BUFF_LEN 1024

int main(int argc, char *argv[]){
    //boot up message 
    cout<<"The client is up and running"<<endl;

    //create tcp client socket,code from beej
    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    //specify IP address and dynamic port for the client socket,code from beej
    struct sockaddr_in client_address;
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    socklen_t len=sizeof(client_address);
    getsockname(client_socket,(struct sockaddr *) &client_address,&len);
    
    //specify IP address and dynamic port for the server socket,code from beej
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(33112);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    //connect to mainserver,code from beej
    socklen_t len1=sizeof(server_address);
    int connection_status = connect(client_socket,(struct sockaddr *) &server_address, len1);
    //check for error within connection_status
    if(connection_status==-1){
        perror("connect");
        exit(0);
    }
    while(1){
        //input country name and user id
        char bufSend[BUFF_LEN] ={0};
        char bufRecv[BUFF_LEN] ={0};
        string CName;
        string UID;
        string request;
        cout<<"Enter country name: ";
        cin >> CName;
        cout<<"Enter user ID: ";
        cin >> UID;

        //send inputs to AWS
        request = CName+" "+UID;
        strcpy(bufSend,request.c_str());//change string to char[]
        //code from beej
        send(client_socket, bufSend, sizeof(bufSend),0);
        cout<<"client has sent "<<UID<<" and "<<CName<<" to main server using TCP"<<endl;
        recv(client_socket, &bufRecv, sizeof(bufRecv), 0);
        string msg1=bufRecv;
        cout<<msg1<<endl;
        memset(bufSend,0,BUFF_LEN);
        memset(bufRecv,0,BUFF_LEN);
    }

    close(client_socket);
    return 0;
}
