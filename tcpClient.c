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
	char bufferquestions[1024];

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	//memset(&serverAddr, '\0', sizeof(serverAddr));
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
		char *user_from_list, *questions, *answers;
		bzero (&buffer, sizeof (buffer));
		bzero (&buffer2, sizeof (buffer2));
		bzero (&bufferquestions, sizeof (bufferquestions));
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
			if(strcmp(buffer, "N") == 0){
				printf("Está bien, sin rencores...\n");
				continue;
			}
			bzero (&buffer, sizeof (buffer));
			recv(clientSocket, buffer, 1024, 0);
			if(strcmp(buffer, "Ok") == 0){
				printf("Usuario registrado, BIENVENIDO\n");
			}else{
				printf("Ha ocurrido un problema inesperado. Intentelo de nuevo\n");
				continue;
			}
			while (1)
			{
				bzero (&buffer, sizeof (buffer));
				printf("\n* ** * MENU * ** *\n");
				printf("1. Iniciar una partida nueva\n");
				printf("2. Continuar partida\n");
				printf("3. Ver estadisticas\n");
				printf("4. Salir\n");
				scanf("%s", &buffer[0]);
				if((buffer[0] - '0') > 4){
					printf("Debe ingresar una opcion válida\n");
					continue;
				}
				send(clientSocket, buffer, strlen(buffer), 0); //Envia el numero de la opción
				if(strcmp(buffer, "4") == 0){
					printf("Saliendo . . .\n");
					break;
				}
				if(strcmp(buffer, "1") == 0){
					bzero (&buffer, sizeof (buffer));
					bzero (&user_from_list, sizeof (user_from_list));
					bzero (&questions, sizeof (questions));
					bzero (&buffer2, sizeof (buffer2));
					bzero (&bufferquestions, sizeof (bufferquestions));
					recv(clientSocket, buffer, 1024, 0); //Recibe lista jugadores disponibles o un codigo de error
					user_from_list = buffer;
					while((user_from_list = strtok(user_from_list, separator)) != NULL){
						printf("%s\n", user_from_list);
						user_from_list = NULL;
					}
					bzero (&buffer, sizeof (buffer));
					printf("Seleccione el numero del usuario con el que desea iniciar una nueva partida\n");
					scanf("%s", &buffer[0]);
					send(clientSocket, buffer, strlen(buffer), 0); //Envia numero de usuario para partida al servidor
					//printf("Buffer1: %s\n", buffer);
					bzero (&buffer, sizeof (buffer));
					recv(clientSocket, buffer, 1024, 0); //Recibe las preguntas del servidor
					//printf("Buffer2: %s\n", buffer);
					questions = buffer;
					sprintf(questions, "%s", buffer);
					//bzero (&buffer, sizeof (buffer));
					//printf("QUESTIONS: %s\n", questions);
					while((questions = strtok(questions, separator)) != NULL){
						printf("%s\n", questions);
						questions = NULL;
						//bzero (&buffer2, sizeof (buffer2));
						valida:
						printf("Seleccione una opción: ");
						scanf("%s", &buffer2[0]);
						if((buffer2[0] - '0') > 3){
							printf("Debe ingresar una opcion válida\n");
							goto valida;
						}
						else{
						strcat(bufferquestions, buffer2);
						strcat(bufferquestions, "$");
						}
					}
					//printf("BUFFER LUEGO: %s\n", bufferquestions);
					send(clientSocket, bufferquestions, strlen(bufferquestions), 0); //Envia las respuestas al servidor
					bzero (&buffer, sizeof (buffer));
					printf("Espere a que el otro jugador responda\n");
					continue;
				}
				if(strcmp(buffer, "2") == 0){
					bzero (&buffer, sizeof (buffer));
					bzero (&user_from_list, sizeof (user_from_list));
					bzero (&questions, sizeof (questions));
					bzero (&buffer2, sizeof (buffer2));
					bzero (&bufferquestions, sizeof (bufferquestions));
					recv(clientSocket, buffer, 1024, 0); //Recibe lista jugadores
					user_from_list = buffer;
					if(strcmp(user_from_list, "-1") == 0){
						printf("No tiene partidas activas actualmente\n");
						continue;
					}
					while((user_from_list = strtok(user_from_list, separator)) != NULL){
						printf("%s\n", user_from_list);
						user_from_list = NULL;
					}
					bzero (&buffer, sizeof (buffer));
					printf("Seleccione el numero del usuario con el que desea continuar la partida\n");
					scanf("%s", &buffer[0]);
					send(clientSocket, buffer, strlen(buffer), 0); //Envia numero de usuario para partida al servidor
					char* puntos;
					bzero (&buffer, sizeof (buffer));
					recv(clientSocket, buffer, 1024, 0); //Recibe puntos jugadores
					puntos = buffer;
					if (strcmp(puntos,"-1") == 0){
						printf("Lo lamento, no esta en su turno de partida, por favor espere su turno...\n");
						continue;
					}
					else{
						while((puntos = strtok(puntos, separator)) != NULL){
						printf("%s\n", puntos);
						puntos = NULL;
						}
						continue;
					}
				}
				if(strcmp(buffer, "3") == 0){
					bzero (&buffer, sizeof (buffer));
					bzero (&user_from_list, sizeof (user_from_list));					
					recv(clientSocket, buffer, 1024, 0); //Recibe las estadisticas
					user_from_list = buffer;
					while((user_from_list = strtok(user_from_list, separator)) != NULL){
						printf("%s\n", user_from_list);
						user_from_list = NULL;
					}
					bzero (&buffer, sizeof (buffer));
					continue;
				}
			}			
		}
		else{
			while (1)
			{
				bzero (&buffer, sizeof (buffer));
				bzero (&user_from_list, sizeof (user_from_list));
				bzero (&questions, sizeof (questions));
				bzero (&buffer2, sizeof (buffer2));
				bzero (&bufferquestions, sizeof (bufferquestions));
				printf("\n* ** * MENU * ** *\n");
				printf("1. Iniciar una partida nueva\n");
				printf("2. Continuar partida\n");
				printf("3. Ver estadisticas\n");
				printf("4. Salir\n");
				scanf("%s", &buffer[0]);
				if((buffer[0] - '0') > 4){
					printf("Debe ingresar una opcion válida\n");
					continue;
				}
				send(clientSocket, buffer, strlen(buffer), 0); //Envia el numero de la opción
				if(strcmp(buffer, "4") == 0){
					printf("Saliendo . . .\n");
					break;
				}
				if(strcmp(buffer, "1") == 0){
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
					//printf("Buffer1: %s\n", buffer);
					bzero (&buffer, sizeof (buffer));
					recv(clientSocket, buffer, 1024, 0); //Recibe las preguntas del servidor
					//printf("Buffer2: %s\n", buffer);
					questions = buffer;
					sprintf(questions, "%s", buffer);
					//bzero (&buffer, sizeof (buffer));
					//printf("QUESTIONS: %s\n", questions);
					while((questions = strtok(questions, separator)) != NULL){
						printf("%s\n", questions);
						questions = NULL;
						valida2:
						printf("Seleccione una opción: ");
						scanf("%s", &buffer2[0]);
						if((buffer2[0] - '0') > 3){
							printf("Debe ingresar una opcion válida\n");
							goto valida2;
						}
						else{
							strcat(bufferquestions, buffer2);
							strcat(bufferquestions, "$");
						}
					}
					//printf("BUFFER LUEGO: %s\n", bufferquestions);
					send(clientSocket, bufferquestions, strlen(bufferquestions), 0); //Envia las respuestas al servidor
					bzero (&buffer, sizeof (buffer));
					printf("Espere a que el otro jugador responda\n");
					continue;
				}
				if(strcmp(buffer, "2") == 0){
					bzero (&buffer, sizeof (buffer));
					bzero (&user_from_list, sizeof (user_from_list));
					bzero (&questions, sizeof (questions));
					bzero (&buffer2, sizeof (buffer2));
					bzero (&bufferquestions, sizeof (bufferquestions));
					recv(clientSocket, buffer, 1024, 0); //Recibe lista jugadores
					user_from_list = buffer;
					if(strcmp(user_from_list, "-1") == 0){
						printf("No tiene partidas activas actualmente\n");
						continue;
					}
					while((user_from_list = strtok(user_from_list, separator)) != NULL){
						printf("%s\n", user_from_list);
						user_from_list = NULL;
					}
					bzero (&buffer, sizeof (buffer));
					printf("Seleccione el numero del usuario con el que desea continuar la partida\n");
					scanf("%s", &buffer[0]);
					send(clientSocket, buffer, strlen(buffer), 0); //Envia numero de usuario para partida al servidor
					char* puntos;
					bzero (&buffer, sizeof (buffer));
					recv(clientSocket, buffer, 1024, 0); //Recibe puntos jugadores
					puntos = buffer;
					if (strcmp(puntos,"-1") == 0){
						printf("Lo lamento, no esta en su turno de partida, por favor espere su turno...\n");
						continue;
					}
					else{
						while((puntos = strtok(puntos, separator)) != NULL){
							printf("%s\n", puntos);
							puntos = NULL;
						}
						continue;
					}
				}
				if(strcmp(buffer, "3") == 0){
					bzero (&buffer, sizeof (buffer));
					bzero (&user_from_list, sizeof (user_from_list));
					recv(clientSocket, buffer, 1024, 0); //Recibe las estadisticas
					user_from_list = buffer;
					while((user_from_list = strtok(user_from_list, separator)) != NULL){
						printf("%s\n", user_from_list);
						user_from_list = NULL;
					}
					bzero (&buffer, sizeof (buffer));
					continue;
				}
			}			
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