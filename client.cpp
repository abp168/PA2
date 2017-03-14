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
#include <sys/poll.h>

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
	
	int type;
	int seqnum=0;
	int ackseq=-1;
	int window=0;
	int send_base = 0;
	int expectedack=0;
	int location=0;
	int timeout;
	int interrupt;
	
	ifstream file;
	file.open(argv[4]);
	
	ofstream ackfile;
	ackfile.open("ack.log.txt");	
	
	ofstream seqnumfile;
	seqnumfile.open("seqnum.log.txt");	
	struct pollfd ufds[2]; // create poll struct to interrupt recvfrom()
	ufds[0].fd = hostsocket;
	ufds[0].events = POLLIN;
	
	
	time_t start,stop;
	
		

	while (true) {	
	

		memset ((char*)&buffer,0,sizeof(buffer));
		memset ((char*)&fbuffer,0,sizeof(fbuffer));
		memset ((char*)&data,0,sizeof(data));

		
		if (!file.eof())
		  {
		    file.read(fbuffer,30);
			type=1;
			if (seqnum==8)
			  {
				seqnum=0;
			  }
		
			//Makes and sends data packet to server
			packet datapacket(type,seqnum,sizeof(fbuffer),fbuffer);
			datapacket.serialize((char*) buffer);
			sendto(hostsocket,buffer,sizeof(buffer),0,(struct sockaddr *)&emulator, sizeof(emulator));
			
			datapacket.printContents();
			seqnumfile<<seqnum;
			seqnumfile<<"\n";
						
			seqnum++;
			window++;
		  }
	   
		if (window==7)
		  {
		     interrupt= poll(ufds,1,2000);
		    //Recieves ackpacket from server
		    packet ackpacket(0,0,0,0);
			
			    
		    if(interrupt == -1)
		      {
			printf("Error in polling.\n");
		      }
		    if (interrupt == 0)
		      {
			printf("Timeout occured!\n");
		      }
		    else
		      {
			if(ufds[0].revents & POLLIN)
			{
			    recvfrom(hostsocket,data,sizeof(data),0,(struct sockaddr *)&emulator, &emulatorlen);
			    ackpacket.deserialize((char*)data);
			    ackpacket.printContents();
			    ackseq=ackpacket.getSeqNum();
			    ackfile<<ackseq;
			    ackfile<<"\n";
		       	    if(send_base == ackseq)
			      {
				send_base++;
				window =0;
			      }
			}
		      }
		  }
			  
			    /* if (expectedack!=ackseq) //|| timeout>=2)
			      {
				location= (expectedack-7) * 30;
				file.seekg(location,ios::cur);
				seqnum=expectedack;
				window=0;	
				
				//				
				//time(&start);											
				//
					
			      }			
			    else
			      {
				
				window--;
				expectedack++;
				if (expectedack==8)
				  {
				    expectedack=0;
				  }

				  }*/
		if ( file.eof())
		  {
					
		    //Makes and sends data EOF packet to server
		    type=3;	
		    packet endpacket(type,seqnum,0,0);
		    endpacket.serialize((char*) buffer);
		    sendto(hostsocket,buffer,sizeof(buffer),0,(struct sockaddr *)&emulator, sizeof(emulator));
		    endpacket.printContents();
		    seqnumfile<<seqnum;
		    seqnumfile<<"\n";
	
		    //Recieves EOF ack from server
		    packet endackpacket(0,0,0,0);
		    interrupt= poll(ufds,1,2000);
		     if(interrupt == -1)
		      {
			printf("Error in polling.\n");
		      }
		    if (interrupt == 0)
		      {
			printf("Timeout occured!\n");
		      }
		    else
		      {
			if(ufds[0].revents & POLLIN)
			{
			    recvfrom(hostsocket,data,sizeof(data),0,(struct sockaddr *)&emulator, &emulatorlen);		
			    endackpacket.deserialize((char*)data);
			    endackpacket.printContents();
			    ackseq=endackpacket.getSeqNum();
			    ackfile<<ackseq;
			    ackfile<<"\n";
			    break;
			}
		      }
		  }
	}			
		
				
//	
				
		/*time(&stop);
		  timeout=difftime(stop,start);*/
//	
				
			
				



	file.close();
	ackfile.close();
	seqnumfile.close();
	close (hostsocket);
}
