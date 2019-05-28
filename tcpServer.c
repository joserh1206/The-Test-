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
int getRival(int game, int actual);
char* checkAnswer(int id_game, int response2, int response, char* username);
void getNewQuestions(int id_game, char socket_com[1024], int newSocket, char response[4], char* answers, int id_player2);
int checkUsernamePassword(char *username, char *password);
int insertPlayerIntoDB(char *username, char *password);
void beginGame(char *username);
char* getTwoQuestionsLastPLayer(int game);
char* getAllGamesAndStatisticsForPlayer(int player);
char* getRightAndWrongPlayer(int player);
int getCorrectAnswer(int game, int question);
char* getPlayersToGame(char *username);
char* getPlayersToGame2(char *username);
char* getGamesInProcess(char* username);
char* getPointsGame(int id_game);
static int callback(void *NotUsed, int argc, char **argv, char **azColName);
void closeDatabase(sqlite3* db);
void updateGoodAnswerStatistics(int player, int game);
void updateBadAnswerStatistics(int player, int game);
void changeTurnGame(int id_game, int playerTurn);
int createStatistics(int player, int game);
int makeGame(char *player2, char *username);
int getNewIdGame();
int getActualIdGame(char *username);
int getQuestionId();
void insertPlayer(int id_game, int id_player1, int id_player2);
int insertQuestionToGame(int id_game, int id_question);
char* getQuestionData(int id_question);
void addMiss(int id_question);
void addSuccess(int id_question);
void addPoints(int id_game, int id_player, int value);
int getValueQuestion(int id_question);
int getGoodOption(int id_game, int id_question);
void setCorrectAnswer(int optionSelected, int id_game, int id_question);
int getTurnPlayer(int id_game);

sqlite3_stmt *stmt;

int main(){

	int sockfd, ret;
	struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	char buffer[1024];

	pid_t childpid;

	addr_size = sizeof(struct sockaddr_in);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Server Socket is created.\n");

	int on=1;
  setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

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
			printf("Newsocket: %d\n", newSocket);
			exit(1);
		}
		printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

		if((childpid = fork()) == 0){
			close(sockfd);
			char *user_id;
			while(1){
				const char separator[2] = "$";
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
				printf("Usuario y contraseña recibido -> %s\n", buffer);
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
							printf ("%s", players);
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
							//char* statistics = getRightAndWrongPlayer(id_player);
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
							printf("El usuario eligio 4\n");
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
								printf ("%s", players);
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
								printf("El usuario eligio 4\n");
								goto new_game;
							}
						}
					}
					else{
						printf("CHECK: %d\n", check);
						printf("No quiero ingresarlo a la base\n");
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
	return 0;
}

//Obtiene las ultimas dos preguntas respondidas
char* getTwoQuestionsLastPLayer(int game){
	sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	char *buffer;
  int rc;
	sprintf(sql, "select QuestionsPerGame.id_question from QuestionsPerGame where QuestionsPerGame.id_game = %d limit 2 offset (select count(*) from QuestionsPerGame where QuestionsPerGame.id_game = %d)-2;", game, game);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	//sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get questions: %s\n", sqlite3_errmsg(db));
    closeDatabase(db);
  }
  else
  {
		bzero(&out, sizeof(out));
		bzero(&buffer, sizeof(buffer));
		sprintf(out, "$");
		
		while(sqlite3_step(stmt) != SQLITE_DONE){
			strcat(out, sqlite3_column_text(stmt,0));
			strcat(out, "$");
		}

  }
	//printf("Out: %s\n", out);
	buffer = out;
	closeDatabase(db);
	return buffer;
}

//Obtener respuesta correct, recibe el id del juego y id de la pregunta
int getCorrectAnswer(int game, int question)
{
  sqlite3 *db = openDatabase();
  char sql[1024];
	int good_answer;
  int rc;
  sprintf(sql, "select QuestionsPerGame.good_option from QuestionsPerGame where QuestionsPerGame.id_game = %d and QuestionsPerGame.id_question = %d;", game, question);
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to select data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
    return -1;
  }
  else
  {
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			good_answer = sqlite3_column_int(stmt,0);
		}
  }
  closeDatabase(db);
	return(good_answer);
}

