int getTurnPlayer(int id_game){
	sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	int turn;
  int rc;
	sprintf(sql, "select distinct Game.turn from Game where Game.id_game = %d;", id_game);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,16);
	if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed get turn: %s\n", sqlite3_errmsg(db));
    closeDatabase(db);
  }
	  else
  {
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			turn = sqlite3_column_int(stmt,0);
		}
  }
  closeDatabase(db);
	return(turn);
}