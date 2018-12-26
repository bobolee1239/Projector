/* server.cpp */

#include <iostream>
#include <stdio.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define IP_ADDR  "140.114.57.83"
#define PORT_NUM 10000
#define WAIT_QUEUE_SIZE 5
#define INPUT_BUFFER_SIZE 512

int main(int argc, char** argv)
{
	int sockfd = 0, clientSockfd = 0;
	char inputBuffer[INPUT_BUFFER_SIZE];
	std::memset(&inputBuffer, 0, INPUT_BUFFER_SIZE);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Fail to create socket server");	
	}

	struct sockaddr_in serverInfo, clientInfo;
	int addrlen = sizeof(clientInfo);

	/* server setup */
	std::memset(&serverInfo, 0, sizeof(serverInfo));
	serverInfo.sin_family = PF_INET;					// ipv4
	serverInfo.sin_addr.s_addr = inet_addr(IP_ADDR);
	serverInfo.sin_port = htons(PORT_NUM);

	bind(sockfd, (struct sockaddr *)&serverInfo, sizeof(serverInfo));
	std::cout << "  * server binding to addr: " << IP_ADDR << std::endl;

	listen(sockfd, WAIT_QUEUE_SIZE);
	std::cout << "  * server listening ...\n";
	
	while(1){
		std::cout << "  * server waiting ...\n";
		clientSockfd = accept(sockfd, (struct sockaddr*)&clientInfo, (socklen_t*)&addrlen);	
		std::cout << "  * server accept addr: " << inet_ntoa(clientInfo.sin_addr) << std::endl;

		int ret;
		FILE* out = fopen("output.png", "wb");
		int count = 0;
		while((ret = recv(clientSockfd, inputBuffer, INPUT_BUFFER_SIZE, 0)) > 0){
			//std::cout << "receive " << ret << " bytes\n"; 
			/* write to file */	
			fwrite(inputBuffer, 1, ret, out);
		
		
			if(ret < INPUT_BUFFER_SIZE) break;

		}
		fclose(out);
	}


	return 0;
}

