/*
spnavcfg - an interactive GUI configurator for the spacenavd daemon.
Copyright (C) 2007-2025 John Tsiombikas <nuclear@mutantstargoat.com>

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
#ifndef SPNAVCFG_H_
#define SPNAVCFG_H_

struct device_info {
	char *name;
	char *path;
	int nbuttons, naxes;
	int type;
};

enum {TX, TY, TZ, RX, RY, RZ};
#define MAX_BUTTONS	128
#define MAX_AXES	128

struct config {
	float sens, sens_axis[6];
	int invert;
	int swapyz;
	int map_axis[MAX_AXES];
	int dead_thres[MAX_AXES];
	int map_bn[MAX_BUTTONS];
	int bnact[MAX_BUTTONS];
	int kbmap[MAX_BUTTONS];
	int led, grab;
	int repeat;
	char *serdev;
};

extern struct device_info devinfo;
extern struct config cfg;

#ifdef __cplusplus
extern "C" {
#endif

int read_devinfo(struct device_info *inf);
int read_cfg(struct config *cfg);

#ifdef __cplusplus
}
#endif

#endif	/* SPNAVCFG_H_ */
