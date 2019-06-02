#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "titles.c"

#define PORT 4442
//#define IP_ADDR "172.19.127.63"
#define IP_ADDR "127.0.0.1"

int main(){

	int clientSocket, ret;
	struct sockaddr_in serverAddr;

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	//memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr(IP_ADDR);

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Server.\n");

	while(1){
		
		system("clear");
		const char separator[2] = "$";
		char buffer[1024];
		char buffer2[1024];
		char bufferquestions[1024];
		char *user_from_list, *questions, *answers;
		bzero (&buffer, sizeof (buffer));
		bzero (&buffer2, sizeof (buffer2));
		bzero (&bufferquestions, sizeof (bufferquestions));
		printClientTitle();
		printf("Ingrese su nombre de usuario:\n");
		scanf("%s", &buffer[0]);
		
		strcat(buffer, "$");

		printf("Ingrese su contrasenia:\n");
		scanf("%s", &buffer2[0]);
		
		strcat(buffer, buffer2);
		send(clientSocket, buffer, strlen(buffer), 0); //Envia el usuario+contrasenia

		bzero (&buffer, sizeof (buffer));
		system("clear");

		recv(clientSocket, buffer, 1024, 0); //Si existe recibe lista de usuarios disponibles, si no recibe -1
		if(strcmp(buffer, "-1") == 0){
			printf("\nEl usuario ingresado no se encuentra en la BD, desea agregarlo? (S/N):\n");
			scanf("%s", &buffer[0]);
			send(clientSocket, buffer, strlen(buffer), 0); //Envia respuesta al servidor
			if(strcmp(buffer, "N") == 0 || strcmp(buffer, "n") == 0){
				printf("\nEstá bien, sin rencores...\n");
				sprintf(buffer, "#");
				send(clientSocket, buffer, strlen(buffer), 0); //Envia las respuestas al servidor
				sleep(1);
				continue;
			}
			bzero (&buffer, sizeof (buffer));
			recv(clientSocket, buffer, 1024, 0);
			if(strcmp(buffer, "Ok") == 0){
				printf("\n -+- Usuario registrado, BIENVENIDO -+- \n");
			}else{
				printf("\nHa ocurrido un problema inesperado. Intentelo de nuevo\n");
				continue;
			}
			while (1)
			{
				system("clear");
				printClientTitle();
				bzero (&buffer, sizeof (buffer));
				printf("\n* ** * MENU * ** *\n");
				printf("1. Iniciar una partida nueva\n");
				printf("2. Continuar partida\n");
				printf("3. Ver estadisticas\n");
				printf("4. Salir\n");
				printf("Seleccione una opción: ");
				scanf("%s", &buffer[0]);
				if((buffer[0] - '0') > 4){
					printf("Debe ingresar una opcion válida\n");
					continue;
				}
				send(clientSocket, buffer, strlen(buffer), 0); //Envia el numero de la opción
				if(strcmp(buffer, "4") == 0){
					printf("Saliendo");
					fflush(stdout);
					sleep(0.5);
					printf(" .");
					fflush(stdout);
					sleep(1);
					printf(" .");
					fflush(stdout);
					sleep(1);
					printf(" .");
					fflush(stdout);
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
					printf("\n*-* - *-+-* - *-* Lista de jugadores *-* - *-+-* - *-*\n");
					while((user_from_list = strtok(user_from_list, separator)) != NULL){
						printf("%s\n", user_from_list);
						user_from_list = NULL;
					}
					bzero (&buffer, sizeof (buffer));
					printf("Seleccione el numero del usuario con el que desea iniciar una nueva partida\n");
					scanf("%s", &buffer[0]);
					send(clientSocket, buffer, strlen(buffer), 0); //Envia numero de usuario para partida al servidor
					bzero (&buffer, sizeof (buffer));
					recv(clientSocket, buffer, 1024, 0); //Recibe las preguntas del servidor
					questions = buffer;
					sprintf(questions, "%s", buffer);
					while((questions = strtok(questions, separator)) != NULL){
						printf("%s\n", questions);
						questions = NULL;
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
					send(clientSocket, bufferquestions, strlen(bufferquestions), 0); //Envia las respuestas al servidor
					bzero (&buffer, sizeof (buffer));
					printf("\n-> Por favor espere a que el otro jugador responda <-\n");
					
					sprintf(buffer, "#");
					send(clientSocket, buffer, strlen(buffer), 0); //Envia las respuestas al servidor
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
						printf("\n* ** * No tiene partidas activas actualmente * ** *\n");
						fflush(stdout);
						sleep(2);
						sprintf(buffer, "#");
						send(clientSocket, buffer, strlen(buffer), 0); //Envia las respuestas al servidor
						continue;
					}
					printf("\n* ** *Partidas activas* ** *\n");
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
						bzero (&buffer, sizeof (buffer));
						printf("\nLo lamento, no esta en su turno, por favor espere...\n");
						sprintf(buffer, "#");
						send(clientSocket, buffer, strlen(buffer), 0); //Envia las respuestas al servidor
						continue;
					}
					else{
						while((puntos = strtok(puntos, separator)) != NULL){
							printf("%s\n", puntos);
							puntos = NULL;
						}
						sprintf(buffer, "&");
						send(clientSocket, buffer, strlen(buffer), 0); //Envia acuse de recibido
						bzero (&buffer, sizeof (buffer));
						recv(clientSocket, buffer, 1024, 0); //Recibe las preguntas
						char* preguntas;
						preguntas = buffer;
						while((preguntas = strtok(preguntas, separator)) != NULL){
							bzero (&buffer2, sizeof (buffer2));
							sprintf(buffer2, "%s", preguntas);
							send(clientSocket, buffer2, strlen(buffer), 0); //Envia pregunta 
							recv(clientSocket, buffer2, 1024, 0); //Recibe la data de la pregunta
							printf("Pregunta:\n %s\n", buffer2);
							valida3_1:
							bzero (&buffer2, sizeof (buffer2));
							printf("Respuesta: ");
							scanf("%s", &buffer2[0]);
							if((buffer2[0] - '0') > 3){
								printf("Debe ingresar una opcion válida\n");
								goto valida3_1;
							}
							send(clientSocket, buffer2, strlen(buffer), 0); //Envia respuesta
							recv(clientSocket, buffer2, 1024, 0); //Recibe acuse de recibido
							printf("%s", buffer2);
							preguntas = NULL;
						}
						sprintf(buffer, "&");
						send(clientSocket, buffer, strlen(buffer), 0); //Envia acuse de recibido
						bzero(&buffer, sizeof (buffer));
						recv(clientSocket, buffer, 1024, 0); //Recibe las preguntas del servidor
						printf("\n*-*-*-*-Nuevas preguntas-*-*-*-*\n");
						questions = buffer;
						sprintf(questions, "%s", buffer);
						while((questions = strtok(questions, separator)) != NULL){
							printf("%s\n", questions);
							questions = NULL;
							valida2_2:
							printf("Seleccione una opción: ");
							scanf("%s", &buffer2[0]);
							if((buffer2[0] - '0') > 3){
								printf("Debe ingresar una opcion válida\n");
								goto valida2_2;
							}
							else{
								strcat(bufferquestions, buffer2);
								strcat(bufferquestions, "$");
							}
						}
						send(clientSocket, bufferquestions, strlen(bufferquestions), 0); //Envia las respuestas al servidor
						bzero (&buffer, sizeof (buffer));
						printf("\n-> Por favor espere a que el otro jugador responda <-\n");
						bzero (&buffer, sizeof (buffer));
						sprintf(buffer, "#");
						send(clientSocket, buffer, strlen(buffer), 0); //Envia las respuestas al servidor
						continue;
					}
				}
				if(strcmp(buffer, "3") == 0){
					bzero (&buffer, sizeof (buffer));
					bzero (&user_from_list, sizeof (user_from_list));					
					recv(clientSocket, buffer, 1024, 0); //Recibe las estadisticas
					if(strcmp(buffer, "$") == 0){
						printf("\n* ** * Primero debe jugar una partida :) * ** *\n");
						fflush(stdout);
						sleep(2);
						bzero(&buffer, sizeof (buffer));
						sprintf(buffer, "#");
						send(clientSocket, buffer, strlen(buffer), 0); //Envia las respuestas al servidor						
						continue;
					}
					user_from_list = buffer;
					printf("\n* ** * Sus estadisticas * ** *\n");
					while((user_from_list = strtok(user_from_list, separator)) != NULL){
						printf("%s\n", user_from_list);
						user_from_list = NULL;
					}					
					printf("Presione cualquier tecla para continuar. . .\n");
					getchar();
					getchar();
					bzero (&buffer, sizeof (buffer));
					sprintf(buffer, "#");
					send(clientSocket, buffer, strlen(buffer), 0); //Envia las respuestas al servidor
					continue;
				}
			}			
		}
		else{
			while (1)
			{
				system("clear");
				printClientTitle();
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
				printf("Seleccione una opción: ");
				scanf("%s", &buffer[0]);
				if((buffer[0] - '0') > 4){
					printf("Debe ingresar una opcion válida\n");
					continue;
				}
				send(clientSocket, buffer, strlen(buffer), 0); //Envia el numero de la opción
				if(strcmp(buffer, "4") == 0){
					printf("Saliendo");
					fflush(stdout);
					sleep(0.5);
					printf(" .");
					fflush(stdout);
					sleep(1);
					printf(" .");
					fflush(stdout);
					sleep(1);
					printf(" .");
					fflush(stdout);	
					sleep(1);
					break;
				}
				if(strcmp(buffer, "1") == 0){
					bzero (&buffer, sizeof (buffer));
					recv(clientSocket, buffer, 1024, 0); //Recibe lista jugadores disponibles
					user_from_list = buffer;
					printf("\n*-* - *-+-* - *-* Lista de jugadores *-* - *-+-* - *-*\n");
					while((user_from_list = strtok(user_from_list, separator)) != NULL){
						printf("%s\n", user_from_list);
						user_from_list = NULL;
					}
					bzero (&buffer, sizeof (buffer));
					printf("Seleccione el numero del usuario con el que desea iniciar una nueva partida\n");
					scanf("%s", &buffer[0]);
					send(clientSocket, buffer, strlen(buffer), 0); //Envia numero de usuario para partida al servidor
					bzero (&buffer, sizeof (buffer));
					recv(clientSocket, buffer, 1024, 0); //Recibe las preguntas del servidor
					questions = buffer;
					sprintf(questions, "%s", buffer);
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
					send(clientSocket, bufferquestions, strlen(bufferquestions), 0); //Envia las respuestas al servidor
					bzero (&buffer, sizeof (buffer));
					printf("Espere a que el otro jugador responda\n");
					sprintf(buffer, "#");
					send(clientSocket, buffer, strlen(buffer), 0); //Envia las respuestas al servidor
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
						printf("\n* ** * No tiene partidas activas actualmente * ** *\n");
						fflush(stdout);
						sleep(2);
						bzero (&buffer, sizeof (buffer));
						sprintf(buffer, "#");
						send(clientSocket, buffer, strlen(buffer), 0); //Envia las respuestas al servidor
						continue;
					}
					printf("\n* ** *Partidas activas* ** *\n");
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
						bzero (&buffer, sizeof (buffer));
						printf("\nLo lamento, no esta en su turno, por favor espere...\n");
						sprintf(buffer, "#");
						send(clientSocket, buffer, strlen(buffer), 0); //Envia mensaje especial
						continue;
					}
					else{
						while((puntos = strtok(puntos, separator)) != NULL){
							printf("%s\n", puntos);
							puntos = NULL;
						}
						sprintf(buffer, "&");
						send(clientSocket, buffer, strlen(buffer), 0); //Envia acuse de recibido
						char* answ_p2;
						bzero (&buffer, sizeof (buffer));
						recv(clientSocket, buffer, 1024, 0); //Recibe preguntas resueltas por jugador oponente
						answ_p2 = buffer;
						if(strcmp(buffer, "&") != 0){
							printf("\n** * ** *** Respuestas del oponente *** ** * **\n");
							while((answ_p2 = strtok(answ_p2, separator)) != NULL){
								printf("\n%s\n", answ_p2);
								answ_p2 = NULL;
							}
							printf("\n** * ** *** **** ***** **** *** ** * **\n");
						}
						bzero (&buffer, sizeof (buffer));
						sprintf(buffer, "&");
						send(clientSocket, buffer, strlen(buffer), 0); //Envia acuse de recibido
						bzero (&buffer, sizeof (buffer));
						recv(clientSocket, buffer, 1024, 0); //Recibe las preguntas
						char* preguntas;
						preguntas = buffer;
						while((preguntas = strtok(preguntas, separator)) != NULL){
							bzero (&buffer2, sizeof (buffer2));
							sprintf(buffer2, "%s", preguntas);
							send(clientSocket, buffer2, strlen(buffer), 0); //Envia pregunta 
							recv(clientSocket, buffer2, 1024, 0); //Recibe la data de la pregunta
							printf("Pregunta:\n %s\n", buffer2);
							valida3:
							bzero (&buffer2, sizeof (buffer2));
							printf("Respuesta: ");
							scanf("%s", &buffer2[0]);
							if((buffer2[0] - '0') > 3){
								printf("Debe ingresar una opcion válida\n");
								goto valida3;
							}
							send(clientSocket, buffer2, strlen(buffer), 0); //Envia respuesta
							recv(clientSocket, buffer2, 1024, 0); //Recibe acuse de recibido
							printf("%s", buffer2);
							preguntas = NULL;
						}
						sprintf(buffer, "&");
						send(clientSocket, buffer, strlen(buffer), 0); //Envia acuse de recibido
						bzero(&buffer, sizeof (buffer));
						recv(clientSocket, buffer, 1024, 0); //Recibe las preguntas del servidor
						printf("\n*-*-*-*-Nuevas preguntas-*-*-*-*\n");
						questions = buffer;
						sprintf(questions, "%s", buffer);
						while((questions = strtok(questions, separator)) != NULL){
							printf("%s\n", questions);
							questions = NULL;
							valida2_1:
							printf("Seleccione una opción: ");
							scanf("%s", &buffer2[0]);
							if((buffer2[0] - '0') > 3){
								printf("Debe ingresar una opcion válida\n");
								goto valida2_1;
							}
							else{
								strcat(bufferquestions, buffer2);
								strcat(bufferquestions, "$");
							}
						}
						send(clientSocket, bufferquestions, strlen(bufferquestions), 0); //Envia las respuestas al servidor
						bzero (&buffer, sizeof (buffer));
						printf("Espere a que el otro jugador responda\n");
						bzero (&buffer, sizeof (buffer));
						sprintf(buffer, "#");
						send(clientSocket, buffer, strlen(buffer), 0); //Envia las respuestas al servidor
						continue;
					}
				}
				if(strcmp(buffer, "3") == 0){
					bzero (&buffer, sizeof (buffer));
					bzero (&user_from_list, sizeof (user_from_list));
					recv(clientSocket, buffer, 1024, 0); //Recibe las estadisticas
					if(strcmp(buffer, "$") == 0){
						printf("\n* ** * Primero debe jugar una partida :) * ** *\n");
						fflush(stdout);
						sleep(2);
						bzero (&buffer, sizeof (buffer));
						sprintf(buffer, "#");
						send(clientSocket, buffer, strlen(buffer), 0); //Envia las respuestas al servidor						
						continue;
					}
					user_from_list = buffer;
					printf("\n* ** * Sus estadisticas * ** *\n");
					while((user_from_list = strtok(user_from_list, separator)) != NULL){
						printf("%s\n", user_from_list);
						user_from_list = NULL;
					}
					printf("Presione cualquier tecla para continuar. . .\n");					
					getchar();
					getchar();
					bzero (&buffer, sizeof (buffer));
					sprintf(buffer, "#");
					send(clientSocket, buffer, strlen(buffer), 0); //Envia las respuestas al servidor
					continue;
				}
			}			
		}
	}

	return 0;
}