int getRival(int game, int actual)
{
  sqlite3 *db = openDatabase();
  char sql[1024];
	int player;
  int rc;
  sprintf(sql, "select Game.id_user from Game where Game.id_game = %d and not Game.id_user = %d;", game, actual);
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to select data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
    return -1;
  }
  else
  {
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			player = sqlite3_column_int(stmt,0);
		}
  }
  closeDatabase(db);
	return(player);
}

//Obtiene la suma de los acierto y los fallos de un jugador
char* getRightAndWrongPlayer(int player){
	sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	char *buffer;
  int rc;
	sprintf(sql, "select coalesce(sum(Statistics.good_answer), 0), coalesce(sum(Statistics.bad_answer), 0) from Statistics where Statistics.id_user = %d;", player);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get players: %s\n", sqlite3_errmsg(db));
    closeDatabase(db);
  }
  else
  {
		bzero(&out, sizeof(out));
		bzero(&buffer, sizeof(buffer));
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			strcat(out, "Total de preguntas acertadas: ");
			strcat(out, sqlite3_column_text(stmt,0));
			strcat(out, "\nTotal de preguntas fallidas: ");
			strcat(out, sqlite3_column_text(stmt,1));
		}
  }
	buffer = out;
	closeDatabase(db);
	return buffer;
}

//Obtiene todos los juegos de un usuario especifico y las respuestas buenas y malas
char* getAllGamesAndStatisticsForPlayer(int player){
	sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	char *buffer;
  int rc;
	printf("Entra en la funcion\n");
	sprintf(sql, "select Statistics.id_game, Statistics.good_answer, Statistics.bad_answer from Statistics where Statistics.id_user = %d;", player);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get players: %s\n", sqlite3_errmsg(db));
    closeDatabase(db);
  }
  else
  {
		bzero(&out, sizeof(out));
		bzero(&buffer, sizeof(buffer));
		sprintf(out, "$");
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			strcat(out, "Id del juego: ");
			strcat(out, sqlite3_column_text(stmt,0));
			strcat(out, " - Preguntas acertadas: ");
			strcat(out, sqlite3_column_text(stmt,1));
			strcat(out, " - Preguntas fallidas: ");
			strcat(out, sqlite3_column_text(stmt,2));
			strcat(out, "$");
		}
  }
	if(strcmp(out, "$") == 0){
		buffer = out;
	}
	else{
		strcat(out, getRightAndWrongPlayer(player));
		buffer = out;
	}
	closeDatabase(db);
	return buffer;
}

int createStatistics(int player, int game)
{
  sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "insert into Statistics (id_user, id_game, good_answer, bad_answer) values (%d,%d,0,0);", player, game);
  rc = sqlite3_exec(db, sql, callback, 0, 0);
  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
    return -1;
  }
  else
  {
		printf("Estadistica registrado correctamente\n");
  }
  closeDatabase(db);
  return 0;
}

int getTurnPlayer(int id_game){
	sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	int turn;
  int rc;
	sprintf(sql, "select distinct Game.turn from Game where Game.id_game = %d;", id_game);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get turn: %s\n", sqlite3_errmsg(db));
    closeDatabase(db);
  }
	  else
  {
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			turn = sqlite3_column_int(stmt,0);
		}
  }
  closeDatabase(db);
	return(turn);
}

