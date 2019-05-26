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
int checkUsernamePassword(char *username, char *password);
int insertPlayerIntoDB(char *username, char *password);
void beginGame(char *username);
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
					bzero (&response, sizeof (response));
					recv(newSocket, response, 1024, 0); //Espera por opcion del menu
					printf("Response del usuario: %s\n", response);
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
							bzero (&response, sizeof (response));
							bzero(&socket_com, sizeof(socket_com));
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
							bzero(&response, sizeof (response));
							recv(newSocket, response, 1024, 0); //Por ahora para que no se cicle	
							//2. Al entrar a este while hay que verificar que no existan partidas existentes
								//->Si existen hay que traer las preguntas que respondió el otro jugador 
								//->Si no existe no hay que cambiar nada
							//3. Guardar y mostrar los puntos de los dos jugadores (esta)
							//Al inicio mostrar los jugadores disponibles para iniciar una partida nueva (Como ya hace)
								//4. ->Además mostrar si desea continuar una partida iniciada (En caso de existir) (esto ya esta)
							//5. Mostrar estadisticas (esta)
							//OPCIONAL 1 -> Agregar preguntas desde el servidor 
							//OPCIONAL 2 -> Enviar respuestas de los jugadores por correo
							}
						}
						if(strcmp(response, "2") == 0){
							printf("El usuario eligio 2\n");
							char* players = getGamesInProcess(username);
							printf ("%s", players);
							sprintf(socket_com, "%s", players);
							send(newSocket, socket_com, strlen(socket_com), 0); //Envia jugadores disponibles
							bzero (&response, sizeof (response));
							recv(newSocket, response, 1024, 0); //Recibe numero del usuario para continuar la partida
							int id_game = atoi(response);
							char* points = getPointsGame(id_game);
							bzero (&socket_com, sizeof (socket_com));
							sprintf(socket_com, "%s", points);
							send(newSocket, socket_com, strlen(socket_com), 0);
						}
						if(strcmp(response, "3") == 0){
							//estadisticas();
							printf("El usuario eligio 3\n");
							bzero (&response, sizeof (response));
							recv(newSocket, response, 1024, 0); //Espera por opcion del menu
						}
						if(strcmp(response, "4") == 0){
							//estadisticas();
							printf("El usuario eligio 4\n");
							break;
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
							printf("Entra al check: %d\n", check);
							bzero (&response, sizeof (response));
							sprintf(response, "Ok");
							send(newSocket, response, strlen(response), 0);
							bzero (&response, sizeof (response));
							recv(newSocket, response, 1024, 0); //Espera por opcion del menu
							printf("Response del usuario: %s\n", response);
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
									bzero(&response, sizeof (response));
									recv(newSocket, response, 1024, 0); //Por ahora para que no se cicle	
									//2. Al entrar a este while hay que verificar que no existan partidas existentes
										//->Si existen hay que traer las preguntas que respondió el otro jugador 
										//->Si no existe no hay que cambiar nada
									//3. Guardar y mostrar los puntos de los dos jugadores (esta)
									//Al inicio mostrar los jugadores disponibles para iniciar una partida nueva (Como ya hace)
										//4. ->Además mostrar si desea continuar una partida iniciada (En caso de existir) (esto ya esta)
									//5. Mostrar estadisticas (esta)
									//OPCIONAL 1 -> Agregar preguntas desde el servidor 
									//OPCIONAL 2 -> Enviar respuestas de los jugadores por correo
								}
							}
							if(strcmp(response, "2") == 0){
								printf("El usuario eligio 2\n");
								char* players = getGamesInProcess(username);
								sprintf(socket_com, "%s", players);
								send(newSocket, socket_com, strlen(socket_com), 0); //Envia jugadores disponibles
								bzero (&response, sizeof (response));
								recv(newSocket, response, 1024, 0); //Recibe numero del usuario para continuar la partida
								int id_game = atoi(response);
								char* points = getPointsGame(id_game);
								bzero (&socket_com, sizeof (socket_com));
								sprintf(socket_com, "%s", points);
								send(newSocket, socket_com, strlen(socket_com), 0);

							}
							if(strcmp(response, "3") == 0){
								printf("El usuario eligio 3\n");
								bzero (&response, sizeof (response));
								recv(newSocket, response, 1024, 0); //Espera por opcion del menu
							}
							if(strcmp(response, "4") == 0){
								printf("El usuario eligio 4\n");
								break;
							}
						}
					}
					else{
						printf("CHECK: %d\n", check);
						printf("No quiero ingresarlo a la base\n");
					}
				}
				bzero(&buffer, sizeof(buffer));
			}
		}
	}
	close(newSocket);
	return 0;
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
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			strcat(out, " Jugador: ");
			strcat(out, sqlite3_column_text(stmt,0));
			strcat(out, " - Puntos: ");
			strcat(out, sqlite3_column_text(stmt,1));
			strcat(out, "$");
		}
  }
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
			buffer = "No se encuentran partidas activas para este jugador\n";
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
	if (id==0){
		id = 1;
	}
	else{
		id = id /2 + 1;
	}
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
	sprintf(sql, "select count(*) from Game");
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