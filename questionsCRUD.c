//Obtiene la info de todas las preguntas
char* getAllQuestionsInfo(){
    sqlite3 *db = openDatabase();
    char sql[1024], out[4096];
    char *buffer;
    int rc;
    sprintf(sql, "select * from Questions;");
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed get questions: %s\n", sqlite3_errmsg(db));
        closeDatabase(db);
    }else
    {
        bzero(&out, sizeof(out));
        bzero(&buffer, sizeof(buffer));
        sprintf(out, "$");

        while(sqlite3_step(stmt) != SQLITE_DONE){
            strcat(out, "\nID:");
            strcat(out, sqlite3_column_text(stmt,0));
            strcat(out, "\nPregunta:");
            strcat(out, sqlite3_column_text(stmt,1));
            strcat(out, "\nValor:");
            strcat(out, sqlite3_column_text(stmt,2));
            strcat(out, "\nA:");
            strcat(out, sqlite3_column_text(stmt,3));
            strcat(out, "\nB:");
            strcat(out, sqlite3_column_text(stmt,4));
            strcat(out, "\nC:");
            strcat(out, sqlite3_column_text(stmt,5));
            strcat(out, "\nVeces acertada:");
            strcat(out, sqlite3_column_text(stmt,6));
            strcat(out, "\nVeces fallida:");
            strcat(out, sqlite3_column_text(stmt,7));
            strcat(out, "$");
        }
    }
    buffer = out;
    closeDatabase(db);
    return buffer;
}

//Inserta preguntas nuevas a la base de datos
void insertNewQuestion(char* question, char* op1, char* op2, char* op3, int value){
	sqlite3 *db = openDatabase();
    char sql[1024];
    int rc;
    sprintf(sql,"insert into Questions (question, option1, option2, option3, value_of_question, correct, incorrect) values ('%s','%s', '%s', '%s', %d, 0, 0);",question,op1,op2,op3,value);
	rc = sqlite3_exec(db, sql, callback, 0, 0);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
        closeDatabase(db);
    }
    else
    {
        printf("Pregunta ingresada al juego\n");
    }
    closeDatabase(db);
}

//Actualizar el valor de una pregunta
void updateValueQuestion(int id_q, int value){
	sqlite3 *db = openDatabase();
    char sql[1024];
    int rc;
    sprintf(sql, "update Questions set value_of_question = %d where Id_question = %d;", value, id_q);
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

//Borra una pregunta, solo si esta no esta asignada a ningún juego
void deleteQuestion(int id_q){
	sqlite3 *db = openDatabase();
    char sql[1024];
    int rc;
    sprintf(sql, "delete from Questions where Id_question = %d and (select count(*) from QuestionsPerGame where QuestionsPerGame.id_question = %d) = 0;",id_q,id_q);
	rc = sqlite3_exec(db, sql, callback, 0, 0);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to delete question: %s\n", sqlite3_errmsg(db));
        closeDatabase(db);
    }
    else
    {
        printf("El proceso de eliminacion ha concluido con exito\n");
    }
    closeDatabase(db);
}

//muestra la preguntas de manera descenedente por # de usos
char* getRankingQuestions(){
    sqlite3 *db = openDatabase();
    char sql[1024], out[4096];
    char *buffer;
    int rc;
    sprintf(sql, "select Questions.question, Questions.correct, Questions.incorrect, (Questions.correct+Questions.incorrect) from Questions order by (Questions.correct+Questions.incorrect) desc;");
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed get questions: %s\n", sqlite3_errmsg(db));
        closeDatabase(db);
    }else
    {
        bzero(&out, sizeof(out));
        bzero(&buffer, sizeof(buffer));
        sprintf(out, "$");

        while(sqlite3_step(stmt) != SQLITE_DONE){
            strcat(out, "\nPregunta:\n");
            strcat(out, sqlite3_column_text(stmt,0));
            strcat(out, "\n# Aciertos:");
            strcat(out, sqlite3_column_text(stmt,1));
            strcat(out, "\n# Fallos:");
            strcat(out, sqlite3_column_text(stmt,2));
            strcat(out, "\n# de usos:");
            strcat(out, sqlite3_column_text(stmt,3));
            strcat(out, "$");
        }
    }
    buffer = out;
    closeDatabase(db);
    return buffer;
}