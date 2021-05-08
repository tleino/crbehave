#include <crbehave.h>
#include <stdio.h>

#include "../tuple.h"

struct tuple _tuple;
struct tuple _t[2];
struct tuple _p, _v, _zero;

static int
given(struct match *m, const char *s, const char *body)
{
	int i;

	if (match(m, "a is tuple\\((.*), (.*), (.*), (.*)\\)", s)) {
		tuple(&_tuple, match_double(m, 1), match_double(m, 2),
		    match_double(m, 3), match_double(m, 4));
		return 1;
	}
	if (match(m, "a(.*) is tuple\\((.*), (.*), (.*), (.*)\\)", s)) {
		if (match_int(m, 1) == 1)
			tuple(&_t[0], match_double(m, 2), match_double(m, 3),
			    match_double(m, 4), match_double(m, 5));
		else if (match_int(m, 1) == 2)
			tuple(&_t[1], match_double(m, 2), match_double(m, 3),
			    match_double(m, 4), match_double(m, 5));
		else
			return -1;
		return 1;
	}
	if (match(m, "p is point\\((.*), (.*), (.*)\\)", s)) {
		point(&_p, match_double(m, 1), match_double(m, 2),
		    match_double(m, 3));
		return 1;
	}
	if (match(m, "p(.*) is point\\((.*), (.*), (.*)\\)", s)) {
		i = match_int(m, 1);
		if (i-1 < 0 || i-1 >= sizeof(_t) / sizeof(_t[0]))
			return -1;
		point(&_t[i], match_double(m, 2), match_double(m, 3),
		    match_double(m, 4));
		return 1;
	}
	if (match(m, "v is vector\\((.*), (.*), (.*)\\)", s)) {
		vector(&_v, match_double(m, 1), match_double(m, 2),
		    match_double(m, 3));
		return 1;
	}
	if (match(m, "zero is vector\\((.*), (.*), (.*)\\)", s)) {
		vector(&_zero, match_double(m, 1), match_double(m, 2),
		    match_double(m, 3));
		return 1;
	}
	if (match(m, "v(.*) is vector\\((.*), (.*), (.*)\\)", s)) {
		i = match_int(m, 1);
		if (i-1 < 0 || i-1 >= sizeof(_t) / sizeof(_t[0]))
			return -1;
		point(&_t[i], match_double(m, 2), match_double(m, 3),
		    match_double(m, 4));
		return 1;
	}

	return -1;
}

static int
then(struct match *m, const char *s, const char *body)
{
	struct tuple t;

	if (match(m, "a.x = (.*)", s))
		return match_expect(m, _tuple.x == match_double(m, 1));
	if (match(m, "a.y = (.*)", s))
		return match_expect(m, _tuple.y == match_double(m, 1));
	if (match(m, "a.z = (.*)", s))
		return match_expect(m, _tuple.z == match_double(m, 1));
	if (match(m, "a.w = (.*)", s))
		return match_expect(m, _tuple.w == match_double(m, 1));
	if (match(m, "a is a point", s))
		return match_expect(m, tuple_is_point(&_tuple));
	if (match(m, "a is not a point", s))
		return match_expect(m, !tuple_is_point(&_tuple));
	if (match(m, "a is a vector", s))
		return match_expect(m, tuple_is_vector(&_tuple));
	if (match(m, "a is not a vector", s))
		return match_expect(m, !tuple_is_vector(&_tuple));
	if (match(m, "p = tuple\\((.*), (.*), (.*), (.*)\\)", s)) {
		tuple(&t, match_double(m, 1), match_double(m, 2),
		    match_double(m, 3), match_double(m, 4));
		return match_expect(m, tuple_is_equal(&_p, &t));
	}
	if (match(m, "v = tuple\\((.*), (.*), (.*), (.*)\\)", s)) {
		tuple(&t, match_double(m, 1), match_double(m, 2),
		    match_double(m, 3), match_double(m, 4));
		return match_expect(m, tuple_is_equal(&_v, &t));
	}
	if (match(m, "a1 \\+ a2 = tuple\\((.*), (.*), (.*), (.*)\\)", s)) {
		tuple(&t, match_double(m, 1), match_double(m, 2),
		    match_double(m, 3), match_double(m, 4));
		tuple_add(&_t[0], &_t[1]);
		return match_expect(m, tuple_is_equal(&_t[0], &t));
	}
	if (match(m, "-a = tuple\\((.*), (.*), (.*), (.*)\\)", s)) {
		tuple(&t, match_double(m, 1), match_double(m, 2),
		    match_double(m, 3), match_double(m, 4));
		tuple_neg(&_tuple);
		return match_expect(m, tuple_is_equal(&_tuple, &t));
	}
	if (match(m, "p1 - p2 = vector\\((.*), (.*), (.*)\\)", s)) {
		vector(&t, match_double(m, 1), match_double(m, 2),
		    match_double(m, 3));
		tuple_sub(&_t[0], &_t[1]);
		return match_expect(m, tuple_is_equal(&_t[0], &t));
	}
	if (match(m, "v1 - v2 = vector\\((.*), (.*), (.*)\\)", s)) {
		vector(&t, match_double(m, 1), match_double(m, 2),
		    match_double(m, 3));
		tuple_sub(&_t[0], &_t[1]);
		return match_expect(m, tuple_is_equal(&_t[0], &t));
	}
	if (match(m, "zero - v = vector\\((.*), (.*), (.*)\\)", s)) {
		vector(&t, match_double(m, 1), match_double(m, 2),
		    match_double(m, 3));
		tuple_sub(&_zero, &_v);
		return match_expect(m, tuple_is_equal(&_zero, &t));
	}
	if (match(m, "p - v = point\\((.*), (.*), (.*)\\)", s)) {
		point(&t, match_double(m, 1), match_double(m, 2),
		    match_double(m, 3));
		tuple_sub(&_p, &_v);
		return match_expect(m, tuple_is_equal(&_t[0], &t));
	}

	return -1;
}

int
main(int argc, char *argv[])
{
	crbehave_run("tuples.feature", given, NULL, then, NULL);
}
