#include <crbehave.h>
#include <stdio.h>
#include <string.h>

#include "../canvas.h"
#include "../ppm.h"

struct canvas c;
FILE *ppm;
static char buf[1024];

static int
given(struct match *m, const char *s, const char *body)
{
	if (match(m, "c is canvas\\((.*), (.*)\\)", s)) {
		c.width = match_int(m, 1);
		c.height = match_int(m, 2);
		return 1;
	}
	return -1;
}

static int
when(struct match *m, const char *s, const char *body)
{
	if (match(m, "ppm is canvas_to_ppm.*", s)) {
		ppm = fopen("/tmp/testppm", "w+");
		if (ppm == NULL)
			return 0;
		ppm_write_canvas(ppm, &c);
		return 1;
	}
	return -1;
}

static int
then(struct match *m, const char *s, const char *body)
{
	char buf[1024];
	int ret;
	size_t n;

	if (match(m, "lines 1-3 of ppm are", s)) {
		fseek(ppm, 0L, SEEK_SET);
		n = fread(buf, 1, sizeof(buf) - 1, ppm);
		buf[n] = '\0';
		ret = match_expect(m, strcmp(body, buf) == 0);
		fclose(ppm);
		return ret;
	}
	return -1;
}

int
main(int argc, char *argv[])
{
	crbehave_run("canvas.feature", given, when, then);
}