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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <spnav.h>
#include "ui.h"

#define CHK_AXINV_TRANS_X			"axinv_trans_x"
#define CHK_AXINV_TRANS_Y			"axinv_trans_y"
#define CHK_AXINV_TRANS_Z			"axinv_trans_z"

#define CHK_AXINV_ROT_X				"axinv_rot_x"
#define CHK_AXINV_ROT_Y				"axinv_rot_y"
#define CHK_AXINV_ROT_Z				"axinv_rot_z"

#define CHK_SWAP_YZ					"swap_yz"

#define CHK_ENABLE_LED				"enable_led"
#define CHK_GRAB_DEVICE				"grab_device"

#define SLIDER_SENS_GLOBAL			"sens_global"

#define SLIDER_SENS_TRANS			"sens_trans"
#define SLIDER_SENS_TRANS_X			"sens_trans_x"
#define SLIDER_SENS_TRANS_Y			"sens_trans_y"
#define SLIDER_SENS_TRANS_Z			"sens_trans_z"

#define SLIDER_SENS_ROT				"sens_rot"
#define SLIDER_SENS_ROT_X			"sens_rot_x"
#define SLIDER_SENS_ROT_Y			"sens_rot_y"
#define SLIDER_SENS_ROT_Z			"sens_rot_z"

#define SLIDER_DEADZONE				"deadzone"
#define SLIDER_DEADZONE_TRANS_X		"deadzone_trans_x"
#define SLIDER_DEADZONE_TRANS_Y		"deadzone_trans_y"
#define SLIDER_DEADZONE_TRANS_Z		"deadzone_trans_z"
#define SLIDER_DEADZONE_ROT_X		"deadzone_rot_x"
#define SLIDER_DEADZONE_ROT_Y		"deadzone_rot_y"
#define SLIDER_DEADZONE_ROT_Z		"deadzone_rot_z"

#define BTN_PING					"ping_daemon"


static void create_ui(void);

G_MODULE_EXPORT void chk_handler(GtkToggleButton *bn, gpointer data);
G_MODULE_EXPORT void slider_handler(GtkRange *rng, gpointer data);
G_MODULE_EXPORT void bn_handler(GtkButton *bn, gpointer data);

struct widgets {

	GtkWidget *win;
	GObject *slider_sens_trans_x;
	GObject *slider_sens_trans_y;
	GObject *slider_sens_trans_z;
	GObject *slider_sens_rot_x;
	GObject *slider_sens_rot_y;
	GObject *slider_sens_rot_z;
	GObject *slider_deadzone_trans_x;
	GObject *slider_deadzone_trans_y;
	GObject *slider_deadzone_trans_z;
	GObject *slider_deadzone_rot_x;
	GObject *slider_deadzone_rot_y;
	GObject *slider_deadzone_rot_z;

};

static struct widgets widgets;

static int fd;


enum {TX, TY, TZ, RX, RY, RZ};
#define MAX_BUTTONS		64

static int def_axinv = 0x36;	/* invert TY, TZ, RY, RZ */

static float sensitivity;
static float sens_axis[6];
static int invert;
static int map_axis[6];
static int map_bn[MAX_BUTTONS];
static int dead_thres[6];
static int grab_device;
static int led, grab;
static int repeat_msec;

static void errorbox(const char *msg)
{
	GtkWidget *dlg;
	dlg = gtk_message_dialog_new(0, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, msg);
	gtk_dialog_run((GtkDialog*)dlg);
	gtk_widget_destroy(dlg);
}

int main(int argc, char **argv)
{
	int i;

	gtk_init(&argc, &argv);

	if((fd = spnav_open()) == -1) {
		errorbox("Failed to connect to spacenavd!");
		return 1;
	}
	if(spnav_protocol() < 1) {
		errorbox("Currently running version of spacenavd is too old for this version of the configuration tool.\n"
				"\nEither update to a recent version of spacenavd (v0.9 or later), or downgrade to spnavcfg v0.3.1.");
		return 1;
	}
	spnav_client_name("spnavcfg");

	/* TODO make these into a nice information panel */
	printf("Device: %s\n", spnav_dev_name(0, 0));
	printf("Path: %s\n", spnav_dev_path(0, 0));
	printf("Buttons: %d\n", spnav_dev_buttons());
	printf("Axes: %d\n", spnav_dev_axes());

	sensitivity = spnav_cfg_get_sens();
	spnav_cfg_get_axis_sens(sens_axis);
	invert = spnav_cfg_get_invert();
	led = spnav_cfg_get_led();
	grab = spnav_cfg_get_grab();

	for(i=0; i<6; i++) {
		map_axis[i] = spnav_cfg_get_axismap(i);
		dead_thres[i] = spnav_cfg_get_deadzone(i);
	}
	for(i=0; i<MAX_BUTTONS; i++) {
		map_bn[i] = spnav_cfg_get_bnmap(i);
	}

	create_ui();

	gtk_widget_show_all(widgets.win);

	gtk_main();
}

