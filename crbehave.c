#include "crbehave_private.h"
#include "expand.h"

#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <ctype.h>

static void record_result(const char *line, int ret);

int total_pass, total_fail, total_missing;
int scenario_number;

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

	ret = (*self->funp)(&m, title, self->body);

	record_result(title, ret);

	free(title);

	match_free(&m);

	return ret;
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
    KeywordCallback funp)
{
	struct crbehave_step *step, *n;

	step = calloc(1, sizeof(struct crbehave_step));	
	if (step == NULL)
		err(1, "calloc");

	step->title = strdup(title);
	step->funp = funp;

	/*
	 * Make sure the step always has a function pointer to call,
	 * even if we got here with a NULL function pointer: that only
	 * indicates we're executing "and"-keyword and should reuse
	 * the last function pointer that was assigned.
	 */
	if (step->funp != NULL)
		scenario->last_step_with_callback = step;
	else if (scenario->last_step_with_callback != NULL)
		step->funp = scenario->last_step_with_callback->funp;
	else {
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
		int (*funp)(struct match *, const char *, const char *);
	} test[] = {
		{ "Given ", given },
		{ "When ", when },
		{ "Then ", then },
		{ "And ", NULL },
		{ "* ", NULL }
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
#if 0
		printf("Should store example: %s\n", line);
		nfields = parse_table_row(strdup(line), fields, ARRLEN(fields));
		printf("Got %d fields\n", nfields);
		for (field = fields; *field != NULL; field++)
			printf("Field: %s\n", *field);
		return 0;
#endif
	} else if (scenario->collect_examples)
		scenario->collect_examples = false;

	/*
	 * Find keyword and create a new step or extend an existing step.
	 */
	for (i = 0; i < ARRLEN(test); i++) {
		len = strlen(test[i].keyword);
		if (strncasecmp(line, test[i].keyword, len) == 0) {
			step = add_step(scenario, &line[len], test[i].funp);
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
		return;
	}

	for (step = self->first_step; step != NULL; step = step->next)
		call_step(step, example);
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
clear_scenario(struct crbehave_scenario *self)
{
	free_steps(self);
	free_examples(self);

	if (self->title != NULL)
		free(self->title);

	memset(self, '\0', sizeof(struct crbehave_scenario));
}

static void
init_scenario(struct crbehave_scenario *self, const char *title,
    bool is_outline)
{
	self->title = strdup(title);
	if (self->title == NULL)
		err(1, "strdup");
	self->is_outline = is_outline;
	if (self->title != NULL)
		puts(self->title);
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
	size_t i, n, len;
	struct crbehave_scenario scenario = { 0 };
	static const struct {
		const char *str;
		bool is_outline;
	} heading[] = {
		{ .str = "Scenario outline: ", .is_outline = true },
		{ .str = "Scenario: ", .is_outline = false }
	};

	if ((fp = fopen(file, "r")) == NULL)
		err(1, "fopen: %s", file);

	n = 0;
	while (getline(&line, &n, fp) >= 0) {
		line[strcspn(line, "\r\n")] = '\0';

		for (i = 0; i < ARRLEN(heading); i++) {
			len = strlen(heading[i].str);
			if (strncasecmp(line, heading[i].str, len) == 0) {
				run_scenario(&scenario, NULL);
				clear_scenario(&scenario);
				init_scenario(&scenario, &line[len],
				    heading[i].is_outline);
				break;
			}
		}
		if (i < ARRLEN(heading))
			continue;
	
		if (parse_line(line, &scenario, given, when, then) == -1)
			errx(1, "parse error: %s", line);
	}
	run_scenario(&scenario, NULL);
	clear_scenario(&scenario);

	free(line);
	fclose(fp);

	printf("\n%d (pass), %d (fail), %d (missing)\n",
	    total_pass, total_fail, total_missing);
}