void getNewQuestions(int id_game, char socket_com[1024], int newSocket, char response[4], char* answers, int id_player2){
	int id_question = getQuestionId();
	insertQuestionToGame(id_game, id_question);
	char* question1 = getQuestionData(id_question);
	printf("Pregunta 1: %s",question1);
	strcat(socket_com, question1);
	strcat(socket_com, "$");
	int id_question2 = getQuestionId();
	insertQuestionToGame(id_game, id_question2);
	char* question2 = getQuestionData(id_question2);
	printf("Pregunta 2: %s",question2);
	strcat(socket_com, question2);
	printf("Preguntas: %s",socket_com);
	send(newSocket, socket_com, strlen(socket_com), 0); //Se envian las 2 preguntas al usuario
	recv(newSocket, response, 1024, 0); //Recibe las respuestas del usuario para las 2 preguntas
	answers = response;
	setCorrectAnswer(answers[0]-'0',id_game,id_question);
	setCorrectAnswer(answers[2]-'0',id_game,id_question2);
	changeTurnGame(id_game, id_player2);
}

void updateGoodAnswerStatistics(int player, int game){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "update Statistics set good_answer = good_answer + 1 where Statistics.id_game = %d and Statistics.id_user = %d;", game, player);
	rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
  }
  else
  {
		printf("Se actualizo el campo de respuestas buenas\n");
  }
  closeDatabase(db);
}

void updateBadAnswerStatistics(int player, int game){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "update Statistics set bad_answer = bad_answer + 1 where Statistics.id_game = %d and Statistics.id_user = %d;", game, player);
	rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
  }
  else
  {
		printf("Se actualizo el campo de respuestas buenas\n");
  }
  closeDatabase(db);
}

char* getPointsGame(int id_game){
	sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	char *buffer;
  int rc;
	sprintf(sql, "select Users.username, Game.points from Game inner join Users on Users.id_user = Game.id_user where Game.id_game = %d;", id_game);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get players: %s\n", sqlite3_errmsg(db));
    closeDatabase(db);
  }
  else
  {
		bzero(&out, sizeof(out));
		bzero(&buffer, sizeof(buffer));
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			strcat(out, "Jugador: ");
			strcat(out, sqlite3_column_text(stmt,0));
			strcat(out, " - Puntos: ");
			strcat(out, sqlite3_column_text(stmt,1));
			strcat(out, "\n$");
		}
  }
	buffer = out;
	closeDatabase(db);
	return buffer;
}

void changeTurnGame(int id_game,int playerTurn){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "update Game set turn = %d where Game.id_game = %d;", playerTurn, id_game);
	rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
  }
  else
  {
		printf("Turno actualizado\n");
  }
  closeDatabase(db);
}

char* getGamesInProcess(char* username){
	sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	char *buffer;
  int rc;
	sprintf(sql, "select Game.id_game, Users.username from Users inner join Game on Users.id_user = Game.id_user where Game.id_game in (select distinct Game.id_game from Game where Game.id_user = (select Users.id_user from Users where Users.username = '%s')) and not Users.username = '%s';", username, username);
	printf("%s", sql);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get players: %s\n", sqlite3_errmsg(db));
    closeDatabase(db);
  }
  else
  {
		bzero(&out, sizeof(out));
		bzero(&buffer, sizeof(buffer));
		int counter = 0;
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			strcat(out, sqlite3_column_text(stmt,0));
			strcat(out, " - ");
			strcat(out, sqlite3_column_text(stmt,1));
			strcat(out, "$");
			counter++;
		}
		if(counter == 0){
			buffer = "-1";
		}
		else{
			buffer = out;
		}
  }
  closeDatabase(db);
	return(buffer);
}

char* getPlayersToGame(char *username)
{
  sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	char *buffer;
  int rc;
	sprintf(sql,"select * from Users where Users.id_user not in (select distinct Users.id_user from Users inner join Game on Users.id_user = Game.id_user where Game.id_game in (select Game.id_game from Game inner join Users on Game.id_user = Users.id_user and Users.username = '%s'))", username);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get players: %s\n", sqlite3_errmsg(db));
    closeDatabase(db);
  }
  else
  {
		bzero(&out, sizeof(out));
		bzero(&buffer, sizeof(buffer));
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			strcat(out, sqlite3_column_text(stmt,0));
			strcat(out, " - ");
			strcat(out, sqlite3_column_text(stmt,1));
			strcat(out, "$");
		}
		buffer = out;
  }
	
  closeDatabase(db);
	return(buffer);
}

