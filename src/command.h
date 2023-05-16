#ifndef COMMAND_H
#define COMMAND_H

#include "dc.h"

void command_handle_close_connection (struct data_client *dc);
int command_handle_login (struct data_client *dc, uint8_t *data, int data_size);
int command_handle_register (struct data_client *dc, uint8_t *data, int data_size);
int command_handle_fill_profile (struct data_client *dc, uint8_t *data, int data_size);
int command_handle_start_upload_photo (struct data_client *dc, uint8_t *data, int data_size);
int command_handle_continue_upload_photo (struct data_client *dc, uint8_t *data, int data_size);
int command_handle_finish_upload_photo (struct data_client *dc, uint8_t *data, int data_size);

void command_write_answer (struct data_client *dc, int ret);
void command_write_answer_with_upload_rs (struct data_client *dc, int ret);

void init_command ( );

#endif
