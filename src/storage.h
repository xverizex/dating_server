#ifndef STORAGE_H
#define STORAGE_H

char *storage_get_photos ( );
void init_storage ( );
char *storage_get_next_name ( );
void storage_add_data_to (const char *filename, uint8_t *data, int data_size);

#endif
