char* getGamesInProcess(char* username){
	sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	char *buffer;
  int rc;
	sprintf(sql, "select Game.id_game, Users.username from Users inner join Game on Users.id_user = Game.id_user where Game.id_game in (select distinct Game.id_game from Game where Game.id_user = (select Users.id_user from Users where Users.username = '%s')) and not Users.username = '%s';", username, username);
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
		int counter = 0;
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			strcat(out, sqlite3_column_text(stmt,0));
			strcat(out, " - ");
			strcat(out, sqlite3_column_text(stmt,1));
			strcat(out, "$");
			counter++;
		}
		if(counter == 0){
			buffer = "-1";
		}
		else{
			buffer = out;
		}
  }
  closeDatabase(db);
	return(buffer);
}