char* getPointsGame(int id_game){
	sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	char *buffer;
  int rc;
	sprintf(sql, "select Users.username, Game.points, Game.level_player from Game inner join Users on Users.id_user = Game.id_user where Game.id_game = %d;", id_game);
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
			strcat(out, " - Nivel: ");
			strcat(out, sqlite3_column_text(stmt,2));
			strcat(out, "\n$");
		}
  }
	buffer = out;
	closeDatabase(db);
	return buffer;
}

void updateLevel(int points, int player, int game){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "update Game set level_player = case when points+%d > 100 then level_player+1 else level_player end where id_user = %d and id_game = %d", points, game, player);
	rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
  }
  else
  {
		printf("Se actualizo el campo de respuestas buenas\n");
  }
  closeDatabase(db);
}

void addPoints(int id_game, int id_player, int points){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "update Game set points = case when points+%d > 100 then (points+%d)-100 else points+%d end where id_user = %d and id_game = %d", points, points, points, id_player, id_game);
	rc = sqlite3_exec(db, sql, callback, 0, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
  }
  else
  {
		printf("Se actualizo el campo de respuestas buenas\n");
  }
  closeDatabase(db);
}