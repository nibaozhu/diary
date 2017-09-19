#include <mysql.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
	MYSQL *mysql = mysql_init(NULL);
	mysql_real_connect(mysql, "127.0.0.1", "root", "123456", "test", MYSQL_PORT_DEFAULT, NULL, 0);
	mysql_ping(mysql);
	char q[1024] = { 0 };

	int i;
	for (i = 0; i < 1024*1024; i++) {
		sprintf(q, "select name, create_datetime from t1 where id = %d", rand() % 4 + 3);
		mysql_real_query(mysql, q, strlen(q));
		MYSQL_RES* res = mysql_store_result(mysql);
		MYSQL_ROW row;
		while(row = mysql_fetch_row(res))
		    printf("name: '%s', create_datetime: '%s', ...\n", row[0], row[1]);
		mysql_free_result(res);
	}
	return 0;
}
