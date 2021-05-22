#include <crbehave.h>
#include <stdio.h>
#include <string.h>

#include "../canvas.h"
#include "../ppm.h"

struct canvas c;
FILE *ppm;

static void
given(struct match *m, const char *s, const char *body)
{
	if (match(m, "c is canvas\\((.*), (.*)\\)", s)) {
		CRBEHAVE_EXPECT(m, c.width = match_int(m, 1));
		CRBEHAVE_EXPECT(m, c.height = match_int(m, 2));
	}
}

static void
when(struct match *m, const char *s, const char *body)
{
	if (match(m, "ppm is canvas_to_ppm.*", s)) {
		ppm = fopen("/tmp/testppm", "w+");
		CRBEHAVE_EXPECT(m, ppm != NULL);
		if (ppm == NULL)
			return;
		ppm_write_canvas(ppm, &c);
	}
}

static void
then(struct match *m, const char *s, const char *body)
{
	char buf[1024];
	size_t n;

	if (match(m, "lines 1-3 of ppm are", s)) {
		fseek(ppm, 0L, SEEK_SET);
		n = fread(buf, 1, sizeof(buf) - 1, ppm);
		buf[n] = '\0';
		fclose(ppm);
		CRBEHAVE_EXPECT_STREQ(m, buf, body);
	}
}

int
main(int argc, char *argv[])
{
	crbehave_run(argc, argv, given, when, then, NULL);
}
