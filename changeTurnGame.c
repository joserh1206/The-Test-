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

void addIterationGame(int id_game){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "update Game set iteration = iteration + 1 where Game.id_game = %d;", id_game);
	rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
  }
  else
  {
		printf("Iteracion actualizado\n");
  }
  closeDatabase(db);
}

int getIteration(int id_game)
{
  sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
	int iteration = 0;
	sprintf(sql, "select Game.iteration from Game where Game.id_game = %d;", id_game);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get iteration: %s\n", sqlite3_errmsg(db));
    
		closeDatabase(db);
  }
  else
  {
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			iteration = sqlite3_column_int(stmt,0);
		}
  }
	
  closeDatabase(db);
	return(iteration);
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