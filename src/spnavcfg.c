/*
spnavcfg - an interactive GUI configurator for the spacenavd daemon.
Copyright (C) 2007-2022 John Tsiombikas <nuclear@siggraph.org>

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
#include <stdlib.h>
#include <string.h>
#include <spnav.h>
#include "spnavcfg.h"
#include "ui.h"

struct device_info devinfo;
struct config cfg;

int read_devinfo(struct device_info *inf)
{
	int len;

	free(inf->name);
	free(inf->path);
	memset(inf, 0, sizeof *inf);

	if((len = spnav_dev_name(0, 0)) > 0) {
		if((inf->name = malloc(len + 1))) {
			spnav_dev_name(inf->name, len + 1);
		}
	}
	if((len = spnav_dev_path(0, 0)) > 0) {
		if((inf->path = malloc(len + 1))) {
			spnav_dev_path(inf->path, len + 1);
		}
	}
	if((inf->nbuttons = spnav_dev_buttons()) > MAX_BUTTONS) {
		inf->nbuttons = MAX_BUTTONS;
	}
	if((inf->naxes = spnav_dev_axes()) > MAX_AXES) {
		inf->naxes = MAX_AXES;
	}
	inf->type = spnav_dev_type();
	return 0;
}

int read_cfg(struct config *cfg)
{
	int i, len;

	free(cfg->serdev);
	memset(cfg, 0, sizeof *cfg);

	cfg->sens = spnav_cfg_get_sens();
	spnav_cfg_get_axis_sens(cfg->sens_axis);
	cfg->invert = spnav_cfg_get_invert();
	cfg->swapyz = spnav_cfg_get_swapyz();
	cfg->led = spnav_cfg_get_led();
	cfg->grab = spnav_cfg_get_grab();

	for(i=0; i<devinfo.naxes; i++) {
		cfg->map_axis[i] = spnav_cfg_get_axismap(i);
		cfg->dead_thres[i] = spnav_cfg_get_deadzone(i);
	}
	for(i=0; i<devinfo.nbuttons; i++) {
		cfg->map_bn[i] = spnav_cfg_get_bnmap(i);
		cfg->bnact[i] = spnav_cfg_get_bnaction(i);
		cfg->kbmap[i] = spnav_cfg_get_kbmap(i);
	}

	if((len = spnav_cfg_get_serial(0, 0)) > 0) {
		if((cfg->serdev = malloc(len + 1))) {
			spnav_cfg_get_serial(cfg->serdev, len + 1);
		}
	}

	cfg->repeat = spnav_cfg_get_repeat();

	update_ui();
	return 0;
}
