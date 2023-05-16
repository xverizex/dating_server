#ifndef CMD_H
#define CMD_H

#include <stdint.h>
#include "dc.h"

enum SUBCOMMAND {
	START_UPLOAD_FREE_INDEX,
	N_SUBCOMMAND
};

struct cmd {
	struct data_client *dc;
};

struct cmd_login {
	struct cmd client;

	char *login;
	char *password;
	uint32_t len_login;
	uint32_t len_password;
};

#define MAX_INTERESTS          19
#define MAX_SEARCH              2
#define TOTAL_INTERESTS        35
#define TOTAL_FOR_WHAT         10
#define TOTAL_SEARCH            6
#define TOTAL_IAM              48

struct cmd_profile {
	struct cmd client;

	char *iam;
	char *short_bio;
	char *bio;
	uint8_t count_search;
	uint8_t search[TOTAL_SEARCH];
	uint8_t count_for_what;
	uint8_t for_what[TOTAL_FOR_WHAT];
	uint8_t day;
	uint8_t month;
	uint16_t year;
	uint8_t count_interests;
	uint8_t interests[TOTAL_INTERESTS];
	char *work;
};

#endif
