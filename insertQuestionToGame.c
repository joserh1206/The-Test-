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