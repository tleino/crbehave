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

/*
 * CRBEHAVE_EXPECT(m, e):
 *   Like assert, evaluates expression 'e'.
 *
 *   Example:
 *     CRBEHAVE_EXPECT(m, 1 == 2);
 */
#define CRBEHAVE_EXPECT(_m, e) \
	match_expect((_m), __FILE__, __LINE__, (e), #e)
int match_expect(struct match *, const char *, int, int, const char *);

/*
 * CRBEHAVE_EXPECT_STREQ(m, val, ref):
 *   Like CRBEHAVE_EXPECT, but has special support for comparing strings
 *   for string equality, including checking for NULL and displaying the
 *   strings in the fail message.
 *
 *   Example:   
 *     CRBEHAVE_EXPECT_STREQ(m, "hello", "world");
 */
#define CRBEHAVE_EXPECT_STREQ(_m, _val, _ref) \
	match_expect_streq((_m), __FILE__, __LINE__, (_val), (_ref))
int match_expect_streq(struct match *, const char *, int, const char *,
    const char *);

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

typedef void (*KeywordCallback)(struct match *, const char *, const char *);
typedef void (*ResetCallback)(void);
int
crbehave_run(int argc, char **argv,
    KeywordCallback given, KeywordCallback when, KeywordCallback then,
    ResetCallback reset);

#endif
