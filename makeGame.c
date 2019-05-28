int makeGame(char *player2, char *username)
{
	int id = getNewIdGame();
	id++;
	
	int id_player1 = getActualIdGame(username);
	int id_player2 = atoi(player2);
	insertPlayer(id, id_player1, id_player2);
	return id;
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

int getNewIdGame()
{
  sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
	int newId = 0;
	sprintf(sql, "select coalesce(max(id_game),0) from Game");
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
