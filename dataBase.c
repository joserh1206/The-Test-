void closeDatabase(sqlite3* db) { 
	sqlite3_finalize(stmt);
	sqlite3_close(db); 
}

sqlite3* openDatabase()
{
	int rc;
  	sqlite3 *db;
  	rc = sqlite3_open("DatabaseTheTest.db", &db);
	if(rc){
		fprintf(stderr, "Cant open database %s\n", sqlite3_errmsg(db));
	}
	else{
		fprintf(stderr, "Acceso a la base de datos!\n");
	}
  	return db;
}