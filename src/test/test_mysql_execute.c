#include <mysql.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
	MYSQL *mysql = mysql_init(NULL);
	mysql_real_connect(mysql, "127.0.0.1", "root", "123456", "test", MYSQL_PORT_DEFAULT, NULL, 0);
	mysql_ping(mysql);

	char query[1024] = "select name, create_day, create_datetime from t1 where id = ?";
	unsigned long length0 = strlen(query);
	MYSQL_STMT *stmt = mysql_stmt_init(mysql);
	mysql_stmt_prepare(stmt, query, length0);

	MYSQL_BIND bnd_param[1];
	memset(bnd_param, 0, sizeof (bnd_param));

	int int_id = 0;
	bnd_param[0].buffer_type = MYSQL_TYPE_LONG;
	bnd_param[0].buffer = (char*)&int_id;
	bnd_param[0].is_null = 0;
	bnd_param[0].length = 0;

	mysql_stmt_bind_param(stmt, bnd_param);

	MYSQL_BIND bnd_result[3];
	my_bool is_null[3];
	unsigned long length[3];
	memset(bnd_result, 0, sizeof (bnd_result));
	memset(is_null, 0, sizeof (is_null));
	memset(length, 0, sizeof (length));

	char str_name[255] = { 0 };
	bnd_result[0].buffer_type = MYSQL_TYPE_STRING;
	bnd_result[0].buffer_length = 255;
	bnd_result[0].buffer = (char*)&str_name;
	bnd_result[0].is_null = &is_null[0];
	bnd_result[0].length = &length[0];

	char str_create_day[255] = { 0 };
	bnd_result[1].buffer_type = MYSQL_TYPE_STRING;
	bnd_result[1].buffer_length = 255;
	bnd_result[1].buffer = (char*)&str_create_day;
	bnd_result[1].is_null = &is_null[1];
	bnd_result[1].length = &length[1];

	char str_create_datetime[255] = { 0 };
	bnd_result[2].buffer_type = MYSQL_TYPE_STRING;
	bnd_result[2].buffer_length = 255;
	bnd_result[2].buffer = (char*)&str_create_datetime;
	bnd_result[2].is_null = &is_null[2];
	bnd_result[2].length = &length[2];

	mysql_stmt_bind_result(stmt, bnd_result);

	int i;
	for (i = 0; i < 1024*1024; i++) {
		int_id = rand() % 4 + 3; // TODO: ...
		mysql_stmt_execute(stmt);
		int rc;
		while ( 0 == (rc = mysql_stmt_fetch(stmt)) )
			printf("name: '%s', create_day: '%s', create_datetime: '%s', ...\n", 
				str_name, str_create_day, str_create_datetime);
	}

	mysql_stmt_close(stmt);
	return 0;
}
