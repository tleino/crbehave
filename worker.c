#include "crbehave_private.h"

#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <assert.h>
#include <sys/wait.h>
#include <string.h>

struct crbehave_worker {
	pid_t pid;
	int sno;
};

static struct pollfd *pollfds;
static struct crbehave_worker *workers;
static int nworkers, maxworkers;

int
init_workers(int n)
{
	maxworkers = n;
	pollfds = calloc(maxworkers, sizeof(struct pollfd));
	if (pollfds == NULL)
		return -1;
	workers = calloc(maxworkers, sizeof(struct crbehave_worker));
	if (workers == NULL)
		return -1;
	return 0;
}

void
free_workers()
{
	if (pollfds != NULL) {
		free(pollfds);
		pollfds = NULL;
	}
	if (workers != NULL) {
		free(workers);
		workers = NULL;
	}
}

/*
 * Returns 0 if the queue is full.
 */
int
crbehave_queue_worker(int sno, void (*workfunc)(int, void *), void *data)
{
	int fds[2];
	pid_t pid;

	if (nworkers == maxworkers)
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
