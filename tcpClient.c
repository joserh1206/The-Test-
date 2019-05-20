#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 4444

int main(){

	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	char buffer[1024];
	char buffer2[1024];

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Server.\n");

	while(1){
		const char separator[2] = "$";
		char *user_from_list;
		bzero (&buffer, sizeof (buffer));
		bzero (&buffer2, sizeof (buffer2));
		printf("Ingrese su nombre de usuario:\n");
		scanf("%s", &buffer[0]);
		
		strcat(buffer, "$");

		printf("Ingrese su contrasenia:\n");
		scanf("%s", &buffer2[0]);
		
		strcat(buffer, buffer2);
		printf("BUFFER: %s\n", buffer);
		send(clientSocket, buffer, strlen(buffer), 0); //Envia el usuario+contrasenia

		bzero (&buffer, sizeof (buffer));

		recv(clientSocket, buffer, 1024, 0); //Si existe recibe lista de usuarios disponibles, si no recibe -1
		if(strcmp(buffer, "-1") == 0){
			printf("El usuario ingresado no se encuentra en la BD, desea agregarlo? (S/N):\n");
			scanf("%s", &buffer[0]);
			send(clientSocket, buffer, strlen(buffer), 0); //Envia respuesta al servidor
			bzero (&buffer, sizeof (buffer));
			recv(clientSocket, buffer, 1024, 0); //Recibe lista jugadores disponibles
			user_from_list = buffer;
			while((user_from_list = strtok(user_from_list, separator)) != NULL){
				printf("%s\n", user_from_list);
				user_from_list = NULL;
			}
			bzero (&buffer, sizeof (buffer));
			printf("Seleccione el numero del usuario con el que desea iniciar una nueva partida\n");
			scanf("%s", &buffer[0]);
			send(clientSocket, buffer, strlen(buffer), 0); //Envia numero de usuario para partida al servidor
		}
		else{
			user_from_list = buffer;
			while((user_from_list = strtok(user_from_list, separator)) != NULL){
				printf("%s\n", user_from_list);
				user_from_list = NULL;
			}
			bzero (&buffer, sizeof (buffer));
			printf("Seleccione el numero del usuario con el que desea iniciar una nueva partida\n");
			scanf("%s", &buffer[0]);
			send(clientSocket, buffer, strlen(buffer), 0); //Envia numero de usuario para partida al servidor
		}
/*
		if(recv(clientSocket, buffer, 1024, 0) < 0){
			printf("[-]Error in receiving data.\n");
		}else{
			printf("Server: %s\n", buffer);
		}*/
	}

	return 0;
}