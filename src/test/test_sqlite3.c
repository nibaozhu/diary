#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <linux/limits.h>

int callback(void *NotUsed, int argc, char **argv, char **colname)
{
	int i;
	static int j = 0;
	printf("j = %d,\t", j++);
	for (i = 0; i < argc; i++)
	{
		printf("i = %d, %s = %s\t", i, colname[i], argv[i]);
	}
	printf("\n");
	return 0;
}

int main(int argc, char **argv)
{
	sqlite3 *db;
	char filename[PATH_MAX];
	int retval = 0;

	memset(filename, 0, sizeof filename);
	if (argc >= 2)
	{
		strncpy(filename, argv[1], sizeof filename - 1);
	}

	retval = sqlite3_open(filename, &db);
	if (retval != 0)
	{
		fprintf(stderr, "%s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return retval;
	}

	char sql[1024];
	memset(sql, 0, sizeof sql);
	char *errmsg = NULL;
	strcpy(sql, "create table t1(url text, title text, description text)");

	retval = sqlite3_exec(db, sql, callback, 0, &errmsg);
	if (retval != SQLITE_OK)
	{
		fprintf(stderr, "%s\n", errmsg);
		sqlite3_free(errmsg);
	}

	int i = 0;
	char url[20] = "abcdefg";
	char title[20] = "abcdefg";
	char description[20] = "abcdefg";
	while (i < 10)
	{
		strfry(url);
		strfry(title);
		strfry(description);
		memset(sql, 0, sizeof sql);
		snprintf(sql, sizeof sql, "insert into t1(url, title, description) values('www.%s.com', '%s_%d', '%s')", 
			url, title, i++, description);

		retval = sqlite3_exec(db, sql, callback, 0, &errmsg);
		if (retval != SQLITE_OK)
		{
			fprintf(stderr, "%s\n", errmsg);
			sqlite3_free(errmsg);
		}
	}

	memset(sql, 0, sizeof sql);
	strcpy(sql, "select url, title, description from t1");

	retval = sqlite3_exec(db, sql, callback, 0, &errmsg);
	if (retval != SQLITE_OK)
	{
		fprintf(stderr, "%s\n", errmsg);
		sqlite3_free(errmsg);
	}

	sqlite3_close(db);
	return argc;
}

