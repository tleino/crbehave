#include <crbehave.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../tuple.h"

struct context {
	struct tuple		 tuple[16];
	char			 name[16][32];
	int			 n;
};

static struct tuple		*find_tuple(const char *);

static struct context		 ctx;

static void
reset()
{
	memset(&ctx, '\0', sizeof(ctx));
}

static struct tuple *
find_tuple(const char *name)
{
	int			  i;

	for (i = 0; i < ctx.n; i++)
		if (ctx.name[i][0] != '\0' && strcmp(name, ctx.name[i]) == 0)
			return &ctx.tuple[i];

	assert(ctx.n < sizeof(ctx.tuple) / sizeof(ctx.tuple[0]));
	snprintf(ctx.name[ctx.n], sizeof(ctx.name[ctx.n]), "%s", name);
	return &ctx.tuple[ctx.n++];
}

static int
given(struct match *m, const char *s, const char *body)
{
	struct tuple *t;

	if (match(m, "(.*) is tuple\\((.*), (.*), (.*), (.*)\\)", s)) {
		t = find_tuple(match_str(m, 1));
		tuple(t, match_double(m, 2), match_double(m, 3),
		    match_double(m, 4), match_double(m, 5));
		CRBEHAVE_EXPECT(m, 1);
	}
	if (match(m, "(.*) is point\\((.*), (.*), (.*)\\)", s)) {
		t = find_tuple(match_str(m, 1));
		point(t, match_double(m, 2), match_double(m, 3),
		    match_double(m, 4));
		CRBEHAVE_EXPECT(m, 1);
	}
	if (match(m, "(.*) is vector\\((.*), (.*), (.*)\\)", s)) {
		t = find_tuple(match_str(m, 1));
		vector(t, match_double(m, 2), match_double(m, 3),
		    match_double(m, 4));
		CRBEHAVE_EXPECT(m, 1);
	}
	if (match(m, "(.*) is vector\\((.*), (.*), (.*)\\)", s)) {
		t = find_tuple(match_str(m, 1));
		vector(t, match_double(m, 2), match_double(m, 3),
		    match_double(m, 4));
		CRBEHAVE_EXPECT(m, 1);
	}
	return -1;
}

static int
then(struct match *m, const char *s, const char *body)
{
	struct tuple		*t, *t2;
	struct tuple		 ref;

	if (match(m, "^([a-z0-9]+)[.]([a-z]+) = ([0-9.]+)", s)) {
		t = find_tuple(match_str(m, 1));
		switch (match_str(m, 2)[0]) {
		case 'x': CRBEHAVE_EXPECT(m, t->x == match_double(m, 3)); break;
		case 'y': CRBEHAVE_EXPECT(m, t->y == match_double(m, 3)); break;
		case 'z': CRBEHAVE_EXPECT(m, t->z == match_double(m, 3)); break;
		case 'w': CRBEHAVE_EXPECT(m, t->w == match_double(m, 3)); break;
		}
	} else if (match(m, "^(.*) is a point", s)) {
		CRBEHAVE_EXPECT(m,
		    tuple_is_point(find_tuple(match_str(m, 1))));
	} else if (match(m, "^(.*) is not a point", s)) {
		CRBEHAVE_EXPECT(m,
		    !tuple_is_point(find_tuple(match_str(m, 1))));
	} else if (match(m, "^(.*) is a vector", s)) {
		CRBEHAVE_EXPECT(m,
		    tuple_is_vector(find_tuple(match_str(m, 1))));
	} else if (match(m, "^(.*) is not a vector", s)) {
		CRBEHAVE_EXPECT(m,
		    !tuple_is_vector(find_tuple(match_str(m, 1))));
	} else if (match(m, "^([a-z0-9]+) = tuple\\((.*), (.*), (.*), (.*)\\)", s)) {
		t = find_tuple(match_str(m, 1));
		tuple(&ref, match_double(m, 2), match_double(m, 3),
		    match_double(m, 4), match_double(m, 5));
		CRBEHAVE_EXPECT(m, tuple_is_equal(t, &ref));
	} else if (match(m,
	    "^([a-z0-9]+) \\+ ([a-z0-9]+) "
	    "= tuple\\((.*), (.*), (.*), (.*)\\)", s)) {
		t = find_tuple(match_str(m, 1));
		t2 = find_tuple(match_str(m, 2));
		tuple(&ref, match_double(m, 3), match_double(m, 4),
		    match_double(m, 5), match_double(m, 6));
		tuple_add(t, t2);
		CRBEHAVE_EXPECT(m, tuple_is_equal(t, &ref));
	} else if (match(m,
	    "^-([a-z0-9]+) = tuple\\((.*), (.*), (.*), (.*)\\)", s)) {
		t = find_tuple(match_str(m, 1));
		tuple(&ref, match_double(m, 2), match_double(m, 3),
		    match_double(m, 4), match_double(m, 5));
		tuple_neg(t);
		CRBEHAVE_EXPECT(m, tuple_is_equal(t, &ref));
	} else if (match(m,
	    "^([a-z0-9]+) - ([a-z0-9]+) = ([a-z]+)\\((.*), (.*), (.*)\\)", s)) {
		t = find_tuple(match_str(m, 1));
		t2 = find_tuple(match_str(m, 2));
		if (strcmp(match_str(m, 3), "vector") == 0)
			vector(&ref, match_double(m, 4), match_double(m, 5),
			    match_double(m, 6));
		else if (strcmp(match_str(m, 3), "point") == 0)
			point(&ref, match_double(m, 4), match_double(m, 5),
			    match_double(m, 6));
		tuple_sub(t, t2);
		CRBEHAVE_EXPECT(m, tuple_is_equal(t, &ref));
	}
	return -1;
}

int
main(int argc, char *argv[])
{
	crbehave_run(argc, argv, "tuples.feature", given, NULL, then, reset);
}
