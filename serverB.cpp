#include <iostream>
#include <stdlib.h>
#include <cctype>
#include <cstdlib>
#include <string>
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <fstream>
#include <vector>
#include <unordered_map>

using namespace std;

#define Max 100//ervery country at most have 100 users, in that the matrix is at most 100*100;     

//distinguish country name with user id.country name is string and user id is digit.
bool isDegital(string str) {
	int strlength = str.size();
	for (int i = 0;i < strlength;i++) {
		if (str.at(i) == '-' && str.size() > 1)  //pssible to find UID below zero
			continue;
		if (str.at(i) > '9' || str.at(i) < '0')	//define the number
			return false;
	}
	return true;
}

//class of create ajmatrix.
class Vertex
{
public:
	Vertex(char lAb) { Label = lAb; }
private:
	char Label;
};
class Graph
{
public:
	Graph();//creat 
	~Graph();//renew
	void addVertex(char lAb);//increase one vertex
	void addEdge(int start, int end);//set edge
	void printMatrix();//print matrix,for test
	string searchMatrix(char vertex);//search matrix
private:
	Vertex* vertexList[Max];   //array to store vertex
	int numVerts;//number of vertexs
	int adjcentMat[Max][Max];//matrix 100*100
};
Graph::Graph()//create matrix
{

	numVerts = 0;
	for (int i = 0;i < Max;i++)
		for (int j = 0;j < Max;j++)
			adjcentMat[i][j] = 0;

}
void Graph::addVertex(char lAb)//add one vertex in the graph.matrix will increase by one row and one colum.
{

	vertexList[numVerts++] = new Vertex(lAb);

}
void Graph::addEdge(int start, int end)//add one edge in the matrix
{
	adjcentMat[start][end] = adjcentMat[end][start] = 1;

}
void Graph::printMatrix()//this function is used for test
{

	for (int i = 0;i < numVerts;i++)
	{
		for (int j = 0;j < numVerts;j++)
		{
			cout << adjcentMat[i][j]<<" ";
		}
		cout << endl;
	}
}
string Graph::searchMatrix(char vertex) {
	string answer="";
    int neiborcount[100] = {0};//this vector record the max number of common neighbor.
    int max = 0;
    int n = -1;//if n == 0(means n is the first element in the matrix)we should avoid that.
    int vertexInt = (char)vertex;
	for (int i = 0;i < numVerts;i++)
	{
        for (int j=0;j< numVerts;j++){
		if (adjcentMat[i][vertexInt] ==1&& adjcentMat[i][j] == 1 && j!=vertexInt &&i!=vertex && adjcentMat[j][vertexInt]==0) {
            neiborcount[j]++;
			}
        }
	}
	//find the most common neighbor
    for(int i=0;i<100;i++){
        if(max<neiborcount[i]){
            max = neiborcount[i];
            n=i;
        }
    }

    //return max and indexinmat.the most common neighbor happend in matrix and it's position in matrix.

	//change int to string to help return.this code is from sample
	ostringstream outstr1;
	outstr1 << max;
	string s1 = outstr1.str();
	answer = answer + " " + s1;
    ostringstream outstr2;
    outstr2 <<n;
    s1 = outstr2.str();
    answer = answer +" "+s1;

	return answer;
}
Graph::~Graph()
{

	for (int i = 0;i < numVerts;i++)
	{
		delete vertexList[i];
	}
}


