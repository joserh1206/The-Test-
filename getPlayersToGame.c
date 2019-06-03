char* getPlayersToGame(int id_player)
{
  sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	char *buffer;
  int rc;
	sprintf(sql,"select Users.id_user, Users.username from Users where Users.id_user != %d and Users.id_user not in (select Game.id_user from Game where Game.id_game in (select Game.id_game from Game where Game.id_user = %d));", id_player,id_player);
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
			strcat(out, " - ");
			strcat(out, sqlite3_column_text(stmt,1));
			strcat(out, "$");
		}
		buffer = out;
  }
	
  closeDatabase(db);
	return(buffer);
}
