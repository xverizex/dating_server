#include <stdio.h>
#include <string.h>
#include <openssl/ssl.h>
#include "err.h"
#include "dc.h"
#include "cmd.h"
#include "db.h"
#include "storage.h"

static void *get_data_from_next (uint8_t **data, int *last_size, int first)
{
	int n = 0;
	char *s = (char *) *data;
	char *start = (char *) (*data - 1);

	if (!first) {
		s = strchr ((char *) s, '\0');
		n++;
		s++;

	}

	int total = s - start;

	if (total >= *last_size)
		return NULL;

	*last_size -= total;
	*data = s;

	return s;
}

static void *get_data_from_index (uint8_t *data, int data_size, int index)
{
	int n = 0;
	char *s = (char *) data;
	char *start = (char *) (data - 1);

	while (n < index) {
		s = strchr ((char *) s, '\0');
		n++;
		s++;
	}

	int total = s - start;

	if (total >= data_size)
		return NULL;

	return s;
}

void command_handle_close_connection (struct data_client *dc)
{
	db_user_close_connection (dc);
}

int command_handle_login (struct data_client *dc, uint8_t *data, int data_size)
{
	int ret = NO_ERROR;

	struct cmd_login c;
	c.client.dc = dc;

	int size = data_size;

	uint8_t *s = &data[1];

	c.login = (char *) get_data_from_next (&s, &size, 1);
	if (size < 0)
		return ERROR_BUFFER_OVERFLOW;

	c.password = (char *) get_data_from_next (&s, &size, 0);
	if (size < 0)
		return ERROR_BUFFER_OVERFLOW;

	if ((!c.login) || (!c.password))
		return ERROR_UNKNOWN_LOGIN_OR_PASSWORD;

	c.len_login = strlen (c.login);
	c.len_password = strlen (c.password);

	if ((c.len_login > 32) || (c.len_password > 32))
		return ERROR_EXCEED_LOGIN_OR_PASSWORD_CHARACTER_COUNT;

	ret = db_user_auth (&c);

	return ret;
}

