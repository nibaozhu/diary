#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <linux/limits.h>

int callback(void *NotUsed, int argc, char **argv, char **colname)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		;;;
	}
	return 0;
}

int main(int argc, char **argv)
{
	sqlite3 *db;
	char filename[PATH_MAX];
	int retval = 0;

	memset(filename, 0, sizeof filename);
	strncpy(filename, argv[1], sizeof filename - 1);

	retval = sqlite3_open(filename, &db);
	if (retval != 0)
	{
		fprintf(stderr, "%s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return retval;
	}

	char sql[SQLITE_LIMIT_SQL_LENGTH];
	memset(sql, 0, sizeof sql);
	char *errmsg = NULL;

	retval = sqlite3_exec(db, sql, callback, 0, &errmsg);
	if (retval != SQLITE_OK)
	{
		fprintf(stderr, "%s\n", errmsg);
		sqlite3_free(errmsg);
	}

	sqlite3_close(db);

	return argc;
}

