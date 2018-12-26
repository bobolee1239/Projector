/* client.cpp */

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define IP_ADDR  "140.114.57.83"
#define PORT_NUM 10000
#define OUTPUT_BUFFER_SIZE 512
#define IMAGE_FILE "gaga.png"

void sendpng(char* filename, char* ip_addr, short port_num, char* buffer, int buffer_size);

int main(int argc, char** argv)
{
	char outputBuffer[OUTPUT_BUFFER_SIZE];
	std::memset(outputBuffer, 0, OUTPUT_BUFFER_SIZE);
	
	int count = 0;
	while(count++ < 30){
		sendpng(IMAGE_FILE, IP_ADDR, PORT_NUM, outputBuffer, OUTPUT_BUFFER_SIZE);
	}

	return 0;
	
}

void sendpng(char* filename, char* ip_addr, short port_num, char* buffer, int buffer_size){
	int sockfd = 0;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("Failed to create socket");
	}

	struct sockaddr_in info;
	std::memset(&info, 0, sizeof(info));
	info.sin_family = PF_INET;			// ipv4
	info.sin_addr.s_addr = inet_addr(ip_addr);
	info.sin_port = htons(port_num);

	int err = connect(sockfd, (struct sockaddr*)&info, sizeof(info));
	if(err == -1){
		perror("Connection error!");
	}


	FILE* file = fopen(filename, "rb");
	int ret;

	/* send mesg to server */
	while(!feof(file)){
		ret = fread(buffer, 1, buffer_size, file);
		send(sockfd, buffer, ret, 0);
	}
	fclose(file);

	std::cout << "  * send complete ! \n";
	close(sockfd);
}

