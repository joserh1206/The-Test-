void getNewQuestions(int id_game, char socket_com[1024], int newSocket, char response[4], char* answers, int id_player2){
	int id_question = getQuestionId();
	insertQuestionToGame(id_game, id_question);
	char* question1 = getQuestionData(id_question);
	strcat(socket_com, question1);
	strcat(socket_com, "$");
	int id_question2 = getQuestionId();
	insertQuestionToGame(id_game, id_question2);
	char* question2 = getQuestionData(id_question2);
	strcat(socket_com, question2);
	send(newSocket, socket_com, strlen(socket_com), 0); //Se envian las 2 preguntas al usuario
	recv(newSocket, response, 1024, 0); //Recibe las respuestas del usuario para las 2 preguntas
	answers = response;
	setCorrectAnswer(answers[0]-'0',id_game,id_question);
	setCorrectAnswer(answers[2]-'0',id_game,id_question2);
	changeTurnGame(id_game, id_player2);
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