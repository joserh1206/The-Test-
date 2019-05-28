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