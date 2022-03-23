#include <stdlib.h>
#include <string.h>
#include <spnav.h>
#include "spnavcfg.h"
#include "ui.h"

struct device_info devinfo;
struct config cfg;

int read_devinfo(struct device_info *inf)
{
	inf->name = strdup(spnav_dev_name(0, 0));
	inf->path = strdup(spnav_dev_path(0, 0));
	inf->nbuttons = spnav_dev_buttons();
	inf->naxes = spnav_dev_axes();
	inf->type = spnav_dev_type();
}

int read_cfg(struct config *cfg)
{
	int i;

	cfg->sens = spnav_cfg_get_sens();
	spnav_cfg_get_axis_sens(cfg->sens_axis);
	cfg->invert = spnav_cfg_get_invert();
	cfg->swapyz = spnav_cfg_get_swapyz();
	cfg->led = spnav_cfg_get_led();
	cfg->grab = spnav_cfg_get_grab();

	for(i=0; i<MAX_AXES; i++) {
		cfg->map_axis[i] = spnav_cfg_get_axismap(i);
		cfg->dead_thres[i] = spnav_cfg_get_deadzone(i);
	}
	for(i=0; i<MAX_BUTTONS; i++) {
		cfg->map_bn[i] = spnav_cfg_get_bnmap(i);
	}

	free(cfg->serdev);
	cfg->serdev = strdup(spnav_cfg_get_serial(0, 0));

	update_ui();
}
