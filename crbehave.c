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
skip_spaces(char *line)
{
	while (*line != '\0' && isspace(*line))
		line++;

	return line;
}

static int
scenario(struct match *m, const char *s)
{
	printf("%3d. %s\n", ++scenario_number, s);
	return 2;
}

static int
run_line(
    char *line,
    size_t test_type,
    int (*given)(struct match *, const char *),
    int (*when)(struct match *, const char *),
    int (*then)(struct match *, const char *))
{
	struct {
		char *keyword;
		int (*funp)(struct match *, const char *);
	} test[] = {
		{ "Scenario: ", scenario },
		{ "Given ", given },
		{ "When ", when },
		{ "Then ", then }
	};
	size_t i, len;
	static size_t last_test = -1;
	struct match m = { 0 };
	int ret;
	char *p;

	line = skip_spaces(line);

	if (strlen(line) == 0)
		return 0;

	for (i = 0; i < sizeof(test) / sizeof(test[0]); i++) {
		len = strlen(test[i].keyword);
		if (test_type == i ||
		    strncasecmp(line, test[i].keyword, len) == 0) {
			if (test_type == -1)
				last_test = i;
			if (test[i].funp != NULL) {
				if (test_type == i) {
					p = strchr(line, ' ');
					ret = (*test[i].funp)(&m, ++p);
				} else
					ret = (*test[i].funp)(&m, &line[len]);
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
				case 2:
					break;
				case -1:
					printf("%10s: %s\n", "missing", line);
					total_missing++;
					break;
				}
			}
			match_free(&m);
			return 0;
		}
	}
	if (i == sizeof(test) / sizeof(test[0])) {
		if (strncasecmp(line, "* ", 2) == 0 ||
		    strncasecmp(line, "And ", 4) == 0) {
			if (last_test == -1)
				return -1;
			return run_line(line, last_test, given, when, then);
		}
		return -1;
	}

	return 0;
}

void
crbehave_run(
    char *file,
    int (*given)(struct match *, const char *),
    int (*when)(struct match *, const char *),
    int (*then)(struct match *, const char *))
{
	FILE *fp;
	char *line = NULL;
	size_t n;

	if ((fp = fopen(file, "r")) == NULL)
		err(1, "fopen: %s", file);

	n = 0;
	while (getline(&line, &n, fp) >= 0) {
		line[strcspn(line, "\r\n")] = '\0';
		if (run_line(line, -1, given, when, then) == -1)
			errx(1, "parse error: %s", line);
	}

	free(line);
	fclose(fp);

	printf("\n%d (pass), %d (fail), %d (missing)\n",
	    total_pass, total_fail, total_missing);
}