// this function is used to send udp answer to mainserver.
void sendAnswer(Graph country,unordered_map<string, int> user_backend_mapping,string UID,int socket,sockaddr_in udpaddr,string Cname){
    char sendBuffer[1024];
	string mainserver="";
    string RUID;
	//if we cannot find the user in this county it will not appear in the backend mapping of user; ,code from beej
	unordered_map<string, int>::iterator it1;
	if(user_backend_mapping.count(UID) == 0){
		mainserver = "user not found";
		cout<<"User "<<UID<<"does not show up in"<<Cname<<endl;
		strncpy(sendBuffer,mainserver.c_str(),mainserver.length()+1);
    	sendto(socket, (const char *)sendBuffer, strlen(sendBuffer),0,(const struct sockaddr*)&udpaddr, sizeof(udpaddr));
		cout<< "The server B has sent user not found to Main Server"<< endl;
		memset(sendBuffer,0,1024);
		return;//finish the function
	}
	it1 = user_backend_mapping.find(UID);
	cout<< "The server is searching possible friends for User"<<UID<<endl;
    string p= country.searchMatrix(it1->second);//search the matrix for number and index string
    string answer ="";//store the recommended user id
    vector<string> resValue;
    string value;
    stringstream input(p);
    while (input >> value){
        resValue.push_back(value);
	}
	//break the index string into vector. resValue[0]is the number of common neighbors.

	//use value to find key(userid).the index of row and colum corrspond to the value of the user backend map.resValue[1]
    unordered_map<string, int>::iterator iter;
    iter = user_backend_mapping.begin();
    while (iter != user_backend_mapping.end()) {
        int f2;
        istringstream instr(resValue[1]);
        instr >> f2;
        if(iter->second == f2){
        RUID = iter->first;//get recommend id
    	}   
		iter++;
    }

	cout<<"Here are the results"<<RUID<<endl;
    if(resValue[0]== "0"){//in this situation have no common neighbor
        mainserver ="is already connected to all other users, no new recommendation";
    }else{//edit the answer we get
        mainserver ="recommend new firend "+RUID+" to user "+UID+" (common neighbors = "+resValue[0]+" )\n";
    }
	//copy the string to send buffer.
    strncpy(sendBuffer,mainserver.c_str(),mainserver.length()+1);
    sendto(socket, (const char *)sendBuffer, strlen(sendBuffer),0,(const struct sockaddr*)&udpaddr, sizeof(udpaddr)); //code from beej
	cout<<"The server B has sent the result(s) to Main server"<<endl;
	memset(sendBuffer,0,1024);//clear the buffer
	return;
}//send answer to main server

