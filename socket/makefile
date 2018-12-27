all: pcServer pxaClient 

pcServer: pcServer.cpp
	g++ pcServer.cpp -g -lpthread -o pcServer

pxaClient: pxaClient.cpp
	g++ pxaClient.cpp -g -lpthread -o pxaClient

server: server.cpp
	g++ server.cpp -o server

client: client.cpp
	g++ client.cpp -o client

clean:
	rm -rf server client pcServer pxaClient
