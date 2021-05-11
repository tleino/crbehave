#include "crbehave_private.h"

#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <err.h>
#include <assert.h>
#include <sys/wait.h>
#include <string.h>

struct crbehave_worker {
	pid_t pid;
	int sno;
};

#define MAX_WORKERS 6
static struct pollfd pollfds[MAX_WORKERS];
static struct crbehave_worker workers[MAX_WORKERS];
static int nworkers;

/*
 * Returns 0 if the queue is full.
 */
int
crbehave_queue_worker(int sno, void (*workfunc)(int, void *), void *data)
{
	int fds[2];
	pid_t pid;

	if (nworkers == MAX_WORKERS)
		return 0;

	if (pipe(fds) != 0)
		err(1, "pipe");
	pid = fork();
	if (pid == 0) {
		close(fds[0]);
		workfunc(fds[1], data);
		close(fds[1]);
		_exit(0);
	} else {
		close(fds[1]);
		pollfds[nworkers].fd = fds[0];
		pollfds[nworkers].events = POLLIN;
		workers[nworkers].pid = pid;
		workers[nworkers].sno = sno;
		nworkers++;
	}

	return 1;
}

/*
 * Returns the number of workers still left.
 */
int
crbehave_reap_workers(int *pass, int *fail)
{
	int nready;
	int i, n;
	int status;
	unsigned char c;

	nready = poll(pollfds, nworkers, -1);
	if (nready == -1)
		err(1, "poll");
	if (nready == 0)
		errx(1, "time out");
	while (nready--) {
		for (i = 0; i < nworkers; i++) {
			if ((pollfds[i].revents & (POLLERR|POLLNVAL)))
				errx(1, "bad fd %d", pollfds[i].fd);
			if ((pollfds[i].revents & (POLLIN|POLLHUP)))
				break;
		}
		assert(i != nworkers);
		if (i == nworkers)
			break;

		if ((n = read(pollfds[i].fd, &c, 1)) != 1) {
			if (n < 0)
				warn("read");
			(*fail)++;
		} else if (c != '1') {
			(*fail)++;
		} else {
			(*pass)++;
		}
		close(pollfds[i].fd);
		pollfds[i].fd = -1;
		pollfds[i].events = 0;

		waitpid(workers[i].pid, &status, 0);
		if (WIFSIGNALED(status))
			warnx("scenario %d terminated by "
			    "signal %d", workers[i].sno,
			    WTERMSIG(status));
		nworkers--;
		memmove(&pollfds[i], &pollfds[i + 1],
		    sizeof(struct pollfd) * (nworkers - i));
		memmove(&workers[i], &workers[i + 1],
		    sizeof(*workers) * (nworkers - i));
	}

	return nworkers;
}
