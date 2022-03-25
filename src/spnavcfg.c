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
	}

	if((len = spnav_cfg_get_serial(0, 0)) > 0) {
		if((cfg->serdev = malloc(len + 1))) {
			spnav_cfg_get_serial(cfg->serdev, len + 1);
		}
	}

	update_ui();
	return 0;
}
