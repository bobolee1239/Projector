/* client.cpp works on PXA */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <pthread.h>

#define SERVER_ADDR "127.0.0.1"
#define PORT_NUM 10000
#define INPUT_BUFFER_SIZE 1024
#define IMAGE_NAME "output"
#define NUM_CLIENTS 2

void* clientRoutine(void* no);


int main(int argc, char* argv[])
{
	pthread_t clients[NUM_CLIENTS];
	pthread_attr_t thread_attr;
	int rc;
	void * status;

	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);

	for(int i=0; i<NUM_CLIENTS; ++i){
		
		std::cout << "Main thread creating client #" << i << std::endl;
		
		rc = pthread_create(&clients[i], &thread_attr, &clientRoutine, (void *)&i);
		if(rc){
			perror("fail to create client");
			exit(EXIT_FAILURE);
		}
		usleep(1000);
	}
	
	/* destroy thread attribute and waiting for other threads */
	pthread_attr_destroy(&thread_attr);
	for(int i=0; i<NUM_CLIENTS; ++i){
		rc = pthread_join(clients[i], &status);
		if(rc){
			perror("fail to thread join");
			exit(EXIT_FAILURE);
		}
		std::cout << "Main thread join with client #" << i << std::endl;
	}
	
	pthread_exit(NULL);
}

void* clientRoutine(void* no){
	int number = *((int *)no);
	/* outfile name */
	char* outfile_name = new char [sizeof(IMAGE_NAME) + 5]; 		// delete [] in the thread 
	char num[2] = {'\0'};
	num[0] = '0' + number;

	strcpy(outfile_name, IMAGE_NAME);
	strcat(outfile_name, num);
	strcat(outfile_name, ".xwd");

	int sockfd;
	struct sockaddr_in sockInfo; 
	char inputBuffer[INPUT_BUFFER_SIZE];
	char outputBuffer[] = "sendata";

//for(int i=0; i<1000; ++i){
	std::memset(inputBuffer, 0, INPUT_BUFFER_SIZE);

	sockfd = 0;

	std::memset(&sockInfo, 0, sizeof(sockInfo));
	sockInfo.sin_family      = PF_INET;					//ipv4
	sockInfo.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	sockInfo.sin_port        = htons(PORT_NUM);

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("Fail to create socket");	
		exit(EXIT_FAILURE);
	}

	int err = connect(sockfd, (struct sockaddr*)&sockInfo, sizeof(sockInfo));
	if(err == -1){
		perror("Connection error!");
		exit(EXIT_FAILURE);
	}
	
	/* keep transfering data */
	int count = 0;
	int ret;
	FILE* out_image;
    while(1){
		ret = send(sockfd, outputBuffer, 8, 0);
		bool isFirst = true;
		while((ret = recv(sockfd, inputBuffer, INPUT_BUFFER_SIZE, 0)) > 0){
			if(isFirst){
#ifdef DEBUG
				std::cout << "\t * count [" << number << "] = " << ++count << std::endl;
				std::cout << "open file: " << outfile_name << std::endl;
#endif
				out_image = fopen(outfile_name, "wb"); 	 
				isFirst = false;
			}
			/* write to file */
			fwrite(inputBuffer, 1, ret, out_image);

			if(ret < INPUT_BUFFER_SIZE)		break;
		}
		if (!isFirst){
#ifdef DEBUG
			std::cout << "closing file: " << outfile_name << std::endl;
#endif
			fclose(out_image);
		}
#ifdef DEBUG
		if(count >= 1000) break;
#endif
    }
    //close(sockfd);	
//}
	delete [] outfile_name;
	std::cout << "Exist client#" << number << " ...\n";
	pthread_exit(NULL);
}
