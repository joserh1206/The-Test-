void updateBadAnswerStatistics(int player, int game){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "update Statistics set bad_answer = bad_answer + 1 where Statistics.id_game = %d and Statistics.id_user = %d;", game, player);
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