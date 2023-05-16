#ifndef DB_H
#define DB_H
#include "dc.h"
#include "cmd.h"

void db_user_close_connection (struct data_client *dc);
int db_user_auth (struct cmd_login *c);
int db_user_register (struct cmd_login *c);
int db_fill_profile (struct cmd_profile *c);
void init_db_mysql (void);

#endif
