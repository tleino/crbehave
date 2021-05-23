#include "crbehave_private.h"
#include "expand.h"

#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

static void record_result(const char *, int, int, int);

int total_pass, total_fail, total_missing;
int scenario_number;
static int verboseflag;

#ifndef ARRLEN
#define ARRLEN(_x) sizeof((_x)) / sizeof((_x)[0])
#endif

static const char *
skip_spaces(const char *line, size_t *n_spaces_skipped)
{
	while (*line != '\0' && isspace(*line)) {
		(*n_spaces_skipped)++;
		line++;
	}

	return line;
}

static int
call_step(struct crbehave_step *self, struct crbehave_example *example)
{
	struct match m = { 0 };
	int ret;
	char *title;
	struct crbehave_scenario *scenario;

	if (self->funp == NULL)
		return -1;

	if (example != NULL && example->line != NULL) {
		scenario = example->scenario;
		title = expand_tokens(self->title, "<>",
		    (const char **) scenario->example_field_names->fields,
		    (const char **) example->fields);
	} else
		title = strdup(self->title);

	(*self->funp)(&m, title, self->body);

	record_result(title, self->scenario->sno, self->type, m.res ? 1 : 0);

	free(title);

	match_free(&m);

	return ret;
}

static void
record_result(const char *line, int sno, int type, int ret)
{
	char *typestr;

	switch (type) {
	case CRBEHAVE_TEST_GIVEN:
		typestr = "GIVEN";
		break;
	case CRBEHAVE_TEST_WHEN:
		typestr = "WHEN";
		break;
	case CRBEHAVE_TEST_THEN:
		typestr = "THEN";
		break;
	}

	switch (ret) {
	case 0:
		printf("%3d\t%s\t%s\t%s\n", sno, "fail", typestr, line);
		total_fail++;
		break;
	case 1:
		if (verboseflag)
			printf("%3d\t%s\t%s\t%s\n", sno, "pass", typestr,
			    line);
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

static struct crbehave_example *
add_example(
   struct crbehave_scenario *scenario, const char *line)
{
	struct crbehave_example *example;

	example = calloc(1, sizeof(struct crbehave_example));
	if (example == NULL)
		err(1, "calloc");

	example->line = strdup(line);
	if (example->line == NULL)
		err(1, "strdup");

	parse_table_row(example->line, example->fields,
	    ARRLEN(example->fields));

	if (scenario->example_field_names == NULL)
		scenario->example_field_names = example;
	else if (scenario->first_example == NULL)
		scenario->first_example = example;
	else
		scenario->first_example->next = example;

	example->scenario = scenario;

	return example;
}

/*
 * Add a new step to a scenario, returning NULL if there is trouble
 * with the arguments, namely a NULL function pointer when a pointer
 * was expected which indicates a parse error situation.
 */
static struct crbehave_step *
add_step(
    struct crbehave_scenario *scenario,
    const char *title,
    KeywordCallback funp,
    int type)
{
	struct crbehave_step *step, *n;

	step = calloc(1, sizeof(struct crbehave_step));	
	if (step == NULL)
		err(1, "calloc");

	step->title = strdup(title);
	step->funp = funp;
	step->scenario = scenario;
	step->type = type;

	/*
	 * Make sure the step always has a function pointer to call,
	 * even if we got here with a NULL function pointer: that only
	 * indicates we're executing "and"-keyword and should reuse
	 * the last function pointer that was assigned.
	 */
	if (step->funp != NULL)
		scenario->last_step_with_callback = step;
	else if (scenario->last_step_with_callback != NULL) {
		step->funp = scenario->last_step_with_callback->funp;
		step->type = scenario->last_step_with_callback->type;
	} else {
		printf("add_step returned null for %s\n", step->title);
		return NULL;
	}

	n = scenario->last_step;
	if (n == NULL)
		scenario->first_step = step;
	else
		n->next = step;
	scenario->last_step = step;

	return step;
}

/*
 * Parses line into a scenario, adding a new step, a new example or
 * extending an existing step. Returns -1 if we've got a parse error
 * situation.
 */
static int
parse_line(
    const char *line,
    struct crbehave_scenario *scenario,
    KeywordCallback given,
    KeywordCallback when,
    KeywordCallback then)
{
	size_t n_spaces_skipped = 0;
	struct {
		char *keyword;
		KeywordCallback funp;
		int type;
	} test[] = {
		{ "Given ", given, CRBEHAVE_TEST_GIVEN },
		{ "When ", when, CRBEHAVE_TEST_WHEN },
		{ "Then ", then, CRBEHAVE_TEST_THEN },
		{ "And ", NULL, 0 },
		{ "* ", NULL, 0 }
	};
	size_t i, len;
	char *p;
	struct crbehave_step *step;

	line = skip_spaces(line, &n_spaces_skipped);

	if (strlen(line) == 0)
		return 0;

	if (strcasecmp(line, "Examples:") == 0) {
		scenario->collect_examples = true;
		return 0;
	}
	if (scenario->collect_examples && line[0] == '|') {
		if (add_example(scenario, line) == NULL)
			return -1;
		return 0;
	} else if (scenario->collect_examples)
		scenario->collect_examples = false;

	/*
	 * Find keyword and create a new step or extend an existing step.
	 */
	for (i = 0; i < ARRLEN(test); i++) {
		len = strlen(test[i].keyword);
		if (strncasecmp(line, test[i].keyword, len) == 0) {
			step = add_step(scenario, &line[len], test[i].funp,
			    test[i].type);
			return (step != NULL) ? 1 : -1;
		}
	}
	step = scenario->last_step;
	if (step == NULL) {
		printf("did not find keyword for line %s\n", line);
		return -1;
	}

	/*
	 * Extend an existing step by parsing its body.
	 */
	if (strncasecmp(line, "\"\"\"", 3) == 0) {
		step->body_offset = n_spaces_skipped;
		return 0;
	}

	len = strlen(line) + 1;
	if (step->body_alloc == 0) {
		step->body_alloc = 512;
		step->body = realloc(step->body, step->body_alloc + 1);
	}
	if (step->body_len + len >= step->body_alloc) {
		step->body_alloc *= 2;
		step->body = realloc(step->body, step->body_alloc + 1);
	}
	p = strcpy(&step->body[step->body_len], line);
	step->body_len += (len - 1);
	step->body[step->body_len] = '\n';
	step->body_len++;
	step->body[step->body_len] = '\0';
	return 0;
}

static void
run_scenario(struct crbehave_scenario *self, struct crbehave_example *example)
{
	struct crbehave_step *step;

	if (self->example_field_names != NULL && example == NULL) {
		example = self->first_example;
		for (; example != NULL; example = example->next)
			run_scenario(self, example);
	} else {
		for (step = self->first_step; step != NULL; step = step->next)
			call_step(step, example);
	}

	if (self->title != NULL && example == NULL)
		printf("%3d: %s\n", self->sno, self->title);
}

static void
free_step(struct crbehave_step *self)
{
	if (self->title != NULL)
		free(self->title);
	if (self->body != NULL)
		free(self->body);
	free(self);
}

static void
free_steps(struct crbehave_scenario *self)
{
	struct crbehave_step *n, *next;

	for (n = self->first_step; n != NULL; n = next) {
		next = n->next;
		free_step(n);
	}
}

static void
free_example(struct crbehave_example *self)
{
	if (self->line != NULL) {
		free(self->line);
		self->line = NULL;
	}
	free(self);
}

static void
free_examples(struct crbehave_scenario *self)
{
	struct crbehave_example *n, *next;

	if (self->example_field_names != NULL)
		free_example(self->example_field_names);
	for (n = self->first_example; n != NULL; n = next) {
		next = n->next;
		free_example(n);
	}
}

static void
free_scenario(struct crbehave_scenario *self)
{
	free_steps(self);
	free_examples(self);

	if (self->title != NULL)
		free(self->title);

	free(self);
}

static void
add_scenario(struct crbehave_scenario **head, const char *title,
    bool is_outline, int sno)
{
	struct crbehave_scenario *self;

	self = calloc(1, sizeof(struct crbehave_scenario));
	if (self == NULL)
		err(1, "calloc");

	if (*head != NULL)
		self->next = *head;
	*head = self;

	self->title = strdup(title);
	if (self->title == NULL)
		err(1, "strdup");
	self->is_outline = is_outline;
	self->sno = sno;
}

struct workfunc
{
	int sno;
	char *arg0;
	char *file;
	KeywordCallback given;
	KeywordCallback when;
	KeywordCallback then;
	ResetCallback reset;
	struct crbehave_scenario *scenario;
	struct crbehave_scenario *head;
};

static void
workfunc(int fd, void *data)
{
	struct workfunc *wf = (struct workfunc *) data;
	char arg[32], *args[3];
	struct crbehave_scenario *np, *next;

	snprintf(arg, sizeof(arg), "%d", wf->sno);
	args[0] = wf->arg0;
	args[1] = arg;
	args[2] = NULL;

	run_scenario(wf->scenario, NULL);
	if (total_fail == 0)
		write(fd, "1", 1);
	else
		write(fd, "0", 1);

	/*
	 * Make analyzers happy, even though after workfunc exits,
	 * the process exits.
	 */
	for (np = wf->head; np != NULL; np = next) {
		next = np->next;
		free_scenario(np);
	}
	free_workers();
}

int
crbehave_run(
    int argc,
    char **argv,
    KeywordCallback given, KeywordCallback when, KeywordCallback then,
    ResetCallback reset)
{
	static FILE *fp;
	char *line = NULL;
	size_t i, n, len;
	struct crbehave_scenario *scenario = NULL, *np, *next, *cs;
	static const struct {
		const char *str;
		bool is_outline;
	} heading[] = {
		{ .str = "Scenario outline: ", .is_outline = true },
		{ .str = "Scenario: ", .is_outline = false }
	};
	int sno = 0;		/* scenario number */
	int rsno = 0;		/* scenario number to run */
	int fail = 0, pass = 0;
	int ch, njobs = 6;
	char *arg0 = argv[0];

	/*
	 * We need to explicitly set optind here, because we're re-entering
	 * here from the worker process and we need to reset the optind
	 * back to the original value.
	 */
	optind = 1;

	while ((ch = getopt(argc, argv, "vj:")) != -1) {
		switch (ch) {
		case 'j':
			njobs = atoi(optarg);
			if (njobs == 0 || njobs < 0 || njobs > 999)
				errx(1, "invalid max jobs limit %d "
				    "(should be 1-999)", njobs);
			break;
		case 'v':
			verboseflag = 1;
			break;
		default:
			fprintf(stderr, "Usage: %s [-v] [-j <jobs>] "
			    "[scenario]\n", arg0);
			exit(1);
		}
	}
	argc -= optind;
	argv += optind;

	if (fp == NULL && (fp = fdopen(STDIN_FILENO, "r")) == NULL)
		err(1, "fdopen");

	if (*argv)
		rsno = atoi(*argv);
	else
		if (init_workers(njobs) == -1)
			err(1, "init_workers");

	/*
	 * Parse the scenarios.
	 */
	n = 0;
	cs = NULL;
	while (getline(&line, &n, fp) >= 0) {
		line[strcspn(line, "\r\n")] = '\0';

		for (i = 0; i < ARRLEN(heading); i++) {
			len = strlen(heading[i].str);
			if (strncasecmp(line, heading[i].str, len) == 0) {
				if (reset != NULL)
					reset();
				sno++;
				if (rsno == sno || rsno == 0) {
					add_scenario(&scenario, &line[len],
					    heading[i].is_outline, sno);
					cs = scenario;
				} else
					cs = NULL;
				break;
			}
		}
		if (i < ARRLEN(heading))
			continue;
	
		if (cs != NULL &&
		    parse_line(line, cs, given, when, then) == -1)
			errx(1, "parse error: %s", line);
	}
	free(line);
	fclose(fp);

	/*
	 * Queue the scenarios or run one directly.
	 */
	for (np = scenario; np != NULL; np = next) {
		next = np->next;

		if (np->sno == rsno) {
			run_scenario(np, NULL);
			free_scenario(np);
			pass = total_pass;
			fail = total_fail;
			scenario = NULL;
		} else if (rsno == 0) {
			struct workfunc wf = { 0 };

			wf.scenario = np;
			wf.head = scenario;
			while (crbehave_queue_worker(
			    np->sno, workfunc, &wf) == 0)
				crbehave_reap_workers(&pass, &fail);
		}
	}
	if (rsno == 0)
		while (crbehave_reap_workers(&pass, &fail) > 0)
			;

	for (np = scenario; np != NULL; np = next) {
		next = np->next;
		free_scenario(np);
	}

	if (reset != NULL)
		reset();

	printf("%s: %d (pass) %d (fail)\n", arg0, pass, fail);

	free_workers();

	if (total_fail > 0)
		return 1;
	else
		return 0;
}
