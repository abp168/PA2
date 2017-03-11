#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include<iostream>
#include <fstream>

#include "packet.h"
#include "packet.cpp"

using namespace std;

int main (int argc, char ** argv){
	//creates UDP socket	
	int hostsocket,emulatorport,serverport;
	hostsocket=socket(AF_INET, SOCK_DGRAM,0);
	
	struct sockaddr_in server;
	memset((char *) &server, 0, sizeof (server));
	server.sin_family= AF_INET;
	serverport=atoi(argv[2]);
	server.sin_port=htons(serverport);
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	
	//binds server to new socket
	bind(hostsocket,(struct sockaddr *)&server,sizeof(server));
	
	//get address for emulator
	struct hostent *em;
	em= gethostbyname(argv[1]);
	
	//get port for emulator
	struct sockaddr_in emulator;
	memset((char *) &emulator, 0, sizeof (emulator));
	emulator.sin_family= AF_INET;
	emulatorport=atoi(argv[3]);
	emulator.sin_port=htons(emulatorport);
	bcopy((char*)em->h_addr,
		 (char*)&emulator.sin_addr.s_addr,
		 em->h_length);
	
	socklen_t emulatorlen=sizeof(emulator);
	
	ofstream outfile;
	outfile.open("output.txt");	
	
	ofstream arrivalfile;
	arrivalfile.open("arrival.log.txt");	
		
	char data[37]; 
	char ack[10]; 
	char data1[30]; 
	
	int type=1;
	int seqnum=0;
	int acktype=0;
	int expectedseq=0;
	
	packet datapacket(type,seqnum,sizeof(data1),data1);		

	while (type!=3){	
		
		memset ((char*)&data,0,sizeof(data));
		memset ((char*)&data1,0,sizeof(data1));
		memset ((char*)&ack,0,sizeof(ack));
		
		// Recieve packet from client and deserialize
		recvfrom(hostsocket,data,sizeof(data),0,(struct sockaddr *)&emulator, &emulatorlen);		
		datapacket.deserialize((char*)data);
		seqnum = datapacket.getSeqNum();
		arrivalfile<<seqnum;
		arrivalfile<<"\n";
		datapacket.printContents();
		
		if (expectedseq!=seqnum){
			packet ackpacket(acktype,expectedseq,0,0);
			ackpacket.serialize((char*)ack);
			sendto(hostsocket,ack,sizeof(ack),0,(struct sockaddr *)&emulator, sizeof(emulator));
			ackpacket.printContents();			
		}
		else{
			type=datapacket.getType();
			if (type==3){

				acktype=2;
				packet ackpacket(acktype,seqnum,0,0);
				ackpacket.serialize((char*)ack);
				sendto(hostsocket,ack,sizeof(ack),0,(struct sockaddr *)&emulator, sizeof(emulator));
	
				arrivalfile<<"\n";
				ackpacket.printContents();
				break;		
			}	
	
			outfile<<datapacket.getData();
		
			//makes ack packet to send to client
			packet ackpacket(acktype,seqnum,0,0);
			ackpacket.serialize((char*)ack);
			sendto(hostsocket,ack,sizeof(ack),0,(struct sockaddr *)&emulator, sizeof(emulator));
			ackpacket.printContents();
			
			expectedseq++;
			if (expectedseq==8){
				expectedseq=0;
			}
		}
	}

	
	outfile.close();
	arrivalfile.close();
	close (hostsocket);
}
	