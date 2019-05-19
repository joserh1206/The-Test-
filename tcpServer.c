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
static int callback(void *NotUsed, int argc, char **argv, char **azColName);
void closeDatabase(sqlite3* db);

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

			while(1){
				const char separator[2] = "$";
				char *word, *type, *username, *password;
				char response[4];
				int check = 0;
				bzero (&username, sizeof (username));
				bzero (&password, sizeof (password));
				bzero (&buffer, sizeof (buffer));

				recv(newSocket, buffer, 1024, 0);
				username = strtok(buffer, separator);
				password = strtok(NULL, separator);
				/*
				if(strcmp(type, "e") == 0){
					printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					break;
				}
				if(strcmp(type, "u") == 0){
					username = word;
					printf("Se envió el nombre de usuario\n");
					printf("Y es %s\n", username);
					send(newSocket, done, strlen(done), 0);
					bzero(&done, sizeof(done));
					bzero(&buffer, sizeof(buffer));
				}
				if(strcmp(type, "p") == 0){
					password = word;
					printf("Se recibió la contrasenia\n");
					printf("Y es %s\n", password);
					send(newSocket, done, strlen(done), 0);
					bzero(&done, sizeof(done));
					bzero(&buffer, sizeof(buffer));
				}
				*/

				check = checkUsernamePassword(username, password);
				//bzero (&buffer, sizeof (buffer));

				if(check){
					printf("El usuario si existe\n");
				}
				else
				{
					printf("El usuario no existe\n");
					sprintf(response, "-1");
					send(newSocket, response, strlen(response), 0);
					bzero (&response, sizeof (response));
					recv(newSocket, response, 1024, 0);
					if(strcmp(response, "S") == 0 || strcmp(response, "s") == 0){
						printf("Usuario a ingresar: %s\n", username);
						printf("Contrasenia a ingresar: %s\n", password);
						check = insertPlayerIntoDB(username, password);
					}
					else{
						printf("No quieron ingresarlo a la base\n");
					}
				}
				bzero(&buffer, sizeof(buffer));
			}
		}

	}

	close(newSocket);


	return 0;
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
	//printf("%s\n", sqlite3_column_text(res, 0));
    return 1;
  }
  else
  {
    //sqlite3_finalize(res);
	printf("No existe\n");
	//printf("%s\n", sqlite3_column_text(res, 0));
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
  printf("1\n");
  printf("2\n");
  printf("%s\n", sql);
  printf("3\n");
  rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return -1;
  }
  else
  {
    //sqlite3_finalize(res);
	printf("Usuario registrado correctamente\n");
	//printf("%s\n", sqlite3_column_text(res, 0));
  }
  closeDatabase(db);
  return 0;
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i < argc; i++){
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
		fprintf(stderr, "BD Open!");
	}
  	return db;
}