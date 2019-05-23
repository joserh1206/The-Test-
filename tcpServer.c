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
static int callback(void *NotUsed, int argc, char **argv, char **azColName);
void closeDatabase(sqlite3* db);
int makeGame(char *player2, char *username);
int getNewIdGame();
int getActualIdGame(char *username);
int getQuestionId();
int insertPlayer(int id_game, int id_player1, int id_player2);

int main(){

	int sockfd, ret;
	struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	char buffer[1024];
	char done[6] = "DONE";

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
				char *word, *type, *username, *password, *buffer2;
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
					printf("El usuario si existe\n");
					buffer2 = getPlayersToGame(username);
					printf("BUFFER -> %s\n", buffer2);
					sprintf(socket_com, "%s", buffer2);
					send(newSocket, socket_com, strlen(socket_com), 0); //Envia jugadores disponibles
					bzero (&response, sizeof (response));
					recv(newSocket, response, 1024, 0); //Recibe numero del usuario para una nueva partida
					printf("El usuario quiere jugar con el jugador n.%s\n", response);
					int id_game = makeGame(response, username);
					while(1){
								int id_question = getQuestionId();
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
						printf("Usuario a ingresar: %s\n", username);
						printf("Contrasenia a ingresar: %s\n", password);
						check = insertPlayerIntoDB(username, password);
						if (check){
							buffer2 = getPlayersToGame(username);
							printf("BUFFER -> %s\n", buffer2);
							send(newSocket, socket_com, strlen(socket_com), 0); //Envia jugadores disponibles
							bzero (&response, sizeof (response));
							recv(newSocket, response, 1024, 0); //Recibe numero del usuario para una nueva partida
							printf("El usuario quiere jugar con el jugador n.%s\n", response);
							int id_game = makeGame(response, username);
							while(1){
								int id_question = getQuestionId();
							}
						}
					}
					else{
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

char* getPlayersToGame(char *username)
{
  sqlite3 *db = openDatabase();
  sqlite3_stmt *stmt;
  char sql[1024], out[2048];
	char *buffer;
  int rc;
	sprintf(sql,"select * from Users where Users.id_user not in (select distinct Users.id_user from Users inner join Game on Users.id_user = Game.id_user where Game.id_game in (select Game.id_game from Game inner join Users on Game.id_user = Users.id_user and Users.username = '%s'))", username);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get players: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
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
	sqlite3_finalize(stmt);
  closeDatabase(db);
	return(buffer);
}

int checkUsernamePassword(char *username, char *password)
{
  sqlite3 *db = openDatabase();
  sqlite3_stmt *res;
  char sql[1024];
  int rc;
  sprintf(sql, "SELECT username, password FROM Users WHERE username = '%s' AND password = '%s';", username, password);
  printf("1\n");
  printf("2\n");
  printf("%s\n", sql);
  printf("3\n");
  rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return -1;
  }
  rc = sqlite3_step(res);
  if (rc == SQLITE_ROW) 
  {
    printf("Existe\n");
		sqlite3_finalize(res);
		closeDatabase(db);
    return 1;
  }
  else
  {
		printf("No existe\n");
		sqlite3_finalize(res);
		closeDatabase(db);
    return 0;
  }
  sqlite3_finalize(res);
  closeDatabase(db);
  return 0;
}

int insertPlayerIntoDB(char *username, char *password)
{
  sqlite3 *db = openDatabase();
  sqlite3_stmt *res;
  char sql[1024];
  int rc;
  sprintf(sql, "INSERT INTO Users (username, password) VALUES ('%s', '%s');", username, password);
  rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		sqlite3_finalize(res);
		closeDatabase(db);
    return -1;
  }
  else
  {
		printf("Usuario registrado correctamente\n");
  }
	sqlite3_finalize(res);
  closeDatabase(db);
  return 0;
}

int makeGame(char *player2, char *username)
{
	int id = getNewIdGame();
	int id_player1 = getActualIdGame(username);
	int id_player2 = atoi(player2);
	id = id /2 + 1;
	insertPlayer(id, id_player1, id_player2);
	return id;
}

int insertPlayer(int id_game, int id_player1, int id_player2){
	sqlite3 *db = openDatabase();
  sqlite3_stmt *res;
  char sql[1024];
  int rc;
  sprintf(sql, "insert into Game (id_game,id_user,points) values (%d, %d, 0), (%d, %d, 0);", id_game, id_player1, id_game, id_player2);
  printf("sql query: %s\n", sql);
	rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(res);
		closeDatabase(db);
  }
  else
  {
		printf("Partida registrada correctamente\n");
  }
	sqlite3_finalize(res);
  closeDatabase(db);
	return id_game;
}

int getActualIdGame(char *username)
{
  sqlite3 *db = openDatabase();
  sqlite3_stmt *stmt;
  char sql[1024];
  int rc;
	int newId = 0;
	sprintf(sql, "select Users.id_user from Users where Users.username = '%s'",username);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get players: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
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
	sqlite3_finalize(stmt);
  closeDatabase(db);
	return(newId);
}

int getQuestionId()
{
  sqlite3 *db = openDatabase();
  sqlite3_stmt *stmt;
  char sql[1024];
  int rc;
	int newId = 0;
	sprintf(sql,"select Questions.Id_question from Questions order by random() limit 1;");
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get players: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
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
	sqlite3_finalize(stmt);
  closeDatabase(db);
	return(newId);
}

/*
select count(*) from Game;
insert into Game (id_game,id_user,points) values ('2',(select Users.id_user from Users where Users.username = 'gera0807'),0), ('2','4',0)

insert into QuestionsPerGame (id_game,id_question,good_option,active) values ('2',2,null,0)

--Muestra la opción de la pregunta
select QuestionsPerGame.good_option from QuestionsPerGame where QuestionsPerGame.id_question = %s

--Actualizar opcion correcta
update QuestionsPerGame set good_option = %d where QuestionsPerGame.id_game = %d and QuestionsPerGame.id_question = %d

--Insertar preguntas al juego
insert into QuestionsPerGame (id_game,id_question,good_option,active) values (%d,%d,null,0)

--Obtener preguntas
select Questions.Id_question, Questions.question, Questions.option1, Questions.option2, Questions.option3 from Questions order by random() limit 1;

--Consulta partidas activas por jugador
select Game.id_game, Game.id_user, Game.turn from Game where Game.id_game in (select Game.id_game from Game where Game.id_user = (select Users.id_user from Users where Users.username = 'gera0807')) and not Game.id_user = (select Users.id_user from Users where Users.username = 'gera0807')
*/

/* int getNewIdGame(){
	sqlite3 *db = openDatabase();
  sqlite3_stmt *stmt;
  char sql[1024];
	char id[4];
  int rc, newId;
	sprintf(sql, "select count(*) from Game");
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get id new game: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
  }
  else
  {
		printf("%s",sqlite3_column_text(stmt,0));
		newId = atoi(sqlite3_column_text(stmt,0));
		return newId;
	}
	return -1;
}
 */

int getNewIdGame()
{
  sqlite3 *db = openDatabase();
  sqlite3_stmt *stmt;
  char sql[1024];
  int rc;
	int newId = 0;
	sprintf(sql, "select count(*) from Game");
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get players: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
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
	sqlite3_finalize(stmt);
  closeDatabase(db);
	return(newId);
}


static int callback(void *NotUsed, int argc, char **argv, char **azColName){
	printf("Esto es callback\n");
    int i;
		printf("%d\n", argc);
    for(i=0; i < argc*2; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

void closeDatabase(sqlite3* db) { sqlite3_close(db); }

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