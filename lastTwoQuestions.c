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
	buffer = out;
	closeDatabase(db);
	return buffer;
}