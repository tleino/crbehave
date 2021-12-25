# crbehave

*crbehave* is a behavior driven development framework for C that uses
standard regular expressions for matching, instead of a specialized
system, and instead of relying on macros.

The *crbehave* reads a specification of scenarios following the Gherkin
*given-when-then* syntax and calls the respective functions for making the
specification executable.

The scenarios are tested in separate worker processes for making sure
complete feature specifications are executed without unnecessary delays,
even if there are crashes or slow test code. The output should be piped
through a sort for making sure the reports are diff-able.

It was originally written by the author after opening the first pages of
Jamis Buck's excellent book, the "The Ray Tracer Challenge - A
Test-Driven Guide to Your First 3D Renderer", after finding, at that
time, there was no adequate behavior driven development framework for C
that would satisfy the author's taste.

## Dependencies

None as long as you're on a conventional Unix-like OS base system with
C compiler and standard headers installed.

## Build and run tests

Build and install:

	./configure ~
	make install

Build and run tests:

	cd examples
	./configure --enable-tests ~
	make check

Run example tests from command line:

	cd examples/tests
	for a in tuples matrix bank canvas ; do
		./${a} <${a}.feature | sort -n | tee ${a}.report
	done

## Command line options

The test programs accept the following command line options:

* **-v** Verbose output
* **-j<n>** Specify the maximum number of worker processes

The test specification is read from *stdin* and the report is pushed
to *stdout*.
