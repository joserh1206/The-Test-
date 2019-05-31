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

void insertPlayer(int id_game, int id_player1, int id_player2){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "insert into Game (id_game,id_user,points,turn,level_player,iteration) values (%d, %d, 0, %d, 1, 1), (%d, %d, 0, %d, 1, 1);", id_game, id_player1, id_player1, id_game, id_player2, id_player1);
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