static int query_x11(void)
{
	Display *dpy;
	Window win, root_win;
	XTextProperty wname;
	Atom type, command_event;
	int fmt;
	unsigned long nitems, bytes_after;
	unsigned char *prop;

	if(!(dpy = XOpenDisplay(0))) {
		return 0;
	}
	root_win = DefaultRootWindow(dpy);

	if((command_event = XInternAtom(dpy, "CommandEvent", True)) == None) {
		XCloseDisplay(dpy);
		return 0;
	}

	XGetWindowProperty(dpy, root_win, command_event, 0, 1, False, AnyPropertyType,
			&type, &fmt, &nitems, &bytes_after, &prop);
	if(!prop) {
		XCloseDisplay(dpy);
		return 0;
	}

	win = *(Window*)prop;
	XFree(prop);

	if(!XGetWMName(dpy, win, &wname) || strcmp("Magellan Window", (char*)wname.value) != 0) {
		XCloseDisplay(dpy);
		return 0;
	}
	XCloseDisplay(dpy);

	/* found a magellan window, still it might belong to the 3dxsrv driver */
	/* XXX ping daemon */
	return 1;
}

static void create_ui(void)
{
	GObject *obj;
	GtkBuilder *gtk_builder;

	gtk_builder = gtk_builder_new();
	gtk_builder_add_from_string(gtk_builder, (gchar*)ui_xml, -1, NULL);

	widgets.win = GTK_WIDGET(gtk_builder_get_object(gtk_builder, "main"));

	obj = gtk_builder_get_object(gtk_builder, CHK_AXINV_TRANS_X);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), (invert ^ def_axinv) & 1);
	obj = gtk_builder_get_object(gtk_builder, CHK_AXINV_TRANS_Y);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), (invert ^ def_axinv) & 2);
	obj = gtk_builder_get_object(gtk_builder, CHK_AXINV_TRANS_Z);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), (invert ^ def_axinv) & 4);
	obj = gtk_builder_get_object(gtk_builder, CHK_AXINV_ROT_X);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), (invert ^ def_axinv) & 8);
	obj = gtk_builder_get_object(gtk_builder, CHK_AXINV_ROT_Y);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), (invert ^ def_axinv) & 0x10);
	obj = gtk_builder_get_object(gtk_builder, CHK_AXINV_ROT_Z);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), (invert ^ def_axinv) & 0x20);

	obj = gtk_builder_get_object(gtk_builder, CHK_SWAP_YZ);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), map_axis[1] == 1);

	obj = gtk_builder_get_object(gtk_builder, SLIDER_SENS_GLOBAL);
	gtk_range_set_value(GTK_RANGE(obj), sensitivity);

	obj = gtk_builder_get_object(gtk_builder, SLIDER_SENS_TRANS);
	gtk_range_set_value(GTK_RANGE(obj), sens_axis[0]);

	widgets.slider_sens_trans_x = gtk_builder_get_object(gtk_builder, SLIDER_SENS_TRANS_X);
	gtk_range_set_value(GTK_RANGE(widgets.slider_sens_trans_x), sens_axis[0]);
	widgets.slider_sens_trans_y = gtk_builder_get_object(gtk_builder, SLIDER_SENS_TRANS_Y);
	gtk_range_set_value(GTK_RANGE(widgets.slider_sens_trans_y), sens_axis[1]);
	widgets.slider_sens_trans_z = gtk_builder_get_object(gtk_builder, SLIDER_SENS_TRANS_Z);
	gtk_range_set_value(GTK_RANGE(widgets.slider_sens_trans_z), sens_axis[2]);

	obj = gtk_builder_get_object(gtk_builder, SLIDER_SENS_ROT);
	gtk_range_set_value(GTK_RANGE(obj), sens_axis[3]);

	widgets.slider_sens_rot_x = gtk_builder_get_object(gtk_builder, SLIDER_SENS_ROT_X);
	gtk_range_set_value(GTK_RANGE(widgets.slider_sens_rot_x), sens_axis[3]);
	widgets.slider_sens_rot_y = gtk_builder_get_object(gtk_builder, SLIDER_SENS_ROT_Y);
	gtk_range_set_value(GTK_RANGE(widgets.slider_sens_rot_y), sens_axis[4]);
	widgets.slider_sens_rot_z = gtk_builder_get_object(gtk_builder, SLIDER_SENS_ROT_Z);
	gtk_range_set_value(GTK_RANGE(widgets.slider_sens_rot_z), sens_axis[5]);

	obj = gtk_builder_get_object(gtk_builder, SLIDER_DEADZONE);
	gtk_range_set_value(GTK_RANGE(obj), dead_thres[TX]);

	widgets.slider_deadzone_trans_x = gtk_builder_get_object(gtk_builder, SLIDER_DEADZONE_TRANS_X);
	gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_trans_x), dead_thres[TX]);
	widgets.slider_deadzone_trans_y = gtk_builder_get_object(gtk_builder, SLIDER_DEADZONE_TRANS_Y);
	gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_trans_y), dead_thres[TY]);
	widgets.slider_deadzone_trans_z = gtk_builder_get_object(gtk_builder, SLIDER_DEADZONE_TRANS_Z);
	gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_trans_z), dead_thres[TZ]);
	widgets.slider_deadzone_rot_x = gtk_builder_get_object(gtk_builder, SLIDER_DEADZONE_ROT_X);
	gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_rot_x), dead_thres[RX]);
	widgets.slider_deadzone_rot_y = gtk_builder_get_object(gtk_builder, SLIDER_DEADZONE_ROT_Y);
	gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_rot_y), dead_thres[RY]);
	widgets.slider_deadzone_rot_z = gtk_builder_get_object(gtk_builder, SLIDER_DEADZONE_ROT_Z);
	gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_rot_z), dead_thres[RZ]);

	obj = gtk_builder_get_object(gtk_builder, CHK_GRAB_DEVICE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), grab_device);

	obj = gtk_builder_get_object(gtk_builder, CHK_ENABLE_LED);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), led);

	gtk_builder_connect_signals(gtk_builder, NULL);

	g_object_unref(G_OBJECT(gtk_builder));

}

