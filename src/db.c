#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>

#include "db.h"
#include "err.h"
#include "cmd.h"

static MYSQL *sql;

static void write_ptr (struct cmd_login *c)
{
	char query[255];
	snprintf (query, 255, "insert into online_users (login, ptr) VALUES ('%s', %ld);",
			c->login,
			c->client.dc->ptr
		 );

	mysql_query (sql, query);
}

static void delete_ptr (struct data_client *dc)
{
	char query[255];
	snprintf (query, 255, "delete from online_users where ptr = %ld;",
			dc->ptr
		 );

	mysql_query (sql, query);
}

void db_user_close_connection (struct data_client *dc)
{
	delete_ptr (dc);
}

static char *get_login_from_ptr (struct cmd_profile *c)
{
	char query[255];
	snprintf (query, 255, "SELECT login from online_users WHERE ptr = %lu;", c->client.dc->ptr);

	mysql_query (sql, query);

	MYSQL_RES *res = mysql_store_result (sql);

	uint64_t num_rows = mysql_num_rows (res);

	if (num_rows != 1) {
		mysql_free_result (res);
		return NULL;
	}

	MYSQL_ROW row = mysql_fetch_row (res);

	char *login = strdup (row[0]);

	mysql_free_result (res);

	return login;
}

static int table_exists (char *tbl)
{
	char query[255];
	snprintf (query, 255, "SHOW TABLES LIKE '%s';", tbl);

	mysql_query (sql, query);

	MYSQL_RES *res = mysql_store_result (sql);

	uint64_t num_rows = mysql_num_rows (res);

	mysql_free_result (res);
	
	if (num_rows != 1) return 0;

	return 1;
}

static void create_table_profile (char *tbl)
{
	char query[1024];

	snprintf (query, 1024,
			"CREATE TABLE IF NOT EXISTS %s ("
			"id BIGINT NOT NULL AUTO_INCREMENT PRIMARY KEY, "
			"iam TEXT NOT NULL, "
			"short_bio TEXT NOT NULL, "
			"bio TEXT NOT NULL, "
			"day TINYINT UNSIGNED NOT NULL, "
			"month TINYINT UNSIGNED NOT NULL, "
			"year SMALLINT UNSIGNED NOT NULL, "
			"work TEXT NOT NULL "
			");",
			tbl
		 );

	mysql_query (sql, query);

	snprintf (query, 1024,
			"CREATE TABLE IF NOT EXISTS %s_interests ("
			"id BIGINT NOT NULL AUTO_INCREMENT PRIMARY KEY, "
			"interest TINYINT UNSIGNED NOT NULL "
			");",
			tbl
		 );

	mysql_query (sql, query);

	snprintf (query, 1024,
			"CREATE TABLE IF NOT EXISTS %s_search ("
			"id BIGINT NOT NULL AUTO_INCREMENT PRIMARY KEY, "
			"search TINYINT UNSIGNED NOT NULL "
			");",
			tbl
		 );

	mysql_query (sql, query);

	snprintf (query, 1024,
			"CREATE TABLE IF NOT EXISTS %s_photo ("
			"id BIGINT NOT NULL AUTO_INCREMENT PRIMARY KEY, "
			"photo STRING NOT NULL "
			");",
			tbl);

	mysql_query (sql, query);
}

static int table_is_fill (char *profile)
{
	char query[255];
	snprintf (query, 255, "SELECT * FROM %s;", profile);

	mysql_query (sql, query);

	MYSQL_RES *res = mysql_store_result (sql);

	uint64_t num_rows = mysql_num_rows (res);

	mysql_free_result (res);
	
	if (num_rows == 0) return 0;

	return 1;
}

static int update_table_profile_interests (struct cmd_profile *c, char *profile)
{
	int ret = NO_ERROR;

	char query[16384 + 255];
	snprintf (query, 255, "DELETE FROM %s;", profile);
	mysql_query (sql, query);

	for (int i = 0; i < c->count_interests; i++) {

		if (c->interests[i] >= MAX_INTERESTS) continue;

		snprintf (query, 16384 + 255,
				"INSERT INTO %s (interest) "
				"VALUES (%d);",
				profile,
				c->interests[i]
			 );

		mysql_query (sql, query);
	}


	return ret;
}

static int update_table_profile_search (struct cmd_profile *c, char *profile)
{
	int ret = NO_ERROR;

	char query[16384 + 255];
	snprintf (query, 255, "DELETE FROM %s;", profile);
	mysql_query (sql, query);

	for (int i = 0; i < c->count_search; i++) {

		if (c->search[i] >= MAX_SEARCH) continue;

		snprintf (query, 16384 + 255,
				"INSERT INTO %s (search) "
				"VALUES (%d);",
				profile,
				c->search[i]
			 );

		mysql_query (sql, query);
	}


	return ret;
}

