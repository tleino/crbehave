#include "crbehave.h"

#include <string.h>
#include <ctype.h>

int
parse_table_row(char *str, char **fields, size_t max_fields)
{
	char *p;
	size_t n = 0;

	p = str;
	do {
		p = strchr(p, '|');
		while (p != NULL && *p != '\0' && isspace(*++p))
			;

		if (n < max_fields && *p != '\0')
			fields[n++] = p;
		else
			break;

		while (p != NULL && *p != '\0' && !isspace(*++p))
			;
		if (p != NULL)
			*p++ = '\0';
	} while (p != NULL);

	fields[n] = NULL;
	return n;
}
