#
# compile statments
#
# client: ./client localhost 5678 5679 file.txt
#
# server: ./server localhost 5678 5679 output.txt

all: client server

.PHONY: client
client: 	
	g++ client.cpp -o client

.PHONY: server
server: 
	g++ server.cpp -o server	

.PHONY: clean
clean:
	rm *.o client server
