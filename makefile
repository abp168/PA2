all: client server

client: 
	client.cpp client.o	
	g++ client.cpp -o client
	./client localhost 5678 5679 file.txt
	
server: 
	server.cpp server.o
	g++ server.cpp -o server	
	./server localhost 5678 5679 output.txt
clean:
	\rm *.o client server
