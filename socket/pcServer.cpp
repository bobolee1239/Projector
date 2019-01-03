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

#define WAIT_QUEUE_SIZE 5
#define OUTPUT_BUFFER_SIZE 1024
#define IMAGE_NAME "sh"
#define CAPTURE_IMG "screencapture -x "
#define EXTENSION ".png"
#define NUM_TESTS 50


void* serverRoutine(void* new_socket);

int main(int argc, char* argv[])
{
	if (argc < 3){
		std::cerr << "Usage: ./pcServer <ip_address> <port_num>\n";
		exit(-1);
	}

	char* SERVER_ADDR = argv[1];
	int PORT_NUM = atoi(argv[2]);
	
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
	char inputBuffer[8];
	std::memset(&inputBuffer, 0, 8);
	std::memset(&outputBuffer, 0, OUTPUT_BUFFER_SIZE);
	
	char* filename = new char [sizeof(IMAGE_NAME) + 4];
	char* command = new char [sizeof(CAPTURE_IMG) + 5];

	std::memset(filename, 0, sizeof(IMAGE_NAME) + 4);
	std::memset(command, 0, sizeof(CAPTURE_IMG) + 5);

	strcpy(filename, IMAGE_NAME);
#ifdef DEBUG
	char num[2] = {'0'};
	num[0] = client_sockfd + '0';
	strcat(filename, num);
#endif
	strcat(filename, EXTENSION);
	
	strcpy(command, CAPTURE_IMG);
	strcat(command, filename);

	int count = 0;
	int ret;
	FILE* image_file;
	int* file_size = new int [1];
	while(1){
		ret = recv(client_sockfd, inputBuffer, 8, 0);
		if(strcmp(inputBuffer, "sendata") != 0){
			std::cerr << "ERROR; Header " << std::endl;
			continue;
		}
		system(command);
		image_file = fopen(filename, "rb");
		
		/* check file size */
		std::memset(file_size, 0, sizeof(*file_size));
		fseek(image_file, 0, SEEK_END);
		*file_size = ftell(image_file);
		fseek(image_file, 0, SEEK_SET);			// set back to origin
		
#ifdef DEBUG
		std::cout << "\t * [" << client_sockfd << "]i = " << ++count << ", file size: " << *file_size  << std::endl;
#endif
		/* send file size to client */
		send(client_sockfd, file_size, sizeof(*file_size), 0);

		while(!feof(image_file)){
			/* send whole image file to client */
			ret = fread(outputBuffer, 1, OUTPUT_BUFFER_SIZE, image_file);
			send(client_sockfd, outputBuffer, ret, 0);
		}

		fclose(image_file);
#ifdef DEBUG
		if(count >= NUM_TESTS) break;
#endif
	}

	close(client_sockfd);

	delete [] filename;
	delete [] command;
	delete [] file_size;

	std::cout << "  * send complete ! Exit thread [client sockfd: " << client_sockfd << "] ..." << std::endl;

	pthread_exit(NULL);
}
