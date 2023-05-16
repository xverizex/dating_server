#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "storage.h"
#include "dc.h"

static char dir_photos[256];
static char index_filename[256];
static int first = 1;

char *storage_get_photos ( )
{
	return dir_photos;
}

void init_storage ( )
{
	char *username = getenv ("USER");

	snprintf (dir_photos, 256, "/home/%s/photos", username);

	if (access (dir_photos, F_OK)) {
		mkdir (dir_photos, 0755);
	}

	snprintf (index_filename, 256, "%s/index_filename", dir_photos);
	if (access (index_filename, F_OK)) {
		FILE *fp = fopen (index_filename, "w");
		fprintf (fp, "@");
		fclose (fp);
	}
}

char *storage_get_next_name ( )
{
	static char filename[256];
	static char path[256];

	if (first) {
		memset (filename, 0, 256);
		first = 0;
		FILE *fp = fopen (index_filename, "r");
		fread(filename, MAX_SIZE_FILENAME_UPLOAD, 1, fp);
		fclose (fp);
	}
	int len = strlen (filename);
	int i = 0;
	for (; i < len; i++) {
		filename[i]++;
		if (filename[i] > 'Z') {
			int indexi = i + 1;
			filename[i] = 'A';
			if (filename[indexi] == 0) {
				filename[indexi] = 'A';
				len++;
				break;
			}
		} else {
			break;
		}
	}
	len = strlen (filename);

	if (i >= len) {
		filename[i] = 0;
	} else {
		filename[len] = 0;
	}

	FILE *fp = fopen (index_filename, "w");
	fprintf(fp, "%s", filename);
	fclose (fp);

	snprintf (path, 256, "%s/%s", dir_photos, filename);

	return path;
}

void storage_add_data_to (const char *filename, uint8_t *data, int data_size)
{
	FILE *fp = fopen (filename, "a");
	fwrite (data, data_size, 1, fp);
	fclose (fp);
}
