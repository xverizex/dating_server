#ifndef DC_H
#define DC_H
#include <openssl/ssl.h>
#include <stdint.h>

#define MAX_UPLOAD_IN_TIME                     8
#define MAX_SIZE_FILENAME_UPLOAD             256 
#define MAX_UPLOAD_SIZE_PHOTO          209715200

struct upload_restrictions {
	uint64_t total_size[MAX_UPLOAD_IN_TIME];
	uint32_t size;
	uint8_t free[MAX_UPLOAD_IN_TIME];
	char filename[MAX_UPLOAD_IN_TIME][MAX_SIZE_FILENAME_UPLOAD];
	uint8_t subcommand;
	uint8_t index;
};

struct data_client {
	SSL *ssl;
	SSL_CTX *ctx;
	int client;
	uint64_t ptr;
	struct upload_restrictions ur;
};

#endif
