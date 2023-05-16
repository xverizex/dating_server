#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

char *string_build_msg (const char *fmt, ...)
{
	char *msg = NULL;
	int n = 0;
	size_t size = 0;
	va_list ap;

	va_start (ap, fmt);
	n = vsnprintf (msg, size, fmt, ap);
	va_end (ap);

	if (n < 0) return NULL;

	size = (size_t) n + 1;
	msg = malloc (size);

	if (msg == NULL) return NULL;

	va_start (ap, fmt);
	n = vsnprintf (msg, size, fmt, ap);
	va_end (ap);

	if (n < 0) {
		free (msg);
		return NULL;
	}

	return msg;
}
