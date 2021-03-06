/* server.cpp work on PC */

#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>

#define SERVER_ADDR "127.0.0.1"
#define PORT_NUM 10000
#define WAIT_QUEUE_SIZE 5
#define OUTPUT_BUFFER_SIZE 1024
#define IMAGE_NAME "sh"

void* serverRoutine(void* new_socket);

int main(int argc, char* argv[])
{
	/* pthread variables */
	pthread_t tid;
	pthread_attr_t thread_attr;
	int ret;
	//int t_idx = 0;

	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);	
	//pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);	

	/* socket variables */
	int server_sockfd = 0, client_sockfd = 0;
	struct sockaddr_in serverInfo, clientInfo;
	socklen_t addrlen = sizeof(clientInfo);
	

	/* server setup */
	std::memset(&serverInfo, 0, sizeof(serverInfo));
	serverInfo.sin_family      = PF_INET;					// ipv4
	serverInfo.sin_addr.s_addr = inet_addr(SERVER_ADDR);	// change to INADDR_ANY if want to listen to all addr
	serverInfo.sin_port        = htons(PORT_NUM);

	if((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Create socket server failed");
		exit(EXIT_FAILURE);
	}
	
	bind(server_sockfd, (struct sockaddr*)&serverInfo, sizeof(serverInfo));
	std::cout << "  * server binding to addr: " << inet_ntoa(serverInfo.sin_addr) << std::endl;

	if(listen(server_sockfd, WAIT_QUEUE_SIZE) == 0){
		std::cout << "  * server listening to addr: " << inet_ntoa(serverInfo.sin_addr) << std::endl;
	} else{
		perror("Listening failed");
		exit(EXIT_FAILURE);
	}

	int count = 0;
	while(1){
		client_sockfd = accept(server_sockfd, (struct sockaddr*)&clientInfo, &addrlen);	
		std::cout << "  * server accept #" << ++count << std::endl;

		/* create a detached thread to handle client request */

		ret = pthread_create(&tid, &thread_attr, &serverRoutine, &client_sockfd);
		if(ret){
			perror("pthread_create()");
			exit(EXIT_FAILURE);
		}

/*
		if(t_idx >= 50){
			t_idx = 0;
			while(t_idx < 50){
				pthread_join(tids[t_idx++], NULL);
			}
			t_idx = 0;
		}
*/
	}

	pthread_attr_destroy(&thread_attr);
	pthread_exit(NULL);
}

void* serverRoutine(void* new_socket){
	int client_sockfd = *((int *)new_socket);

	char outputBuffer[OUTPUT_BUFFER_SIZE];
	std::memset(&outputBuffer, 0, OUTPUT_BUFFER_SIZE);
	
	char* filename = new char [sizeof(IMAGE_NAME) + 4];
	strcpy(filename, IMAGE_NAME);

	FILE* image_file = fopen(strcat(filename, ".png"), "rb");
	int ret;
	
	while(!feof(image_file)){
		/* send whole image file to client */
		ret = fread(outputBuffer, 1, OUTPUT_BUFFER_SIZE, image_file);
		send(client_sockfd, outputBuffer, ret, 0);
	}

	fclose(image_file);
	close(client_sockfd);
	delete [] filename;

	std::cout << "  * send complete ! Exit thread ..." << std::endl;

	pthread_exit(NULL);
}