char* getPlayersToGame2(char *username)
{
  sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	char *buffer;
  int rc;
	sprintf(sql,"select * from Users where not Users.username = '%s'", username);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get players: %s\n", sqlite3_errmsg(db));
    closeDatabase(db);
  }
  else
  {
		bzero(&out, sizeof(out));
		bzero(&buffer, sizeof(buffer));
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			strcat(out, sqlite3_column_text(stmt,0));
			strcat(out, " - ");
			strcat(out, sqlite3_column_text(stmt,1));
			strcat(out, "$");
		}
		buffer = out;
  }
	
  closeDatabase(db);
	return(buffer);
}

int checkUsernamePassword(char *username, char *password)
{
  sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "SELECT username, password FROM Users WHERE username = '%s' AND password = '%s';", username, password);
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
    return -1;
  }
  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW) 
  {
		closeDatabase(db);
    return 1;
  }
  else
  {
		closeDatabase(db);
    return 0;
  }
  closeDatabase(db);
  return 0;
}

int insertPlayerIntoDB(char *username, char *password)
{
  sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "INSERT INTO Users (username, password) VALUES ('%s', '%s');", username, password);
  rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
    return -1;
  }
  else
  {
		printf("Usuario registrado correctamente\n");
  }
  closeDatabase(db);
  return 1;
}

int makeGame(char *player2, char *username)
{
	int id = getNewIdGame();
	id++;
	
	int id_player1 = getActualIdGame(username);
	int id_player2 = atoi(player2);
	insertPlayer(id, id_player1, id_player2);
	return id;
}

void insertPlayer(int id_game, int id_player1, int id_player2){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "insert into Game (id_game,id_user,points,turn) values (%d, %d, 0, %d), (%d, %d, 0, %d);", id_game, id_player1, id_player1, id_game, id_player2, id_player1);
	rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
  }
  else
  {
		printf("Partida registrada correctamente\n");
  }
  closeDatabase(db);
}

int getActualIdGame(char *username)
{
  sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
	int newId = 0;
	sprintf(sql, "select Users.id_user from Users where Users.username = '%s'",username);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get players: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
  }
  else
  {
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			newId = sqlite3_column_int(stmt,0);
		}
  }
  closeDatabase(db);
	return(newId);
}

char* checkAnswer(int id_game, int response2, int response, char* username){
	int correct_answ = getCorrectAnswer(id_game, response2);
	if(correct_answ == response)
	{
		int id_player = getActualIdGame(username);
		addSuccess(response2);
		updateGoodAnswerStatistics(id_player, id_game);
		addPoints(id_game, id_player, getValueQuestion(response2));
		return "\n* *** * Felicidades acertó la pregunta * ** *\n";	
	}
	else
	{
		int id_player = getActualIdGame(username);
		addMiss(response2);
		updateBadAnswerStatistics(id_player, id_game);
		return "\nRespuesta incorrecta :(\n";
	}
}

int getQuestionId()
{
  sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
	int newId = 0;
	sprintf(sql,"select Questions.Id_question from Questions order by random() limit 1;");
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get players: %s\n", sqlite3_errmsg(db));
  	closeDatabase(db);
  }
  else
  {
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			newId = sqlite3_column_int(stmt,0);
		}
  }
  closeDatabase(db);
	return(newId);
}

int insertQuestionToGame(int id_game, int id_question){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql,"insert into QuestionsPerGame (id_game,id_question,good_option) values (%d,%d,null);",id_game,id_question);
	rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
  }
  else
  {
		printf("Pregunta ingresada al juego\n");
  }
  closeDatabase(db);
	return id_game;
}

