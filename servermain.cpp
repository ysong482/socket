#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <string>
#include <bits/stdc++.h>
#include <chrono>
#include <string>
#include <string.h>
#include <stdio.h>

using namespace std;
#define MAXLINE 2048

//compare two string.if str2 contain str1,return true, else return false.
bool haveCountry(string str1,string str2){
    string S = str2;
	vector<string> resA;
    string result;
    stringstream input(S);
    while (input >> result){
		resA.push_back(result);
    }
    int length = resA.size();
    for(int i=0;i<length;i++){
        if(resA[i]==str1){
            return true;
        }
    }
    return false;
}

int main() {
    cout<<"Main server is up and running"<<endl;
    //CREATE Mainserver TCP, UDP SOCKETS FOR Mainserver,code from beej
    int server_socket, endserverAB_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    endserverAB_socket = socket(AF_INET, SOCK_DGRAM, 0);

    //Mainserver TCP ADDRESS,code from beej
    struct sockaddr_in server_address, client_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(33112);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");


    //bind,code from beej
    bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));


    //listen for tcp from client,code from beej
    listen(server_socket, 5);

    int client_socket;
    pid_t pid;


    //continuously run the server
    while (1) {

        //Mainserver UDP ADDRESS,code from beej
        struct sockaddr_in udp_address;
        udp_address.sin_family = AF_INET;
        udp_address.sin_port = htons(32112);
        udp_address.sin_addr.s_addr = inet_addr("127.0.0.1");

        //SERVER A UDP ADDRESS,code from beej
        struct sockaddr_in serverA_address;
        serverA_address.sin_family = AF_INET;
        serverA_address.sin_port = htons(30112);
        serverA_address.sin_addr.s_addr = inet_addr("127.0.0.1");

        //SERVER B UDP ADDRESS,code from beej
        struct sockaddr_in serverB_address;
        serverB_address.sin_family = AF_INET;
        serverB_address.sin_port = htons(31112);
        serverB_address.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        bind(endserverAB_socket, (struct sockaddr *) &udp_address, sizeof(udp_address));


        //send a request to endserver
        
        char request[1024];
        char countryList[20] ="Request List";
        char countryA[1024];
        char countryB[1024];
        //socklen_t len = sizeof(client_address);,code from beej
        socklen_t addrA_size = sizeof(serverA_address);
        socklen_t addrB_size = sizeof(serverB_address);

        //send request country list to server A,code from beej
        sendto(endserverAB_socket, &countryList, sizeof(countryList), 0, (struct sockaddr *) &serverA_address, sizeof(serverA_address));
        recvfrom(endserverAB_socket, (char *) countryA, sizeof(countryA), 0, (struct sockaddr *) &serverA_address, &addrA_size);
        string ListA = countryA;
        cout<<"The main server has received the country list from server A using UDP over port 32112"<<endl;
       
        // send request country list to server B,code from beej
        sendto(endserverAB_socket, &countryList, sizeof(countryList), 0, (struct sockaddr *) &serverB_address, sizeof(serverB_address));
        recvfrom(endserverAB_socket, (char *) countryB, sizeof(countryB), 0, (struct sockaddr *) &serverB_address, &addrB_size);
        string ListB = countryB;
        cout<<"The main server has received the country list from server B using UDP over port 32112"<<endl;

        cout<<"server A: "<<ListA<<endl;
        cout<<"server B: "<<ListB<<endl;

        char recvBuffer[1024];
        //accept TCP,code from beej
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address,(socklen_t*)&client_address))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        pid = fork();
        if(pid == -1){
            perror("fork error");
        }
        if(pid == 0){
            close(server_socket);
            while(1){
                //receive the inputs from client,code from beej
                recv(client_socket, &request, sizeof(request), 0);
                
                string S = request;
                vector<string> resC;
                string result;
                stringstream input(S);
                while (input >> result) {
                resC.push_back(result);
                }//break command into vector,res[0]is country name,res[1]is userid.
                cout<< "The Main server has received the request on User"<<resC[1]<<" in "<<resC[0]<<"from client using TCP over port 33112"<<endl;

                //request to endserver
                if(haveCountry(resC[0],ListA)== true){
                    cout<<resC[0]<<" shows up in server A/B"<<endl;
                    //send udp to endserverA,code from beej
                    sendto(endserverAB_socket, &request, sizeof(request), 0, (struct sockaddr *) &serverA_address, sizeof(serverA_address));
                    cout<<"The Main Server has sent request from User"<<resC[1]<<" to server A/B using UDP over port 32112"<<endl;
                    //receive answer from endserverA,code from beej
                    recvfrom(endserverAB_socket, (char *) recvBuffer, sizeof(recvBuffer), 0, (struct sockaddr *) &serverA_address, &addrA_size);
                    cout<<"The Main server has received searching result(s) of User "<<resC[1]<< "from server<A/B>"<<endl;
                    //send answer back to client ,code from beej
                    send(client_socket, &recvBuffer, sizeof(recvBuffer),0);
                    cout<<"The Main Server has sent searching result(s) to client using TCP over port 33112"<<endl;
                    memset(recvBuffer,0,1024);
                }else if(haveCountry(resC[0],ListB)== true){
                    //send udp to endserverB,code from beej
                    sendto(endserverAB_socket, &request, sizeof(request), 0, (struct sockaddr *) &serverB_address, sizeof(serverB_address));
                    cout<<"The Main Server has sent request from User"<<resC[1]<<" to server A/B using UDP over port 32112"<<endl;
                    //receive answer from endserverB,code from beej
                    recvfrom(endserverAB_socket, (char *) recvBuffer, sizeof(recvBuffer), 0, (struct sockaddr *) &serverB_address, &addrB_size);
                    cout<<"The Main server has received searching result(s) of User "<<resC[1]<< "from server<A/B>"<<endl;
                    //send answer back to client ,code from beej
                    send(client_socket, &recvBuffer, sizeof(recvBuffer),0);
                    cout<<"The Main Server has sent searching result(s) to client using TCP over port 33112"<<endl;
                    memset(recvBuffer,0,1024);
                }else{
                    //return "country not found" to client;
                    cout<< resC[0]<<"does not show up in server A&B"<<endl;
                    char nonrequest[] = "country not found";
                    //send answer back to client ,code from beej
                    send(client_socket, &nonrequest, sizeof(nonrequest),0);
                    cout<<"The Main Server has sent Country Name: Not found to client1/2 using TCP over port 33112"<<endl;
                }
            }
            close(client_socket);
        }else{
            close(client_socket);
        }
        
    }//end while

    //close the sockets
    close(server_socket);
    close(endserverAB_socket);
    return 0;
}


