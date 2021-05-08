#include <crbehave.h>
#include <stdio.h>
#include <string.h>

struct account
{
	char *name;
	int balance;
};

static struct account accounts[2];

static struct account *a;

static struct account *
find_account(const char *name)
{
	if (strcmp(name, "alice") == 0)
		return &accounts[0];
	else if (strcmp(name, "bob") == 0)
		return &accounts[1];
	return NULL;
}

static int
given(struct match *m, const char *s, const char *body)
{
	if (match(m, "(.*) logs into bank and has balance of (.*) coins",
	    s)) {
		if ((a = find_account(match_str(m, 1))) != NULL) {
			a->balance = match_int(m, 2);
			return 1;
		}
	}
	return -1;
}

static int
when(struct match *m, const char *s, const char *body)
{
	if (match(m, "(.*) withdraws (.*) coins", s)) {
		if ((a = find_account(match_str(m, 1))) != NULL) {
			a->balance -= match_int(m, 2);
			return 1;
		}
	}
	return -1;
}

static int
then(struct match *m, const char *s, const char *body)
{
	if (match(m, "(.*) has (.*) coins left", s)) {
		if ((a = find_account(match_str(m, 1))) != NULL)
			return match_expect(m, a->balance ==
			    match_int(m, 2));
	}
	return -1;
}

int
main(int argc, char *argv[])
{
	crbehave_run("bank.feature", given, when, then, NULL);
}