int command_handle_fill_profile (struct data_client *dc, uint8_t *data, int data_size)
{
	int ret = NO_ERROR;

	struct cmd_profile c;
	c.client.dc = dc;

	int n = 0;

	uint8_t *s = &data[1];
	int size = data_size;

	c.iam = (char *) get_data_from_next (&s, &size, 1);
	if (size < 0)
		return ERROR_BUFFER_OVERFLOW;
	int len_iam = strlen (c.iam);
	if (len_iam == 0) 
		return ERROR_NOT_I_AM;

	if (len_iam > TOTAL_IAM)
		return ERROR_EXCEED_I_AM;

	c.short_bio = (char *) get_data_from_next (&s, &size, 0);
	c.bio = (char *) get_data_from_next (&s, &size, 0);
	c.count_search = *(uint8_t *) get_data_from_next (&s, &size, 0);
	s++;
	size--;
	if (size < 0)
		return ERROR_BUFFER_OVERFLOW;

	if (c.count_search > TOTAL_SEARCH)
		return ERROR_EXCEED_COUNT_SEARCH;

	for (int i = 0; i < c.count_search; i++, s++) {
		c.search[i] = *s;
		size--;
		if (size < 0)
			return ERROR_BUFFER_OVERFLOW;
	}

	c.count_for_what = *(uint8_t *) get_data_from_next (&s, &size, 0);
	s++;
	size--;
	if (size < 0)
		return ERROR_BUFFER_OVERFLOW;

	if (c.count_for_what > TOTAL_FOR_WHAT)
		return ERROR_EXCEED_COUNT_FOR_WHAT;

	for (int i = 0; i < c.count_for_what; i++, s++) {
		c.for_what[i] = *s;
		size--;
		if (size < 0)
			return ERROR_BUFFER_OVERFLOW;
	}

	uint8_t months[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	c.day = *s;
	s++;
	size--;
	if (size < 0)
		return ERROR_BUFFER_OVERFLOW;
	c.month = *s;
	s++;
	size--;
	if (size < 0)
		return ERROR_BUFFER_OVERFLOW;
	c.year = *((uint16_t *) s);
	s += 2;
	size -= 2;
	if (size < 0)
		return ERROR_BUFFER_OVERFLOW;

	if (c.month < 1 || c.month > 12)
		return ERROR_NOT_CORRECT_MONTH_BIRTHDAY;

	if (c.month == 2) {
		int leap_year = c.year % 4 == 0 && c.year % 100 != 0? 1: c.year % 100 == 0 && c.year % 400 == 0? 1: 0;
		if (leap_year) months[1] = 29;

		if (c.day == 0 || c.day > months[c.month - 1])
			return ERROR_NOT_CORRECT_DAY_BIRTHDAY;

	} else {
		if (c.day == 0 || c.day > months[c.month - 1])
			return ERROR_NOT_CORRECT_DAY_BIRTHDAY;
	}
	
	c.count_interests = *s;
	s++;
	size--;
	if (size < 0)
		return ERROR_BUFFER_OVERFLOW;

	if (c.count_interests > TOTAL_INTERESTS)
		return ERROR_EXCEED_COUNT_INTERESTS;

	for (int i = 0; i < c.count_interests; i++, s++) {
		c.interests[i] = *s;
		size--;
		if (size < 0)
			return ERROR_BUFFER_OVERFLOW;
	}

	c.work = (char *) get_data_from_next (&s, &size, 0);

	ret = db_fill_profile (&c);

	return ret;
}

int command_handle_register (struct data_client *dc, uint8_t *data, int data_size)
{
	int ret = NO_ERROR;

	struct cmd_login c;
	c.client.dc = dc;

	int size = data_size;

	uint8_t *s = &data[1];

	c.login = (char *) get_data_from_next (&s, &size, 1);
	if (size < 0)
		return ERROR_BUFFER_OVERFLOW;
	c.password = (char *) get_data_from_next (&s, &size, 0);
	if (size < 0)
		return ERROR_BUFFER_OVERFLOW;

	if ((!c.login) || (!c.password))
		return ERROR_UNKNOWN_LOGIN_OR_PASSWORD;

	c.len_login = strlen (c.login);
	c.len_password = strlen (c.password);

	if ((c.len_login > 32) || (c.len_password > 32))
		return ERROR_EXCEED_LOGIN_OR_PASSWORD_CHARACTER_COUNT;

	ret = db_user_register (&c);

	return ret;
}

void command_write_answer (struct data_client *dc, int ret)
{
	uint8_t d[1];
	d[0] = ret;

	SSL_write (dc->ssl, d, 1);
}

int command_handle_continue_upload_photo (struct data_client *dc, uint8_t *data, int data_size)
{
	int ret = NO_ERROR;

	/* index to upload photo */
	uint8_t index_to_upload = data[1];
	if (dc->ur.free[index_to_upload])
		return ERROR_UPLOAD_TO_UNAVAILABLE_FILE;

	if ((dc->ur.total_size[index_to_upload] + data_size) > MAX_UPLOAD_SIZE_PHOTO) {
		dc->ur.free[index_to_upload] = 1;
		return ERROR_EXCEED_UPLOAD_SIZE;
	}

	storage_add_data_to (dc->ur.filename[index_to_upload], &data[2], data_size - 2);

	return ret;
}

int command_handle_start_upload_photo (struct data_client *dc, uint8_t *data, int data_size)
{
	int ret = NO_ERROR;
	static int inf = 0;

	uint8_t free_index = MAX_UPLOAD_IN_TIME + 1;

	for (int i = 0; i < MAX_UPLOAD_IN_TIME; i++) {
		if (dc->ur.free[i]) {
			free_index = i;
			break;
		}
	}
	
	if (free_index >= MAX_UPLOAD_IN_TIME)
		return ERROR_NOT_FOUND_FREE_INDEX;

	char *filename = storage_get_next_name ( );
	uint32_t len = strlen (filename);
	inf++;

	dc->ur.subcommand = START_UPLOAD_FREE_INDEX;
	dc->ur.index = free_index;
	dc->ur.free[free_index] = 0;
	dc->ur.total_size[free_index] = 0L;
	snprintf (dc->ur.filename[free_index], 256, "%s.jpg", filename);

	return ret;
}

int command_handle_finish_upload_photo (struct data_client *dc, uint8_t *data, int data_size)
{
	int ret = NO_ERROR;

	if (dc->ur.free[data[1]])
		return ERROR_INDEX_UPLOAD_PHOTO_ALREADY_FREE;

	dc->ur.free[data[1]] = 1;
	dc->ur.total_size[data[1]] = 0L;

	return ret;
}

void command_write_answer_with_upload_rs (struct data_client *dc, int ret)
{
	uint8_t d[3];
	d[0] = ret;

	if (ret == NO_ERROR) {
		d[1] = dc->ur.subcommand;
		d[2] = dc->ur.index;
		SSL_write (dc->ssl, d, 2);
	} else {
		SSL_write (dc->ssl, d, 1);
	}
}

void init_command ( )
{
}
