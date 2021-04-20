#include "crbehave.h"

#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <ctype.h>

enum match_type {
	MATCH_NONE, MATCH_STR, MATCH_INT, MATCH_DOUBLE
};

static void match_set_type(struct match *, int);
static void record_result(const char *line, int ret);

int total_pass, total_fail, total_missing;
int scenario_number;

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

static void
match_set_type(struct match *m, int type)
{
	if (m->type == MATCH_STR) {
		free(m->v.str);
		m->v.str = NULL;
	}
	m->type = type;
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

static char *
skip_spaces(char *line, size_t *n_spaces_skipped)
{
	while (*line != '\0' && isspace(*line)) {
		(*n_spaces_skipped)++;
		line++;
	}

	return line;
}

static int
scenario(struct match *m, const char *s, const char *body)
{
	printf("%3d. %s\n", ++scenario_number, s);
	return 2;
}

static int
call_step(struct crbehave_step *self)
{
	struct match m = { 0 };
	int ret;

	if (self->funp == NULL)
		return -1;

	ret = (*self->funp)(&m, self->title, self->body);

	record_result(self->title, ret);

	if (self->title != NULL)
		free(self->title);
	if (self->body != NULL)
		free(self->body);
	self->title = NULL;
	self->body = NULL;
	self->body_len = 0;
	self->body_alloc = 0;

	match_free(&m);

	return ret;
}

/*
 * Returns 1 if step was called (ret is set to return value).
 * Returns 0 if step was reading body content.
 * Returns -1 if error.
 */
static int
parse_step(
    struct crbehave_step *self,
    const char *line,
    size_t offset,
    int *ret,
    int (*given)(struct match *, const char *, const char *),
    int (*when)(struct match *, const char *, const char *),
    int (*then)(struct match *, const char *, const char *))
{
	struct {
		char *keyword;
		int (*funp)(struct match *, const char *, const char *);
	} test[] = {
		{ "Scenario: ", scenario },
		{ "Given ", given },
		{ "When ", when },
		{ "Then ", then }
	};
	size_t i, len;
	char *p;

	if ((strncasecmp(line, "And ", 4) == 0 ||
	    strncasecmp(line, "* ", 2) == 0) &&
	    (self->funp == given || self->funp == when ||
	    self->funp == then)) {
		*ret = call_step(self);
		self->title = strdup(&line[4]);
		return 1;
	}

	for (i = 0; i < sizeof(test) / sizeof(test[0]); i++) {
		len = strlen(test[i].keyword);
		if (strncasecmp(line, test[i].keyword, len) == 0) {
			*ret = call_step(self);
			self->title = strdup(&line[len]);
			self->funp = test[i].funp;
			return 1;
		}
	}

	if (self->funp != given && self->funp != when && self->funp != then)
		return -1;

	if (strncasecmp(line, "\"\"\"", 3) == 0) {
		self->body_offset = offset;
		return 0;
	}

	len = strlen(line) + 1;
	if (self->body_alloc == 0) {
		self->body_alloc = 512;
		self->body = realloc(self->body, self->body_alloc + 1);
	}
	if (self->body_len + len >= self->body_alloc) {
		self->body_alloc *= 2;
		self->body = realloc(self->body, self->body_alloc + 1);
	}
	p = strcpy(&self->body[self->body_len], line);
	self->body_len += (len - 1);
	self->body[self->body_len] = '\n';
	self->body_len++;
	self->body[self->body_len] = '\0';

	return 0;
}

static void
record_result(const char *line, int ret)
{
	switch (ret) {
	case 0:
		printf("%10s: %s\n", "fail", line);
		total_fail++;
		break;
	case 1:
#if 0
		printf("%10s: %s\n", "pass", line);
#endif
		total_pass++;
		break;
	case 2:	/* do not count, do not show, e.g. Scenario */
		break;
	case -1:
		printf("%10s: %s\n", "missing", line);
		total_missing++;
		break;
	}
}

static int
run_line(
    char *line,
    struct crbehave_step *step,
    int (*given)(struct match *, const char *, const char *),
    int (*when)(struct match *, const char *, const char *),
    int (*then)(struct match *, const char *, const char *))
{
	int ret;
	size_t n_spaces_skipped = 0;

	line = skip_spaces(line, &n_spaces_skipped);

	if (strlen(line) == 0)
		return 0;

	switch (parse_step(step, line, n_spaces_skipped, &ret,
	    given, when, then)) {
	case 1:	/* step was executed */
		break;
	case 0: /* step was not executed */
		break;
	case -1: /* error */
		step->funp = NULL;
		return -1;		
		break;
	}

	return 0;
}

void
crbehave_run(
    char *file,
    int (*given)(struct match *, const char *, const char *),
    int (*when)(struct match *, const char *, const char *),
    int (*then)(struct match *, const char *, const char *))
{
	FILE *fp;
	char *line = NULL;
	size_t n;
	struct crbehave_step step = { 0 };

	if ((fp = fopen(file, "r")) == NULL)
		err(1, "fopen: %s", file);

	n = 0;
	while (getline(&line, &n, fp) >= 0) {
		line[strcspn(line, "\r\n")] = '\0';
		if (run_line(line, &step, given, when, then) == -1)
			errx(1, "parse error: %s", line);
	}

	free(line);
	fclose(fp);

	call_step(&step);

	printf("\n%d (pass), %d (fail), %d (missing)\n",
	    total_pass, total_fail, total_missing);
}
