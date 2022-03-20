#ifndef SPNAVCFG_H_
#define SPNAVCFG_H_

struct device_info {
	char *name, *path;
	int nbuttons, naxes;
	int type;
};

enum {TX, TY, TZ, RX, RY, RZ};
#define MAX_BUTTONS	64
#define MAX_AXES	64

struct config {
	float sens, sens_axis[6];
	int invert;
	int map_axis[MAX_AXES];
	int dead_thres[MAX_AXES];
	int map_bn[MAX_BUTTONS];
	int led, grab;
	int repeat;
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
