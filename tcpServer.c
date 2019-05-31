#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sqlite3.h>

#include "header.h"
#include "lastTwoQuestions.c"
#include "getCorrectAnswer.c"
#include "getRival.c"
#include "getRightAndWrongPlayer.c"
#include "getAllGamesAndStatisticsForPlayer.c"
#include "createStatistics.c"
#include "getTurnPlayer.c"
#include "getNewQuestions.c"
#include "updateGoodAnswerStatistics.c"
#include "updateBadAnswerStatistics.c"
#include "getPointsGame.c"
#include "changeTurnGame.c"
#include "getGamesInProcess.c"
#include "getPlayersToGame.c"
#include "checkUsernamePassword.c"
#include "insertPlayerIntoDB.c"
#include "makeGame.c"
#include "insertQuestionToGame.c"
#include "getValueQuestion.c"
#include "callback.c"
#include "dataBase.c"
#include "titles.c"

int main(){
	char buffer_menu[1024];
	printServerTitle();
	bzero (&buffer_menu, sizeof (buffer_menu));
	printf("-+ + -- ++ -- ++ Menu ++ -- ++ -- + -+\n");
	printf("1. Iniciar el servidor\n");
	printf("2. Eliminar preguntas\n");
	printf("3. Agregar preguntas\n");
	printf("4. Modificar valores de preguntas\n");
	printf("5. Ver estadísticas\n");
	printf("Seleccione una opción: ");
	scanf("%s", &buffer_menu[0]);

	if(strcmp(buffer_menu, "1") == 0){
		system("clear");
		printServerTitle();
		int sockfd, ret;
		struct sockaddr_in serverAddr;

		int newSocket;
		struct sockaddr_in newAddr;

		socklen_t addr_size;

		pid_t childpid;

		addr_size = sizeof(struct sockaddr_in);

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if(sockfd < 0){
			printf("\n[-]Error in connection.\n");
			exit(1);
		}
		printf("\n[+]Servidor The Test listo para jugar.\n");

		int on=1;
		setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

		memset(&serverAddr, '\0', sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(PORT);
		serverAddr.sin_addr.s_addr = inet_addr(IP_ADDR);

		ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
		if(ret < 0){
			printf("[-]Error in binding.\n");
			exit(1);
		}
		printf("\[+]Puerto -> %d\n", PORT);

		if(listen(sockfd, 10) == 0){
			printf("[+]Esperando....\n");
		}else{
			printf("[-]Error in binding.\n");
		}
		while(1){
			newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
			if(newSocket < 0){
				printf("Socket: %d\n", newSocket);
				exit(1);
			}
			printf("Nueva conexion recibida: %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

			if((childpid = fork()) == 0){
				close(sockfd);
				char *user_id;
				while(1){
					const char separator[2] = "$";
					char buffer[1024];
					char *word, *type, *username, *password, *buffer2, *answers;
					char response[4], socket_com[1024];
					int check = 0;
					new_game:
					bzero (&username, sizeof (username));
					bzero (&password, sizeof (password));
					bzero (&buffer, sizeof (buffer));

					recv(newSocket, buffer, 1024, 0); //Recibe usuario+contrasenia
					username = strtok(buffer, separator);
					password = strtok(NULL, separator);
					check = checkUsernamePassword(username, password);

					if(check){
						sprintf(response, "1");
						send(newSocket, response, strlen(response), 0); //Envia verificacion al cliente
						
						ciclo:
						bzero (&response, sizeof (response));
						recv(newSocket, response, 1024, 0); //Espera por opcion del menu
						if(strcmp(response, "1") == 0){
							bzero(&socket_com, sizeof(socket_com));
							printf("El usuario eligió 1\n");
							if (getNewIdGame() == 0){
								buffer2 = getPlayersToGame2(username);
							}
							else{
								buffer2 = getPlayersToGame(username);
							}
							sprintf(socket_com, "%s", buffer2);
							send(newSocket, socket_com, strlen(socket_com), 0); //Envia jugadores disponibles
							bzero (&response, sizeof (response));
							recv(newSocket, response, 1024, 0); //Recibe numero del usuario para una nueva partida
							int id_player2 = atoi(response);
							int id_game = makeGame(response, username);
							createStatistics(getActualIdGame(username),id_game);
							createStatistics(id_player2,id_game);
							while(1){
								bzero (&response, sizeof (response));
								bzero(&socket_com, sizeof(socket_com));
								getNewQuestions(id_game, socket_com, newSocket, response, answers, id_player2);
								bzero (&response, sizeof (response));
								recv(newSocket, response, 1024, 0); //Por ahora para que no se cicle
								if(strcmp(response, "#") == 0){
									goto ciclo;
								}	
							}
						}
							if(strcmp(response, "2") == 0){
								bzero(&socket_com, sizeof(socket_com));
								printf("El usuario eligio 2\n");
								char* players = getGamesInProcess(username);
								sprintf(socket_com, "%s", players);
								send(newSocket, socket_com, strlen(socket_com), 0); //Envia jugadores disponibles
								bzero (&response, sizeof (response));
								recv(newSocket, response, 1024, 0); //Recibe numero del juego para continuar la partida
								int id_game = atoi(response);
								if (getActualIdGame(username) == getTurnPlayer(id_game)){
									char* points = getPointsGame(id_game);
									bzero (&socket_com, sizeof (socket_com));
									sprintf(socket_com, "%s", points);
									char* questions = getTwoQuestionsLastPLayer(id_game);
									char* data;
									char response2[4];
									send(newSocket, socket_com, strlen(socket_com), 0);//Envia los usuarios y sus puntos								
									bzero(&response, sizeof (response));
									recv(newSocket, response, 1024, 0); 
									send(newSocket, questions, strlen(questions), 0); //Envia la preguntas
									bzero(&response2, sizeof (response2));
									recv(newSocket, response2, 1024, 0); //Recibe pregunta 1
									printf("\nPregunta 1 -> %s\n", response2);
									data = getQuestionData(atoi(response2));
									send(newSocket, data, strlen(data), 0); //Envia la data de la pregunta
									bzero(&response, sizeof (response));
									recv(newSocket, response, 1024, 0); //Recibe respuesta de la 1
									bzero (&socket_com, sizeof (socket_com));
									sprintf(socket_com, "%s", checkAnswer(id_game, atoi(response2), atoi(response), username));
									send(newSocket, socket_com, strlen(socket_com), 0);//Envia acuse de recibido
									//********************************************************************************
									bzero(&response2, sizeof (response2));
									bzero(&response, sizeof (response));
									recv(newSocket, response2, 1024, 0); //Recibe pregunta 2
									printf("\nPregunta 2 -> %s\n", response2);
									data = getQuestionData(atoi(response2));
									send(newSocket, data, strlen(data), 0); //Envia la data de la pregunta
									bzero(&response, sizeof (response));
									recv(newSocket, response, 1024, 0); //Recibe respuesta de la 2
									printf("\nRespuesta 2 -> %s\n", response);
									bzero (&socket_com, sizeof (socket_com));
									sprintf(socket_com, "%s", checkAnswer(id_game, atoi(response2), atoi(response), username));
									send(newSocket, socket_com, strlen(socket_com), 0);//Envia acuse de recibido
									bzero(&response, sizeof (response));
									bzero(&socket_com, strlen(socket_com));
									recv(newSocket, response, 1024, 0); //Recibe acuse de recibido
									int id_player2 = getRival(id_game, getActualIdGame(username));
									getNewQuestions(id_game, socket_com, newSocket, response, answers, id_player2);
									changeTurnGame(id_game, id_player2);
								}else{
									char* mensaje = "-1";
									bzero (&socket_com, sizeof (socket_com));
									sprintf(socket_com, "%s", mensaje);
									send(newSocket, socket_com, strlen(socket_com), 0); //Envia codigo de error
									bzero(&response, sizeof (response));
									recv(newSocket, response, 1024, 0); 
									if(strcmp(response, "#") == 0){
										goto ciclo;
									}	
								}
								bzero(&response, sizeof (response));
								recv(newSocket, response, 1024, 0); 
								if(strcmp(response, "#") == 0){
									goto ciclo;
								}	
							}
							if(strcmp(response, "3") == 0){
								bzero(&socket_com, sizeof(socket_com));
								printf("El usuario eligio 3\n");
								bzero(&response, sizeof (response));
								int id_player = getActualIdGame(username);
								char* statistics = getAllGamesAndStatisticsForPlayer(id_player);
								bzero (&socket_com, sizeof (socket_com));
								sprintf(socket_com, "%s", statistics);
								send(newSocket, socket_com, strlen(socket_com), 0);
								recv(newSocket, response, 1024, 0); 
								if(strcmp(response, "#") == 0){
									goto ciclo;
								}	
							}
							if(strcmp(response, "4") == 0){
								//estadisticas();
								printf("El usuario %s cerró la sesión\n", username);
								goto new_game;
							}
						}
					else
					{
						printf("El usuario no existe\n");
						sprintf(response, "-1");
						send(newSocket, response, strlen(response), 0); //Pregunta si ingresarlo a la BD
						bzero (&response, sizeof (response));
						recv(newSocket, response, 1024, 0); //Recibe respuesta usuario respecto a BD
						if(strcmp(response, "S") == 0 || strcmp(response, "s") == 0){
							check = insertPlayerIntoDB(username, password);
							if (check){
								bzero (&response, sizeof (response));
								sprintf(response, "Ok");
								send(newSocket, response, strlen(response), 0);
								ciclo2:
								bzero (&response, sizeof (response));
								recv(newSocket, response, 1024, 0); //Espera por opcion del menu
								if(strcmp(response, "1") == 0){
									printf("El usuario eligió 1\n");
									if (getNewIdGame() == 0){
										buffer2 = getPlayersToGame2(username);
									}
									else{
										buffer2 = getPlayersToGame(username);
									}
									sprintf(socket_com, "%s", buffer2);
									send(newSocket, socket_com, strlen(socket_com), 0); //Envia jugadores disponibles
									bzero (&response, sizeof (response));
									recv(newSocket, response, 1024, 0); //Recibe numero del usuario para una nueva partida
									int id_player2 = atoi(response);
									int id_game = makeGame(response, username);
									createStatistics(getActualIdGame(username),id_game);
									createStatistics(id_player2,id_game);
									while(1){
										bzero(&response, sizeof (response));
										bzero(&socket_com, sizeof(socket_com));
										getNewQuestions(id_game, socket_com, newSocket, response, answers, id_player2);
										bzero(&response, sizeof (response));
										recv(newSocket, response, 1024, 0); 
										if(strcmp(response, "#") == 0){
											goto ciclo2;
										}	
									}
								}
								if(strcmp(response, "2") == 0){
									bzero(&socket_com, sizeof(socket_com));
									printf("El usuario eligio 2\n");
									char* players = getGamesInProcess(username);
									sprintf(socket_com, "%s", players);
									send(newSocket, socket_com, strlen(socket_com), 0); //Envia jugadores disponibles
									bzero (&response, sizeof (response));
									recv(newSocket, response, 1024, 0); //Recibe numero del juego para continuar la partida
									int id_game = atoi(response);
									if (getActualIdGame(username) == getTurnPlayer(id_game)){
										char* points = getPointsGame(id_game);
										bzero (&socket_com, sizeof (socket_com));
										sprintf(socket_com, "%s", points);
										char* questions = getTwoQuestionsLastPLayer(id_game);
										char* data;
										char response2[4];
										//sprintf("Ultimas dos preguntas: %s\n", questions);
										send(newSocket, socket_com, strlen(socket_com), 0);//Envia los usuarios y sus puntos								
										bzero(&response, sizeof (response));
										recv(newSocket, response, 1024, 0); 
										send(newSocket, questions, strlen(questions), 0); //Envia la preguntas
										bzero(&response2, sizeof (response2));
										recv(newSocket, response2, 1024, 0); //Recibe pregunta 1
										printf("\nPregunta 1 -> %s\n", response2);
										data = getQuestionData(atoi(response2));
										send(newSocket, data, strlen(data), 0); //Envia la data de la pregunta
										bzero(&response, sizeof (response));
										recv(newSocket, response, 1024, 0); //Recibe respuesta de la 1
										bzero (&socket_com, sizeof (socket_com));
										sprintf(socket_com, "%s", checkAnswer(id_game, atoi(response2), atoi(response), username));
										send(newSocket, socket_com, strlen(socket_com), 0);//Envia acuse de recibido
										//********************************************************************************
										bzero(&response2, sizeof (response2));
										bzero(&response, sizeof (response));
										recv(newSocket, response2, 1024, 0); //Recibe pregunta 2
										printf("\nPregunta 2 -> %s\n", response2);
										data = getQuestionData(atoi(response2));
										send(newSocket, data, strlen(data), 0); //Envia la data de la pregunta
										bzero(&response, sizeof (response));
										recv(newSocket, response, 1024, 0); //Recibe respuesta de la 2
										printf("\nRespuesta 2 -> %s\n", response);
										bzero (&socket_com, sizeof (socket_com));
										sprintf(socket_com, "%s", checkAnswer(id_game, atoi(response2), atoi(response), username));
										send(newSocket, socket_com, strlen(socket_com), 0);//Envia acuse de recibido
										bzero(&response, sizeof (response));
										bzero(&socket_com, strlen(socket_com));
										recv(newSocket, response, 1024, 0); //Recibe acuse de recibido
										int id_player2 = getRival(id_game, getActualIdGame(username));
										getNewQuestions(id_game, socket_com, newSocket, response, answers, id_player2);
										changeTurnGame(id_game, id_player2);
									}
									else
									{
										char* mensaje = "-1";
										bzero (&socket_com, sizeof (socket_com));
										sprintf(socket_com, "%s", mensaje);
										send(newSocket, socket_com, strlen(socket_com), 0); //Envia codigo de error
										bzero(&response, sizeof (response));
										recv(newSocket, response, 1024, 0); 
										if(strcmp(response, "#") == 0){
											goto ciclo;
										}	
									}
									bzero(&response, sizeof (response));
									recv(newSocket, response, 1024, 0); 
									if(strcmp(response, "#") == 0){
										goto ciclo;
									}	
								}
								if(strcmp(response, "3") == 0){
									printf("El usuario eligio 3\n");
									bzero (&response, sizeof (response));
									int id_player = getActualIdGame(username);
									//char* statistics = getRightAndWrongPlayer(id_player);
									char* statistics = getAllGamesAndStatisticsForPlayer(id_player);
									bzero (&socket_com, sizeof (socket_com));
									sprintf(socket_com, "%s", statistics);
									send(newSocket, socket_com, strlen(socket_com), 0);
									recv(newSocket, response, 1024, 0); 
									if(strcmp(response, "#") == 0){
										goto ciclo2;
									}	
								}
								if(strcmp(response, "4") == 0){
									printf("El usuario %s cerró la sesión\n", username);
									goto new_game;
								}
							}
						}
						else{
							printf("No quiere ingresarlo a la base\n");
							bzero (&response, sizeof (response));
							recv(newSocket, response, 1024, 0);
							if(strcmp(response, "#") == 0){
								goto new_game;
							}	
						}
					}
					bzero(&buffer, sizeof(buffer));
				}
			}
		}
		close(newSocket);
	}
	else{
		printf("%s\n", buffer_menu);
	}
	return 0;
}