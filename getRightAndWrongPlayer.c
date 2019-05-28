//Obtiene la suma de los acierto y los fallos de un jugador
char* getRightAndWrongPlayer(int player){
	sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	char *buffer;
  int rc;
	sprintf(sql, "select coalesce(sum(Statistics.good_answer), 0), coalesce(sum(Statistics.bad_answer), 0) from Statistics where Statistics.id_user = %d;", player);
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
			strcat(out, "Total de preguntas acertadas: ");
			strcat(out, sqlite3_column_text(stmt,0));
			strcat(out, "\nTotal de preguntas fallidas: ");
			strcat(out, sqlite3_column_text(stmt,1));
		}
  }
	buffer = out;
	closeDatabase(db);
	return buffer;
}
