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
#ifndef CMD_H_
#define CMD_H_

enum {
	CMD_PING,		/* check daemon status */
	CMD_CFG,		/* update configuration (and HUP daemon) */
	CMD_STARTX,		/* signal daemon to enable X protocol */
	CMD_STOPX		/* yada yada stop X protocol */
};

#endif	/* CMD_H_ */
