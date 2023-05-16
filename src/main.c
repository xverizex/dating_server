#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "logger.h"
#include "debug.h"
#include "server.h"

int main (int argc, char **argv)
{
#ifdef DEBUG
	debug_show_usage ();
#endif
	logger_init ( );
	server_connection_init ( );

	server_loop ( );
}
