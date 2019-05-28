int getRival(int game, int actual)
{
  sqlite3 *db = openDatabase();
  char sql[1024];
	int player;
  int rc;
  sprintf(sql, "select Game.id_user from Game where Game.id_game = %d and not Game.id_user = %d;", game, actual);
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to select data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
    return -1;
  }
  else
  {
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			player = sqlite3_column_int(stmt,0);
		}
  }
  closeDatabase(db);
	return(player);
}
