#define PORT 4444
int getRival(int game, int actual);
char* checkAnswer(int id_game, int response2, int response, char* username);
void getNewQuestions(int id_game, char socket_com[1024], int newSocket, char response[4], char* answers, int id_player2);
int checkUsernamePassword(char *username, char *password);
int insertPlayerIntoDB(char *username, char *password);
void beginGame(char *username);
char* getTwoQuestionsLastPLayer(int game);
char* getAllGamesAndStatisticsForPlayer(int player);
char* getRightAndWrongPlayer(int player);
int getCorrectAnswer(int game, int question);
char* getPlayersToGame(char *username);
char* getPlayersToGame2(char *username);
char* getGamesInProcess(char* username);
char* getPointsGame(int id_game);
static int callback(void *NotUsed, int argc, char **argv, char **azColName);
void updateGoodAnswerStatistics(int player, int game);
void updateBadAnswerStatistics(int player, int game);
void changeTurnGame(int id_game, int playerTurn);
int createStatistics(int player, int game);
int makeGame(char *player2, char *username);
int getNewIdGame();
int getActualIdGame(char *username);
int getQuestionId();
void insertPlayer(int id_game, int id_player1, int id_player2);
int insertQuestionToGame(int id_game, int id_question);
char* getQuestionData(int id_question);
void addMiss(int id_question);
void addSuccess(int id_question);
void addPoints(int id_game, int id_player, int value);
int getValueQuestion(int id_question);
int getGoodOption(int id_game, int id_question);
void setCorrectAnswer(int optionSelected, int id_game, int id_question);
int getTurnPlayer(int id_game);
sqlite3* openDatabase();
void closeDatabase(sqlite3* db);
sqlite3_stmt *stmt;