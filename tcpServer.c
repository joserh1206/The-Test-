#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sqlite3.h>

#define PORT 4444

sqlite3* openDatabase();

int main(){

	int sockfd, ret;
	 struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	char buffer[1024];
	char done[6] = "DONE";

	pid_t childpid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Server Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in binding.\n");
		exit(1);
	}
	printf("[+]Bind to port %d\n", 4444);

	if(listen(sockfd, 10) == 0){
		printf("[+]Listening....\n");
	}else{
		printf("[-]Error in binding.\n");
	}


	while(1){
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		if(newSocket < 0){
			exit(1);
		}
		printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

		if((childpid = fork()) == 0){
			close(sockfd);

			while(1){
				const char separator[2] = "$";
				char *word, *type;
				recv(newSocket, buffer, 1024, 0);
				word = strtok(buffer, separator);
				type = strtok(NULL, separator);
				if(strcmp(type, "e") == 0){
					printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					break;
				}
				if(strcmp(type, "u") == 0){
					printf("Se envió el nombre de usuario\n");
					printf("Y es %s\n", word);
					send(newSocket, done, strlen(done), 0);
					bzero(&done, sizeof(done));
					bzero(&buffer, sizeof(buffer));
				}
				if(strcmp(type, "p") == 0){
					printf("Se recibió la contrasenia\n");
					printf("Y es %s\n", word);
					send(newSocket, done, strlen(done), 0);
					bzero(&done, sizeof(done));
					bzero(&buffer, sizeof(buffer));
				}
				
				
				else{
					//sqlite3 *db = openDatabase();
					//printf("Client: %s\n", buffer);
					//send(newSocket, buffer, strlen(buffer), 0);
					bzero(&buffer, sizeof(buffer));
				}
			}
		}

	}

	close(newSocket);


	return 0;
}
/*
sqlite3* openDatabase()
{
  sqlite3 *db;
  sqlite3_open("DatabaseTheTest.db", &db);

  if( (sqlite3_open("DatabaseTheTest.db", &db)) ) 
  {
    fprintf(stderr, "[Database]Can't open database: %s\n", sqlite3_errmsg(db));
    exit(0);
  } 

  return db;
}*/