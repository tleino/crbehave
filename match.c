#include "crbehave_private.h"

#include <err.h>
#include <stdlib.h>
#include <string.h>

enum match_type {
	MATCH_NONE, MATCH_STR, MATCH_INT, MATCH_DOUBLE
};

static void match_set_type(struct match *, int);

int
match_expect(struct match *m, int val)
{
	if (val == 0)
		return 0;
	else
		return 1;
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
