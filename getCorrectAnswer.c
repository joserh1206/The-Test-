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

char* checkAnswer(int id_game, int response2, int response, char* username){
  setSelectedAnswer(response, id_game, response2);
  int correct_answ = getCorrectAnswer(id_game, response2);
	if(correct_answ == response)
	{
		int id_player = getActualIdGame(username);
		addSuccess(response2);
		updateGoodAnswerStatistics(id_player, id_game);
		updateLevel(getValueQuestion(response2),id_player,id_game);
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

void setSelectedAnswer(int optionSelected, int id_game, int id_question){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "update QuestionsPerGame set selected_option = %d where QuestionsPerGame.id_game = %d and QuestionsPerGame.id_question = %d", optionSelected, id_game, id_question);
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

char* getGoodAndSelectedOption(int id_game)
{
  sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	char *buffer;
  int rc;
	sprintf(sql, "select Questions.question, QuestionsPerGame.good_option, QuestionsPerGame.selected_option from QuestionsPerGame inner join Questions on QuestionsPerGame.id_question and Questions.Id_question where QuestionsPerGame.id_question in (select QuestionsPerGame.id_question from QuestionsPerGame where QuestionsPerGame.id_game = %d limit 2 offset (select count(*) from QuestionsPerGame where QuestionsPerGame.id_game = %d)-4);", id_game,id_game);
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
      strcat(out, "Pregunta: ");
			strcat(out, sqlite3_column_text(stmt,0));
			strcat(out, " - Opción elegida por mi: ");
			strcat(out, sqlite3_column_text(stmt,1));
			strcat(out, " - Opción elegida por el contrario: ");
			strcat(out, sqlite3_column_text(stmt,2));
      strcat(out, "$");
		}
  }
	buffer = out;
  closeDatabase(db);
	return(buffer);
}