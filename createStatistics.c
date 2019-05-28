int createStatistics(int player, int game)
{
  sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "insert into Statistics (id_user, id_game, good_answer, bad_answer) values (%d,%d,0,0);", player, game);
  rc = sqlite3_exec(db, sql, callback, 0, 0);
  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
    return -1;
  }
  else
  {
		printf("Estadistica registrado correctamente\n");
  }
  closeDatabase(db);
  return 0;
}