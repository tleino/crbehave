#ifndef CRBEHAVE_PRIVATE_H
#define CRBEHAVE_PRIVATE_H

#include "crbehave.h"

#include <sys/types.h>
#include <stdbool.h>
#include <regex.h>

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
	int res;
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
	bool collect_examples;
	struct crbehave_step *next;
};

struct crbehave_scenario;

struct crbehave_example {
	char *line;
	char *fields[MAX_TABLE_COLS];
	struct crbehave_example *next;
	struct crbehave_scenario *scenario;
};

struct crbehave_scenario {
	struct crbehave_step *first_step;
	struct crbehave_step *last_step;
	struct crbehave_step *last_step_with_callback;
	struct crbehave_example *example_field_names;
	struct crbehave_example *first_example;
	char *title;
	bool is_outline;
	bool collect_examples;	
	int sno;			/* scenario number */
};

int				 crbehave_queue_worker(int,
				     void (*)(int, void *), void *);
int				 crbehave_reap_workers(int *, int *);

#endif
