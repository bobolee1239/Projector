/* client.cpp works on PXA */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string.h>
#include <math.h>

#define INPUT_BUFFER_SIZE 1024
#define IMAGE_NAME "./screenshot/output"
#define EXTENSION ".png"

#ifdef DEBUG
#define NUM_TESTS 50 
#endif

void clientRoutine(char* server_ip, short server_port_num);

int main(int argc, char* argv[])
{
	if (argc < 3){
		std::cerr << "[Usage] ./client <ip_addr> <port_num>" << std::endl;
		exit(-1);
	}

	char* server_ip = argv[1];
	int server_port_num = atoi(argv[2]);

	clientRoutine(server_ip, server_port_num);

	return 0;
}

void clientRoutine(char* server_ip, short server_port_num){
	/* outfile name */
	char* outfile_name = new char [sizeof(IMAGE_NAME) + 6]; 		// delete [] in the thread 

	std::memset(outfile_name, 0, sizeof(IMAGE_NAME) + 6);
	strcpy(outfile_name, IMAGE_NAME);
	strcat(outfile_name, EXTENSION);

	int sockfd;
	struct sockaddr_in sockInfo; 
	char inputBuffer[INPUT_BUFFER_SIZE];
	char outputBuffer[] = "sendata";

//for(int i=0; i<1000; ++i){
	std::memset(inputBuffer, 0, INPUT_BUFFER_SIZE);

	sockfd = 0;

	std::memset(&sockInfo, 0, sizeof(sockInfo));
	sockInfo.sin_family      = PF_INET;					//ipv4
	sockInfo.sin_addr.s_addr = inet_addr(server_ip);
	sockInfo.sin_port        = htons(server_port_num);

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
	int* data_size = new int [1];

    while(1){
		ret = send(sockfd, outputBuffer, 8, 0);

		/* recognize size of received file */
		std::memset(data_size, 0, sizeof(*data_size));
		ret = recv(sockfd, data_size, sizeof(*data_size), 0);		// receive data size;

		bool isFirst = true;

		/* receiving whole img */
		while(1){
			std::memset(inputBuffer, 0, INPUT_BUFFER_SIZE);
			if(*data_size < INPUT_BUFFER_SIZE){
				// the last chunk 
				ret = recv(sockfd, inputBuffer, *data_size,0);	
			} else {
				ret = recv(sockfd, inputBuffer, INPUT_BUFFER_SIZE, 0);
			}
			if(isFirst){
#ifdef DEBUG // for debugging sake 
				std::cout << "\t * count = " << ++count << ", file_size: " << *data_size << std::endl;
				std::memset(outfile_name, 0, sizeof(IMAGE_NAME) + 6);
				strcpy(outfile_name, IMAGE_NAME);
				char count_str[4] = {'\0'};
				for(int i=0; i<int(log10(NUM_TESTS)); ++i)	
					count_str[i] = '0';
				int tmp = count;
				int cc = 0;
				while(tmp != 0){
					count_str[int(log10(NUM_TESTS)) - cc++] = (tmp%10) + '0';
					tmp /= 10;
				}
				strcat(outfile_name, count_str);
				strcat(outfile_name, EXTENSION);
				std::cout << "open file: " << outfile_name << std::endl;
#endif
				out_image = fopen(outfile_name, "wb"); 	 
				isFirst = false;
			}
			/* write to file */
			ret = fwrite(inputBuffer, 1, ret, out_image);
			
			*data_size = *data_size - ret; 

			if(*data_size == 0)		break;
#ifdef DEBUG
			else if(*data_size < 0) std::cerr << "ERROR; send too many bytes!!" << std::endl;
#endif
		}
		if (!isFirst){
#ifdef DEBUG
			std::cout << "closing file: " << outfile_name << std::endl;
#endif
			fclose(out_image);
		}
#ifdef DEBUG
		if(count >= NUM_TESTS) break;
#endif
    }
    //close(sockfd);	
//}
	delete [] outfile_name;
	delete data_size;
	std::cout << "Exist client ...\n";
}
