#ifndef EXPAND_H
#define EXPAND_H

/*
 * dst = expand_token(src, token, value):
 *   Expand a given src by rewriting all occurances of token with
 *   a value. The dst is a newly allocated string that needs to be freed.
 *
 *   Example:
 *     char *dst;
 *
 *     dst = expand_token("<monster> does a thing", "<monster>", "orc");
 *     puts(dst);
 *     free(dst);
 */
char* expand_token(const char *, const char *, const char *);

/*
 * dst = expand_tokens(src, tokens, values):
 *   Expand a given src by rewriting all occurances of tokens with
 *   a respective value. The dst is a newly allocated string that needs
 *   to be freed.
 *
 *   Example:
 *     char *dst;
 *     const char *tokens[] = { "monster", "victim" };
 *     const char *values[] = { "orc", "elf" };
 *
 *     dst = expand_tokens("<monster> kills <victim>", "<>", tokens, values);
 *     puts(dst);
 *     free(dst);
 */
char* expand_tokens(const char *, const char *, const char **,
    const char **);

#endif