int main(){
    unordered_map<string, int> country_backend_mapping;//get the list of countries
	unordered_map<string, int> user_backend_mapping;//user matrix buffer

	ifstream infile;
	string S;
	infile.open("./data2.txt");
	string str2;
	int count = 0;//cout the number of element in each country.
	int index = 0;

    //creat 10 countries matrixes;(at most 10 countries in one data)
	Graph country1;
	unordered_map<string, int> user_backend_mapping1;
	Graph country2;
	unordered_map<string, int> user_backend_mapping2;
	Graph country3;
	unordered_map<string, int> user_backend_mapping3;
	Graph country4;
	unordered_map<string, int> user_backend_mapping4;
	Graph country5;
	unordered_map<string, int> user_backend_mapping5;
	Graph country6;
	unordered_map<string, int> user_backend_mapping6;
	Graph country7;
	unordered_map<string, int> user_backend_mapping7;
	Graph country8;
	unordered_map<string, int> user_backend_mapping8;
	Graph country9;
	unordered_map<string, int> user_backend_mapping9;
	Graph country10;
	unordered_map<string, int> user_backend_mapping10;
	while (getline(infile, str2))//read the data line by line
	{
		//break each line in to vector.
		S = str2;
		vector<string> res;
		string result;
		stringstream input(S);
		while (input >> result)
			res.push_back(result);
		int lengthOfres = res.size();
		for (int i = 0;i < lengthOfres;i++) {
			if (index == 0) {//if the string we detected is string means it is a country name and following number is its user id.
				if (isDegital(res[i]) == false) {
					user_backend_mapping.clear();
					country_backend_mapping[res[i]] = index;
					index++;
					count = 0;
					continue;
				}
				else {//store the user id in map and add it into the matrix
					unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
					if (got == user_backend_mapping.end()) {
						user_backend_mapping[res[i]] = count;
						count++;
						char Vertex = count;
						country1.addVertex(Vertex);
					}

				}//creat a new map
				unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
				if (res[0] != res[i]) {
					unordered_map<string, int>::const_iterator got0 = user_backend_mapping.find(res[0]);
					unordered_map<string, int>::const_iterator goti = user_backend_mapping.find(res[i]);
					country1.addEdge(got0->second, goti->second);
				}//add one vertex into the graph

			}
			else if (index == 1) {
				if (isDegital(res[i]) == false) {
					user_backend_mapping1.insert(user_backend_mapping.begin(), user_backend_mapping.end());
					user_backend_mapping.clear();
					country_backend_mapping[res[i]] = index;

					index++;
					count = 0;
					continue;
				}
				else {
					unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
					if (got == user_backend_mapping.end()) {
						user_backend_mapping[res[i]] = count;
						count++;
						char Vertex = count;
						country1.addVertex(Vertex);
					}

				}//creat a new map
				unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
				if (res[0] != res[i]) {
					unordered_map<string, int>::const_iterator got0 = user_backend_mapping.find(res[0]);
					unordered_map<string, int>::const_iterator goti = user_backend_mapping.find(res[i]);
					country1.addEdge(got0->second, goti->second);
				}//add one vertex into the graph

			}
			else if (index == 2)
			{
				if (isDegital(res[i]) == false) {
					user_backend_mapping2.insert(user_backend_mapping.begin(), user_backend_mapping.end());
					user_backend_mapping.clear();
					country_backend_mapping[res[i]] = index;

					index++;
					count = 0;
					continue;
				}
				else {
					unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
					if (got == user_backend_mapping.end()) {
						user_backend_mapping[res[i]] = count;
						count++;
						char Vertex = count;
						country2.addVertex(Vertex);
					}

				}//creat a new map
				unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
				if (res[0] != res[i]) {
					unordered_map<string, int>::const_iterator got0 = user_backend_mapping.find(res[0]);
					unordered_map<string, int>::const_iterator goti = user_backend_mapping.find(res[i]);
					country2.addEdge(got0->second, goti->second);
				}//add one vertex into the graph

			}
			else if (index == 3)
			{
				if (isDegital(res[i]) == false) {
					user_backend_mapping3.insert(user_backend_mapping.begin(), user_backend_mapping.end());
					user_backend_mapping.clear();
					country_backend_mapping[res[i]] = index;

					index++;
					count = 0;
					continue;
				}
				else {
					unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
					if (got == user_backend_mapping.end()) {
						user_backend_mapping[res[i]] = count;
						count++;
						char Vertex = count;
						country3.addVertex(Vertex);
					}

				}//creat a new map
				unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
				if (res[0] != res[i]) {
					unordered_map<string, int>::const_iterator got0 = user_backend_mapping.find(res[0]);
					unordered_map<string, int>::const_iterator goti = user_backend_mapping.find(res[i]);
					country3.addEdge(got0->second, goti->second);
				}//add one vertex into the graph
			}
			else if (index == 4)
			{
				if (isDegital(res[i]) == false) {
					user_backend_mapping4.insert(user_backend_mapping.begin(), user_backend_mapping.end());
					user_backend_mapping.clear();
					country_backend_mapping[res[i]] = index;

					index++;
					count = 0;
					continue;
				}
				else {
					unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
					if (got == user_backend_mapping.end()) {
						user_backend_mapping[res[i]] = count;
						count++;
						char Vertex = count;
						country4.addVertex(Vertex);
					}

				}//creat a new map
				unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
				if (res[0] != res[i]) {
					unordered_map<string, int>::const_iterator got0 = user_backend_mapping.find(res[0]);
					unordered_map<string, int>::const_iterator goti = user_backend_mapping.find(res[i]);
					country4.addEdge(got0->second, goti->second);
				}//add one vertex into the graph
			}
			else if (index == 5)
			{
				if (isDegital(res[i]) == false) {
					user_backend_mapping5.insert(user_backend_mapping.begin(), user_backend_mapping.end());
					user_backend_mapping.clear();
					country_backend_mapping[res[i]] = index;
					index++;
					count = 0;
					continue;
				}
				else {
					unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
					if (got == user_backend_mapping.end()) {
						user_backend_mapping[res[i]] = count;
						count++;
						char Vertex = count;
						country5.addVertex(Vertex);
					}

				}//creat a new map
				unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
				if (res[0] != res[i]) {
					unordered_map<string, int>::const_iterator got0 = user_backend_mapping.find(res[0]);
					unordered_map<string, int>::const_iterator goti = user_backend_mapping.find(res[i]);
					country5.addEdge(got0->second, goti->second);
				}//add one vertex into the graph
			}
			else if (index == 6)
			{
				if (isDegital(res[i]) == false) {
					user_backend_mapping6.insert(user_backend_mapping.begin(), user_backend_mapping.end());
					user_backend_mapping.clear();
					country_backend_mapping[res[i]] = index;
					index++;
					count = 0;
					continue;
				}
				else {
					unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
					if (got == user_backend_mapping.end()) {
						user_backend_mapping[res[i]] = count;
						count++;
						char Vertex = count;
						country6.addVertex(Vertex);
					}

				}//creat a new map
				unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
				if (res[0] != res[i]) {
					unordered_map<string, int>::const_iterator got0 = user_backend_mapping.find(res[0]);
					unordered_map<string, int>::const_iterator goti = user_backend_mapping.find(res[i]);
					country6.addEdge(got0->second, goti->second);
				}//add one vertex into the graph
			}
			else if (index == 7)
			{
				if (isDegital(res[i]) == false) {
					user_backend_mapping7.insert(user_backend_mapping.begin(), user_backend_mapping.end());
					user_backend_mapping.clear();
					country_backend_mapping[res[i]] = index;
					index++;
					count = 0;
					continue;
				}
				else {
					unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
					if (got == user_backend_mapping.end()) {
						user_backend_mapping[res[i]] = count;
						count++;
						char Vertex = count;
						country7.addVertex(Vertex);
					}

				}//creat a new map
				unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
				if (res[0] != res[i]) {
					unordered_map<string, int>::const_iterator got0 = user_backend_mapping.find(res[0]);
					unordered_map<string, int>::const_iterator goti = user_backend_mapping.find(res[i]);
					country7.addEdge(got0->second, goti->second);
				}//add one vertex into the graph
			}
			else if (index == 8)
			{
				if (isDegital(res[i]) == false) {
					user_backend_mapping8.insert(user_backend_mapping.begin(), user_backend_mapping.end());
					user_backend_mapping.clear();
					country_backend_mapping[res[i]] = index;
					index++;
					count = 0;
					continue;
				}
				else {
					unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
					if (got == user_backend_mapping.end()) {
						user_backend_mapping[res[i]] = count;
						count++;
						char Vertex = count;
						country8.addVertex(Vertex);
					}

				}//creat a new map
				unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
				if (res[0] != res[i]) {
					unordered_map<string, int>::const_iterator got0 = user_backend_mapping.find(res[0]);
					unordered_map<string, int>::const_iterator goti = user_backend_mapping.find(res[i]);
					country8.addEdge(got0->second, goti->second);
				}//add one vertex into the graph
			}
			else if (index == 9)
			{
				if (isDegital(res[i]) == false) {
					user_backend_mapping9.insert(user_backend_mapping.begin(), user_backend_mapping.end());
					user_backend_mapping.clear();
					country_backend_mapping[res[i]] = index;
					index++;
					count = 0;
					continue;
				}
				else {
					unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
					if (got == user_backend_mapping.end()) {
						user_backend_mapping[res[i]] = count;
						count++;
						char Vertex = count;
						country9.addVertex(Vertex);
					}

				}//creat a new map
				unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
				if (res[0] != res[i]) {
					unordered_map<string, int>::const_iterator got0 = user_backend_mapping.find(res[0]);
					unordered_map<string, int>::const_iterator goti = user_backend_mapping.find(res[i]);
					country9.addEdge(got0->second, goti->second);
				}//add one vertex into the graph
			}
			else if (index == 10)
			{
				if (isDegital(res[i]) == false) {
					user_backend_mapping10.insert(user_backend_mapping.begin(), user_backend_mapping.end());
					user_backend_mapping.clear();
					country_backend_mapping[res[i]] = index;

					index++;
					count = 0;
					continue;
				}
				else {
					unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
					if (got == user_backend_mapping.end()) {
						user_backend_mapping[res[i]] = count;
						count++;
						char Vertex = count;
						country10.addVertex(Vertex);
					}

				}//creat a new map
				unordered_map<string, int>::const_iterator got = user_backend_mapping.find(res[i]);
				if (res[0] != res[i]) {
					unordered_map<string, int>::const_iterator got0 = user_backend_mapping.find(res[0]);
					unordered_map<string, int>::const_iterator goti = user_backend_mapping.find(res[i]);
					country10.addEdge(got0->second, goti->second);
				}//add one vertex into the graph
			}
			else {
			}
		}
	}
    //create the Server B socket ,code from beej
    int serverB_socket;
    serverB_socket = socket(AF_INET, SOCK_DGRAM,0);

    //define the Server B address ,code from beej
    struct sockaddr_in serverB_address, udp_address;
    serverB_address.sin_family = AF_INET;
    serverB_address.sin_port = htons(31112);
    serverB_address.sin_addr.s_addr = inet_addr("127.0.0.1");

	//boot up message
	cout<<"The server B is up and running using UDP on port 31112"<<endl;

    //bind the socket to our specified IP and sin_port ,code from beej
    bind(serverB_socket, (struct sockaddr*) &serverB_address, sizeof(serverB_address));


    //receive the map id from mainserver
    char recvBuffer[1024];
    
    //get the country list form data;
    string udpcountry;
    char countryList[1024];
    unordered_map<string, int>::iterator iter;
	iter = country_backend_mapping.begin();
	while (iter != country_backend_mapping.end()) {
		udpcountry = udpcountry + " " + iter->first;
		iter++;
	}
    strncpy(countryList,udpcountry.c_str(),udpcountry.length()+1);

    while(1)
    {
		//receive request from main server, if the request is"request list", endserver will send country list to the mainserver
        socklen_t addr_size; 
        addr_size=sizeof(udp_address);
        recvfrom(serverB_socket,&recvBuffer,sizeof(recvBuffer),0, (struct sockaddr*)&udp_address, &addr_size);
        string command = "Request List";
        string received_req;
        received_req = recvBuffer;
		memset(recvBuffer,0,1024);

        //mainserver request country list
        if(received_req == command){
			// code from beej
            sendto(serverB_socket, (const char *)countryList, strlen(countryList),0,(const struct sockaddr*)&udp_address, sizeof(udp_address));
			cout<<"The server B has sent a country list to Main server"<<endl;
			memset(countryList,0,1024);
        }else{
            //break the command from mainserver in to vector.
			cout<<str2<<endl;
            string S = received_req;
            vector<string> resA;
            string result;
            stringstream input(S);
            while (input >> result){
                resA.push_back(result);
            }//result0 is country name and result1 is user id
            
			cout<<"The server B has received request for finding possible friends of User"<<resA[1]<<" in "<< resA[0]<<endl;

            //search the matrix with certain country
            unordered_map<string, int>::iterator it;//country list
	        it = country_backend_mapping.find(resA[0]);
            int matrixID = it->second;//find the matrix id
            if(matrixID== 0){//country name has matrix country1
                sendAnswer(country1,user_backend_mapping1,resA[1],serverB_socket,udp_address,resA[0]);
            }else if(matrixID==1){
                sendAnswer(country2,user_backend_mapping2,resA[1],serverB_socket,udp_address,resA[0]);
            }else if(matrixID==2){
                sendAnswer(country3,user_backend_mapping3,resA[1],serverB_socket,udp_address,resA[0]);
            }else if(matrixID==3){
                sendAnswer(country4,user_backend_mapping4,resA[1],serverB_socket,udp_address,resA[0]);
            }else if(matrixID==4){
                sendAnswer(country5,user_backend_mapping5,resA[1],serverB_socket,udp_address,resA[0]);
            }else if(matrixID==5){
                sendAnswer(country6,user_backend_mapping6,resA[1],serverB_socket,udp_address,resA[0]);
            }else if(matrixID==6){
                sendAnswer(country7,user_backend_mapping7,resA[1],serverB_socket,udp_address,resA[0]);
            }else if(matrixID==7){
                sendAnswer(country8,user_backend_mapping8,resA[1],serverB_socket,udp_address,resA[0]);
            }else if(matrixID==8){
                sendAnswer(country9,user_backend_mapping9,resA[1],serverB_socket,udp_address,resA[0]);
            }else if(matrixID==9){
                sendAnswer(country10,user_backend_mapping10,resA[1],serverB_socket,udp_address,resA[0]);
            }else{
                cout<<"no counrty matrix"<<endl;
            }
        }
    }//end while loop

    //close the socket,it will never be used
    close(serverB_socket);

    return 0;
}