#ifndef MATCH_H
#define MATCH_H

#define MATCH_MAX_SUB 16

#include <sys/types.h>
#include <regex.h>
#include <stdio.h>

struct match
{
	regex_t preg;	
	regmatch_t pmatch[MATCH_MAX_SUB];
	int type;
	const char *str;
	union {
		int val;
		char *str;
	} v;
};

typedef enum crbehave_test {
	CRBEHAVE_TEST_GIVEN, CRBEHAVE_TEST_WHEN, CRBEHAVE_TEST_THEN
} CRBehaveTest;

struct crbehave_step
{
	CRBehaveTest type;
	int (*funp)(struct match *, const char *, const char *);
	char *title;
	size_t body_offset;
	char *body;
	size_t body_alloc;
	size_t body_len;
};

int   match(struct match *, const char *, const char *);
char *match_str(struct match *, int);
int   match_int(struct match *, int);
double match_double(struct match *, int);
void  match_free(struct match *);
int   match_expect(struct match *, int);

void
crbehave_run(
    char *file,
    int (*given)(struct match *, const char *, const char *),
    int (*when)(struct match *, const char *, const char *),
    int (*then)(struct match *, const char *, const char *));

#endif
