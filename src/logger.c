#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "logger.h"

static FILE *fp_logger;

static char *set_level_string (enum LOGGER_LEVEL lvl)
{
	switch (lvl) {
		case LOGGER_INFO: return "info";
		case LOGGER_WARN: return "warning";
		case LOGGER_ERROR: return "error";
	}

	return NULL;
}

static char *build_with_cur_time (char *level, char *msg)
{
	static char out_msg[512];

	time_t t = time (0);

	char *localtime = strdup (ctime (&t));

	char *end = strchr (localtime, '\n');
	if (end) *end = 0;

	snprintf (out_msg, 512, "%s %s: %s",
		localtime,
		level,
		msg
	 	);


	free (localtime);

	return out_msg;
}

void logger_init ( )
{
	char *user = getenv ("USER");
	char line[255];
	snprintf (line, 255, "/home/%s/dating.log", user);

	fp_logger = fopen (line, "a");
}

/*
 * msg - needs to memory from heap
 */
void logger_msg (enum LOGGER_LEVEL lvl, char *msg)
{
	assert (lvl < N_LOGGER_LEVEL);

	char *level = set_level_string (lvl);

	char *out_msg = build_with_cur_time (level, msg);

	fprintf (fp_logger, "%s\n", out_msg);
}
