/*
spnavcfg - an interactive GUI configurator for the spacenavd daemon.
Copyright (C) 2007-2009 John Tsiombikas <nuclear@siggraph.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>

void frontend(int pfd);
void backend(int pfd);

static void sig(int s);

int main(int argc, char **argv)
{
	int cpid;
	int pipefd[2];

	signal(SIGCHLD, sig);

	/*pipe(pipefd);*/
	socketpair(AF_UNIX, SOCK_STREAM, 0, pipefd);
	if((cpid = fork()) == 0) {
		/* child should be the setuid-root backend, write cfg and kill */
		close(pipefd[1]);
		backend(pipefd[0]);
		_exit(0);
	} else {
		/* parent, GUI frontend */
		close(pipefd[0]);
		frontend(pipefd[1]);
		kill(cpid, SIGTERM);
	}
	return 0;
}

static void sig(int s)
{
	if(s == SIGCHLD) {
		wait(0);
	}
}
