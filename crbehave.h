#ifndef CRBEHAVE_H
#define CRBEHAVE_H

#include <sys/types.h>

#define MATCH_MAX_SUB 256

#define MAX_TABLE_COLS 32

struct match;

int   match(struct match *, const char *, const char *);
char *match_str(struct match *, int);
int   match_int(struct match *, int);
double match_double(struct match *, int);
void  match_free(struct match *);
int   match_expect(struct match *, int);

/*
 * int nfields = parse_table_row(str, fields, max_fields):
 *   Parses table row and writes the result to fields, up to
 *   max_fields. Returns number of fields.
 *
 *   Example:
 *     char *fields[2];
 *     int nfields;
 *
 *     nfields = parse_table_row("| col1 | col2 |", fields, 2);
 *     puts(fields[0]);
 *     puts(fields[1]);
 */
int parse_table_row(char *, char **, size_t);

typedef int (*KeywordCallback)(struct match *, const char *, const char *);
void
crbehave_run(char *file,
    KeywordCallback given, KeywordCallback when, KeywordCallback then);

#endif
