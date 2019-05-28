//Obtiene todos los juegos de un usuario especifico y las respuestas buenas y malas
char* getAllGamesAndStatisticsForPlayer(int player){
	sqlite3 *db = openDatabase();
  char sql[1024], out[2048];
	char *buffer;
  int rc;
	sprintf(sql, "select Statistics.id_game, Statistics.good_answer, Statistics.bad_answer from Statistics where Statistics.id_user = %d;", player);
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
		sprintf(out, "$");
		while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
			strcat(out, "Id del juego: ");
			strcat(out, sqlite3_column_text(stmt,0));
			strcat(out, " - Preguntas acertadas: ");
			strcat(out, sqlite3_column_text(stmt,1));
			strcat(out, " - Preguntas fallidas: ");
			strcat(out, sqlite3_column_text(stmt,2));
			strcat(out, "$");
		}
  }
	if(strcmp(out, "$") == 0){
		buffer = out;
	}
	else{
		strcat(out, getRightAndWrongPlayer(player));
		buffer = out;
	}
	closeDatabase(db);
	return buffer;
}
