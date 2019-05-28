int getValueQuestion(int id_question){
	sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
	int value = 0;
	sprintf(sql, "select Questions.value_of_question from Questions where Questions.Id_question = %d", id_question);
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
			value = sqlite3_column_int(stmt,0);
		}
  }
  closeDatabase(db);
	return(value);
}