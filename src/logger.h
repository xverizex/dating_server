#ifndef LOGGER_H
#define LOGGER_H

enum LOGGER_LEVEL {
	LOGGER_INFO,
	LOGGER_WARN,
	LOGGER_ERROR,
	N_LOGGER_LEVEL
};

void logger_init ( );
void logger_msg (enum LOGGER_LEVEL lvl, char *msg);

#endif
