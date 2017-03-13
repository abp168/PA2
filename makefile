#
# compile statments
#
# client: ./client localhost 5678 5679 file.txt
#
# server: ./server localhost 5678 5679 output.txt

all: client server

client: 
	client.cpp client.o	
	g++ client.cpp -o client

server: 
	server.cpp server.o
	g++ server.cpp -o server	

clean:
	\rm *.o client server