G_MODULE_EXPORT void chk_handler(GtkToggleButton *bn, gpointer data)
{
	int tmp;
	int state = gtk_toggle_button_get_active(bn);
	const gchar* ctrlname = gtk_buildable_get_name(GTK_BUILDABLE(bn));

	if(strcmp(ctrlname, CHK_AXINV_TRANS_X) == 0) {
		invert ^= 1;
		spnav_cfg_set_invert(invert);
	} else if(strcmp(ctrlname, CHK_AXINV_TRANS_Y) == 0) {
		invert ^= 2;
		spnav_cfg_set_invert(invert);
	} else if(strcmp(ctrlname, CHK_AXINV_TRANS_Z) == 0) {
		invert ^= 4;
		spnav_cfg_set_invert(invert);
	} else if(strcmp(ctrlname, CHK_AXINV_ROT_X) == 0) {
		invert ^= 8;
		spnav_cfg_set_invert(invert);
	} else if(strcmp(ctrlname, CHK_AXINV_ROT_Y) == 0) {
		invert ^= 0x10;
		spnav_cfg_set_invert(invert);
	} else if(strcmp(ctrlname, CHK_AXINV_ROT_Z) == 0) {
		invert ^= 0x20;
		spnav_cfg_set_invert(invert);
	} else if(strcmp(ctrlname, CHK_GRAB_DEVICE) == 0) {
		spnav_cfg_set_grab(state);
	} else if(strcmp(ctrlname, CHK_ENABLE_LED) == 0) {
		printf("LED: %d\n", state);
		spnav_cfg_set_led(state);
	} else if(strcmp(ctrlname, CHK_SWAP_YZ) == 0) {
		tmp = map_axis[TY];
		map_axis[TY] = map_axis[TZ];
		map_axis[TZ] = tmp;
		tmp = map_axis[RY];
		map_axis[RY] = map_axis[RZ];
		map_axis[RZ] = tmp;
		spnav_cfg_set_axismap(TY, map_axis[TY]);
		spnav_cfg_set_axismap(TZ, map_axis[TZ]);
		spnav_cfg_set_axismap(RY, map_axis[RY]);
		spnav_cfg_set_axismap(RZ, map_axis[RZ]);
	}

}