static int update_table_profile (struct cmd_profile *c, char *profile)
{
	int ret = NO_ERROR;

	char query[16384 + 255];
	snprintf (query, 255, "DELETE FROM %s;", profile);
	mysql_query (sql, query);

	int len_short_bio = strlen (c->short_bio);
	int len_bio = strlen (c->bio);
	
	for (int i = 0; i < c->count_search; i++) {
		if (c->search[i] >= MAX_SEARCH) 
			return ERROR_EXCEED_MAX_SEARCH_PARAM;
	}

	char *short_bio = malloc (len_short_bio + 1);
	memset (short_bio, 0, len_short_bio + 1);

	char *bio = malloc (len_bio + 1);
	memset (bio, 0, len_bio + 1);

	int len_iam = strlen (c->iam);
	char *iam = malloc (len_iam + 1);
	memset (iam, 0, len_iam + 1);

	int len_work = strlen (c->work);
	char *work = malloc (len_work + 1);
	memset (work, 0, len_work + 1);

	mysql_escape_string (iam, c->iam, len_iam);
	mysql_escape_string (short_bio, c->short_bio, len_short_bio);
	mysql_escape_string (bio, c->bio, len_bio);
	mysql_escape_string (work, c->work, len_work);

	len_iam = strlen (iam);
	len_short_bio = strlen (short_bio);
	len_bio = strlen (bio);
	len_work = strlen (work);

	snprintf (query, 16384 + 255,
			"INSERT INTO %s ("
			"iam, "
			"short_bio, "
			"bio, "
			"day, "
			"month, "
			"year, "
			"work ) "
			"VALUES ( "
			"'%s', "
			"'%s', "
			"'%s', "
			"%d, "
			"%d, "
			"%d, "
			"'%s'"
			")"
			";",
			profile,
			len_iam > 0? iam: "",
			len_short_bio > 0? short_bio: "",
			len_bio > 0? bio: "",
			c->day,
			c->month,
			c->year,
			len_work > 0? work: ""
		 );

	mysql_query (sql, query);

	free (iam);
	free (short_bio);
	free (bio);
	free (work);
	
	return ret;
}

int db_fill_profile (struct cmd_profile *c)
{
	int ret = NO_ERROR;

	char *login = get_login_from_ptr (c);
	if (login == NULL) {
		return ERROR_CURRENT_USER_IS_NOT_ONLINE;
	}

	char profile[255];
	snprintf (profile, 255, "%s_profile", login);


	char profile_interests[255];
	snprintf (profile_interests, 255, "%s_interests", profile);

	char profile_search[255];
	snprintf (profile_search, 255, "%s_search", profile);

	free (login);

	if (!table_exists (profile)) {
		create_table_profile (profile);
	}

	ret = update_table_profile (c, profile);

	if (ret != NO_ERROR) return ret;
	
	ret = update_table_profile_interests (c, profile_interests);

	if (ret != NO_ERROR) return ret;

	ret = update_table_profile_search (c, profile_search);

	return ret;
}

int db_user_auth (struct cmd_login *c)
{
	int ret = NO_ERROR;

	char *login = malloc (c->len_login + 1);
	char *password = malloc (c->len_password + 1);

	memset (login, 0, c->len_login + 1);
	memset (password, 0, c->len_password + 1);

	mysql_escape_string (login, c->login, c->len_login);
	mysql_escape_string (password, c->password, c->len_password);

	int len_login = strlen (login);
	int len_password = strlen (password);

	if (!len_login || !len_password)
		return ERROR_LOGIN_OR_PASSWORD_IS_NULL;

	char query[255];
	snprintf (query, 255, "select * from users where login = '%s' and password = '%s';",
			login,
			password
		 );


	mysql_query (sql, query);

	MYSQL_RES *res = mysql_store_result (sql);

	uint64_t num_rows = mysql_num_rows (res);

	mysql_free_result (res);

	if (num_rows != 1) {
		free (login);
		free (password);
		return ERROR_UNKNOWN_LOGIN_OR_PASSWORD;
	}

	c->login = login;
	c->password = password;

	write_ptr (c);

	return ret;
}

int db_user_register (struct cmd_login *c)
{
	int ret = NO_ERROR;

	char *login = malloc (c->len_login + 1);
	char *password = malloc (c->len_password + 1);

	memset (login, 0, c->len_login + 1);
	memset (password, 0, c->len_password + 1);

	mysql_escape_string (login, c->login, c->len_login);
	mysql_escape_string (password, c->password, c->len_password);

	int len_login = strlen (login);
	int len_password = strlen (password);

	if (!len_login || !len_password)
		return ERROR_LOGIN_OR_PASSWORD_IS_NULL;

	char query[255];
	snprintf (query, 255, "select * from users where login = '%s';",
			login
		 );

	mysql_query (sql, query);

	MYSQL_RES *res = mysql_store_result (sql);

	uint64_t num_rows = mysql_num_rows (res);

	mysql_free_result (res);

	if (num_rows > 0) {
		free (login);
		free (password);
		return ERROR_LOGIN_EXISTS;
	}

	snprintf (query, 255, "insert into users (login, password) values ('%s', '%s');",
			login,
			password
		 );

	mysql_query (sql, query);

	c->login = login;
	c->password = password;

	write_ptr (c);

	return ret;
}


void init_db_mysql ( )
{
	sql = mysql_init (0);

	mysql_real_connect (sql,
			"localhost",
			"test",
			"test",
			"dating",
			0,
			0,
			0
			);

	char query[255];
	snprintf (query, 255, "DELETE FROM online_users;");

	mysql_query (sql, query);
}
