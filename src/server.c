#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>
#include "logger.h"
#include "common.h"
#include "err.h"
#include "dc.h"
#include "db.h"
#include "command.h"
#include "storage.h"

/* for input client connections */
static int sockfd;

/* for ssl accept connections */
static SSL_CTX *ctx;

#define MAX_CLIENTS       1000
static struct epoll_event events[MAX_CLIENTS];

static int epollfd;
static pthread_t t1;


static void init_socket_server_connection ( )
{
	int err = sockfd = socket (AF_INET, SOCK_STREAM, 0);
	if (err == -1) {
		logger_msg (LOGGER_ERROR, string_build_msg ("socket error"));
		exit (1);
	}

	int opt = 1;
	err = setsockopt (sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof (opt));
	if (err == -1) {
		logger_msg (LOGGER_ERROR, string_build_msg ("socket option reuseport error"));
		exit (1);
	}

	struct sockaddr_in s;
	s.sin_family = AF_INET;
	s.sin_port = htons (8022);
	inet_aton ("0.0.0.0", &s.sin_addr);

	err = bind (sockfd, (const struct sockaddr *) &s, sizeof (s));
	if (err == -1) {
		logger_msg (LOGGER_ERROR, string_build_msg ("socket bind error"));
		exit (1);
	}

	listen (sockfd, 0);
}

static void init_ctx ( )
{
	const SSL_METHOD *method;

	OpenSSL_add_all_algorithms ( );
	SSL_load_error_strings ( );
	method = SSLv23_server_method ( );
	ctx = SSL_CTX_new (method);
	if (ctx == NULL) {
		ERR_print_errors_fp (stderr);
		abort ( );
	}
}

static void load_certificates (char *cert, char *key)
{
	if (SSL_CTX_use_certificate_file (ctx, cert, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp (stderr);
		abort ( );
	}

	if (SSL_CTX_use_PrivateKey_file (ctx, key, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp (stderr);
		abort ( );
	}

	if (!SSL_CTX_check_private_key (ctx)) {
		logger_msg (LOGGER_ERROR, string_build_msg ("private key does not match the public certificate"));
		abort ( );
	}
}

static void init_socket_ssl_server_connection ( )
{
	char *cert = string_build_msg ("/home/%s/cert.pem", getenv ("USER"));
	char *key = string_build_msg ("/home/%s/key.pem", getenv ("USER"));

	SSL_library_init ( );
	init_ctx ( );
	load_certificates (cert, key);

	free (cert);
	free (key);
}

static void init_epoll_queue ( )
{
	epollfd = epoll_create1 (0);	
	if (epollfd == -1) {
		logger_msg (LOGGER_ERROR, string_build_msg ("can't create epoll"));
		abort ( );
	}
}

enum HANDLE_CMD {
	CMD_LOGIN,
	CMD_REGISTER,
	CMD_FILL_PROFILE,
	CMD_UPLOAD_PHOTO,
	CMD_CONTINUE_UPLOAD_PHOTO,
	CMD_FINISH_UPLOAD_PHOTO,
	N_HANDLE_CMD
};

static void handle_input_client (struct data_client *dc, uint8_t *data, uint32_t data_size)
{
	int ret = NO_ERROR;

	switch (data[0]) {
		case CMD_LOGIN:
			ret = command_handle_login (dc, data, data_size);
			command_write_answer (dc, ret);
			break;
		case CMD_REGISTER:
			ret = command_handle_register (dc, data, data_size);
			command_write_answer (dc, ret);
			break;
		case CMD_FILL_PROFILE:
			ret = command_handle_fill_profile (dc, data, data_size);
			command_write_answer (dc, ret);
			break;
		case CMD_UPLOAD_PHOTO:
			ret = command_handle_start_upload_photo (dc, data, data_size);
			command_write_answer_with_upload_rs (dc, ret);
			break;
		case CMD_CONTINUE_UPLOAD_PHOTO:
			ret = command_handle_continue_upload_photo (dc, data, data_size);
			command_write_answer (dc, ret);
			break;
		case CMD_FINISH_UPLOAD_PHOTO:
			ret = command_handle_finish_upload_photo (dc, data, data_size);
			command_write_answer (dc, ret);
			break;
	}

	if (ret != NO_ERROR) {
		command_handle_close_connection (dc);

		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.ptr = dc;
		epoll_ctl (epollfd, EPOLL_CTL_DEL, dc->client, &ev);
		SSL_free (dc->ssl);
		close (dc->client);
		free (dc);
	}
}

static void *thread_input_clients (void *_data)
{
	(void) _data;

	uint32_t data_size = 125000;//16384;
	uint8_t *data = malloc (data_size);

	for(;;) {
		int nfds = epoll_wait (epollfd, events, MAX_CLIENTS, -1);
		if (nfds == -1) {
			logger_msg (LOGGER_ERROR, string_build_msg ("epoll wait returns -1"));
			exit (1);
		}

		for (int n = 0; n < nfds; n++) {
			struct data_client *dc = events[n].data.ptr;	

			int size = SSL_read (dc->ssl, data, data_size);
			if (size >= 1) {
				handle_input_client (dc, data, size);
			} else {
				command_handle_close_connection (dc);
				struct epoll_event ev;
				ev.events = EPOLLIN;
				ev.data.ptr = dc;
				epoll_ctl (epollfd, EPOLL_CTL_DEL, dc->client, &ev);
				SSL_free (dc->ssl);
				close (dc->client);
				free (dc);
			}
		}
	}
}

static void init_thread_for_epoll ( )
{
	pthread_create (&t1, NULL, thread_input_clients, NULL);
}


/*
 * Init connection socket and ssl library
 */
void server_connection_init ( )
{
	init_db_mysql ( );
	init_storage ( );
	init_command ( );
	init_socket_server_connection ( );
	init_socket_ssl_server_connection ( );
	init_epoll_queue ( );
	init_thread_for_epoll ( );
}

static uint32_t perform_pre_actions (SSL *ssl, int client)
{
	if (SSL_accept (ssl) == 0) {
		ERR_print_errors_fp (stderr);
		return 1;
	} else {
		struct data_client *dc = malloc (sizeof (struct data_client));
		memset (dc->ur.free, 1, MAX_UPLOAD_IN_TIME);
		dc->ctx = ctx;
		dc->client = client;
		dc->ssl = ssl;
		dc->ptr = ((uint64_t *) ssl);

		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.ptr = dc;

		int err = epoll_ctl (epollfd, EPOLL_CTL_ADD, client, &ev);
		if (err == -1) {
			logger_msg (LOGGER_ERROR, string_build_msg ("failed add client to epoll"));
			return 1;
		}
		return 0;
	}

	return 0;
}

void server_loop ( )
{
	while (1) {
		struct sockaddr_in addr;
		socklen_t len = sizeof (addr);
		SSL *ssl = NULL;

		int client = accept (sockfd, (struct sockaddr *) &addr, &len);
		logger_msg (LOGGER_INFO, string_build_msg ("connect from: %s:%d", inet_ntoa (addr.sin_addr), ntohs (addr.sin_port)));

		ssl = SSL_new (ctx);
		SSL_set_fd (ssl, client);

		if (perform_pre_actions (ssl, client)) {
			SSL_free (ssl);
			close (client);
		}
	}
}