char* getQuestionData(int id_question)
{
	printf("Entra a questionsData con el id: %d\n", id_question);
  sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	char *buffer;
  int rc;
	int newId = 0;
	sprintf(sql, "select Questions.question, Questions.option1, Questions.option2, Questions.option3 from Questions where Questions.Id_question = %d",id_question);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get players: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
  }
  else
  {
		bzero(&out, sizeof(out));
		bzero(&buffer, sizeof(buffer));
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			strcat(out, sqlite3_column_text(stmt,0));
			strcat(out, "\n");
			strcat(out, "\t1 - ");
			strcat(out, sqlite3_column_text(stmt,1));
			strcat(out, "\n");
			strcat(out, "\t2 - ");
			strcat(out, sqlite3_column_text(stmt,2));
			strcat(out, "\n");
			strcat(out, "\t3 - ");
			strcat(out, sqlite3_column_text(stmt,3));
			strcat(out, "\n");
		}
		buffer = out;
		printf("Buffer data:\n %s\n", buffer);
  }
  closeDatabase(db);
	return(buffer);
}

void setCorrectAnswer(int optionSelected, int id_game, int id_question){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "update QuestionsPerGame set good_option = %d where QuestionsPerGame.id_game = %d and QuestionsPerGame.id_question = %d", optionSelected, id_game, id_question);
	rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
  }
  else
  {
		printf("Respuesta asignada a la pregunta\n");
  }
  closeDatabase(db);
}

int getGoodOption(int id_game, int id_question)
{
  sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
	int option = 0;
	sprintf(sql, "select QuestionsPerGame.good_option from QuestionsPerGame where QuestionsPerGame.id_question = %d and QuestionsPerGame.id_game = %d", id_game, id_question);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get players: %s\n", sqlite3_errmsg(db));
    
		closeDatabase(db);
  }
  else
  {
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			option = sqlite3_column_int(stmt,0);
		}
  }
	
  closeDatabase(db);
	return(option);
}

int getValueQuestion(int id_question){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
	int value = 0;
	sprintf(sql, "select Questions.value_of_question from Questions where Questions.Id_question = %d", id_question);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get players: %s\n", sqlite3_errmsg(db));
    
		closeDatabase(db);
  }
  else
  {
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			value = sqlite3_column_int(stmt,0);
		}
  }
  closeDatabase(db);
	return(value);
}

void addPoints(int id_game, int id_player, int value){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "update Game set points = points + %d where Game.id_game = %d and Game.id_user = %d", value, id_game, id_player);
	rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
  }
  else
  {
		printf("Puntos actualizados al jugador\n");
  }
  closeDatabase(db);
}

void addSuccess(int id_question){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "update Questions set correct = correct + 1 where Questions.Id_question = %d", id_question);
	rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
  }
  else
  {
		printf("Aciertos de la pregunta aumentado\n");
  }
  closeDatabase(db);
}

void addMiss(int id_question)
{
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "update Questions set incorrect = incorrect + 1 where Questions.Id_question = %d", id_question);
	rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
  }
  else
  {
		printf("Errores de la pregunta aumentado\n");
  }
  closeDatabase(db);
}

int getNewIdGame()
{
  sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
	int newId = 0;
	sprintf(sql, "select coalesce(max(id_game),0) from Game");
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get players: %s\n", sqlite3_errmsg(db));
    
  	closeDatabase(db);
  }
  else
  {
		printf("Entra al else\n");
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			printf("COLUMN: %d\n", sqlite3_column_int(stmt,0));
			newId = sqlite3_column_int(stmt,0);
			printf("NEWID: %d\n", newId);
		}
  }
  closeDatabase(db);
	return(newId);
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i < argc*2; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    return 0;
}

void closeDatabase(sqlite3* db) { 
	sqlite3_finalize(stmt);
	sqlite3_close(db); 
}

sqlite3* openDatabase()
{
	int rc;
  	sqlite3 *db;
  	rc = sqlite3_open("DatabaseTheTest.db", &db);
	if(rc){
		fprintf(stderr, "Cant open database %s\n", sqlite3_errmsg(db));
	}
	else{
		fprintf(stderr, "BD Open!\n");
	}
  	return db;
}