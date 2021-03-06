#include <crbehave.h>
#include <stdio.h>
#include <string.h>

#define setmatrix(_M, row, col, val) (_M)[row * 4 + col] = val;
#define getmatrix(_M, row, col) (_M)[row * 4 + col]
double M[4 * 4];

static void
given(struct match *m, const char *s, const char *body)
{
	int i, row, col;
	if (match(m, "the following 4x4 matrix", s)) {
		if (match(m,
		    ".*\\| (.*) \\| (.*) \\| (.*) \\| (.*) \\|"
		    ".*\\| (.*) \\| (.*) \\| (.*) \\| (.*) \\|"
		    ".*\\| (.*) \\| (.*) \\| (.*) \\| (.*) \\|"
		    ".*\\| (.*) \\| (.*) \\| (.*) \\| (.*) \\|", body)) {
			i = 0;
			for (row = 0; row < 4; row++)
				for(col = 0; col < 4; col++)	
					setmatrix(M, row, col,
					    match_double(m, ++i));
		}
		CRBEHAVE_EXPECT(m, 1);
	}
}

static void
then(struct match *m, const char *s, const char *body)
{
	int row, col;
	double val;

	if (match(m, "M\\[(.*),(.*)\\] = (.*)", s)) {
		row = match_int(m, 1);
		col = match_int(m, 2);
		val = match_double(m, 3);
		CRBEHAVE_EXPECT(m, getmatrix(M, row, col) == val);
	}
}

int
main(int argc, char *argv[])
{
	crbehave_run(argc, argv, given, NULL, then, NULL);
}
