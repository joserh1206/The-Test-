char* getPointsGame(int id_game){
	sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	char *buffer;
  int rc;
	sprintf(sql, "select Users.username, Game.points from Game inner join Users on Users.id_user = Game.id_user where Game.id_game = %d;", id_game);
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
			strcat(out, "Jugador: ");
			strcat(out, sqlite3_column_text(stmt,0));
			strcat(out, " - Puntos: ");
			strcat(out, sqlite3_column_text(stmt,1));
			strcat(out, "\n$");
		}
  }
	buffer = out;
	closeDatabase(db);
	return buffer;
}

void addPoints(int id_game, int id_player, int value){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "update Game set points = points + %d where Game.id_game = %d and Game.id_user = %d", value, id_game, id_player);
	rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
  }
  else
  {
		printf("Puntos actualizados al jugador\n");
  }
  closeDatabase(db);
}