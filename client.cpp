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
#include <ctime>
#include <time.h>

#include "packet.h"
#include "packet.cpp"



using namespace std;

int main (int argc, char ** argv){
	
	struct timeval timeout;
	timeout.tv_sec = 2;
	
    //creates a UDP socket
	int hostsocket,emulatorport,clientport;
	hostsocket=socket(AF_INET, SOCK_DGRAM,0);			
	
	struct sockaddr_in client;
	memset((char *) &client, 0, sizeof (client));
	client.sin_family= AF_INET;
	clientport=atoi(argv[3]);
	client.sin_port=htons(clientport);
	client.sin_addr.s_addr=htonl(INADDR_ANY);
	
	//binds client to new socket
	bind(hostsocket,(struct sockaddr *)&client,sizeof(client));
	
	//get address for emulator
	struct hostent *em;
	em= gethostbyname(argv[1]);
	
	//get port for emulator
	struct sockaddr_in emulator;
	memset((char *) &emulator, 0, sizeof (emulator));
	emulator.sin_family= AF_INET;
	emulatorport=atoi(argv[2]);
	emulator.sin_port= htons(emulatorport);
	bcopy((char*)em->h_addr,
		 (char*)&emulator.sin_addr.s_addr,
		 em->h_length);
	
	socklen_t emulatorlen=sizeof(emulator);
	
	char buffer[37];
	char fbuffer[30];
	char data[37]; 
	int type;
	int seqnum=0;
	int ackseq=0;
	int window=0;
	
	ifstream file;
	file.open(argv[4]);
	
	ofstream ackfile;
	ackfile.open("ack.log.txt");	
	
	ofstream seqnumfile;
	seqnumfile.open("seqnum.log.txt");	

  
    while (!file.eof() && ackseq==seqnum) {		
		
		setsockopt(hostsocket,SOL_SOCKET, SO_RCVTIMEO,&timeout,sizeof(timeout));
	
		memset ((char*)&buffer,0,sizeof(buffer));
		memset ((char*)&fbuffer,0,sizeof(fbuffer));
		memset ((char*)&data,0,sizeof(data));
		if (window==7 || !file.eof())
		{
			//Recieves ackpacket from server
			packet ackpacket(0,0,0,0);
			recvfrom(hostsocket,data,sizeof(data),0,(struct sockaddr *)&emulator, &emulatorlen);		
			ackpacket.deserialize((char*)data);
			ackpacket.printContents();
			ackseq=ackpacket.getSeqNum();
			ackfile<<ackpacket.getSeqNum();
			ackfile<<"\n";
			window--;
			
		}
		else{
		
			file.read(fbuffer,30);
			type=1;
		
			//Makes and sends data packet to server
			packet datapacket(type,seqnum,sizeof(fbuffer),fbuffer);
			datapacket.serialize((char*) buffer);
			sendto(hostsocket,buffer,sizeof(buffer),0,(struct sockaddr *)&emulator, sizeof(emulator));
			datapacket.printContents();
			seqnumfile<<datapacket.getSeqNum();
			seqnumfile<<"\n";
			
			seqnum++;
			window++;
		}
		
		memset ((char*)&buffer,0,sizeof(buffer));
		memset ((char*)&fbuffer,0,sizeof(fbuffer));
		memset ((char*)&data,0,sizeof(data));

	}
	
	memset ((char*)&buffer,0,sizeof(buffer));
	memset ((char*)&fbuffer,0,sizeof(fbuffer));
	memset ((char*)&data,0,sizeof(data));
	
	type=3;
	
	//Makes and sends data EOF packet to server
	packet endpacket(type,seqnum,0,0);
	endpacket.serialize((char*) buffer);
	sendto(hostsocket,buffer,sizeof(buffer),0,(struct sockaddr *)&emulator, sizeof(emulator));
	endpacket.printContents();
	seqnumfile<<endpacket.getSeqNum();
	seqnumfile<<"\n";
	
	//Recieves EOF ack from server
	packet endackpacket(0,0,0,0);
	recvfrom(hostsocket,data,sizeof(data),0,(struct sockaddr *)&emulator, &emulatorlen);		
	endackpacket.deserialize((char*)data);
	endackpacket.printContents();
	ackfile<<endackpacket.getSeqNum();
	ackfile<<"\n";

	file.close();
	ackfile.close();
	seqnumfile.close();
	close (hostsocket);
}