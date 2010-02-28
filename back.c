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
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include "cfgfile.h"
#include "cmd.h"

#define CFGFILE		"/etc/spnavrc"
#define PIDFILE		"/var/run/spnavd.pid"

int get_daemon_pid(void);
static int update_cfg(void);
static void sig(int s);

static struct cfg cfg;
static int dpid = -1;

int backend(int pfd)
{
	signal(SIGTERM, sig);

	for(;;) {
		ssize_t res;
		int cmd, tmp;

		/* get command */
		cmd = 0;
		if(read(pfd, &cmd, 1) == -1 && errno != EINTR) {
			perror("pipe read blew up in my face! wtf");
			return -1;
		}

		switch(cmd) {
		case CMD_PING:
			tmp = (dpid = get_daemon_pid()) != -1;
			write(pfd, &tmp, 1);
			break;

		case CMD_CFG:
			{
				char *buf = (char*)&cfg;
				int sz = sizeof cfg;

				while(sz && (res = read(pfd, buf, sz)) > 0) {
					buf += res;
					sz -= res;
				}
				update_cfg();
			}
			break;

		case CMD_STARTX:
		case CMD_STOPX:
			if(dpid == -1) {
				if((dpid = get_daemon_pid()) == -1) {
					return -1;
				}
			}
			if(kill(dpid, cmd == CMD_STARTX ? SIGUSR1 : SIGUSR2) == -1) {
				if(errno != EPERM) {
					dpid = -1;
					kill(getppid(), SIGUSR2);
				}
			}
			break;

		default:
			fprintf(stderr, "unknown CMD: %d\n", (int)cmd);
			break;
		}
	}

	return 0;
}

int get_daemon_pid(void)
{
	FILE *fp;
	char buf[64];

	if(!(fp = fopen(PIDFILE, "r"))) {
		fprintf(stderr, "no spacenav pid file, can't find daemon\n");
		return -1;
	}
	fgets(buf, sizeof buf, fp);
	fclose(fp);

	if(!isdigit(buf[0])) {
		fprintf(stderr, "fucked up pidfile, can't find daemon\n");
		return -1;
	}
	return atoi(buf);
}

static int update_cfg(void)
{
	if(write_cfg(CFGFILE, &cfg) == -1) {
		fprintf(stderr, "failed to update config file\n");
		return -1;
	}

	if(dpid == -1) {
		if((dpid = get_daemon_pid()) == -1) {
			return -1;
		}
	}

	if(kill(dpid, SIGHUP) == -1 && errno != EPERM) {
		dpid = -1;	/* invalidate pid, will be searched again next time */
		return -1;
	}

	return 0;
}

static void sig(int s)
{
	_exit(0);
}
