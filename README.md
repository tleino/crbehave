# crbehave

_crbehave_ is a behavior driven development framework for C that uses standard regular expressions for matching, instead of a specialized system, and instead of relying on macros.

The _crbehave_ reads a specification of a behavior that follows the _Gherkin_
given-when-then syntax.

The _crbehave_ is originally written for OpenBSD, but should be portable to most POSIX-compatible systems without trouble. There is no magic or complexity, simply compile the crbehave.c file and link it to your test executable.

It was originally written by the author after opening the first pages of Jamis Buck's excellent book, the "The Ray Tracer Challenge - A Test-Driven Guide to Your First 3D Renderer", after finding, at that time, there was no adequate behavior driven development framework for C that would satisfy the author's taste. In particular, the author wanted to mimic the simplicity of the original Ruby framework that uses simple regular expressions.
