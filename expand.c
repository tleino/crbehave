#include "expand.h"

#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <stdio.h>

static char **delimit_tokens(const char *, const char **);
static void free_tokens(char **);

char*
expand_token(const char *str, const char *token, const char *value)
{
	const char *tokens[32], **tokenp;
	size_t ntokens, token_len, value_len, len;
	const char *p;
	char *dst, *dstp;

	/*
	 * Find all tokens from the str and remember their pointers.
	 */
	p = str;
	ntokens = 0;
	token_len = strlen(token);
	while ((p = strstr(p, token)) != NULL) {
		if (ntokens < sizeof(tokens) / sizeof(tokens[0]))
			tokens[ntokens++] = p;
		p += token_len;
	}
	tokens[ntokens] = NULL;

	/*
	 * Calculate the space requirement.
	 */
	value_len = strlen(value);
	len = strlen(str) + ((value_len - token_len) * ntokens);

	/*
	 * Allocate the destination string.
	 */
	dst = malloc((len + 1) * sizeof(*dst));
	if (dst == NULL)
		err(1, "malloc");

	/*
	 * Make the destination string.
	 */
	p = str;
	dstp = dst;
	tokenp = tokens;
	while (*p != '\0') {
		if (p == *tokenp) {
			memcpy(dstp, value, value_len);
			dstp += value_len;
			p += token_len;
			tokenp++;
		} else {
			*dstp++ = *p++;
		}
	}
	*dstp = '\0';
	return dst;
}

char*
expand_tokens(const char *str, const char *delim, const char **tokens,
    const char **values)
{
	char *dst, *src;
	size_t ndst;
	const char **valuep;
	char **dtokens, **tokenp;

	dtokens = delimit_tokens(delim, tokens);

	/*
	 * XXX Horribly inefficient, but works.
	 */
	ndst = 0;
	src = strdup(str);
	if (src == NULL)
		err(1, "strdup");
	valuep = values;
	for (tokenp = dtokens; *tokenp != NULL; tokenp++, valuep++) {
		dst = expand_token(src, *tokenp, *valuep);
		free(src);
		src = strdup(dst);
		if (src == NULL)
			err(1, "strdup");
		free(dst);
	}

	free_tokens(dtokens);

	return src;
}

static char**
delimit_tokens(const char *delim, const char **tokens)
{
	static char *dst[32];
	const char **tokenp;
	char **p;
	size_t delim_len;

	p = dst;
	delim_len = strlen(delim);
	for (tokenp = tokens; *tokenp != NULL; tokenp++, p++) {
		*p = malloc(strlen(*tokenp) + 2 + 1);
		if (delim != NULL && delim_len == 2)
			sprintf(*p, "%c%s%c", delim[0], *tokenp, delim[1]);
		else
			strcpy(*p, *tokenp);
	}
	*p = NULL;

	return dst;
}

static void
free_tokens(char **tokens)
{
	char **tokenp;

	for (tokenp = tokens; *tokenp != NULL; tokenp++)
		free(*tokenp);
}

#ifdef TEST
#include <stdio.h>
#include <assert.h>

int
main(int argc, char *argv[])
{
	const char *str = "<monster> does a thing to <victim>";
	char *out, *out2;
	const char *tokens[] = { "monster", "victim", NULL };
	const char *values[] = { "orc", "elf", NULL };

	out = expand_token(str, "<monster>", "orc");
	puts(out);

	out2 = expand_token(out, "<victim>", "elf");
	puts(out2);

	free(out2);
	free(out);

	out = expand_tokens(str, "<>", tokens, values);
	puts(out);
	free(out);

	out = expand_tokens("no tokens here", "<>", tokens, values);
	puts(out);
	free(out);
}
#endif
