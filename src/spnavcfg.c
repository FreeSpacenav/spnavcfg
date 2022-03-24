#include <stdlib.h>
#include <string.h>
#include <spnav.h>
#include "spnavcfg.h"
#include "ui.h"

struct device_info devinfo;
struct config cfg;

int read_devinfo(struct device_info *inf)
{
	int sz;

	free(inf->name);
	free(inf->path);
	memset(inf, 0, sizeof *inf);

	if((sz = spnav_dev_name(0, 0)) > 0) {
		if((inf->name = malloc(sz))) {
			spnav_dev_name(inf->name, sz);
		}
	}
	if((sz = spnav_dev_path(0, 0)) > 0) {
		if((inf->path = malloc(sz))) {
			spnav_dev_path(inf->path, sz);
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
	int i, sz;

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
	}

	if((sz = spnav_cfg_get_serial(0, 0)) > 0) {
		if((cfg->serdev = malloc(sz))) {
			spnav_cfg_get_serial(cfg->serdev, sz);
		}
	}

	update_ui();
	return 0;
}