G_MODULE_EXPORT void slider_handler(GtkRange *rng, gpointer data)
{
	int i;
	const gchar* ctrlname = gtk_buildable_get_name(GTK_BUILDABLE(rng));
	gdouble value = gtk_range_get_value(rng);

	if(strcmp(ctrlname, SLIDER_SENS_GLOBAL) == 0) {
		sensitivity = value;
		spnav_cfg_set_sens(value);
	} else if(strcmp(ctrlname, SLIDER_SENS_TRANS) == 0) {
		sens_axis[TX] = sens_axis[TY] = sens_axis[TZ] = value;
		spnav_cfg_set_axis_sens(sens_axis);
		gtk_range_set_value(GTK_RANGE(widgets.slider_sens_trans_x), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_sens_trans_y), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_sens_trans_z), value);
	} else if(strcmp(ctrlname, SLIDER_SENS_TRANS_X) == 0) {
		sens_axis[TX] = value;
		spnav_cfg_set_axis_sens(sens_axis);
	} else if(strcmp(ctrlname, SLIDER_SENS_TRANS_Y) == 0) {
		sens_axis[TY] = value;
		spnav_cfg_set_axis_sens(sens_axis);
	} else if(strcmp(ctrlname, SLIDER_SENS_TRANS_Z) == 0) {
		sens_axis[TZ] = value;
		spnav_cfg_set_axis_sens(sens_axis);
	} else if(strcmp(ctrlname, SLIDER_SENS_ROT) == 0) {
		sens_axis[RX] = sens_axis[RY] = sens_axis[RZ] = value;
		spnav_cfg_set_axis_sens(sens_axis);
		gtk_range_set_value(GTK_RANGE(widgets.slider_sens_rot_x), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_sens_rot_y), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_sens_rot_z), value);
	} else if(strcmp(ctrlname, SLIDER_SENS_ROT_X) == 0) {
		sens_axis[RX] = value;
		spnav_cfg_set_axis_sens(sens_axis);
	} else if(strcmp(ctrlname, SLIDER_SENS_ROT_Y) == 0) {
		sens_axis[RY] = value;
		spnav_cfg_set_axis_sens(sens_axis);
	} else if(strcmp(ctrlname, SLIDER_SENS_ROT_Z) == 0) {
		sens_axis[RZ] = value;
		spnav_cfg_set_axis_sens(sens_axis);
	} else if(strcmp(ctrlname, SLIDER_DEADZONE) == 0) {
		for(i=0; i<6; i++) {
			dead_thres[i] = value;
			spnav_cfg_set_deadzone(i, value);
		}
		gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_trans_x), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_trans_y), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_trans_z), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_rot_x), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_rot_y), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_rot_z), value);
	} else if(strcmp(ctrlname, SLIDER_DEADZONE_TRANS_X) == 0) {
		dead_thres[TX] = value;
		spnav_cfg_set_deadzone(TX, value);
	} else if(strcmp(ctrlname, SLIDER_DEADZONE_TRANS_Y) == 0) {
		dead_thres[TY] = value;
		spnav_cfg_set_deadzone(TY, value);
	} else if(strcmp(ctrlname, SLIDER_DEADZONE_TRANS_Z) == 0) {
		dead_thres[TZ] = value;
		spnav_cfg_set_deadzone(TZ, value);
	} else if(strcmp(ctrlname, SLIDER_DEADZONE_ROT_X) == 0) {
		dead_thres[RX] = value;
		spnav_cfg_set_deadzone(RX, value);
	} else if(strcmp(ctrlname, SLIDER_DEADZONE_ROT_Y) == 0) {
		dead_thres[RY] = value;
		spnav_cfg_set_deadzone(RY, value);
	} else if(strcmp(ctrlname, SLIDER_DEADZONE_ROT_Z) == 0) {
		dead_thres[RZ] = value;
		spnav_cfg_set_deadzone(RZ, value);
	}
}

G_MODULE_EXPORT void bn_handler(GtkButton *bn, gpointer data)
{
	GtkWidget *dlg;
	int tmp;
	const gchar* ctrlname = gtk_buildable_get_name(GTK_BUILDABLE(bn));

#if 0
	if(strcmp(ctrlname, BTN_PING) == 0) {
		if(tmp) {	/* daemon alive */
			dlg = gtk_message_dialog_new(GTK_WINDOW(widgets.win), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "The spacenavd driver is running fine.");
			gtk_widget_show_all(dlg);
			g_signal_connect_swapped(dlg, "response", G_CALLBACK(gtk_widget_destroy), dlg);
		} else {	/* daemon dead */
			dlg = gtk_message_dialog_new(GTK_WINDOW(widgets.win), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "The driver isn't running at the moment.\n"
				"You can still modify the configuration through this panel though.");
			gtk_widget_show_all(dlg);
			g_signal_connect_swapped(dlg, "response", G_CALLBACK(gtk_widget_destroy), dlg);
		}
	}
#endif
}
