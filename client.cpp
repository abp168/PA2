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
	int type=1;
	int seqnum=0;
	int ackseq=0;
	char seqnuma[2];
	char ackseqn[2];
	
	ifstream file;
	file.open(argv[4]);
	
	ofstream ackfile;
	ackfile.open("ack.log.txt");	
	
	ofstream seqnumfile;
	seqnumfile.open("seqnum.log.txt");	

  
    while (file.read(fbuffer,30)) {			
		
		//Makes and sends data packet to server
		packet datapacket(type,seqnum,sizeof(fbuffer),fbuffer);
		datapacket.serialize((char*) buffer);
		sendto(hostsocket,buffer,sizeof(buffer),0,(struct sockaddr *)&emulator, sizeof(emulator));
		

	
		//Recieves ackpacket from server
		packet ackpacket(0,0,0,0);
		recvfrom(hostsocket,data,sizeof(data),0,(struct sockaddr *)&emulator, &emulatorlen);		
		ackpacket.deserialize((char*)data);
		
		ackseq=ackpacket.getSeqNum();
		sprintf(ackseqn,"%d",ackseq);	
		ackfile.write(ackseqn,sizeof(seqnuma));
		
		
		memset ((char*)&buffer,0,sizeof(buffer));
		memset ((char*)&fbuffer,0,sizeof(fbuffer));
		memset ((char*)&data,0,sizeof(data));
		seqnum=seqnum+1;
		type=1;
	
	}

	
	
	file.close();
	close (hostsocket);
}