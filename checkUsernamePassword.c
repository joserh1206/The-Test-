int checkUsernamePassword(char *username, char *password)
{
  sqlite3 *db = openDatabase();
  char sql[1024];
  int rc;
  sprintf(sql, "SELECT username, password FROM Users WHERE username = '%s' AND password = '%s';", username, password);
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
		closeDatabase(db);
    return -1;
  }
  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW) 
  {
		closeDatabase(db);
    return 1;
  }
  else
  {
		closeDatabase(db);
    return 0;
  }
  closeDatabase(db);
  return 0;
}