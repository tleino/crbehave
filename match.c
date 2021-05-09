#include "crbehave_private.h"

#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum match_type {
	MATCH_NONE, MATCH_STR, MATCH_INT, MATCH_DOUBLE
};

static void match_set_type(struct match *, int);

int
match_expect(struct match *m, const char *file, int line, int res,
    const char *exp)
{
	if (res) {
#if 0
 		printf("\tok\t%s:%d: %s\n", file, line, exp);
#endif
	} else
		printf("\tfail\t%s:%d: %s\n", file, line, exp);

	if (res)
		m->res++;
	else
		m->res = 0;

	return res;
}

int
match_expect_streq(struct match *m, const char *file, int line,
    const char *val, const char *ref)
{
	int fail = 1;
	int multiline = 0;
	const char *status = "fail";

	multiline = (
	    (ref != NULL && strchr(ref, '\n') != NULL) ||
	    (val != NULL && strchr(val, '\n') != NULL));

	if (val != NULL && ref != NULL && strcmp(val, ref) == 0) {
		fail = 0;
		status = "ok";
	}

	if (fail == 1) {
		printf("\t%s\t%s:%d:", status, file, line);

		if (multiline)
			printf("\n%s\n==\n%s\n^^\n", val, ref);
		else
			printf(" \"%s\" == \"%s\"\n", val, ref);
	}

	if (fail == 0)
		m->res++;
	else
		m->res = 0;

	return (fail == 1) ? 0 : 1;
}

int
match(struct match *m, const char *pattern, const char *str)
{
	int err;
	char buf[256];

	match_free(m);

	regcomp(&m->preg, pattern, REG_EXTENDED | REG_ICASE);

	if ((err = regexec(&m->preg, str, MATCH_MAX_SUB, m->pmatch, 0)) != 0) {
		if (err != REG_NOMATCH) {
			regerror(err, &m->preg, buf, sizeof(buf));
			warnx("%s", buf);
		}
		return 0;
	}

	m->str = str;
	m->type = 0;
	return 1;
}

int
match_int(struct match *m, int arg)
{
	int val;
	char *s;

	if (arg >= MATCH_MAX_SUB || arg < 0)
		return 0;

	s = match_str(m, arg);
	val = atoi(s);
	match_set_type(m, MATCH_INT);
	return val;
}

double
match_double(struct match *m, int arg)
{
	double val;
	char *s;

	if (arg >= MATCH_MAX_SUB || arg < 0)
		return 0;

	s = match_str(m, arg);
	val = atof(s);
	match_set_type(m, MATCH_DOUBLE);
	return val;
}

char *
match_str(struct match *m, int arg)
{
	regmatch_t *p;

	if (arg >= MATCH_MAX_SUB || arg < 0)
		return NULL;

	match_set_type(m, MATCH_STR);

	p = &m->pmatch[arg];
	m->v.str = strndup(&m->str[p->rm_so], p->rm_eo - p->rm_so);

	return m->v.str;
}

void
match_free(struct match *m)
{
	if (m->type == MATCH_STR)
		free(m->v.str);
	m->type = 0;
	regfree(&m->preg);
}

static void
match_set_type(struct match *m, int type)
{
	if (m->type == MATCH_STR) {
		free(m->v.str);
		m->v.str = NULL;
	}
	m->type = type;
}
